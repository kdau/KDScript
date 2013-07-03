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
	  last_opacity (255), drawing_offset ()
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

char*
HUDElement::GetParamString (const char* param, const char* default_value)
{
	char* handler_val = GetObjectParamString (handler, param, default_value);
	char* result = GetObjectParamString (host, param, handler_val);
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
HUDElement::DrawText (const char* text, CanvasPoint position)
{
	CHECK_DRAWING ();
	position = OFFSET (position);
	pDOS->DrawString (text, position.x, position.y);
}

int
HUDElement::LoadBitmap (const char* path)
{
	char dir[256], fname[256], ext[256], file[256];
	_splitpath_s (path, NULL, 0, dir, 256, fname, 256, ext, 256);
	snprintf (file, 256, "%s%s", fname, ext);
	return pDOS->GetBitmap (file, dir);
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
	CanvasPoint position, Direction direction)
{
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
	case SYMBOL_RETICULE:
	case SYMBOL_CROSSHAIRS:
		return CanvasPoint (size.w / 2, size.h / 2);
	case SYMBOL_NONE:
	default:
		return OFFSCREEN;
	}
}

HUDElement::Symbol
HUDElement::InterpretSymbol (const char* symbol)
{
	if (!symbol || !stricmp (symbol, "@none"))
		return SYMBOL_NONE;

	else if (!stricmp (symbol, "@arrow"))
		return SYMBOL_ARROW;

	else if (!stricmp (symbol, "@crosshairs"))
		return SYMBOL_CROSSHAIRS;

	else if (!stricmp (symbol, "@reticule"))
		return SYMBOL_RETICULE;

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
	  SCRIPT_VAROBJ (QuestArrow, enabled, iHostObjId), objective (-1),
	  symbol (SYMBOL_NONE), symbol_dirn (DIRN_NONE), bitmap (-1),
	  image_pos (), text (), text_pos (), color (0)
{}

// No override of EnterGameMode. If the canvas size did change and the text
// needs to be moved relative to the image, the next Prepare call will do it.

bool
cScr_QuestArrow::Prepare ()
{
	if (!enabled) return false;

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
		DrawSymbol (symbol, SYMBOL_SIZE, image_pos, symbol_dirn);

	DrawText (text, text_pos);
}

long
cScr_QuestArrow::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_HUDElement::OnBeginScript (pMsg, mpReply);

	enabled.Init (GetParamBool ("quest_arrow", true));
	UpdateObjective ();
	UpdateImage ();
	UpdateText ();
	UpdateColor ();

	SubscribeProperty ("DesignNote");
	SubscribeProperty ("GameName"); // for quest_arrow_text == "@name"

	return result;
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

void
cScr_QuestArrow::UpdateImage ()
{
	if (bitmap > -1) // even if it hasn't actually changed
	{
		FreeBitmap (bitmap);
		bitmap = -1;
		ScheduleRedraw ();
	}

	char* image = GetParamString ("quest_arrow_image", "@arrow");
	Symbol _symbol = SYMBOL_NONE;

	if (image[0] == '@')
		_symbol = InterpretSymbol (image);
	else
	{
		bitmap = LoadBitmap (image);
		if (bitmap < 0)
		{
			DebugPrintf ("Warning: could not load quest_arrow_image"
				" at `%s'.", image);
			_symbol = SYMBOL_ARROW;
		}
	}

	if (image) g_pMalloc->Free (image);
	if (symbol != _symbol)
	{
		symbol = _symbol;
		ScheduleRedraw ();
	}
}

void
cScr_QuestArrow::UpdateText ()
{
	char* __text = GetParamString ("quest_arrow_text", "@name");
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr _text;

	if (!__text || !stricmp (__text, "") || !stricmp (__text, "@none"))
		{}

	else if (!stricmp (__text, "@name"))
		pDS->GetObjString (_text, ObjId (), "objnames");

	else if (!stricmp (__text, "@desc"))
		pDS->GetObjString (_text, ObjId (), "objdescs");

	else if (!stricmp (__text, "@goal"))
		GetTextFromObjective (_text);

	else if (__text[0] == '@')
		DebugPrintf ("Warning: invalid quest_arrow_text value of `%s'.",
			__text);

	else
		pDS->GetString (_text, "hud", __text, "", "strings");

	if (__text) g_pMalloc->Free (__text);
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

	char* image = GetParamString ("tool_sight_image", "@crosshairs");
	Symbol _symbol = SYMBOL_NONE;

	if (image[0] == '@')
	{
		_symbol = InterpretSymbol (image);
		if (_symbol == SYMBOL_ARROW)
		{
			DebugString ("Warning: a tool sight cannot have an "
				"arrow symbol.");
			_symbol = SYMBOL_CROSSHAIRS;
		}
	}
	else
	{
		bitmap = LoadBitmap (image);
		if (bitmap < 0)
		{
			DebugPrintf ("Warning: could not load tool_sight_image"
				" at `%s'.", image);
			_symbol = SYMBOL_CROSSHAIRS;
		}
	}

	if (image) g_pMalloc->Free (image);
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

