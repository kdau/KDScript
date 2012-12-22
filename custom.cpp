/******************************************************************************
 *  PublicScripts.cpp
 *
 *  This file is part of Public Scripts
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
#include "PublicScripts.h"
#include "ScriptModule.h"

#include <lg/interface.h>
#include <lg/scrmanagers.h>
#include <lg/scrservices.h>
#include <lg/objects.h>
#include <lg/links.h>
#include <lg/properties.h>
#include <lg/propdefs.h>
#include <lg/tools.h>
#include <ScriptLib.h>
#include "utils.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <list>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <memory>

using namespace std;


/***
 * TrapMetaProp
 */
struct MPTrap_params
{
	bool bTurnOn;
	object iDest;
	object iEffect;
	int iCount;
};

int cScr_MPTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	MPTrap_params* params = reinterpret_cast<MPTrap_params*>(pData);
	const char* pszLinkData = reinterpret_cast<const char*>(pLQ->Data());
	if (!pszLinkData || ((pszLinkData[0]|0x20) != 'a' && (pszLinkData[0]|0x20) != 'r'))
		return 1;

	sLink slDest;
	pLQ->Link(&slDest);
	int iMP;
	if (pszLinkData[1] == '@')
	{
		char szSrcData[16];
		snprintf(szSrcData, 16, "S%s", pszLinkData+2);
		iMP = GetOneLinkByDataDest("ScriptParams", slDest.source, szSrcData, -1);
	}
	else
		iMP = StrToObject(pszLinkData+1);

	if (iMP)
	{
		if (((pszLinkData[0] | 0x20) == 'a') ^ !params->bTurnOn)
			AddSingleMetaProperty(iMP, slDest.dest);
		else
			RemoveSingleMetaProperty(iMP, slDest.dest);
		params->iCount++;
	}

	return 1;
}

long cScr_MPTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	MPTrap_params data;
	data.bTurnOn = bTurnOn;
	data.iCount = 0;
	IterateLinks("ScriptParams", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&data));

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * ToolMP
 */
int cScr_MPTool::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	MPTrap_params* params = reinterpret_cast<MPTrap_params*>(pData);
	const char* pszLinkData = reinterpret_cast<const char*>(pLQ->Data());
	if (!pszLinkData || ((pszLinkData[0]|0x20) != 'a' && (pszLinkData[0]|0x20) != 'r'))
		return 1;

	sLink slDest;
	pLQ->Link(&slDest);

	if (params->iDest != slDest.dest)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		true_bool bInherits;
		pOS->InheritsFrom(bInherits, params->iDest, slDest.dest);
		if (!bInherits)
			return 1;
	}

	int iMP;
	if (pszLinkData[1] == '@')
	{
		char szSrcData[16];
		snprintf(szSrcData, 16, "S%s", pszLinkData+2);
		iMP = GetOneLinkByDataDest("ScriptParams", slDest.source, szSrcData, -1);
	}
	else
		iMP = StrToObject(pszLinkData+1);

	if (iMP)
	{
		if ((pszLinkData[0] | 0x20) == 'a')
			AddSingleMetaProperty(iMP, params->iEffect);
		else
			RemoveSingleMetaProperty(iMP, params->iEffect);
		params->iCount++;
	}

	return 1;
}

long cScr_MPTool::OnFrobToolEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	MPTrap_params data;
	data.iCount = 0;
	data.iEffect = data.iDest = pFrobMsg->DstObjId;
	char* pszParam = GetObjectParamString(ObjId(), "effect");
	if (pszParam)
	{
		if (!_stricmp(pszParam, "source"))
			data.iEffect = pFrobMsg->SrcObjId;
		g_pMalloc->Free(pszParam);
	}
	IterateLinks("ScriptParams", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&data));
	if (data.iCount)
	{
		PlayEnvSchema(ObjId(), "Event Activate", ObjId(), 0, kEnvSoundAtObjLoc);
		SService<IDamageSrv> pDmg(g_pScriptManager);
		pDmg->Slay(ObjId(), ObjId());
	}
	mpReply = 1;
	return cBaseScript::OnFrobToolEnd(pFrobMsg, mpReply);
}


/***
 * TrapScrMsgRelay
 */
struct SMRelay_data
{
	cMultiParm one;
	cMultiParm two;
	cMultiParm three;
};

int cScr_SMRelay::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_SMRelay* scrSMRelay = static_cast<cScr_SMRelay*>(pScript);
	SMRelay_data* data = reinterpret_cast<SMRelay_data*>(pData);
	sLink sl;
	pLQ->Link(&sl);
	char* msg = FixupScriptParamsHack(reinterpret_cast<const char*>(pLQ->Data()));
	if (msg)
	{
		scrSMRelay->PostMessage(sl.dest, msg, data->one, data->two, data->three);
		g_pMalloc->Free(msg);
	}
	return 1;
}

long cScr_SMRelay::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SMRelay_data data;

	int status_index = 1;
	int have_data = 0;
	char* pszParams = GetObjectParams(ObjId());
	if (pszParams)
	{
		char* pszData;

		pszData = GetParamString(pszParams, "data3");
		if (pszData)
		{
			have_data |= 1<<2;
			StringToMultiParm(data.three, pszData);
			g_pMalloc->Free(pszData);
		}
		pszData = GetParamString(pszParams, "data2");
		if (pszData)
		{
			have_data |= 1<<1;
			StringToMultiParm(data.two, pszData);
			g_pMalloc->Free(pszData);
		}
		pszData = GetParamString(pszParams, "data1");
		if (pszData)
		{
			have_data |= 1;
			StringToMultiParm(data.one, pszData);
			g_pMalloc->Free(pszData);
		}

		status_index = GetParamInt(pszParams, "status_index", 1);
		pszData = GetParamString(pszParams, "data");
		if (pszData)
		{
			if (status_index)
				have_data |= 1<<(status_index-1);
			if (!(have_data & 1))
				StringToMultiParm(data.one, pszData);
			else if (!(have_data & 2))
				StringToMultiParm(data.two, pszData);
			else if (!(have_data & 4))
				StringToMultiParm(data.three, pszData);
			g_pMalloc->Free(pszData);
		}

		g_pMalloc->Free(pszParams);
	}
	switch (status_index)
	{
	case 1:
		data.one = int(bTurnOn);
		break;
	case 2:
		data.two = int(bTurnOn);
		break;
	case 3:
		data.three = int(bTurnOn);
		break;
	default:
		if (!bTurnOn)
			return 0;
	}

	IterateLinks("ScriptParams", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&data));

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapSMTrans
 */
struct SMTrans_params
{
	const char* pszMsg;
	int iCount;
};

int cScr_SMTrans::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_SMTrans* scrSMTrans = static_cast<cScr_SMTrans*>(pScript);
	SMTrans_params* params = reinterpret_cast<SMTrans_params*>(pData);
	char* pszData = FixupScriptParamsHack(reinterpret_cast<const char*>(pLQ->Data()));
	if (pszData)
	{
		if (!_stricmp(pszData, params->pszMsg))
		{
			sLink sl;
			pLQ->Link(&sl);
			scrSMTrans->PostMessage(sl.dest, scrSMTrans->GetFlags()?"TurnOff":"TurnOn", 0);
			params->iCount++;
		}
		g_pMalloc->Free(pszData);
	}
	return 1;
}

long cScr_SMTrans::OnTurnOn(sScrMsg*, cMultiParm&)
{
	// Prevent BaseTrap from doing anything.
	return 0;
}

long cScr_SMTrans::OnTurnOff(sScrMsg*, cMultiParm&)
{
	// Prevent BaseTrap from doing anything.
	return 0;
}

STDMETHODIMP cScr_SMTrans::ReceiveMessage(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace)
{
	cBaseTrap::ReceiveMessage(pMsg, pReply, eTrace);

	InitTrapVars();
	// Clear all but Invert
	// Can't use Once because this script is likely on an object which uses the lock
	SetFlags(GetFlags() & (kTrapFlagInvert));
	SMTrans_params params = {pMsg->message, 0};
	IterateLinks("ScriptParams", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&params));

	return 0;
}


/***
 * Forwarder
 */
struct Forwarder_params
{
	sScrMsg* pMsg;
	int iCount;
};

int cScr_Forwarder::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	Forwarder_params* params = reinterpret_cast<Forwarder_params*>(pData);
	char* pszData = FixupScriptParamsHack(reinterpret_cast<const char*>(pLQ->Data()));
	if (pszData)
	{
		if (!_stricmp(pszData, params->pMsg->message))
		{
			sLink sl;
			pLQ->Link(&sl);
			int iSaveDest = params->pMsg->to;
			params->pMsg->to = sl.dest;
			params->pMsg->flags = kScrMsgPostToOwner;
			cMultiParm mpRet;
			g_pScriptManager->SendMessage(params->pMsg, &mpRet);
			params->pMsg->to = iSaveDest;
			params->iCount++;
		}
		g_pMalloc->Free(pszData);
	}
	return 1;
}

long cScr_Forwarder::OnTurnOn(sScrMsg*, cMultiParm&)
{
	// Prevent BaseTrap from doing anything.
	return 0;
}

long cScr_Forwarder::OnTurnOff(sScrMsg*, cMultiParm&)
{
	// Prevent BaseTrap from doing anything.
	return 0;
}

STDMETHODIMP cScr_Forwarder::ReceiveMessage(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace)
{
	cBaseTrap::ReceiveMessage(pMsg, pReply, eTrace);

	Forwarder_params params = {pMsg, 0};
	pMsg->AddRef();
	int iSaveFlags = pMsg->flags;
	IterateLinks("ScriptParams", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&params));
	pMsg->flags = iSaveFlags;
	pMsg->Release();

	return 0;
}


/***
 * Validator
 */
long cScr_Validator::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iValidateParam.Init(0);

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_Validator::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	char szParam[12];
	sprintf(szParam, "%d", int(m_iValidateParam));
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinksData(ObjId(), bTurnOn?"TurnOn":"TurnOff", pLTS->LinkKindNamed("ScriptParams"), szParam);

	char* pszParams = GetObjectParams(ObjId());
	if (pszParams)
	{
		char* pszOrder = GetParamString(pszParams, "order");
		if (pszOrder)
		{
			if (!_stricmp(pszOrder, "increment"))
			{
				int iNext = m_iValidateParam + GetParamInt(pszParams, "increment", 1);
				if (iNext >= GetParamInt(pszParams, "rollover"))
					iNext = 0;
				m_iValidateParam = iNext;
			}
			else if (!_stricmp(pszOrder, "step"))
			{
				int iNext = m_iValidateParam + GetParamInt(pszParams, "increment", 1);
				int iStep = GetParamInt(pszParams, "rollover");
				while (iNext >= iStep)
				{
					iNext -= iStep;
				}
				m_iValidateParam = iNext;
			}
			g_pMalloc->Free(pszOrder);
		}
		g_pMalloc->Free(pszParams);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

long cScr_Validator::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Validate"))
	{
		m_iValidateParam = pMsg->data;
		return 0;
	}
	return cBaseTrap::OnMessage(pMsg, mpReply);
}


/***
 * LinkTemplate
 */
