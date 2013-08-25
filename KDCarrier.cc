/******************************************************************************
 *  KDCarrier.cc
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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
	  PARAMETER (create_attachments, true)
{
	listen_message ("Sim", &KDCarrier::on_sim);
	listen_message ("Create", &KDCarrier::on_create);
	listen_message ("AIModeChange", &KDCarrier::on_ai_mode_change);
	listen_message ("Alertness", &KDCarrier::on_alertness);
	listen_message ("Slain", &KDCarrier::on_slain);
}

Message::Result
KDCarrier::on_sim (SimMessage& message)
{
	if (message.is_starting ())
		do_create_attachments ();
	return Message::CONTINUE;
}

Message::Result
KDCarrier::on_create (GenericMessage&)
{
	do_create_attachments ();
	return Message::CONTINUE;
}

void
KDCarrier::do_create_attachments ()
{
	if (!create_attachments) return;

	for (auto& attachment : CreatureAttachmentLink::get_all
		(host (), Object::ANY, Link::Inheritance::SOURCE))
	{
		// Don't attach two objects to the same joint.
		bool has_existing = false;
		for (auto& existing : CreatureAttachmentLink::get_all (host ()))
			if (attachment.get_joint () == existing.get_joint ())
				{ has_existing = true; break; }
		if (has_existing) continue;

		mono () << "Attaching a new "
			<< attachment.get_dest ().get_editor_name ()
			<< " to joint " << int (attachment.get_joint ()) << "."
			<< std::endl;

		CreatureAttachmentLink::create (host (),
			Object::create (attachment.get_dest ()),
			attachment.get_joint ());
	}
}

Message::Result
KDCarrier::on_ai_mode_change (AIModeChangeMessage& message)
{
	if (message.get_new_mode () == AI::Mode::DEAD) // killed or knocked out
		notify_carried ("CarrierBrainDead");
	return Message::CONTINUE;
}

Message::Result
KDCarrier::on_slain (SlayMessage&)
{
	notify_carried ("CarrierSlain");
	return Message::CONTINUE;
}

Message::Result
KDCarrier::on_alertness (AIAlertnessMessage& message)
{
	AI::Alert level = message.get_new_level ();
	if (level > AI::Alert::NONE)
		notify_carried ("CarrierAlerted", int (level));
	return Message::CONTINUE;
}

void
KDCarrier::notify_carried (const char* _message, int data)
{
	GenericMessage message (_message);
	message.set_data (Message::DATA1, data);
	message.broadcast (host (), "Contains");
	message.broadcast (host (), "CreatureAttachment");
	message.broadcast (host (), "~DetailAttachement");
}

