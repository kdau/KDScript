/******************************************************************************
 *  BaseScript.cpp
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
#include "BaseScript.h"
#include "ScriptModule.h"

#include <lg/interface.h>
#include <lg/scrmanagers.h>

#include "ScriptLib.h"

#include <exception>
#include <cstring>

#ifdef __BORLANDC__
/* The free BCC doesn't include TASM. */
/* We could use this all the time, but it's easier to debug if we break directly. */
#define DO_BREAK	::DebugBreak()
#else
#define DO_BREAK	asm("int 3")
#endif

const sMessageHandler cBaseScript::sm_BaseMessageHandlers[] = {
	{ "BeginScript", cBaseScript::HandleBeginScript },
	{ "EndScript", cBaseScript::HandleEndScript },
	{ "Timer", cBaseScript::HandleTimer },
	{ "Sim", cBaseScript::HandleSim },
#if (_DARKGAME == 2)
	{ "DarkGameModeChange", cBaseScript::HandleDarkGameModeChange },
#endif
	{ "Difficulty", cBaseScript::HandleDifficulty },
	{ "QuestChange", cBaseScript::HandleQuestChange },
	{ "Create", cBaseScript::HandleCreate },
	{ "Destroy", cBaseScript::HandleDestroy },
	{ "Slain", cBaseScript::HandleSlain },
	{ "Damage", cBaseScript::HandleDamage },
	{ "Container", cBaseScript::HandleContainer },
	{ "Contained", cBaseScript::HandleContained },
	{ "Combine", cBaseScript::HandleCombine },
	{ "TweqComplete", cBaseScript::HandleTweqComplete },
	{ "PhysMadePhysical", cBaseScript::HandlePhysMadePhysical },
	{ "PhysMadeNonPhysical", cBaseScript::HandlePhysMadeNonPhysical },
	{ "PhysFellAsleep", cBaseScript::HandlePhysFellAsleep },
	{ "PhysWokeUp", cBaseScript::HandlePhysWokeUp },
	{ "PhysCollision", cBaseScript::HandlePhysCollision },
	{ "PhysEnter", cBaseScript::HandlePhysEnter },
	{ "PhysExit", cBaseScript::HandlePhysExit },
	{ "PhysContactCreate", cBaseScript::HandlePhysContactCreate },
	{ "PhysContactDestroy", cBaseScript::HandlePhysContactDestroy },
#if (_DARKGAME == 2)
	{ "MediumTransition", cBaseScript::HandleMediumTransition },
#endif
	{ "ObjRoomTransit", cBaseScript::HandleObjRoomTransit },
	{ "WorldFocus", cBaseScript::HandleWorldFocus },
	{ "WorldDeFocus", cBaseScript::HandleWorldDeFocus },
	{ "WorldSelect", cBaseScript::HandleWorldSelect },
	{ "WorldDeSelect", cBaseScript::HandleWorldDeSelect },
	{ "InvFocus", cBaseScript::HandleInvFocus },
	{ "InvDeFocus", cBaseScript::HandleInvDeFocus },
	{ "InvSelect", cBaseScript::HandleInvSelect },
	{ "InvDeSelect", cBaseScript::HandleInvDeSelect },
	{ "FrobWorldBegin", cBaseScript::HandleFrobWorldBegin },
	{ "FrobWorldEnd", cBaseScript::HandleFrobWorldEnd },
	{ "FrobInvBegin", cBaseScript::HandleFrobInvBegin },
	{ "FrobInvEnd", cBaseScript::HandleFrobInvEnd },
	{ "FrobToolBegin", cBaseScript::HandleFrobToolBegin },
	{ "FrobToolEnd", cBaseScript::HandleFrobToolEnd },
	{ "NowLocked", cBaseScript::HandleNowLocked },
	{ "NowUnlocked", cBaseScript::HandleNowUnlocked },
	{ "TurnOn", cBaseScript::HandleTurnOn },
	{ "TurnOff", cBaseScript::HandleTurnOff },
	{ "SchemaDone", cBaseScript::HandleSchemaDone },
	{ "SoundDone", cBaseScript::HandleSoundDone },
#if (_DARKGAME == 3)
	{ "KeypadDone", cBaseScript::HandleKeypadDone },
	{ "YorNDone", cBaseScript::HandleYorNDone },
#endif
};

