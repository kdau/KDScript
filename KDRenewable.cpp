/******************************************************************************
 *  KDRenewable.cpp
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

#include "KDRenewable.h"
#include <ScriptLib.h>
#include "utils.h"

const int
cScr_Renewable::DEFAULT_TIMING = 180; // seconds = three minutes

cScr_Renewable::cScr_Renewable (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_Renewable::OnSim (sSimMsg* pMsg, cMultiParm&)
{
	if (!pMsg->fStarting) return S_FALSE;

	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm _timing; pPS->Get (_timing, ObjId (), "ScriptTiming", NULL);
	ulong timing = 1000ul *
		((_timing.type == kMT_Int) ? int (_timing) : DEFAULT_TIMING);

	Renew ();
	SetTimedMessage ("Renew", timing, kSTM_Periodic);
	return S_OK;
}

long
cScr_Renewable::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "Renew"))
		return Renew () ? S_OK : S_FALSE;
	else
		return cBaseScript::OnTimer (pMsg, mpReply);
}

bool
cScr_Renewable::Renew ()
{
	SService<IObjectSrv> pOS (g_pScriptManager);
	SService<IPropertySrv> pPS (g_pScriptManager);

	// Check for a previously created instance.
	LinkIter previous (ObjId (), Any, "Owns");
	if (HasPlayerTouched (previous.Destination ()))
		DestroyLink (previous);
	else if (previous)
		return false;

	// Identify the resource archetype and stack count threshold.
	object archetype;
	int threshold = -1;
	for (LinkIter script_params (ObjId (), Any, "ScriptParams");
	     script_params; ++script_params)
	{
		const char* data = (const char*) script_params.GetData ();
		if (data && sscanf (data, "%d", &threshold) == 1)
		{
			archetype = script_params.Destination ();
			break;
		}
	}
	if (!archetype || threshold < 0)
		return false;

	// Transmogrify the archetype for the inventory check.
	object inv_type = archetype;
	sLink transmute;
	if (GetOneLinkInherited ("Transmute", archetype, Any, &transmute))
		inv_type = transmute.dest;
	else
	{
		cAnsiStr archetype_name = ObjectToStr (archetype);
		if (!strcmp (archetype_name, "EarthCrystal"))
			inv_type = StrToObject ("EarthArrow");
		else if (!strcmp (archetype_name, "WaterCrystal"))
			inv_type = StrToObject ("water");
		else if (!strcmp (archetype_name, "FireCrystal"))
			inv_type = StrToObject ("firearr");
		else if (!strcmp (archetype_name, "AirCrystal"))
			inv_type = StrToObject ("GasArrow");
	}

	// Check the inventory count.
	int inventory = 0;
	cMultiParm stack;
	for (LinkIter contains (StrToObject ("Player"), Any, "Contains");
	     contains; ++contains)
	{
		if (!InheritsFrom (inv_type, contains.Destination ())) continue;
		pPS->Get (stack, contains.Destination (), "StackCount", NULL);
		inventory += (stack.type == kMT_Int) ? int (stack) : 1;
	}
	if (inventory >= threshold)
		return false;

	// Create new instance.
	object instance;
	pOS->Create (instance, archetype);
	pOS->Teleport (instance, cScrVec (), cScrVec (), ObjId ());
	pPS->Remove (instance, "PhysType");
	pPS->Remove (instance, "PhysInitVel");
	CreateLink ("Owns", ObjId (), instance);

	return true;
}

