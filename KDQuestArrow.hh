/******************************************************************************
 *  KDQuestArrow.hh
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

#ifndef KDQUESTARROW_HH
#define KDQUESTARROW_HH

#include "KDHUDElement.hh"

class KDQuestArrow : public Script, public KDHUDElement
{
public:
	KDQuestArrow (const String& name, const Object& host);

private:
	virtual void initialize ();
	virtual void deinitialize ();

	virtual bool prepare ();
	virtual void redraw ();

	Message::Result on_on (Message&);
	Message::Result on_off (Message&);
	Message::Result on_contained (ContainmentMessage&);
	Message::Result on_ai_mode_change (AIModeMessage&);

	Message::Result on_property_change (PropertyMessage&);
	void update_text ();

	static const ZIndex PRIORITY;
	static const CanvasSize SYMBOL_SIZE;
	static const int PADDING;

	Persistent<bool> enabled;
	Parameter<Objective> objective;
	Parameter<float> range; float distance;
	Parameter<bool> obscured;

	Parameter<Image> image;
	Parameter<String> _text; String text;
	Parameter<Color> color;
	Parameter<bool> shadow;

	Direction direction;
	CanvasPoint image_pos;
	CanvasPoint text_pos;
};

#endif // KDQUESTARROW_HH

