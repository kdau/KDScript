/******************************************************************************
 *  KDCarried.cc
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

#include "KDCarried.hh"

KDCarried::KDCarried (const String& _name, const Object& _host)
	: Script (_name, _host),
	  PARAMETER (drop_on_alert, AI::Alert::NONE),
	  PARAMETER (was_dropped, false),
	  PARAMETER (inert_until_dropped, false),
	  PARAMETER (off_when_dropped, false)
{
	listen_message ("PostSim", &KDCarried::on_post_sim);
	listen_message ("Create", &KDCarried::on_create);
	listen_message ("CarrierAlerted", &KDCarried::on_carrier_alerted);
	listen_message ("CarrierBrainDead", &KDCarried::on_drop);
	listen_message ("CarrierSlain", &KDCarried::on_drop);
	listen_message ("Drop", &KDCarried::on_drop);
	listen_message ("FixPhysics", &KDCarried::on_fix_physics);
}

Message::Result
KDCarried::on_post_sim (Message&)
{
	// Add the FrobInert metaproperty, if requested.
	if (inert_until_dropped)
	{
		host ().add_metaprop (Object ("FrobInert"));

		// Decrease the pickable pocket count if Contains-linked.
		ContainsLink container = Link::get_one ("~Contains", host ());
		if (container != Link::NONE &&
		    container.type != Container::Type::GENERIC)
		{
			QuestVar pockets ("DrSPocketCnt");
			pockets.set (pockets.get () - 1);
		}
	}

	return Message::CONTINUE;
}

Message::Result
KDCarried::on_create (Message&)
{
	// Only proceed for objects created in-game.
	if (Engine::get_mode () != Engine::Mode::GAME)
		return Message::HALT;

	// Don't affect any already-dropped clone.
	if (was_dropped)
		return Message::HALT;

	// Add the FrobInert metaproperty, if requested.
	if (inert_until_dropped)
		host ().add_metaprop (Object ("FrobInert"));

	return Message::CONTINUE;
}

Message::Result
KDCarried::on_carrier_alerted (Message& message)
{
	AI::Alert new_alert = message.get_data (Message::DATA1, AI::Alert::NONE);
	if (drop_on_alert > AI::Alert::NONE && drop_on_alert <= new_alert)
		return on_drop (message);
	else
		return Message::CONTINUE;
}

Message::Result
KDCarried::on_drop (Message&)
{
	Physical dropped = host ();
	was_dropped = true;

	Vector location = dropped.get_location (),
		rotation = dropped.get_rotation ();

	// Remove the FrobInert metaproperty, if requested.
	if (inert_until_dropped)
		dropped.remove_metaprop (Object ("FrobInert"));

	// Turn off the object and ControlDevice-linked objects, if requested.
	if (off_when_dropped)
	{
		GenericMessage ("TurnOff").send (dropped, dropped);
		GenericMessage ("TurnOff").broadcast (dropped, "ControlDevice");
	}

#ifdef IS_THIEF2
	// Confirm that the object would not fall out of the world if dropped.
	Physical position_test;
	if (dropped.is_physical ())
		position_test = dropped;
	else
	{
		position_test = Object::create_temp_fnord ();
		position_test.set_position (location, rotation);
		position_test.physics_type = Physical::PhysicsType::OBB;
	}
	if (!position_test.is_position_valid ())
	{
		mono () << "Not dropping from invalid location " << location
			<< "." << std::endl;
		return Message::HALT;
	}
#endif // IS_THIEF2

	ContainsLink container = Link::get_one ("~Contains", dropped);
	if (container != Link::NONE)
		container.destroy ();

	Link creature = Link::get_one ("~CreatureAttachment", dropped);
	if (creature != Link::NONE)
		creature.destroy ();

	Link detail = Link::get_one ("DetailAttachement", dropped);
	if (detail != Link::NONE)
	{
		dropped = dropped.clone ();
		mono () << "Replacing self with clone: "
			<< dropped.get_editor_name () << std::endl;

		// Add a reference link to the ex-carrying AI.
		Link::create ("CulpableFor", detail.get_dest (), dropped);

		// Unlink from the carrier, which will destroy this object.
		detail.destroy ();

		// Remove the clone's FrobInert if requested. Yes, again.
		if (inert_until_dropped)
			dropped.remove_metaprop (Object ("FrobInert"));
	}

	// Ensure that the object is physical.
	if (!dropped.is_physical ())
	{
		// Create an OBB model to allow check of object dimensions.
		dropped.physics_type = Physical::PhysicsType::OBB;

		// Schedule a switch to a correctly sized sphere model.
		GenericMessage ("FixPhysics").post (host (), dropped);
	}

	// Teleport the object to its original position. Yes, this is needed.
	dropped.set_position (location, rotation);

	// Give the object a small push to cause it to drop.
	dropped.velocity = { 0.0f, 0.0f, -0.1f };

	return Message::HALT;
}

Message::Result
KDCarried::on_fix_physics (Message&)
{
	// Get the object dimensions based on the temporary OBB model.
	Vector dims = host_as<OBBPhysical> ().physics_size;
	float radius = std::max ({ dims.x, dims.y, dims.z }) / 2.0f;

	// Switch to a sphere model and set the appropriate radius.
	SpherePhysical sphere = host ();
	sphere.physics_type = Physical::PhysicsType::SPHERE;
	sphere.submodel_count = 1;
	if (radius > 0.0f) sphere.physics_radius_1 = radius;

	return Message::HALT;
}