const sMessageHandler cBaseRoomScript::sm_RoomMessageHandlers[] = {
	{ "ObjectRoomEnter", cBaseRoomScript::HandleObjectRoomEnter },
	{ "ObjectRoomExit", cBaseRoomScript::HandleObjectRoomExit },
	{ "CreatureRoomEnter", cBaseRoomScript::HandleCreatureRoomEnter },
	{ "CreatureRoomExit", cBaseRoomScript::HandleCreatureRoomExit },
	{ "PlayerRoomEnter", cBaseRoomScript::HandlePlayerRoomEnter },
	{ "PlayerRoomExit", cBaseRoomScript::HandlePlayerRoomExit },
	{ "RemotePlayerRoomEnter", cBaseRoomScript::HandleRemotePlayerRoomEnter },
	{ "RemotePlayerRoomExit", cBaseRoomScript::HandleRemotePlayerRoomExit },
};

const sMessageHandler cBaseDoorScript::sm_DoorMessageHandlers[] = {
	{ "DoorOpen", cBaseDoorScript::HandleDoorOpen },
	{ "DoorClose", cBaseDoorScript::HandleDoorClose },
	{ "DoorOpening", cBaseDoorScript::HandleDoorOpening },
	{ "DoorClosing", cBaseDoorScript::HandleDoorClosing },
	{ "DoorHalt", cBaseDoorScript::HandleDoorHalt },
#if (_DARKGAME == 2)
	{ "PickStateChange", cBaseDoorScript::HandlePickStateChange },
#endif
};

const sMessageHandler cBaseAIScript::sm_AIMessageHandlers[] = {
	{ "AIModeChange", cBaseAIScript::HandleAIModeChange },
	{ "Alertness", cBaseAIScript::HandleAlertness },
#if (_DARKGAME == 2)
	{ "HighAlert", cBaseAIScript::HandleHighAlert },
#endif
	{ "SignalAI", cBaseAIScript::HandleSignalAI },
	{ "ObjActResult", cBaseAIScript::HandleObjActResult },
	{ "PatrolPoint", cBaseAIScript::HandlePatrolPoint },
	{ "StartAttack", cBaseAIScript::HandleStartAttack },
	{ "StartWindup", cBaseAIScript::HandleStartWindup },
	{ "EndAttack", cBaseAIScript::HandleEndAttack },
	{ "MotionStart", cBaseAIScript::HandleMotionStart },
	{ "MotionEnd", cBaseAIScript::HandleMotionEnd },
	{ "MotionFlagReached", cBaseAIScript::HandleMotionFlagReached },
};

const sMessageHandler cBaseMovingTerrainScript::sm_MovingTerrainMessageHandlers[] = {
	{ "MovingTerrainWaypoint", HandleMovingTerrainWaypoint },
	{ "WaypointReached", HandleWaypointReached },
	{ "Call", HandleCall },
};

const sMessageHandler cBasePPlateScript::sm_PPlateMessageHandlers[] = {
	{ "PressurePlateActive", HandlePressurePlateActive },
	{ "PressurePlateInactive", HandlePressurePlateInactive },
	{ "PressurePlateActivating", HandlePressurePlateActivating },
	{ "PressurePlateDeactivating", HandlePressurePlateDeactivating },
};


void cBaseScript::InitScript(void)
{
	RegisterMessageHandlers(sm_BaseMessageHandlers, sizeof(sm_BaseMessageHandlers)/sizeof(sm_BaseMessageHandlers[0]));

	SInterface<ISimManager> pSim(g_pScriptManager);
	m_bSim = pSim->LastMsg() & (kSimStart | kSimResume);
}