long cScr_LinkTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SInterface<ILinkManager> pLM(g_pScriptManager);
	char* pszParams = GetObjectParams(ObjId());
	if (pszParams)
	{
		object iObj = GetParamObject(pszParams, "object");
		if (iObj)
		{
			object iDest = GetParamObject(pszParams, "dest");
			linkkind lFlavor;
			{
				char* pszFlavor = GetParamString(pszParams, "flavor");
				if (pszFlavor)
				{
					SInterface <IRelation> pRel = pLM->GetRelationNamed(pszFlavor);
					if (pRel)
						lFlavor = pRel->GetID();
					g_pMalloc->Free(pszFlavor);
				}
			}
			linkkind lIgnoreFlavor = 0;
			if (!lFlavor)
			{
				SInterface<IRelation> pRel = pLM->GetRelationNamed(g_pszCDLinkFlavor);
				if (pRel)
					lIgnoreFlavor = - pRel->GetID();
			}
			bool bOnCreate = GetParamBool(pszParams, "on_create");
			bool bOffDestroy = GetParamBool(pszParams, "off_destroy");
			bool bSingleton = GetParamBool(pszParams, "singleton");

			object iOldSource, iNewSource;
			if (bTurnOn)
			{
				iOldSource = ObjId();
				iNewSource = iObj;
			}
			else
			{
				iOldSource = iObj;
				iNewSource = ObjId();
			}

			linkset lsLinks(pLM->Query(iOldSource, iDest, lFlavor));
			for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
			{
				sLink sl = lsLinks.Get();
				if (!sl.flavor || sl.flavor == lIgnoreFlavor)
					continue;
				if (!(bSingleton && pLM->AnyLinks(sl.flavor, iNewSource, sl.dest))
				 && (bTurnOn || !bOffDestroy))
				{
					link lNewLink = pLM->Add(iNewSource, sl.dest, sl.flavor);
					if (lNewLink)
					{
						// AddFull data gets preempted by the AIWatchLinkDefaults property
						// so we do this in two steps.
						// FIXME: investigate if there are fields that should be masked
						void* pData = lsLinks.Data();
						if (pData)
							pLM->SetData(lNewLink, pData);
					}
					else
					{
						cMultiParm mp = sl.dest;
						DebugString("Failed to make link to ", static_cast<const char*>(mp));
					}
				}
				if (!(bTurnOn && bOnCreate))
				{
					pLM->Remove(lsLinks.Link());
				}
			}
		}
		g_pMalloc->Free(pszParams);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapTeleportDelta
 */
static cScrVec vec_rotate(const cScrVec& v, const cScrVec& r)
{
	double m[9];
	double ix,iy,iz, jx,jy,jz;

	ix = sin(r.x * M_PI_2 / 90.0);
	iy = sin(r.y * M_PI_2 / 90.0);
	iz = sin(r.z * M_PI_2 / 90.0);
	jx = cos(r.x * M_PI_2 / 90.0);
	jy = cos(r.y * M_PI_2 / 90.0);
	jz = cos(r.z * M_PI_2 / 90.0);
	m[0] = jy*jz; m[1] = (ix*iy*jz)-(jx*iz); m[2] = (jx*iy*jz)+(ix*iz);
	m[3] = jy*iz; m[4] = (ix*iy*iz)+(jx*jz); m[5] = (jx*iy*iz)-(ix*jz);
	m[6] = -iy; m[7] = ix*jy; m[8] = jx*jy;
	return cScrVec(
		(v.x*m[0])+(v.y*m[1])+(v.z*m[2]),
		(v.x*m[3])+(v.y*m[4])+(v.z*m[5]),
		(v.x*m[6])+(v.y*m[7])+(v.z*m[8])
	);
}

void cScr_DeltaTeleport::Teleport(object iObj)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	SService<IPhysSrv> pPhys(g_pScriptManager);
	cScrVec vPos, vDestPos, vDeltaPos;
	cScrVec vRot, vDestRot, vDeltaRot;
	cScrVec vVel;
	pOS->Position(vPos, iObj);
	pOS->Position(vDestPos, ObjId());
	pOS->Facing(vRot, iObj);
	pOS->Facing(vDestRot, ObjId());

	object iDelta = GetOneLinkByDataDest("ScriptParams", ObjId(), "dm", -1);
	if (iDelta)
	{
		pOS->Position(vDeltaPos, iDelta);
		vPos -= vDeltaPos;
		pOS->Facing(vDeltaRot, iDelta);
		vDestRot -= vDeltaRot;
		vRot += vDestRot;

		if (vDestRot.MagSquared() > 0.001f)
		{
			vPos = vec_rotate(vPos, vDestRot);
			pPhys->GetVelocity(iObj, vVel);
			vVel = vec_rotate(vVel, vDestRot);
			pPhys->SetVelocity(iObj, vVel);
		}
		vDestPos += vPos;
	}
	else
	{
		vRot = vDestRot;
	}

	pOS->Teleport(iObj, vDestPos, vRot, 0);
}

long cScr_DeltaTeleport::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		object iObj = GetOneLinkDest(g_pszCDLinkFlavor, ObjId());
		if (iObj)
		{
#if (_DARKGAME == 3)
			SService<INetworkingSrv> pNet(g_pScriptManager);
			if (pNet->IsProxy(iObj))
				pNet->SendToProxy(iObj, ObjId(), "NetTeleport", iObj);
			else
				Teleport(iObj);
#else
			Teleport(iObj);
#endif
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

#if (_DARKGAME == 3)
long cScr_DeltaTeleport::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "NetTeleport"))
	{
		Teleport(pMsg->data);
		return 0;
	}
	return cBaseTrap::OnMessage(pMsg, mpReply);
}
#endif


/***
 * RandomRelay
 */
long cScr_RelayRandom::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	bool bWeighted = false;
	bool bRecharge = false;
	bool bReusable = false;
	bool bDontKill = false;
	char* pszParams = GetObjectParams(ObjId());
	if (pszParams)
	{
		bReusable = GetParamBool(pszParams, "reusable");
		bDontKill = GetParamBool(pszParams, "preserve");
		bWeighted = GetParamBool(pszParams, "weighted");
		bRecharge = GetParamBool(pszParams, "rechargeable");

		g_pMalloc->Free(pszParams);
	}
	if (!bReusable)
		SetFlag(kTrapFlagOnce);
	if (bRecharge)
		bWeighted = true;

	if (bWeighted)
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);

		int iTotal = 0;
		list<pair<int,link> > vLinks;
		linkset lsLinks;
		pLS->GetAll(lsLinks, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
		for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
		{
			cMultiParm mpWeight;
			pLTS->LinkGetData(mpWeight, lsLinks.Link(), NULL);
			pair<int,link> entry = make_pair(int(mpWeight),lsLinks.Link());
			if (entry.first != 0)
			{
				list<pair<int,link> >::iterator _i =
						lower_bound(vLinks.begin(),vLinks.end(),entry);
				vLinks.insert(_i, entry);
				if (entry.first > 0)
					iTotal += entry.first;
			}
		}

		if (!vLinks.empty())
		{
			list<pair<int,link> >::iterator iLink =
					lower_bound(vLinks.begin(),vLinks.end(),make_pair(0,link()));
			if (iLink == vLinks.end())
			{
				// If the links exist, then we can infer rechargeable
				iTotal = 0;
				for (iLink = vLinks.begin(); iLink != vLinks.end(); ++iLink)
				{
					iTotal += (iLink->first = - iLink->first);
					cMultiParm mpRecharge = iLink->first;
					pLTS->LinkSetData(iLink->second, NULL, mpRecharge);
				}
				vLinks.sort();
				iLink = vLinks.begin();
			}
			int iSel;
			{
				SService<IDataSrv> pDS(g_pScriptManager);
				iSel = pDS->RandInt(0, iTotal-1);
			}
			for (int iLevel = 0; iLink != vLinks.end(); ++iLink)
			{
				iLevel += iLink->first;
				if (iLevel > iSel)
				{
					sLink slDest;
					pLTS->LinkGet(iLink->second, slDest);
					PostMessage(slDest.dest, bTurnOn?"TurnOn":"TurnOff", 0);
					if (bRecharge)
					{
						cMultiParm mpNeg = - iLink->first;
						pLTS->LinkSetData(iLink->second, NULL, mpNeg);
					}
					else if (!bDontKill)
					{
						pLS->Destroy(iLink->second);
					}
					break;
				}
			}
		}
	}
	else // ! bWeighted
	{
		sLink slDest;
		link lDest = GetAnyLink(g_pszCDLinkFlavor, ObjId(), 0, &slDest);
		if (lDest)
		{
			PostMessage(slDest.dest, bTurnOn?"TurnOn":"TurnOff", 0);
			if (!bDontKill)
			{
				SService<ILinkSrv> pLS(g_pScriptManager);
				pLS->Destroy(lDest);
			}
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrigSim
 */
long cScr_SimTrigger::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
		SetTimedMessage("DelayInit", 100, kSTM_OneShot, "SimTrigger");

	return cBaseTrap::OnSim(pSimMsg, mpReply);
}

long cScr_SimTrigger::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "DelayInit") && pTimerMsg->data == "SimTrigger")
	{
		DoTrigger(true);
		SetTimedMessage("DelayTerm", 500, kSTM_OneShot, "SimTrigger");
		return 0;
	}
	// Gives time for other scripts on this object to activate.
	if (!strcmp(pTimerMsg->name, "DelayTerm"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oArc;
		true_bool bPhysical;
		pOS->Named(oArc, "Physical");
		pOS->InheritsFrom(bPhysical, ObjId(), oArc);
		if (!bPhysical)
			pOS->Destroy(ObjId());
		return 0;
	}
	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}


/***
 * TrapStim
 */
struct StimTrap_data
{
	IActReactSrv* pARSrv;
	object iStim;
	float fIntensity;
};

int cScr_StimTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_StimTrap* scrStimTrap = static_cast<cScr_StimTrap*>(pScript);
	StimTrap_data* params = reinterpret_cast<StimTrap_data*>(pData);

	sLink sl;
	pLQ->Link(&sl);

	params->pARSrv->ARStimulate(sl.dest, params->iStim, params->fIntensity, scrStimTrap->ObjId());
	return 1;
}

void cScr_StimTrap::StimLinks(float fScale, bool bSelf)
{
	StimTrap_data params;
	params.iStim = GetObjectParamObject(ObjId(), "stim");
	if (!params.iStim)
		params.iStim = StrToObject("ScriptStim");
	if (!params.iStim)
		return;

	params.fIntensity = GetObjectParamFloat(ObjId(), "intensity", 1.0f) * fScale;

	SService<IActReactSrv> pARSrv(g_pScriptManager);
	if (bSelf)
	{
		pARSrv->ARStimulate(ObjId(), params.iStim, params.fIntensity, ObjId());
	}
	params.pARSrv = pARSrv.get();
	IterateLinks(g_pszCDLinkFlavor, ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&params));
}

long cScr_StimTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	StimLinks(bTurnOn ? 1.0f : -1.0f);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TimedStimulator
 */
void cScr_StimRepeater::GetLinkParams(int* piInterval, int* piInitial)
{
	SInterface<ITraitManager> pTM(g_pScriptManager);
	SInterface<ILinkManager> pLM(g_pScriptManager);
	SInterface<IRelation> pRel = pLM->GetRelationNamed("ScriptParams");
	if (!pRel)
		return;

	link iLinkID = 0;
	SInterface<IObjectQuery> pTree = pTM->Query(ObjId(), kTraitQueryMetaProps | kTraitQueryFull);
	if (!pTree)
		return;

	for (; ! pTree->Done(); pTree->Next())
	{
		if (pRel->AnyLinks(pTree->Object(), pTree->Object()))
		{
			SInterface<ILinkQuery> pLQ = pRel->Query(pTree->Object(), pTree->Object());
			iLinkID = pLQ->ID();
			break;
		}
	}

	if (iLinkID)
	{
		const char* pszData = reinterpret_cast<const char*>(pRel->GetData(iLinkID));
		if (pszData)
		{
			sscanf(pszData, "%d / %d", piInterval, piInitial);
		}
	}
}

void cScr_StimRepeater::StartTimer(void)
{
	if (m_hTimer == NULL)
	{
		m_hTimer = SetTimedMessage("TimedStimThrob", m_iInterval, kSTM_Periodic);
	}
}

long cScr_StimRepeater::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_hTimer.Init();
	int iInitial = 0;
	int iInterval = 1000;
	if (!m_iInterval.Valid())
	{
		char* pszInterval = GetObjectParamString(ObjId(), "interval");
		if (pszInterval)
		{
			iInterval = strtotime(pszInterval);
			iInitial = GetObjectParamInt(ObjId(), "stage");
			g_pMalloc->Free(pszInterval);
		}
		else // Old-style link kludgery
		{
			GetLinkParams(&iInterval, &iInitial);
		}
	}

	m_iInterval.Init(iInterval);
	m_iScale.Init(iInitial);

	if (pMsg->time > 0 && !GetObjectParamBool(ObjId(), "device"))
	{
		StartTimer();
	}

	return cScr_StimTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_StimRepeater::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "TimedStimThrob"))
	{
		StimLinks(m_iScale, true);
		m_iScale += 1;
		return 0;
	}

	return cScr_StimTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_StimRepeater::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Stage"))
	{
		m_iScale = pMsg->data;
		return 0;
	}

	return cScr_StimTrap::OnMessage(pMsg, mpReply);
}

long cScr_StimRepeater::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		StartTimer();
	}
	else
	{
		if (m_hTimer != NULL)
		{
			KillTimedMessage(m_hTimer);
			m_hTimer = NULL;
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrigOBBSpec
 */
long cScr_OBBSpec::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_OBBSpec::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_OBBSpec::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	return cBaseTrap::OnSim(pSimMsg, mpReply);
}

long cScr_OBBSpec::OnPhysEnter(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	sLink slArc;
	if (GetOneLinkByDataInherited("ScriptParams", ObjId(), pPhysMsg->transObj, &slArc, "arc", -1))
	{
		/*
		SService<IObjectSrv> pOS(g_pScriptManager);
		true_bool bRelated;
		if (*pOS->InheritsFrom(bRelated, pPhysMsg->transObj, slArc.dest))
		{
		*/
			if (TrackCreatureEnter(pPhysMsg->transObj))
			{
				SpecTrigger(true, pPhysMsg->transObj);
				if (Population() == 1)
					DoTrigger(true, pPhysMsg->transObj);
			}
		//}
	}

	return cBaseTrap::OnPhysEnter(pPhysMsg, mpReply);
}

long cScr_OBBSpec::OnPhysExit(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	sLink slArc;
	if (GetOneLinkByDataInherited("ScriptParams", ObjId(), pPhysMsg->transObj, &slArc, "arc", -1))
	{
		/*
		SService<IObjectSrv> pOS(g_pScriptManager);
		true_bool bRelated;
		if (*pOS->InheritsFrom(bRelated, pPhysMsg->transObj, slArc.dest))
		{
		*/
			if (TrackCreatureExit(pPhysMsg->transObj))
			{
				if (Population() == 0)
					DoTrigger(false, pPhysMsg->transObj);
				SpecTrigger(false, pPhysMsg->transObj);
			}
		//}
	}
	return cBaseTrap::OnPhysExit(pPhysMsg, mpReply);
}

long cScr_OBBSpec::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Obituary"))
	{
		if (TrackCreatureExit(pMsg->from) && Population() == 0)
			DoTrigger(false, pMsg->from);
	}

	return cBaseTrap::OnMessage(pMsg, mpReply);
}

