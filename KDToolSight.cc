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

const HUDElement::ZIndex
KDToolSight::PRIORITY = 30;

const CanvasSize
KDToolSight::SYMBOL_SIZE = { 24, 24 };

KDToolSight::KDToolSight (const String& _name, const Object& _host)
	: Script (_name, _host),
	  KDHUDElement (PRIORITY),
	  selected (false),
	  THIEF_PARAMETER_FULL (when_remote, "tool_sight_when_remote", false),
	  THIEF_PARAMETER_FULL (deselect_on_use, "tool_sight_deselect", false),
	  THIEF_PARAMETER_FULL (image, "tool_sight_image",
	  	Symbol::CROSSHAIRS, false, false),
	  THIEF_PARAMETER_FULL (color, "tool_sight_color", Color (0x808080)),
	  THIEF_PARAMETER_FULL (offset_x, "tool_sight_offset_x", 0),
	  THIEF_PARAMETER_FULL (offset_y, "tool_sight_offset_y", 0)
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
	Script::initialize ();
	KDHUDElement::initialize ();
	ObjectProperty::subscribe ("DesignNote", host ());
}

void
KDToolSight::deinitialize ()
{
	ObjectProperty::unsubscribe ("DesignNote", host ());
	KDHUDElement::deinitialize ();
	Script::deinitialize ();
}

bool
KDToolSight::prepare ()
{
	if (!selected) return false;
	try { if (!when_remote && Camera::is_remote ()) return false; }
	catch (...) {} // probably because of pre-1.22 NewDark; proceed anyway
	auto size = image->bitmap ? image->bitmap->get_size () : SYMBOL_SIZE;
	set_position (calculate_position (Position::CENTER, size,
		CanvasPoint (offset_x, offset_y)));
	set_size (size);
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
	return Message::HALT;
}

Message::Result
KDToolSight::on_inv_deselect (Message&)
{
	selected = false;
	return Message::HALT;
}

Message::Result
KDToolSight::on_frob_inv_end (FrobMessage&)
{
	if (deselect_on_use)
		Player ().clear_item ();
	return Message::HALT;
}

Message::Result
KDToolSight::on_property_change (PropertyMessage& message)
{
	if (message.property == Property ("DesignNote"))
		schedule_redraw ();
	return Message::HALT;
}

