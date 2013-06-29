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
#include <ScriptLib.h>
#include "ScriptModule.h"
#include "utils.h"



/* CustomHUD */

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
	return 0;
}

long
cScr_CustomHUD::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "RegisterElement") &&
	    pMsg->data.type == kMT_Int)
	{
		elements.push_back
			(reinterpret_cast<cScr_HUDElement*> (long (pMsg->data)));
		return 0;
	}
	else
	if (!stricmp (pMsg->message, "UnregisterElement") &&
	    pMsg->data.type == kMT_Int)
	{
		elements.remove
			(reinterpret_cast<cScr_HUDElement*> (long (pMsg->data)));
		return 0;
	}

	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_CustomHUD::OnEndScript (sScrMsg*, cMultiParm&)
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (NULL);
	return 0;
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
		element->CanvasChanged ();
}



/* HUDElement */


cScr_HUDElement::cScr_HUDElement (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId), pDOS (g_pScriptManager),
	  handler (), handle (-1), draw (true), redraw (true), redrawing (false)
{}

void
cScr_HUDElement::DrawStage1 ()
{
	draw = Prepare ();
}

void
cScr_HUDElement::DrawStage2 ()
{
	if (!draw) return;
	if (redraw && pDOS->BeginTOverlayUpdate (handle))
	{
		redraw = false;
		redrawing = true;
		Redraw ();
		redrawing = false;
		pDOS->EndTOverlayUpdate ();
	}
	pDOS->DrawTOverlayItem (handle);
}

void
cScr_HUDElement::CanvasChanged ()
{}

long
cScr_HUDElement::OnBeginScript (sScrMsg*, cMultiParm&)
{
	// locate the handler object
	ScriptParamsIter handler_link (ObjId (), "CustomHUD");
	handler = handler_link ? handler_link : StrToObject ("CustomHUD");
	if (!handler)
		throw std::runtime_error ("could not locate handler object");

	// obtain an overlay handle
	handle = pDOS->CreateTOverlayItem (0, 0, 1, 1, 255, true); //FIXME Set actual size here?!
	if (handle == -1)
		throw std::runtime_error ("could not allocate overlay");

	SimpleSend (ObjId (), handler, "RegisterElement", long (this));
	return 0;
}

long
cScr_HUDElement::OnEndScript (sScrMsg*, cMultiParm&)
{
	if (handle > -1)
		pDOS->DestroyTOverlayItem (handle);

	if (handler)
		SimpleSend (ObjId (), handler, "UnregisterElement", long (this));

	return 0;
}

bool
cScr_HUDElement::GetParamBool (const char* param, bool default_value)
{
	return GetObjectParamBool (ObjId (), param,
		GetObjectParamBool (handler, param, default_value));
}

ulong
cScr_HUDElement::GetParamColor (const char* param, ulong default_value)
{
	return GetObjectParamColor (ObjId (), param,
		GetObjectParamColor (handler, param, default_value));
}

float
cScr_HUDElement::GetParamFloat (const char* param, float default_value)
{
	return GetObjectParamFloat (ObjId (), param,
		GetObjectParamFloat (handler, param, default_value));
}

int
cScr_HUDElement::GetParamInt (const char* param, int default_value)
{
	return GetObjectParamInt (ObjId (), param,
		GetObjectParamInt (handler, param, default_value));
}

char*
cScr_HUDElement::GetParamString (const char* param, const char* default_value)
{
	char* on_handler = GetObjectParamString (handler, param, default_value);
	char* result = GetObjectParamString (ObjId (), param, on_handler);
	if (on_handler) g_pMalloc->Free (on_handler);
	return result;
}

void
cScr_HUDElement::SetParamBool (const char* param, bool value)
{
	SetObjectParamBool (ObjId (), param, value);
}

void
cScr_HUDElement::SetParamFloat (const char* param, float value)
{
	SetObjectParamFloat (ObjId (), param, value);
}

void
cScr_HUDElement::SetParamInt (const char* param, int value)
{
	SetObjectParamInt (ObjId (), param, value);
}

void
cScr_HUDElement::SetParamString (const char* param, const char* value)
{
	SetObjectParamString (ObjId (), param, value);
}

bool
cScr_HUDElement::Prepare ()
{
	return true;
}

bool
cScr_HUDElement::NeedsRedraw ()
{
	return redraw;
}

void
cScr_HUDElement::ScheduleRedraw ()
{
	redraw = true;
}

void
cScr_HUDElement::SetOpacity (int opacity)
{
	pDOS->UpdateTOverlayAlpha (handle, opacity);
}

void
cScr_HUDElement::GetCanvasSize (int& width, int& height)
{
	SService<IEngineSrv> pES (g_pScriptManager);
	pES->GetCanvasSize (width, height);
}

void
cScr_HUDElement::SetPosition (int x, int y)
{
	pDOS->UpdateTOverlayPosition (handle, x, y);
}

void
cScr_HUDElement::SetSize (int width, int height)
{
	pDOS->UpdateTOverlaySize (handle, width, height);
}