void cScr_OBBSpec::SpecTrigger(bool bTurnOn, object iSource)
{
	if (!IsLocked())
	{
		SService<ILinkSrv> pLS(g_pScriptManager);
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		linkkind lSP = pLTS->LinkKindNamed("ScriptParams");
		linkset lsLinks;
		pLS->GetAll(lsLinks, lSP, ObjId(), 0);
		for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
		{
			if (!_stricmp(reinterpret_cast<const char*>(lsLinks.Data()), "cd"))
			{
				sLink sl = lsLinks.Get();
				PostMessage(sl.dest, bTurnOn?"TurnOn":"TurnOff", iSource);
			}
		}
	}
}


/***
 * TrigBodyPickup
 */
long cScr_CorpseTrigger::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->event == kContainAdd)
		CDSend("TurnOn", ObjId(), pContMsg->container);
	else if (pContMsg->event == kContainRemove)
		CDSend("TurnOff", ObjId(), pContMsg->container);

	return cBaseScript::OnContained(pContMsg, mpReply);
}


/***
 * TrigDamageRTC
 */
long cScr_DamageRTC::OnDamage(sDamageScrMsg* pDmgMsg, cMultiParm& mpReply)
{
	cMultiParm mpIgnore;
	SendMessage(mpIgnore, pDmgMsg->culprit, "TurnOn", pDmgMsg->damage);

	return cBaseScript::OnDamage(pDmgMsg, mpReply);
}


/***
 * TrapHP
 */
long cScr_HPTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	object iVictim = GetObjectParamObject(ObjId(), "target");
	if (iVictim)
	{
		object iCulprit = GetObjectParamObject(ObjId(), "culprit", ObjId());
		int iHP;
		iHP = GetObjectParamInt(ObjId(), "hitcount", 1);
		SService<IDamageSrv> pDS(g_pScriptManager);
		pDS->Damage(iVictim, iCulprit, (bTurnOn) ? -iHP : iHP, GetObjectParamObject(ObjId(), "damage"));
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * AIItemGiver
 */
long cScr_TransferItem::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "TransferItem"))
	{
		// Thief1 IContainService doesn't have IsHeld, this does though
		SInterface<IContainSys> pCS(g_pScriptManager);
		object iDest = StrToObject(static_cast<const char*>(pMsg->data));
		object iItem = StrToObject(static_cast<const char*>(pMsg->data2));
		if (!iDest || !iItem )
		{
			mpReply = 0;
			return 0;
		}
		// Weird... 0 is true. MAXLONG is false
		// Not-so-weird really, as the return value is actually just the contains type.
		if (LONG_MAX == pCS->IsHeld(ObjId(), iItem))
		{
			DebugString("AI isn't container for ", static_cast<const char*>(pMsg->data2));
			mpReply = 0;
			return 0;
		}

#if (_DARKGAME == 3)
		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->RemoveFromContainer(iItem, ObjId());
		//pShock->AddInvObj(iItem);
		// Or something. I think this is where you'd want to
		// send a net-msg to a proxy.
		/*
		SService<INetworkingSrv> pNet(g_pScriptManager);
		if (pNet->IsProxy(iDest))
		{
			cAnsiStr str;
			str.FmtStr("%d,%d", int(iDest), int(iObj));
			pNet->SendToProxy(iDest, ObjId(), "NetTransferItem", str);
		}
		else
			TransferItem(iDest, iItem);
		*/
		if (iDest == StrToObject("Player"))
			pShock->AddInvObj(iItem);
		else
			pCS->Add(iDest, iItem, 0, 1);
		pShock->RefreshInv();
#else
		pCS->Remove(ObjId(), iItem);
		pCS->Add(iDest, iItem, 0, 1);
#endif

		mpReply = 1;
		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * IntrinsicText
 */
void cScr_QuickText::ScanText(char* psz)
{
	char* e = psz+strlen(psz);
	char* p = strstr(psz, "||");
	while (p)
	{
		*p++ = '\n';
		memmove(p, p+1, (e--)-p);
		p = strstr(p, "||");
	}
}

void cScr_QuickText::DisplayText(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
#if (_DARKGAME != 3)
	if (pPS->Possessed(ObjId(), "Book"))
	{
		cMultiParm mpBook;
		pPS->Get(mpBook, ObjId(), "Book", NULL);
		ulong clr = 0;
		int iTime = 0;
		char* pszDNote = GetObjectParams(ObjId());
		if (pszDNote)
		{
			char* pszColor = GetParamString(pszDNote, "clr");
			if (pszColor)
			{
				clr = strtocolor(pszColor);
				g_pMalloc->Free(pszColor);
			}
			else
				clr = makecolor(GetParamInt(pszDNote, "clr_red"),
						GetParamInt(pszDNote, "clr_green"),
						GetParamInt(pszDNote, "clr_blue"));
			iTime = GetParamTime(pszDNote, "time");
			g_pMalloc->Free(pszDNote);
		}

		SService<IDataSrv> pDS(g_pScriptManager);
		SInterface<IGameStrings> pGS(g_pScriptManager);
		auto_ptr<char> szBookFile (new char[strlen(mpBook) + 10]);
		strcpy(szBookFile.get(), "..\\books\\");
		strcat(szBookFile.get(), mpBook);
		cScrStr str;
		pDS->GetString(str, szBookFile.get(), "page_0", "", "strings");
		if (!str.IsEmpty())
		{
			if (iTime == 0)
				iTime = CalcTextTime(str, 500);
			SService<IDarkUISrv> pUI(g_pScriptManager);
			pUI->TextMessage(str, clr, iTime);
		}
		str.Free();
	}
	else
#endif
	{
		char* pszDNote = GetObjectParams(ObjId());
		if (pszDNote)
		{
			char* pszText = GetParamString(pszDNote, "text");
			if (pszText)
			{
				ScanText(pszText);
				ulong c;
				char* pszColor = GetParamString(pszDNote, "clr");
				if (pszColor)
				{
					c = strtocolor(pszColor);
					g_pMalloc->Free(pszColor);
				}
				else
					c = makecolor(GetParamInt(pszDNote, "clr_red"),
							GetParamInt(pszDNote, "clr_green"),
							GetParamInt(pszDNote, "clr_blue"));
				int iTime = GetParamTime(pszDNote, "time");
				if (iTime == 0)
					iTime = CalcTextTime(pszText, 500);
				ShowString(pszText, iTime, c);

				g_pMalloc->Free(pszText);
			}
			else
			{
				ScanText(pszDNote);
				ShowString(pszDNote, CalcTextTime(pszDNote, 500));
			}

			g_pMalloc->Free(pszDNote);
		}
	}
}

long cScr_QuickText::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
		DisplayText();
	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

long cScr_FrobText::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	DisplayText();
	return cScr_QuickText::OnFrobWorldEnd(pFrobMsg, mpReply);
}

long cScr_FocusText::OnWorldSelect(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DisplayText();
	return cScr_QuickText::OnWorldFocus(pMsg, mpReply);
}


/***
 * InvSchema
 */
long cScr_InvSchema::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ObjSoundName"))
	{
		cMultiParm mpSoundName;
		pPS->Get(mpSoundName, ObjId(), "ObjSoundName", NULL);
		object iSchema = StrToObject(mpSoundName);
		if (iSchema)
		{
			if (pFrobMsg->Frobber == StrToObject("Player"))
				PlaySchemaAmbient(ObjId(), iSchema);
			else
				PlaySchema(ObjId(), iSchema, pFrobMsg->Frobber);
		}
	}
#else
	sLink slSchema;
	if (GetOneLinkInheritedSrc("SoundDescription", ObjId(), 0, &slSchema))
	{
		if (pFrobMsg->Frobber == StrToObject("Player"))
			PlaySchemaAmbient(ObjId(), slSchema.dest);
		else
			PlaySchema(ObjId(), slSchema.dest, pFrobMsg->Frobber);
	}
#endif

	return cBaseScript::OnFrobInvEnd(pFrobMsg, mpReply);
}


/***
 * Sippable
 */
#if (_DARKGAME == 3)
#define OBJNAME_PROP "ObjName"
#define OBJNAME_FILE "objname"
#else
#define OBJNAME_PROP "GameName"
#define OBJNAME_FILE "objnames"
#endif
void cScr_Sippable::SetSipsLeft(int iSips)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	char szBuf[32];

	if (pPS->Possessed(ObjId(), OBJNAME_PROP))
	{
		cMultiParm mpName;
		pPS->Get(mpName, ObjId(), OBJNAME_PROP, NULL);
		cAnsiStr strName = static_cast<const char*>(mpName);
		cAnsiStr strResName;
		uint n = strName.Find(':');
		if (n != cAnsiStr::MaxPos)
			strName.AllocCopy(strResName, n, 0, 0);
		else
			strResName = strName;
		if (strResName.GetAt(0) == '@')
			strResName.Remove(0, 1);
		uint p = strResName.Find('@');
		if (p != cAnsiStr::MaxPos)
		{
			strResName.SetAt(p, ':');
			strName = strResName;
			strResName.Remove(p, strResName.GetLength());
			n = p;
		}
		{
			SService<IDataSrv> pDS(g_pScriptManager);
			cScrStr strLocalName;
			pDS->GetString(strLocalName, OBJNAME_FILE, strResName, "", "strings");
			if (!strLocalName.IsEmpty())
			{
				strName = strLocalName;
			}
			else
			{
				uint q = strName.Find('"', n) + 1;
				if (q > 0)
				{
					strName.Remove(0, q);
					q = strName.Find('"');
					if (q != cAnsiStr::MaxPos)
						strName.Remove(q, strName.GetLength()-q);
				}
				else
					strName.Remove(0, n+1);
				n = strName.ReverseFind('\n');
				if (n != cAnsiStr::MaxPos)
					strName.Remove(n, strName.GetLength()-n);
				// Save to ResName. Only necessary if string has %d, but
				// better be safe here. Doing it in the next check would
				// be wasteful when the string is from a file.
				strResName.Append('@');
				strResName.Append('"');
				strResName += strName;
				strResName.Append('"');
			}
			strLocalName.Free();
		}

		// Visible string is now in strName, resource name is strResName
		n = strName.Find("%i");
		if (n == cAnsiStr::MaxPos) n = strName.Find("%d");
		if (n != cAnsiStr::MaxPos)
		{
			sprintf(szBuf, "%i", iSips);
			strName.Replace(szBuf, n, 2);
			// Of course, if iSips is 1, it'll still say "1 sips"
			// but this is a classic problem of l10n, and there's no
			// reasonable way to solve it. At least, not for this
			// puny little script.
		}
		else
		{
			if (iSips == 1)
				strcpy(szBuf, "\n1 sip left");
			else
				sprintf(szBuf, "\n%i sips left", iSips);
			strName += szBuf;
		}

		// strName should be the completed name, now we want to wrap it
		// in quotes and add the resource name
		strResName.Insert('@', 0);
		strResName += ": ";
		strResName += strName.Quoted(cAnsiStr::kEscapeQuotes);
		mpName = static_cast<const char*>(strResName);
		pPS->SetSimple(ObjId(), OBJNAME_PROP, mpName);
	}

	SetScriptData("m_iSips", iSips);
}

int cScr_Sippable::GetSipsLeft(void)
{
	if (IsScriptDataSet("m_iSips"))
	{
		cMultiParm mpSipsLeft = GetScriptData("m_iSips");
		return mpSipsLeft;
	}
	return GetObjectParamInt(ObjId(), "sips");
}

long cScr_Sippable::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	int iSips = GetSipsLeft() - 1;

	if (iSips <= 0)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
	}

	SetSipsLeft(iSips);

	return cBaseScript::OnFrobInvEnd(pFrobMsg, mpReply);
}

long cScr_Sippable::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	SetSipsLeft(GetSipsLeft());

	return cBaseScript::OnContained(pContMsg, mpReply);
}


/***
 * Zapper
 */
long cScr_Zapper::OnEndAttack(sAttackMsg* pAttackMsg, cMultiParm& mpReply)
{
	// The AITarget link can sometimes lag behind the actual target.
	// Probably because of a race between the attack maneuver and
	// when the link actually gets updated.
	// As a result, the zap may be wrong when the AI changes targets.
	object iTarget = GetOneLinkDest("AITarget", ObjId());
	object iStim = GetObjectParamObject(ObjId(), "zap_stim");
	if (!iStim)
#if (_DARKGAME == 3)
		iStim = StrToObject("Anti-Human");
#else
		iStim = StrToObject("MagicZapStim");
#endif
	if (iTarget && iStim)
	{
		float fIntensity = GetObjectParamFloat(ObjId(), "zap_strength", 3.0);

		SService<IActReactSrv> pARSrv(g_pScriptManager);
		pARSrv->ARStimulate(iTarget, iStim, fIntensity, ObjId());

		if (! GetObjectParamBool(ObjId(), "no_zap_sound"))
		{
			object iSchema = StrToObject(
					(iTarget == StrToObject("Player")) ?
#if (_DARKGAME == 3)
					"dam_sting" : "exp_cryopsi");
#else
					"ghmagic" : "hit_magic");
#endif
			if (iSchema)
			{
				PlaySchema(ObjId(), iSchema, iTarget);
			}
		}
	}

	return cBaseAIScript::OnEndAttack(pAttackMsg, mpReply);
}


