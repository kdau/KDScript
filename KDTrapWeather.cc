/******************************************************************************
 *  KDTrapWeather.cc
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

#include "KDTrapWeather.hh"

KDTrapWeather::KDTrapWeather (const String& _name, const Object& _host)
	: TrapTrigger (_name, _host),
	  transition (*this, &KDTrapWeather::step, "Weather", 50ul,
		0ul, Curve::LINEAR, "transition", "curve"),

	  THIEF_PARAMETER (precip_freq_on, -1.0f),
	  THIEF_PARAMETER (precip_freq_off, -1.0f),
	  THIEF_PERSISTENT (start_freq),
	  THIEF_PERSISTENT (end_freq),

	  THIEF_PARAMETER (precip_speed_on, -1.0f),
	  THIEF_PARAMETER (precip_speed_off, -1.0f),
	  THIEF_PERSISTENT (start_speed),
	  THIEF_PERSISTENT (end_speed),

	  THIEF_PARAMETER (precip_radius_on, -1.0f),
	  THIEF_PARAMETER (precip_radius_off, -1.0f),
	  THIEF_PERSISTENT (start_radius),
	  THIEF_PERSISTENT (end_radius),

	  THIEF_PARAMETER (precip_opacity_on, -1.0f),
	  THIEF_PARAMETER (precip_opacity_off, -1.0f),
	  THIEF_PERSISTENT (start_opacity),
	  THIEF_PERSISTENT (end_opacity),

	  THIEF_PARAMETER (precip_brightness_on, -1.0f),
	  THIEF_PARAMETER (precip_brightness_off, -1.0f),
	  THIEF_PERSISTENT (start_brightness),
	  THIEF_PERSISTENT (end_brightness),

	  THIEF_PARAMETER (precip_wind_on, Vector ()),
	  THIEF_PARAMETER (precip_wind_off, Vector ()),
	  THIEF_PERSISTENT (start_wind),
	  THIEF_PERSISTENT (end_wind)
{}

Message::Result
KDTrapWeather::on_trap (bool on, Message&)
{
	Parameter<float>
		&freq = on ? precip_freq_on : precip_freq_off,
		&speed = on ? precip_speed_on : precip_speed_off,
		&radius = on ? precip_radius_on : precip_radius_off,
		&opacity = on ? precip_opacity_on : precip_opacity_off,
		&brightness = on ? precip_brightness_on : precip_brightness_off;
	Parameter<Vector>& wind = on ? precip_wind_on : precip_wind_off;

	Precipitation precip = Mission::get_precipitation ();
	start_freq = precip.frequency;
	start_speed = precip.speed;
	start_radius = precip.radius;
	start_opacity = precip.opacity;
	start_brightness = precip.brightness;
	start_wind = precip.wind;

	end_freq = (freq >= 0.0f) ? freq : start_freq;
	end_speed = (speed >= 0.0f) ? speed : start_speed;
	end_radius = (radius >= 0.0f) ? radius : start_radius;
	end_opacity = (opacity >= 0.0f) ? opacity : start_opacity;
	end_brightness = (brightness >= 0.0f) ? brightness : start_brightness;
	end_wind = wind.exists () ? Vector (wind) : start_wind;

	if (freq.exists () || speed.exists () || radius.exists () ||
	    opacity.exists () || brightness.exists () || wind.exists ())
	{
		log (Log::NORMAL, "Starting weather transition over %|| ms.",
			transition.length);
		transition.start ();
	}

	return Message::HALT;
}

bool
KDTrapWeather::step ()
{
	Precipitation precip = Mission::get_precipitation ();
	precip.frequency = transition.interpolate (start_freq, end_freq);
	precip.speed = transition.interpolate (start_speed, end_speed);
	precip.radius = transition.interpolate (start_radius, end_radius);
	precip.opacity = transition.interpolate (start_opacity, end_opacity);
	precip.brightness = transition.interpolate
		(start_brightness, end_brightness);
	precip.wind = transition.interpolate (start_wind, end_wind);
	Mission::set_precipitation (precip);
	return true;
}