void cBaseRoomScript::InitScript(void)
{
	RegisterMessageHandlers(sm_RoomMessageHandlers, sizeof(sm_RoomMessageHandlers)/sizeof(sm_RoomMessageHandlers[0]));
}

void cBaseDoorScript::InitScript(void)
{
	RegisterMessageHandlers(sm_DoorMessageHandlers, sizeof(sm_DoorMessageHandlers)/sizeof(sm_DoorMessageHandlers[0]));
}

void cBaseAIScript::InitScript(void)
{
	RegisterMessageHandlers(sm_AIMessageHandlers, sizeof(sm_AIMessageHandlers)/sizeof(sm_AIMessageHandlers[0]));
}

void cBaseMovingTerrainScript::InitScript(void)
{
	RegisterMessageHandlers(sm_MovingTerrainMessageHandlers, sizeof(sm_MovingTerrainMessageHandlers)/sizeof(sm_MovingTerrainMessageHandlers[0]));
}

void cBasePPlateScript::InitScript(void)
{
	RegisterMessageHandlers(sm_PPlateMessageHandlers, sizeof(sm_PPlateMessageHandlers)/sizeof(sm_PPlateMessageHandlers[0]));
}


cBaseScript::cBaseScript(const char* pszName, int iHostObjId)
	: cScript(pszName, iHostObjId),
	  m_bSim(false), m_iMessageTime(0)
{
	InitScript();
}

cBaseScript::~cBaseScript()
{
	for (unsigned int n = 0; n < m_aDynamicHandlers.size(); ++n)
	{
		delete[] m_aDynamicHandlers[n]->pszName;
		delete m_aDynamicHandlers[n];
	}
}

STDMETHODIMP cBaseScript::ReceiveMessage(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace)
{
	long iRet = 0;
	cScript::ReceiveMessage(pMsg, pReply, eTrace);

	switch (eTrace)
	{
	case kSpew:
	{
		char msg[64];
		snprintf(msg, 64, "\"%s\" at %d", pMsg->message, pMsg->time);
		DebugString("Got message ", msg);
		break;
	}
	case kBreak:
	{
		char msg[64];
		snprintf(msg, 64, "\"%s\" at %d", pMsg->message, pMsg->time);
		DebugString("Break on message ", msg);
		DO_BREAK;
		break;
	}
	default:
		break;
	}

	m_iMessageTime = pMsg->time;
	if (!::_stricmp(pMsg->message, "Sim"))
	{
		m_bSim = static_cast<sSimMsg*>(pMsg)->fStarting;
	}

	try
	{
		sMultiParm mpReply;
		// Easier than changing the interface or having umpteen checks for NULL
		if (pReply == NULL)
			pReply = &mpReply;
		iRet = DispatchMessage(pMsg, pReply);
	}
	catch (std::exception& err)
	{
		DebugString("An error occurred, ", err.what());
		iRet = S_FALSE;
	}
	catch (...)
	{
		DebugString("An unknown error occurred.");
		iRet = S_FALSE;
	}
	return iRet;
}

void cBaseScript::RegisterMessageHandlers(const sMessageHandler* pHandlers, uint iCount)
{
	if (iCount == 0)
		return;
	m_aMessageHandlers.resize(m_aMessageHandlers.size() + iCount);
	for (uint n=0; n<iCount; ++n)
	{
		insert_sorted(m_aMessageHandlers, sMessageHandlerNode(pHandlers+n));
	}
}

