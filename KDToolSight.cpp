/******************************************************************************
 *  KDToolSight.cpp
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

#include "KDToolSight.h"
#include <ScriptLib.h>
#include "utils.h"

const CanvasSize
cScr_ToolSight::SYMBOL_SIZE = { 24, 24 };

cScr_ToolSight::cScr_ToolSight (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  selected (false),
	  symbol (SYMBOL_NONE), bitmap (), color (0), offset ()
{}

bool
cScr_ToolSight::Initialize ()
{
	if (!cScr_HUDElement::Initialize ()) return false;
	OnPropertyChanged ("DesignNote");
	SubscribeProperty ("DesignNote");
	return true;
}

bool
cScr_ToolSight::Prepare ()
{
	if (!selected) return false;

	// get canvas, image, and text size and calculate element size
	CanvasSize canvas = GetCanvasSize (),
		elem_size = bitmap ? bitmap->GetSize () : SYMBOL_SIZE;

	// calculate center of canvas and position of element
	CanvasPoint canvas_center (canvas.w / 2, canvas.h / 2),
		elem_pos (canvas_center.x - elem_size.w / 2 + offset.x,
			canvas_center.y - elem_size.h / 2 + offset.y);

	SetPosition (elem_pos);
	SetSize (elem_size);
	return true;
}

void
cScr_ToolSight::Redraw ()
{
	SetDrawingColor (color);
	if (bitmap)
		DrawBitmap (bitmap, HUDBitmap::STATIC);
	else if (symbol != SYMBOL_NONE)
		DrawSymbol (symbol, SYMBOL_SIZE);
}

long
cScr_ToolSight::OnInvSelect (sScrMsg* pMsg, cMultiParm& mpReply)
{
	selected = true;
	return cScr_HUDElement::OnInvSelect (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvFocus (sScrMsg* pMsg, cMultiParm& mpReply)
{
	selected = true;
	return cScr_HUDElement::OnInvFocus (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvDeSelect (sScrMsg* pMsg, cMultiParm& mpReply)
{
	selected = false;
	return cScr_HUDElement::OnInvDeSelect (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvDeFocus (sScrMsg* pMsg, cMultiParm& mpReply)
{
	selected = false;
	return cScr_HUDElement::OnInvDeFocus (pMsg, mpReply);
}

void
cScr_ToolSight::OnPropertyChanged (const char* property)
{
	if (!!strcmp (property, "DesignNote")) return;

	UpdateImage ();

	ulong _color = GetParamColor ("tool_sight_color", 0x808080);
	if (color != _color)
	{
		color = _color;
		ScheduleRedraw ();
	}

	CanvasPoint _offset (GetParamInt ("tool_sight_offset_x", 0),
		GetParamInt ("tool_sight_offset_y", 0));
	if (offset != _offset)
	{
		offset = _offset;
		ScheduleRedraw ();
	}
}

void
cScr_ToolSight::UpdateImage ()
{
	// hold local reference to old bitmap in case it is unchanged
	HUDBitmapPtr old_bitmap = bitmap;
	bitmap.reset ();

	cAnsiStr image = GetParamString ("tool_sight_image", "@crosshairs");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image, false);
	else
	{
		bitmap = LoadBitmap (image);
		if (!bitmap)
			_symbol = SYMBOL_CROSSHAIRS;
	}

	if (symbol != _symbol || bitmap != old_bitmap)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}
}

