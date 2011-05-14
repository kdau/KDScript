/******************************************************************************
 *  T1Scripts.cpp
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
#if (_DARKGAME == 1)
#include "T1Scripts.h"
#include "ScriptModule.h"

#include <lg/types.h>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include <ScriptLib.h>
#include "utils.h"

#include <cstring>

using namespace std;


/***
 * TrapOffFilter
 */
long cScr_TrapOffFilter::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend(pMsg->message, ObjId(), pMsg->data);
	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

/***
 * TrapOnce
 */
long cScr_TrapOnce::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (! IsScriptDataSet("Once"))
	{
		CDSend(pMsg->message, ObjId(), pMsg->data);
		SetScriptData("Once", 1);
	}
	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_TrapOnce::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (! IsScriptDataSet("Once"))
	{
		CDSend(pMsg->message, ObjId(), pMsg->data);
		SetScriptData("Once", 1);
	}
	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

/***
 * TrapRevert
 */
long cScr_TrapRevert::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "Revert"))
	{
		CDSend(static_cast<int>(pTimerMsg->data)?"TurnOn":"TurnOff", ObjId());
		ClearScriptData("Timer");

		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}
long cScr_TrapRevert::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend(pMsg->message, ObjId());

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ScriptTiming"))
	{
		cMultiParm mpTiming;
		pPS->Get(mpTiming, ObjId(), "ScriptTiming", NULL);
		if (static_cast<int>(mpTiming) > 0)
		{
			if (IsScriptDataSet("Timer"))
			{
				cMultiParm mpHandle = GetScriptData("Timer");
				KillTimedMessage(reinterpret_cast<tScrTimer>(static_cast<int>(mpHandle)));
			}
			tScrTimer hTimer =
				SetTimedMessage("Revert", static_cast<int>(mpTiming), kSTM_OneShot, 0);
			SetScriptData("Timer", reinterpret_cast<int>(hTimer));
		}
	}

	return cBaseScript::OnTurnOn(pMsg, mpReply);
}

long cScr_TrapRevert::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend(pMsg->message, ObjId());

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ScriptTiming"))
	{
		cMultiParm mpTiming;
		pPS->Get(mpTiming, ObjId(), "ScriptTiming", NULL);
		if (static_cast<int>(mpTiming) > 0)
		{
			if (IsScriptDataSet("Timer"))
			{
				cMultiParm mpHandle = GetScriptData("Timer");
				KillTimedMessage(reinterpret_cast<tScrTimer>(static_cast<int>(mpHandle)));
			}
			tScrTimer hTimer =
				SetTimedMessage("Revert", static_cast<int>(mpTiming), kSTM_OneShot, 1);
			SetScriptData("Timer", reinterpret_cast<int>(hTimer));
		}
	}

	return cBaseScript::OnTurnOff(pMsg, mpReply);
}

/***
 * TrapDirection
 */
long cScr_TrapDirection::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	CDSend(bTurnOn ? "GoForward" : "GoReverse", ObjId());

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapPatroller
 */
int cScr_TrapPatroller::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
	sLink sl;
	pLQ->Link(&sl);
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(sl.dest, "AI_Patrol"))
		pPS->Add(sl.dest, "AI_Patrol");
	pPS->SetSimple(sl.dest, "AI_Patrol", reinterpret_cast<int>(pData));

	return 1;
}

long cScr_TrapPatroller::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(int(bTurnOn)));

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrapNonFinalComplete
 */
bool cScr_TrapNonFinalComplete::EvaluateGoals(int num_goals)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	char szGoalState[24];
	char szGoalVisible[24];
	char szGoalReverse[24];
	char szGoalOptional[28];
	char szGoalFinal[24];
	int n = 0;
	strcpy(szGoalState, "goal_state_");
	strcpy(szGoalVisible, "goal_visible_");
	strcpy(szGoalReverse, "goal_reverse_");
	strcpy(szGoalOptional, "goal_optional_");
	strcpy(szGoalFinal, "goal_final_");
	for (; n < num_goals; ++n)
	{
		sprintf(szGoalState+11, "%d", n);
		sprintf(szGoalVisible+13, "%d", n);
		sprintf(szGoalReverse+13, "%d", n);
		sprintf(szGoalOptional+14, "%d", n);
		sprintf(szGoalFinal+11, "%d", n);
		if (pQS->Get(szGoalVisible))
		{
			int state = pQS->Get(szGoalState);
			if (state == 3)
				return false;
			if (state != 2
			&& !(pQS->Get(szGoalFinal) || pQS->Get(szGoalOptional)))
			{
				if (pQS->Get(szGoalReverse) ? state!=0 : state==0)
					return false;
			}
		}
	}
	return true;
}

