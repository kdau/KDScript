/******************************************************************************
 *  KDSubtitled.cc: HUDSubtitle, KDSubtitled, KDSubtitledAI, KDSubtitledVO
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

#include "KDSubtitled.hh"



// HUDSubtitle

const HUD::ZIndex
HUDSubtitle::PRIORITY = 20;

const int
HUDSubtitle::BORDER = 1;

const int
HUDSubtitle::PADDING = 8;

const Color
HUDSubtitle::BACKGROUND_COLOR (0, 0, 0);

HUDSubtitle::HUDSubtitle (const Object& _speaker, const Object& _schema,
		const String& _text, Color _color)
	: HUDElement (),
	  speaker (_speaker), schema (_schema), text (_text), color (_color)
{
	initialize (PRIORITY);
}

HUDSubtitle::~HUDSubtitle ()
{}

bool
HUDSubtitle::prepare ()
{
	// Get the canvas and text size and calculate the element size.
	CanvasSize canvas = Engine::get_canvas_size (),
		text_size = get_text_size (text),
		elem_size;
	elem_size.w = BORDER + PADDING + text_size.w + PADDING + BORDER;
	elem_size.h = BORDER + PADDING + text_size.h + PADDING + BORDER;

	// Get the speaker's position in canvas coordinates.
	CanvasPoint speaker_pos;
	if (speaker == Player ())
		speaker_pos = CanvasPoint (canvas.w / 2, canvas.h / 2);
	else
	{
		speaker_pos = centroid_to_canvas (speaker);
		if (!speaker_pos.valid ()) return false;
	}

	// Calculate the element position.
	CanvasPoint elem_pos;
	elem_pos.x = std::max (0, std::min (canvas.w - elem_size.w,
		speaker_pos.x - elem_size.w / 2));
	elem_pos.y = std::max (0, std::min (canvas.h - elem_size.h,
		speaker_pos.y - PADDING)); // slightly above center

	set_position (elem_pos);
	set_size (elem_size);
	return true;
}

void
HUDSubtitle::redraw ()
{
	// draw background
	set_drawing_color (BACKGROUND_COLOR);
	fill_area ();

	// draw border
	set_drawing_color (color);
	draw_box ();

	// draw text
	draw_text (text, CanvasPoint (BORDER+PADDING, BORDER+PADDING));
}



// KDSubtitled

const float
KDSubtitled::EARSHOT = 80.0f;

const Color
KDSubtitled::DEFAULT_COLOR (255, 255, 255);

KDSubtitled::KDSubtitled (const String& _name, const Object& _host)
	: Script (_name, _host), element (nullptr)
{
	listen_message ("Subtitle", &KDSubtitled::on_subtitle);
	listen_timer ("FinishSubtitle", &KDSubtitled::on_finish_subtitle);
	listen_message ("EndScript", &KDSubtitled::on_end_script);
}

KDSubtitled::~KDSubtitled ()
{
	finish_subtitle ();
}

bool
KDSubtitled::start_subtitle (const Object& speaker, const Object& schema)
{
	// Confirm speaker and schema objects are valid.
	if (!speaker.exists () || !schema.exists () ||
	    !schema.inherits_from (Object ("Schema")))
	{
		mono () << "Warning: Can't subtitle invalid speaker/schema pair "
			<< speaker.number << "/" << schema.number << "."
			<< std::endl;
		return false;
	}

	// Get subtitle text.
	String text =
		Mission::get_text ("strings", "subtitles", schema.get_name ());
	if (text.empty ())
		return false;

	// Stop any previous subtitle on this object.
	finish_subtitle ();

	// Get or calculate the schema duration.
	Time duration = ScriptHost (schema).script_timing.exists ()
		? ScriptHost (schema).script_timing
		: Mission::calc_text_duration (text, 700ul);

	// Get subtitle color.
	Parameter<Color> schema_color (schema, "subtitle_color", DEFAULT_COLOR),
		speaker_color (speaker, "subtitle_color", DEFAULT_COLOR);
	Color color = schema_color.exists () ? schema_color : speaker_color;

	try
	{
		// Create a HUD subtitle element, if desired.
		if (QuestVar ("subtitles_use_hud").get ())
			element = new HUDSubtitle
				(speaker, schema, text, color);
	}
	catch (...)
	{
		element = nullptr;
	}

	if (element)
		// Schedule its destruction.
		start_timer ("FinishSubtitle", duration, false, schema);
	else
		// Go the old-fashioned way.
		Mission::show_text (text, duration, color);

	return true;
}

void
KDSubtitled::finish_subtitle (const Object& schema)
{
	// This only applies to HUD elements.
	if (!element) return;

	// Try to prevent an early finish of a later subtitle.
	if (schema != Object::ANY && element->get_schema () != schema) return;

	// Destroy the HUD element.
	delete element;
	element = nullptr;
}

Message::Result
KDSubtitled::on_subtitle (GenericMessage& message)
{
	Object schema = message.has_data (Message::DATA1)
		? message.get_data<Object> (Message::DATA1)
		: Object::NONE;
	Object speaker = message.has_data (Message::DATA2)
		? message.get_data<Object> (Message::DATA2)
		: message.get_from ();
	start_subtitle (speaker, schema);
	return Message::HALT;
}

Message::Result
KDSubtitled::on_finish_subtitle (TimerMessage& message)
{
	finish_subtitle (message.get_data<Object> (Message::DATA1));
	return Message::HALT;
}

Message::Result
KDSubtitled::on_end_script (GenericMessage&)
{
	finish_subtitle ();
	return Message::CONTINUE;
}



// KDSubtitledAI

KDSubtitledAI::KDSubtitledAI (const String& _name, const Object& _host)
	: KDSubtitled (_name, _host)
{
	listen_message ("PropertyChange", &KDSubtitledAI::on_property_change);
}

void
KDSubtitledAI::initialize ()
{
	Property (host (), "Speech").subscribe (Object::SELF);
}

Message::Result
KDSubtitledAI::on_property_change (PropertyChangeMessage& message)
{
	// Confirm that the relevant property has changed.
	if (message.get_object () != host () ||
	    message.get_prop_name () != "Speech")
		return Message::CONTINUE;

	// Confirm that the speech schema is valid.
	Object schema = host_as<AI> ().last_speech_schema;
	if (!schema.exists ()) return Message::HALT;

	// If this is the end of a speech schema, finish the subtitle instead.
	if (!host_as<AI> ().is_speaking)
	{
		finish_subtitle (schema);
		return Message::CONTINUE;
	}

	// Confirm that the speech is in the player's (estimated) earshot.
	Vector host_pos = host ().get_location (),
		player_pos = Player ().get_location ();
	if (host_pos.distance (player_pos) >= EARSHOT)
		return Message::HALT;

	// Display the subtitle.
	start_subtitle (host (), schema);
	return Message::CONTINUE;
}



// KDSubtitledVO

KDSubtitledVO::KDSubtitledVO (const String& _name, const Object& _host)
	: KDSubtitled (_name, _host)
{
	// This script lacks trap behavior because VOSounds isn't a StdTrap.
	listen_message ("TurnOn", &KDSubtitledVO::on_turn_on);
	listen_timer ("InitialDelay", &KDSubtitledVO::on_initial_delay);
}

Message::Result
KDSubtitledVO::on_turn_on (GenericMessage&)
{
	// Identify the schema.
	SoundSchema schema =
		Link::get_one ("SoundDescription", host ()).get_dest ();
	if (schema == Object::NONE)
		return Message::HALT;

	// If the schema has the "Play Once" flag, only display the subtitle if
	// the schema hasn't been played already. At this point, the schema has
	// probably already been started, so it will always appear to have been
	// played already. Instead, the workaround used here is to track the
	// status on an unrelated property, StimKO, which would never appear on
	// a sound schema otherwise. Note that this method does not recognize
	// if any non-KDSubtitledVO object has played the schema.
	if (schema.play_once)
	{
		Property played (schema, "StimKO");
		if (played.exists ())
			return Message::HALT;
		else
			played.set (true);
	}

	// Wait until the initial delay is over before displaying the subtitle.
	start_timer ("InitialDelay", schema.initial_delay, false, schema);
	return Message::CONTINUE;
}

Message::Result
KDSubtitledVO::on_initial_delay (TimerMessage& message)
{
	// Display the subtitle.
	start_subtitle (Player (), message.get_data<Object> (Message::DATA1));
	return Message::CONTINUE;
}

