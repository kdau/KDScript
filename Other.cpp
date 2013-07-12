/******************************************************************************
 *  Other.cpp: all other scripts
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
#include <lg/objects.h>
#include <ScriptLib.h>



/* KDCarried */

cScr_Carried::cScr_Carried (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_Carried::OnSim (sSimMsg*, cMultiParm&)
{
	// add FrobInert if requested
	if (GetObjectParamBool (ObjId (), "inert_until_dropped", false))
		AddSingleMetaProperty ("FrobInert", ObjId ());

	return S_OK;
}

long
cScr_Carried::OnCreate (sScrMsg*, cMultiParm&)
{
	// only proceed for objects created in-game
	SService<IVersionSrv> pVS (g_pScriptManager);
	if (pVS->IsEditor () == 1) return S_FALSE;

	// add FrobInert if requested
	if (GetObjectParamBool (ObjId (), "inert_until_dropped", false))
		AddSingleMetaProperty ("FrobInert", ObjId ());

	return S_OK;
}

long
cScr_Carried::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!stricmp (pMsg->message, "Drop") ||
	    !stricmp (pMsg->message, "CarrierBrainDead"))
	{
		Drop ();
		return S_OK;
	}

	if (!stricmp (pMsg->message, "CarrierAlerted") &&
	    pMsg->data.type == kMT_Int) // new alert level
	{
		int new_alert = int (pMsg->data), min_alert =
			GetObjectParamInt (ObjId (), "drop_on_alert", kNoAlert);
		if (min_alert > kNoAlert && min_alert <= new_alert)
		{
			Drop ();
			return S_OK;
		}
		else
			return S_FALSE;
	}

	if (!stricmp (pMsg->message, "FixPhysics"))
	{
		FixPhysics ();
		return S_OK;
	}

	return cBaseScript::OnMessage (pMsg, mpReply);
}

void
cScr_Carried::Drop ()
{
	SService<IObjectSrv> pOS (g_pScriptManager);
	SService<IPropertySrv> pPS (g_pScriptManager);
	SService<IPhysSrv> pPhS (g_pScriptManager);
	SService<IQuestSrv> pQS (g_pScriptManager);

	object drop = ObjId ();
	cScrVec position; pOS->Position (position, drop);
	cScrVec facing; pOS->Facing (facing, drop);

	// remove FrobInert if requested
	if (GetObjectParamBool (drop, "inert_until_dropped", false))
		RemoveSingleMetaProperty ("FrobInert", drop);

	// turn off the object and others if requested
	if (GetObjectParamBool (drop, "off_when_dropped", false))
	{
		SimpleSend (drop, drop, "TurnOff");
		CDSend ("TurnOff", drop);
	}

#if (_DARKGAME == 2)
	// confirm that object would not drop out of world
	true_bool position_valid;
	pOS->IsPositionValid (position_valid, drop);
	if (!position_valid)
	{
		DebugPrintf ("Not dropping %s at invalid position (%f,%f,%f).",
			(const char*) FormatObjectName (drop),
			position.x, position.y, position.z);
		return;
	}
#endif

	for (LinkIter container (Any, ObjId (), "Contains");
	     container; ++container)
	{
		switch (*(const int*) container.GetData ())
		{
		case kContainTypeAlternate:
		case kContainTypeHand:
		case kContainTypeBelt:
			// decrease pickable pocket count
			pQS->Set ("DrSPocketCnt",
				pQS->Get ("DrSPocketCnt") - 1,
				kQuestDataMission);
			break;
		case kContainTypeGeneric:
		default:
			// generic contents don't count as pickable pockets
			break;
		}

		// unlink from Contains carrier
		DestroyLink (container);
	}

	for (LinkIter creature (Any, ObjId (), "CreatureAttachment");
	     creature; ++creature)
		// unlink from CreatureAttachment carrier
		DestroyLink (creature);

	LinkIter dattach (ObjId (), Any, "DetailAttachement");
	if (dattach)
	{
		// unlink from DetailAttachement carrier, destroying this object
		object ai = dattach.Destination ();
		DestroyLink (dattach);

		// clone self and add reference link to AI
		pOS->Create (drop, ObjId ());
		CreateLink ("CulpableFor", ai, drop);
		DebugPrintf ("Replacing self with clone %d.", int (drop));
	}

	// ensure that the object is physical
	if (!pPS->Possessed (drop, "PhysType"))
	{
		// create an OBB model to allow check of object dimensions
		pPS->Add (drop, "PhysType");
		pPS->Set (drop, "PhysType", "Type", kPMT_OBB);

		// schedule to switch to correctly sized sphere
		SimplePost (drop, drop, "FixPhysics");
	}

	// teleport object to own position to avoid winding up at origin (?!)
	pOS->Teleport (drop, position, facing, None);

	// give object a push to cause it to drop
	pPhS->SetVelocity (drop, {0.0, 0.0, -0.1});
}