void cBaseScript::RegisterDynamicMessageHandler(const char* pszName, MessageHandlerProc pfnHandler)
{
	for (uint n = 0; n < m_aDynamicHandlers.size(); ++n)
	{
		if (!::_stricmp(m_aDynamicHandlers[n]->pszName, pszName))
		{
			sMessageHandler* pHandler = m_aDynamicHandlers[n];
			delete[] pHandler->pszName;
			pHandler->pszName = new char[::strlen(pszName)+1];
			::strcpy(const_cast<char*>(pHandler->pszName), pszName);
			pHandler->pfnHandler = pfnHandler;
			// The dynamic sMessageHandler should already be registered.
			return;
		}
	}

	uint n = find_first(m_aMessageHandlers, pszName);
	if (n >= m_aMessageHandlers.size() || m_aMessageHandlers[n] != pszName)
	{
		sMessageHandler* pHandler = new sMessageHandler;
		pHandler->pszName = new char[::strlen(pszName)+1];
		::strcpy(const_cast<char*>(pHandler->pszName), pszName);
		pHandler->pfnHandler = pfnHandler;
		m_aDynamicHandlers.append(pHandler);
		insert_sorted(m_aMessageHandlers, sMessageHandlerNode(pHandler));
	}
}

void cBaseScript::UnregisterMessageHandler(const char* pszName)
{
	for (uint n = 0; n < m_aDynamicHandlers.size(); ++n)
	{
		if (!::_stricmp(m_aDynamicHandlers[n]->pszName, pszName))
		{
			uint m = find_first(m_aMessageHandlers, pszName);
			if (n < m_aMessageHandlers.size()
			 && m_aMessageHandlers[m] == pszName
			 && m_aMessageHandlers[m].pData->pfnHandler == m_aDynamicHandlers[n]->pfnHandler)
			{
				m_aMessageHandlers.erase(m);
			}
			delete[] m_aDynamicHandlers[n]->pszName;
			delete m_aDynamicHandlers[n];
			m_aDynamicHandlers.erase(n);

			break;
		}
	}
}

long cBaseScript::DispatchMessage(sScrMsg* pMsg, sMultiParm* pReply)
{
	uint n = find_first(m_aMessageHandlers, pMsg->message);
	if (n < m_aMessageHandlers.size() && m_aMessageHandlers[n] == pMsg->message)
	{
		return m_aMessageHandlers[n].pData->pfnHandler(this, pMsg, pReply);
	}
	return OnMessage(pMsg, static_cast<cMultiParm&>(*pReply));
}

cMultiParm* cBaseScript::SendMessage(cMultiParm& mpReply, object iDest, const char* pszMessage, const cMultiParm& mpData1, const cMultiParm& mpData2, const cMultiParm& mpData3)
{
	return g_pScriptManager->SendMessage2(mpReply, ObjId(), iDest, pszMessage, mpData1, mpData2, mpData3);
}

void cBaseScript::PostMessage(object iDest, const char* pszMessage, const cMultiParm& mpData1, const cMultiParm& mpData2, const cMultiParm& mpData3)
{
#if (_DARKGAME == 1)
	g_pScriptManager->PostMessage2(ObjId(), iDest, pszMessage, mpData1, mpData2, mpData3);
#else
	g_pScriptManager->PostMessage2(ObjId(), iDest, pszMessage, mpData1, mpData2, mpData3, kScrMsgPostToOwner);
#endif
}

tScrTimer cBaseScript::SetTimedMessage(const char* pszName, ulong iTime, eScrTimedMsgKind eType, const cMultiParm& mpData)
{
	return g_pScriptManager->SetTimedMessage2(ObjId(), pszName, iTime, eType, mpData);
}

void cBaseScript::KillTimedMessage(tScrTimer hTimer)
{
	g_pScriptManager->KillTimedMessage(hTimer);
}

bool cBaseScript::IsScriptDataSet(const char* pszName)
{
	sScrDatumTag tag;
	tag.objId = ObjId();
	tag.pszClass = Name();
	tag.pszName = pszName;
	return g_pScriptManager->IsScriptDataSet(&tag);
}

cMultiParm cBaseScript::GetScriptData(const char* pszName)
{
	cMultiParm mpRet;
	sScrDatumTag tag;
	tag.objId = ObjId();
	tag.pszClass = Name();
	tag.pszName = pszName;
	g_pScriptManager->GetScriptData(&tag, &mpRet);
	return mpRet;
}

