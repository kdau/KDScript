/******************************************************************************
 *  KDToolSight.cc
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

#include "KDToolSight.hh"

const HUD::ZIndex
KDToolSight::PRIORITY = 30;

const CanvasSize
KDToolSight::SYMBOL_SIZE = { 24, 24 };

KDToolSight::KDToolSight (const String& _name, const Object& _host)
	: KDHUDElement (_name, _host, PRIORITY),
	  selected (false),
	  PARAMETER_ (deselect_on_use, "tool_sight_deselect", false),
	  PARAMETER_ (image, "tool_sight_image",
	  	Symbol::CROSSHAIRS, false, false),
	  PARAMETER_ (color, "tool_sight_color", Color (128, 128, 128)),
	  PARAMETER_ (offset_x, "tool_sight_offset_x", 0),
	  PARAMETER_ (offset_y, "tool_sight_offset_y", 0)
{
	listen_message ("InvSelect", &KDToolSight::on_inv_select);
	listen_message ("InvFocus", &KDToolSight::on_inv_select);
	listen_message ("InvDeSelect", &KDToolSight::on_inv_deselect);
	listen_message ("InvDeFocus", &KDToolSight::on_inv_deselect);
	listen_message ("FrobInvEnd", &KDToolSight::on_frob_inv_end);
	listen_message ("PropertyChange", &KDToolSight::on_property_change);
}

void
KDToolSight::initialize ()
{
	KDHUDElement::initialize ();
	ObjectProperty::subscribe ("DesignNote", host ());
}

bool
KDToolSight::prepare ()
{
	if (!selected) return false;

	// Get canvas, image, and text sizes and calculate the element size.
	CanvasSize canvas = Engine::get_canvas_size (),
		elem_size = image->bitmap
			? image->bitmap->get_size () : SYMBOL_SIZE;

	// Calculate the center of the canvas and the position of the element.
	CanvasPoint canvas_center (canvas.w / 2, canvas.h / 2),
		elem_pos (canvas_center.x - elem_size.w / 2 + offset_x,
			canvas_center.y - elem_size.h / 2 + offset_y);

	set_position (elem_pos);
	set_size (elem_size);
	return true;
}

void
KDToolSight::redraw ()
{
	set_drawing_color (color);
	if (image->bitmap)
		draw_bitmap (image->bitmap, HUDBitmap::STATIC);
	else if (image->symbol != Symbol::NONE)
		draw_symbol (image->symbol, SYMBOL_SIZE);
}

Message::Result
KDToolSight::on_inv_select (Message&)
{
	selected = true;
	return Message::CONTINUE;
}

Message::Result
KDToolSight::on_inv_deselect (Message&)
{
	selected = false;
	return Message::CONTINUE;
}

Message::Result
KDToolSight::on_frob_inv_end (FrobMessage&)
{
	if (deselect_on_use)
		Player ().clear_item ();
	return Message::CONTINUE;
}

Message::Result
KDToolSight::on_property_change (PropertyChangeMessage& message)
{
	if (message.get_property () == Property ("DesignNote"))
		schedule_redraw ();
	return Message::CONTINUE;
}