/***
 * PhysModelCorrect
 */
void cScr_PhysScale::DoResize(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(ObjId(), "Scale"))
		return;

	SInterface<ITraitManager> pTM(g_pScriptManager);
	object iArc = pTM->GetArchetype(ObjId());
	cScrVec vScale, vOBBDims;
	cMultiParm mpProp;

	pPS->Get(mpProp, ObjId(), "Scale", NULL);
	vScale = *static_cast<const mxs_vector*>(mpProp);
	if (pPS->Possessed(iArc, "Scale"))
	{
		pPS->Get(mpProp, iArc, "Scale", NULL);
		vScale /= *static_cast<const mxs_vector*>(mpProp);
	}
	DebugPrintf("Adjusting phys model scale of %d by %0.2f,%0.2f,%0.2f",
		ObjId(), vScale.x, vScale.y, vScale.z);

	pPS->Remove(ObjId(), "PhysDims");
	pPS->Add(ObjId(), "PhysDims");

	pPS->Get(mpProp, ObjId(), "PhysType", "Type");
	switch (int(mpProp))
	{
	case kPMT_OBB:
		pPS->Get(mpProp, ObjId(), "PhysDims", "Size");
		vOBBDims = *static_cast<const mxs_vector*>(mpProp);
		DebugPrintf("Old dims were %0.2f,%0.2f,%0.2f",
			vOBBDims.x, vOBBDims.y, vOBBDims.z);
		vOBBDims *= vScale;
		DebugPrintf("New dims  %0.2f,%0.2f,%0.2f\n",
			vOBBDims.x, vOBBDims.y, vOBBDims.z);
		mpProp = vOBBDims;
		pPS->Set(ObjId(), "PhysDims", "Size", mpProp);
		// Offset
		pPS->Get(mpProp, ObjId(), "PhysDims", "Offset 1");
		vOBBDims = *static_cast<const mxs_vector*>(mpProp);
		vOBBDims *= vScale;
		mpProp = vOBBDims;
		pPS->Set(ObjId(), "PhysDims", "Offset 1", mpProp);
		break;
	case kPMT_Sphere:
	case kPMT_SphereHat:
		pPS->Get(mpProp, ObjId(), "PhysDims", "Radius 1");
		DebugPrintf("Old radius was %f", static_cast<float>(mpProp));
		mpProp = vScale.z * static_cast<float>(mpProp);
		DebugPrintf("New radius is %f\n", static_cast<float>(mpProp));
		pPS->Set(ObjId(), "PhysDims", "Radius 1", mpProp);
		pPS->Get(mpProp, ObjId(), "PhysDims", "Radius 2");
		mpProp = vScale.z * static_cast<float>(mpProp);
		pPS->Set(ObjId(), "PhysDims", "Radius 2", mpProp);
		// Offset
		pPS->Get(mpProp, ObjId(), "PhysDims", "Offset 1");
		vOBBDims = *static_cast<const mxs_vector*>(mpProp);
		vOBBDims *= vScale;
		mpProp = vOBBDims;
		pPS->Set(ObjId(), "PhysDims", "Offset 1", mpProp);
		pPS->Get(mpProp, ObjId(), "PhysDims", "Offset 2");
		vOBBDims = *static_cast<const mxs_vector*>(mpProp);
		vOBBDims *= vScale;
		mpProp = vOBBDims;
		pPS->Set(ObjId(), "PhysDims", "Offset 2", mpProp);
		break;
	}
}

long cScr_PhysScale::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		DoResize();
	}

	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cScr_PhysScale::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeScale)
	{
		DoResize();
	}

	return cBaseScript::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_PhysScale::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim() && !_stricmp(pMsg->message, "PMResize"))
	{
		DoResize();
		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * TrapPhantom
 */
const int cScr_AlphaTrap::sm_iFadeResolution = 50;

cScr_AlphaTrap::cScr_AlphaTrap(const char* pszName, int iHostObjId)
	: cBaseTrap(pszName, iHostObjId),
	  //SCRIPT_VAROBJ(cScr_AlphaTrap, m_hFadeTimer, iHostObjId),
	  SCRIPT_VAROBJ(cScr_AlphaTrap, m_bActive, iHostObjId),
	  SCRIPT_VAROBJ(cScr_AlphaTrap, m_iSign, iHostObjId),
	  SCRIPT_VAROBJ(cScr_AlphaTrap, m_iStartTime, iHostObjId),
	  m_fAlphaMin(0.0), m_fAlphaMax(1.0), m_iFadeTime(1000), m_iCurve(0)
{
	char* pszParams = GetObjectParams(ObjId());
	if (pszParams)
	{
		char* psz;
		m_iCurve = GetParamInt(pszParams, "curve");
		m_fAlphaMin = GetParamFloat(pszParams, "alpha_min", 0.0);
		m_fAlphaMax = GetParamFloat(pszParams, "alpha_max", 1.0);
		if ((psz = GetParamString(pszParams, "fade_time")))
		{
			m_iFadeTime = strtotime(psz);
			g_pMalloc->Free(psz);
		}
		else if ((psz = GetParamString(pszParams, "rate")))
		{
			float fRate = strtod(psz, NULL);
			if (fRate != 0)
				m_iFadeTime = 1000.0f * (m_fAlphaMax - m_fAlphaMin) / fRate;
			g_pMalloc->Free(psz);
		}
		g_pMalloc->Free(pszParams);
	}
}

long cScr_AlphaTrap::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	//m_hFadeTimer.Init();
	m_bActive.Init(0);
	m_iSign.Init(0);
	m_iStartTime.Init(0);

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(ObjId(), "RenderAlpha"))
	{
		pPS->Add(ObjId(), "RenderAlpha");
		pPS->SetSimple(ObjId(), "RenderAlpha", m_fAlphaMax);
	}

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_AlphaTrap::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "PhantomUpdate"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		bool bTurnOn = bool(pTimerMsg->data);
		if ((bTurnOn && m_iSign != 1)
		 ||(!bTurnOn && m_iSign != -1))
			return 0;
		double fFade = double(pTimerMsg->time - m_iStartTime) / m_iFadeTime;
		if (fFade >= 1.0)
		{
			cMultiParm mpAlpha = bTurnOn ? m_fAlphaMax : m_fAlphaMin;
			pPS->SetSimple(ObjId(), "RenderAlpha", mpAlpha);
			DoTrigger(bTurnOn);
			//KillTimedMessage(m_hFadeTimer);
			//m_hFadeTimer = NULL;
			m_bActive = false;
			m_iSign = 0;
		}
		else
		{
			double fCurrAlpha = bTurnOn ?
				  CalculateCurve(m_iCurve, fFade, m_fAlphaMin, m_fAlphaMax)
				: CalculateCurve(m_iCurve, fFade, m_fAlphaMax, m_fAlphaMin);
			pPS->SetSimple(ObjId(), "RenderAlpha", fCurrAlpha);
		}
		if (m_bActive)
			SetTimedMessage("PhantomUpdate", sm_iFadeResolution, kSTM_OneShot, int(bTurnOn));
		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_AlphaTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if ((bTurnOn && m_iSign > 0)
	 ||(!bTurnOn && m_iSign < 0))
		return 0;

	/*
	if (m_hFadeTimer)
	{
		KillTimedMessage(m_hFadeTimer);
		m_hFadeTimer = NULL;
	}
	*/

	SService<IPropertySrv> pPS(g_pScriptManager);
	cMultiParm mpAlpha;

	if (m_iFadeTime <= 0)
	{
		mpAlpha = bTurnOn ? m_fAlphaMax : m_fAlphaMin;
		pPS->SetSimple(ObjId(), "RenderAlpha", mpAlpha);
		DoTrigger(bTurnOn);
		m_iSign = 0;
		m_bActive = false;
	}
	else
	{
		pPS->Get(mpAlpha, ObjId(), "RenderAlpha", NULL);
		float fCurrAlpha = mpAlpha;
		if (bTurnOn)
		{
			int iAdjTime = double(m_iFadeTime) * (fCurrAlpha - m_fAlphaMin) / (m_fAlphaMax - m_fAlphaMin);
			m_iStartTime = pMsg->time - iAdjTime;
			if (m_iSign != 1)
			{
				m_iSign = 1;
				m_bActive = true;
				SetTimedMessage("PhantomUpdate", sm_iFadeResolution, kSTM_OneShot, int(bTurnOn));
			}
		}
		else
		{
			int iAdjTime = double(m_iFadeTime) * (m_fAlphaMax - fCurrAlpha) / (m_fAlphaMax - m_fAlphaMin);
			m_iStartTime = pMsg->time - iAdjTime;
			if (m_iSign != -1)
			{
				m_iSign = -1;
				m_bActive = true;
				SetTimedMessage("PhantomUpdate", sm_iFadeResolution, kSTM_OneShot, int(bTurnOn));
			}
		}
		/*
		if (!m_bActive)
		{
			m_bActive = true;
			SetTimedMessage("PhantomUpdate", sm_iFadeResolution, kSTM_OneShot, int(bTurnOn));
		}
		*/
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapMotion
 */
struct MotionTrap_params
{
	IPuppetSrv* pPuppet;
	const char* pszMot;
};

int cScr_MotionTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	MotionTrap_params* data = reinterpret_cast<MotionTrap_params*>(pData);
	sLink sl;
	pLQ->Link(&sl);

	true_bool _p;
	data->pPuppet->PlayMotion(_p, sl.dest, data->pszMot);

	return 1;
}

long cScr_MotionTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		char* pszMot = GetObjectParamString(ObjId(), "mot");
		if (pszMot)
		{
			SService<IPuppetSrv> pPuppet(g_pScriptManager);
			MotionTrap_params data = {pPuppet.get(), pszMot};
			IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&data));
			g_pMalloc->Free(pszMot);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


#if (_DARKGAME == 2) || (_DARKGAME == 3)
/***
 * TrapCamShift
 */
long cScr_CameraTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 2)
	SService<ICameraSrv> pCam(g_pScriptManager);
	if (bTurnOn)
		pCam->StaticAttach(ObjId());
	else
		pCam->CameraReturn(ObjId());
#elif (_DARKGAME == 3)
	SService<IShockGameSrv> pShock(g_pScriptManager);
	if (bTurnOn)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		cScrStr strName;
		pOS->GetName(strName, ObjId());
		if (!strName.IsEmpty())
		{
			pShock->AttachCamera(strName);
		}
		else
		{
			DebugString("Attachment object must be named!");
		}
		strName.Free();
		pShock->NoMove(1);
	}
	else
	{
		pShock->NoMove(0);
		pShock->AttachCamera("Player");
	}
#endif

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}
#endif


/***
 * TrigAIAwareShift
 */
cScr_TrackAwareness::cScr_TrackAwareness(const char* pszName, int iHostObjId)
	: cBaseScript(pszName, iHostObjId),
	  SCRIPT_VAROBJ(cScr_TrackAwareness,m_iPrevVis,iHostObjId),
	  SCRIPT_VAROBJ(cScr_TrackAwareness,m_iPrevAud,iHostObjId)
{
	DarkHookInitializeService(g_pScriptManager, g_pMalloc);
	static const sMessageHandler handlers[] = {{"DHNotify",HandleDHNotify}};
	RegisterMessageHandlers(handlers, 1);
}

