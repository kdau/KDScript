/******************************************************************************
 *  SS2Scripts.cpp
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
#if (_DARKGAME == 3)
#include "SS2Scripts.h"
#include "ScriptModule.h"

#include <lg/types.h>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include <lg/objects.h>
#include <ScriptLib.h>
#include "utils.h"

#include <cstring>

using namespace std;


/***
 * NotifyRegion
 */
int cScr_NotifyRegion::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void*)
{
	sLink sl;
	pLQ->Link(&sl);
	static_cast<cScr_NotifyRegion*>(pScript)->PostMessage(sl.dest, "Obituary");

	return 1;
}

long cScr_NotifyRegion::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	IterateLinksByData("ScriptParams", ObjId(), 0, "Population", -1, LinkIter, this, NULL);

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}

/***
 * StdParticleGroup
 */
long cScr_StdParticleGroup::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPGroupSrv> pSFX(g_pScriptManager);
	pSFX->SetActive(ObjId(), 1);

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_StdParticleGroup::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPGroupSrv> pSFX(g_pScriptManager);
	pSFX->SetActive(ObjId(), 0);

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

long cScr_StdParticleGroup::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Die"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}

/***
 * TrapCapacitor
 */
long cScr_TrapCapacitor::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "Decharge"))
	{
		ClearScriptData("TimerHandle");

		if (IsScriptDataSet("LastMsg"))
		{
			int iMsg = GetScriptData("LastMsg");
			int iFrobber = GetScriptData("LastFrobber");
			DirectTrigger(iMsg, iFrobber);
		}

		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_TrapCapacitor::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsScriptDataSet("TimerHandle"))
	{
		SetScriptData("LastMsg", static_cast<int>(bTurnOn));
		SetScriptData("LastFrobber", static_cast<int>(pMsg->data));
	}
	else
	{
		DirectTrigger(bTurnOn, pMsg->data);
		if (bTurnOn)
		{
			int iDischarge = 30000;
			SService<IPropertySrv> pPS(g_pScriptManager);
			if (pPS->Possessed(ObjId(), "DelayTime"))
				// cBaseTrap already read it for us.
				iDischarge = GetTiming();

			tScrTimer hTimer = SetTimedMessage("Decharge", iDischarge, kSTM_OneShot, pMsg->data);
			SetScriptData("TimerHandle", reinterpret_cast<int>(hTimer));
			ClearScriptData("LastMsg");
			ClearScriptData("LastFrobber");
		}
	}
	if (bTurnOn)
		// Don't let cBaseTrap set a timer.
		SetTiming(0);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapConverse
 */
long cScr_TrapConverse::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		SService<IAIScrSrv> pAI(g_pScriptManager);
		true_bool _p;
		pAI->StartConversation(_p, ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapPatrol
 */
int cScr_TrapPatrol::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	sLink sl;
	pLQ->Link(&sl);
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(sl.dest, "AI_Patrol"))
		pPS->Add(sl.dest, "AI_Patrol");
	pPS->SetSimple(sl.dest, "AI_Patrol", reinterpret_cast<int>(pData));

	return 1;
}

long cScr_TrapPatrol::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	IterateLinks("SwitchLink", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(int(bTurnOn)));

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapRevert
 */
long cScr_TrapRevert::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	long iRet = cBaseTrap::OnTurnOff(pMsg, mpReply);

	if (GetTiming() > 0)
	{
		SetTimedMessage("TrapTimer", GetTiming(),
				kSTM_OneShot, GetFlag(kTrapFlagInvert)?0:1);
	}
	return iRet;
}

long cScr_TrapRevert::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	DirectTrigger(bTurnOn, pMsg->data);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapTexture
 */
long cScr_TrapTexture::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	char* pszNew = GetObjectParamString(ObjId(), "TerrRepOn");
	char* pszOld = GetObjectParamString(ObjId(), "TerrRepOff");
	if (pszNew && pszOld)
	{
DebugPrintf("Swapping Texture %s %s %s", pszOld, (bTurnOn ? "->" : "<-"), pszNew);
		SService<IAnimTextureSrv> pTS(g_pScriptManager);
		if (bTurnOn)
			pTS->ChangeTexture(ObjId(), NULL, pszOld, NULL, pszNew);
		else
			pTS->ChangeTexture(ObjId(), NULL, pszNew, NULL, pszOld);
	}
	g_pMalloc->Free(pszOld);
	g_pMalloc->Free(pszNew);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrigAIAlert
 */
