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
#include <cstdarg>
#include <cstdlib>
#include <lg/objstd.h>
#include <darkhook.h>
#include <ScriptLib.h>
#include "ScriptModule.h"



/* CanvasPoint */

const CanvasPoint
CanvasPoint::ORIGIN = { 0, 0 };

const CanvasPoint
CanvasPoint::OFFSCREEN = { -1, -1 };

CanvasPoint::CanvasPoint (int _x, int _y)
	: x (_x), y (_y)
{}

bool
CanvasPoint::Valid () const
{
	// This does not (yet) check whether the point is within the canvas max.
	return x >= 0 && y >= 0;
}

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



/* CanvasSize */

CanvasSize::CanvasSize (int _w, int _h)
	: w (_w), h (_h)
{}

bool
CanvasSize::Valid () const
{
	// An empty size still counts as valid.
	return w >= 0 && h >= 0;
}

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



/* CanvasRect */

const CanvasRect
CanvasRect::NOCLIP = { 0, 0, -1, -1 };

const CanvasRect
CanvasRect::OFFSCREEN = { -1, -1, -1, -1 };

CanvasRect::CanvasRect (int _x, int _y, int _w, int _h)
	: x (_x), y (_y), w (_w), h (_h)
{}

bool
CanvasRect::Valid () const
{
	// A rect may legitimately extend off the screen. This does not (yet)
	// check that at least part of the rect is onscreen.
	return (*this == NOCLIP) || (w >= 0 && h >= 0);
}

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



/* HUDBitmap */

const std::size_t
HUDBitmap::STATIC = 0;

const int
HUDBitmap::INVALID_HANDLE = -1;

HUDBitmap::HUDBitmap (const char* path, bool animation)
{
	if (!path) throw std::invalid_argument ("no path specified");

	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], file[_MAX_FNAME];
	_splitpath (path, NULL, dir, fname, ext);
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);

	for (std::size_t frame = 0; frame < (animation ? 128 : 1); ++frame)
	{
		if (frame == STATIC)
			snprintf (file, _MAX_FNAME, "%s%s", fname, ext);
		else
			snprintf (file, _MAX_FNAME, "%s_%d%s",
				fname, frame, ext);

		int handle = pDOS->GetBitmap (file, dir);
		if (handle != INVALID_HANDLE)
			frames.push_back (handle);
		else
			break;
	}

	if (frames.empty ())
		throw std::runtime_error ("bitmap file invalid or not found");
}

HUDBitmap::~HUDBitmap ()
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	for (auto frame : frames)
		pDOS->FlushBitmap (frame);
}

CanvasSize
HUDBitmap::GetSize () const
{
	CanvasSize result;
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->GetBitmapSize (frames.front (), result.w, result.h);
	return result;
}

std::size_t
HUDBitmap::GetFrames () const
{
	return frames.size ();
}

void
HUDBitmap::Draw (std::size_t frame, CanvasPoint position, CanvasRect clip)
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	if (clip == CanvasRect::NOCLIP)
		pDOS->DrawBitmap (frames.at (frame), position.x, position.y);
	else
	{
		CanvasSize size = GetSize ();
		if (clip.w == CanvasRect::NOCLIP.w) clip.w = size.w - clip.x;
		if (clip.h == CanvasRect::NOCLIP.h) clip.h = size.h - clip.y;
		pDOS->DrawSubBitmap (frames.at (frame), position.x, position.y,
			clip.x, clip.y, clip.w, clip.h);
	}
}



/* CustomHUD */

CustomHUD::CustomHUD ()
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (this);
}

CustomHUD::~CustomHUD ()
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (NULL);
}

CustomHUDPtr
CustomHUD::Get ()
{
	static std::weak_ptr<CustomHUD> single;
	CustomHUDPtr result = single.lock ();
	if (!result)
	{
		result = CustomHUDPtr (new CustomHUD ());
		single = result;
	}
	return result;
}

void
CustomHUD::DrawHUD ()
{
	for (auto element : elements)
		element->DrawStage1 ();
}

void
CustomHUD::DrawTOverlay ()
{
	for (auto element : elements)
		element->DrawStage2 ();
}

void
CustomHUD::OnUIEnterMode ()
{
	for (auto element : elements)
		element->EnterGameMode ();
}

void
CustomHUD::RegisterElement (HUDElement& element)
{
	elements.push_back (&element);
}

void
CustomHUD::UnregisterElement (HUDElement& element)
{
	elements.remove (&element);
}