long cScr_TrapNonFinalComplete::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (! IsScriptDataSet("num_goals"))
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		char szGoalState[24];
		char szGoalVisible[26];
		int n = 0;
		while (1)
		{
			sprintf(szGoalState, "goal_state_%d", n);
			if (pQS->Exists(szGoalState))
			{
				sprintf(szGoalVisible, "goal_visible_%d", n);
				pQS->SubscribeMsg(ObjId(), szGoalState, kQuestDataAny);
				pQS->SubscribeMsg(ObjId(), szGoalVisible, kQuestDataAny);
			}
			else
				break;
			++n;
		}
		SetScriptData("num_goals", n);
		SetScriptData("state", 0);
	}

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_TrapNonFinalComplete::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsScriptDataSet("num_goals"))
	{
		SService<IQuestSrv> pQS(g_pScriptManager);
		char szGoalState[24];
		char szGoalVisible[26];
		int n = GetScriptData("num_goals");
		while (n--)
		{
			sprintf(szGoalState, "goal_state_%d", n);
			sprintf(szGoalVisible, "goal_visible_%d", n);
			pQS->UnsubscribeMsg(ObjId(), szGoalState);
			pQS->UnsubscribeMsg(ObjId(), szGoalVisible);
		}
	}

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_TrapNonFinalComplete::OnQuestChange(sQuestMsg* pQuestMsg, cMultiParm& mpReply)
{
	if (IsScriptDataSet("num_goals"))
	{
		if (!strnicmp(pQuestMsg->m_pName, "goal_state_", 11)
		 || !strnicmp(pQuestMsg->m_pName, "goal_visible_", 13))
		{
			int iNumGoals = GetScriptData("num_goals");
			int iState = GetScriptData("state");
			int newstate = EvaluateGoals(iNumGoals);
			if (newstate != static_cast<int>(iState))
			{
				DoTrigger(newstate);
				SetScriptData("state", newstate);
			}
		}
	}

	return cBaseTrap::OnQuestChange(pQuestMsg, mpReply);
}

long cScr_TrapNonFinalComplete::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
	if (pSimMsg->fStarting && IsScriptDataSet("num_goals"))
	{
		int iNumGoals = GetScriptData("num_goals");
		int state = EvaluateGoals(iNumGoals);
		DoTrigger(state);
		SetScriptData("state", state);
	}

	return cBaseTrap::OnSim(pSimMsg, mpReply);
}

/***
 * TrapTexture
 */
long cScr_TrapTexture::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "TerrRepOn") && pPS->Possessed(ObjId(), "TerrRepOff"))
	{
		cMultiParm mpOld, mpNew;
		pPS->Get(mpOld, ObjId(), "TerrRepOff", NULL);
		pPS->Get(mpNew, ObjId(), "TerrRepOn", NULL);

		SService<IAnimTextureSrv> pTS(g_pScriptManager);
		if (bTurnOn)
			pTS->ChangeTexture(ObjId(), NULL, mpOld, NULL, mpNew);
		else
			pTS->ChangeTexture(ObjId(), NULL, mpNew, NULL, mpOld);
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * TrigOBBPlayer
 */
long cScr_TrigOBBPlayer::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_TrigOBBPlayer::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysEnterExit);

	return cBaseTrap::OnEndScript(pMsg, mpReply);
}

long cScr_TrigOBBPlayer::OnPhysEnter(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	if (pPhysMsg->transObj == StrToObject("Player"))
	{
		DoTrigger(true);
		SService<ISoundScrSrv> pSound(g_pScriptManager);
		true_bool _p;
		pSound->PlayEnvSchema(_p, ObjId(), "Event Activate", ObjId(), 0, kEnvSoundOnObj);
	}

	return cBaseTrap::OnPhysEnter(pPhysMsg, mpReply);
}

long cScr_TrigOBBPlayer::OnPhysExit(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	if (pPhysMsg->transObj == StrToObject("Player"))
	{
		DoTrigger(false);
		SService<ISoundScrSrv> pSound(g_pScriptManager);
		true_bool _p;
		pSound->PlayEnvSchema(_p, ObjId(), "Event Deactivate", ObjId(), 0, kEnvSoundOnObj);
	}

	return cBaseTrap::OnPhysExit(pPhysMsg, mpReply);
}


/***
 * TrigRoomPopChange
 */
