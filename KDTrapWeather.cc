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
	  PERSISTENT (start_freq), PERSISTENT (end_freq),

	  PARAMETER (precip_speed_on, -1.0f),
	  PARAMETER (precip_speed_off, -1.0f),
	  PERSISTENT (start_speed), PERSISTENT (end_speed),

	  PARAMETER (precip_radius_on, -1.0f),
	  PARAMETER (precip_radius_off, -1.0f),
	  PERSISTENT (start_radius), PERSISTENT (end_radius),

	  PARAMETER (precip_opacity_on, -1.0f),
	  PARAMETER (precip_opacity_off, -1.0f),
	  PERSISTENT (start_opacity), PERSISTENT (end_opacity),

	  PARAMETER (precip_brightness_on, -1.0f),
	  PARAMETER (precip_brightness_off, -1.0f),
	  PERSISTENT (start_brightness), PERSISTENT (end_brightness),

	  PARAMETER (precip_wind_on, Vector ()),
	  PARAMETER (precip_wind_off, Vector ()),
	  PERSISTENT (start_wind), PERSISTENT (end_wind)
{}

bool
KDTrapWeather::prepare (bool on)
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

	return freq.exists () || speed.exists () || radius.exists () ||
		opacity.exists () || brightness.exists () || wind.exists ();
}

bool
KDTrapWeather::increment ()
{
	Precipitation precip = Mission::get_precipitation ();
	precip.frequency = interpolate (start_freq, end_freq);
	precip.speed = interpolate (start_speed, end_speed);
	precip.radius = interpolate (start_radius, end_radius);
	precip.opacity = interpolate (start_opacity, end_opacity);
	precip.brightness = interpolate (start_brightness, end_brightness);
	precip.wind = interpolate (start_wind, end_wind);
	Mission::set_precipitation (precip);
	return true;
}