#define CHECK_REDRAWING(retval) \
	if (!redrawing) \
	{ \
		DEBUG_PRINTF ("%s called outside of redraw; ignoring.", __func__); \
		return retval; \
	}

void
cScr_HUDElement::FillBackground (int color, int opacity)
{
	CHECK_REDRAWING ();
	pDOS->FillTOverlay (color, opacity);
}

void
cScr_HUDElement::SetDrawingColor (ulong color)
{
	CHECK_REDRAWING ();
	pDOS->SetTextColor (getred (color), getgreen (color), getblue (color));
}

void
cScr_HUDElement::DrawLine (int x1, int y1, int x2, int y2)
{
	CHECK_REDRAWING ();
	pDOS->DrawLine (x1, y1, x2, y2);
}

void
cScr_HUDElement::GetTextSize (const char* text, int& width, int& height)
{
	if (!redrawing) width = height = 0;
	CHECK_REDRAWING ();
	pDOS->GetStringSize (text, width, height);
}

void
cScr_HUDElement::DrawText (const char* text, int x, int y)
{
	CHECK_REDRAWING ();
	pDOS->DrawString (text, x, y);
}

int
cScr_HUDElement::LoadBitmap (const char* path)
{
	char dir[256], fname[256], ext[256], file[256];
	_splitpath_s (path, NULL, 0, dir, 256, fname, 256, ext, 256);
	snprintf (file, 256, "%s%s", fname, ext);
	return pDOS->GetBitmap (file, dir);
}

void
cScr_HUDElement::GetBitmapSize (int bitmap, int& width, int& height)
{
	if (bitmap > -1)
		pDOS->GetBitmapSize (bitmap, width, height);
	else
		width = height = 0;
}

void
cScr_HUDElement::DrawBitmap (int bitmap, int x, int y, int src_x, int src_y,
                        int src_width, int src_height)
{
	CHECK_REDRAWING ();
	if (src_x == 0 && src_y == 0 && src_width == -1 && src_height == -1)
		pDOS->DrawBitmap (bitmap, x, y);
	else
	{
		int bw, bh; GetBitmapSize (bitmap, bw, bh);
		if (src_width == -1) src_width = bw - src_x;
		if (src_height == -1) src_height = bh - src_y;
		pDOS->DrawSubBitmap (bitmap, x, y, src_x, src_y,
			src_width, src_height);
	}
}

void
cScr_HUDElement::FreeBitmap (int bitmap)
{
	if (bitmap > -1) pDOS->FlushBitmap (bitmap);
}

bool
cScr_HUDElement::LocationToScreen (const cScrVec& location, int& x, int& y)
{
	// no check performed; can be called from Prepare or Redraw
	return pDOS->WorldToScreen (location, x, y);
}

bool
cScr_HUDElement::ObjectToScreen (object target,
                                 int& x1, int& y1, int& x2, int& y2)
{
	// no check performed; can be called from Prepare or Redraw
	return pDOS->GetObjectScreenBounds (target, x1, y1, x2, y2);
}



/* QuestArrow */

cScr_QuestArrow::cScr_QuestArrow (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_HUDElement (pszName, iHostObjId),
	  SCRIPT_VAROBJ (QuestArrow, enabled, iHostObjId),
	  objective (-1), image (IMAGE_NONE), bitmap (-1), text (), color (0)
{
	DarkHookInitializeService (g_pScriptManager, g_pMalloc);
}

// No override of CanvasChanged. If the canvas size did change and the text
// needs to be moved relative to the image, the next Prepare call do it.

bool
cScr_QuestArrow::Prepare ()
{
	if (!enabled) return false;

	SService<IObjectSrv> pOS (g_pScriptManager);
	cScrVec op; pOS->Position (op, ObjId ());
	int ox, oy; bool onscreen = LocationToScreen (op, ox, oy);
	if (!onscreen) return false;

	int cw, ch; GetCanvasSize (cw, ch);

	int iw, ih;
	if (image == IMAGE_BITMAP)
		GetBitmapSize (bitmap, iw, ih);
	else
		iw = ih = SYMBOL_SIZE;

	//FIXME Consider text size for both position and size. (How?)
	//FIXME Schedule redraw if text must move.

	int ex = ox - iw / 2, ey = oy - ih / 2,
		ew = iw, eh = ih;
	SetPosition (ex, ey);
	if (NeedsRedraw ()) SetSize (ew, eh); //FIXME Not working.

	return true;
}

const int
cScr_QuestArrow::SYMBOL_SIZE = 48;

void
cScr_QuestArrow::Redraw ()
{
	SetDrawingColor (color);
	int qtr = SYMBOL_SIZE / 4;

	//FIXME Draw at actual position.
	switch (image)
	{
	case IMAGE_ARROW:
		DrawLine (0, 0, 0, qtr);
		DrawLine (0, 0, qtr, 0);
		DrawLine (0, 0, 4*qtr, 4*qtr);
		break;
	case IMAGE_CROSSHAIRS:
		DrawLine (2*qtr, 0, 2*qtr, 4*qtr);
		DrawLine (0, 2*qtr, 4*qtr, 2*qtr);
		DrawLine (qtr, qtr, 3*qtr, qtr);
		DrawLine (qtr, qtr, qtr, 3*qtr);
		DrawLine (3*qtr, qtr, 3*qtr, 3*qtr);
		DrawLine (qtr, 3*qtr, 3*qtr, 3*qtr);
		break;
	case IMAGE_BITMAP:
		DrawBitmap (bitmap, 0, 0);
		break;
	case IMAGE_NONE:
	default:
		break;
	}

	//FIXME Draw the text.
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

	UpdateOpacity ();

	try
	{
		SService<IDarkHookScriptService> pDHS (g_pScriptManager);
		pDHS->InstallPropHook (ObjId (), kDHNotifyDefault,
			"DesignNote", ObjId ());
		pDHS->InstallPropHook (ObjId (), kDHNotifyDefault,
			"GameName", ObjId ()); // if quest_arrow_text == "@name"
	}
	catch (no_interface&)
	{
		DebugString ("The DarkHook service could not be located. "
			"The quest arrow may not update properly.");
	}

	return result;
}

long
cScr_QuestArrow::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "QuestArrowOn"))
	{
		enabled = true;
		return 0;
	}
	else
	if (!stricmp (pMsg->message, "QuestArrowOff") ||
	    !stricmp (pMsg->message, "Slain") ||
	    (!stricmp (pMsg->message, "AIModeChange") &&
	        static_cast<sAIModeChangeMsg*> (pMsg)->mode == kAIM_Dead))
	{
		enabled = false;
		return 0;
	}
	else
	if (!stricmp (pMsg->message, "DHNotify"))
		return OnDHNotify (static_cast<sDHNotifyMsg*> (pMsg), mpReply);

	return cScr_HUDElement::OnMessage (pMsg, mpReply);
}