long cScr_TrigRoomPopChange::OnCreatureRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseRoomScript::OnCreatureRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnCreatureRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseRoomScript::OnCreatureRoomExit(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnPlayerRoomEnter(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseRoomScript::OnPlayerRoomEnter(pRoomMsg, mpReply);
}

long cScr_TrigRoomPopChange::OnPlayerRoomExit(sRoomMsg* pRoomMsg, cMultiParm& mpReply)
{
	CDSend("TurnOn", ObjId());

	return cBaseRoomScript::OnPlayerRoomExit(pRoomMsg, mpReply);
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
	pLS->GetAll(lsLinks, pLTS->LinkKindNamed("Route"), ObjId(), 0);
	for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
	{
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
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), StrToObject("Player"), ObjId());
	if (bIsHeld)
		cScr_TrigRoomObject::ObjEnteringRoom();
}

void cScr_TrigRoomDelivery::ObjExitingRoom(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	true_bool bIsHeld;
	pLS->AnyExist(bIsHeld, pLTS->LinkKindNamed("Contains"), StrToObject("Player"), ObjId());
	if (bIsHeld)
		cScr_TrigRoomObject::ObjExitingRoom();
}

long cScr_TrigRoomDelivery::OnContained(sContainedScrMsg* pContMsg, cMultiParm& mpReply)
{
	if (pContMsg->container == StrToObject("Player"))
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
 * BaseFindSecret
 */
void cBaseFindSecret::InitHidden(void)
{
	sScrDatumTag tag;
	tag.pszClass = "cBaseFindSecret";
	tag.pszName = "InitHid";
	tag.objId = m_iObjId;
	if (! g_pScriptManager->IsScriptDataSet(&tag))
	{
		cMultiParm mp;
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(m_iObjId, "DarkStat"))
		{
			pPS->Get(mp, m_iObjId, "DarkStat", NULL);
			if (static_cast<int>(mp) & 4)
			{
				SService<IQuestSrv> pQS(g_pScriptManager);
				pQS->Set("DrSScrtCnt", pQS->Get("DrSScrtCnt")+1, kQuestDataMission);
			}
		}
		mp = 1;
		g_pScriptManager->SetScriptData(&tag, &mp);
	}
}

void cBaseFindSecret::FindHidden(void)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(m_iObjId, "DarkStat"))
	{
		cMultiParm mpStats;
		pPS->Get(mpStats, m_iObjId, "DarkStat", NULL);
		if (static_cast<int>(mpStats) & 4)
		{
			SService<IQuestSrv> pQS(g_pScriptManager);
			pQS->Set("DrSSecrets", pQS->Get("DrSSecrets")+1, kQuestDataMission);
			pPS->SetSimple(m_iObjId, "DarkStat", static_cast<int>(mpStats) & ~4);
		}
	}
}

/***
 * TrapFindSecret
 */
long cScr_TrapFindSecret::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitHidden();
	return cBaseTrap::OnBeginScript(pMsg, mpReply);
}

long cScr_TrapFindSecret::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
		FindHidden();
	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}

/***
 * FrobFind
 */
long cScr_FrobFind::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitHidden();
	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_FrobFind::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	FindHidden();
	return cBaseScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}

/***
 * SlayFind
 */
long cScr_SlayFind::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitHidden();
	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_SlayFind::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	FindHidden();
	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}

/***
 * HiddenDoor
 */
long cScr_HiddenDoor::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitHidden();
	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_HiddenDoor::OnDoorOpen(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	FindHidden();
	return cBaseDoorScript::OnDoorOpen(pDoorMsg, mpReply);
}


/***
 * NonAutoDoor
 */
eDoorState cScr_NonAutoDoor::TargetState(eDoorState state)
{
	static const eDoorState _targ[5] = { kDoorStateClosed, kDoorStateOpen, kDoorStateOpen, kDoorStateHalted};
	return _targ[state];
}

cAnsiStr cScr_NonAutoDoor::StateChangeTags(sDoorMsg::eDoorAction state, sDoorMsg::eDoorAction oldstate)
{
	static const char* _events[5] = { "Open", "Closed", "Opening", "Closing", "Halted" };
	cAnsiStr tags;
	tags.FmtStr("Event StateChange, OpenState %s, OldOpenState %s", _events[state], _events[oldstate]);
	if (IsScriptDataSet("PlayerFrob"))
	{
		tags += ", CreatureType Player";
		if (!(state == sDoorMsg::kDoorOpening || state == sDoorMsg::kDoorClosing))
		{
			ClearScriptData("PlayerFrob");
		}
	}
	return tags;
}