long cScr_TrigAIAlert::OnAlertness(sAIAlertnessMsg* pAlertMsg, cMultiParm& mpReply)
{
	if (pAlertMsg->level == kHighAlert)
		CDSend("TurnOn", ObjId());

	return cBaseAIScript::OnAlertness(pAlertMsg, mpReply);
}

/***
 * TrigWorldFrob
 */
long cScr_TrigWorldFrob::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	DoTrigger(true, pFrobMsg->Frobber);

	return cBaseTrap::OnFrobWorldEnd(pFrobMsg, mpReply);
}

/***
 * TrigWorldFocus
 */
long cScr_TrigWorldFocus::OnWorldSelect(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DoTrigger(true);

	return cBaseTrap::OnWorldSelect(pMsg, mpReply);
}

long cScr_TrigWorldFocus::OnWorldDeSelect(sScrMsg* pMsg, cMultiParm& mpReply)
{
	DoTrigger(false);

	return cBaseTrap::OnWorldDeSelect(pMsg, mpReply);
}

/***
 * TrigInvFrob
 */
long cScr_TrigInvFrob::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	DoTrigger(true, pFrobMsg->Frobber);

	return cBaseTrap::OnFrobInvEnd(pFrobMsg, mpReply);
}

/***
 * TrigFlicker
 */
void cScr_TrigFlicker::Blink(bool on)
{
	if (m_hSafety)
	{
		KillTimedMessage(m_hSafety);
		m_hSafety = NULL;
	}

	DirectTrigger(on);
	m_iFlicker = on;

	if (on)
	{
		int iSafety = 15000;
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "CfgTweqBlink"))
		{
			cMultiParm mpRate;
			pPS->Get(mpRate, ObjId(), "CfgTweqBlink", "Rate");
			iSafety = static_cast<int>(mpRate) * 2;
		}
		SetTimedMessage("Check", iSafety, kSTM_OneShot);
	}
}

