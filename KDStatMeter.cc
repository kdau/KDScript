/******************************************************************************
 *  KDStatMeter.cc
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

#include "KDStatMeter.hh"



namespace Thief {

THIEF_ENUM_CODING (KDStatMeter::Style, CODE, CODE,
	THIEF_ENUM_VALUE (PROGRESS, "progress"),
	THIEF_ENUM_VALUE (UNITS, "units"),
	THIEF_ENUM_VALUE (GEM, "gem"),
)

THIEF_ENUM_CODING (KDStatMeter::Position, CODE, CODE,
	THIEF_ENUM_VALUE (NW, "nw"),
	THIEF_ENUM_VALUE (NORTH, "north", "n"),
	THIEF_ENUM_VALUE (NE, "ne"),
	THIEF_ENUM_VALUE (WEST, "west", "w"),
	THIEF_ENUM_VALUE (CENTER, "center", "c"),
	THIEF_ENUM_VALUE (EAST, "east", "e"),
	THIEF_ENUM_VALUE (SW, "sw"),
	THIEF_ENUM_VALUE (SOUTH, "south", "s"),
	THIEF_ENUM_VALUE (SE, "se"),
)

THIEF_ENUM_CODING (KDStatMeter::Orient, CODE, CODE,
	THIEF_ENUM_VALUE (HORIZ, "horiz"),
	THIEF_ENUM_VALUE (VERT, "vert"),
)

} // namespace Thief



const HUDElement::ZIndex
KDStatMeter::PRIORITY = 0;

const int
KDStatMeter::MARGIN = 16;



KDStatMeter::KDStatMeter (const String& _name, const Object& _host)
	: KDHUDElement (_name, _host, PRIORITY),

	  THIEF_PERSISTENT (enabled),
	  THIEF_PARAMETER_FULL (style, "stat_meter_style", Style::PROGRESS),
	  THIEF_PARAMETER_FULL (image, "stat_meter_image", Symbol::NONE, true, false),
	  THIEF_PARAMETER_FULL (spacing, "stat_meter_spacing", 8),
	  THIEF_PARAMETER_FULL (_text, "stat_meter_text"),

	  THIEF_PARAMETER_FULL (position, "stat_meter_position", Position::NW),
	  THIEF_PARAMETER_FULL (offset_x, "stat_meter_offset_x", 0),
	  THIEF_PARAMETER_FULL (offset_y, "stat_meter_offset_y", 0),
	  THIEF_PARAMETER_FULL (orient, "stat_meter_orient", Orient::HORIZ),
	  THIEF_PARAMETER_FULL (request_w, "stat_meter_width", -1),
	  THIEF_PARAMETER_FULL (request_h, "stat_meter_height", -1),

	  THIEF_PARAMETER_FULL (quest_var, "stat_source_qvar"),
	  THIEF_PARAMETER_FULL (prop_name, "stat_source_property"),
	  THIEF_PARAMETER_FULL (prop_field, "stat_source_field"),
	  THIEF_PARAMETER_FULL (prop_comp, "stat_source_component",
		Vector::Component::NONE),
	  THIEF_PARAMETER_FULL (prop_obj, "stat_source_object", host ()),

	  THIEF_PARAMETER_FULL (_min, "stat_range_min", 0.0f),
	  THIEF_PARAMETER_FULL (_max, "stat_range_max", 1.0f),
	  min (0.0f), max (1.0f),
	  THIEF_PARAMETER_FULL (low, "stat_range_low", 25),
	  THIEF_PARAMETER_FULL (high, "stat_range_high", 75),

	  THIEF_PARAMETER_FULL (color_bg, "stat_color_bg", Color (0x000000)),
	  THIEF_PARAMETER_FULL (color_low, "stat_color_low", Color (0x0000ff)),
	  THIEF_PARAMETER_FULL (color_med, "stat_color_med", Color (0x00ffff)),
	  THIEF_PARAMETER_FULL (color_high, "stat_color_high", Color (0x00ff00)),

	  value (0.0f),
	  value_pct (0.0f),
	  value_int (0),
	  value_tier (Tier::MEDIUM)
{
	listen_message ("PostSim", &KDStatMeter::on_post_sim);
	listen_message ("StatMeterOn", &KDStatMeter::on_on);
	listen_message ("StatMeterOff", &KDStatMeter::on_off);
	listen_message ("PropertyChange", &KDStatMeter::on_property_change);
}



void
KDStatMeter::initialize ()
{
	KDHUDElement::initialize ();

	if (!enabled.exists ())
		enabled = Parameter<bool> (host (), "stat_meter", true);

	ObjectProperty::subscribe ("DesignNote", host ());
	update_text ();
	update_range ();
}

void
KDStatMeter::deinitialize ()
{
	KDHUDElement::deinitialize ();
	ObjectProperty::unsubscribe ("DesignNote", host ());
}



bool
KDStatMeter::prepare ()
{
	if (!enabled) return false;

	// Get the current value.
	QuestVar qvar (quest_var->data ());
	ObjectProperty property (prop_name->data (), prop_obj);
	if (qvar.exists ())
		value = qvar;
	else if (property.exists ())
	{
		bool have_value = false;

		try
		{
			value = property.get_field<int> (prop_field);
			have_value = true;
		}
		catch (...) {} // not an int

		if (!have_value)
		try
		{
			value = property.get_field<float> (prop_field);
			have_value = true;
		}
		catch (...) {} // not a float

		if (!have_value && prop_comp != Vector::Component::NONE)
		try
		{
			value = property.get_field<Vector> (prop_field)
				[prop_comp];
			have_value = true;
		}
		catch (...) {} // not a vector

		if (!have_value) return false;
	}
	else
		return false;

	// Clamp the value and calculate derived versions.
	value = std::min (max, std::max (min, value));
	value_pct = (max != min) ? (value - min) / (max - min) : 0.0f;
	value_int = std::lround (value);
	if (value_pct * 100.0f <= low)
		value_tier = Tier::LOW;
	else if (value_pct * 100.0f >= high)
		value_tier = Tier::HIGH;
	else
		value_tier = Tier::MEDIUM;

	// Get the sizes of the canvas and the text.
	CanvasSize canvas = Engine::get_canvas_size (),
		text_size = get_text_size (text);

	// Calculate the size of the meter.
	CanvasSize request_size = get_request_size (),
		meter_size = request_size;
	if (style == Style::UNITS)
	{
		if (orient == Orient::HORIZ)
			meter_size.w = value_int * request_size.w +
				(value_int - 1) * spacing;
		else // Orient::VERT
			meter_size.h = value_int * request_size.h +
				(value_int - 1) * spacing;
	}
	else if (orient == Orient::VERT)
	{
		meter_size.w = request_size.h;
		meter_size.h = request_size.w;
	}

	// Calculate the size of the element.
	CanvasSize elem_size;
	bool show_text;
	if (orient == Orient::HORIZ && !text.empty ())
	{
		show_text = true;
		elem_size.w = std::max (meter_size.w, text_size.w);
		elem_size.h = meter_size.h + spacing + text_size.h;
	}
	else // Orient::VERT and/or empty text
	{
		show_text = false;
		elem_size = meter_size;
	}

	// Calculate the element position and relative positions of meter/text.
	CanvasPoint elem_pos;
	switch (position)
	{
	case Position::NW: case Position::WEST: case Position::SW: default:
		elem_pos.x = MARGIN;
		meter_pos.x = text_pos.x = 0;
		break;
	case Position::NORTH: case Position::CENTER: case Position::SOUTH:
		elem_pos.x = (canvas.w - elem_size.w) / 2;
		meter_pos.x = std::max (0, (text_size.w - meter_size.w) / 2);
		text_pos.x = std::max (0, (meter_size.w - text_size.w) / 2);
		break;
	case Position::NE: case Position::EAST: case Position::SE:
		elem_pos.x = canvas.w - MARGIN - elem_size.w;
		meter_pos.x = std::max (0, text_size.w - meter_size.w);
		text_pos.x = std::max (0, meter_size.w - text_size.w);
		break;
	}
	switch (position)
	{
	case Position::NW: case Position::NORTH: case Position::NE: default:
		elem_pos.y = MARGIN;
		meter_pos.y = 0;
		text_pos.y = meter_size.h + spacing; // text below
		break;
	case Position::WEST: case Position::CENTER: case Position::EAST:
		elem_pos.y = (canvas.h - elem_size.h) / 2;
		meter_pos.y = 0;
		text_pos.y = meter_size.h + spacing; // text below
		break;
	case Position::SW: case Position::SOUTH: case Position::SE:
		elem_pos.y = canvas.h - MARGIN - elem_size.h;
		meter_pos.y = show_text ? (text_size.h + spacing) : 0;
		text_pos.y = 0; // text above
		break;
	}

	set_position (elem_pos + CanvasPoint (offset_x, offset_y));
	set_size (elem_size);
	// If this script ever becomes an overlay, it won't be able to redraw
	// every frame. It would need to subscribe to the qvar/property.
	schedule_redraw ();
	return true;
}

void
KDStatMeter::redraw ()
{
	// Calculate value-sensitive colors.
	Color tier_color, color_blend = (value_pct < 0.5f)
		? Thief::interpolate (color_low, color_med, value_pct * 2.0f)
		: Thief::interpolate (color_med, color_high,
			(value_pct - 0.5f) * 2.0f);
	switch (value_tier)
	{
	case Tier::LOW: tier_color = color_low; break;
	case Tier::HIGH: tier_color = color_high; break;
	case Tier::MEDIUM: default: tier_color = color_med; break;
	}

	// Draw the text, if any.
	if (orient == Orient::HORIZ && !text.empty ())
	{
		set_drawing_color
			((style == Style::GEM) ? color_blend : tier_color);
		draw_text_shadowed (text, text_pos);
		set_drawing_offset (meter_pos);
	}

	CanvasSize request_size = get_request_size ();

	// Draw a progress bar meter.
	if (style == Style::PROGRESS)
	{
		CanvasRect bar_area (request_size);
		if (orient == Orient::VERT) // invert dimensions
		{
			bar_area.w = request_size.h;
			bar_area.h = request_size.w;
		}

		set_drawing_color (color_bg);
		fill_area (bar_area);

		set_drawing_color (tier_color);
		draw_box (bar_area);

		if (orient == Orient::HORIZ)
		{
			bar_area.w = std::lround (bar_area.w * value_pct);
			if (position == Position::NE ||
			    position == Position::EAST ||
			    position == Position::SE) // right to left
				bar_area.x = request_size.w - bar_area.w;
		}
		else // Orient::VERT
		{
			bar_area.h = std::lround (bar_area.h * value_pct);
			if (position != Position::NW &&
			    position != Position::NORTH &&
			    position != Position::NE) // bottom to top
				bar_area.y = request_size.w - bar_area.h;
		}
		fill_area (bar_area);
	}

	// Draw a meter with individual units.
	else if (style == Style::UNITS)
	{
		set_drawing_color (tier_color);
		CanvasPoint unit;
		for (int i = 1; i <= value_int; ++i)
		{
			if (image->bitmap)
				draw_bitmap
					(image->bitmap, HUDBitmap::STATIC, unit);
			else
				draw_symbol ((image->symbol != Symbol::NONE)
					? image->symbol : Symbol::SQUARE,
					request_size, unit);

			if (orient == Orient::HORIZ)
				unit.x += request_size.w + spacing;
			else // Orient::VERT
				unit.y += request_size.h + spacing;
		}
	}

	// Draw a solid gem meter.
	else if (style == Style::GEM)
	{
		if (image->bitmap)
			draw_bitmap (image->bitmap, std::lround (value_pct *
				(image->bitmap->count_frames () - 1)));
		else
		{
			CanvasRect gem_area (request_size);
			if (orient == Orient::VERT)
			{
				gem_area.w = request_size.h;
				gem_area.h = request_size.w;
			}
			set_drawing_color (color_blend);
			fill_area (gem_area);
			set_drawing_color (color_bg);
			draw_box (gem_area);
		}
	}

	set_drawing_offset ();
}



CanvasSize
KDStatMeter::get_request_size () const
{
	CanvasSize request_size (128, 32);
	if (image->bitmap)
		request_size = image->bitmap->get_size ();
	else
	{
		if (style == Style::UNITS) request_size.w = 32;
		if (request_w > 0) request_size.w = request_w;
		if (request_h > 0) request_size.h = request_h;
	}
	return request_size;
}



Message::Result
KDStatMeter::on_post_sim (Message&)
{
	// Update anything based on an object that may have been absent.

	schedule_redraw ();
	prop_obj.reparse ();
	update_text ();
	update_range ();

	// Issue any one-time warnings on configuration.

	if (style == Style::PROGRESS && image->bitmap)
		log (Log::WARNING, "Bitmap image \"%1%\" will be ignored for a "
			"progress-style meter.", image.get_raw ());

	if (style != Style::UNITS && image->symbol != Symbol::NONE)
		log (Log::WARNING, "Symbol \"%1%\" will be ignored for a "
			"non-units-style meter.", image.get_raw ());

	if (!quest_var->empty () && !prop_name->empty ())
		log (Log::WARNING, "Both a quest variable and a property were "
			"specified; will use the quest variable and ignore the "
			"property.");

	if (quest_var->empty () && prop_name->empty ())
		log (Log::WARNING, "Neither a quest variable nor a property "
			"was specified; the stat meter will not be displayed.");

	if (min > max)
		log (Log::WARNING, "Minimum value %1% is greater than maximum "
			"value %2%.", min, max);

	if (low < 0 || low > 100)
		log (Log::WARNING, "Low bracket %1%%% is outside the range "
			"[0,100].", low);

	if (high < 0 || high > 100)
		log (Log::WARNING, "High bracket %1%%% is outside the range "
			"[0,100].", high);

	if (low >= high)
		log (Log::WARNING, "Low bracket %1%%% is greater than or equal "
			"to high bracket %2%%%.", low, high);

	return Message::HALT;
}



Message::Result
KDStatMeter::on_on (Message&)
{
	enabled = true;
	return Message::HALT;
}


Message::Result
KDStatMeter::on_off (Message&)
{
	enabled = false;
	return Message::HALT;
}



Message::Result
KDStatMeter::on_property_change (PropertyMessage& message)
{
	if (message.property == Property ("DesignNote"))
	{
		// Too many to check, so just assume the meter is affected.
		schedule_redraw ();

		update_text ();
		update_range ();
	}
	return Message::HALT;
}

void
KDStatMeter::update_text ()
{
	text.clear ();

	if (_text->empty () || _text == "@none")
		{}

	else if (_text->front () != '@')
		text = Mission::get_text ("strings", "hud", _text);

	else if (_text == "@name")
	{
		if (prop_obj != Object::NONE)
			text = prop_obj->get_display_name ();
		else
			text = Mission::get_text ("strings", "hud", quest_var);
	}

	else if (_text == "@description")
	{
		if (prop_obj != Object::NONE)
			text = prop_obj->get_description ();
		else
			log (Log::WARNING, "\"@description\" is not a valid "
				"stat meter text source for a quest variable "
				"statistic.");
	}

	else
		log (Log::WARNING, "\"%1%\" is not a valid stat meter text "
			"source.", _text);
}

void
KDStatMeter::update_range ()
{
	min = _min;
	max = _max;

	// Provide special min/max defaults if neither is set.
	if (!_min.exists () && !_max.exists () && quest_var->empty ())
	{
		if (prop_name == "AI_Visibility" && prop_field == "Level")
		{
			min = 0.0f;
			max = 100.0f;
		}
		else if (prop_name == "HitPoints")
		{
			Being being (prop_obj->number);
			if (being.max_hit_points.exists ())
			{
				min = 0.0f;
				max = being.max_hit_points;
			}
		}
	}
}

