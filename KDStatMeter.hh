/******************************************************************************
 *  KDStatMeter.hh
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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

#ifndef KDSTATMETER_HH
#define KDSTATMETER_HH

#include "KDHUDElement.hh"

class KDStatMeter : public KDHUDElement
{
public:
	KDStatMeter (const String& name, const Object& host);

	enum class Style { PROGRESS, UNITS, GEM };

	enum class Position
	{
		NW,   NORTH,  NE,
		WEST, CENTER, EAST,
		SW,   SOUTH,  SE
	};

	enum class Orient { HORIZ, VERT };

	enum class Component { NONE, X, Y, Z };

private:
	virtual void initialize ();
	virtual bool prepare ();
	virtual void redraw ();

	CanvasSize get_request_size () const;

	Message::Result on_post_sim (GenericMessage&);

	Message::Result on_on (GenericMessage&);
	Message::Result on_off (GenericMessage&);

	Message::Result on_property_change (PropertyChangeMessage&);
	void update_text ();
	void update_range ();

	static const HUD::ZIndex PRIORITY;
	static const int MARGIN;

	Persistent<bool> enabled;

	// parameters

	Parameter<Style> style;
	Parameter<Image> image;
	Parameter<int> spacing;
	Parameter<String> _text; String text;

	Parameter<Position> position;
	Parameter<int> offset_x, offset_y;
	Parameter<Orient> orient;
	Parameter<int> request_w, request_h;

	Parameter<String> quest_var, prop_name, prop_field;
	Parameter<Component> prop_comp;
	Parameter<Object> prop_obj;

	Parameter<float> _min, _max; float min, max;
	Parameter<int> low, high;

	Parameter<Color> color_bg, color_low, color_med, color_high;

	// temporary data

	float value, value_pct;
	int value_int;
	enum class Tier { LOW, MEDIUM, HIGH } value_tier;

	CanvasPoint meter_pos;
	CanvasPoint text_pos;
};

#endif // KDSTATMETER_HH