void cScr_NonAutoDoor::PingDoubles(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind flavor = pLTS->LinkKindNamed("ScriptParams");
	pLS->BroadcastOnAllLinksData(ObjId(), "SynchUp", flavor, "Double");
	pLS->BroadcastOnAllLinksData(ObjId(), "SynchUp", -flavor, "Double");
}

long cScr_NonAutoDoor::OnNowLocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim())
		PingDoubles();

	return cBaseDoorScript::OnNowLocked(pMsg, mpReply);
}

long cScr_NonAutoDoor::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (IsSim())
		PingDoubles();

	return cBaseDoorScript::OnNowUnlocked(pMsg, mpReply);
}

long cScr_NonAutoDoor::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<IDoorSrv> pDS(g_pScriptManager);
	SService<ILockSrv> pLock(g_pScriptManager);

	if (pLock->IsLocked(ObjId()) && kDoorStateClosed == pDS->GetDoorState(ObjId()))
	{
		SService<ISoundScrSrv> pSound(g_pScriptManager);
		true_bool _p;
		pSound->PlayEnvSchema(_p, ObjId(), "Event Reject, Operation OpenDoor", ObjId(), 0, kEnvSoundOnObj);
	}
	else
	{
		if (pFrobMsg->Frobber == StrToObject("Player"))
			SetScriptData("PlayerFrob", 1);
		pDS->ToggleDoor(ObjId());
	}

	return cBaseDoorScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}

long cScr_NonAutoDoor::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<IDamageSrv> pDmg(g_pScriptManager);

	pPS->SetSimple(ObjId(), "HitPoints", 1);

	DeleteAllLinks("Lock", ObjId(), 0);
	if (pPS->Possessed(ObjId(), "Locked"))
	{
		pPS->SetSimple(ObjId(), "Locked", 0);
		pPS->Remove(ObjId(), "Locked");
	}
	if (pPS->Possessed(ObjId(), "KeyDst"))
		pPS->Remove(ObjId(), "KeyDst");

	// I wonder if I should really do this, since it's a "non-auto" door
	// Well, Thief 2 does it, so...
	SService<IDoorSrv> pDS(g_pScriptManager);
	pDS->OpenDoor(ObjId());

	return cBaseDoorScript::OnSlain(pSlayMsg, mpReply);
}

long cScr_NonAutoDoor::OnDoorOpen(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->HaltSchema(_p, ObjId(), "", 0);
	pSound->PlayEnvSchema(_p, ObjId(),
		static_cast<const char*>(StateChangeTags(pDoorMsg->ActionType, pDoorMsg->PrevActionType)),
		ObjId(), 0, kEnvSoundOnObj);

	return cBaseDoorScript::OnDoorOpen(pDoorMsg, mpReply);
}

long cScr_NonAutoDoor::OnDoorClose(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->HaltSchema(_p, ObjId(), "", 0);
	pSound->PlayEnvSchema(_p, ObjId(),
		static_cast<const char*>(StateChangeTags(pDoorMsg->ActionType, pDoorMsg->PrevActionType)),
		ObjId(), 0, kEnvSoundOnObj);

	return cBaseDoorScript::OnDoorClose(pDoorMsg, mpReply);
}

long cScr_NonAutoDoor::OnDoorOpening(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	PingDoubles();
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->HaltSchema(_p, ObjId(), "", 0);
	pSound->PlayEnvSchema(_p, ObjId(),
		static_cast<const char*>(StateChangeTags(pDoorMsg->ActionType, pDoorMsg->PrevActionType)),
		ObjId(), 0, kEnvSoundOnObj);

	return cBaseDoorScript::OnDoorOpening(pDoorMsg, mpReply);
}

long cScr_NonAutoDoor::OnDoorClosing(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	PingDoubles();
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->HaltSchema(_p, ObjId(), "", 0);
	pSound->PlayEnvSchema(_p, ObjId(),
		static_cast<const char*>(StateChangeTags(pDoorMsg->ActionType, pDoorMsg->PrevActionType)),
		ObjId(), 0, kEnvSoundOnObj);

	return cBaseDoorScript::OnDoorClosing(pDoorMsg, mpReply);
}

long cScr_NonAutoDoor::OnDoorHalt(sDoorMsg* pDoorMsg, cMultiParm& mpReply)
{
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->HaltSchema(_p, ObjId(), "", 0);
	pSound->PlayEnvSchema(_p, ObjId(),
		static_cast<const char*>(StateChangeTags(pDoorMsg->ActionType, pDoorMsg->PrevActionType)),
		ObjId(), 0, kEnvSoundOnObj);

	return cBaseDoorScript::OnDoorHalt(pDoorMsg, mpReply);
}