void cBaseScript::SetScriptData(const char* pszName, const cMultiParm& mpData)
{
	sScrDatumTag tag;
	tag.objId = ObjId();
	tag.pszClass = Name();
	tag.pszName = pszName;
	g_pScriptManager->SetScriptData(&tag, &mpData);
}

cMultiParm cBaseScript::ClearScriptData(const char* pszName)
{
	cMultiParm mpRet;
	sScrDatumTag tag;
	tag.objId = ObjId();
	tag.pszClass = Name();
	tag.pszName = pszName;
	g_pScriptManager->ClearScriptData(&tag, &mpRet);
	return mpRet;
}

int cBaseScript::FixupPlayerLinks(void)
{
	int iPlayer = StrToObject("Player");
	if (iPlayer)
		::FixupPlayerLinks(ObjId(), iPlayer);
	else
		g_pScriptManager->SetTimedMessage2(ObjId(), "DelayInit", 1, kSTM_OneShot, "FixupPlayerLinks");
	return 0;
}

long cBaseScript::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm&)
{
	if (!::strcmp(pTimerMsg->name, "DelayInit") && pTimerMsg->data == "FixupPlayerLinks")
	{
		int iPlayer = StrToObject("Player");
		if (iPlayer)
			::FixupPlayerLinks(ObjId(), iPlayer);
	}
	return 0;
}

void cBaseScript::DebugString(const char* pszMsg1, const char* pszMsg2)
{
	g_pfnMPrintf("%s [%d]: %s%s\n", Name(), ObjId(), pszMsg1, pszMsg2);
}

