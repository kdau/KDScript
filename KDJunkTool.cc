/******************************************************************************
 *  KDJunkTool.cc
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDJunkTool.hh"

KDJunkTool::KDJunkTool (const String& _name, const Object& _host)
	: Script (_name, _host),
	  PARAMETER_ (lugged, "junk_tool_lugged", true),
	  PARAMETER_ (drop, "junk_tool_drop", false),
	  PERSISTENT (previous_weapon, Object::NONE)
{
	listen_message ("Contained", &KDJunkTool::on_contained);
	listen_message ("Destroy", &KDJunkTool::on_destroy);

	listen_timer ("ClearWeapon", &KDJunkTool::on_clear_weapon);

	listen_message ("InvDeSelect", &KDJunkTool::on_needs_reselect);
	listen_message ("FrobInvEnd", &KDJunkTool::on_needs_reselect);
	listen_message ("FrobToolEnd", &KDJunkTool::on_needs_reselect);
	listen_message ("Reselect", &KDJunkTool::on_reselect);

	listen_message ("InvDeFocus", &KDJunkTool::on_needs_tool_use);
	listen_timer ("StartToolUse", &KDJunkTool::on_start_tool_use);

	listen_message ("Slain", &KDJunkTool::on_slain);

}



Message::Result
KDJunkTool::on_contained (ContainmentMessage& message)
{
	if (message.get_container ().inherits_from (Object ("Avatar")))
		switch (message.get_event ())
		{
		case ContainmentMessage::ADD: start_carry (); break;
		case ContainmentMessage::REMOVE: finish_carry (); break;
		default: break;
		}
	return Message::CONTINUE;
}

Message::Result
KDJunkTool::on_destroy (GenericMessage&)
{
	if (Player ().is_in_inventory (host ()))
		finish_carry ();
	return Message::CONTINUE;
}

void
KDJunkTool::start_carry ()
{
	Player player;
	if (!player.is_in_inventory (host ())) return;

	// Keep clearing any weapon while the tool is in inventory.
	start_timer ("ClearWeapon", 1, false);

	// If lugged: slow down the player, show limb model if any, and grunt.
	if (lugged)
	{
		player.add_speed_control ("JunkTool", 0.6f);
		if (host_as<Interactive> ().limb_model.exists ())
			player.show_arm ();
		SoundSchema ("garlift").play_voiceover ();
	}

	// Select the tool and start tool use.
	GenericMessage ("Reselect").post (host (), host ());
}

void
KDJunkTool::finish_carry ()
{
	Player player;

	// If a weapon had been selected, reselect it.
	if (previous_weapon != Object::NONE)
		player.select_weapon (previous_weapon);

	// For a lugged tool, restore player speed, hide carry model, and grunt.
	if (lugged)
	{
		player.remove_speed_control ("JunkTool");
		player.hide_arm ();
		SoundSchema ("gardrop").play_voiceover ();
	}
}



Message::Result
KDJunkTool::on_clear_weapon (TimerMessage&)
{
	Player player;
	if (player.is_in_inventory (host ()))
	{
		Weapon weapon = player.get_selected_weapon ();
		if (weapon != Object::NONE)
		{
			previous_weapon = weapon;
			player.clear_weapon ();
		}
		start_timer ("ClearWeapon", 100, false);
	}
	return Message::HALT;
}



Message::Result
KDJunkTool::on_needs_reselect (GenericMessage&)
{
	// Reselect the tool in the next cycle (won't work in this one).
	GenericMessage ("Reselect").post (host (), host ());
	return Message::CONTINUE;
}

Message::Result
KDJunkTool::on_reselect (GenericMessage&)
{
	Player player;
	if (player.is_in_inventory (host ()))
	{
		// (Re)select the tool.
		player.select_item (host ());

		// Create a fake frobbable and schedule to start tool use.
		create_frobbable ();
		start_timer ("StartToolUse", 50, false);
	}

	return Message::HALT;
}

Object
KDJunkTool::create_frobbable ()
{
	// The command for starting tool use does not work consistently. In some
	// cases, it does not work unless there is a frobbable item currently 
	// focused in the world. (I can't tell why.) To make the command always
	// be effective, this method creates a fake frobbable object in front of
	// the player that will destroy itself instantly, temporarily creating
	// the conditions required by the command.

	// Create the object and make it frobbable.
	Rendered frobbable = Object::create_temp_fnord (100ul);
	Interactive (frobbable).frob_world_action =
		Interactive::FrobAction::FROB_SCRIPTS;

	// Move and enlarge it to fill the player's view.
	frobbable.set_position ({ 2.0f, 0.0f, 0.0f }, Vector (), Player ());
	frobbable.model_scale = { 10.0f, 10.0f, 20.0f };

	// Make it technically, but not actually, visible.
	frobbable.render_type = Rendered::RenderType::NORMAL;
	frobbable.opacity = 0.01f;

	return frobbable;
}



Message::Result
KDJunkTool::on_needs_tool_use (GenericMessage&)
{
	start_timer ("StartToolUse", 1, false);
	return Message::CONTINUE;
}

Message::Result
KDJunkTool::on_start_tool_use (TimerMessage&)
{
	Player ().start_tool_use ();
	return Message::HALT;
}



Message::Result
KDJunkTool::on_slain (SlayMessage&)
{
	Player player;
	if (drop && player.is_in_inventory (host ()))
	{
		// Forget the slaying so the tool can be dropped again later.
		host_as<Damageable> ().resurrect (host ());

		// Drop the tool.
		player.drop_item ();
	}
	return Message::CONTINUE;
}



