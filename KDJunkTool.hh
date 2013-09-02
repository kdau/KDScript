/******************************************************************************
 *  KDJunkTool.hh
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

#ifndef KDJUNKTOOL_HH
#define KDJUNKTOOL_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDJunkTool : public Script
{
public:
	KDJunkTool (const String& name, const Object& host);

private:
	Message::Result on_contained (ContainmentMessage&);
	Message::Result on_destroy (GenericMessage&);
	void start_carry ();
	void finish_carry ();

	Message::Result on_clear_weapon (TimerMessage&);

	Message::Result on_needs_reselect (GenericMessage&);
	Message::Result on_reselect (GenericMessage&);
	Object create_frobbable ();

	Message::Result on_needs_tool_use (GenericMessage&);
	Message::Result on_start_tool_use (TimerMessage&);

	Message::Result on_slain (SlayMessage&);

	Parameter<bool> lugged, drop;
	Persistent<Weapon> previous_weapon;
};

#endif // KDJUNKTOOL_HH

