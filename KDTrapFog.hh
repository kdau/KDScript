/******************************************************************************
 *  KDTrapFog.hh
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

#ifndef KDTRAPFOG_HH
#define KDTRAPFOG_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDTrapFog : public TrapTrigger
{
public:
	KDTrapFog (const String& name, const Object& host);

private:
	virtual Message::Result on_trap (bool on, Message&);
	bool step ();

	Transition transition;

	Parameter<Fog::Zone> fog_zone;
	Parameter<Color> fog_color_on, fog_color_off;
	Parameter<float> fog_dist_on, fog_dist_off;

	Persistent<Color> start_color, end_color;
	Persistent<float> start_distance, end_distance;
};

#endif // KDTRAPFOG_HH