long cScr_TrigFlicker::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeFlicker && pTweqMsg->Op == kTweqOpFrameEvent)
	{
		Blink(m_iFlicker == 0);
	}

	return cBaseTrap::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_TrigFlicker::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "Check"))
	{
		DirectTrigger(false);
		m_iFlicker = 0;
		m_hSafety = NULL;
		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_TrigFlicker::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pAR(g_pScriptManager);
	if (bTurnOn)
	{
		//DirectTrigger(true);
		//m_iFlicker = 1;
		pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
			kTweqTypeFlicker, kTweqDoActivate,
			cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
	}
	else
	{
		pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
			kTweqTypeFlicker, kTweqDoHalt,
			cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
		if (m_iFlicker != 0)
		{
			Blink(false);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrigUnlock
 */
long cScr_TrigUnlock::OnNowLocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOff", ObjId());

	return cBaseScript::OnNowLocked(pMsg, mpReply);
}

long cScr_TrigUnlock::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseScript::OnNowUnlocked(pMsg, mpReply);
}

/***
 * TrigPPlate
 */
long cScr_TrigPPlate::OnPressurePlateActive(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBasePPlateScript::OnPressurePlateActive(pMsg, mpReply);
}

long cScr_TrigPPlate::OnPressurePlateInactive(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOff", ObjId());

	return cBasePPlateScript::OnPressurePlateInactive(pMsg, mpReply);
}

/***
 * TrigPPlateImmed
 */
long cScr_TrigPPlateImmed::OnPressurePlateActivating(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBasePPlateScript::OnPressurePlateActivating(pMsg, mpReply);
}

long cScr_TrigPPlateImmed::OnPressurePlateDeactivating(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend("TurnOff", ObjId());

	return cBasePPlateScript::OnPressurePlateDeactivating(pMsg, mpReply);
}

/***
 * TrapRequireAll
 */
long cScr_TrapRequireAll::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	ulong iOnce = GetFlags() & kTrapFlagOnce;
	UnsetFlag(kTrapFlagOnce);
	uint iRequirement = IterateLinks("~SwitchLink", ObjId(), 0, NULL, NULL, NULL);

	bool bRealOn = bTurnOn != GetFlag(kTrapFlagInvert);
	if (bRealOn)
	{
		if (TurnOn(pMsg->from) && iRequirement == Requirements())
		{
			DirectTrigger(bTurnOn, pMsg->data);
			SetFlag(iOnce);
		}
	}
	else
	{
		uint iPriorCount = Requirements();
		if (TurnOff(pMsg->from) && iRequirement == iPriorCount)
		{
			DirectTrigger(bTurnOn, pMsg->data);
			SetFlag(iOnce);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapRequireAny
 */
long cScr_TrapRequireAny::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	ulong iOnce = GetFlags() & kTrapFlagOnce;
	UnsetFlag(kTrapFlagOnce);
	bool bRealOn = bTurnOn != GetFlag(kTrapFlagInvert);
	if (bRealOn)
	{
		if (TurnOn(pMsg->from) && 1 == Requirements())
		{
			DirectTrigger(bTurnOn, pMsg->data);
			SetFlag(iOnce);
		}
	}
	else
	{
		if (TurnOff(pMsg->from) && 0 == Requirements())
		{
			DirectTrigger(bTurnOn, pMsg->data);
			SetFlag(iOnce);
		}
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrigContained
 */
long cScr_TrigContained::OnContained(sContainedScrMsg* pContainedScrMsg, cMultiParm& mpReply)
{
	DoTrigger(true, pContainedScrMsg->container);

	return cBaseTrap::OnContained(pContainedScrMsg, mpReply);
}

/***
 * TrigRoomPlayerTrans
 */
int cScr_TrigRoomPlayerTrans::AreRoomsRelated(object iDest, object iSource)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed("ScriptParams");

	true_bool bRel;
	pLS->AnyExist(bRel, lk, iDest, iSource);
	if (bRel)
	{
		linkset ls;
		pLS->GetAll(ls, lk, iDest, iSource);
		for (; ls.AnyLinksLeft(); ls.NextLink())
		{
			if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Route"))
				return 1;
		}
	}

	pLS->AnyExist(bRel, lk, iSource, iDest);
	if (bRel)
	{
		linkset ls;
		pLS->GetAll(ls, lk, iSource, iDest);
		for (; ls.AnyLinksLeft(); ls.NextLink())
		{
			if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Route"))
				return -1;
		}
	}

	return 0;
}

long cScr_TrigRoomPlayerTrans::OnPlayerRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	int iRel = AreRoomsRelated(pRoomMsg->FromObjId, pRoomMsg->ToObjId);
	if (iRel > 0)
	{
		CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);
	}
	else if (iRel < 0)
	{
		CDSend("TurnOff", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnPlayerRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomPlayerTrans::OnPlayerRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	int iRel = AreRoomsRelated(pRoomMsg->FromObjId, pRoomMsg->ToObjId);
	if (iRel > 0)
	{
		CDSend("TurnOff", ObjId(), pRoomMsg->MoveObjId);
	}
	else if (iRel < 0)
	{
		CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnPlayerRoomExit(pRoomMsg, mpReply);
}

/***
 * TrigRoomCreature
 */
long cScr_TrigRoomCreature::OnCreatureRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (TrackCreatureEnter(pRoomMsg->MoveObjId) && 1 == Population())
	{
		CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnCreatureRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomCreature::OnCreatureRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (TrackCreatureExit(pRoomMsg->MoveObjId) && 0 == Population())
	{
		CDSend("TurnOff", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnCreatureRoomExit(pRoomMsg, mpReply);
}

long cScr_TrigRoomCreature::OnPlayerRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (TrackPlayerEnter(pRoomMsg->MoveObjId) && 1 == Population())
	{
		CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnPlayerRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomCreature::OnPlayerRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	if (TrackPlayerExit(pRoomMsg->MoveObjId) && 0 == Population())
	{
		CDSend("TurnOff", ObjId(), pRoomMsg->MoveObjId);
	}

	return cBaseRoomScript::OnPlayerRoomExit(pRoomMsg, mpReply);
}

long cScr_TrigRoomCreature::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (pMsg->from != 0 && !_stricmp(pMsg->message, "Obituary"))
	{
		if (TrackCreatureExit(pMsg->from) && 0 == Population())
		{
			CDSend("TurnOff", ObjId(), pMsg->from);
		}
	}

	return cBaseRoomScript::OnMessage(pMsg, mpReply);
}

/***
 * TrigRoomPopChange
 */
long cScr_TrigRoomPopChange::OnCreatureRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);

	return cBaseRoomScript::OnCreatureRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnCreatureRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);

	return cBaseRoomScript::OnCreatureRoomExit(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnPlayerRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);

	return cBaseRoomScript::OnPlayerRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnPlayerRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId(), pRoomMsg->MoveObjId);

	return cBaseRoomScript::OnPlayerRoomExit(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (pMsg->from != 0 && !_stricmp(pMsg->message, "Obituary"))
	{
		CDSend("TurnOn", ObjId(), pMsg->from);
	}

	return cBaseRoomScript::OnMessage(pMsg, mpReply);
}

/***
 * TrigRoomObject
 */
bool cScr_TrigRoomObject::ObjIsInRoom(void)
{
	return static_cast<int>(GetScriptData("RoomObject"));
}

void cScr_TrigRoomObject::ObjEnteringRoom(void)
{
	CDSend("TurnOn", ObjId());
}

void cScr_TrigRoomObject::ObjExitingRoom(void)
{
	CDSend("TurnOff", ObjId());
}

long cScr_TrigRoomObject::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SetScriptData("RoomObject", 0);

	return cBaseRoomScript::OnBeginScript(pMsg, mpReply);
}

long cScr_TrigRoomObject::OnObjRoomTransit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	SService<IObjectSrv> pOS(g_pScriptManager);

	bool bOldRoomRoute = false;
	bool bNewRoomRoute = false;

	linkset lsLinks;
	pLS->GetAll(lsLinks, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
	for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
	{
		if (!lsLinks.Data() || 0 != _stricmp(reinterpret_cast<const char*>(lsLinks.Data()), "Route"))
			continue;
		sLink sl = lsLinks.Get();
		true_bool bInherits;
		if (!bOldRoomRoute)
		{
			pOS->InheritsFrom(bInherits, pRoomMsg->FromObjId, sl.dest);
			bOldRoomRoute = bInherits;
		}
		if (!bNewRoomRoute)
		{
			pOS->InheritsFrom(bInherits, pRoomMsg->ToObjId, sl.dest);
			bNewRoomRoute = bInherits;
		}
	}

	if (bNewRoomRoute)
	{
		if (!bOldRoomRoute)
		{
			ObjEnteringRoom();
			SetScriptData("RoomObject", 1);
		}
	}
	else
	{
		if (bOldRoomRoute)
		{
			ObjExitingRoom();
			SetScriptData("RoomObject", 0);
		}
	}

	return cBaseRoomScript::OnObjRoomTransit(pRoomMsg, mpReply);
}

/***
 * TrigRoomDelivery
 */
void cScr_TrigRoomDelivery::ObjEnteringRoom(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bIsHeld;
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), 0, ObjId());
	if (bIsHeld)
	{
		link lCont;
		sLink sl;
		pLS->GetOne(lCont, pLTS->LinkKindNamed("Contains"), 0, ObjId());
		pLTS->LinkGet(lCont, sl);
		SService<INetworkingSrv> pNet(g_pScriptManager);
		if (pNet->IsPlayer(sl.dest))
			cScr_TrigRoomObject::ObjEnteringRoom();
	}
}

