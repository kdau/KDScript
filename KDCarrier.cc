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
	  THIEF_PARAMETER (create_attachments, true),
	  THIEF_PERSISTENT_FULL (detected_braindeath, false),
	  THIEF_PERSISTENT_FULL (detected_slaying, false)
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
	Script::initialize ();
	ObjectProperty::subscribe ("DeathStage", host ());
}

void
KDCarrier::deinitialize ()
{
	Script::deinitialize ();
	ObjectProperty::unsubscribe ("DeathStage", host ());
}



Message::Result
KDCarrier::on_sim (SimMessage& message)
{
	if (message.event == SimMessage::START)
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
		AI::Joint joint = attachment.joint;

		// Don't attach two objects to the same joint.
		bool has_existing = false;
		for (auto& existing : CreatureAttachmentLink::get_all (host ()))
			if (joint == existing.joint)
				{ has_existing = true; break; }
		if (has_existing) continue;

		log (Log::NORMAL, "Attaching a new %|| to joint %||.",
			attachment.get_dest (), int (joint));

		CreatureAttachmentLink::create (host (),
			Object::create (attachment.get_dest ()), joint);
	}
}



Message::Result
KDCarrier::on_ai_mode_change (AIModeMessage& message)
{
	if (message.new_mode == AI::Mode::DEAD) // killed or knocked out
	{
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
KDCarrier::on_property_change (PropertyMessage& message)
{
	if (message.property == Property ("DeathStage") &&
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
	if (message.new_level > AI::Alert::NONE)
		notify_carried ("CarrierAlerted", false,
			int (message.new_level));
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

