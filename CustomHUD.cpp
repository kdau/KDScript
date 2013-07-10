/******************************************************************************
 *  CustomHUD.cpp: script to draw custom HUD elements
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
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

#include "CustomHUD.h"
#include <cmath>
#include <sec_api/stdlib_s.h>
#include <lg/objstd.h>
#include <darkhook.h>
#include <ScriptLib.h>
#include "ScriptModule.h"
#include "utils.h"



/* CanvasPoint, CanvasSize, CanvasRect */

CanvasPoint::CanvasPoint (int _x, int _y)
	: x (_x), y (_y)
{}

CanvasPoint::CanvasPoint (const CanvasSize& area)
	: x (area.w), y (area.h)
{}

bool
CanvasPoint::operator == (const CanvasPoint& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool
CanvasPoint::operator != (const CanvasPoint& rhs) const
{
	return x != rhs.x || y != rhs.y;
}

CanvasPoint
CanvasPoint::operator + (const CanvasPoint& rhs) const
{
	return CanvasPoint (x + rhs.x, y + rhs.y);
}

CanvasPoint
CanvasPoint::operator - (const CanvasPoint& rhs) const
{
	return CanvasPoint (x - rhs.x, y - rhs.y);
}

CanvasPoint
CanvasPoint::operator * (int rhs) const
{
	return CanvasPoint (x * rhs, y * rhs);
}

CanvasPoint
CanvasPoint::operator / (int rhs) const
{
	return CanvasPoint (x / rhs, y / rhs);
}

CanvasSize::CanvasSize (int _w, int _h)
	: w (_w), h (_h)
{}

bool
CanvasSize::operator == (const CanvasSize& rhs) const
{
	return w == rhs.w && h == rhs.h;
}

bool
CanvasSize::operator != (const CanvasSize& rhs) const
{
	return w != rhs.w || h != rhs.h;
}

CanvasRect::CanvasRect (int _x, int _y, int _w, int _h)
	: x (_x), y (_y), w (_w), h (_h)
{}

bool
CanvasRect::operator == (const CanvasRect& rhs) const
{
	return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
}

bool
CanvasRect::operator != (const CanvasRect& rhs) const
{
	return x != rhs.x || y != rhs.y || w != rhs.w || h != rhs.h;
}

CanvasRect
CanvasRect::operator + (const CanvasPoint& rhs) const
{
	return CanvasRect (x + rhs.x, y + rhs.y, w, h);
}

CanvasRect
CanvasRect::operator - (const CanvasPoint& rhs) const
{
	return CanvasRect (x - rhs.x, y - rhs.y, w, h);
}



/* KDCustomHUD */

cScr_CustomHUD::cScr_CustomHUD (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_CustomHUD::OnBeginScript (sScrMsg*, cMultiParm&)
{
	if (!!stricmp (ObjectToStr (ObjId ()), "CustomHUD"))
		DebugString ("Warning: this script should be placed on an object named `CustomHUD'.");

	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (this);

	return S_OK;
}

long
cScr_CustomHUD::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "RegisterElement") &&
	    pMsg->data.type == kMT_Int)
	{
		elements.push_back
			(reinterpret_cast<cScr_HUDElement*> (long (pMsg->data)));
		return S_OK;
	}
	else
	if (!stricmp (pMsg->message, "UnregisterElement") &&
	    pMsg->data.type == kMT_Int)
	{
		elements.remove
			(reinterpret_cast<cScr_HUDElement*> (long (pMsg->data)));
		return S_OK;
	}

	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_CustomHUD::OnEndScript (sScrMsg*, cMultiParm&)
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (NULL);
	return S_OK;
}

void
cScr_CustomHUD::DrawHUD ()
{
	for (auto element : elements)
		element->DrawStage1 ();
}

void
cScr_CustomHUD::DrawTOverlay ()
{
	for (auto element : elements)
		element->DrawStage2 ();
}

void
cScr_CustomHUD::OnUIEnterMode ()
{
	for (auto element : elements)
		element->EnterGameMode ();
}



/* HUDElement */

#define IS_OVERLAY() (overlay > -1)

#define CHECK_OVERLAY(retval) \
	if (!IS_OVERLAY ()) \
	{ \
		DEBUG_PRINTF ("%s called for non-overlay element; ignoring.", \
			__func__); \
		return retval; \
	}

#define CHECK_DRAWING(retval) \
	if (!drawing) \
	{ \
		DEBUG_PRINTF ("%s called outside of draw cycle; ignoring.", \
			__func__); \
		return retval; \
	}

// if not an overlay, add the element position to the drawing position
#define OFFSET(position) \
	(position + (IS_OVERLAY () ? ORIGIN : last_position) + drawing_offset)

HUDElement::HUDElement (object _host)
	: pDOS (g_pScriptManager), host (_host), handler (), overlay (-1),
	  draw (true), redraw (true), drawing (false),
	  last_position (), last_size (1, 1), last_scale (1),
	  last_opacity (255), last_color (0xffffff), drawing_offset ()
{}

HUDElement::~HUDElement ()
{
	Deinitialize ();
}

void
HUDElement::DrawStage1 ()
{
	drawing = true;
	draw = Prepare ();
	if (draw && !IS_OVERLAY ())
	{
		redraw = false;
		Redraw ();
	}
	drawing = false;
}

void
HUDElement::DrawStage2 ()
{
	if (!draw || !IS_OVERLAY ()) return;
	if (redraw && pDOS->BeginTOverlayUpdate (overlay))
	{
		redraw = false;
		drawing = true;
		Redraw ();
		drawing = false;
		pDOS->EndTOverlayUpdate ();
	}
	pDOS->DrawTOverlayItem (overlay);
}