long
cScr_QuestArrow::OnDHNotify (sDHNotifyMsg* pMsg, cMultiParm&)
{
	if (pMsg->typeDH != kDH_Property) return 1;
	bool params = !strcmp (pMsg->sProp.pszPropName, "DesignNote");

	if (params)
	{
		UpdateObjective ();
		UpdateImage ();
		UpdateColor ();
		UpdateOpacity ();
	}

	if (params || !strcmp (pMsg->sProp.pszPropName, "GameName"))
		UpdateText ();

	return 0;
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
	if (bitmap > -1)
	{
		FreeBitmap (bitmap);
		bitmap = -1;
	}

	char* _image = GetParamString ("quest_arrow_image", "@crosshairs");

	if (!_image || !stricmp (_image, "@none"))
		image = IMAGE_NONE;

	else if (!stricmp (_image, "@arrow"))
		image = IMAGE_ARROW;

	else if (!stricmp (_image, "@crosshairs"))
		image = IMAGE_CROSSHAIRS;

	else if (_image[0] == '@')
	{
		DebugPrintf ("Warning: invalid quest_arrow_image value of `%s'.", _image);
		image = IMAGE_NONE;
	}

	else
	{
		bitmap = LoadBitmap (_image);
		if (bitmap == -1)
		{
			DebugPrintf ("Warning: could not load quest_arrow_image at `%s'.", _image);
			image = IMAGE_NONE;
		}
		else
			image = IMAGE_CROSSHAIRS;
	}

	if (_image) g_pMalloc->Free (_image);
	ScheduleRedraw ();
}

void
cScr_QuestArrow::UpdateText ()
{
	char* _text = GetParamString ("quest_arrow_text", "@name");
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr msgstr;

	if (!_text || !stricmp (_text, "") || !stricmp (_text, "@none"))
		text.Empty ();

	else if (!stricmp (_text, "@name"))
	{
		pDS->GetObjString (msgstr, ObjId (), "objnames");
		text = msgstr;
	}

	else if (!stricmp (_text, "@desc"))
	{
		pDS->GetObjString (msgstr, ObjId (), "objdescs");
		text = msgstr;
	}

	else if (!stricmp (_text, "@goal"))
	{
		GetTextFromObjective (msgstr);
		text = msgstr;
	}

	else if (_text[0] == '@')
	{
		DebugPrintf ("Warning: invalid quest_arrow_text value of `%s'.", _text);
		text.Empty ();
	}

	else
	{
		pDS->GetString (msgstr, "hud", _text, "", "strings");
		text = msgstr;
	}

	if (_text) g_pMalloc->Free (_text);
	ScheduleRedraw ();
}

void
cScr_QuestArrow::UpdateColor ()
{
	color = GetParamColor ("quest_arrow_color", makecolor (255, 255, 255));
	ScheduleRedraw ();
}

void
cScr_QuestArrow::UpdateOpacity ()
{
	int opacity = GetParamInt ("quest_arrow_opacity", 255);
	if (opacity == -1) opacity = 127; //FIXME Support distance-based opacity. Update when needed.
	SetOpacity (opacity);
}