long cScr_TrackAwareness::HandleDHNotify(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_TrackAwareness*>(pScript)->OnNotify(static_cast<sDHNotifyMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cScr_TrackAwareness::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	try
	{
		SService<IDarkHookScriptService> pDH2(g_pScriptManager);
		pDH2->InstallRelHook(ObjId(), kDHNotifyDefault, "AIAwareness", ObjId());
	}
	catch (no_interface&)
	{
		DebugString("Unable to locate DarkHook service.");
	}

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_TrackAwareness::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	try
	{
		SService<IDarkHookScriptService> pDH2(g_pScriptManager);
		pDH2->UninstallRelHook(ObjId(), "AIAwareness", ObjId());
	}
	catch (no_interface&)
	{
		DebugString("Unable to locate DarkHook service.");
	}
	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_TrackAwareness::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
		FixupPlayerLinks();
	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cScr_TrackAwareness::OnNotify(sDHNotifyMsg* pDHMsg, cMultiParm&)
{
	if (pDHMsg->typeDH != kDH_Relation
	 || 0 != _stricmp(pDHMsg->sRel.pszRelName, "AIAwareness")
	 || !(pDHMsg->sRel.event & kRel_Change))
		return 1;

	const sAIAwareness* pAIAware =
		reinterpret_cast<sAIAwareness*>(pDHMsg->sRel.pRel->GetData(pDHMsg->sRel.lLinkId));
	bool bHasVis = false, bHasAud = false;
	bool bHasLinks = false;

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	linkkind lSP = pLTS->LinkKindNamed("ScriptParams");
	linkset lsFrom;
	pLS->GetAll(lsFrom, lSP, ObjId(), pDHMsg->sRel.iLinkDest);
	for (; lsFrom.AnyLinksLeft(); lsFrom.NextLink())
	{
		const char* pszData = reinterpret_cast<const char*>(lsFrom.Data());
		if (!pszData)
			continue;
		if (!strncmp(pszData, "af", 2))
			bHasLinks = true;
		else if (!strncmp(pszData, "as", 2))
		{
			for (pszData += 2; *pszData; ++pszData)
			{
				if (*pszData == 'v')
					bHasVis = true;
				else if (*pszData == 'a')
					bHasAud = true;
			}
		}
	}
	bool bVisChange = bHasVis ^ (0 != (pAIAware->uFlags & kAIAware_Seen));
	bool bAudChange = bHasAud ^ (0 != (pAIAware->uFlags & kAIAware_Heard));

	if (bHasLinks && (bVisChange || bAudChange))
	{
		linkset lsRelay;
		pLS->GetAll(lsRelay, lSP, ObjId(), pDHMsg->sRel.iLinkDest);
		for (; lsRelay.AnyLinksLeft(); lsRelay.NextLink())
		{
			const char* pszData = reinterpret_cast<const char*>(lsRelay.Data());
			if (!pszData)
				continue;
			if (!strncmp(pszData, "as", 2))
				pLS->Destroy(lsRelay.Link());
			else if (strncmp(pszData, "af", 2))
				continue;
			cAnsiStr strData = pszData;
			if (bVisChange)
			{
				strData.SetAt(1, 'v');
				pLS->BroadcastOnAllLinksData(ObjId(),
					bHasVis ? "TurnOff" : "TurnOn",
					lSP, static_cast<const char*>(strData));
			}
			if (bAudChange)
			{
				strData.SetAt(1, 'a');
				pLS->BroadcastOnAllLinksData(ObjId(),
					bHasAud ? "TurnOff" : "TurnOn",
					lSP, static_cast<const char*>(strData));
			}
		}
		char szData[5];
		{
			char* p = szData+2;
			memcpy(szData, "as\0\0\0", 5);
			if (pAIAware->uFlags & kAIAware_Seen)
				*p++ = 'v';
			if (pAIAware->uFlags & kAIAware_Heard)
				*p++ = 'a';
		}
		{
			link lTrack;
			pLS->Create(lTrack, lSP, ObjId(), pDHMsg->sRel.iLinkDest);
			pLTS->LinkSetData(lTrack, NULL, szData);
		}
	}

	return 0;
}


/***
 * Spy
 */
cAnsiStr cScr_Spy::FormatMultiParm(const sMultiParm& mp, const char* pszExtra)
{
	char buf[64];
	cAnsiStr str = pszExtra;

	switch (mp.type)
	{
	case kMT_Int:
		sprintf(buf, "%d", mp.i);
		str += buf;
		break;
	case kMT_Float:
		sprintf(buf, "%0.4f", mp.f);
		str += buf;
		break;
	case kMT_String:
		if (!mp.psz)
			str += "<null>";
		else
			str += mp.psz;
		break;
	case kMT_Vector:
		if (!mp.psz)
			str += "<invalid>";
		else
		{
			sprintf(buf, "(%0.2f,%0.2f,%0.2f)", mp.pVector->x, mp.pVector->y, mp.pVector->z);
			str += buf;
		}
		break;
	case kMT_Undef:
	default:
		sprintf(buf, "<undef:%d>", (int)mp.type);
		str += buf;
		break;
	}
	return str;
}

cAnsiStr cScr_Spy::FormatObject(object iObjId)
{
	cAnsiStr str = "0";
	if (iObjId)
	{
		SInterface<IObjectSystem> pOS(g_pScriptManager);
		const char* pszName = pOS->GetName(iObjId);
		if (pszName)
		{
			str.FmtStr("%s (%d)", pszName, int(iObjId));
		}
		else
		{
			SInterface<ITraitManager> pTM(g_pScriptManager);
			object iArc = pTM->GetArchetype(iObjId);
			pszName = pOS->GetName(iArc);
			if (pszName)
				str.FmtStr("A %s (%d)", pszName, int(iObjId));
			else
				str.FmtStr("%d", int(iObjId));
		}
	}
	return str;
}

inline const char* cScr_Spy::ContainsEvent(int e)
{
	static const char* events[] = { "QueryAdd", "QueryCombine", "Add", "Remove", "Combine" };
	return events[e];
}

STDMETHODIMP cScr_Spy::ReceiveMessage(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace)
{
	cScript::ReceiveMessage(pMsg, pReply, eTrace);

#if (_DARKGAME == 3)
	SService<IShockGameSrv> pShock(g_pScriptManager);
#else
	SService<IDarkUISrv> pUISrv(g_pScriptManager);
#endif
	SService<IDebugScrSrv> pDSS(g_pScriptManager);
	cAnsiStr outstr;
	cAnsiStr obj1 = FormatObject(pMsg->to);
	cAnsiStr obj2 = FormatObject(pMsg->from);
	cAnsiStr datastr = FormatMultiParm(pMsg->data, "")
			+ FormatMultiParm(pMsg->data2, ",")
			+ FormatMultiParm(pMsg->data3, ",");
	outstr.FmtStr(999, "SPY %d.%03d obj %s, src %s: %s\n  {%s}",
			pMsg->time/1000, pMsg->time%1000,
			static_cast<const char*>(obj1), static_cast<const char*>(obj2), pMsg->message,
			static_cast<const char*>(datastr));
#if (_DARKGAME == 3)
	pShock->AddText(static_cast<const char*>(outstr), 0, 3000);
#else
	pUISrv->TextMessage(static_cast<const char*>(outstr), 0, 3000);
#endif
	pDSS->MPrint(static_cast<const char*>(outstr), cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null);
	/*
	 print "SPY obj %d, src %d: %s {%s}"
	 if Sim | DarkGameModeChange : print "SPY obj %d: %s (%s)"
	 if Timer : print "SPY obj %d: %s [%s]"
	 if Difficulty : print SPY obj %d: %s = %d"
	 if *Stimulus : print "SPY obj %d: Stimulated by %s with %s (intensity %f)"
	 if Damage : print "SPY obj %d: Damaged %d pts by %s w/ %d"
	 if Slain : print "SPY obj %d: Slain by %s w/ %d"
	 if Frob* : print "SPY obj %d: %s: %s @%d -> %s @%d for %0.4f %s"
	 if AIModeChange : print "SPY obj %d: AI Mode Change %d -> %d"
	 if Contained : print "SPY obj %d: Contained by %s w/ event %s"
	 if Container : print "SPY obj %d: Object %s Contained w/ event %s"
	 if MediumTransition : print "SPY obj %d: %s %d -> %d"
	 if PickStateChange : print "SPY obj %d: %s %d -> %d"
	 if WaypointReached | MovingTerrainWaypoint : print "SPY obj %d: %s %s"
	*/
	if (!_stricmp(pMsg->message, "Sim"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %s", ObjId(), pMsg->message,
			static_cast<sSimMsg*>(pMsg)->fStarting ? "Starting" : "Stopping");
	}
	else if (!_stricmp(pMsg->message, "DarkGameModeChange"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s%s%s", ObjId(), pMsg->message,
			static_cast<sDarkGameModeScrMsg*>(pMsg)->fResuming ? " Resuming" : "",
			static_cast<sDarkGameModeScrMsg*>(pMsg)->fSuspending ? " Suspending" : "");
	}
	else if (!_stricmp(pMsg->message, "Timer"))
	{
		datastr = FormatMultiParm(pMsg->data, "");
		outstr.FmtStr(999, "SPY obj %d: %s = %s (%s)", ObjId(), pMsg->message,
			static_cast<const char*>(static_cast<sScrTimerMsg*>(pMsg)->name),
			static_cast<const char*>(datastr));
	}
	else if (!_stricmp(pMsg->message, "Difficulty"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s = %d", ObjId(), pMsg->message,
			static_cast<sDiffScrMsg*>(pMsg)->difficulty);
	}
	else if (!_stricmp(pMsg->message, "QuestChange"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s \"%d\"\n  %d -> %d", ObjId(), pMsg->message,
			static_cast<sQuestMsg*>(pMsg)->m_pName,
			static_cast<sQuestMsg*>(pMsg)->m_oldValue,
			static_cast<sQuestMsg*>(pMsg)->m_newValue);
	}
	else if (!_stricmp(pMsg->message, "Damage"))
	{
		obj1 = FormatObject(static_cast<sDamageScrMsg*>(pMsg)->culprit);
		outstr.FmtStr(999, "SPY obj %d: Damaged %d pts by %s w/ %d", ObjId(),
			static_cast<sDamageScrMsg*>(pMsg)->damage,
			static_cast<const char*>(obj1),
			static_cast<sDamageScrMsg*>(pMsg)->kind);
	}
	else if (!_stricmp(pMsg->message, "Slain"))
	{
		obj1 = FormatObject(static_cast<sSlayMsg*>(pMsg)->culprit);
		outstr.FmtStr(999, "SPY obj %d: Slain by %s w/ %d", ObjId(),
			static_cast<const char*>(obj1),
			static_cast<sSlayMsg*>(pMsg)->kind);
	}
	else if (!strnicmp(pMsg->message, "Frob", 4))
	{
		obj1 = FormatObject(static_cast<sFrobMsg*>(pMsg)->SrcObjId);
		obj2 = FormatObject(static_cast<sFrobMsg*>(pMsg)->DstObjId);
		outstr.FmtStr(999, "SPY obj %d: %s by %s\n  %s @%d -> %s @%d for %0.4f%s",
			ObjId(), pMsg->message,
			static_cast<const char*>(FormatObject(static_cast<sFrobMsg*>(pMsg)->Frobber)),
			static_cast<const char*>(obj1), static_cast<sFrobMsg*>(pMsg)->SrcLoc,
			static_cast<const char*>(obj2), static_cast<sFrobMsg*>(pMsg)->DstLoc,
			static_cast<sFrobMsg*>(pMsg)->Sec,
			static_cast<sFrobMsg*>(pMsg)->Abort ? " Aborted" : "");
	}
	else if (!_stricmp(pMsg->message, "Contained"))
	{
		obj1 = FormatObject(static_cast<sContainedScrMsg*>(pMsg)->container);
		outstr.FmtStr(999, "SPY obj %d: Contained by %s w/ event %s", ObjId(),
			static_cast<const char*>(obj1),
			ContainsEvent(static_cast<sContainedScrMsg*>(pMsg)->event));
	}
	else if (!_stricmp(pMsg->message, "Container"))
	{
		obj1 = FormatObject(static_cast<sContainerScrMsg*>(pMsg)->containee);
		outstr.FmtStr(999, "SPY obj %d: Object %s Contained w/ event %s", ObjId(),
			static_cast<const char*>(obj1),
			ContainsEvent(static_cast<sContainerScrMsg*>(pMsg)->event));
	}
	else if (!_stricmp(pMsg->message, "SchemaDone") || !_stricmp(pMsg->message, "SoundDone"))
	{
		obj1 = FormatObject(static_cast<sSchemaDoneMsg*>(pMsg)->targetObject);
		outstr.FmtStr(999, "SPY obj %d: %s %s\n  on %s @(%0.4f,%0.4f,%0.4f)", ObjId(), pMsg->message,
			static_cast<const char*>(static_cast<sSchemaDoneMsg*>(pMsg)->name),
			static_cast<const char*>(obj1),
			static_cast<sSchemaDoneMsg*>(pMsg)->coordinates.x,
			static_cast<sSchemaDoneMsg*>(pMsg)->coordinates.y,
			static_cast<sSchemaDoneMsg*>(pMsg)->coordinates.z);
	}
	else if (!strnicmp(pMsg->message, "Motion", 6))
	{
		outstr.FmtStr(999, "SPY obj %d: %s type %d\n %s [%X]", ObjId(), pMsg->message,
			static_cast<sBodyMsg*>(pMsg)->ActionType,
			static_cast<const char*>(static_cast<sBodyMsg*>(pMsg)->MotionName),
			static_cast<sBodyMsg*>(pMsg)->FlagValue);
	}
	else if (!_stricmp(pMsg->message, "TweqComplete"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s type %d\n Op: %d Dir: %d", ObjId(), pMsg->message,
			static_cast<sTweqMsg*>(pMsg)->Type,
			static_cast<sTweqMsg*>(pMsg)->Op,
			static_cast<sTweqMsg*>(pMsg)->Dir);
	}
	else if (!_stricmp(pMsg->message, "AIModeChange"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %d -> %d", ObjId(), pMsg->message,
			static_cast<sAIModeChangeMsg*>(pMsg)->previous_mode,
			static_cast<sAIModeChangeMsg*>(pMsg)->mode);
	}
	else if (!_stricmp(pMsg->message, "Alertness") || !_stricmp(pMsg->message, "HighAlert"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %d -> %d", ObjId(), pMsg->message,
			static_cast<sAIAlertnessMsg*>(pMsg)->oldLevel,
			static_cast<sAIAlertnessMsg*>(pMsg)->level);
	}
	else if (!_stricmp(pMsg->message, "SignalAI"))
	{
		outstr.FmtStr(999, "SPY obj %d: Signal \"%s\"", ObjId(),
			static_cast<const char*>(static_cast<sAISignalMsg*>(pMsg)->signal));
	}
	else if (!_stricmp(pMsg->message, "PatrolPoint")
	      || !_stricmp(pMsg->message, "MovingTerrainWaypoint")
	      || !_stricmp(pMsg->message, "WaypointReached"))
	{
		// These have similar structures so handle them together
		obj1 = FormatObject(static_cast<sAIPatrolPointMsg*>(pMsg)->patrolObj);
		outstr.FmtStr(999, "SPY obj %d: %s %s", ObjId(), pMsg->message,
			static_cast<const char*>(obj1));
	}
	else if (!_stricmp(pMsg->message, "PhysCollision"))
	{
		obj1 = FormatObject(static_cast<sPhysMsg*>(pMsg)->collObj);
		outstr.FmtStr(999, "SPY obj %d: PhysCollision type %d @%d\n  by %s @%d\n  speed %0.6f (%0.4f,%0.4f,%0.4f)[%0.4f,%0.4f,%0.4f]",
			ObjId(),
			static_cast<sPhysMsg*>(pMsg)->collType,
			static_cast<sPhysMsg*>(pMsg)->Submod,
			static_cast<const char*>(obj1),
			static_cast<sPhysMsg*>(pMsg)->collSubmod,
			static_cast<sPhysMsg*>(pMsg)->collMomentum,
			static_cast<sPhysMsg*>(pMsg)->collPt.x,
			static_cast<sPhysMsg*>(pMsg)->collPt.y,
			static_cast<sPhysMsg*>(pMsg)->collPt.z,
			static_cast<sPhysMsg*>(pMsg)->collNormal.x,
			static_cast<sPhysMsg*>(pMsg)->collNormal.y,
			static_cast<sPhysMsg*>(pMsg)->collNormal.z
			);
	}
	else if (!_stricmp(pMsg->message, "PhysContactCreate") || !_stricmp(pMsg->message, "PhysContactDestroy"))
	{
		obj1 = FormatObject(static_cast<sPhysMsg*>(pMsg)->contactObj);
		outstr.FmtStr(999, "SPY obj %d: %s type %d @%d\n  by %s @%d",
			ObjId(), pMsg->message,
			static_cast<sPhysMsg*>(pMsg)->contactType,
			static_cast<sPhysMsg*>(pMsg)->Submod,
			static_cast<const char*>(obj1),
			static_cast<sPhysMsg*>(pMsg)->contactSubmod);
	}
	else if (!_stricmp(pMsg->message, "PhysEnter") || !_stricmp(pMsg->message, "PhysExit"))
	{
		obj1 = FormatObject(static_cast<sPhysMsg*>(pMsg)->transObj);
		outstr.FmtStr(999, "SPY obj %d: %s @%d\n  by %s @%d",
			ObjId(), pMsg->message,
			static_cast<sPhysMsg*>(pMsg)->Submod,
			static_cast<const char*>(obj1),
			static_cast<sPhysMsg*>(pMsg)->transSubmod);
	}
	else if (strstr(pMsg->message, "RoomEnter") || strstr(pMsg->message, "RoomExit"))
	{
		obj2 = FormatObject(static_cast<sRoomMsg*>(pMsg)->MoveObjId);
		if (static_cast<sRoomMsg*>(pMsg)->FromObjId == ObjId())
		{
			obj1 = FormatObject(static_cast<sRoomMsg*>(pMsg)->ToObjId);
			outstr.FmtStr(999, "SPY obj %d: %s to %s\n  by %s", ObjId(), pMsg->message,
				static_cast<const char*>(obj1),
				static_cast<const char*>(obj2));
		}
		else
		{
			obj1 = FormatObject(static_cast<sRoomMsg*>(pMsg)->FromObjId);
			outstr.FmtStr(999, "SPY obj %d: %s from %s\n  by %s", ObjId(), pMsg->message,
				static_cast<const char*>(obj1),
				static_cast<const char*>(obj2));
		}
	}
	else if (strstr(pMsg->message, "Stimulus"))
	{
		SService<ILinkToolsSrv> pLTS(g_pScriptManager);
		sLink slSource;
		pLTS->LinkGet(static_cast<sStimMsg*>(pMsg)->source, slSource);
		obj1 = FormatObject(slSource.source);
		obj2 = FormatObject(static_cast<sStimMsg*>(pMsg)->stimulus);
		outstr.FmtStr(999, "SPY obj %d: Act/React\n  Stimulated by %s with %s (intensity %0.6f)",
			ObjId(), static_cast<const char*>(obj1), static_cast<const char*>(obj2),
			static_cast<sStimMsg*>(pMsg)->intensity);
	}
	else if (!_stricmp(pMsg->message, "MediumTransition"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %d -> %d",
			ObjId(), pMsg->message,
			static_cast<sMediumTransMsg*>(pMsg)->nFromType,
			static_cast<sMediumTransMsg*>(pMsg)->nToType);
	}
	else if (!_stricmp(pMsg->message, "PickStateChange"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %d -> %d",
			ObjId(), pMsg->message,
			static_cast<sPickStateScrMsg*>(pMsg)->PrevState,
			static_cast<sPickStateScrMsg*>(pMsg)->NewState);
	}
	else if (!_stricmp(pMsg->message, "DHNotify"))
	{
		switch (static_cast<sDHNotifyMsg*>(pMsg)->typeDH)
		{
		case kDH_Property:
			obj1 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sProp.idObj);
			outstr.FmtStr(999, "SPY obj %d: Property \"%s\" event %d %s",
				ObjId(),
				static_cast<sDHNotifyMsg*>(pMsg)->sProp.pszPropName,
				static_cast<sDHNotifyMsg*>(pMsg)->sProp.event,
				static_cast<const char*>(obj1));
			break;
		case kDH_Relation:
			obj1 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sRel.iLinkSource);
			obj2 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sRel.iLinkDest);
			outstr.FmtStr(999, "SPY obj %d: Link %s event %d %s -> %s",
				ObjId(),
				static_cast<sDHNotifyMsg*>(pMsg)->sRel.pszRelName,
				static_cast<sDHNotifyMsg*>(pMsg)->sRel.event,
				static_cast<const char*>(obj1),
				static_cast<const char*>(obj2));
			break;
		case kDH_Object:
			obj1 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sObj.idObj);
			outstr.FmtStr(999, "SPY obj %d: Object event %d %s",
				ObjId(),
				static_cast<sDHNotifyMsg*>(pMsg)->sObj.event,
				static_cast<const char*>(obj1));
			break;
		case kDH_Trait:
			obj1 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sTrait.idObj);
			obj2 = FormatObject(static_cast<sDHNotifyMsg*>(pMsg)->sTrait.idSubj);
			outstr.FmtStr(999, "SPY obj %d: Trait event %d %s -> %s",
				ObjId(),
				static_cast<sDHNotifyMsg*>(pMsg)->sTrait.event,
				static_cast<const char*>(obj1),
				static_cast<const char*>(obj2));
			break;
		default:
			return 0;
		}
	}
	else if (!_stricmp(pMsg->message, "ObjActResult"))
	{
		outstr.FmtStr(999, "SPY obj %d: %s %d -> %d = %d",
			ObjId(), pMsg->message,
			static_cast<sAIObjActResultMsg*>(pMsg)->action,
			static_cast<sAIObjActResultMsg*>(pMsg)->target,
			static_cast<sAIObjActResultMsg*>(pMsg)->result);
	}
	else
		return 0;