HUDBitmapPtr
CustomHUD::LoadBitmap (const char* path, bool animation)
{
	HUDBitmapPtr result;

	// try an existing bitmap
	HUDBitmaps::iterator existing = bitmaps.find (path);
	if (existing != bitmaps.end ())
	{
		result = existing->second.lock ();
		if (result)
			return result;
		else
			bitmaps.erase (existing);
	}

	try
	{
		// load a new one
		result = HUDBitmapPtr (new HUDBitmap (path, animation));
		bitmaps.insert (std::make_pair (path, result));
	}
	catch (std::exception& e)
	{
		::DebugPrintf ("Warning: Could not load bitmap at `%s': %s.",
			path, e.what ());
	}

	return result;
}



/* HUDElement */

#define INVALID_OVERLAY -1

#define CHECK_OVERLAY(retval) \
	if (!IsOverlay ()) \
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

// add the element position (if not overlay) and offset to the point
#define OFFSET(point) (point + \
	(IsOverlay () ? CanvasPoint::ORIGIN : position) + drawing_offset)

HUDElement::HUDElement (object _host)
	: pDOS (g_pScriptManager), host (_host),
	  draw (true), redraw (true), drawing (false),
	  overlay (INVALID_OVERLAY), opacity (255),
	  position (), size (1, 1), scale (1),
	  drawing_color (0xffffff), drawing_offset ()
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
	if (draw && !IsOverlay ())
	{
		redraw = false;
		Redraw ();
	}
	drawing = false;
}

void
HUDElement::DrawStage2 ()
{
	if (!draw || !IsOverlay ()) return;
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
	// register with the handler object
	hud = CustomHUD::Get ();
	hud->RegisterElement (*this);
	return true;
}

