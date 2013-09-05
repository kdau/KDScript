/******************************************************************************
 *  KDCarrier.cc
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "KDCarrier.hh"

KDCarrier::KDCarrier (const String& _name, const Object& _host)
	: Script (_name, _host),
	  PARAMETER (create_attachments, true),
	  PERSISTENT (detected_braindeath, false),
	  PERSISTENT (detected_slaying, false)
{
	listen_message ("Sim", &KDCarrier::on_sim);
	listen_message ("Create", &KDCarrier::on_create);

	listen_message ("AIModeChange", &KDCarrier::on_ai_mode_change);
	listen_message ("IgnorePotion", &KDCarrier::on_ignore_potion);

	listen_message ("Slain", &KDCarrier::on_slain);
	listen_message ("PropertyChange", &KDCarrier::on_property_change);

	listen_message ("Alertness", &KDCarrier::on_alertness);
}

void
KDCarrier::initialize ()
{
	ObjectProperty::subscribe ("DeathStage", host ());
}



Message::Result
KDCarrier::on_sim (SimMessage& message)
{
	if (message.is_starting ())
		do_create_attachments ();
	return Message::HALT;
}

Message::Result
KDCarrier::on_create (Message&)
{
	do_create_attachments ();
	return Message::HALT;
}

void
KDCarrier::do_create_attachments ()
{
	if (!create_attachments) return;

	for (auto& attachment : CreatureAttachmentLink::get_all
		(host (), Object::ANY, Link::Inheritance::SOURCE))
	{
		CreatureAttachmentLink::Joint joint = attachment.joint;

		// Don't attach two objects to the same joint.
		bool has_existing = false;
		for (auto& existing : CreatureAttachmentLink::get_all (host ()))
			if (joint == existing.joint)
				{ has_existing = true; break; }
		if (has_existing) continue;

		mono () << "Attaching a new "
			<< attachment.get_dest ().get_editor_name ()
			<< " to joint " << int (joint) << "."
			<< std::endl;

		CreatureAttachmentLink::create (host (),
			Object::create (attachment.get_dest ()), joint);
	}
}



Message::Result
KDCarrier::on_ai_mode_change (AIModeChangeMessage& message)
{
	if (message.get_new_mode () == AI::Mode::DEAD)
	{ // The AI has been killed or knocked out.
		if (detected_braindeath) // The message has already been sent.
			detected_braindeath = false;
		else
			notify_carried ("CarrierBrainDead");
	}
	return Message::HALT;
}

Message::Result
KDCarrier::on_ignore_potion (Message&)
{
	// The AI is being knocked out.
	detected_braindeath = true;
	notify_carried ("CarrierBrainDead", true);
	return Message::HALT;
}



Message::Result
KDCarrier::on_slain (SlayMessage&)
{
	if (detected_slaying) // The message has already been sent.
		detected_slaying = false;
	else
		notify_carried ("CarrierSlain");
	return Message::HALT;
}

Message::Result
KDCarrier::on_property_change (PropertyChangeMessage& message)
{
	if (message.get_property () == Property ("DeathStage") &&
	    host_as<Damageable> ().death_stage == 12) // The AI is being slain.
	{
		detected_slaying = true;
		notify_carried ("CarrierSlain", true);
	}
	return Message::HALT;
}



Message::Result
KDCarrier::on_alertness (AIAlertnessMessage& message)
{
	AI::Alert level = message.get_new_level ();
	if (level > AI::Alert::NONE)
		notify_carried ("CarrierAlerted", false, int (level));
	return Message::HALT;
}



void
KDCarrier::notify_carried (const char* _message, bool _delay, int data)
{
	auto message = GenericMessage::with_data (_message, data);
	Time delay = _delay ? 250ul : 0ul;
	message.broadcast (host (), "Contains", delay);
	message.broadcast (host (), "CreatureAttachment", delay);
	message.broadcast (host (), "~DetailAttachement", delay);
}

