/******************************************************************************
 *  KDCarried.cpp
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDCarried.h"
#include <ScriptLib.h>
#include "utils.h"

cScr_Carried::cScr_Carried (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_Carried::OnSim (sSimMsg* pMsg, cMultiParm&)
{
	// add FrobInert if requested
	if (pMsg->fStarting &&
	    GetObjectParamBool (ObjId (), "inert_until_dropped", false))
		AddSingleMetaProperty ("FrobInert", ObjId ());

	return S_OK;
}

long
cScr_Carried::OnCreate (sScrMsg*, cMultiParm&)
{
	// only proceed for objects created in-game
	SService<IVersionSrv> pVS (g_pScriptManager);
	if (pVS->IsEditor () == 1) return S_FALSE;

	// don't affect any dropped clone
	if (GetObjectParamBool (ObjId (), "dropped", false))
		return S_FALSE;

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

	SetObjectParamBool (ObjId (), "dropped", true);

	cScrVec position; pOS->Position (position, ObjId ());
	cScrVec facing; pOS->Facing (facing, ObjId ());

	// remove FrobInert if requested
	if (GetObjectParamBool (ObjId (), "inert_until_dropped", false))
		RemoveSingleMetaProperty ("FrobInert", ObjId ());

	// turn off the object and others if requested
	if (GetObjectParamBool (ObjId (), "off_when_dropped", false))
	{
		SimpleSend (ObjId (), ObjId (), "TurnOff");
		CDSend ("TurnOff", ObjId ());
	}

#if (_DARKGAME == 2)
	// confirm that object would not drop out of world
	true_bool position_valid;
	pOS->IsPositionValid (position_valid, ObjId ());
	if (!position_valid)
	{
		DebugPrintf ("Not dropping from invalid position (%f,%f,%f).",
			position.x, position.y, position.z);
		return;
	}
#endif // _DARKGAME == 2

	object drop = ObjId ();

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

		// remove clone's FrobInert if requested (yes, this is needed)
		if (GetObjectParamBool (drop, "inert_until_dropped", false))
			RemoveSingleMetaProperty ("FrobInert", drop);
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

