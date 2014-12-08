/******************************************************************************
 *  KDTrapShowImage.cc
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

#include "KDTrapShowImage.hh"

const HUDElement::ZIndex
KDTrapShowImage::PRIORITY = 50;

KDTrapShowImage::KDTrapShowImage (const String& _name, const Object& _host)
	: TrapTrigger (_name, _host, THIEF_DEFAULT_LOG_LEVEL, true),
	  KDHUDElement (PRIORITY),
	  THIEF_PERSISTENT_FULL (enabled, false),
	  THIEF_PARAMETER (image, Symbol::NONE, false, false, false),
	  THIEF_PARAMETER_FULL (use_hud, "image_use_hud", true),
	  THIEF_PARAMETER_FULL (position, "image_position", Position::CENTER),
	  THIEF_PARAMETER_FULL (offset_x, "image_offset_x", 0),
	  THIEF_PARAMETER_FULL (offset_y, "image_offset_y", 0)
{
	listen_message ("PropertyChange", &KDTrapShowImage::on_property_change);
}

void
KDTrapShowImage::initialize ()
{
	Script::initialize ();
	KDHUDElement::initialize ();
	ObjectProperty::subscribe ("DesignNote", host ());
}

void
KDTrapShowImage::deinitialize ()
{
	ObjectProperty::unsubscribe ("DesignNote", host ());
	KDHUDElement::deinitialize ();
	Script::deinitialize ();
}

bool
KDTrapShowImage::prepare ()
{
	if (!enabled || !use_hud || !image->bitmap) return false;
	CanvasSize size = image->bitmap->get_size ();
	set_position (calculate_position (position, size,
		CanvasPoint (offset_x, offset_y)));
	set_size (size);
	return true;
}

void
KDTrapShowImage::redraw ()
{
	draw_bitmap (image->bitmap);
}

Message::Result
KDTrapShowImage::on_trap (bool on, Message&)
{
	enabled = on;
	if (!use_hud)
		Interface::show_image (image.get_raw ());
	return Message::CONTINUE;
}

Message::Result
KDTrapShowImage::on_property_change (PropertyMessage& message)
{
	if (message.property == Property ("DesignNote"))
		schedule_redraw ();
	return Message::HALT;
}

