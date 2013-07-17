/******************************************************************************
 *  KDSubtitled.cpp: HUDSubtitle, KDSubtitled, KDSubtitledAI, KDSubtitledVO
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

#include "KDSubtitled.h"
#include <ScriptLib.h>
#include <darkhook.h>
#include "utils.h"



/* HUDSubtitle */

const int
HUDSubtitle::BORDER = 1;

const int
HUDSubtitle::PADDING = 8;

HUDSubtitle::HUDSubtitle (object host, object _schema,
		const char* _text, ulong _color)
	: HUDElement (host), schema (_schema), text (_text), color (_color),
	  player (host == StrToObject ("Player"))
{
	Initialize ();
}

HUDSubtitle::~HUDSubtitle ()
{}

object
HUDSubtitle::GetSchema ()
{
	return schema;
}

bool
HUDSubtitle::Prepare ()
{
	// get canvas and text size and calculate element size
	CanvasSize canvas = GetCanvasSize (),
		text_size = GetTextSize (text),
		elem_size;
	elem_size.w = BORDER + PADDING + text_size.w + PADDING + BORDER;
	elem_size.h = BORDER + PADDING + text_size.h + PADDING + BORDER;

	// get host's position in canvas coordinates
	CanvasPoint host_pos;
	if (player)
		host_pos = CanvasPoint (canvas.w / 2, canvas.h / 2);
	else
	{
		host_pos = ObjectCentroidToCanvas (GetHost ());
		if (!host_pos.Valid ()) return false;
	}

	// calculate element position
	CanvasPoint elem_pos;
	elem_pos.x = std::max (0, std::min (canvas.w - elem_size.w,
		host_pos.x - elem_size.w / 2));
	elem_pos.y = std::max (0, std::min (canvas.h - elem_size.h,
		host_pos.y + PADDING));

	SetPosition (elem_pos);
	SetSize (elem_size);
	return true;
}

void
HUDSubtitle::Redraw ()
{
	// draw background
	SetDrawingColor (0x000000);
	FillArea ();

	// draw border
	SetDrawingColor (color);
	DrawBox ();

	// draw text
	DrawText (text, CanvasPoint (BORDER+PADDING, BORDER+PADDING));
}



/* KDSubtitled */

const float
cScr_Subtitled::EARSHOT = 80.0;

cScr_Subtitled::cScr_Subtitled (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId), element (NULL)
{}

cScr_Subtitled::~cScr_Subtitled ()
{
	EndSubtitle (Any);
}

long
cScr_Subtitled::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "Subtitle"))
	{
		object host = (pMsg->data2.type == kMT_Int)
			? int (pMsg->data2) : pMsg->from;
		object schema = (pMsg->data.type == kMT_Int)
			? object (int (pMsg->data))
			: (pMsg->data.type == kMT_String)
			? StrToObject ((const char*) pMsg->data)
			: None;
		return Subtitle (host, schema) ? S_OK : S_FALSE;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_Subtitled::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "EndSubtitle") &&
	    pMsg->data.type == kMT_Int) // schema
	{
		EndSubtitle (int (pMsg->data));
		return S_OK;
	}
	return cBaseScript::OnTimer (pMsg, mpReply);
}

long
cScr_Subtitled::OnEndScript (sScrMsg*, cMultiParm&)
{
	EndSubtitle (Any);
	return S_OK;
}