void
HUDElement::EnterGameMode ()
{}

bool
HUDElement::Initialize ()
{
	// locate the handler object
	ScriptParamsIter handler_link (host, "CustomHUD");
	handler = handler_link ? handler_link : StrToObject ("CustomHUD");

	if (!handler)
	{
		DebugPrintf ("Warning: could not locate handler object. "
			"This HUD element cannot be drawn.");
		return false;
	}

	// register with handler
	SimpleSend (host, handler, "RegisterElement", long (this));
	return true;
}

void
HUDElement::Deinitialize ()
{
	// unregister from handler
	if (handler)
	{
		SimpleSend (host, handler, "UnregisterElement", long (this));
		handler = 0;
	}

	// destroy any overlay
	if (IS_OVERLAY ())
	{
		pDOS->DestroyTOverlayItem (overlay);
		overlay = -1;
	}
}

object
HUDElement::GetHost ()
{
	return host;
}

bool
HUDElement::GetParamBool (const char* param, bool default_value)
{
	return GetObjectParamBool (host, param,
		GetObjectParamBool (handler, param, default_value));
}

ulong
HUDElement::GetParamColor (const char* param, ulong default_value)
{
	return GetObjectParamColor (host, param,
		GetObjectParamColor (handler, param, default_value));
}

float
HUDElement::GetParamFloat (const char* param, float default_value)
{
	return GetObjectParamFloat (host, param,
		GetObjectParamFloat (handler, param, default_value));
}

int
HUDElement::GetParamInt (const char* param, int default_value)
{
	return GetObjectParamInt (host, param,
		GetObjectParamInt (handler, param, default_value));
}

object
HUDElement::GetParamObject (const char* param, object default_value)
{
	return GetObjectParamObject (host, param,
		GetObjectParamObject (handler, param, default_value));
}

cAnsiStr
HUDElement::GetParamString (const char* param, const char* default_value)
{
	char* handler_val = GetObjectParamString (handler, param, default_value);
	char* host_val = GetObjectParamString (host, param, handler_val);
	cAnsiStr result;
	if (host_val) { result = host_val; g_pMalloc->Free (host_val); }
	if (handler_val) g_pMalloc->Free (handler_val);
	return result;
}

void
HUDElement::SetParamBool (const char* param, bool value)
{
	SetObjectParamBool (host, param, value);
}

void
HUDElement::SetParamFloat (const char* param, float value)
{
	SetObjectParamFloat (host, param, value);
}

void
HUDElement::SetParamInt (const char* param, int value)
{
	SetObjectParamInt (host, param, value);
}

void
HUDElement::SetParamString (const char* param, const char* value)
{
	SetObjectParamString (host, param, value);
}

bool
HUDElement::Prepare ()
{
	return true;
}

bool
HUDElement::NeedsRedraw ()
{
	return redraw;
}

void
HUDElement::ScheduleRedraw ()
{
	redraw = true;
}

bool
HUDElement::SetIsOverlay (bool is_overlay)
{
	if (is_overlay && !IS_OVERLAY ())
	{
		overlay = pDOS->CreateTOverlayItem
			(last_position.x, last_position.y, last_size.w,
			last_size.h, last_opacity, true);
		if (!IS_OVERLAY ())
		{
			DebugPrintf ("Warning: could not create HUD overlay.");
			return false;
		}
	}
	else if (!is_overlay && IS_OVERLAY ())
	{
		pDOS->DestroyTOverlayItem (overlay);
		overlay = -1;
	}
	return true;
}

void
HUDElement::SetOpacity (int opacity)
{
	last_opacity = opacity;
	CHECK_OVERLAY ();
	pDOS->UpdateTOverlayAlpha (overlay, opacity);
}

CanvasSize
HUDElement::GetCanvasSize ()
{
	int width = 0, height = 0;
	SService<IEngineSrv> pES (g_pScriptManager);
	pES->GetCanvasSize (width, height);
	return CanvasSize (width, height);
}

void
HUDElement::SetPosition (CanvasPoint position)
{
	last_position = position;
	if (IS_OVERLAY ())
		pDOS->UpdateTOverlayPosition (overlay, position.x, position.y);
	// For non-overlay elements, last_position will be used when drawing.
}

CanvasSize
HUDElement::GetSize ()
{
	return last_size;
}

void
HUDElement::SetSize (CanvasSize size)
{
	last_size = size;
	if (IS_OVERLAY ())
	{
		// This is not intended behavior and should be avoided.
		SetIsOverlay (false); // destroy old overlay
		SetIsOverlay (true); // and get a new one at the new size
		if (last_scale != 1) SetScale (last_scale); // restore scale
		ScheduleRedraw ();
	}
	// For non-overlay elements, size is ignored.
}

void
HUDElement::SetScale (int scale)
{
	last_scale = scale;
	CHECK_OVERLAY ();
	pDOS->UpdateTOverlaySize (overlay,
		last_size.w * scale, last_size.h * scale);
}

void
HUDElement::SetDrawingColor (ulong color)
{
	last_color = color;
	CHECK_DRAWING ();
	pDOS->SetTextColor (getred (color), getgreen (color), getblue (color));
}

void
HUDElement::SetDrawingOffset (CanvasPoint offset)
{
	CHECK_DRAWING ();
	drawing_offset = offset;
}

