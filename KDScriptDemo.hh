/******************************************************************************
 *  KDScriptDemo.hh
 *
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDSCRIPTDEMO_HH
#define KDSCRIPTDEMO_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDScriptDemo : public Script
{
public:
	KDScriptDemo (const String& name, const Object& host);

private:
	Message::Result on_sim (SimMessage&);
	Message::Result on_turn_on (Message&);
	Message::Result on_turn_off (Message&);

	Parameter<String> on_ambient, off_ambient;
};

#endif // KDSCRIPTDEMO_HH