bool
cScr_Subtitled::Subtitle (object host, object schema)
{
	// confirm host and schema objects are valid
	if (!ObjectExists (host) || !ObjectExists (schema) ||
	    !InheritsFrom ("Schema", schema))
	{
		DebugPrintf ("Warning: Can't subtitle invalid host/schema pair "
			"%d/%d.", int (host), int (schema));
		return false;
	}

	// get subtitle text
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr text;
	pDS->GetString (text, "subtitles", ObjectToStr (schema), "", "strings");
	if (text.IsEmpty ()) return false;

	// end any previous subtitle on this object
	EndSubtitle (Any);

	// get or calculate schema duration
	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm _duration;
	if (pPS->Possessed (schema, "ScriptTiming"))
		pPS->Get (_duration, schema, "ScriptTiming", NULL);
	int duration = (_duration.type == kMT_Int)
		? int (_duration) : CalcTextTime (text, 700);

	// get subtitle color
	ulong color = GetObjectParamColor (schema, "subtitle_color",
		GetObjectParamColor (host, "subtitle_color", 0xffffff));

	try
	{
		// check whether to use HUD
		SService<IQuestSrv> pQS (g_pScriptManager);
		if (pQS->Get ("subtitles_use_hud") != 1)
			throw std::runtime_error ("nevermind");

		// create a HUD subtitle element
		element = new HUDSubtitle (host, schema, text, color);

		// schedule its deletion
		SetTimedMessage ("EndSubtitle", duration, kSTM_OneShot, schema);
	}
	catch (...) // go the old-fashioned way
	{
		element = NULL;
		ShowString (text, duration, color);
	}

	return true;
}

void
cScr_Subtitled::EndSubtitle (object schema)
{
	// only applicable to HUD elements
	if (!element) return;

	// try to prevent early end of later subtitle
	if (schema && element->GetSchema () != schema) return;

	// destroy HUD element
	delete element;
	element = NULL;
}



/* KDSubtitledAI */

cScr_SubtitledAI::cScr_SubtitledAI (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_Subtitled (pszName, iHostObjId)
{
	DarkHookInitializeService (g_pScriptManager, g_pMalloc);
}

long
cScr_SubtitledAI::OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply)
{
	try
	{
		SService<IDarkHookScriptService> pDHS (g_pScriptManager);
		pDHS->InstallPropHook (ObjId (), kDHNotifyDefault,
			"Speech", ObjId ());
	}
	catch (no_interface&)
	{
		DebugString ("Error: The DarkHook service could not be "
			"located. This AI's speech will not be subtitled.");
	}
	return cScr_Subtitled::OnBeginScript (pMsg, mpReply);
}

long
cScr_SubtitledAI::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	long result = cScr_Subtitled::OnMessage (pMsg, mpReply);
	if (!!stricmp (pMsg->message, "DHNotify")) return result;

	auto dh = static_cast<sDHNotifyMsg*> (pMsg);
	if (dh->typeDH != kDH_Property) return result;
	if (!!strcmp (dh->sProp.pszPropName, "Speech")) return result;

	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm schema, flags;

	pPS->Get (schema, ObjId (), "Speech", "schemaID");
	if (schema.type != kMT_Int || int (schema) == None)
		return result; // not a valid schema

	pPS->Get (flags, ObjId (), "Speech", "flags");
	if (flags.type != kMT_Int || int (flags) != 1)
		return result; // not a start-of-schema change

	// confirm speech is in (estimated) earshot of player
	SService<IObjectSrv> pOS (g_pScriptManager);
	cScrVec host_pos; pOS->Position (host_pos, ObjId ());
	cScrVec player_pos; pOS->Position (player_pos, StrToObject ("Player"));
	if (host_pos.Distance (player_pos) >= EARSHOT)
		return result; // too far away

	// display the subtitle
	Subtitle (ObjId (), int (schema));
	return S_OK;
}



/* KDSubtitledVO */

cScr_SubtitledVO::cScr_SubtitledVO (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_Subtitled (pszName, iHostObjId)
{}

long
cScr_SubtitledVO::OnTurnOn (sScrMsg*, cMultiParm&)
{
	// get one SoundDescription-linked schema
	object schema =
		LinkIter (ObjId (), Any, "SoundDescription").Destination ();
	if (!schema) return S_FALSE;

	// only display subtitle if schema hasn't been played
	// use unrelated StimKO property as placeholder
	SService<IPropertySrv> pPS (g_pScriptManager);
	if (pPS->Possessed (schema, "StimKO"))
		return S_FALSE;
	else
	{
		pPS->Add (schema, "StimKO");
		pPS->SetSimple (schema, "StimKO", true);
	}

	// display the subtitle
	Subtitle (StrToObject ("Player"), schema);
	return S_OK;
}