void
HUDElement::FillBackground (int color, int opacity)
{
	CHECK_OVERLAY ();
	CHECK_DRAWING ();
	pDOS->FillTOverlay (color, opacity);
}

void
HUDElement::FillArea (CanvasRect area)
{
	CHECK_DRAWING ();

	if (area == NOCLIP)
	{
		area.x = last_position.x;
		area.y = last_position.y;
		area.w = last_size.w;
		area.h = last_size.h;
	}
	else
		area = OFFSET (area);

	for (int y = area.y; y < area.y + area.h; ++y)
		pDOS->DrawLine (area.x, y, area.x + area.w, y);
}

void
HUDElement::DrawBox (CanvasRect area)
{
	CHECK_DRAWING ();

	if (area == NOCLIP)
	{
		area.x = last_position.x;
		area.y = last_position.y;
		area.w = last_size.w;
		area.h = last_size.h;
	}
	else
		area = OFFSET (area);

	pDOS->DrawLine (area.x, area.y, area.x + area.w, area.y);
	pDOS->DrawLine (area.x, area.y, area.x, area.y + area.h);
	pDOS->DrawLine (area.x + area.w, area.y,
		area.x + area.w, area.y + area.h);
	pDOS->DrawLine (area.x, area.y + area.h,
		area.x + area.w, area.y + area.h);
}

void
HUDElement::DrawLine (CanvasPoint from, CanvasPoint to)
{
	CHECK_DRAWING ();
	from = OFFSET (from);
	to = OFFSET (to);
	pDOS->DrawLine (from.x, from.y, to.x, to.y);
}

CanvasSize
HUDElement::GetTextSize (const char* text)
{
	CanvasSize result;
	CHECK_DRAWING (result);
	pDOS->GetStringSize (text, result.w, result.h);
	return result;
}

void
HUDElement::DrawText (const char* text, CanvasPoint position, bool shadowed)
{
	CHECK_DRAWING ();
	position = OFFSET (position);
	if (shadowed)
	{
		pDOS->SetTextColor (0, 0, 0);
		pDOS->DrawString (text, position.x + 1, position.y + 1);
		SetDrawingColor (last_color);
	}
	pDOS->DrawString (text, position.x, position.y);
}

int
HUDElement::LoadBitmap (const char* path)
{
	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], file[256];
	_splitpath (path, NULL, dir, fname, ext);
	snprintf (file, 256, "%s%s", fname, ext);
	return pDOS->GetBitmap (file, dir);
}

void
HUDElement::LoadBitmaps (const char* path, std::vector<int>& bitmaps)
{
	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], file[256];
	_splitpath (path, NULL, dir, fname, ext);

	for (int frame = 0; frame < 128; ++frame)
	{
		if (frame == 0)
			snprintf (file, 256, "%s%s", fname, ext);
		else
			snprintf (file, 256, "%s_%d%s", fname, frame, ext);

		int bitmap = pDOS->GetBitmap (file, dir);
		if (bitmap > -1)
			bitmaps.push_back (bitmap);
		else
			break;
	}
}

CanvasSize
HUDElement::GetBitmapSize (int bitmap)
{
	CanvasSize result;
	if (bitmap > -1)
		pDOS->GetBitmapSize (bitmap, result.w, result.h);
	return result;
}

void
HUDElement::DrawBitmap (int bitmap, CanvasPoint position, CanvasRect clip)
{
	CHECK_DRAWING ();
	position = OFFSET (position);
	if (clip == NOCLIP)
		pDOS->DrawBitmap (bitmap, position.x, position.y);
	else
	{
		CanvasSize bitmap_size = GetBitmapSize (bitmap);
		if (clip.w == -1) clip.w = bitmap_size.w - clip.x;
		if (clip.h == -1) clip.h = bitmap_size.h - clip.y;
		pDOS->DrawSubBitmap (bitmap, position.x, position.y,
			clip.x, clip.y, clip.w, clip.h);
	}
}

void
HUDElement::FreeBitmap (int bitmap)
{
	if (bitmap > -1) pDOS->FlushBitmap (bitmap);
}

CanvasPoint
HUDElement::LocationToCanvas (const cScrVec& location)
{
	CanvasPoint result;
	CHECK_DRAWING (OFFSCREEN);
	bool onscreen = pDOS->WorldToScreen (location, result.x, result.y);
	return onscreen ? result : OFFSCREEN;
}

CanvasRect
HUDElement::ObjectToCanvas (object target)
{
	CHECK_DRAWING (OFFSCREEN_R);
	int x1, y1, x2, y2;
	bool onscreen = pDOS->GetObjectScreenBounds (target, x1, y1, x2, y2);
	return onscreen ? CanvasRect (x1, y1, x2 - x1, y2 - y1) : OFFSCREEN_R;
}

CanvasPoint
HUDElement::ObjectCentroidToCanvas (object target)
{
	SService<IObjectSrv> pOS (g_pScriptManager);
	cScrVec centroid; pOS->Position (centroid, target);
	CanvasPoint result = LocationToCanvas (centroid);
	if (result == OFFSCREEN) // centroid is out, try bounds
	{
		CanvasRect bounds = ObjectToCanvas (target);
		if (bounds != OFFSCREEN_R) // use center of bounds instead
		{
			result.x = bounds.x + bounds.w / 2;
			result.y = bounds.y + bounds.h / 2;
		}
	}
	return result;
}

