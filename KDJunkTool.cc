/******************************************************************************
 *  KDJunkTool.cpp
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

#include "KDJunkTool.h"
#include <ScriptLib.h>
#include "utils.h"

cScr_JunkTool::cScr_JunkTool (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_JunkTool::OnContained (sContainedScrMsg* pMsg, cMultiParm&)
{
	if (!InheritsFrom ("Avatar", pMsg->container))
		return S_FALSE;

	switch (pMsg->event)
	{
	case kContainAdd:
		StartJunk (pMsg->container);
		return S_OK;
	case kContainRemove:
		EndJunk (pMsg->container);
		return S_OK;
	default:
		return S_FALSE;
	}
}

long
cScr_JunkTool::OnInvDeSelect (sScrMsg*, cMultiParm&)
{
	// reselect the tool in the next cycle; won't work in this one
	SimplePost (ObjId (), ObjId (), "ReselectMe");
	return S_OK;
}

long
cScr_JunkTool::OnInvDeFocus (sScrMsg*, cMultiParm&)
{
	SetTimedMessage ("BeginToolUse", 1, kSTM_OneShot);
	return S_OK;
}

long
cScr_JunkTool::OnFrobInvEnd (sFrobMsg*, cMultiParm&)
{
	// resume tool use
	SimplePost (ObjId (), ObjId (), "ReselectMe");
	return S_OK;
}

long
cScr_JunkTool::OnFrobToolEnd (sFrobMsg*, cMultiParm&)
{
	// resume tool use
	SimplePost (ObjId (), ObjId (), "ReselectMe");
	return S_OK;
}

long
cScr_JunkTool::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->message, "ReselectMe"))
	{
		object avatar = GetAvatarContainer ();
		if (!avatar) return S_FALSE;

		// (re)select the object
		SService<IDarkUISrv> pDUIS (g_pScriptManager);
		pDUIS->InvSelect (ObjId ());

		// create a fake frobbable and schedule to begin tool use
		CreateFrobbable (avatar);
		SetTimedMessage ("BeginToolUse", 50, kSTM_OneShot);
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_JunkTool::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "BeginToolUse"))
	{
		ExecuteCommand ("use_item 0");
		return S_OK;
	}

	if (!strcmp (pMsg->name, "ClearWeapon") && GetAvatarContainer ())
	{
		ExecuteCommand ("clear_weapon");
		SetTimedMessage ("ClearWeapon", 100, kSTM_OneShot);
		return S_OK;
	}

	return cBaseScript::OnTimer (pMsg, mpReply);
}

long
cScr_JunkTool::OnSlain (sSlayMsg*, cMultiParm&)
{
	object avatar = GetAvatarContainer ();
	if (avatar && GetObjectParamBool (ObjId (), "junk_tool_drop", false))
	{
		// forget the slaying so we can drop again later
		SService<IPropertySrv> pPS (g_pScriptManager);
		if (pPS->Possessed (ObjId (), "DeathStage"))
			pPS->Remove (ObjId (), "DeathStage");

		// drop the tool
		ExecuteCommand ("drop_item");
		return S_OK;
	}
	return S_FALSE;
}

long
cScr_JunkTool::OnDestroy (sScrMsg*, cMultiParm&)
{
	if (object avatar = GetAvatarContainer ())
		EndJunk (avatar);
	return S_OK;
}

object
cScr_JunkTool::GetAvatarContainer ()
{
	LinkIter container (Any, ObjId (), "Contains");
	return (container && InheritsFrom ("Avatar", container.Source ()))
		? container.Source () : None;
}

void
cScr_JunkTool::StartJunk (object avatar)
{
	// keep clearing any weapon while we are in inventory
	SetTimedMessage ("ClearWeapon", 1, kSTM_OneShot);

	// for lugged tool, slow down player and play lifting grunt
	if (GetObjectParamBool (ObjId (), "junk_tool_lugged", true))
	{
		SService<IDarkInvSrv> pDIS (g_pScriptManager);
		pDIS->AddSpeedControl ("JunkTool", 0.6, 0.6);
		PlayVoiceOver (avatar, StrToObject ("garlift"));
	}

	// select the object and begin tool use
	SimplePost (ObjId (), ObjId (), "ReselectMe");
}

void
cScr_JunkTool::EndJunk (object avatar)
{
	// for lugged tool, restore player speed and play dropping grunt
	if (GetObjectParamBool (ObjId (), "junk_tool_lugged", true))
	{
		SService<IDarkInvSrv> pDIS (g_pScriptManager);
		pDIS->RemoveSpeedControl ("JunkTool");
		PlayVoiceOver (avatar, StrToObject ("gardrop"));
	}
}

object
cScr_JunkTool::CreateFrobbable (object avatar)
{
	// The "use_item 0" command (to begin tool use) sometimes only works 
	// when there is a frobbable item currently focused in the world. (I
	// have no idea when, or why it's variable.) To make the command always
	// be effective, this method creates a fake frobbable object in front of
	// the player that will destroy itself instantly, temporarily creating
	// the conditions required by "use_item 0".

	SService<IObjectSrv> pOS (g_pScriptManager);
	SService<IPropertySrv> pPS (g_pScriptManager);

	// create the fake frobbable object
	object frobbable;
	pOS->Create (frobbable, StrToObject ("Marker"));

	// make it frobbable
	pPS->Add (frobbable, "FrobInfo");
	pPS->Set (frobbable, "FrobInfo", "World Action", 2); // Script

	// fill the player's view
	pOS->Teleport (frobbable, { 2.0, 0.0, 0.0 }, cScrVec (), avatar);
	pPS->Add (frobbable, "Scale");
	pPS->SetSimple (frobbable, "Scale", cScrVec (10.0, 10.0, 20.0));

	// make it technically but not actually visible
	pPS->Add (frobbable, "RenderType");
	pPS->SetSimple (frobbable, "RenderType", 0); // Normal
	pPS->Add (frobbable, "RenderAlpha");
	pPS->SetSimple (frobbable, "RenderAlpha", 0.0);

	// ensure it will be destroyed
	pOS->SetTransience (frobbable, true);
	pPS->Add (frobbable, "CfgTweqDelete");
	pPS->Set (frobbable, "CfgTweqDelete", "Halt", 0); // Destroy Obj
	pPS->Set (frobbable, "CfgTweqDelete", "AnimC", 2); // Sim
	pPS->Set (frobbable, "CfgTweqDelete", "Rate", 100);
	pPS->Add (frobbable, "StTweqDelete");
	pPS->Set (frobbable, "StTweqDelete", "AnimS", 1); // On

	return frobbable;
}