void
cScr_Carried::FixPhysics ()
{
	// get object dimensions
	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm _dims;
	pPS->Get (_dims, ObjId (), "PhysDims", "Size");

	// switch to sphere model and set appropriate radius
	pPS->Set (ObjId (), "PhysType", "Type", kPMT_Sphere);
	pPS->Set (ObjId (), "PhysType", "# Submodels", 1);
	if (_dims.type == kMT_Vector)
	{
		cScrVec dims = cScrVec (_dims);
		float radius =
			std::max (dims.x, std::max (dims.y, dims.z)) / 2.0;
		pPS->Set (ObjId (), "PhysDims", "Radius 1", radius);
	}
}



/* KDCarrier */

cScr_Carrier::cScr_Carrier (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseAIScript (pszName, iHostObjId)
{}

long
cScr_Carrier::OnSim (sSimMsg* pMsg, cMultiParm&)
{
	if (pMsg->fStarting) CreateAttachments ();
	return S_OK;
}

long
cScr_Carrier::OnCreate (sScrMsg*, cMultiParm&)
{
	CreateAttachments ();
	return S_OK;
}

long
cScr_Carrier::OnAIModeChange (sAIModeChangeMsg* pMsg, cMultiParm&)
{
	if (pMsg->mode == kAIM_Dead) // slain or knocked out
		NotifyCarried ("CarrierBrainDead");
	return S_OK;
}

long
cScr_Carrier::OnAlertness (sAIAlertnessMsg* pMsg, cMultiParm&)
{
	if (pMsg->level > kNoAlert)
		NotifyCarried ("CarrierAlerted", pMsg->level);
	return S_OK;
}

void
cScr_Carrier::NotifyCarried (const char* message, const cMultiParm& data)
{
	// notify Contains objects
	for (LinkIter contents (ObjId (), Any, "Contains");
	     contents; ++contents)
		SimpleSend (ObjId (), contents.Destination (),
			message, data);

	// notify CreatureAttachment objects
	for (LinkIter attachment (ObjId (), Any, "CreatureAttachment");
	     attachment; ++attachment)
		SimpleSend (ObjId (), attachment.Destination (),
			message, data);

	// notify ~DetailAttachement objects
	for (LinkIter attachment (Any, ObjId (), "DetailAttachement");
	     attachment; ++attachment)
		SimpleSend (ObjId (), attachment.Source (),
			message, data);
}

void
cScr_Carrier::CreateAttachments ()
{
	if (!GetObjectParamBool (ObjId (), "create_attachments", true)) return;

	SInterface<ITraitManager> pTM (g_pScriptManager);
	SInterface<IObjectQuery> tree;

	tree = pTM->Query (ObjId (), kTraitQueryMetaProps | kTraitQueryFull);
	if (!tree) return;

	for (; ! tree->Done (); tree->Next ())
		for (LinkIter archetypes (tree->Object (), Any,
			"CreatureAttachment"); archetypes; ++archetypes)
		{
			cMultiParm joint;
			archetypes.GetDataField ("Joint", joint);
			CreateAttachment (archetypes.Destination (), joint);
		}
}

void
cScr_Carrier::CreateAttachment (object archetype, int joint)
{
	SService<IObjectSrv> pOS (g_pScriptManager);
	SService<ILinkToolsSrv> pLTS (g_pScriptManager);

	if (GetOneLinkByDataDest ("CreatureAttachment", ObjId (),
			&joint, sizeof (joint)) != None)
		return; // don't attach a second object to the same joint

	DebugPrintf ("Attaching a new %s to joint %d.",
		(const char*) FormatObjectName (archetype), joint);

	object attachment;
	pOS->Create (attachment, archetype);
	if (!attachment) return;

	link attach_link =
		CreateLink ("CreatureAttachment", ObjId (), attachment);
	if (attach_link)
		pLTS->LinkSetData (attach_link, "Joint", joint);
}



/* KDGetInfo */

cScr_GetInfo::cScr_GetInfo (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_GetInfo::OnBeginScript (sScrMsg*, cMultiParm&)
{
	// not everything is available yet, so wait for next message cycle
	SimplePost (ObjId (), ObjId (), "UpdateVariables");
	return S_OK;
}

long
cScr_GetInfo::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->message, "UpdateVariables"))
	{
		UpdateVariables ();
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_GetInfo::OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg, cMultiParm&)
{
	if (pMsg->fResuming)
		UpdateVariables ();
	return S_OK;
}