long cBaseScript::HandleBeginScript(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnBeginScript(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleEndScript(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnEndScript(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleTimer(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnTimer(static_cast<sScrTimerMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleSim(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnSim(static_cast<sSimMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

#if (_DARKGAME == 2)
long cBaseScript::HandleDarkGameModeChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnDarkGameModeChange(static_cast<sDarkGameModeScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}
#endif

long cBaseScript::HandleDifficulty(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnDifficulty(static_cast<sDiffScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleQuestChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnQuestChange(static_cast<sQuestMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleCreate(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnCreate(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleDestroy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnDestroy(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleSlain(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnSlain(static_cast<sSlayMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleDamage(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnDamage(static_cast<sDamageScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleContainer(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnContainer(static_cast<sContainerScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleContained(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnContained(static_cast<sContainedScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleCombine(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnCombine(static_cast<sCombineScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleTweqComplete(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnTweqComplete(static_cast<sTweqMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysMadePhysical(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysMadePhysical(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysMadeNonPhysical(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysMadeNonPhysical(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysFellAsleep(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysFellAsleep(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysWokeUp(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysWokeUp(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysCollision(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysCollision(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysEnter(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysExit(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysContactCreate(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysContactCreate(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandlePhysContactDestroy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnPhysContactDestroy(static_cast<sPhysMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

#if (_DARKGAME == 2)
long cBaseScript::HandleMediumTransition(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnMediumTransition(static_cast<sMediumTransMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}
#endif

long cBaseScript::HandleObjRoomTransit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnObjRoomTransit(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleWorldFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnWorldFocus(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleWorldDeFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnWorldDeFocus(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleWorldSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnWorldSelect(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleWorldDeSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnWorldDeSelect(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleInvFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnInvFocus(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleInvDeFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnInvDeFocus(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleInvSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnInvSelect(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleInvDeSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnInvDeSelect(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobWorldBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobWorldBegin(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobWorldEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobWorldEnd(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobInvBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobInvBegin(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobInvEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobInvEnd(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobToolBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobToolBegin(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleFrobToolEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnFrobToolEnd(static_cast<sFrobMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleNowLocked(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnNowLocked(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleNowUnlocked(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnNowUnlocked(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleTurnOn(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnTurnOn(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleTurnOff(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnTurnOff(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleSchemaDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnSchemaDone(static_cast<sSchemaDoneMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleSoundDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnSoundDone(static_cast<sSoundDoneMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

#if (_DARKGAME == 3)
long cBaseScript::HandleKeypadDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnKeypadDone(static_cast<sKeypadMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseScript::HandleYorNDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseScript*>(pScript)->OnYorNDone(static_cast<sYorNMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}
#endif

long cBaseRoomScript::HandleObjectRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnObjectRoomEnter(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandleObjectRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnObjectRoomExit(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandleCreatureRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnCreatureRoomEnter(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandleCreatureRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnCreatureRoomExit(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandlePlayerRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnPlayerRoomEnter(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandlePlayerRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnPlayerRoomExit(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandleRemotePlayerRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnRemotePlayerRoomEnter(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseRoomScript::HandleRemotePlayerRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseRoomScript*>(pScript)->OnRemotePlayerRoomExit(static_cast<sRoomMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseDoorScript::HandleDoorOpen(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnDoorOpen(static_cast<sDoorMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseDoorScript::HandleDoorClose(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnDoorClose(static_cast<sDoorMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseDoorScript::HandleDoorOpening(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnDoorOpening(static_cast<sDoorMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseDoorScript::HandleDoorClosing(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnDoorClosing(static_cast<sDoorMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseDoorScript::HandleDoorHalt(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnDoorHalt(static_cast<sDoorMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

#if (_DARKGAME == 2)
long cBaseDoorScript::HandlePickStateChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseDoorScript*>(pScript)->OnPickStateChange(static_cast<sPickStateScrMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}
#endif

long cBaseAIScript::HandleAIModeChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnAIModeChange(static_cast<sAIModeChangeMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleAlertness(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnAlertness(static_cast<sAIAlertnessMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

#if (_DARKGAME == 2)
long cBaseAIScript::HandleHighAlert(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnHighAlert(static_cast<sAIHighAlertMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}
#endif

long cBaseAIScript::HandleSignalAI(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnSignalAI(static_cast<sAISignalMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleObjActResult(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnObjActResult(static_cast<sAIObjActResultMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandlePatrolPoint(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnPatrolPoint(static_cast<sAIPatrolPointMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleStartAttack(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnStartAttack(static_cast<sAttackMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleStartWindup(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnStartWindup(static_cast<sAttackMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleEndAttack(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnEndAttack(static_cast<sAttackMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleMotionStart(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnMotionStart(static_cast<sBodyMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleMotionEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnMotionEnd(static_cast<sBodyMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseAIScript::HandleMotionFlagReached(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseAIScript*>(pScript)->OnMotionFlagReached(static_cast<sBodyMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseMovingTerrainScript::HandleMovingTerrainWaypoint(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseMovingTerrainScript*>(pScript)->OnMovingTerrainWaypoint(static_cast<sMovingTerrainMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseMovingTerrainScript::HandleWaypointReached(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseMovingTerrainScript*>(pScript)->OnWaypointReached(static_cast<sWaypointMsg*>(pMsg), static_cast<cMultiParm&>(*pReply));
}

long cBaseMovingTerrainScript::HandleCall(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBaseMovingTerrainScript*>(pScript)->OnCall(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBasePPlateScript::HandlePressurePlateActive(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBasePPlateScript*>(pScript)->OnPressurePlateActive(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBasePPlateScript::HandlePressurePlateInactive(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBasePPlateScript*>(pScript)->OnPressurePlateInactive(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBasePPlateScript::HandlePressurePlateActivating(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBasePPlateScript*>(pScript)->OnPressurePlateActivating(pMsg, static_cast<cMultiParm&>(*pReply));
}

long cBasePPlateScript::HandlePressurePlateDeactivating(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply)
{
	return static_cast<cBasePPlateScript*>(pScript)->OnPressurePlateDeactivating(pMsg, static_cast<cMultiParm&>(*pReply));
}