long cScr_NonAutoDoor::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "SynchUp"))
	{
		SService<IDoorSrv> pDS(g_pScriptManager);
		eDoorState target = TargetState(eDoorState(pDS->GetDoorState(pMsg->from)));
		eDoorState current = TargetState(eDoorState(pDS->GetDoorState(ObjId())));
		if (current != target)
		{
			if (target == kDoorStateClosed)
				pDS->CloseDoor(ObjId());
			else
				pDS->OpenDoor(ObjId());
		}

		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(pMsg->from, "Locked") && pPS->Possessed(ObjId(), "Locked"))
		{
			cMultiParm mpLock, mpTargLock;
			pPS->Get(mpLock, ObjId(), "Locked", NULL);
			pPS->Get(mpTargLock, pMsg->from, "Locked", NULL);
			if (static_cast<int>(mpLock) != static_cast<int>(mpTargLock))
			{
				pPS->SetSimple(ObjId(), "Locked", mpTargLock);
			}
		}

		return 0;
	}
	else if (!_stricmp(pMsg->message, "PlayerToolFrob"))
	{
		SetScriptData("PlayerFrob", 1);
		return 0;
	}

	return cBaseDoorScript::OnMessage(pMsg, mpReply);
}

long cScr_NonAutoDoor::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IDoorSrv> pDS(g_pScriptManager);
	pDS->OpenDoor(ObjId());

	return cBaseDoorScript::OnTurnOn(pMsg, mpReply);
}

long cScr_NonAutoDoor::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IDoorSrv> pDS(g_pScriptManager);
	pDS->CloseDoor(ObjId());

	return cBaseDoorScript::OnTurnOff(pMsg, mpReply);
}


/***
 * TransformLock
 */
long cScr_TransformLock::OnNowUnlocked(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IActReactSrv> pAR(g_pScriptManager);
	pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0, ObjId(), 0, kTweqTypeModels, kTweqDoActivate,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "KeyDst"))
		pPS->Set(ObjId(), "KeyDst", "RegionMask", 0);

	return cBaseScript::OnNowUnlocked(pMsg, mpReply);
}


/***
 * CleanUpAttack
 */
long cScr_CleanUpAttack::OnAIModeChange(sAIModeChangeMsg* pAIModeMsg, cMultiParm& mpReply)
{
	if (pAIModeMsg->mode == 5)
	{
		cMultiParm mpIgnore;
		SendMessage(mpIgnore, ObjId(), "AbortAttack");
	}

	return cBaseAIScript::OnAIModeChange(pAIModeMsg, mpReply);
}


/***
 * NoPingBack
 */
long cScr_NoPingBack::OnAIModeChange(sAIModeChangeMsg* pAIModeMsg, cMultiParm& mpReply)
{
	if (pAIModeMsg->mode == 5)
		AddSingleMetaProperty("NoPingBack", ObjId());

	return cBaseAIScript::OnAIModeChange(pAIModeMsg, mpReply);
}


/***
 * NoticesPlayerBumps
 */
long cScr_NoticesPlayerBumps::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysCollision);

	return cBaseAIScript::OnBeginScript(pMsg, mpReply);
}

long cScr_NoticesPlayerBumps::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysCollision);

	return cBaseAIScript::OnEndScript(pMsg, mpReply);
}

long cScr_NoticesPlayerBumps::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "Relax"))
	{
		SService<IAIScrSrv> pAI(g_pScriptManager);
		pAI->SetMinimumAlert(ObjId(), kNoAlert);

		return 0;
	}

	return cBaseAIScript::OnTimer(pTimerMsg, mpReply);
}

long cScr_NoticesPlayerBumps::OnPhysCollision(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	if (pPhysMsg->collObj == StrToObject("Player"))
	{
		SService<IAIScrSrv> pAI(g_pScriptManager);
		pAI->SetMinimumAlert(ObjId(), kModerateAlert);
		SetTimedMessage("Relax", 1000, kSTM_OneShot);
	}

	return cBaseAIScript::OnPhysCollision(pPhysMsg, mpReply);
}


/***
 * ShutUpYerDead
 */
int cScr_ShutUpYerDead::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void*)
{
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);
	pLTS->LinkSetData(pLQ->ID(), "Flags", 0);

	return 1;
}

long cScr_ShutUpYerDead::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	IterateLinks("AIAwareness", ObjId(), 0, LinkIter, this, NULL);
	cMultiParm mpIgnore;
	SendMessage(mpIgnore, ObjId(), "IgnorePotion");

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}


/***
 * SlayHaltSpeech
 */