#if (_DARKGAME == 3)
	pShock->AddText(static_cast<const char*>(outstr), 0, 3000);
#else
	pUISrv->TextMessage(static_cast<const char*>(outstr), 0, 3000);
#endif
	pDSS->MPrint(static_cast<const char*>(outstr), cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null);

	return 0;
}


/***
 * TrapDelay
 */
void cScr_Delayer::DelayMessage(const char* pszMsg)
{
	if (GetTiming() > 0)
		SetTimedMessage("TrapDelay", GetTiming(), kSTM_OneShot, pszMsg);
	else
		CDSend(pszMsg, ObjId());
	if (GetFlag(kTrapFlagOnce))
		SetLock(true);
}

long cScr_Delayer::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "TrapDelay"))
	{
		const char* pszMsg = pTimerMsg->data;
		if (pszMsg)
			CDSend(pszMsg, ObjId());
		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_Delayer::OnTurnOn(sScrMsg*, cMultiParm&)
{
	InitTrapVars();
	if (! IsLocked() && ! GetFlag(kTrapFlagNoOn))
	{
		if (GetFlag(kTrapFlagInvert))
			DelayMessage("TurnOff");
		else
			DelayMessage("TurnOn");
	}

	return 0;
}

long cScr_Delayer::OnTurnOff(sScrMsg*, cMultiParm&)
{
	InitTrapVars();
	if (! IsLocked() && ! GetFlag(kTrapFlagNoOff))
	{
		if (GetFlag(kTrapFlagInvert))
			DelayMessage("TurnOn");
		else
			DelayMessage("TurnOff");
	}

	return 0;
}

long cScr_Delayer::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitTrapVars();
	if (! IsLocked())
		DelayMessage(pMsg->message);

	return cBaseTrap::OnMessage(pMsg, mpReply);
}


/***
 * TrapFlipFlop
 */
long cScr_FlipFlop::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iState.Init(0);
	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_FlipFlop::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_FlipFlop::OnTurnOn(sScrMsg* pMsg, cMultiParm&)
{
	if (m_iState == 0)
	{
		DoTrigger(true, pMsg->data);
		m_iState = 1;
	}
	else
	{
		DoTrigger(false, pMsg->data);
		m_iState = 0;
	}
	return 0;
}

long cScr_FlipFlop::OnTurnOff(sScrMsg*, cMultiParm&)
{
	return 0;
}

/***
 * TrapCinema
 */
long cScr_PlayMovie::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		char* pszMovie = GetObjectParamString(ObjId(), "movie");
		if (pszMovie)
		{
#if (_DARKGAME == 3)
			SService<IShockGameSrv> pShock(g_pScriptManager);
			pShock->PlayVideo(pszMovie);
#else
			SService<IDebugScrSrv> pDSS(g_pScriptManager);
			pDSS->Command("movie", pszMovie, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null);
#endif
			g_pMalloc->Free(pszMovie);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapQVarText
 */
long cScr_TrapQVText::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
		Display(0);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * QVarPlaque
 */
long cScr_FrobQVText::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (pNet->IsPlayer(pFrobMsg->Frobber))
		Display(pFrobMsg->Frobber);
	else
#endif
		Display(0);

	return cBaseScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}

/***
 * QVarScroll
 */
long cScr_InvQVText::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	Display(pFrobMsg->Frobber);

	return cBaseScript::OnFrobInvEnd(pFrobMsg, mpReply);
}


/***
 * TrapMissionQVar
 */
int cScr_TrapQVarMis::GetQVar(const char* pszName)
{
	return cQVarProcessor::GetQVar(ObjId(), pszName);
}

void cScr_TrapQVarMis::SetQVar(const char* pszName, int iVal)
{
	cQVarProcessor::SetQVar(ObjId(), pszName, iVal, false);
}

long cScr_TrapQVarMis::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		char* pszInit = GetObjectParamString(ObjId(), "initqv");
		if (pszInit)
		{
			char* pszName = NULL;
			auto_ptr<char> pszParam (GetQVarParams(ObjId(), NULL, NULL, &pszName));
			if (pszName)
			{
				int iInit;
				if (pszInit[0] == '$')
					iInit = GetQVar(pszInit+1);
				else
					iInit = strtol(pszInit, NULL, 0);
				SetQVar(pszName, iInit);
			}
			g_pMalloc->Free(pszInit);
		}
	}

	return cBaseTrap::OnSim(pSimMsg, mpReply);
}

long cScr_TrapQVarMis::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	char cOperation;
	int iValue = 0;
	char* pszName = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), &cOperation, &iValue, &pszName));
	if (pszName)
		SetQVar(pszName, TrapProcess(bTurnOn, cOperation, iValue, GetQVar(pszName)));

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapCampaignQVar
 */
void cScr_TrapQVarCmp::SetQVar(const char* pszName, int iVal)
{
	cQVarProcessor::SetQVar(ObjId(), pszName, iVal, true);
}

long cScr_TrapQVarCmp::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
#if (_DARKGAME != 3)
	if (! pSimMsg->fStarting)
	{
		char* pszName = NULL;
		auto_ptr<char> pszParam (GetQVarParams(ObjId(), NULL, NULL, &pszName));
		if (pszName)
		{
			SService<IQuestSrv> pQS(g_pScriptManager);
			if (pQS->Exists(pszName))
			{
				int iVal = pQS->Get(pszName);
				pQS->Delete(pszName);
				pQS->Set(pszName, iVal, kQuestDataCampaign);
			}
		}
	}
#endif
	return cScr_TrapQVarMis::OnSim(pSimMsg, mpReply);
}


/***
 * TrigQVar
 */
