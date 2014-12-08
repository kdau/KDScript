/******************************************************************************
 *  KDHUDElement.cc: HUDElement, KDHUDElement
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

#include "KDHUDElement.hh"



namespace Thief {

THIEF_ENUM_CODING (KDHUDElement::Position, CODE, CODE,
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

THIEF_ENUM_CODING (KDHUDElement::Symbol, CODE, CODE,
	THIEF_ENUM_VALUE (NONE, "@none"),
	THIEF_ENUM_VALUE (ARROW, "@arrow"),
	THIEF_ENUM_VALUE (CROSSHAIRS, "@crosshairs"),
	THIEF_ENUM_VALUE (RETICULE, "@reticule", "@reticle"),
	THIEF_ENUM_VALUE (SQUARE, "@square"),
)

} // namespace Thief



KDHUDElement::KDHUDElement (const String& _name, const Object& _host,
		ZIndex _priority)
	: Script (_name, _host), priority (_priority)
{}

void
KDHUDElement::initialize ()
{
	Script::initialize ();
	HUDElement::initialize (priority);
}

void
KDHUDElement::deinitialize ()
{
	Script::deinitialize ();
	HUDElement::deinitialize ();
}


const int
KDHUDElement::MARGIN = 16;

CanvasPoint
KDHUDElement::calculate_position (Position type, const CanvasSize& element,
	const CanvasPoint& offset, int margin)
{
	CanvasSize canvas = Engine::get_canvas_size ();
	CanvasPoint result;

	// Calculate the X coordinate.
	switch (type)
	{
	case Position::NW: case Position::WEST: case Position::SW: default:
		result.x = margin;
		break;
	case Position::NORTH: case Position::CENTER: case Position::SOUTH:
		result.x = (canvas.w - element.w) / 2;
		break;
	case Position::NE: case Position::EAST: case Position::SE:
		result.x = canvas.w - margin - element.w;
		break;
	}

	// Calculate the Y coordinate.
	switch (type)
	{
	case Position::NW: case Position::NORTH: case Position::NE: default:
		result.y = margin;
		break;
	case Position::WEST: case Position::CENTER: case Position::EAST:
		result.y = (canvas.h - element.h) / 2;
		break;
	case Position::SW: case Position::SOUTH: case Position::SE:
		result.y = canvas.h - margin - element.h;
		break;
	}

	return result + offset;
}



const Color
KDHUDElement::SHADOW_COLOR (0, 0, 0);

const CanvasPoint
KDHUDElement::SHADOW_OFFSET (1, 1);

void
KDHUDElement::draw_text_shadowed (const String& text, CanvasPoint position)
{
	Color real_color = get_drawing_color ();
	set_drawing_color (SHADOW_COLOR);
	draw_text (text, position + SHADOW_OFFSET);
	set_drawing_color (real_color);
	draw_text (text, position);
}



void
KDHUDElement::draw_symbol (Symbol symbol, CanvasSize size,
	CanvasPoint position, Direction direction, bool shadowed)
{
	if (shadowed)
	{
		Color real_color = get_drawing_color ();
		set_drawing_color (SHADOW_COLOR);
		draw_symbol (symbol, size, position + SHADOW_OFFSET, direction);
		set_drawing_color (real_color);
	}

	CanvasPoint xqtr (size.w / 4, 0), yqtr (0, size.h / 4);
	adjust_drawing_offset (position);

	switch (symbol)
	{
	case Symbol::ARROW:
		draw_line (yqtr*2, yqtr*2 + xqtr*4);
		switch (direction)
		{
		case Direction::LEFT:
			draw_line (yqtr*2, yqtr   + xqtr);
			draw_line (yqtr*2, yqtr*3 + xqtr);
			break;
		case Direction::RIGHT:
			draw_line (yqtr*2 + xqtr*4, yqtr   + xqtr*3);
			draw_line (yqtr*2 + xqtr*4, yqtr*3 + xqtr*3);
			break;
		case Direction::NONE:
		default:
			break; // no head on a directionless arrow
		}
		break;
	case Symbol::RETICULE:
		draw_line (xqtr   + yqtr,   xqtr*3 + yqtr);
		draw_line (xqtr   + yqtr,   xqtr   + yqtr*3);
		draw_line (xqtr*3 + yqtr,   xqtr*3 + yqtr*3);
		draw_line (xqtr   + yqtr*3, xqtr*3 + yqtr*3);
		// includes crosshairs
	case Symbol::CROSSHAIRS:
		draw_line (xqtr*2, xqtr*2 + yqtr*4);
		draw_line (yqtr*2, xqtr*4 + yqtr*2);
		break;
	case Symbol::SQUARE:
		fill_area (CanvasRect (size));
		break;
	case Symbol::NONE:
	default:
		break;
	}

	adjust_drawing_offset (-position);
}

CanvasPoint
KDHUDElement::get_symbol_hotspot (Symbol symbol, CanvasSize size,
	Direction direction) const
{
	switch (symbol)
	{
	case Symbol::ARROW:
		switch (direction)
		{
		case Direction::LEFT:
			return CanvasPoint (0, size.h / 2);
		case Direction::RIGHT:
			return CanvasPoint (size.w, size.h / 2);
		case Direction::NONE:
		default:
			break; // fall through to below (= center)
		}
	case Symbol::CROSSHAIRS:
	case Symbol::RETICULE:
	case Symbol::SQUARE:
		return CanvasPoint (size.w / 2, size.h / 2);
	case Symbol::NONE:
	default:
		return CanvasPoint::OFFSCREEN;
	}
}



// KDHUDElement::Image

KDHUDElement::Image::Image (Symbol _symbol, HUDBitmap::Ptr _bitmap)
	: symbol (_symbol), bitmap (_bitmap)
{}

namespace Thief {

template<>
bool
Parameter<KDHUDElement::Image>::decode (const String& raw) const
{
	if (raw.empty ())
		return false;

	else if (raw.front () == '@') // Interpret as a symbol name.
	{
		if (!config.symbolic)
			throw std::runtime_error ("A non-symbolic HUD element "
				"cannot have a symbolic image.");

		value.symbol = KDHUDElement::Symbol
			(EnumCoding::get<KDHUDElement::Symbol> ().decode (raw));
		value.bitmap = nullptr;

		if (value.symbol == KDHUDElement::Symbol::ARROW &&
		    !config.directional)
			throw std::runtime_error ("A non-directional HUD "
				"element cannot have an arrow symbol.");
	}

	else // Interpret as a path to a bitmap.
	{
		value.symbol = KDHUDElement::Symbol::NONE;
		value.bitmap = HUDBitmap::load (raw, config.animated);
		if (!value.bitmap)
			throw std::runtime_error ("Could not load bitmap.");
	}

	return true;
}

} // namespace Thief

