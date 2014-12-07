/******************************************************************************
 *  KDScriptDemo.cc
 *
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDScriptDemo.hh"

KDScriptDemo::KDScriptDemo (const String& _name, const Object& _host)
	: Script (_name, _host),
	  THIEF_PARAMETER (on_ambient),
	  THIEF_PARAMETER (off_ambient)
{
	listen_message ("Sim", &KDScriptDemo::on_sim);
	listen_message ("TurnOn", &KDScriptDemo::on_turn_on);
	listen_message ("TurnOff", &KDScriptDemo::on_turn_off);
}

Message::Result
KDScriptDemo::on_sim (SimMessage& message)
{
	if (message.event != SimMessage::START)
		return Message::CONTINUE;
	for (auto& link : ScriptParamsLink::get_all_by_data (host (), "Enable"))
		link.get_dest ().remove_metaprop (Object ("FrobInert"));
	for (auto& link : ScriptParamsLink::get_all_by_data (host (), "Destroy"))
		link.get_dest ().destroy ();
	return Message::CONTINUE;
}

Message::Result
KDScriptDemo::on_turn_on (Message&)
{
	Room ("DemoArena").ambient_schema = on_ambient;
	return Message::HALT;
}

Message::Result
KDScriptDemo::on_turn_off (Message&)
{
	Room ("DemoArena").ambient_schema = off_ambient;
	return Message::HALT;
}

