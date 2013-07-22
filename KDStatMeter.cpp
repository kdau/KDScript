/******************************************************************************
 *  KDStatMeter.cpp
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

#include "KDStatMeter.h"
#include <cmath>
#include <ScriptLib.h>
#include "utils.h"

const int
cScr_StatMeter::MARGIN = 16;

cScr_StatMeter::cScr_StatMeter (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (StatMeter, enabled, iHostObjId),
	  style (STYLE_PROGRESS), symbol (SYMBOL_NONE), spacing (0),
	  position (POS_NW), orient (ORIENT_HORIZ), prop_comp (COMP_NONE),
	  prop_object (iHostObjId), post_sim_fix (false),
	  min (0.0), max (0.0), low (0), high (0),
	  color_bg (0), color_low (0), color_med (0), color_high (0),
	  value (0.0), value_pct (0.0), value_tier (VALUE_MED)
{}

bool
cScr_StatMeter::Initialize ()
{
	if (!cScr_HUDElement::Initialize ()) return false;
	enabled.Init (GetParamBool ("stat_meter", true));
	OnPropertyChanged ("DesignNote"); // update all cached params
	SubscribeProperty ("DesignNote");
	return true;
}

bool
cScr_StatMeter::Prepare ()
{
	if (!enabled) return false;

	if (post_sim_fix)
		// re-resolve any target object that may have been absent
		UpdateObject ();

	// get the current value
	if (!qvar.IsEmpty ())
	{
		SService<IQuestSrv> pQS (g_pScriptManager);
		value = pQS->Get (qvar);
	}
	else if (!prop_name.IsEmpty () && prop_object)
	{
		SService<IPropertySrv> pPS (g_pScriptManager);
		if (!pPS->Possessed (prop_object, prop_name)) return false;

		cMultiParm value_mp;
		pPS->Get (value_mp, prop_object, prop_name,
			prop_field.IsEmpty () ? NULL : (const char*) prop_field);

		switch (value_mp.type)
		{
		case kMT_Int: value = int (value_mp); break;
		case kMT_Float: value = float (value_mp); break;
		case kMT_Vector:
			switch (prop_comp)
			{
			case COMP_X: value = cScrVec (value_mp).x; break;
			case COMP_Y: value = cScrVec (value_mp).y; break;
			case COMP_Z: value = cScrVec (value_mp).z; break;
			case COMP_NONE: default: return false;
			}
			break;
		default: return false;
		}
	}
	else
		return false;

	// clamp value and calculate derived versions
	value = std::min (max, std::max (min, value));
	value_pct = (max != min) ? (value - min) / (max - min) : 0.0;
	value_int = std::lround (value);
	if (value_pct * 100.0 <= low)
		value_tier = VALUE_LOW;
	else if (value_pct * 100.0 >= high)
		value_tier = VALUE_HIGH;
	else
		value_tier = VALUE_MED;

	// get canvas and text sizes
	CanvasSize canvas = GetCanvasSize (),
		text_size = GetTextSize (text);

	// calculate meter size
	CanvasSize meter_size = request_size;
	if (style == STYLE_UNITS)
	{
		if (orient == ORIENT_HORIZ)
			meter_size.w = value_int * request_size.w +
				(value_int - 1) * spacing;
		else // ORIENT_VERT
			meter_size.h = value_int * request_size.h +
				(value_int - 1) * spacing;
	}
	else if (orient == ORIENT_VERT)
	{
		meter_size.w = request_size.h;
		meter_size.h = request_size.w;
	}

	// calculate element size
	CanvasSize elem_size;
	bool show_text;
	if (orient == ORIENT_HORIZ && !text.IsEmpty ())
	{
		show_text = true;
		elem_size.w = std::max (meter_size.w, text_size.w);
		elem_size.h = meter_size.h + spacing + text_size.h;
	}
	else // ORIENT_VERT and/or empty text
	{
		show_text = false;
		elem_size = meter_size;
	}

	// calculate element position and positions of meter and text inside
	CanvasPoint elem_pos;
	switch (position)
	{
	case POS_NW: case POS_WEST: case POS_SW: default:
		elem_pos.x = MARGIN;
		meter_pos.x = text_pos.x = 0;
		break;
	case POS_NORTH: case POS_CENTER: case POS_SOUTH:
		elem_pos.x = (canvas.w - elem_size.w) / 2;
		meter_pos.x = std::max (0, (text_size.w - meter_size.w) / 2);
		text_pos.x = std::max (0, (meter_size.w - text_size.w) / 2);
		break;
	case POS_NE: case POS_EAST: case POS_SE:
		elem_pos.x = canvas.w - MARGIN - elem_size.w;
		meter_pos.x = std::max (0, text_size.w - meter_size.w);
		text_pos.x = std::max (0, meter_size.w - text_size.w);
		break;
	}
	switch (position)
	{
	case POS_NW: case POS_NORTH: case POS_NE: default: // text below
		elem_pos.y = MARGIN;
		meter_pos.y = 0;
		text_pos.y = meter_size.h + spacing;
		break;
	case POS_WEST: case POS_CENTER: case POS_EAST: // text below
		elem_pos.y = (canvas.h - elem_size.h) / 2;
		meter_pos.y = 0;
		text_pos.y = meter_size.h + spacing;
		break;
	case POS_SW: case POS_SOUTH: case POS_SE: // text above
		elem_pos.y = canvas.h - MARGIN - elem_size.h;
		meter_pos.y = show_text ? (text_size.h + spacing) : 0;
		text_pos.y = 0;
		break;
	}

	SetPosition (elem_pos + offset);
	SetSize (elem_size);
	// If this script ever becomes an overlay, it won't be able to redraw
	// every frame. We would need to subscribe to the qvar/property.
	ScheduleRedraw ();
	return true;
}

void
cScr_StatMeter::Redraw ()
{
	// calculate value-sensitive colors
	ulong tier_color, color_blend = (value_pct < 0.5)
		? AverageColors (color_low, color_med, value_pct * 2.0)
		: AverageColors (color_med, color_high, (value_pct-0.5) * 2.0);
	switch (value_tier)
	{
	case VALUE_LOW: tier_color = color_low; break;
	case VALUE_HIGH: tier_color = color_high; break;
	case VALUE_MED: default: tier_color = color_med; break;
	}

	// draw text, if any
	if (orient == ORIENT_HORIZ && !text.IsEmpty ())
	{
		SetDrawingColor ((style == STYLE_GEM)
			? color_blend : tier_color);
		DrawText (text, text_pos, true);
		SetDrawingOffset (meter_pos);
	}

	// draw progress bar
	if (style == STYLE_PROGRESS)
	{
		CanvasRect fill_area (request_size);
		if (orient == ORIENT_VERT) // invert dimensions
		{
			fill_area.w = request_size.h;
			fill_area.h = request_size.w;
		}

		SetDrawingColor (color_bg);
		FillArea (fill_area);

		SetDrawingColor (tier_color);
		DrawBox (fill_area);

		if (orient == ORIENT_HORIZ)
		{
			fill_area.w = std::lround (fill_area.w * value_pct);
			if (position == POS_NE || position == POS_EAST ||
			    position == POS_SE) // right to left
				fill_area.x = request_size.w - fill_area.w;
		}
		else // ORIENT_VERT
		{
			fill_area.h = std::lround (fill_area.h * value_pct);
			if (position != POS_NW && position != POS_NORTH &&
			    position != POS_NE) // bottom to top
				fill_area.y = request_size.w - fill_area.h;
		}
		FillArea (fill_area);
	}

	// draw individual units
	else if (style == STYLE_UNITS)
	{
		SetDrawingColor (tier_color);
		CanvasPoint unit;
		for (int i = 1; i <= value_int; ++i)
		{
			if (bitmap)
				DrawBitmap (bitmap, HUDBitmap::STATIC, unit);
			else if (symbol != SYMBOL_NONE)
				DrawSymbol (symbol, request_size, unit);

			if (orient == ORIENT_HORIZ)
				unit.x += request_size.w + spacing;
			else // ORIENT_VERT
				unit.y += request_size.h + spacing;
		}
	}

	// draw solid gem
	else if (style == STYLE_GEM)
	{
		if (bitmap)
			DrawBitmap (bitmap, std::lround
				(value_pct * (bitmap->GetFrames () - 1)));
		else
		{
			CanvasRect fill_area (request_size);
			if (orient == ORIENT_VERT)
			{
				fill_area.w = request_size.h;
				fill_area.h = request_size.w;
			}
			SetDrawingColor (color_blend);
			FillArea (fill_area);
			SetDrawingColor (color_bg);
			DrawBox (fill_area);
		}
	}

	SetDrawingOffset ();
}

long
cScr_StatMeter::OnSim (sSimMsg* pMsg, cMultiParm& mpReply)
{
	if (pMsg->fStarting)
		post_sim_fix = true;
	return cScr_HUDElement::OnSim (pMsg, mpReply);
}

long
cScr_StatMeter::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "StatMeterOn"))
	{
		enabled = true;
		return S_OK;
	}

	if (!stricmp (pMsg->message, "StatMeterOff"))
	{
		enabled = false;
		return S_OK;
	}

	return cScr_HUDElement::OnMessage (pMsg, mpReply);
}

void
cScr_StatMeter::OnPropertyChanged (const char* property)
{
	if (!!strcmp (property, "DesignNote")) return;
	ScheduleRedraw (); // too many to check, so just assume we're affected

	cAnsiStr _style = GetParamString ("stat_meter_style", "");
	if (!stricmp (_style, "progress")) style = STYLE_PROGRESS;
	else if (!stricmp (_style, "units")) style = STYLE_UNITS;
	else if (!stricmp (_style, "gem")) style = STYLE_GEM;
	else
	{
		if (!_style.IsEmpty ())
			DebugPrintf ("Warning: `%s' is not a valid stat meter "
				"style.", (const char*) _style);
		style = STYLE_PROGRESS;
	}

	UpdateImage ();

	spacing = GetParamInt ("stat_meter_spacing", 8);

	UpdateText ();

	cAnsiStr _position = GetParamString ("stat_meter_position", "");
	if (!stricmp (_position, "center")) position = POS_CENTER;
	else if (!stricmp (_position, "north")) position = POS_NORTH;
	else if (!stricmp (_position, "ne")) position = POS_NE;
	else if (!stricmp (_position, "east")) position = POS_EAST;
	else if (!stricmp (_position, "se")) position = POS_SE;
	else if (!stricmp (_position, "south")) position = POS_SOUTH;
	else if (!stricmp (_position, "sw")) position = POS_SW;
	else if (!stricmp (_position, "west")) position = POS_WEST;
	else if (!stricmp (_position, "nw")) position = POS_NW;
	else
	{
		if (!_position.IsEmpty ())
			DebugPrintf ("Warning: `%s' is not a valid stat meter "
				"position.", (const char*) _position);
		position = POS_NW;
	}

	offset.x = GetParamInt ("stat_meter_offset_x", 0);
	offset.y = GetParamInt ("stat_meter_offset_y", 0);

	cAnsiStr _orient = GetParamString ("stat_meter_orient", "");
	if (!stricmp (_orient, "horiz")) orient = ORIENT_HORIZ;
	else if (!stricmp (_orient, "vert")) orient = ORIENT_VERT;
	else
	{
		if (!_orient.IsEmpty ())
			DebugPrintf ("Warning: `%s' is not a valid stat meter "
				"orientation.", (const char*) _orient);
		orient = ORIENT_HORIZ;
	}

	if (bitmap)
		request_size = bitmap->GetSize ();
	else
	{
		request_size.w = GetParamInt ("stat_meter_width",
			(style == STYLE_UNITS) ? 32 : 128);
		request_size.h = GetParamInt ("stat_meter_height", 32);
	}

	qvar = GetParamString ("stat_source_qvar", "");

	prop_name = GetParamString ("stat_source_property", "");
	prop_field = GetParamString ("stat_source_field", "");
	if (!qvar.IsEmpty () && !prop_name.IsEmpty ())
		DebugPrintf ("Warning: Both a quest variable and a property "
			"were specified; will use the quest variable and "
			"ignore the property.");
	else if (qvar.IsEmpty () && prop_name.IsEmpty ())
		DebugPrintf ("Warning: Neither a quest variable nor a property "
			"was specified; the stat meter will not be displayed.");

	cAnsiStr _prop_comp = GetParamString ("stat_source_component", "");
	if (!stricmp (_prop_comp, "x")) prop_comp = COMP_X;
	else if (!stricmp (_prop_comp, "y")) prop_comp = COMP_Y;
	else if (!stricmp (_prop_comp, "z")) prop_comp = COMP_Z;
	else
	{
		if (!_prop_comp.IsEmpty ())
			DebugPrintf ("Warning: `%s' is not a valid vector "
				"component.", (const char*) _prop_comp);
		prop_comp = COMP_NONE;
	}

	UpdateObject ();

	low = GetParamInt ("stat_range_low", 25);
	if (low < 0 || low > 100)
		DebugPrintf ("Warning: Low bracket %d%% is out of range.", low);
	high = GetParamInt ("stat_range_high", 75);
	if (high < 0 || high > 100)
		DebugPrintf ("Warning: High bracket %d%% is out of range.", high);
	if (low >= high)
		DebugPrintf ("Warning: Low bracket %d%% is greater than or "
			"equal to high bracket %d%%.", low, high);

	color_bg = GetParamColor ("stat_color_bg", 0x000000);
	color_low = GetParamColor ("stat_color_low", 0x0000ff);
	color_med = GetParamColor ("stat_color_med", 0x00ffff);
	color_high = GetParamColor ("stat_color_high", 0x00ff00);
}

void
cScr_StatMeter::UpdateImage ()
{
	// hold local reference to old bitmap in case it is unchanged
	HUDBitmapPtr old_bitmap = bitmap;
	bitmap.reset ();

	cAnsiStr image = GetParamString ("stat_meter_image",
		(style == STYLE_UNITS) ? "@square" : "@none");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image, false);
	else
	{
		bitmap = LoadBitmap (image, true);
		if (!bitmap)
			_symbol = (style == STYLE_GEM)
				? SYMBOL_NONE : SYMBOL_ARROW;
	}

	if (style == STYLE_PROGRESS && bitmap)
	{
		DebugPrintf ("Warning: Bitmap image `%s' will be ignored for "
			"a progress-style meter.", (const char*) image);
		bitmap.reset ();
	}

	if (style != STYLE_UNITS && symbol != SYMBOL_NONE)
	{
		DebugPrintf ("Warning: Symbol %s will be ignored for a "
			"non-units-style meter.", (const char*) image);
		symbol = SYMBOL_NONE;
	}

	if (symbol != _symbol || bitmap != old_bitmap)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}

}

void
cScr_StatMeter::UpdateText ()
{
	cAnsiStr __text = GetParamString ("stat_meter_text", "@none");
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr _text;

	if (__text.IsEmpty () || !stricmp (__text, "@none"))
		{}

	else if (!stricmp (__text, "@name"))
	{
		if (prop_object)
			pDS->GetObjString (_text, prop_object, "objnames");
		else
			pDS->GetString (_text, "hud", qvar, "", "strings");
	}

	else if (!stricmp (__text, "@description"))
	{
		if (prop_object)
			pDS->GetObjString (_text, prop_object, "objdescs");
		else
			DebugPrintf ("Warning: `@description' is not a valid "
				"stat meter text source for a quest variable "
				"statistic.");
	}

	else if (__text.GetAt (0) == '@')
		DebugPrintf ("Warning: `%s' is not a valid stat meter text "
			"source.", (const char*) __text);

	else
		pDS->GetString (_text, "hud", __text, "", "strings");

	if (!!strcmp (text, _text))
	{
		text = _text;
		ScheduleRedraw ();
	}

	//FIXME LGMM _text.Free ();
}

void
cScr_StatMeter::UpdateObject ()
{
	prop_object = GetParamObject ("stat_source_object", ObjId ());
	UpdateText (); // in case _text == @name or == @description

	if (qvar.IsEmpty () && !stricmp (prop_name, "AI_Visibility") &&
	    !stricmp (prop_field, "Level"))
	{
		min = 0.0;
		max = 100.0;
	}
	else if (qvar.IsEmpty () && !stricmp (prop_name, "HitPoints"))
	{
		min = 0.0;
		SService<IPropertySrv> pPS (g_pScriptManager);
		cMultiParm max_hp;
		pPS->Get (max_hp, prop_object, "MAX_HP", NULL);
		max = (max_hp.type == kMT_Int) ? int (max_hp) : 22.0;
	}
	else
	{
		min = 0.0;
		max = 1.0;
	}
	min = GetParamFloat ("stat_range_min", min);
	max = GetParamFloat ("stat_range_max", max);
	if (min > max)
		DebugPrintf ("Warning: Minimum value %f is greater than "
			"maximum value %f.", min, max);
}

