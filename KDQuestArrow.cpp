/******************************************************************************
 *  KDQuestArrow.cpp
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

#include "KDQuestArrow.h"
#include <ScriptLib.h>
#include "utils.h"

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
cScr_QuestArrow::Initialize ()
{
	if (!cScr_HUDElement::Initialize ()) return false;

	enabled.Init (GetParamBool ("quest_arrow",
		!HasPlayerTouched (ObjId ())));

	OnPropertyChanged ("DesignNote"); // update all cached params

	SubscribeProperty ("DesignNote");
	SubscribeProperty ("GameName"); // for quest_arrow_text == "@name"

	return true;
}

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
cScr_QuestArrow::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "QuestArrowOn"))
	{
		enabled = true;
		return S_OK;
	}

	if (!stricmp (pMsg->message, "QuestArrowOff") ||
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
cScr_QuestArrow::OnSlain (sSlayMsg* pMsg, cMultiParm& mpReply)
{
	enabled = false;
	return cScr_HUDElement::OnSlain (pMsg, mpReply);
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
	bool incomplete = (pQS->Get (qvar) == kGoalIncomplete);

	snprintf (qvar, 256, "goal_visible_%d", objective);
	bool visible = (pQS->Get (qvar) == true);

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

