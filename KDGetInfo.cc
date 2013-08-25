/******************************************************************************
 *  KDGetInfo.cc
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDGetInfo.hh"

KDGetInfo::KDGetInfo (const String& _name, const Object& _host)
	: Script (_name, _host)
{
	listen_message ("BeginScript", &KDGetInfo::on_begin_script);
	listen_message ("DarkGameModeChange", &KDGetInfo::on_mode_change);
	listen_message ("UpdateVariables", &KDGetInfo::on_update_variables);
	listen_message ("EndScript", &KDGetInfo::on_end_script);
}

Message::Result
KDGetInfo::on_begin_script (GenericMessage&)
{
	// Some settings aren't available yet, so wait for next message cycle.
	GenericMessage ("UpdateVariables").post (host (), host ());
	return Message::CONTINUE;
}

Message::Result
KDGetInfo::on_mode_change (GameModeChangeMessage& message)
{
	if (message.is_resuming ())
		GenericMessage ("UpdateVariables").send (host (), host ());
	return Message::CONTINUE;
}

Message::Result
KDGetInfo::on_update_variables (GenericMessage&)
{
	QuestVar ("info_directx_version").set (Engine::get_directx_version ());

	CanvasSize canvas = Engine::get_canvas_size ();
	QuestVar ("info_display_height").set (canvas.h);
	QuestVar ("info_display_width").set (canvas.w);

	if (Engine::has_config ("sfx_eax"))
		QuestVar ("info_has_eax").set
			(Engine::get_config<int> ("sfx_eax"));

	if (Engine::has_config ("fogging"))
		QuestVar ("info_has_fog").set
			(Engine::get_config<int> ("fogging"));

	if (Engine::has_config ("game_hardware"))
		QuestVar ("info_has_hw3d").set
			(Engine::get_config<int> ("game_hardware"));

	if (Engine::has_config ("enhanced_sky"))
		QuestVar ("info_has_sky").set
			(Engine::get_config<int> ("enhanced_sky"));

	if (Engine::has_config ("render_weather"))
		QuestVar ("info_has_weather").set
			(Engine::get_config<int> ("render_weather"));

#ifdef IS_THIEF2
	if (Engine::get_mode () == Engine::Mode::GAME)
		QuestVar ("info_mission").set (Mission::get_number ());
#endif // IS_THIEF2

	QuestVar ("info_mode").set ((Engine::get_mode () == Engine::Mode::EDIT)
		? 1 : Engine::is_editor () ? 2 : 0);

	Version version = Engine::get_version ();
	QuestVar ("info_version_major").set (version.major);
	QuestVar ("info_version_minor").set (version.minor);

	return Message::CONTINUE;
}

Message::Result
KDGetInfo::on_end_script (GenericMessage&)
{
	QuestVar ("info_directx_version").unset ();
	QuestVar ("info_display_height").unset ();
	QuestVar ("info_display_width").unset ();
	QuestVar ("info_has_eax").unset ();
	QuestVar ("info_has_fog").unset ();
	QuestVar ("info_has_hw3d").unset ();
	QuestVar ("info_has_sky").unset ();
	QuestVar ("info_has_weather").unset ();
	QuestVar ("info_mission").unset ();
	QuestVar ("info_mode").unset ();
	QuestVar ("info_version_major").unset ();
	QuestVar ("info_version_minor").unset ();
	return Message::CONTINUE;
}

