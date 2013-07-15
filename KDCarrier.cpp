/******************************************************************************
 *  KDCarrier.cpp
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

#include "KDCarrier.h"
#include <lg/objects.h>
#include <ScriptLib.h>
#include "utils.h"

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

