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
	: KDTransitionTrap (_name, _host),
	  PARAMETER (fog_zone, Fog::GLOBAL),
	  PARAMETER (fog_color_on),
	  PARAMETER (fog_color_off),
	  PARAMETER (fog_dist_on, -1.0f),
	  PARAMETER (fog_dist_off, -1.0f),
	  PERSISTENT_ (start_color),
	  PERSISTENT_ (end_color),
	  PERSISTENT_ (start_distance),
	  PERSISTENT_ (end_distance)
{}

bool
KDTrapFog::prepare (bool on)
{
	Color _end_color = on ? fog_color_on : fog_color_off;
	float _end_distance = on ? fog_dist_on : fog_dist_off;

	if (fog_zone < Fog::GLOBAL || fog_zone > Fog::_MAX_ZONE ||
	    (_end_color == Color () && _end_distance < 0.0f))
		return false;

	Fog current = Mission::get_fog (fog_zone);
	start_color = current.color;
	start_distance = current.distance;
	end_color = (_end_color != Color ()) ? _end_color : start_color;
	end_distance = (_end_distance >= 0.0f) ? _end_distance : start_distance;

	// Notify the Player object in case KDSyncGlobalFog is present.
	GenericMessage notice ("FogZoneChange");
	notice.set_data (Message::DATA1, Fog::Zone (fog_zone));
	notice.set_data (Message::DATA2, Color (end_color));
	notice.set_data (Message::DATA3, float (end_distance));
	notice.send (host (), Player ());

	return true;
}

bool
KDTrapFog::increment ()
{
	Mission::set_fog (fog_zone, Fog {
		interpolate (start_color, end_color),
		Fog::interpolate_distance (fog_zone == Fog::GLOBAL,
			start_distance, end_distance, get_progress (), curve)
	});
	return true;
}

