/******************************************************************************
 *  KDTrapShowImage.hh
 *
 *  Copyright (C) 2014 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDTRAPSHOWIMAGE_HH
#define KDTRAPSHOWIMAGE_HH

#include "KDHUDElement.hh"

class KDTrapShowImage : public TrapTrigger, public KDHUDElement
{
public:
	KDTrapShowImage (const String& name, const Object& host);

private:
	virtual void initialize ();
	virtual void deinitialize ();

	virtual bool prepare ();
	virtual void redraw ();

	virtual Message::Result on_trap (bool on, Message&);
	Message::Result on_property_change (PropertyMessage&);

	static const ZIndex PRIORITY;

	Persistent<bool> enabled;

	Parameter<Image> image;
	Parameter<bool> use_hud;
	Parameter<Position> position;
	Parameter<int> offset_x, offset_y;
};

#endif // KDTRAPSHOWIMAGE_HH

