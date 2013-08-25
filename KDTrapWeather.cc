/******************************************************************************
 *  KDTrapWeather.cc
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

#include "KDTrapWeather.hh"

KDTrapWeather::KDTrapWeather (const String& _name, const Object& _host)
        : KDTransitionTrap (_name, _host),
	  PARAMETER (precip_freq_on, -1.0f),
	  PARAMETER (precip_freq_off, -1.0f),
	  PARAMETER (precip_speed_on, -1.0f),
	  PARAMETER (precip_speed_off, -1.0f),
	  PERSISTENT (start_freq),
	  PERSISTENT (end_freq),
	  PERSISTENT (start_speed),
	  PERSISTENT (end_speed)
{}

bool
KDTrapWeather::prepare (bool on)
{
	float freq = on ? precip_freq_on : precip_freq_off;
	float speed = on ? precip_speed_on : precip_speed_off;

	Precipitation precip = Mission::get_precipitation ();
	start_freq = precip.frequency;
	start_speed = precip.speed;

	end_freq = (freq >= 0.0f) ? freq : start_freq;
	end_speed = (speed >= 0.0f) ? speed : start_speed;

	return (start_freq != end_freq) || (start_speed != end_speed);
}

bool
KDTrapWeather::increment ()
{
	Precipitation precip = Mission::get_precipitation ();
	precip.frequency = interpolate (start_freq, end_freq);
	precip.speed = interpolate (start_speed, end_speed);
	Mission::set_precipitation (precip);
	return true;
}

