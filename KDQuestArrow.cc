/******************************************************************************
 *  KDQuestArrow.cc
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

#include "KDQuestArrow.hh"



const HUDElement::ZIndex
KDQuestArrow::PRIORITY = 10;

const CanvasSize
KDQuestArrow::SYMBOL_SIZE = { 32, 32 };

const int
KDQuestArrow::PADDING = 8;



KDQuestArrow::KDQuestArrow (const String& _name, const Object& _host)
	: KDHUDElement (_name, _host, PRIORITY),
	  THIEF_PERSISTENT (enabled),
	  THIEF_PARAMETER_FULL (objective, "quest_arrow_goal"),
	  THIEF_PERSISTENT_FULL (old_objective, Objective::NONE),
	  THIEF_PARAMETER_FULL (range, "quest_arrow_range", 0.0f),
	  THIEF_PARAMETER_FULL (obscured, "quest_arrow_obscured", false),
	  THIEF_PARAMETER_FULL (image, "quest_arrow_image",
		Symbol::ARROW, false, true),
	  THIEF_PARAMETER_FULL (_text, "quest_arrow_text", "@name"),
	  THIEF_PARAMETER_FULL (color, "quest_arrow_color", Color (0xffffff)),
	  THIEF_PARAMETER_FULL (shadow, "quest_arrow_shadow", true),
	  direction (Direction::NONE),
	  image_pos (),
	  text_pos ()
{
	listen_message ("QuestArrowOn", &KDQuestArrow::on_on);
	listen_message ("QuestArrowOff", &KDQuestArrow::on_off);
	listen_message ("Contained", &KDQuestArrow::on_contained);
	listen_message ("Slain", &KDQuestArrow::on_off);
	listen_message ("AIModeChange", &KDQuestArrow::on_ai_mode_change);

	listen_message ("PropertyChange", &KDQuestArrow::on_property_change);
	listen_message ("QuestChange", &KDQuestArrow::on_quest_change);
}



void
KDQuestArrow::initialize ()
{
	KDHUDElement::initialize ();

	if (!enabled.exists ())
		enabled = Parameter<bool> (host (), "quest_arrow",
			!Player ().has_touched (host ()));

	ObjectProperty::subscribe ("DesignNote", host ());
	update_objective ();
	update_text ();

	// for quest_arrow_text == "@name"
	ObjectProperty::subscribe ("GameName", host ());
}

bool
KDQuestArrow::prepare ()
{
	if (!enabled) return false;

	// Confirm that the object is within range, if required.
	if (range != 0.0f && host ().get_location ().distance
			(Player ().get_location ()) > range)
		return false;

	// Confirm that the object is actually visible, if required.
	if (!obscured && !Engine::rendered_this_frame (host ()))
		return false;

	// Get the canvas, image, and text size and calculate the element size.
	CanvasSize canvas = Engine::get_canvas_size (),
		image_size = image->bitmap
			? image->bitmap->get_size () : SYMBOL_SIZE,
		text_size = get_text_size (text),
		elem_size;
	elem_size.w = image_size.w + PADDING + text_size.w;
	elem_size.h = std::max (image_size.h, text_size.h);

	// Get the object's position in canvas coordinates.
	CanvasPoint obj_pos = centroid_to_canvas (host ());
	if (!obj_pos.valid ()) return false;

	// Choose the alignment of image and text.
	direction = (obj_pos.x > canvas.w / 2)
		? Direction::RIGHT // text on the left, image on the right
		: Direction::LEFT; // text on the right, image on the left

	// Calculate the absolute position of the image.
	CanvasPoint image_hotspot = image->bitmap
		? CanvasPoint (image_size.w / 2, image_size.h / 2)
		: get_symbol_hotspot (image->symbol, SYMBOL_SIZE, direction);
	CanvasPoint image_apos = obj_pos - image_hotspot;

	// Calculate the absolute position of the text.
	CanvasPoint text_apos = {
		(direction == Direction::RIGHT)
			? (image_apos.x - PADDING - text_size.w)
			: (image_apos.x + image_size.w + PADDING),
		obj_pos.y - text_size.h / 2 };

	// Calculate the element position.
	CanvasPoint elem_pos = { std::min (image_apos.x, text_apos.x),
		std::min (image_apos.y, text_apos.y) };

	// Update the relative position of the image, if needed.
	if (image_pos != image_apos - elem_pos)
	{
		image_pos = image_apos - elem_pos;
		schedule_redraw ();
	}

	// Update the relative position of the text, if needed.
	if (text_pos != text_apos - elem_pos)
	{
		text_pos = text_apos - elem_pos;
		schedule_redraw ();
	}

	set_position (elem_pos);
	set_size (elem_size);

	return true;
}

void
KDQuestArrow::redraw ()
{
	set_drawing_color (color);

	if (image->bitmap)
		draw_bitmap (image->bitmap, HUDBitmap::STATIC, image_pos);
	else if (image->symbol != Symbol::NONE)
		draw_symbol (image->symbol, SYMBOL_SIZE, image_pos,
			direction, shadow);

	if (shadow)
		draw_text_shadowed (text, text_pos);
	else
		draw_text (text, text_pos);
}



Message::Result
KDQuestArrow::on_on (Message&)
{
	enabled = true;
	return Message::HALT;
}


Message::Result
KDQuestArrow::on_off (Message&)
{
	enabled = false;
	return Message::HALT;
}

Message::Result
KDQuestArrow::on_contained (ContainmentMessage& message)
{
	if (message.event == ContainmentMessage::ADD &&
	    message.container == Player ())
		enabled = false;
	return Message::HALT;
}

Message::Result
KDQuestArrow::on_ai_mode_change (AIModeMessage& message)
{
	if (message.new_mode == AI::Mode::DEAD)
		enabled = false;
	return Message::HALT;
}



Message::Result
KDQuestArrow::on_property_change (PropertyMessage& message)
{
	if (message.property == Property ("DesignNote"))
	{
		schedule_redraw ();
		update_objective ();
		update_text ();
	}
	else if (message.property == Property ("GameName"))
	{
		schedule_redraw ();
		update_text ();
	}
	return Message::HALT;
}

Message::Result
KDQuestArrow::on_quest_change (QuestMessage&)
{
	if (objective->number != Objective::NONE)
		enabled = (objective->visible &&
			objective->state == Objective::State::INCOMPLETE);
	return Message::HALT;
}



void
KDQuestArrow::update_objective ()
{
	// Don't update if the objective has not changed.
	if (objective->number == old_objective) return;

	// Unsubscribe from any old objective.
	if (old_objective != Objective::NONE)
	{
		Objective old (old_objective);
		old.state.unsubscribe (host ());
		old.visible.unsubscribe (host ());
	}

	// Identify the new objective, if any.
	old_objective = objective->number;
	if (objective->number == Objective::NONE) return;

	// Update the enabled state.
	enabled = (objective->visible &&
		objective->state == Objective::State::INCOMPLETE);

	// Subscribe to the new objective.
	objective->state.subscribe (host ());
	objective->visible.subscribe (host ());

	// In case text == "@objective", update it.
	update_text ();
}

void
KDQuestArrow::update_text ()
{
	text.clear ();

	if (_text->empty () || _text == "@none")
		{}

	else if (_text->front () != '@')
		text = Mission::get_text ("strings", "hud", _text);

	else if (_text == "@name")
		text = host ().get_display_name ();

	else if (_text == "@description")
		text = host ().get_description ();

	else if (_text == "@objective")
#ifdef IS_THIEF2
	{
		if (Engine::is_editor ())
			log (Log::INFO, "The \"@objective\" value for "
				"quest_arrow_text is not available in DromEd. "
				"Test this arrow in the real game.");
		else if (objective->number != Objective::NONE)
		{
			String dir = "intrface\\miss" +
				std::to_string (Mission::get_number ());
			String msgid = "text_" +
				std::to_string (objective->number);
			text = Mission::get_text (dir, "goals", msgid);
		}
	}
#else // !IS_THIEF2
		log (Log::WARNING, "quest_arrow_text cannot be \"@objective\" "
			"in this game. No text will be shown.");
#endif // IS_THIEF2

	else
		log (Log::WARNING, "\"%1\" is not a valid quest arrow text "
			"source.", _text);
}