void cScr_TrigRoomDelivery::ObjExitingRoom(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bIsHeld;
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), 0, ObjId());
	if (bIsHeld)
	{
		link lCont;
		sLink sl;
		pLS->GetOne(lCont, pLTS->LinkKindNamed("Contains"), 0, ObjId());
		pLTS->LinkGet(lCont, sl);
		SService<INetworkingSrv> pNet(g_pScriptManager);
		if (pNet->IsPlayer(sl.dest))
			cScr_TrigRoomObject::ObjExitingRoom();
	}
}

long cScr_TrigRoomDelivery::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	SService<INetworkingSrv> pNet(g_pScriptManager);
	if (pNet->IsPlayer(pContMsg->container))
	{
		if (pContMsg->event == kContainAdd)
		{
			if (ObjIsInRoom())
				cScr_TrigRoomObject::ObjEnteringRoom();
		}
		else if (pContMsg->event == kContainRemove)
		{
			if (ObjIsInRoom())
				cScr_TrigRoomObject::ObjExitingRoom();
		}
	}

	return cScr_TrigRoomObject::OnContained(pContMsg, mpReply);
}

/***
 * TrigRoomDeposit
 */
void cScr_TrigRoomDeposit::ObjEnteringRoom(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bIsHeld;
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), 0, ObjId());
	if (!bIsHeld)
		cScr_TrigRoomObject::ObjEnteringRoom();
}