long
cScr_GetInfo::OnEndScript (sScrMsg*, cMultiParm&)
{
	DeleteVariables ();
	return S_OK;
}

void
cScr_GetInfo::UpdateVariables ()
{
	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	SService<IEngineSrv> pES (g_pScriptManager);
	SService<IQuestSrv> pQS (g_pScriptManager);
	SService<IVersionSrv> pVS (g_pScriptManager);

	pQS->Set ("info_directx_version", pES->IsRunningDX6 () ? 6 : 9,
		kQuestDataMission);

	int display_height = 0, display_width = 0;
	pES->GetCanvasSize (display_width, display_height);
	pQS->Set ("info_display_height", display_height, kQuestDataMission);
	pQS->Set ("info_display_width", display_width, kQuestDataMission);

	int value = 0;
	if (pES->ConfigGetInt ("sfx_eax", value))
		pQS->Set ("info_has_eax", value, kQuestDataMission);
	if (pES->ConfigGetInt ("fogging", value))
		pQS->Set ("info_has_fog", value, kQuestDataMission);
	if (pES->ConfigGetInt ("game_hardware", value))
		pQS->Set ("info_has_hw3d", value, kQuestDataMission);
	if (pES->ConfigGetInt ("enhanced_sky", value))
		pQS->Set ("info_has_sky", value, kQuestDataMission);
	if (pES->ConfigGetInt ("render_weather", value))
		pQS->Set ("info_has_weather", value, kQuestDataMission);

#if (_DARKGAME == 2)
	pQS->Set ("info_mission", pDGS->GetCurrentMission (), kQuestDataMission);
#endif

	pQS->Set ("info_mode", pVS->IsEditor (), kQuestDataMission);

	int version_major = 0, version_minor = 0;
	pVS->GetVersion (version_major, version_minor);
	pQS->Set ("info_version_major", version_major, kQuestDataMission);
	pQS->Set ("info_version_minor", version_minor, kQuestDataMission);
}

void
cScr_GetInfo::DeleteVariables ()
{
	SService<IQuestSrv> pQS (g_pScriptManager);
	pQS->Delete ("info_directx_version");
	pQS->Delete ("info_display_height");
	pQS->Delete ("info_display_width");
	pQS->Delete ("info_has_eax");
	pQS->Delete ("info_has_fog");
	pQS->Delete ("info_has_hw3d");
	pQS->Delete ("info_has_sky");
	pQS->Delete ("info_has_weather");
	pQS->Delete ("info_mission");
	pQS->Delete ("info_mode");
	pQS->Delete ("info_version_major");
	pQS->Delete ("info_version_minor");
}



/* KDJunkTool */

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
		SService<IDebugScrSrv> pDbS (g_pScriptManager);
		pDbS->Command ("use_item", "0", "", "", "", "", "", "");
		return S_OK;
	}

	if (!strcmp (pMsg->name, "ClearWeapon") && GetAvatarContainer ())
	{
		SService<IDebugScrSrv> pDbS (g_pScriptManager);
		pDbS->Command ("clear_weapon", "", "", "", "", "", "", "");
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
		SService<IDebugScrSrv> pDbS (g_pScriptManager);
		pDbS->Command ("drop_item", "", "", "", "", "", "", "");
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



/* KDTrapNextMission */

cScr_TrapNextMission::cScr_TrapNextMission (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId)
{}

long
cScr_TrapNextMission::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	int next_mission = GetObjectParamInt (ObjId (),
		bState ? "next_mission_on" : "next_mission_off", 0);

	if (next_mission < 1) return S_FALSE;

#if (_DARKGAME == 2)
	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	pDGS->SetNextMission (next_mission);
	return S_OK;
#else
	DebugPrintf ("Error: This script is not available for this game.");
	return S_FALSE;
#endif
}