long cScr_TrigQVar::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (!pNet->IsProxy(ObjId()))
	{
#endif
	char* pszQVar = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), NULL, NULL, &pszQVar));
	if (pszQVar)
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		pQS->SubscribeMsg(ObjId(), pszQVar, kQuestDataAny);
	}
#if (_DARKGAME == 3)
	}
#endif
	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_TrigQVar::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (!pNet->IsProxy(ObjId()))
	{
#endif
	char* pszQVar = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), NULL, NULL, &pszQVar));
	if (pszQVar)
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		pQS->UnsubscribeMsg(ObjId(), pszQVar);
	}
#if (_DARKGAME == 3)
	}
#endif

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_TrigQVar::OnQuestChange(sQuestMsg* pQuestMsg, cMultiParm& mpReply)
{
	char cOp;
	int iArg = 0;
	char* pszQVar = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), &cOp, &iArg, &pszQVar));
	if (pszQVar)
	{
		if (!_stricmp(pQuestMsg->m_pName, pszQVar))
		{
			bool oldcondition = TrigProcess(cOp, iArg, pQuestMsg->m_oldValue, pszParam.get()+1);
			bool newcondition = TrigProcess(cOp, iArg, pQuestMsg->m_newValue, pszParam.get()+1);
			if (newcondition != oldcondition)
			{
				DoTrigger(newcondition);
			}
		}
	}

	return cBaseTrap::OnQuestChange(pQuestMsg, mpReply);
}


/***
 * TrigQVarChange
 */
char* cScr_TrigQVarChange::GetQVar(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(ObjId(),
#if (_DARKGAME == 3)
		"QBName"
#else
		"TrapQVar"
#endif
	))
		return NULL;
	cMultiParm mpQVar;
	pPS->Get(mpQVar, ObjId(),
#if (_DARKGAME == 3)
		"QBName",
#else
		"TrapQVar",
#endif
		NULL);

	const char* pszProp = mpQVar;
	if (!pszProp)
		return NULL;
	char* pszQVar = new char[strlen(pszProp)+1];
	strcpy(pszQVar, pszProp);
	return pszQVar;
}

long cScr_TrigQVarChange::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (!pNet->IsProxy(ObjId()))
	{
#endif
	auto_ptr<char> pszQVar (GetQVar());
	if (pszQVar.get())
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		pQS->SubscribeMsg(ObjId(), pszQVar.get(), kQuestDataAny);
	}
#if (_DARKGAME == 3)
	}
#endif
	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_TrigQVarChange::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (!pNet->IsProxy(ObjId()))
	{
#endif
	auto_ptr<char> pszQVar (GetQVar());
	if (pszQVar.get())
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		pQS->UnsubscribeMsg(ObjId(), pszQVar.get());
	}
#if (_DARKGAME == 3)
	}
#endif

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_TrigQVarChange::OnQuestChange(sQuestMsg* pQuestMsg, cMultiParm& mpReply)
{
	auto_ptr<char> pszQVar (GetQVar());
	if (pszQVar.get())
	{
		if (!_stricmp(pQuestMsg->m_pName, pszQVar.get())
		 && pQuestMsg->m_oldValue != pQuestMsg->m_newValue)
		{
			DoTrigger(true);
		}
	}

	return cBaseTrap::OnQuestChange(pQuestMsg, mpReply);
}


/***
 * TrapQVarFilter
 */
long cScr_TrapQVarRelay::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	char cOp;
	int iArg = 0;
	char* pszQVar = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), &cOp, &iArg, &pszQVar));
	if (pszQVar)
	{
		if (TrigProcess(cOp, iArg, GetQVar(ObjId(), pszQVar), pszParam.get()+1))
		{
			DirectTrigger(bTurnOn, pMsg->data);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * BasePostRead
 */
bool cScr_PostReader::DoVoiceOver(object iFrobber)
{
	object iSchema = 0;
#if (_DARKGAME == 3)
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ObjSoundName"))
	{
		cMultiParm mpSoundName;
		pPS->Get(mpSoundName, ObjId(), "ObjSoundName", NULL);
		iSchema = StrToObject(mpSoundName);
	}
#else
	sLink slSchema;
	if (!GetOneLinkInheritedSrc("SoundDescription", ObjId(), 0, &slSchema))
		return false;
	iSchema = slSchema.dest;
#endif

	if (iSchema && PlayVoiceOver(ObjId(), iSchema))
	{
		/* Voice overs don't generate a SchemaDone messages,
		 * So we have to fall back on a boring ol' timer.
		 * We'd have to anyway, because the sound service doesn't
		 * report whether the sound was actually played or not,
		 * just if the queueing was successful.
		 */
		SetTiming(1500);
		InitTrapVars();
		SetTimedMessage("PostSchema", GetTiming(), kSTM_OneShot, iFrobber);
		SetTiming(0);
		return true;
	}

	return false;
}

bool cScr_PostReader::DoQVar(void)
{
	char cOperation;
	int iValue = 0;
	char* pszName = NULL;
	auto_ptr<char> pszParam (GetQVarParams(ObjId(), &cOperation, &iValue, &pszName));
	if (!pszName)
	{
		return false;
	}

	SetQVar(ObjId(), pszName, TrapProcess(true, cOperation, iValue, GetQVar(ObjId(), pszName)));
	return true;
}

void cScr_PostReader::Read(object iFrobber)
{
	SetTimedMessage("PostRead", 500, kSTM_OneShot, iFrobber);
	ShowBook(ObjId(), true);
}

long cScr_PostReader::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "PostRead"))
	{
		if (!DoVoiceOver(pTimerMsg->data))
		{
			DoQVar();
		}
		DoTrigger(true, pTimerMsg->data);
		return 0;
	}
	if (!strcmp(pTimerMsg->name, "PostSchema"))
	{
		DoQVar();
		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}

long cScr_PostReader::OnTurnOn(sScrMsg*, cMultiParm&)
{
	return 0;
}

long cScr_PostReader::OnTurnOff(sScrMsg*, cMultiParm&)
{
	return 0;
}


/***
 * PostReadPlaque
 */
long cScr_FrobPostRead::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	Read(pFrobMsg->Frobber);

	return cScr_PostReader::OnFrobWorldEnd(pFrobMsg, mpReply);
}


/***
 * PostReadScroll
 */
long cScr_InvPostRead::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	Read(pFrobMsg->Frobber);

	return cScr_PostReader::OnFrobInvEnd(pFrobMsg, mpReply);
}


/***
 * TrigWaypoint
 */
long cScr_TrigTerr::OnWaypointReached(sWaypointMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseMovingTerrainScript::OnWaypointReached(pMsg, mpReply);
}


/***
 * TrapDestroyDoor
 */
int cScr_DoorKillerTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void*)
{
	SService<IDoorSrv> pDS(g_pScriptManager);
	SService<IObjectSrv> pOS(g_pScriptManager);

	sLink sl;
	pLQ->Link(&sl);

	pDS->OpenDoor(sl.dest);
	pOS->Destroy(sl.dest);

	return 1;
}

long cScr_DoorKillerTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		IterateLinks(g_pszCDLinkFlavor, ObjId(), 0, LinkIter, this, NULL);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrigOBBCreature
 */
long cScr_OBBCret::OnPhysEnter(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oCret;
#if (_DARKGAME == 3)
	pOS->Named(oCret, "Monsters");
#else
	pOS->Named(oCret, "Creature");
#endif

	true_bool bRelated;
	if (*pOS->InheritsFrom(bRelated, pPhysMsg->transObj, oCret))
	{
		if (TrackCreatureEnter(pPhysMsg->transObj) && Population() == 1)
			DoTrigger(true);
	}

	return cBaseTrap::OnPhysEnter(pPhysMsg, mpReply);
}

long cScr_OBBCret::OnPhysExit(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oCret;
#if (_DARKGAME == 3)
	pOS->Named(oCret, "Monsters");
#else
	pOS->Named(oCret, "Creature");
#endif

	true_bool bRelated;
	if (*pOS->InheritsFrom(bRelated, pPhysMsg->transObj, oCret))
	{
		if (TrackCreatureExit(pPhysMsg->transObj) && Population() == 0)
			DoTrigger(false);
	}

	return cBaseTrap::OnPhysExit(pPhysMsg, mpReply);
}


/***
 * TrapFader
 */
long cScr_FadeTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	int iTime = GetObjectParamTime(ObjId(), "fade_time", GetTiming());
#if (_DARKGAME == 3)
	SService<INetworkingSrv> pNet(g_pScriptManager);
	pNet->Broadcast(ObjId(), "NetFade", 0, bTurnOn);

	ulong iColor = 0;
	char* pszColor = GetObjectParamString(ObjId(), "fade_color");
	if (pszColor)
	{
		iColor = strtocolor(pszColor);
		g_pMalloc->Free(pszColor);
	}
	SService<IShockGameSrv> pShock(g_pScriptManager);
	if (bTurnOn)
	{
		pShock->StartFadeOut(iTime, getred(iColor), getgreen(iColor), getblue(iColor));
	}
	else
	{
		pShock->StartFadeIn(iTime, getred(iColor), getgreen(iColor), getblue(iColor));
	}
#elif (_DARKGAME == 2)
	SService<IDarkGameSrv> pDark(g_pScriptManager);
	if (bTurnOn)
	{
		pDark->FadeToBlack(float(iTime) / 1000.0f);
	}
	else
	{
		SInterface<IObjectSystem> pOS(g_pScriptManager);
		SInterface<ITraitManager> pTM(g_pScriptManager);

		object iPlayer = pOS->GetObjectNamed("Player");
		object iAvatar = pTM->GetArchetype(iPlayer);
		object iFlash = pOS->GetObjectNamed("renderflash");
		if (iAvatar && iFlash)
		{
			// Step 1. Create a new archetype
			iFlash = pOS->BeginCreate(iFlash, 0);
			if (! iFlash)
			{
				pDark->FadeToBlack(-1.0f);
				return 0;
			}
			// Step 2. Set the RenderFlash property
			{
				SInterface<IPropertyManager> pPM(g_pScriptManager);
				SInterface<IRendFlashProperty> pRendFlashProp =
						static_cast<IRendFlashProperty*>(pPM->GetPropertyNamed("RenderFlash"));
				if (! pRendFlashProp)
				{
					pDark->FadeToBlack(-1.0f);
					return 0;
				}
				sRenderFlash data;
				data.active = data.blue = data.green = data.red = 0;
				data.starttime = data.worldduration = data.screenduration = 0;
				data.range = data.time = 0;
				data.effectduration = iTime;
				pRendFlashProp->Set(iFlash, &data);
			}
			pOS->SetObjTransience(iFlash, TRUE);
			pOS->EndCreate(iFlash);
			// Step 3. Link the avatar to the new flash
			SService<ILinkSrv> pLS(g_pScriptManager);
			SService<ILinkToolsSrv> pLTS(g_pScriptManager);
			linkkind flavor = pLTS->LinkKindNamed("RenderFlash");
			object iOrigFlash;
			link lRendFlash;
			pLS->GetOne(lRendFlash, flavor, iAvatar, 0);
			if (lRendFlash)
			{
				sLink sl;
				pLTS->LinkGet(lRendFlash, sl);
				iOrigFlash = sl.dest;
				pLS->Destroy(lRendFlash);
			}
			pLS->Create(lRendFlash, flavor, iAvatar, iFlash);
			// Step 4. Activate the flash
			{
				SService<ICameraSrv> pCam(g_pScriptManager);
				pCam->StaticAttach(iPlayer);
				pCam->CameraReturn(iPlayer);
			}
			// Don't forget to undo the real fade effect
			pDark->FadeToBlack(-1.0f);
			// Step 5. Clean up the mess we just made
			pLS->Destroy(lRendFlash);
			if (iOrigFlash)
				pLS->Create(lRendFlash, flavor, iAvatar, iOrigFlash);
			// The created flash object will destroy itself
		}
	}
#else // _DARKGAME == 1
	SService<IDarkGameSrv> pDark(g_pScriptManager);
	if (bTurnOn)
	{
		pDark->FadeToBlack(float(iTime) / 1000.0f);
	}
	else
	{
		pDark->FadeToBlack(-1.0f);
	}
#endif

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

#if (_DARKGAME == 3)
long cScr_FadeTrap::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "NetFade"))
	{
		int iTime = GetObjectParamTime(ObjId(), "fade_time", GetTiming());
		ulong iColor = 0;
		char* pszColor = GetObjectParamString(ObjId(), "fade_color");
		if (pszColor)
		{
			iColor = strtocolor(pszColor);
			g_pMalloc->Free(pszColor);
		}
		SService<IShockGameSrv> pShock(g_pScriptManager);
		if (pMsg->data)
		{
			pShock->StartFadeOut(iTime, getred(iColor), getgreen(iColor), getblue(iColor));
		}
		else
		{
			pShock->StartFadeIn(iTime, getred(iColor), getgreen(iColor), getblue(iColor));
		}
		return 0;
	}
	return cBaseTrap::OnMessage(pMsg, mpReply);
}
#endif


/***
 * Transmogrify
 */