void
HUDElement::DrawSymbol (Symbol symbol, CanvasSize size,
	CanvasPoint position, Direction direction, bool shadowed)
{
	if (shadowed)
	{
		pDOS->SetTextColor (0, 0, 0);
		CanvasPoint shadow_pos (position.x + 1, position.y + 1);
		DrawSymbol (symbol, size, shadow_pos, direction, false);
		SetDrawingColor (last_color);
	}

	CanvasPoint xqtr (size.w / 4, 0), yqtr (0, size.h / 4);
	drawing_offset = drawing_offset + position;

	switch (symbol)
	{
	case SYMBOL_ARROW:
		DrawLine (yqtr*2, yqtr*2 + xqtr*4);
		switch (direction)
		{
		case DIRN_LEFT:
			DrawLine (yqtr*2, yqtr   + xqtr);
			DrawLine (yqtr*2, yqtr*3 + xqtr);
			break;
		case DIRN_RIGHT:
			DrawLine (yqtr*2 + xqtr*4, yqtr   + xqtr*3);
			DrawLine (yqtr*2 + xqtr*4, yqtr*3 + xqtr*3);
			break;
		case DIRN_NONE:
		default:
			break; // no head on a directionless arrow
		}
		break;
	case SYMBOL_RETICULE:
		DrawLine (xqtr   + yqtr,   xqtr*3 + yqtr);
		DrawLine (xqtr   + yqtr,   xqtr   + yqtr*3);
		DrawLine (xqtr*3 + yqtr,   xqtr*3 + yqtr*3);
		DrawLine (xqtr   + yqtr*3, xqtr*3 + yqtr*3);
		// includes crosshairs
	case SYMBOL_CROSSHAIRS:
		DrawLine (xqtr*2, xqtr*2 + yqtr*4);
		DrawLine (yqtr*2, xqtr*4 + yqtr*2);
		break;
	case SYMBOL_SQUARE:
		FillArea (CanvasRect (0, 0, size.w, size.h));
		break;
	case SYMBOL_NONE:
	default:
		break;
	}

	drawing_offset = drawing_offset - position;
}

CanvasPoint
HUDElement::GetSymbolCenter (Symbol symbol, CanvasSize size,
	Direction direction)
{
	switch (symbol)
	{
	case SYMBOL_ARROW:
		switch (direction)
		{
		case DIRN_LEFT:
			return CanvasPoint (0, size.h / 2);
		case DIRN_RIGHT:
			return CanvasPoint (size.w, size.h / 2);
		case DIRN_NONE:
		default:
			break; // fall through to below (= center)
		}
	case SYMBOL_CROSSHAIRS:
	case SYMBOL_RETICULE:
	case SYMBOL_SQUARE:
		return CanvasPoint (size.w / 2, size.h / 2);
	case SYMBOL_NONE:
	default:
		return OFFSCREEN;
	}
}

HUDElement::Symbol
HUDElement::InterpretSymbol (const char* symbol, bool directional)
{
	if (!symbol || !stricmp (symbol, "@none"))
		return SYMBOL_NONE;

	else if (!stricmp (symbol, "@arrow"))
	{
		if (!directional)
		{
			DebugPrintf ("Warning: a non-directional HUD element "
				"cannot have an arrow symbol.");
			return SYMBOL_NONE;
		}
		return SYMBOL_ARROW;
	}

	else if (!stricmp (symbol, "@crosshairs"))
		return SYMBOL_CROSSHAIRS;

	else if (!stricmp (symbol, "@reticule"))
		return SYMBOL_RETICULE;

	else if (!stricmp (symbol, "@square"))
		return SYMBOL_SQUARE;

	else if (symbol[0] == '@')
		DebugPrintf ("Warning: invalid symbol name `%s'.", symbol);

	return SYMBOL_NONE;
}



/* KDHUDElement */

cScr_HUDElement::cScr_HUDElement (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId), HUDElement (iHostObjId)
{
	DarkHookInitializeService (g_pScriptManager, g_pMalloc);
}

long
cScr_HUDElement::OnBeginScript (sScrMsg*, cMultiParm&)
{
	return Initialize () ? S_OK : S_FALSE;
}

long
cScr_HUDElement::OnEndScript (sScrMsg*, cMultiParm&)
{
	Deinitialize ();
	return S_OK;
}

