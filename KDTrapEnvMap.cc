/******************************************************************************
 *  KDTrapEnvMap.cc
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

#include "KDTrapEnvMap.hh"

KDTrapEnvMap::KDTrapEnvMap (const String& _name, const Object& _host)
	: TrapTrigger (_name, _host),
	  THIEF_PARAMETER (env_map_zone, 0u),
	  THIEF_PARAMETER (env_map_on),
	  THIEF_PARAMETER (env_map_off)
{}

Message::Result
KDTrapEnvMap::on_trap (bool on, Message&)
{
	if (Engine::get_version () < Version (1, 20))
	{
		log (Log::ERROR, "This script cannot be used with this version "
			"of the Dark Engine. Upgrade to NewDark version 1.20 or "
			"higher.");
		return Message::ERROR;
	}

	if (env_map_zone < 0 || env_map_zone >= 64)
	{
		log (Log::ERROR, "The environment map zone %|| is invalid. It "
			"must be between 0 and 63, inclusive.", env_map_zone);
		return Message::ERROR;
	}

	const String& texture = on ? env_map_on : env_map_off;
	if (!texture.empty ())
	{
		log (Log::NORMAL, "Setting environment map zone %|| to "
			"texture %||.", env_map_zone, texture);
		Mission::set_envmap_texture (env_map_zone, texture);
	}

	return Message::HALT;
}