long cScr_SlayHaltSpeech::OnSlain(sSlayMsg* pSlayMsg, cMultiParm& mpReply)
{
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	pSound->HaltSpeech(ObjId());

	return cBaseScript::OnSlain(pSlayMsg, mpReply);
}


/***
 * TimedPotion
 */
cScr_TimedPotion::cScr_TimedPotion(const char* pszName, int iHostObjId)
	: cBaseScript(pszName, iHostObjId)
{
	static const sMessageHandler handlers[] = {{"StartPotion",HandleStartPotion}};
	RegisterMessageHandlers(handlers, 1);
}

long cScr_TimedPotion::HandleStartPotion(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cScr_TimedPotion*>(pScript)->OnStartPotion(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cScr_TimedPotion::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	link lCont;
	pLS->GetOne(lCont, pLTS->LinkKindNamed("~Contains"), ObjId(), 0);
	if (lCont)
	{
		sLink sl;
		pLTS->LinkGet(lCont, sl);

		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(sl.dest, "AI"))
		{
			AddSingleMetaProperty("M-QuaffHeal", sl.dest);
		}
	}

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_TimedPotion::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!strcmp(pTimerMsg->name, "PreEndPotion"))
	{
		SService<ISoundScrSrv> pSound(g_pScriptManager);
		true_bool _p;
		pSound->PlayEnvSchema(_p, ObjId(), "Event Deactivate", ObjId(), 0,
			(static_cast<int>(pTimerMsg->data) == StrToObject("Player")) ? kEnvSoundAmbient : kEnvSoundAtObjLoc);

		tScrTimer hTimer = SetTimedMessage("EndPotion", 1000, kSTM_OneShot, pTimerMsg->data);
		SetScriptData("Timer", reinterpret_cast<int>(hTimer));
	}
	else if (!strcmp(pTimerMsg->name, "EndPotion"))
	{
		StopPotion(static_cast<int>(pTimerMsg->data));

		ClearScriptData("Timer");

		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());

		return 0;
	}

	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}

long cScr_TimedPotion::OnFrobInvEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	object oPotionClone = 0;

	true_bool bHasLink;
	pLS->AnyExist(bHasLink, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
	if (bHasLink)
	{
		cMultiParm mpMyType, mpHisType;
		if (pPS->Possessed(ObjId(), "CombineType"))
		{
			pPS->Get(mpMyType, ObjId(), "CombineType", NULL);
			linkset lsLinks;
			pLS->GetAll(lsLinks, pLTS->LinkKindNamed("ScriptParams"), ObjId(), 0);
			for (; lsLinks.AnyLinksLeft(); lsLinks.NextLink())
			{
				if (lsLinks.Data() && !_stricmp(reinterpret_cast<const char*>(lsLinks.Data()), "Potion"))
				{
					sLink sl = lsLinks.Get();
					if (pPS->Possessed(ObjId(), "CombineType"))
					{
						pPS->Get(mpHisType, sl.dest, "CombineType", NULL);
						if (!_stricmp(mpMyType, mpHisType))
						{
							oPotionClone = sl.dest;
							break;
						}
					}
				}
			}
		}
	}

	if (!oPotionClone)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->BeginCreate(oPotionClone, ObjId());
		if (!oPotionClone)
			return 1;
		pPS->Add(oPotionClone, "HasRefs");
		pPS->SetSimple(oPotionClone, "HasRefs", 0);
		link l;
		pLS->Create(l, pLTS->LinkKindNamed("ScriptParams"), ObjId(), oPotionClone);
		if (l)
			pLTS->LinkSetData(l, NULL, "Potion");
		pOS->EndCreate(oPotionClone);
	}

	PostMessage(oPotionClone, "StartPotion", pFrobMsg->Frobber);

	return cBaseScript::OnFrobInvEnd(pFrobMsg, mpReply);
}

long cScr_TimedPotion::OnStartPotion(sScrMsg* pMsg, cMultiParm&)
{
	int iTimeout = 8600;
	if (IsScriptDataSet("Timer"))
	{
		cMultiParm mpHandle = GetScriptData("Timer");
		KillTimedMessage(reinterpret_cast<tScrTimer>(static_cast<int>(mpHandle)));
	}

	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "ScriptTiming"))
	{
		cMultiParm mpTiming;
		pPS->Get(mpTiming, ObjId(), "ScriptTiming", NULL);
		iTimeout = static_cast<int>(mpTiming);
		if (iTimeout < 1000)
			iTimeout = 1000;
	}

	tScrTimer hTimer = SetTimedMessage("PreEndPotion", iTimeout, kSTM_OneShot, pMsg->data);
	SetScriptData("Timer", reinterpret_cast<int>(hTimer));

	StartPotion(static_cast<int>(pMsg->data), iTimeout);

	return 0;
}