long
cScr_HUDElement::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "DHNotify"))
	{
		auto dh = static_cast<sDHNotifyMsg*> (pMsg);
		if (dh->typeDH != kDH_Property) return S_FALSE;
		OnPropertyChanged (dh->sProp.pszPropName);
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

bool
cScr_HUDElement::SubscribeProperty (const char* property)
{
	try
	{
		SService<IDarkHookScriptService> pDHS (g_pScriptManager);
		return pDHS->InstallPropHook (ObjId (), kDHNotifyDefault,
			property, ObjId ());
	}
	catch (no_interface&)
	{
		DebugString ("The DarkHook service could not be located. "
			"This custom HUD element may not update properly.");
		return false;
	}
}

void
cScr_HUDElement::OnPropertyChanged (const char*)
{}

void
cScr_HUDElement::Redraw ()
{}



/* KDQuestArrow */

const CanvasSize
cScr_QuestArrow::SYMBOL_SIZE = { 32, 32 };

const int
cScr_QuestArrow::PADDING = 8;

cScr_QuestArrow::cScr_QuestArrow (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (QuestArrow, enabled, iHostObjId), obscured (false),
	  objective (-1), symbol (SYMBOL_NONE), symbol_dirn (DIRN_NONE),
	  bitmap (-1), image_pos (), text (), text_pos (), color (0)
{}

// No override of EnterGameMode. If the canvas size did change and the text
// needs to be moved relative to the image, the next Prepare call will do it.

bool
cScr_QuestArrow::Prepare ()
{
	if (!enabled) return false;

	// confirm object is actually visible, if required
	if (!obscured)
	{
		SService<IObjectSrv> pOS (g_pScriptManager);
		true_bool rendered; pOS->RenderedThisFrame (rendered, ObjId ());
		if (!rendered) return false;
	}

	// get canvas, image, and text size and calculate element size
	CanvasSize canvas = GetCanvasSize (),
		image_size = (bitmap > -1)
			? GetBitmapSize (bitmap) : SYMBOL_SIZE,
		text_size = GetTextSize (text),
		elem_size;
	elem_size.w = image_size.w + PADDING + text_size.w;
	elem_size.h = std::max (image_size.h, text_size.h);

	// get object's position in canvas coordinates
	CanvasPoint obj_pos = ObjectCentroidToCanvas (ObjId ());
	if (obj_pos == OFFSCREEN) return false;

	// choose alignment of image and text
	symbol_dirn = (obj_pos.x > canvas.w / 2)
		? DIRN_RIGHT // text on left, image on right
		: DIRN_LEFT; // text on right, image on left

	// calculate absolute position of image
	CanvasPoint image_center, image_apos;
	image_center = (bitmap > -1)
		? CanvasPoint (image_size.w / 2, image_size.h / 2)
		: GetSymbolCenter (symbol, SYMBOL_SIZE, symbol_dirn);
	image_apos.x = obj_pos.x - image_center.x;
	image_apos.y = obj_pos.y - image_center.y;

	// calculate absolute position of text
	CanvasPoint text_apos;
	if (symbol_dirn == DIRN_RIGHT) // text on left
		text_apos.x = image_apos.x - PADDING - text_size.w;
	else // text on right
		text_apos.x = image_apos.x + image_size.w + PADDING;
	text_apos.y = obj_pos.y - text_size.h / 2;

	// calculate element position
	CanvasPoint elem_pos;
	elem_pos.x = std::min (image_apos.x, text_apos.x);
	elem_pos.y = std::min (image_apos.y, text_apos.y);

	// update relative position of image, if needed
	if (image_pos != image_apos - elem_pos)
	{
		image_pos = image_apos - elem_pos;
		ScheduleRedraw ();
	}

	// update relative position of text, if needed
	if (text_pos != text_apos - elem_pos)
	{
		text_pos = text_apos - elem_pos;
		ScheduleRedraw ();
	}

	SetPosition (elem_pos);
	if (NeedsRedraw ()) SetSize (elem_size);

	return true;
}

void
cScr_QuestArrow::Redraw ()
{
	SetDrawingColor (color);

	if (bitmap > -1)
		DrawBitmap (bitmap, image_pos);
	else if (symbol != SYMBOL_NONE)
		DrawSymbol (symbol, SYMBOL_SIZE, image_pos, symbol_dirn, true);

	DrawText (text, text_pos, true);
}

long
cScr_QuestArrow::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_HUDElement::OnBeginScript (pMsg, mpReply);

	enabled.Init (GetParamBool ("quest_arrow", true));
	OnPropertyChanged ("DesignNote"); // update all cached params

	SubscribeProperty ("DesignNote");
	SubscribeProperty ("GameName"); // for quest_arrow_text == "@name"

	return result;
}

long
cScr_QuestArrow::OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bitmap > -1)
	{
		FreeBitmap (bitmap);
		bitmap = -1;
	}
	return cScr_HUDElement::OnEndScript (pMsg, mpReply);
}

long
cScr_QuestArrow::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "QuestArrowOn"))
	{
		enabled = true;
		return S_OK;
	}

	if (!stricmp (pMsg->message, "QuestArrowOff") ||
	    !stricmp (pMsg->message, "Slain") ||
	    (!stricmp (pMsg->message, "AIModeChange") &&
	        static_cast<sAIModeChangeMsg*> (pMsg)->mode == kAIM_Dead))
	{
		enabled = false;
		return S_OK;
	}

	return cScr_HUDElement::OnMessage (pMsg, mpReply);
}

void
cScr_QuestArrow::OnPropertyChanged (const char* property)
{
	if (!strcmp (property, "DesignNote"))
	{
		obscured = GetParamBool ("quest_arrow_obscured", false);
		UpdateObjective ();
		UpdateImage ();
		UpdateText ();
		UpdateColor ();
	}
	else if (!strcmp (property, "GameName"))
		UpdateText ();
}

long
cScr_QuestArrow::OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply)
{
	if (pMsg->event == 2 && // link added
	    InheritsFrom ("Avatar", pMsg->container))
		enabled = false;
	return cScr_HUDElement::OnContained (pMsg, mpReply);
}

long
cScr_QuestArrow::OnQuestChange (sQuestMsg* pMsg, cMultiParm& mpReply)
{
	SetEnabledFromObjective ();
	return cScr_HUDElement::OnQuestChange (pMsg, mpReply);
}

void
cScr_QuestArrow::UpdateObjective ()
{
	SService<IQuestSrv> pQS (g_pScriptManager);
	char qvar[256];

	// unsubscribe from old objective
	if (objective > -1)
	{
		snprintf (qvar, 256, "goal_state_%d", objective);
		pQS->UnsubscribeMsg (ObjId (), qvar);
		snprintf (qvar, 256, "goal_visible_%d", objective);
		pQS->UnsubscribeMsg (ObjId (), qvar);
	}

	objective = GetParamInt ("quest_arrow_goal", -1);
	if (objective < 0) return;

	SetEnabledFromObjective ();

	// subscribe to new objective
	snprintf (qvar, 256, "goal_state_%d", objective);
	pQS->SubscribeMsg (ObjId (), qvar, kQuestDataAny);
	snprintf (qvar, 256, "goal_visible_%d", objective);
	pQS->SubscribeMsg (ObjId (), qvar, kQuestDataAny);
}

