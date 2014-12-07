/******************************************************************************
 *  KDRoomAmbient.hh
 *
 *  Copyright (C) 2012-2014 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDROOMAMBIENT_HH
#define KDROOMAMBIENT_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDRoomAmbient : public Script
{
public:
	KDRoomAmbient (const String& name, const Object& host);

private:
	virtual void initialize ();
	virtual void deinitialize ();

	Message::Result on_player_enter (Message&);
	Message::Result on_property_change (PropertyMessage&);

	AmbientHacked get_fnord (bool create = true);
	bool is_owning_room ();
	void set_owning_room ();
	void set_ambient ();
};

#endif // KDROOMAMBIENT_HH

