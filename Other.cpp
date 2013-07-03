/******************************************************************************
 *  Other.cpp: miscellaneous useful scripts
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

#include "Other.h"
#include <lg/sound.h>
#include <ScriptLib.h>
#include <darkhook.h>
#include "utils.h"



/* KDShortText */

cScr_ShortText::cScr_ShortText (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_ShortText::OnFrobWorldEnd (sFrobMsg*, cMultiParm&)
{
	if (GetObjectParamBool (ObjId (), "text_on_frob", true))
		DisplayMessage ();
	return S_OK;
}

long
cScr_ShortText::OnWorldSelect (sScrMsg*, cMultiParm&)
{
	if (GetObjectParamBool (ObjId (), "text_on_focus", true))
		DisplayMessage ();
	return S_OK;
}

void
cScr_ShortText::DisplayMessage ()
{
	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm msgid;
	if (pPS->Possessed (ObjId (), "Book"))
		pPS->Get (msgid, ObjId (), "Book", NULL);
	else if (char* _msgid = GetObjectParamString (ObjId (), "text"))
	{
		msgid = _msgid;
		g_pMalloc->Free (_msgid);
	}

	if (msgid.type != kMT_String) return;

	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr msgstr;
	pDS->GetString (msgstr, "short", msgid, "", "strings");
	if (msgstr.IsEmpty ()) return;

	ShowString (msgstr,
		GetObjectParamTime (ObjId (), "text_time", CalcTextTime (msgstr)),
		GetObjectParamColor (ObjId (), "text_color", 0));
}



/* KDSubtitled */

cScr_Subtitled::cScr_Subtitled (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  SCRIPT_VAROBJ (Subtitled, last_host, iHostObjId),
	  SCRIPT_VAROBJ (Subtitled, last_schema, iHostObjId)
{}

long
cScr_Subtitled::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "Subtitle") &&
	    pMsg->data.type == kMT_Int) // schema
	{
		object host = (pMsg->data2.type == kMT_Int)
			? int (pMsg->data2) : pMsg->from;
		Subtitle (host, int (pMsg->data));
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

void
cScr_Subtitled::Subtitle (object host, object schema)
{
	// confirm host and schema objects are valid
	if (!ObjectExists (host) || !ObjectExists (schema) ||
	    !InheritsFrom ("Schema", schema))
	{
		DebugPrintf ("Warning: can't subtitle invalid host/schema pair "
			"%d/%d.", int (host), int (schema));
		return;
	}

	// get subtitle text
	SService<IDataSrv> pDS (g_pScriptManager);
	cScrStr text;
	pDS->GetString (text, "subtitles", ObjectToStr (schema), "", "strings");
	if (text.IsEmpty ()) return;

	// get or calculate schema duration
	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm _duration;
	if (pPS->Possessed (schema, "ScriptTiming"))
		pPS->Get (_duration, schema, "ScriptTiming", NULL);
	int duration = (_duration.type == kMT_Int)
		? int (_duration) : CalcTextTime (text, 750);

	// get subtitle color
	ulong color = GetObjectParamColor (schema, "subtitle_color",
		GetObjectParamColor (host, "subtitle_color", 0xffffff));

	// display subtitle - FIXME Optionally use HUD for prettiness. Schedule EndSubtitle if so.
	DEBUG_PRINTF ("subtitling schema %s",
		(const char*) FormatObjectName (schema));
	last_host = host;
	last_schema = schema;
	ShowString (text, duration, color);
}

void
cScr_Subtitled::EndSubtitle (object host, object schema)
{
	// confirm that this is the most recent subtitle
	if (last_host != host || last_schema != schema) return;

	//FIXME Clear HUD subtitle, if any.
}



/* KDSubtitledAI */

cScr_SubtitledAI::cScr_SubtitledAI (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_Subtitled (pszName, iHostObjId)
{
	DarkHookInitializeService (g_pScriptManager, g_pMalloc);
}

long
cScr_SubtitledAI::OnBeginScript (sScrMsg*, cMultiParm&)
{
	try
	{
		SService<IDarkHookScriptService> pDHS (g_pScriptManager);
		pDHS->InstallPropHook (ObjId (), kDHNotifyDefault,
			"Speech", ObjId ());
	}
	catch (no_interface&)
	{
		DebugString ("The DarkHook service could not be located. "
			"This AI's speech will not be subtitled.");
		return S_FALSE;
	}
	return S_OK;
}

long
cScr_SubtitledAI::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!!stricmp (pMsg->message, "DHNotify"))
		return cBaseScript::OnMessage (pMsg, mpReply);

	auto dh = static_cast<sDHNotifyMsg*> (pMsg);
	if (dh->typeDH != kDH_Property ||
	    !!strcmp (dh->sProp.pszPropName, "Speech"))
		return S_FALSE;

	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm schema, flags;

	pPS->Get (schema, ObjId (), "Speech", "schemaID");
	if (schema.type != kMT_Int || int (schema) == 0)
		return S_FALSE; // not a valid schema

	pPS->Get (flags, ObjId (), "Speech", "flags");
	if (flags.type != kMT_Int || int (flags) != 1)
		return S_FALSE; // not a start-of-schema change

	//FIXME Confirm in earshot.

	// display the subtitle
	Subtitle (ObjId (), int (schema));
	return S_OK;
}



/* KDSubtitledVO */

cScr_SubtitledVO::cScr_SubtitledVO (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cScr_Subtitled (pszName, iHostObjId),
	  SCRIPT_VAROBJ (SubtitledVO, played, iHostObjId)
{}

long
cScr_SubtitledVO::OnTurnOn (sScrMsg*, cMultiParm&)
{
	// get one SoundDescription-linked schema
	object schema =
		LinkIter (ObjId (), 0, "SoundDescription").Destination ();
	if (!schema) return S_FALSE;

	// only display subtitle once (per object)
	if (played.Valid () && bool (played))
		return S_FALSE;
	else
		played = true;

	// display the subtitle
	Subtitle (StrToObject ("Player"), schema);
	return S_OK;
}