void cScr_TrigRoomDeposit::ObjExitingRoom(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bIsHeld;
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), 0, ObjId());
	if (!bIsHeld)
		cScr_TrigRoomObject::ObjExitingRoom();
}

long cScr_TrigRoomDeposit::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->event == kContainAdd)
	{
		if (ObjIsInRoom())
			cScr_TrigRoomObject::ObjExitingRoom();
	}
	else if (pContMsg->event == kContainRemove)
	{
		if (ObjIsInRoom())
			cScr_TrigRoomObject::ObjEnteringRoom();
	}

	return cScr_TrigRoomObject::OnContained(pContMsg, mpReply);
}

/***
 * AnimLight
 */
cScr_AnimLight::cScr_AnimLight(const char* pszName, int iHostObjId)
	: cBaseTrap(pszName, iHostObjId),
	  SCRIPT_VAROBJ(cScr_AnimLight,m_iOnMode,iHostObjId),
	  SCRIPT_VAROBJ(cScr_AnimLight,m_iOffMode,iHostObjId)
{
	static const sMessageHandler handlers[] = {{"LightChange",HandleLightChange}, {"Toggle",HandleToggle}};
	RegisterMessageHandlers(handlers, 2);
}

long cScr_AnimLight::HandleLightChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_AnimLight*>(pScript)->OnLightChange(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_AnimLight::HandleToggle(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_AnimLight*>(pScript)->OnToggle(pMsg, static_cast<cMultiParm&>(*pReply));
}

void cScr_AnimLight::InitModes(void)
{
	static const int _OnModes[] = { 0,1,2,4,4,4,6,6,8,9 };
	static const int _OffModes[] = {5,7,5,3,3,5,7,7,7,5 };

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "AnimLight"))
	{
		cMultiParm mpMode;
		pPS->Get(mpMode, ObjId(), "AnimLight", "Mode");
		if (static_cast<int>(mpMode) >= 0 && static_cast<int>(mpMode) < 10)
		{
			m_iOnMode = _OnModes[static_cast<int>(mpMode)];
			m_iOffMode = _OffModes[static_cast<int>(mpMode)];
		}
		else
		{
			m_iOnMode = 4;
			m_iOffMode = 3;
		}
	}
	else
	{
		m_iOnMode = 4;
		m_iOffMode = 3;
	}
}

void cScr_AnimLight::TurnLightOn(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "AnimLight"))
	{
		if (m_iOnMode < 0)
			InitModes();
		SService<ILightScrSrv> pLS(g_pScriptManager);
		pLS->SetMode(ObjId(), m_iOnMode);
	}
	if (pPS->Possessed(ObjId(), "AmbientHacked"))
	{
		cMultiParm mpFlags;
		pPS->Get(mpFlags, ObjId(), "AmbientHacked", "Flags");
		mpFlags = static_cast<int>(mpFlags) & ~4;
		pPS->Set(ObjId(), "AmbientHacked", "Flags", mpFlags);
	}
	if (pPS->Possessed(ObjId(), "SelfIllum"))
	{
		pPS->SetSimple(ObjId(), "SelfIllum", 1.875f);
	}

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), "TurnOn", pLTS->LinkKindNamed("~ParticleAttachement"));
	pLS->BroadcastOnAllLinks(ObjId(), "TurnOn", pLTS->LinkKindNamed("SwitchLink"));
}