void
cScr_QuestArrow::SetEnabledFromObjective ()
{
	if (objective < 0) return;

	SService<IQuestSrv> pQS (g_pScriptManager);
	char qvar[256];

	snprintf (qvar, 256, "goal_state_%d", objective);
	bool incomplete = pQS->Get (qvar) == 0;

	snprintf (qvar, 256, "goal_visible_%d", objective);
	bool visible = pQS->Get (qvar) == 1;

	enabled = incomplete && visible;
}

#if (_DARKGAME == 2)
void
cScr_QuestArrow::GetTextFromObjective (cScrStr& msgstr)
{
	if (objective < 0)
	{
		msgstr = NULL;
		return;
	}

	char msgid[256], path[256];
	snprintf (msgid, 256, "text_%d", objective);

	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	snprintf (path, 256, "intrface\\miss%d", pDGS->GetCurrentMission ());

	SService<IDataSrv> pDS (g_pScriptManager);
	pDS->GetString (msgstr, "goals", msgid, "", path);
}
#endif // _DARKGAME == 2

void
cScr_QuestArrow::UpdateImage ()
{
	if (bitmap > -1) // even if it hasn't actually changed
	{
		FreeBitmap (bitmap);
		bitmap = -1;
		ScheduleRedraw ();
	}

	cAnsiStr image = GetParamString ("quest_arrow_image", "@arrow");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image, true);
	else
	{
		bitmap = LoadBitmap (image);
		if (bitmap < 0)
		{
			DebugPrintf ("Warning: could not load quest_arrow_image"
				" at `%s'.", (const char*) image);
			_symbol = SYMBOL_ARROW;
		}
	}

	if (symbol != _symbol)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}
}

void
cScr_QuestArrow::UpdateText ()
{
	cAnsiStr __text = GetParamString ("quest_arrow_text", "@name");
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr _text;

	if (__text.IsEmpty () || !stricmp (__text, "@none"))
		{}

	else if (!stricmp (__text, "@name"))
		pDS->GetObjString (_text, ObjId (), "objnames");

	else if (!stricmp (__text, "@description"))
		pDS->GetObjString (_text, ObjId (), "objdescs");

	else if (!stricmp (__text, "@objective"))
#if (_DARKGAME == 2)
		GetTextFromObjective (_text);
#else
		DebugPrintf ("Warning: quest_arrow_text cannot be `@objective' "
			"in this game. No text will be shown.");
#endif // _DARKGAME == 2

	else if (__text.GetAt (0) == '@')
		DebugPrintf ("Warning: invalid quest_arrow_text value of `%s'.",
			(const char*) __text);

	else
		pDS->GetString (_text, "hud", __text, "", "strings");

	if (!!strcmp (text, _text))
	{
		text = _text;
		ScheduleRedraw ();
	}
}

void
cScr_QuestArrow::UpdateColor ()
{
	ulong _color = GetParamColor ("quest_arrow_color", 0xffffff);
	if (color != _color)
	{
		color = _color;
		ScheduleRedraw ();
	}
}



/* KDStatMeter */

const int
cScr_StatMeter::MARGIN = 16;

cScr_StatMeter::cScr_StatMeter (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (StatMeter, enabled, iHostObjId),
	  style (STYLE_PROGRESS), position (POS_NW), orient (ORIENT_HORIZ),
	  symbol (SYMBOL_NONE), bitmaps (), size (), spacing (0),
	  qvar (), prop_name (), prop_field (), prop_comp (COMP_NONE),
	  prop_object (iHostObjId), post_sim_fix (false),
	  min (0.0), max (0.0), low (0), high (0),
	  color_bg (0), color_low (0), color_med (0), color_high (0),
	  value (0.0), value_pct (0.0), value_tier (VALUE_MED)
{}

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

	// get canvas size
	CanvasSize canvas = GetCanvasSize ();

	// calculate element size
	CanvasSize elem_size = size;
	if (style == STYLE_PROGRESS && orient == ORIENT_VERT)
	{
		elem_size.w = size.h;
		elem_size.h = size.w;
	}
	else if (style == STYLE_UNITS)
	{
		if (orient == ORIENT_HORIZ)
			elem_size.w = value_int * size.w +
				(value_int - 1) * spacing;
		else // ORIENT_VERT
			elem_size.h = value_int * size.h +
				(value_int - 1) * spacing;
	}

	// calculate element position
	CanvasPoint elem_pos;
	switch (position)
	{
	case POS_NW: case POS_WEST: case POS_SW: default:
		elem_pos.x = MARGIN; break;
	case POS_NORTH: case POS_CENTER: case POS_SOUTH:
		elem_pos.x = (canvas.w - elem_size.w) / 2; break;
	case POS_NE: case POS_EAST: case POS_SE:
		elem_pos.x = canvas.w - MARGIN - elem_size.w; break;
	}
	switch (position)
	{
	case POS_NW: case POS_NORTH: case POS_NE: default:
		elem_pos.y = MARGIN; break;
	case POS_WEST: case POS_CENTER: case POS_EAST:
		elem_pos.y = (canvas.h - elem_size.h) / 2; break;
	case POS_SW: case POS_SOUTH: case POS_SE:
		elem_pos.y = canvas.h - MARGIN - elem_size.h; break;
	}

	// If this script ever becomes an overlay, it won't be able to redraw
	// every frame. We would need to subscribe to the qvar/property.
	SetPosition (elem_pos);
	if (NeedsRedraw ())
		SetSize (elem_size);
	else
		ScheduleRedraw ();

	return true;
}

