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
#include "utils.h"



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
		int min_alert = GetObjectParamInt (ObjId (), "drop_on_alert", 0),
			new_alert = int (pMsg->data);
		if (min_alert > 0 && min_alert <= new_alert)
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

	for (LinkIter container (0, ObjId (), "Contains");
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

	for (LinkIter creature (0, ObjId (), "CreatureAttachment");
	     creature; ++creature)
		// unlink from CreatureAttachment carrier
		DestroyLink (creature);

	LinkIter dattach (ObjId (), 0, "DetailAttachement");
	if (dattach)
	{
		// unlink from DetailAttachement carrier, destroying this object
		object ai = dattach.Destination ();
		DestroyLink (dattach);

		// clone self and add reference link to AI
		pOS->Create (drop, ObjId ());
		CreateLink ("CulpableFor", ai, drop);
		DebugPrintf ("replacing self with clone %d", int (drop));
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
	pOS->Teleport (drop, position, facing, 0);

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
	if (pMsg->level > 0)
		NotifyCarried ("CarrierAlerted", pMsg->level);
	return S_OK;
}

void
cScr_Carrier::NotifyCarried (const char* message, const cMultiParm& data)
{
	// notify Contains objects
	for (LinkIter contained (ObjId (), 0, "Contains");
	     contained; ++contained)
		SimpleSend (ObjId (), contained.Destination (),
			message, data);

	// notify CreatureAttachment objects
	for (LinkIter attachment (ObjId (), 0, "CreatureAttachment");
	     attachment; ++attachment)
		SimpleSend (ObjId (), attachment.Destination (),
			message, data);

	// notify ~DetailAttachement objects
	for (LinkIter attachment (0, ObjId (), "DetailAttachement");
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
		for (LinkIter archetypes (tree->Object (), 0,
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

	if (GetOneLinkByData ("CreatureAttachment", ObjId (), 0,
			NULL, &joint, sizeof (joint)) != 0)
		return; // don't attach a second object to the same joint

	DebugPrintf ("attaching %s to joint %d",
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
	DebugPrintf ("Error: the KDTrapNextMission script is not available "
		"for this game.");
	return S_FALSE;
#endif
}

