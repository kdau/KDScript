/******************************************************************************
 *  KDHUDElement.cpp: HUDElement, KDHUDElement
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

#include "KDHUDElement.h"
#include <cstdarg>
#include <ScriptLib.h>
#include <darkhook.h>
#include "utils.h"
#include "ScriptModule.h"



/* HUDElement */

#define INVALID_OVERLAY -1

#ifdef DEBUG
#define _DEBUG_PRINTF(format, ...) _DebugPrintf (format, __VA_ARGS__)
#else
#define _DEBUG_PRINTF(format, ...)
#endif

#define CHECK_OVERLAY(retval) \
	if (!IsOverlay ()) \
	{ \
		_DEBUG_PRINTF ("%s called for non-overlay element; ignoring.", \
			__func__); \
		return retval; \
	}

#define CHECK_DRAWING(retval) \
	if (!drawing) \
	{ \
		_DEBUG_PRINTF ("%s called outside of draw cycle; ignoring.", \
			__func__); \
		return retval; \
	}

HUDElement::HUDElement (object _host)
	: pDOS (g_pScriptManager), host (_host),
	  draw (true), redraw (true), drawing (false),
	  overlay (INVALID_OVERLAY), opacity (255),
	  position (), size (1, 1), scale (1.0),
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
	if (hud) // already registered
		return false;
	else if (hud = CustomHUD::Get ()) // register with the handler
	{
		_DEBUG_PRINTF ("Registering %p with CustomHUD %p.",
			this, hud.get ());
		return hud->RegisterElement (*this);
	}
	else
		return false;
}

void
HUDElement::Deinitialize ()
{
	// destroy any overlay
	DestroyOverlay ();

	// unregister with the handler
	if (hud)
	{
		_DEBUG_PRINTF ("Unregistering %p with CustomHUD %p.",
			this, hud.get ());
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
		if (scale != 1.0) // restore scale if needed
			pDOS->UpdateTOverlaySize (overlay,
				size.w * scale, size.h * scale);
		ScheduleRedraw ();
	}

	// For non-overlay elements, size is ignored.
}

void
HUDElement::SetScale (float _scale)
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
	Offset (area);
	for (int y = area.y; y < area.y + area.h; ++y)
		pDOS->DrawLine (area.x, y, area.x + area.w, y);
}

void
HUDElement::DrawBox (CanvasRect area)
{
	CHECK_DRAWING ();
	Offset (area);
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
	Offset (from);
	Offset (to);
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
	Offset (_position);
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
	Offset (_position);
	if (bitmap)
		bitmap->Draw (frame, _position, clip);
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
	drawing_offset += _position;

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
		FillArea (CanvasRect (_size));
		break;
	case SYMBOL_NONE:
	default:
		break;
	}

	drawing_offset -= _position;
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
HUDElement::Offset (CanvasPoint& point)
{
	point += drawing_offset;
	if (!IsOverlay ()) point += position;
}

void
HUDElement::Offset (CanvasRect& area)
{
	Offset (*(CanvasPoint*)(&area));
	if (area.w == CanvasRect::NOCLIP.w) area.w = size.w;
	if (area.h == CanvasRect::NOCLIP.h) area.h = size.h;
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

void
cScr_HUDElement::OnAnyMessage (sScrMsg*)
{
	// ignore messages outside of game mode
	SService<IVersionSrv> pVS (g_pScriptManager);
	if (pVS->IsEditor () == 1) return;

	Initialize ();
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

