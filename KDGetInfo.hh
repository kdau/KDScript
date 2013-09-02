/******************************************************************************
 *  KDGetInfo.hh
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

#ifndef KDGETINFO_HH
#define KDGETINFO_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDGetInfo : public Script
{
public:
	KDGetInfo (const String& name, const Object& host);

private:
	Message::Result on_begin_script (Message&);
	Message::Result on_mode_change (GameModeChangeMessage&);
	Message::Result on_update_variables (Message&);
	Message::Result on_end_script (Message&);
};

#endif // KDGETINFO_HH

