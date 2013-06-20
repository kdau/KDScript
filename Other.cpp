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
#include <ScriptLib.h>
#include "utils.h"



/* KDShortText */

cScr_ShortText::cScr_ShortText (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_ShortText::OnFrobWorldEnd (sFrobMsg*, cMultiParm&)
{
	if (GetObjectParamBool (ObjId (), "on_frob", true))
		DisplayMessage ();
	return 0;
}

long
cScr_ShortText::OnWorldSelect (sScrMsg*, cMultiParm&)
{
	if (GetObjectParamBool (ObjId (), "on_focus", true))
		DisplayMessage ();
	return 0;
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
		GetObjectParamTime (ObjId (), "time", CalcTextTime (msgstr)),
		GetObjectParamColor (ObjId (), "color", 0));
}