void cScr_AnimLight::TurnLightOff(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "AnimLight"))
	{
		if (m_iOffMode < 0)
			InitModes();
		SService<ILightScrSrv> pLS(g_pScriptManager);
		pLS->SetMode(ObjId(), m_iOffMode);
	}
	if (pPS->Possessed(ObjId(), "AmbientHacked"))
	{
		cMultiParm mpFlags;
		pPS->Get(mpFlags, ObjId(), "AmbientHacked", "Flags");
		mpFlags = static_cast<int>(mpFlags) | 4;
		pPS->Set(ObjId(), "AmbientHacked", "Flags", mpFlags);
	}
	if (pPS->Possessed(ObjId(), "SelfIllum"))
	{
		pPS->SetSimple(ObjId(), "SelfIllum", 0.0f);
	}

	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLS->BroadcastOnAllLinks(ObjId(), "TurnOff", pLTS->LinkKindNamed("~ParticleAttachement"));
	pLS->BroadcastOnAllLinks(ObjId(), "TurnOff", pLTS->LinkKindNamed("SwitchLink"));
}

bool cScr_AnimLight::IsLightOn(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(ObjId(), "AnimLight"))
		return false;

	if (m_iOnMode < 0)
		return true;

	cMultiParm mpMode;
	pPS->Get(mpMode, ObjId(), "AnimLight", "Mode");
	return m_iOnMode == static_cast<int>(mpMode);
}

long cScr_AnimLight::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	m_iOnMode.Init(-1);
	m_iOffMode.Init(-1);

	SService<IPropertySrv> pPS(g_pScriptManager);
	cMultiParm mpInitted;
	pPS->Get(mpInitted, ObjId(), "Initted", NULL);
	if (!static_cast<bool>(mpInitted))
	{
		InitModes();
		SetTimedMessage("InitLight", 100, kSTM_OneShot);
		mpInitted = 1;
		pPS->SetSimple(ObjId(), "Initted", mpInitted);
	}

	SService<ILightScrSrv> pLS(g_pScriptManager);
	pLS->Subscribe(ObjId());

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_AnimLight::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<ILightScrSrv> pLS(g_pScriptManager);
	pLS->Unsubscribe(ObjId());

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

/*
long cScr_AnimLight::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting)
	{
		InitModes();
		if (IsLightOn())
			TurnLightOn();
		else
			TurnLightOff();
	}

	return cBaseTrap::OnSim(pSimMsg, mpReply);
}
*/

long cScr_AnimLight::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	TurnLightOff();
	SetTimedMessage("ReallySlay", 100, kSTM_OneShot);

	return cBaseTrap::OnSlain(pSlayMsg, mpReply);
}

long cScr_AnimLight::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "ReallySlay"))
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
		return 0;
	}
	if (!strcmp(pTimerMsg->name, "InitLight"))
	{
		if (IsLightOn())
			TurnLightOn();
		else
			TurnLightOff();
		return 0;
	}

	return cBaseTrap::OnTimer(pTimerMsg, mpReply);
}

long cScr_AnimLight::OnTweqComplete(sTweqMsg* pTweqMsg, cMultiParm& mpReply)
{
	if (pTweqMsg->Type == kTweqTypeFlicker)
	{
		TurnLightOn();
		return 0;
	}

	return cBaseTrap::OnTweqComplete(pTweqMsg, mpReply);
}

long cScr_AnimLight::OnLightChange(sScrMsg* pMsg, cMultiParm&)
{
	if (static_cast<int>(pMsg->data) == static_cast<int>(pMsg->data3))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "SelfIllum"))
		{
			SService<INetworkingSrv> pNet(g_pScriptManager);
			pNet->Suspend();
			pPS->SetLocal(ObjId(), "SelfIllum", NULL, 1.875f);
			pNet->Resume();
		}
	}
	else if (static_cast<int>(pMsg->data) == static_cast<int>(pMsg->data2))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "SelfIllum"))
		{
			SService<INetworkingSrv> pNet(g_pScriptManager);
			pNet->Suspend();
			pPS->SetLocal(ObjId(), "SelfIllum", NULL, 0.0f);
			pNet->Resume();
		}
	}

	return 0;
}