void
cScr_StatMeter::Redraw ()
{
	CanvasSize elem_size = GetSize ();

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

	// draw meter
	if (style == STYLE_PROGRESS)
	{
		SetDrawingColor (color_bg);
		FillArea ();

		SetDrawingColor (tier_color);
		DrawBox ();

		CanvasRect fill_area (0, 0, elem_size.w, elem_size.h);
		if (orient == ORIENT_HORIZ)
			switch (position)
			{
			case POS_NE: case POS_EAST: case POS_SE: // right->left
				fill_area.x = std::lround
					(elem_size.w * (1.0 - value_pct));
				fill_area.w = std::lround
					(elem_size.w * value_pct);
				break;
			default: // left->right
				fill_area.w *= value_pct;
				break;
			}
		else // ORIENT_VERT
			switch (position)
			{
			case POS_NW: case POS_NORTH: case POS_NE: // top->bottom
				fill_area.h *= value_pct;
				break;
			default: // bottom->top
				fill_area.y = std::lround
					(elem_size.h * (1.0 - value_pct));
				fill_area.h = std::lround
					(elem_size.h * value_pct);
				break;
			}
		FillArea (fill_area);
	}

	else if (style == STYLE_UNITS)
	{
		SetDrawingColor (tier_color);
		CanvasPoint unit;
		for (int i = 1; i <= value_int; ++i)
		{
			if (!bitmaps.empty ())
				DrawBitmap (bitmaps[0], unit);
			else if (symbol != SYMBOL_NONE)
				DrawSymbol (symbol, size, unit);

			if (orient == ORIENT_HORIZ)
				unit.x += size.w + spacing;
			else // ORIENT_VERT
				unit.y += size.h + spacing;
		}
	}

	else if (style == STYLE_GEM)
	{
		if (!bitmaps.empty ())
			DrawBitmap (bitmaps[std::lround
				(value_pct * (bitmaps.size () - 1))]);
		else
		{
			SetDrawingColor (color_blend);
			FillArea ();
			SetDrawingColor (color_bg);
			DrawBox ();
		}
	}
}

long
cScr_StatMeter::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_HUDElement::OnBeginScript (pMsg, mpReply);
	enabled.Init (GetParamBool ("stat_meter", true));
	OnPropertyChanged ("DesignNote"); // update all cached params
	SubscribeProperty ("DesignNote");
	return result;
}

long
cScr_StatMeter::OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	FreeBitmaps ();
	return cScr_HUDElement::OnEndScript (pMsg, mpReply);
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
	ScheduleRedraw ();

	cAnsiStr _style = GetParamString ("stat_meter_style", NULL);
	if (!stricmp (_style, "progress")) style = STYLE_PROGRESS;
	else if (!stricmp (_style, "units")) style = STYLE_UNITS;
	else if (!stricmp (_style, "gem")) style = STYLE_GEM;
	else
	{
		if (!_style.IsEmpty ())
			DebugPrintf ("Warning: invalid meter style `%s'.",
				(const char*) _style);
		style = STYLE_PROGRESS;
	}

	cAnsiStr _position = GetParamString ("stat_meter_position", NULL);
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
			DebugPrintf ("Warning: invalid position `%s'.",
				(const char*) _position);
		position = POS_NW;
	}

	cAnsiStr _orient = GetParamString ("stat_meter_orient", NULL);
	if (!stricmp (_orient, "horiz")) orient = ORIENT_HORIZ;
	else if (!stricmp (_orient, "vert")) orient = ORIENT_VERT;
	else
	{
		if (!_orient.IsEmpty ())
			DebugPrintf ("Warning: invalid meter orientation `%s'.",
				(const char*) _orient);
		orient = ORIENT_HORIZ;
	}

	UpdateImage ();

	if (!bitmaps.empty ())
		size = GetBitmapSize (bitmaps[0]);
	else
	{
		size.w = GetParamInt ("stat_meter_width",
			(style == STYLE_UNITS) ? 32 : 128);
		size.h = GetParamInt ("stat_meter_height", 32);
	}

	spacing = GetParamInt ("stat_meter_spacing", 8);

	qvar = GetParamString ("stat_source_qvar", NULL);

	prop_name = GetParamString ("stat_source_property", NULL);
	prop_field = GetParamString ("stat_source_field", NULL);
	if (!qvar.IsEmpty () && !prop_name.IsEmpty ())
		DebugPrintf ("Warning: qvar and property specified; will use "
			"quest variable and ignore property.");

	cAnsiStr _prop_comp = GetParamString ("stat_source_component", NULL);
	if (!stricmp (_prop_comp, "x")) prop_comp = COMP_X;
	else if (!stricmp (_prop_comp, "y")) prop_comp = COMP_Y;
	else if (!stricmp (_prop_comp, "z")) prop_comp = COMP_Z;
	else
	{
		if (!_prop_comp.IsEmpty ())
			DebugPrintf ("Warning: invalid vector component `%s'.",
				(const char*) _prop_comp);
		prop_comp = COMP_NONE;
	}

	UpdateObject ();

	low = GetParamInt ("stat_range_low", 25);
	if (low < 0 || low > 100)
		DebugPrintf ("Warning: low bracket %d%% is out of range.", low);
	high = GetParamInt ("stat_range_high", 75);
	if (high < 0 || high > 100)
		DebugPrintf ("Warning: high bracket %d%% is out of range.", high);
	if (low >= high)
		DebugPrintf ("Warning: low bracket %d%% is greater than or "
			"equal to high bracket %d%%", low, high);

	color_bg = GetParamColor ("stat_color_bg", 0x000000);
	color_low = GetParamColor ("stat_color_low", 0x0000ff);
	color_med = GetParamColor ("stat_color_med", 0x00ffff);
	color_high = GetParamColor ("stat_color_high", 0x00ff00);
}

