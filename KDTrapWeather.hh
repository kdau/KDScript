/******************************************************************************
 *  KDTrapWeather.hh
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

#ifndef KDTRAPWEATHER_HH
#define KDTRAPWEATHER_HH

#include "KDTransitionTrap.hh"

class KDTrapWeather : public KDTransitionTrap
{
public:
	KDTrapWeather (const String& name, const Object& host);

private:
	virtual bool prepare (bool on);
	virtual bool increment ();

	Parameter<float> precip_freq_on, precip_freq_off;
	Parameter<float> precip_speed_on, precip_speed_off;

	Persistent<float> start_freq, end_freq;
	Persistent<float> start_speed, end_speed;
};

#endif // KDTRAPWEATHER_HH