long cScr_AnimLight::OnToggle(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsLightOn())
		OnSwitch(false, pMsg, mpReply);
	else
		OnSwitch(true, pMsg, mpReply);
	return 0;
}

long cScr_AnimLight::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "StTweqBlink"))
		{
			SService<IActReactSrv> pAR(g_pScriptManager);
			pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
				kTweqTypeFlicker, kTweqDoActivate,
				cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
			if (m_iOnMode < 0)
				InitModes();
			SService<ILightScrSrv> pLS(g_pScriptManager);
			pLS->SetMode(ObjId(), kAnimLight_FlickerMinMax);
		}
		else
			TurnLightOn();
	}
	else
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
			if (pPS->Possessed(ObjId(), "StTweqBlink"))
		{
			SService<IActReactSrv> pAR(g_pScriptManager);
			pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0f, ObjId(), 0,
				kTweqTypeFlicker, kTweqDoHalt,
				cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);
			pPS->Set(ObjId(), "StTweqBlink", "Cur Time", 0);
		}
		TurnLightOff();
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapAddPsi
 */
long cScr_TrapAddPsi::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	int iAdd = GetObjectParamInt(ObjId(), "psi");
	if (iAdd != 0)
	{
		if (!bTurnOn)
			iAdd = -iAdd;

		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->SetPlayerPsiPoints(pShock->GetPlayerPsiPoints() + iAdd);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapKeypad
 */
long cScr_TrapKeypad::OnKeypadDone(sKeypadMsg* pKeypadMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "KeypadCode"))
	{
		cMultiParm mpCode;
		pPS->Get(mpCode, ObjId(), "KeypadCode", NULL);
		if (pKeypadMsg->code == static_cast<int>(mpCode))
		{
			int iFrobber = GetScriptData("Frobber");
			DirectTrigger(true, iFrobber);
		}
		else
		{
			SService<ILinkSrv> pLS(g_pScriptManager);
			SService<ILinkToolsSrv> pLTS(g_pScriptManager);
			pLS->BroadcastOnAllLinksData(ObjId(), "TurnOn", pLTS->LinkKindNamed("ScriptParams"), "ErrorOutput");
		}
	}

	return cBaseTrap::OnKeypadDone(pKeypadMsg, mpReply);
}

long cScr_TrapKeypad::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		SetScriptData("Frobber", static_cast<int>(pMsg->data));
		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->Keypad(ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapNanites
 */
long cScr_TrapNanites::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	m_bNoCharge = true;
	long iRet = cBaseTrap::OnTimer(pTimerMsg, mpReply);
	m_bNoCharge = false;
	return iRet;
}

long cScr_TrapNanites::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!m_bNoCharge)
	{
		int iCost = GetObjectParamInt(ObjId(), "nanites");
		if (iCost != 0 && (iCost < 0 || bTurnOn))
		{
			SService<IShockGameSrv> pShock(g_pScriptManager);
			if (pShock->PayNanites(-iCost))
			{
				pShock->AddTranslatableText("NeedNanites","misc",StrToObject("Player"),5000);
				SService<ILinkSrv> pLS(g_pScriptManager);
				SService<ILinkToolsSrv> pLTS(g_pScriptManager);
				pLS->BroadcastOnAllLinksData(ObjId(), "TurnOn", pLTS->LinkKindNamed("ScriptParams"), "ErrorOutput");
				return 0;
			}
		}
	}

	DirectTrigger(bTurnOn);

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TraitMachineReusable
 */
int cScr_NewTraitMachine::TraitsUsed(object iPlayer)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (! pPS->Possessed(iPlayer, "TraitsDesc"))
		return 0;

	cMultiParm mpTrait;
	pPS->Get(mpTrait, iPlayer, "TraitsDesc", "Trait 1");
	if (static_cast<int>(mpTrait) == 0)
		return 0;
	pPS->Get(mpTrait, iPlayer, "TraitsDesc", "Trait 2");
	if (static_cast<int>(mpTrait) == 0)
		return 1;
	pPS->Get(mpTrait, iPlayer, "TraitsDesc", "Trait 3");
	if (static_cast<int>(mpTrait) == 0)
		return 2;
	pPS->Get(mpTrait, iPlayer, "TraitsDesc", "Trait 4");
	if (static_cast<int>(mpTrait) == 0)
		return 3;
	return 4;
}

