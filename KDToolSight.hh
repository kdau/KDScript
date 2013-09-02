/******************************************************************************
 *  KDToolSight.hh
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

#ifndef KDTOOLSIGHT_HH
#define KDTOOLSIGHT_HH

#include "KDHUDElement.hh"

class KDToolSight : public KDHUDElement
{
public:
	KDToolSight (const String& name, const Object& host);

private:
	virtual void initialize ();
	virtual bool prepare ();
	virtual void redraw ();

	Message::Result on_inv_select (Message&);
	Message::Result on_inv_deselect (Message&);

	Message::Result on_frob_inv_end (FrobMessage&);

	Message::Result on_property_change (PropertyChangeMessage&);

	static const HUD::ZIndex PRIORITY;
	static const CanvasSize SYMBOL_SIZE;

	bool selected; // not persistent because selection isn't
	Parameter<bool> deselect_on_use;

	Parameter<Image> image;
	Parameter<Color> color;
	Parameter<int> offset_x, offset_y;
};

#endif // KDTOOLSIGHT_HH