/***
 * Invisible
 */
long cScr_Invisible::OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!_stricmp(pMsg->message, "Invisible"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(ObjId(), "AI_Visibility"))
		{
			pPS->Set(ObjId(), "AI_Visibility", "Light rating", 0);
			pPS->Set(ObjId(), "AI_Visibility", "Movement rating", 0);
			pPS->Set(ObjId(), "AI_Visibility", "Exposure rating", 0);
		}

		DeleteAllLinks("~AIAttack", ObjId(), 0);

		return 0;
	}

	return cBaseScript::OnMessage(pMsg, mpReply);
}


/***
 * InvisiPotion
 */
void cScr_InvisiPotion::StartPotion(object iFrobber, ulong iDuration)
{
	if (iFrobber == StrToObject("Player"))
	{
		AddSingleMetaProperty("M-Invisible", iFrobber);

		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(iFrobber, "AI_Visibility"))
		{
			pPS->Set(iFrobber, "AI_Visibility", "Last update time", iDuration + GetSimTime());
		}

		PostMessage(iFrobber, "Invisible");
	}
}

void cScr_InvisiPotion::StopPotion(object iFrobber)
{
	if (iFrobber == StrToObject("Player"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		if (pPS->Possessed(iFrobber, "AI_Visibility"))
		{
			pPS->Set(iFrobber, "AI_Visibility", "Last update time", GetSimTime());
		}
		RemoveSingleMetaProperty("M-Invisible", iFrobber);
		PostMessage(iFrobber, "Visible");
	}
}


/***
 * LoGravPotion
 */
void cScr_LoGravPotion::StartPotion(object iFrobber, ulong)
{
	if (iFrobber == StrToObject("Player"))
	{
		SService<IPhysSrv> pPS(g_pScriptManager);
		pPS->SetGravity(iFrobber, 0.5);
		cScrVec vSpeed;
		pPS->GetVelocity(iFrobber, vSpeed);
		vSpeed.z /= 2.0;
		pPS->SetVelocity(iFrobber, vSpeed);

		SService<IDarkInvSrv> pDS(g_pScriptManager);
		pDS->AddSpeedControl("LoGrav", 0.5, 1.0);
	}
}

void cScr_LoGravPotion::StopPotion(object iFrobber)
{
	if (iFrobber == StrToObject("Player"))
	{
		SService<IPhysSrv> pPS(g_pScriptManager);
		pPS->SetGravity(iFrobber, 1.0);

		SService<IDarkInvSrv> pDS(g_pScriptManager);
		pDS->RemoveSpeedControl("LoGrav");
	}
}


/***
 * LastMissionLoot
 */
long cScr_LastMissionLoot::OnDifficulty(sDiffScrMsg* pDiffMsg, cMultiParm& mpReply)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	if (pQS->Exists("total_loot"))
	{
		SService<IPropertySrv> pPS(g_pScriptManager);
		int total_loot = pQS->Get("total_loot");
		if (total_loot > 0)
		{
			if (!pPS->Possessed(ObjId(), "Loot"))
				pPS->Add(ObjId(), "Loot");
			pPS->Set(ObjId(), "Loot", "Gold", total_loot);
			pPS->Set(ObjId(), "Loot", "Gems", 0);
			pPS->Set(ObjId(), "Loot", "Art", 0);
		}
		else
		{
			if (pPS->Possessed(ObjId(), "Loot"))
				pPS->Remove(ObjId(), "Loot");
		}
	}

	return cBaseScript::OnDifficulty(pDiffMsg, mpReply);
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
 * ReGravitize
 */
long cScr_ReGravitize::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_ReGravitize::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_ReGravitize::OnPhysCollision(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (pPS->Possessed(ObjId(), "PhysAttr"))
	{
		pPS->Set(ObjId(), "PhysAttr", "Gravity %", 3.2578125f);
	}

	return cBaseScript::OnPhysContactCreate(pPhysMsg, mpReply);
}


/***
 * TrapSlayer
 */
int cScr_SlayTrap::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript* pScript, void* pData)
{
	cScr_SlayTrap* scrSlayTrap = static_cast<cScr_SlayTrap*>(pScript);
	object iDamageType = reinterpret_cast<int>(pData);
	SService<IPropertySrv> pPS(g_pScriptManager);
	SService<IDamageSrv> pDS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	if (pPS->Possessed(sl.dest, "HitPoints"))
	{
		pPS->SetSimple(sl.dest, "HitPoints", 1);
		pDS->Damage(sl.dest, scrSlayTrap->ObjId(), 1, iDamageType);
	}
	pDS->Slay(sl.dest, scrSlayTrap->ObjId());

	return 1;
}