void cScr_NewTraitMachine::SetLock(bool bLock)
{
	SService<IPropertySrv> pPropSrv(g_pScriptManager);

	object iLock = GetOneLinkDest("Lock", ObjId());
	if (iLock)
	{
		if (pPropSrv->Possessed(ObjId(), "Locked"))
			pPropSrv->Remove(ObjId(), "Locked");
	}
	else
		iLock = ObjId();

	if (! pPropSrv->Possessed(iLock, "Locked"))
		pPropSrv->Add(iLock, "Locked");
	pPropSrv->SetSimple(iLock, "Locked", static_cast<int>(bLock));
}

long cScr_NewTraitMachine::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<ILockSrv> pLock(g_pScriptManager);
	if (!pLock->IsLocked(ObjId()))
	{
		if (4 > TraitsUsed(pFrobMsg->Frobber))
		{
			SService<ISoundScrSrv> pSounds(g_pScriptManager);
			true_bool __p;
			pSounds->PlayEnvSchema(__p, ObjId(), "Event Activate", ObjId(), 0, kEnvSoundOnObj, kSoundNetNormal);

			SService<IShockGameSrv> pShock(g_pScriptManager);
			pShock->OverlayChangeObj(20, 1, ObjId());
		}
		else
		{
			SService<IShockGameSrv> pShock(g_pScriptManager);
			pShock->AddTranslatableText("ErrorMaxed", "misc", pFrobMsg->Frobber, 5000);
		}
	}
	else
	{
		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->AddTranslatableText("TraitMachineUsed", "misc", pFrobMsg->Frobber, 5000);
	}

	return cBaseScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}

long cScr_NewTraitMachine::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SetLock(false);

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_NewTraitMachine::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SetLock(true);

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

long cScr_NewTraitMachine::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Used"))
	{
		SetLock(true);

		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->RefreshInv();

		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * PhysARContact
 */
long cScr_PhysARContact::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysContact);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_PhysARContact::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysContact);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_PhysARContact::OnPhysContactCreate(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->EndContact(ObjId(), pPhysMsg->contactObj);
	pARSrv->BeginContact(ObjId(), pPhysMsg->contactObj);

	return cBaseScript::OnPhysContactCreate(pPhysMsg, mpReply);
}

long cScr_PhysARContact::OnPhysContactDestroy(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pARSrv(g_pScriptManager);
	pARSrv->EndContact(ObjId(), pPhysMsg->contactObj);

	return cBaseScript::OnPhysContactDestroy(pPhysMsg, mpReply);
}


/***
 * CloneContactFrob
 */
long cScr_CloneContactFrob::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<IObjectSrv> pOS(g_pScriptManager);
	object oClone;
	pOS->BeginCreate(oClone, ObjId());
	if (oClone)
	{
		pPS->SetSimple(oClone, "HasRefs", 0);
		pOS->EndCreate(oClone);

		SService<IActReactSrv> pARSrv(g_pScriptManager);
		pARSrv->BeginContact(oClone, pFrobMsg->Frobber);
	}

	SInterface<IContainSys> pCS(g_pScriptManager);
	pCS->StackAdd(ObjId(), -1, 1);
	/*
	SService<IContainSrv> pCS(g_pScriptManager);
	pCS->StackAdd(ObjId(), -1);
	cMultiParm mpStack;
	pPS->Get(mpStack, ObjId(), "StackCount", NULL);
	if (0 == static_cast<int>(mpStack))
	{
		SService<IShockGameSrv> pShock(g_pScriptManager);
		pShock->DestroyInvObj(ObjId());
	}
	*/
	SService<IShockGameSrv> pShock(g_pScriptManager);
	pShock->RefreshInv();

	return cBaseScript::OnFrobInvEnd(pFrobMsg, mpReply);
}


#endif // _DARKGAME == 3