#if (_DARKGAME == 3)
#define TRANSMOGRIFY_LINK "Mutate"
#else
#define TRANSMOGRIFY_LINK "Transmute"
#endif
void cScr_Transmogrify::DoContain(object iPlayer)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	sLink slNew;
	object oNew;
	if (GetOneLinkInheritedSrc(TRANSMOGRIFY_LINK, ObjId(), 0, &slNew))
	{
		pOS->Create(oNew, slNew.dest);
		if (oNew)
		{
			SService<IPropertySrv> pPS(g_pScriptManager);
			if (pPS->Possessed(ObjId(), "StackCount"))
			{
				cMultiParm mpStack;
				pPS->Get(mpStack, ObjId(), "StackCount", NULL);
				pPS->SetSimple(oNew, "StackCount", mpStack);
			}
#if (_DARKGAME == 3)
			SService<IShockGameSrv> pShock(g_pScriptManager);
			pShock->DestroyInvObj(ObjId());
			pShock->AddInvObj(oNew);
			pShock->RefreshInv();
#else
			SService<IContainSrv> pContSrv(g_pScriptManager);
			pContSrv->Add(oNew, iPlayer, 0, 1);
			PostMessage(ObjId(), "TransSelect", slNew.dest);
			// Not yet, have to wrestle with the 'Crystal' archetype hack.
			//pOS->Destroy(ObjId());
#endif
		}
	}
}

long cScr_Transmogrify::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->event == kContainAdd)
	{
#if (_DARKGAME == 3)
		SService<INetworkingSrv> pNet(g_pScriptManager);
		if (pNet->IsPlayer(pContMsg->container))
		{
			if (pNet->IsProxy(pContMsg->container))
				pNet->SendToProxy(pContMsg->container, ObjId(), "NetTransmogrify", pContMsg->container);
			else
				DoContain(pContMsg->container);
		}
#else
		SService<IObjectSrv> pOS(g_pScriptManager);
		object oPlayer;
		pOS->Named(oPlayer, "Player");
		if (oPlayer == pContMsg->container)
			DoContain(pContMsg->container);
#endif
	}

	return cBaseScript::OnContained(pContMsg, mpReply);
}

long cScr_Transmogrify::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
#if (_DARKGAME == 3)
	if (!_stricmp(pMsg->message, "NetTransmogrify"))
	{
		DoContain(pMsg->data);
		return 0;
	}
	else
#endif
	if (!_stricmp(pMsg->message, "TransSelect"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
		SService<IDebugScrSrv> pDSS(g_pScriptManager);
		cScrStr strName;
		pOS->GetName(strName, int(pMsg->data));
		pDSS->Command("inv_select", strName, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null, cScrStr::Null);
		strName.Free();
		return 0;
	}
	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * TrapRequireOne
 */
long cScr_RequireOneTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	ulong iOnce = GetFlags() & kTrapFlagOnce;
	UnsetFlag(kTrapFlagOnce);
	bool bRealOn = bTurnOn != GetFlag(kTrapFlagInvert);
	if (bRealOn)
	{
		if (TurnOn(pMsg->from))
		{
			DirectTrigger(bool(1 == (Requirements() & 1)) != GetFlag(kTrapFlagInvert), pMsg->data);
			SetFlag(iOnce);
		}
	}
	else
	{
		if (TurnOff(pMsg->from))
		{
			DirectTrigger(bool(1 == (Requirements() & 1)) != GetFlag(kTrapFlagInvert), pMsg->data);
			SetFlag(iOnce);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapRequireOneOnly
 */
long cScr_RequireSingleTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	ulong iOnce = GetFlags() & kTrapFlagOnce;
	UnsetFlag(kTrapFlagOnce);
	bool bRealOn = bTurnOn != GetFlag(kTrapFlagInvert);
	if (bRealOn)
	{
		if (TurnOn(pMsg->from))
		{
			DirectTrigger(bool(1 == Requirements()) != GetFlag(kTrapFlagInvert), pMsg->data);
			SetFlag(iOnce);
		}
	}
	else
	{
		if (TurnOff(pMsg->from))
		{
			DirectTrigger(bool(1 == Requirements()) != GetFlag(kTrapFlagInvert), pMsg->data);
			SetFlag(iOnce);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapSquelch
 */
long cScr_Squelch::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	int iTiming = GetTiming();
	SetTiming(0);
	if (IsScriptDataSet("LastOn"))
	{
		cMultiParm mpOn = GetScriptData("LastOn");
		if (bool(mpOn) == bTurnOn)
		{
			cMultiParm mpTime = GetScriptData("LastTime");
			if ((pMsg->time - int(mpTime)) <= iTiming)
				return 0;
		}
	}
	SetScriptData("LastOn", bTurnOn);
	SetScriptData("LastTime", pMsg->time);
	DirectTrigger(bTurnOn, pMsg->data);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * PropertyScript
 */
char const* const* cScr_PropertyTrap::GetFieldNames(char* names)
{
	if (!names || !*names)
		return NULL;
	char** fields;
	int n = 1;
	char* p = names;
	while ((p = strchr(p, ','))) ++n;
	fields = new char*[n+1];
	fields[n] = NULL;
	for (p = names, n = 0; p; n++)
	{
		while (isspace(*p)) ++p;
		fields[n] = p;
		p = strchr(p, ',');
		char* q;
		if (p)
			q = p++;
		else
			q = fields[n] + strlen(fields[n]);
		while (isspace(*(q-1))) --q;
		*q = '\0';
	}
	return fields;
}

int cScr_PropertyTrap::MatchFieldName(const char* name, const sFieldDesc* fields, int num_fields)
{
	ulong len = strlen(name);
	int num = 0;
	int i;
	char *p;
	p = strchr(name, '[');
	if (p)
	{
		len = p++ - name;
		num = strtol(p, NULL, 10);
		if (num < 1) num = 1;
	}
	if (num > num_fields)
		num = num_fields;
	if (len == 0)
		return num-1;
	for (i = 0; i < num_fields; i++)
	{
		if (!strnalnumcmp(name, fields[i].name, len))
		{
			if (--num == 0)
				return i;
		}
	}
	return -1;
}

bool cScr_PropertyTrap::ParsePropertyFields(IStructDescTools* pSD, const sStructDesc* pSDesc,
				void* pData, char* pszValues, char const* const* pszFields)
{
	int maxn, n = 0;
	char* pszVal = pszValues;
	if (pszFields)
	{
		maxn = 0;
		while (pszFields[maxn])
			++maxn;
	}
	else
		maxn = pSDesc->num_fields;
	for (pszVal = pszValues; pszVal && *pszVal && n < maxn; ++n)
	{
		char* psz;
		if (*pszVal == '(' && (psz = strchr(pszVal, ')')))
		{
			if (++pszVal == psz)
			{
				psz = strchr(psz, ',');
				if (psz) ++psz;
		DebugPrintf("PropTrap:    skipping field %s", pszFields?pszFields[n]:pSDesc->fields[n].name);
				pszVal = psz;
				continue;
			}
			*psz++ = '\0';
			psz = strchr(psz, ',');
			if (psz) ++psz;
		}
		else
		{
			psz = strchr(pszVal, ',');
			if (psz)
				*psz++ = '\0';
		}
		DebugPrintf("PropTrap:    setting field %s: %s", pszFields?pszFields[n]:pSDesc->fields[n].name, pszVal);
		if (pszFields)
		{
			int f = MatchFieldName(pszFields[n], pSDesc->fields, pSDesc->num_fields);
			if (f == -1 || pSD->ParseField(&pSDesc->fields[f], pszVal, pData) != S_OK)
				return false;
		}
		else
		{
			if (pSD->ParseField(&pSDesc->fields[n], pszVal, pData) != S_OK)
				return false;
		}
		pszVal = psz;
	}
	return true;
}

bool cScr_PropertyTrap::ParseProperty(int iObjId, IProperty* pProp, const char* pszTypeName,
				char* pszValues, char const* const* pszFields)
{
	SInterface<IStructDescTools> pSD(g_pScriptManager);
	SInterface<IPropertyStore> pStore(pProp);
	sDatum dat;
	void* pData;
	bool created = !pStore->Relevant(iObjId);
	if (created)
	{
		SInterface<ITrait> pTrait(pProp);
		int inherit = pTrait->GetDonor(iObjId);
		if (inherit)
			pStore->Copy(dat, iObjId, inherit);
		else
			pStore->Create(dat, iObjId);
	}
	// Operate on a copy of the data in case parsing fails.
	pStore->GetCopy(iObjId, &dat);
	SInterface<IDataOps> pOps(pStore->GetOps());
	if (pOps && pOps->BlockSize(dat) >= 0)
		pData = dat.pv;
	else
		pData = &dat;

	const sStructDesc* pSDesc = pSD->Lookup(pszTypeName);
	if (pSDesc->num_fields > 0)
	{
		if (!ParsePropertyFields(pSD, pSDesc, pData, pszValues, pszFields))
		{
			pStore->ReleaseCopy(iObjId, dat);
			if (created)
				pStore->Delete(iObjId);
			return false;
		}
	}

	pStore->Set(iObjId, dat);
	pStore->ReleaseCopy(iObjId, dat);
	pProp->Touch(iObjId);
	return true;
}

bool cScr_PropertyTrap::ParseStringProperty(int iObjId, IProperty* pProp, char* pszValue)
{
	try
	{
		SInterface<IStringProperty> pSProp(pProp);
		DebugPrintf("PropTrap:    setting string: %s", pszValue);
		if (pSProp->Set(iObjId, pszValue) == S_OK)
			return true;
	}
	catch (no_interface&)
	{
	}
	return false;
}

struct PropTrap_params
{
	IProperty* pProp;
	char* pszValues;
	char const* const* pszFields;
};

int cScr_PropertyTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_PropertyTrap* scrPropTrap = static_cast<cScr_PropertyTrap*>(pScript);
	PropTrap_params* params = reinterpret_cast<PropTrap_params*>(pData);
	sLink sl;
	pLQ->Link(&sl);
	const sPropertyTypeDesc* pTypeDesc = params->pProp->DescribeType();
	DebugPrintf("PropTrap: Setting property %s on %d.", pTypeDesc->szTypeName, int(sl.dest));
	if (pTypeDesc->uTypeSize == 0)
		scrPropTrap->ParseStringProperty(sl.dest, params->pProp, params->pszValues);
	else
		scrPropTrap->ParseProperty(sl.dest, params->pProp, pTypeDesc->szTypeName, params->pszValues, params->pszFields);
	return 1;
}

long cScr_PropertyTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	char* pszPropName = GetObjectParamString(ObjId(), "property");
	char* pszValues = GetObjectParamString(ObjId(), bTurnOn ? "onvalue" : "offvalue");
	char* pszFields = GetObjectParamString(ObjId(), "fields");
	if (pszPropName && pszValues)
	{
		SInterface<IPropertyManager> pPM(g_pScriptManager);
		SInterface<IProperty> pProp = pPM->GetPropertyNamed(pszPropName);
		if (pProp && pProp->GetID() != -1)
		{
			PropTrap_params data = {pProp.get(), pszValues, GetFieldNames(pszFields)};
			IterateLinks(g_pszCDLinkFlavor, ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(&data));
			delete[] data.pszFields;
		}
	}
	if (pszFields)
		g_pMalloc->Free(pszFields);
	if (pszValues)
		g_pMalloc->Free(pszValues);
	if (pszPropName)
		g_pMalloc->Free(pszPropName);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * PropertyScript
 */
int cScr_SimplePropertyTrap::LinkIterOn(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	IPropertyManager* pPM = static_cast<IPropertyManager*>(pData);
	cScr_SimplePropertyTrap* scrPropTrap = static_cast<cScr_SimplePropertyTrap*>(pScript);
	const char* pszLinkData = static_cast<const char*>(pLQ->Data());
	if (!pszLinkData)
		return 1;
	sLink sl;
	pLQ->Link(&sl);
	if (pszLinkData[0] == '@')
	{
		SInterface<IObjectSystem> pOS(g_pScriptManager);
		object iSrc = pOS->GetObjectNamed(pszLinkData+1);
		if (iSrc)
			pOS->CloneObject(sl.dest, iSrc);
		return 1;
	}
	SInterface<IProperty> pProp = pPM->GetPropertyNamed(pszLinkData);
	if (pProp && pProp->GetID() != -1)
	{
		if (pProp->IsRelevant(scrPropTrap->ObjId()))
			pProp->Copy(sl.dest, scrPropTrap->ObjId());
		else
			pProp->Create(sl.dest);
	}

	return 1;
}

int cScr_SimplePropertyTrap::LinkIterOff(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	IPropertyManager* pPM = static_cast<IPropertyManager*>(pData);
	const char* pszLinkData = static_cast<const char*>(pLQ->Data());
	if (!pszLinkData || pszLinkData[0] == '@')
		return 1;
	sLink sl;
	pLQ->Link(&sl);
	SInterface<IProperty> pProp = pPM->GetPropertyNamed(pszLinkData);
	if (pProp && pProp->GetID() != -1)
	{
		pProp->Delete(sl.dest);
	}

	return 1;
}

long cScr_SimplePropertyTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SInterface<IPropertyManager> pPM(g_pScriptManager);
	IterateLinks("ScriptParams", ObjId(), 0, bTurnOn ? LinkIterOn : LinkIterOff, this, pPM.get());

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


