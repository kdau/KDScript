/******************************************************************************
 *  KDTrapNextMission.hh
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

#ifndef KDTRAPNEXTMISSION_HH
#define KDTRAPNEXTMISSION_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDTrapNextMission : public TrapTrigger
{
public:
	KDTrapNextMission (const String& name, const Object& host);

private:
	virtual Message::Result on_trap (bool on, Message&);

	Parameter<int> next_mission_on, next_mission_off;
};

#endif // KDTRAPNEXTMISSION_HH

