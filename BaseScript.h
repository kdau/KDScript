/******************************************************************************
 *  BaseScript.h
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
#ifndef BASESCRIPT_H
#define BASESCRIPT_H

#if !SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/dynarray.h>
#include <lg/scrmanagers.h>
#include "Script.h"
#include "MsgHandlerArray.h"
#endif // SCR_GENSCRIPTS


/**
 * AbstractScript: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBaseScript : public cScript
{
public:
	cBaseScript(const char* pszName, int iHostObjId);
	virtual ~cBaseScript();

	STDMETHOD(ReceiveMessage)(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace);

protected:
	void InitScript(void);

	virtual long OnMessage(sScrMsg* pMsg, cMultiParm& mpReply)
		{ return cScript::ReceiveMessage(pMsg, &mpReply, kNoAction); }
	virtual long OnBeginScript(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnEndScript(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&)
		{ return 0; }
#if (_DARKGAME == 2)
	virtual long OnDarkGameModeChange(sDarkGameModeScrMsg*, cMultiParm&)
		{ return 0; }
#endif
	virtual long OnDifficulty(sDiffScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnQuestChange(sQuestMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnCreate(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDestroy(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnSlain(sSlayMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDamage(sDamageScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnContainer(sContainerScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnContained(sContainedScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnCombine(sCombineScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysMadePhysical(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysMadeNonPhysical(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysFellAsleep(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysWokeUp(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysEnter(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysExit(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysContactCreate(sPhysMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPhysContactDestroy(sPhysMsg*, cMultiParm&)
		{ return 0; }
#if (_DARKGAME == 2)
	virtual long OnMediumTransition(sMediumTransMsg*, cMultiParm&)
		{ return 0; }
#endif
	virtual long OnObjRoomTransit(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnWorldFocus(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnWorldDeFocus(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnWorldSelect(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnWorldDeSelect(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnInvFocus(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnInvDeFocus(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnInvSelect(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnInvDeSelect(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobWorldBegin(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobInvBegin(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobToolBegin(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnFrobToolEnd(sFrobMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnNowLocked(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnTurnOn(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnTurnOff(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnSchemaDone(sSchemaDoneMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnSoundDone(sSoundDoneMsg*, cMultiParm&)
		{ return 0; }
#if (_DARKGAME == 3)
	virtual long OnKeypadDone(sKeypadMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnYorNDone(sYorNMsg*, cMultiParm&)
		{ return 0; }
#endif

private:
	static const sMessageHandler sm_BaseMessageHandlers[];
	cDynArray<sMessageHandlerNode> m_aMessageHandlers;
	cDynArray<sMessageHandler*> m_aDynamicHandlers;

	long DispatchMessage(sScrMsg* pMsg, sMultiParm* pReply);

	static long HandleBeginScript(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleEndScript(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleTimer(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleSim(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#if (_DARKGAME == 2)
	static long HandleDarkGameModeChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#endif
	static long HandleDifficulty(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleQuestChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleCreate(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDestroy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleSlain(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDamage(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleContainer(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleContained(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleCombine(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleTweqComplete(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysMadePhysical(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysMadeNonPhysical(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysFellAsleep(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysWokeUp(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysCollision(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysContactCreate(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePhysContactDestroy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#if (_DARKGAME == 2)
	static long HandleMediumTransition(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#endif
	static long HandleObjRoomTransit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleWorldFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleWorldDeFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleWorldSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleWorldDeSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleInvFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleInvDeFocus(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleInvSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleInvDeSelect(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobWorldBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobWorldEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobInvBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobInvEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobToolBegin(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleFrobToolEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleNowLocked(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleNowUnlocked(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleTurnOn(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleTurnOff(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleSchemaDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleSoundDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#if (_DARKGAME == 3)
	static long HandleKeypadDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleYorNDone(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#endif

protected:
	void RegisterMessageHandlers(const sMessageHandler* pHandlers, uint iCount);
	void RegisterDynamicMessageHandler(const char* pszName, MessageHandlerProc pfnHandler);
	void UnregisterMessageHandler(const char* pszName);

	bool IsSim(void)
		{ return m_bSim; }
	int GetSimTime(void)
		{ return m_iMessageTime; }

	cMultiParm* SendMessage(cMultiParm& mpReply, object iDest, const char* pszMessage, const cMultiParm& mpData1 = cMultiParm::Undef, const cMultiParm& mpData2 = cMultiParm::Undef, const cMultiParm& mpData3 = cMultiParm::Undef);
	void PostMessage(object iDest, const char* pszMessage, const cMultiParm& mpData1 = cMultiParm::Undef, const cMultiParm& mpData2 = cMultiParm::Undef, const cMultiParm& mpData3 = cMultiParm::Undef);
	tScrTimer SetTimedMessage(const char* pszName, ulong iTime, eScrTimedMsgKind eType, const cMultiParm& mpData = cMultiParm::Undef);
	void KillTimedMessage(tScrTimer hTimer);

	bool IsScriptDataSet(const char* pszName);
	cMultiParm GetScriptData(const char* pszName);
	void SetScriptData(const char* pszName, const cMultiParm& mpData);
	cMultiParm ClearScriptData(const char* pszName);

	int FixupPlayerLinks(void);

	void DebugString(const char* pszMsg1, const char* pszmsg2 = "");

private:
	bool m_bSim;
	int m_iMessageTime;

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseScript","CustomScript",cBaseScript)
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseRoomScript
 * Inherits: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBaseRoomScript : public cBaseScript
{
public:
	cBaseRoomScript(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ InitScript(); }

protected:
	void InitScript(void);

	virtual long OnObjectRoomEnter(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnObjectRoomExit(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnCreatureRoomEnter(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnCreatureRoomExit(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPlayerRoomEnter(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPlayerRoomExit(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnRemotePlayerRoomEnter(sRoomMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnRemotePlayerRoomExit(sRoomMsg*, cMultiParm&)
		{ return 0; }

private:
	static const sMessageHandler sm_RoomMessageHandlers[];

	static long HandleObjectRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleObjectRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleCreatureRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleCreatureRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePlayerRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePlayerRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleRemotePlayerRoomEnter(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleRemotePlayerRoomExit(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseRoomScript","BaseScript",cBaseRoomScript)
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseDoorScript
 * Inherits: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBaseDoorScript : public cBaseScript
{
public:
	cBaseDoorScript(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ InitScript(); }

protected:
	void InitScript(void);

	virtual long OnDoorOpen(sDoorMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDoorClose(sDoorMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDoorOpening(sDoorMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDoorClosing(sDoorMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnDoorHalt(sDoorMsg*, cMultiParm&)
		{ return 0; }
#if (_DARKGAME == 2)
	virtual long OnPickStateChange(sPickStateScrMsg*, cMultiParm&)
		{ return 0; }
#endif

private:
	static const sMessageHandler sm_DoorMessageHandlers[];

	static long HandleDoorOpen(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDoorClose(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDoorOpening(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDoorClosing(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleDoorHalt(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#if (_DARKGAME == 2)
	static long HandlePickStateChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#endif
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseDoorScript","BaseScript",cBaseDoorScript)
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseAIScript
 * Inherits: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBaseAIScript : public cBaseScript
{
public:
	cBaseAIScript(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ InitScript(); }

protected:
	void InitScript(void);

	virtual long OnAIModeChange(sAIModeChangeMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnAlertness(sAIAlertnessMsg*, cMultiParm&)
		{ return 0; }
#if (_DARKGAME == 2)
	virtual long OnHighAlert(sAIHighAlertMsg*, cMultiParm&)
		{ return 0; }
#endif
	virtual long OnSignalAI(sAISignalMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnObjActResult(sAIObjActResultMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPatrolPoint(sAIPatrolPointMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnStartAttack(sAttackMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnStartWindup(sAttackMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnEndAttack(sAttackMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnMotionStart(sBodyMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnMotionEnd(sBodyMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnMotionFlagReached(sBodyMsg*, cMultiParm&)
		{ return 0; }

private:
	static const sMessageHandler sm_AIMessageHandlers[];

	static long HandleAIModeChange(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleAlertness(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#if (_DARKGAME == 2)
	static long HandleHighAlert(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
#endif
	static long HandleSignalAI(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleObjActResult(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePatrolPoint(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleStartAttack(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleStartWindup(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleEndAttack(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleMotionStart(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleMotionEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleMotionFlagReached(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseAIScript","BaseScript",cBaseAIScript)
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseMovingTerrainScript
 * Inherits: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBaseMovingTerrainScript : public cBaseScript
{
public:
	cBaseMovingTerrainScript(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ InitScript(); }

protected:
	void InitScript(void);

	virtual long OnMovingTerrainWaypoint(sMovingTerrainMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnWaypointReached(sWaypointMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnCall(sScrMsg*, cMultiParm&)
		{ return 0; }

private:
	static const sMessageHandler sm_MovingTerrainMessageHandlers[];

	static long HandleMovingTerrainWaypoint(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleWaypointReached(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleCall(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseMovingTerrainScript","BaseScript",cBaseMovingTerrainScript)
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BasePPlateScript
 * Inherits: BaseScript
 */
#if !SCR_GENSCRIPTS
class cBasePPlateScript : public cBaseScript
{
public:
	cBasePPlateScript(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ InitScript(); }

protected:
	void InitScript(void);

	virtual long OnPressurePlateActive(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPressurePlateInactive(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPressurePlateActivating(sScrMsg*, cMultiParm&)
		{ return 0; }
	virtual long OnPressurePlateDeactivating(sScrMsg*, cMultiParm&)
		{ return 0; }

private:
	static const sMessageHandler sm_PPlateMessageHandlers[];

	static long HandlePressurePlateActive(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePressurePlateInactive(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePressurePlateActivating(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePressurePlateDeactivating(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BasePPlateScript","BaseScript",cBasePPlateScript)
#endif // SCR_GENSCRIPTS

#endif // BASESCRIPT_H