long cScr_SlayTrap::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		object iDamageType;
		pOS->Named(iDamageType, "BashStim");

		IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, reinterpret_cast<void*>(int(iDamageType)));

		pOS->Destroy(ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * TrapTerminator
 */
int cScr_ReallyDestroy::LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void*)
{
	SService<IObjectSrv> pOS(g_pScriptManager);
	sLink sl;
	pLQ->Link(&sl);
	pOS->Destroy(sl.dest);

	return 1;
}

long cScr_ReallyDestroy::OnSwitch(bool bTurnOn, sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (bTurnOn)
	{
		IterateLinks("ControlDevice", ObjId(), 0, LinkIter, this, NULL);

		SService<IObjectSrv> pOS(g_pScriptManager);
		pOS->Destroy(ObjId());
	}

	return cBaseTrap::OnSwitch(bTurnOn, pMsg, mpReply);
}


/***
 * NumberButton
 */
void cScr_NumberButton::ActivateButton(object iFrobber)
{
	SService<ISoundScrSrv> pSound(g_pScriptManager);
	true_bool _p;
	pSound->PlayEnvSchema(_p, ObjId(), "Event Activate", ObjId(), 0, kEnvSoundOnObj);

	SService<IActReactSrv> pAR(g_pScriptManager);
	pAR->React(pAR->GetReactionNamed("tweq_control"), 1.0, ObjId(), 0, kTweqTypeJoints, kTweqDoActivate,
		cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef, cMultiParm::Undef);

	SService<ILockSrv> pLock(g_pScriptManager);
	if (pLock->IsLocked(ObjId()))
		return;

	CDSend("TurnOn", ObjId(), iFrobber);

	SService<IQuestSrv> pQS(g_pScriptManager);
	SService<IPropertySrv> pPS(g_pScriptManager);
	cAnsiStr strTxRep, strQVar;
	if (pPS->Possessed(ObjId(), "OTxtRepr0"))
	{
		cMultiParm mpTxRep;
		pPS->Get(mpTxRep, ObjId(), "OTxtRepr0", NULL);
		strTxRep = mpTxRep;
	}
	if (pPS->Possessed(ObjId(), "TrapQVar"))
	{
		cMultiParm mpQVar;
		pPS->Get(mpQVar, ObjId(), "TrapQVar", NULL);
		strQVar = mpQVar;
	}
	if (!strTxRep.IsEmpty() && !strQVar.IsEmpty())
	{
		uint basename = strTxRep.ReverseExcluding("\\/");
		if (basename == cAnsiStr::MaxPos)
			basename = 0;
		uint digit = strTxRep.FindOneOf("0123456789", basename);
		if (digit != cAnsiStr::MaxPos)
		{
			int qvar = pQS->Exists(strQVar) ? pQS->Get(strQVar) : 0;
			qvar = ((qvar * 10) + (strTxRep[digit] - '0')) % 100000000;
			pQS->Set(strQVar, qvar, kQuestDataMission);
		}
	}
}

long cScr_NumberButton::OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->SubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnBeginScript(pMsg, mpReply);
}

long cScr_NumberButton::OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply)
{
	SService<IPhysSrv> pPhys(g_pScriptManager);
	pPhys->UnsubscribeMsg(ObjId(), kPhysCollision);

	return cBaseScript::OnEndScript(pMsg, mpReply);
}

long cScr_NumberButton::OnPhysCollision(sPhysMsg* pPhysMsg, cMultiParm& mpReply)
{
	if (pPhysMsg->collSubmod == 4)
	{
		SService<IObjectSrv> pOS(g_pScriptManager);
		true_bool bInherits;
		object iArc, iCret;
		if (!*pOS->InheritsFrom(bInherits, pPhysMsg->collObj, *pOS->Named(iArc, "Avatar"))
		 && !*pOS->InheritsFrom(bInherits, pPhysMsg->collObj, *pOS->Named(iCret, "Creature")))
		{
			ActivateButton(pPhysMsg->collObj);
		}
	}

	return cBaseScript::OnPhysCollision(pPhysMsg, mpReply);
}

long cScr_NumberButton::OnFrobWorldEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply)
{
	ActivateButton(pFrobMsg->Frobber);
	return cBaseScript::OnFrobWorldEnd(pFrobMsg, mpReply);
}

#endif // _DARKGAME == 1