void
cScr_StatMeter::UpdateImage ()
{
	FreeBitmaps ();

	cAnsiStr image = GetParamString ("stat_meter_image",
		(style == STYLE_GEM) ? "@none" : "@square");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image);

	else
	{
		LoadBitmaps (image, bitmaps);
		if (bitmaps.empty ())
		{
			DebugPrintf ("Warning: could not load stat_meter_image"
				" at `%s'.", (const char*) image);
			_symbol = (style == STYLE_GEM)
				? SYMBOL_NONE : SYMBOL_ARROW;
		}
	}

	if (symbol != _symbol)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}

	if ((!bitmaps.empty () || symbol != SYMBOL_NONE) &&
	    style == STYLE_PROGRESS)
		DebugString ("Warning: stat_meter_image will be ignored for "
			"a progress-style meter.");

	if (symbol != SYMBOL_NONE && style == STYLE_GEM)
		DebugString ("Warning: symbol will be ignored for a gem-style "
			"meter.");
}

void
cScr_StatMeter::FreeBitmaps ()
{
	while (!bitmaps.empty ())
	{
		FreeBitmap (bitmaps.back ());
		bitmaps.pop_back ();
	}
}

void
cScr_StatMeter::UpdateObject ()
{
	prop_object = GetParamObject ("stat_source_object", ObjId ());

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
		DebugPrintf ("Warning: minimum value %f is greater than "
			"maximum value %f", min, max);
}



/* KDToolSight */

const CanvasSize
cScr_ToolSight::SYMBOL_SIZE = { 24, 24 };

cScr_ToolSight::cScr_ToolSight (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (ToolSight, enabled, iHostObjId),
	  symbol (SYMBOL_NONE), bitmap (-1), color (0)
{}

bool
cScr_ToolSight::Prepare ()
{
	if (!enabled) return false;

	// get canvas, image, and text size and calculate element size
	CanvasSize canvas = GetCanvasSize (),
		elem_size = (bitmap > -1)
			? GetBitmapSize (bitmap) : SYMBOL_SIZE;

	// calculate center of canvas and position of element
	CanvasPoint canvas_center (canvas.w / 2, canvas.h / 2),
		elem_pos (canvas_center.x - elem_size.w / 2,
			canvas_center.y - elem_size.h / 2);

	SetPosition (elem_pos);
	if (NeedsRedraw ()) SetSize (elem_size);

	return true;
}

void
cScr_ToolSight::Redraw ()
{
	SetDrawingColor (color);
	if (bitmap > -1)
		DrawBitmap (bitmap);
	else if (symbol != SYMBOL_NONE)
		DrawSymbol (symbol, SYMBOL_SIZE);
}

long
cScr_ToolSight::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_HUDElement::OnBeginScript (pMsg, mpReply);
	enabled.Init (false);
	UpdateImage ();
	UpdateColor ();
	SubscribeProperty ("DesignNote");
	return result;
}

long
cScr_ToolSight::OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bitmap > -1)
	{
		FreeBitmap (bitmap);
		bitmap = -1;
	}
	return cScr_HUDElement::OnEndScript (pMsg, mpReply);
}

void
cScr_ToolSight::OnPropertyChanged (const char* property)
{
	if (!strcmp (property, "DesignNote"))
	{
		UpdateImage ();
		UpdateColor ();
	}
}

long
cScr_ToolSight::OnInvSelect (sScrMsg* pMsg, cMultiParm& mpReply)
{
	enabled = true;
	return cScr_HUDElement::OnInvSelect (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvFocus (sScrMsg* pMsg, cMultiParm& mpReply)
{
	enabled = true;
	return cScr_HUDElement::OnInvFocus (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvDeSelect (sScrMsg* pMsg, cMultiParm& mpReply)
{
	enabled = false;
	return cScr_HUDElement::OnInvDeSelect (pMsg, mpReply);
}

long
cScr_ToolSight::OnInvDeFocus (sScrMsg* pMsg, cMultiParm& mpReply)
{
	enabled = false;
	return cScr_HUDElement::OnInvDeFocus (pMsg, mpReply);
}

void
cScr_ToolSight::UpdateImage ()
{
	if (bitmap > -1) // even if it hasn't actually changed
	{
		FreeBitmap (bitmap);
		bitmap = -1;
		ScheduleRedraw ();
	}

	cAnsiStr image = GetParamString ("tool_sight_image", "@crosshairs");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image);

	else
	{
		bitmap = LoadBitmap (image);
		if (bitmap < 0)
		{
			DebugPrintf ("Warning: could not load tool_sight_image"
				" at `%s'.", (const char*) image);
			_symbol = SYMBOL_CROSSHAIRS;
		}
	}

	if (symbol != _symbol)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}
}

void
cScr_ToolSight::UpdateColor ()
{
	ulong _color = GetParamColor ("tool_sight_color", 0x808080);
	if (color != _color)
	{
		color = _color;
		ScheduleRedraw ();
	}
}

