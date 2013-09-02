/******************************************************************************
 *  KDCarried.hh
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDCARRIED_HH
#define KDCARRIED_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDCarried : public Script
{
public:
	KDCarried (const String& name, const Object& host);

private:
	Message::Result on_post_sim (Message&);
	Message::Result on_create (Message&);

	Message::Result on_carrier_alerted (Message&);
	Message::Result on_drop (Message&);
	Message::Result on_fix_physics (Message&);

	Parameter<AI::Alert> drop_on_alert;
	Parameter<bool> was_dropped, inert_until_dropped, off_when_dropped;
};

#endif // KDCARRIED_HH