void
HUDElement::Deinitialize ()
{
	// destroy any overlay
	DestroyOverlay ();

	// unregister with the handler object
	if (hud)
	{
		hud->UnregisterElement (*this);
		hud.reset ();
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
	return GetObjectParamBool (host, param, GetObjectParamBool
		(StrToObject ("CustomHUD"), param, default_value));
}

ulong
HUDElement::GetParamColor (const char* param, ulong default_value)
{
	return GetObjectParamColor (host, param, GetObjectParamColor
		(StrToObject ("CustomHUD"), param, default_value));
}

float
HUDElement::GetParamFloat (const char* param, float default_value)
{
	return GetObjectParamFloat (host, param, GetObjectParamFloat
		(StrToObject ("CustomHUD"), param, default_value));
}

int
HUDElement::GetParamInt (const char* param, int default_value)
{
	return GetObjectParamInt (host, param, GetObjectParamInt
		(StrToObject ("CustomHUD"), param, default_value));
}

object
HUDElement::GetParamObject (const char* param, object default_value)
{
	return GetObjectParamObject (host, param, GetObjectParamObject
		(StrToObject ("CustomHUD"), param, default_value));
}

cAnsiStr
HUDElement::GetParamString (const char* param, const char* default_value)
{
	char* handler_val = GetObjectParamString
		(StrToObject ("CustomHUD"), param, default_value);
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

void
HUDElement::Redraw ()
{
	_DebugPrintf ("Error: Redraw unimplemented in a HUD element class.");
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
HUDElement::IsOverlay ()
{
	return overlay > INVALID_OVERLAY;
}

bool
HUDElement::CreateOverlay ()
{
	if (IsOverlay ()) return true;

	overlay = pDOS->CreateTOverlayItem (position.x, position.y,
		size.w, size.h, opacity, true);
	ScheduleRedraw ();

	if (IsOverlay ())
		return true;
	else
	{
		_DebugPrintf ("Error: Could not create HUD overlay.");
		return false;
	}
}

void
HUDElement::DestroyOverlay ()
{
	if (IsOverlay ())
	{
		pDOS->DestroyTOverlayItem (overlay);
		overlay = INVALID_OVERLAY;
		ScheduleRedraw ();
	}
}

void
HUDElement::SetOpacity (int _opacity)
{
	opacity = _opacity;
	CHECK_OVERLAY ();
	pDOS->UpdateTOverlayAlpha (overlay, opacity);
}

CanvasSize
HUDElement::GetCanvasSize ()
{
	CanvasSize result;
	SService<IEngineSrv> pES (g_pScriptManager);
	pES->GetCanvasSize (result.w, result.h);
	return result;
}

void
HUDElement::SetPosition (CanvasPoint _position)
{
	if (position == _position) return;
	position = _position;
	if (IsOverlay ())
		pDOS->UpdateTOverlayPosition (overlay, position.x, position.y);
	else
		ScheduleRedraw ();
}

CanvasSize
HUDElement::GetSize ()
{
	return size;
}

void
HUDElement::SetSize (CanvasSize _size)
{
	if (size == _size) return;
	size = _size;

	// This is not intended behavior. Set size before creating overlay.
	if (IsOverlay ())
	{
		DestroyOverlay (); // destroy overlay at old size
		CreateOverlay (); // get a new one at the new size
		if (scale != 1) // restore scale if needed
			pDOS->UpdateTOverlaySize (overlay,
				size.w * scale, size.h * scale);
		ScheduleRedraw ();
	}

	// For non-overlay elements, size is ignored.
}

void
HUDElement::SetScale (int _scale)
{
	if (scale == _scale) return;
	scale = _scale;
	CHECK_OVERLAY ();
	pDOS->UpdateTOverlaySize (overlay, size.w * scale, size.h * scale);
}

void
HUDElement::SetDrawingColor (ulong color)
{
	// don't check for equality; this method may be used to restore
	drawing_color = color;
	CHECK_DRAWING ();
	pDOS->SetTextColor (getred (drawing_color), getgreen (drawing_color),
		getblue (drawing_color));
}

void
HUDElement::SetDrawingOffset (CanvasPoint offset)
{
	CHECK_DRAWING ();
	drawing_offset = offset;
}

void
HUDElement::FillBackground (int color, int _opacity)
{
	CHECK_OVERLAY ();
	CHECK_DRAWING ();
	pDOS->FillTOverlay (color, _opacity);
}

void
HUDElement::FillArea (CanvasRect area)
{
	CHECK_DRAWING ();

	if (area == CanvasRect::NOCLIP)
	{
		area.x = position.x;
		area.y = position.y;
		area.w = size.w;
		area.h = size.h;
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

	if (area == CanvasRect::NOCLIP)
	{
		area.x = position.x;
		area.y = position.y;
		area.w = size.w;
		area.h = size.h;
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
HUDElement::DrawText (const char* text, CanvasPoint _position, bool shadowed)
{
	CHECK_DRAWING ();
	_position = OFFSET (_position);
	if (shadowed)
	{
		pDOS->SetTextColor (0, 0, 0);
		pDOS->DrawString (text, _position.x + 1, _position.y + 1);
		SetDrawingColor (drawing_color);
	}
	pDOS->DrawString (text, _position.x, _position.y);
}

HUDBitmapPtr
HUDElement::LoadBitmap (const char* path, bool animation)
{
	return hud ? hud->LoadBitmap (path, animation) : HUDBitmapPtr ();
}

void
HUDElement::DrawBitmap (HUDBitmapPtr& bitmap, std::size_t frame,
	CanvasPoint _position, CanvasRect clip)
{
	CHECK_DRAWING ();
	if (bitmap)
		bitmap->Draw (frame, OFFSET (_position), clip);
	else
		_DebugPrintf ("Error: DrawBitmap called for invalid bitmap.");
}

CanvasPoint
HUDElement::LocationToCanvas (const cScrVec& location)
{
	CanvasPoint result;
	CHECK_DRAWING (CanvasPoint::OFFSCREEN);
	bool onscreen = pDOS->WorldToScreen (location, result.x, result.y);
	return onscreen ? result : CanvasPoint::OFFSCREEN;
}

CanvasRect
HUDElement::ObjectToCanvas (object target)
{
	CHECK_DRAWING (CanvasRect::OFFSCREEN);
	int x1, y1, x2, y2;
	bool onscreen = pDOS->GetObjectScreenBounds (target, x1, y1, x2, y2);
	return onscreen ? CanvasRect (x1, y1, x2 - x1, y2 - y1)
		: CanvasRect::OFFSCREEN;
}

CanvasPoint
HUDElement::ObjectCentroidToCanvas (object target)
{
	SService<IObjectSrv> pOS (g_pScriptManager);
	cScrVec centroid; pOS->Position (centroid, target);
	CanvasPoint result = LocationToCanvas (centroid);
	if (!result.Valid ()) // centroid is out, try bounds
	{
		CanvasRect bounds = ObjectToCanvas (target);
		if (bounds.Valid ()) // use center of bounds instead
		{
			result.x = bounds.x + bounds.w / 2;
			result.y = bounds.y + bounds.h / 2;
		}
	}
	return result;
}

void
HUDElement::DrawSymbol (Symbol symbol, CanvasSize _size,
	CanvasPoint _position, Direction direction, bool shadowed)
{
	if (shadowed)
	{
		pDOS->SetTextColor (0, 0, 0);
		CanvasPoint shadow_pos (_position.x + 1, _position.y + 1);
		DrawSymbol (symbol, _size, shadow_pos, direction, false);
		SetDrawingColor (drawing_color);
	}

	CanvasPoint xqtr (_size.w / 4, 0), yqtr (0, _size.h / 4);
	drawing_offset = drawing_offset + _position;

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
		FillArea (CanvasRect (0, 0, _size.w, _size.h));
		break;
	case SYMBOL_NONE:
	default:
		break;
	}

	drawing_offset = drawing_offset - _position;
}

CanvasPoint
HUDElement::GetSymbolCenter (Symbol symbol, CanvasSize _size,
	Direction direction)
{
	switch (symbol)
	{
	case SYMBOL_ARROW:
		switch (direction)
		{
		case DIRN_LEFT:
			return CanvasPoint (0, _size.h / 2);
		case DIRN_RIGHT:
			return CanvasPoint (_size.w, _size.h / 2);
		case DIRN_NONE:
		default:
			break; // fall through to below (= center)
		}
	case SYMBOL_CROSSHAIRS:
	case SYMBOL_RETICULE:
	case SYMBOL_SQUARE:
		return CanvasPoint (_size.w / 2, _size.h / 2);
	case SYMBOL_NONE:
	default:
		return CanvasPoint::OFFSCREEN;
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
			_DebugPrintf ("Warning: A non-directional HUD element "
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
		_DebugPrintf ("Warning: `%s' is not a valid symbol name.",
			symbol);

	return SYMBOL_NONE;
}

void
HUDElement::_DebugPrintf (const char* format, ...)
{
	va_list va;
	char message[1000];

	va_start (va, format);
	_vsnprintf (message, 999, format, va);
	va_end (va);

	g_pfnMPrintf ("HUDElement [%d]: %s\n", int (host), message);
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
		DebugString ("Warning: The DarkHook service could not be "
			"located. This custom HUD element may work properly.");
		return false;
	}
}

void
cScr_HUDElement::OnPropertyChanged (const char*)
{}



/* KDQuestArrow */

const CanvasSize
cScr_QuestArrow::SYMBOL_SIZE = { 32, 32 };

const int
cScr_QuestArrow::PADDING = 8;

cScr_QuestArrow::cScr_QuestArrow (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (QuestArrow, enabled, iHostObjId),
	  obscured (false), objective (OBJECTIVE_NONE),
	  symbol (SYMBOL_NONE), symbol_dirn (DIRN_NONE),
	  bitmap (), image_pos (), text (), text_pos (),
	  color (0), shadow (true)
{}

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
		image_size = bitmap ? bitmap->GetSize () : SYMBOL_SIZE,
		text_size = GetTextSize (text),
		elem_size;
	elem_size.w = image_size.w + PADDING + text_size.w;
	elem_size.h = std::max (image_size.h, text_size.h);

	// get object's position in canvas coordinates
	CanvasPoint obj_pos = ObjectCentroidToCanvas (ObjId ());
	if (!obj_pos.Valid ()) return false;

	// choose alignment of image and text
	symbol_dirn = (obj_pos.x > canvas.w / 2)
		? DIRN_RIGHT // text on left, image on right
		: DIRN_LEFT; // text on right, image on left

	// calculate absolute position of image
	CanvasPoint image_center, image_apos;
	image_center = bitmap ? CanvasPoint (image_size.w / 2, image_size.h / 2)
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
	SetSize (elem_size);
	return true;
}

void
cScr_QuestArrow::Redraw ()
{
	SetDrawingColor (color);

	if (bitmap)
		DrawBitmap (bitmap, HUDBitmap::STATIC, image_pos);
	else if (symbol != SYMBOL_NONE)
		DrawSymbol (symbol, SYMBOL_SIZE, image_pos, symbol_dirn, shadow);

	DrawText (text, text_pos, shadow);
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
	bitmap.reset ();
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
	if (pMsg->event == kContainAdd &&
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
	if (objective > OBJECTIVE_NONE)
	{
		snprintf (qvar, 256, "goal_state_%d", objective);
		pQS->UnsubscribeMsg (ObjId (), qvar);
		snprintf (qvar, 256, "goal_visible_%d", objective);
		pQS->UnsubscribeMsg (ObjId (), qvar);
	}

	objective = GetParamInt ("quest_arrow_goal", OBJECTIVE_NONE);
	if (objective <= OBJECTIVE_NONE) return;

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
	if (objective <= OBJECTIVE_NONE) return;

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
	if (objective <= OBJECTIVE_NONE)
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
	// hold local reference to old bitmap in case it is unchanged
	HUDBitmapPtr old_bitmap = bitmap;
	bitmap.reset ();

	cAnsiStr image = GetParamString ("quest_arrow_image", "@arrow");
	Symbol _symbol = SYMBOL_NONE;

	if (image.GetAt (0) == '@')
		_symbol = InterpretSymbol (image, true);
	else
	{
		bitmap = LoadBitmap (image);
		if (!bitmap)
			_symbol = SYMBOL_ARROW;
	}

	if (symbol != _symbol || bitmap != old_bitmap)
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
		DebugPrintf ("Warning: `%s' is not a valid quest arrow text "
			"source.", (const char*) __text);

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
	bool _shadow = GetParamBool ("quest_arrow_shadow", true);
	if (color != _color || shadow != _shadow)
	{
		color = _color;
		shadow = _shadow;
		ScheduleRedraw ();
	}
}



/* KDStatMeter */

const int
cScr_StatMeter::MARGIN = 16;

cScr_StatMeter::cScr_StatMeter (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (StatMeter, enabled, iHostObjId), style (STYLE_PROGRESS),
	  position (POS_NW), offset (), orient (ORIENT_HORIZ),
	  symbol (SYMBOL_NONE), bitmap (), size (), spacing (0),
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

	// draw progress bar
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
				DrawSymbol (symbol, size, unit);

			if (orient == ORIENT_HORIZ)
				unit.x += size.w + spacing;
			else // ORIENT_VERT
				unit.y += size.h + spacing;
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
	bitmap.reset ();
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
	ScheduleRedraw (); // too many to check, so just assume we're affected

	cAnsiStr _style = GetParamString ("stat_meter_style", NULL);
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
			DebugPrintf ("Warning: `%s' is not a valid stat meter "
				"position.", (const char*) _position);
		position = POS_NW;
	}

	offset.x = GetParamInt ("stat_meter_offset_x", 0);
	offset.y = GetParamInt ("stat_meter_offset_y", 0);

	cAnsiStr _orient = GetParamString ("stat_meter_orient", NULL);
	if (!stricmp (_orient, "horiz")) orient = ORIENT_HORIZ;
	else if (!stricmp (_orient, "vert")) orient = ORIENT_VERT;
	else
	{
		if (!_orient.IsEmpty ())
			DebugPrintf ("Warning: `%s' is not a valid stat meter "
				"orientation.", (const char*) _orient);
		orient = ORIENT_HORIZ;
	}

	UpdateImage ();

	if (bitmap)
		size = bitmap->GetSize ();
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
		DebugPrintf ("Warning: Both a quest variable and a property "
			"were specified; will use the quest variable and "
			"ignore the property.");

	cAnsiStr _prop_comp = GetParamString ("stat_source_component", NULL);
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
		(style == STYLE_GEM) ? "@none" : "@square");
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

	if (symbol != _symbol || bitmap != old_bitmap)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}

	if (style == STYLE_PROGRESS && (bitmap || symbol != SYMBOL_NONE))
		DebugString ("Warning: stat_meter_image will be ignored for "
			"a progress-style meter.");

	if (style == STYLE_GEM && symbol != SYMBOL_NONE)
		DebugString ("Warning: Symbol %s will be ignored for a "
			"gem-style meter.", (const char*) image);
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
		DebugPrintf ("Warning: Minimum value %f is greater than "
			"maximum value %f.", min, max);
}



/* KDToolSight */

const CanvasSize
cScr_ToolSight::SYMBOL_SIZE = { 24, 24 };

cScr_ToolSight::cScr_ToolSight (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (ToolSight, enabled, iHostObjId),
	  symbol (SYMBOL_NONE), bitmap (), color (0), offset ()
{}

bool
cScr_ToolSight::Prepare ()
{
	if (!enabled) return false;

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
cScr_ToolSight::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_HUDElement::OnBeginScript (pMsg, mpReply);
	enabled.Init (false);
	OnPropertyChanged ("DesignNote");
	SubscribeProperty ("DesignNote");
	return result;
}

long
cScr_ToolSight::OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	bitmap.reset ();
	return cScr_HUDElement::OnEndScript (pMsg, mpReply);
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

