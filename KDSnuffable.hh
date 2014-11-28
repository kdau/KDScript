/******************************************************************************
 *  KDSnuffable.hh
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

#ifndef KDSNUFFABLE_HH
#define KDSNUFFABLE_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDSnuffable : public Script
{
public:
	KDSnuffable (const String& name, const Object& host);

private:
	virtual void initialize ();
	Message::Result on_post_sim (Message&);

	Message::Result turn_on (Message&);
	void on_common ();

	Message::Result turn_off (Message&);
	void off_common ();

	Message::Result toggle (Message&);

	Persistent<AnimLight::Mode> on_mode, off_mode;
	Parameter<bool> relight_on_frob;
	Parameter<Object> light_on_schema, light_off_schema;
};

#endif // KDSNUFFABLE_HH

