/******************************************************************************
 *  KDTrapFog.cc
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

#include "KDTrapFog.hh"

KDTrapFog::KDTrapFog (const String& _name, const Object& _host)
	: TrapTrigger (_name, _host),
	  transition (*this, &KDTrapFog::step, "Fog"),
	  THIEF_PARAMETER (fog_zone, Fog::GLOBAL),
	  THIEF_PARAMETER (fog_color_on),
	  THIEF_PARAMETER (fog_color_off),
	  THIEF_PARAMETER (fog_dist_on, -1.0f),
	  THIEF_PARAMETER (fog_dist_off, -1.0f),
	  THIEF_PERSISTENT (start_color),
	  THIEF_PERSISTENT (end_color),
	  THIEF_PERSISTENT (start_distance),
	  THIEF_PERSISTENT (end_distance)
{}

Message::Result
KDTrapFog::on_trap (bool on, Message&)
{
	Color _end_color = on ? fog_color_on : fog_color_off;
	float _end_distance = on ? fog_dist_on : fog_dist_off;

	if (fog_zone < Fog::GLOBAL || fog_zone > Fog::_MAX_ZONE ||
	    (_end_color == Color () && _end_distance < 0.0f))
		return Message::HALT;

	Fog current = Mission::get_fog (fog_zone);
	start_color = current.color;
	start_distance = current.distance;
	end_color = (_end_color != Color ()) ? _end_color : start_color;
	end_distance = (_end_distance >= 0.0f) ? _end_distance : start_distance;

	// Notify the Player object in case KDSyncGlobalFog is present.
	GenericMessage::with_data ("FogZoneChange",
		Fog::Zone (fog_zone), Color (end_color), float (end_distance))
			.send (host (), Player ());

	log (Log::NORMAL, "Starting fog transition for zone %|| from color %|| "
		"at distance %|| to color %|| at distance %|| over %|| ms.",
		int (fog_zone), start_color, start_distance, end_color,
		end_distance, transition.length);
	transition.start ();
	return Message::HALT;
}

bool
KDTrapFog::step ()
{
	Mission::set_fog (fog_zone, Fog {
		transition.interpolate (start_color, end_color),
		Fog::interpolate_distance (fog_zone == Fog::GLOBAL,
			start_distance, end_distance,
			transition.get_progress (), transition.curve)
	});
	return true;
}

