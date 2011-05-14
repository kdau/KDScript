/******************************************************************************
 *  SS2Scripts.h
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
#ifndef SS2SCRIPTS_H
#define SS2SCRIPTS_H

#if (_DARKGAME == 3)

#ifndef SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/scrservices.h>
#include <lg/links.h>
#include "BaseScript.h"
#include "BaseTrap.h"
#include "CommonScripts.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS

/**
 * Script: NotifyRegion
 * Inherits: BaseScript
 * Messages: Slain, Obituary
 * Links: ScriptParams(Population)
 *
 * Sends an ``Obituary`` message when slain. The message is sent to
 * objects that link to this one with a ``ScriptParams`` link that has
 * the data set to $$Population$$. OBB and room scripts create the link
 * so they can be notified when a creature dies.
 *
 * This script should be in a metaproperty named ``M-NotifyRegion``
 * which will be added to creatures that enter the room or OBB.
 */
#if !SCR_GENSCRIPTS
class cScr_NotifyRegion : public cBaseScript
{
public:
	cScr_NotifyRegion(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NotifyRegion","BaseScript",cScr_NotifyRegion)
#endif // SCR_GENSCRIPTS

/**
 * Script: StdParticleGroup
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff, Die
 * Properties: SFX\Particles
 *
 * Controls SFX particles. The object is destroyed when it gets the ``Die`` message.
 */
#if !SCR_GENSCRIPTS
class cScr_StdParticleGroup : public cBaseScript
{
public:
	cScr_StdParticleGroup(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("StdParticleGroup","BaseScript",cScr_StdParticleGroup)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapCapacitor
 * Inherits: BaseTrap
 * Properties: Script\Delay Time
 *
 * Relays ``TurnOn`` and ``TurnOff``. When turned on, the trap will not relay
 * any messages until the timer has expired. A message sent to the trap during
 * the delay will be relayed after the time expires. Only the last message received
 * is sent.
 *
 * This trap can be used to prevent an effect from turning off too quickly. It allows
 * sounds or tweq animations to complete before the object is deactivated.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapCapacitor : public cBaseTrap
{
public:
	cScr_TrapCapacitor(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm& mpReply);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapCapacitor","BaseTrap",cScr_TrapCapacitor)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapConverse
 * Inherits: BaseTrap
 * Links: AIConversationActor
 * Properties: AI\Conversations\Conversation
 *
 * Starts a conversation when turned on. Turning off the trap does nothing.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapConverse : public cBaseTrap
{
public:
	cScr_TrapConverse(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapConverse","BaseTrap",cScr_TrapConverse)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapPatrol
 * Inherits: BaseTrap
 * Links: SwitchLink
 * Properties: AI\Ability Settings\Patrol: Does patrol
 *
 * Sets the ``Patrol: Does patrol`` property of objects linked with ``SwitchLink``.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapPatrol : public cBaseTrap
{
public:
	cScr_TrapPatrol(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapPatrol","BaseTrap",cScr_TrapPatrol)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRevert
 * Inherits: BaseTrap
 * Properties: Script\Delay Time
 *
 * Relays ``TurnOn`` and ``TurnOff``. After the time set in ``Script\Delay Time``,
 * the opposite message will be sent.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapRevert : public cBaseTrap
{
public:
	cScr_TrapRevert(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRevert","BaseTrap",cScr_TrapRevert)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapTexture
 * Inherits: BaseTrap
 * Properties: Engine Features\Retexture Radius
 * Parameters: terrreplaceon(string), terrreplaceoff(string)
 *
 * Change the texture on nearby terrain.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapTexture : public cBaseTrap
{
public:
	cScr_TrapTexture(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapTexture","BaseTrap",cScr_TrapTexture)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigAIAlert
 * Inherits: BaseAIScript
 * Messages: Alertness
 * Links: SwitchLink
 *
 * Sends ``TurnOn`` when the AI goes to high alert.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigAIAlert : public cBaseAIScript
{
public:
	cScr_TrigAIAlert(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnAlertness(sAIAlertnessMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigAIAlert","BaseAIScript",cScr_TrigAIAlert)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigWorldFrob
 * Inherits: BaseTrap
 * Messages: FrobWorldEnd
 *
 * Sends ``TurnOn`` when the object is frobbed.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigWorldFrob : public cBaseTrap
{
public:
	cScr_TrigWorldFrob(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigWorldFrob","BaseTrap",cScr_TrigWorldFrob)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigWorldFocus
 * Inherits: BaseTrap
 * Messages: WorldSelect, WorldDeSelect
 *
 * Sends ``TurnOn`` and ``TurnOff`` when the object is hilighted.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigWorldFocus : public cBaseTrap
{
public:
	cScr_TrigWorldFocus(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnWorldSelect(sScrMsg*, cMultiParm&);
	virtual long OnWorldDeSelect(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigWorldFocus","BaseTrap",cScr_TrigWorldFocus)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigInvFrob
 * Inherits: BaseTrap
 * Messages: FrobInvEnd
 *
 * Sends ``TurnOn`` when the object is frobbed in the player's inventory.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigInvFrob : public cBaseTrap
{
public:
	cScr_TrigInvFrob(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigInvFrob","BaseTrap",cScr_TrigInvFrob)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigFlicker
 * Inherits: BaseTrap
 * Messages: TweqComplete
 * Properties: Tweq\Flicker, Tweq\FlickerState
 *
 * Uses a flicker tweq to send alternate ``TurnOn`` and ``TurnOff`` messages.
 * The tweq can be activated and deactivated by sending ``TurnOn`` and ``TurnOff``
 * to the trap. ``TurnOff`` will always be the last message sent when the trap
 * is deactivated.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigFlicker : public cBaseTrap
{
public:
	cScr_TrigFlicker(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_TrigFlicker,m_iFlicker,iHostObjId),
		  SCRIPT_VAROBJ(cScr_TrigFlicker,m_hSafety,iHostObjId)
	{ }

private:
	script_int m_iFlicker;
	script_handle<tScrTimer> m_hSafety;

	void Blink(bool);

protected:
	virtual long OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply)
	{
		m_iFlicker.Init(0);
		m_hSafety.Init();
		return cBaseTrap::OnBeginScript(pMsg, mpReply);
	}
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigFlicker","BaseTrap",cScr_TrigFlicker)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigUnlock
 * Inherits: BaseScript
 * Messages: NowLocked, NowUnlocked
 * Links: SwitchLink
 *
 * Triggers when the object is locked and unlocked.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigUnlock : public cBaseScript
{
public:
	cScr_TrigUnlock(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnNowLocked(sScrMsg*, cMultiParm&);
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigUnlock","BaseScript",cScr_TrigUnlock)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigPPlate
 * Inherits: BasePPlateScript
 * Messages: PressurePlateActive, PressurePlateInactive
 * Links: SwitchLink
 *
 * A pressure plate that triggers when it stops moving.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigPPlate : public cBasePPlateScript
{
public:
	cScr_TrigPPlate(const char* pszName, int iHostObjId)
		: cBasePPlateScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnPressurePlateActive(sScrMsg*, cMultiParm&);
	virtual long OnPressurePlateInactive(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigPPlate","BasePPlateScript",cScr_TrigPPlate)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigPPlateImmed
 * Inherits: BasePPlateScript
 * Messages: PressurePlateActivating, PressurePlateDeactivating
 * Links: SwitchLink
 *
 * A pressure plate that triggers when it starts moving.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigPPlateImmed : public cBasePPlateScript
{
public:
	cScr_TrigPPlateImmed(const char* pszName, int iHostObjId)
		: cBasePPlateScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnPressurePlateActivating(sScrMsg*, cMultiParm&);
	virtual long OnPressurePlateDeactivating(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigPPlateImmed","BasePPlateScript",cScr_TrigPPlateImmed)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRequireAll
 * Inherits: TrapRequirement
 * Links: SwitchLink
 *
 * Counts the ``SwitchLink`` links that control this object and trigger
 * when all of them have sent ``TurnOn``.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapRequireAll : public cBaseTrap, protected cRequirement
{
public:
	cScr_TrapRequireAll(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cRequirement(iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRequireAll","BaseTrap",cScr_TrapRequireAll)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRequireAny
 * Inherits: TrapRequirement
 *
 * Trigger when at least one ``TurnOn`` has been received. The messages
 * don't have to come from ``SwitchLink`` links.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapRequireAny : public cBaseTrap, protected cRequirement
{
public:
	cScr_TrapRequireAny(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cRequirement(iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRequireAny","BaseTrap",cScr_TrapRequireAny)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigContained
 * Inherits: BaseTrap
 * Messages: Contained
 *
 * Sends ``TurnOn`` when the object is picked up or dropped.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigContained : public cBaseTrap
{
public:
	cScr_TrigContained(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigContained","BaseTrap",cScr_TrigContained)
#endif // SCR_GENSCRIPTS


/**
 * Script: TrigRoomPlayerTrans
 * Inherits: BaseRoomScript
 * Messages: PlayerRoomEnter, PlayerRoomExit
 * Links: SwitchLink, ScriptParams(Route)
 *
 * Triggers when the player moves between two designated rooms. This script is
 * used on one of the rooms. A ``ScriptParams`` link with the data $$Route$$ is
 * made to another room. When the player is in the first room and he moves into
 * the other room, then ``TurnOn`` is sent along ``SwitchLink`` links from the
 * first room. If the player moves from the second room to the first, then ``TurnOff``
 * is sent.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomPlayerTrans : public cBaseRoomScript
{
public:
	cScr_TrigRoomPlayerTrans(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId)
	{ }

protected:
	int AreRoomsRelated(object iDest, object iSource);

protected:
	virtual long OnPlayerRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomExit(sRoomMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomPlayerTrans","BaseRoomScript",cScr_TrigRoomPlayerTrans)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomCreature
 * Inherits: BaseRoomScript
 * Messages: CreatureRoomEnter, CreatureRoomExit, PlayerRoomEnter, PlayerRoomExit, Obituary
 * Links: SwitchLink, ScriptParams(Population)
 * Metaproperties: M-NotifyRegion
 *
 * Triggers when any AI or player is in the room.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomCreature : public cBaseRoomScript, protected cTrackPopulation
{
public:
	cScr_TrigRoomCreature(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId),
		  cTrackPopulation(iHostObjId)
	{ }

protected:
	virtual long OnCreatureRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnCreatureRoomExit(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomExit(sRoomMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomCreature","BaseRoomScript",cScr_TrigRoomCreature)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomPopChange
 * Inherits: BaseRoomScript
 * Messages: CreatureRoomEnter, CreatureRoomExit, Obituary
 *
 * Sends ``TurnOn`` when any AI or player enters or exits the room.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomPopChange : public cBaseRoomScript
{
public:
	cScr_TrigRoomPopChange(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnCreatureRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnCreatureRoomExit(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomExit(sRoomMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomPopChange","BaseRoomScript",cScr_TrigRoomPopChange)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomObject
 * Inherits: BaseRoomScript
 * Messages: ObjRoomTransit
 * Links: SwitchLink, ScriptParams(Route)
 *
 * Trigger when this object is inside a designated room. Link from the object to
 * any number of rooms using ``ScriptParams`` links that have the data $$Route$$.
 * The script sends ``TurnOn`` along ``SwitchLink`` links when the object enters
 * any of the rooms. ``TurnOff`` is sent when it exits the room. Moving between
 * two rooms that are both linked will do nothing.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomObject : public cBaseRoomScript
{
public:
	cScr_TrigRoomObject(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnObjRoomTransit(sRoomMsg*, cMultiParm&);

	virtual void ObjEnteringRoom(void);
	virtual void ObjExitingRoom(void);
	bool ObjIsInRoom(void);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomObject","BaseRoomScript",cScr_TrigRoomObject)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomDelivery
 * Inherits: TrigRoomObject
 * Messages: Contained
 *
 * Triggers when an object enters a designated room while it is being held by
 * a player.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomDelivery : public cScr_TrigRoomObject
{
public:
	cScr_TrigRoomDelivery(const char* pszName, int iHostObjId)
		: cScr_TrigRoomObject(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);

	virtual void ObjEnteringRoom(void);
	virtual void ObjExitingRoom(void);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomDelivery","TrigRoomObject",cScr_TrigRoomDelivery)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomDeposit
 * Inherits: TrigRoomObject
 * Messages: Contained
 *
 * Triggers when an object enters a designated room, but only when it is not
 * in a container. If the object is removed from the container (because the player
 * dropped it) then ``TurnOn`` will be sent along ``ControlDevice`` links.
 * Picking up the object or moving it out of the room will send ``TurnOff``.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigRoomDeposit : public cScr_TrigRoomObject
{
public:
	cScr_TrigRoomDeposit(const char* pszName, int iHostObjId)
		: cScr_TrigRoomObject(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);

	virtual void ObjEnteringRoom(void);
	virtual void ObjExitingRoom(void);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomDeposit","TrigRoomObject",cScr_TrigRoomDeposit)
#endif // SCR_GENSCRIPTS


/**
 * Script: AnimLight
 * Inherits: BaseTrap
 * Messages: TurnOn, TurnOff, Toggle, Slain, TweqComplete, LightChange
 * Links: SwitchLink, ParticleAttachement
 * Properties Renderer\AnimLight, Renderer\Self Illumination, AmbientHacked, Tweq\Flicker
 *
 * Control an animated light. The light can be turned on, turned off, or switched
 * between on and off. A light can have ambient schemas or link to a particle SFX
 * that will be activated or deactivated with the light. It also triggers ``SwitchLink``
 * links. If the ``Tweq\Flicker`` property is set, then when the light is turned on it
 * will flicker for the duration of the tweq. After the tweq completes, the light will
 * be set to normal brightness.
 */
#if !SCR_GENSCRIPTS
class cScr_AnimLight : public cBaseTrap
{
public:
	cScr_AnimLight(const char* pszName, int iHostObjId);

private:
	script_int m_iOnMode;
	script_int m_iOffMode;

protected:
	void InitModes(void);
	void TurnLightOn(void);
	void TurnLightOff(void);
	bool IsLightOn(void);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
	virtual long OnLightChange(sScrMsg*, cMultiParm&);
	virtual long OnToggle(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);

private:
	static long HandleLightChange(cScript*, sScrMsg*, sMultiParm*);
	static long HandleToggle(cScript*, sScrMsg*, sMultiParm*);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("AnimLight","BaseTrap",cScr_AnimLight)
#endif // SCR_GENSCRIPTS


/**
 * Script: TrapAddPsi
 * Inherits: BaseTrap
 * Parameters: psi(number)
 *
 * Give the player psi points. The psi is added when the trap is turned on
 * and subtracted when turned off.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapAddPsi : public cBaseTrap
{
public:
	cScr_TrapAddPsi(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapAddPsi","BaseTrap",cScr_TrapAddPsi)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapKeypad
 * Inherits: BaseTrap
 * Messages: KeypadDone
 * Links: ScriptParams(ErrorOutput)
 * Properties: Script\Keypad Code
 *
 * Shows the keypad when turned on. If the correct code is entered, then
 * ``TurnOn`` is sent along ``SwitchLink`` links. An incorrect code will
 * send ``TurnOn`` along ``ScriptParams`` links that have the data set to
 * $$ErrorOutput$$.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapKeypad : public cBaseTrap
{
public:
	cScr_TrapKeypad(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnKeypadDone(sKeypadMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapKeypad","BaseTrap",cScr_TrapKeypad)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapNanites
 * Inherits: BaseTrap
 * Links: ScriptParams(ErrorOutput)
 * Parameters: nanites(number)
 *
 * Relays ``TurnOn`` and ``TurnOff`` if the player has sufficient nanites.
 *
 * With a positive value, this trap will give nanites to the player when turned on.
 * The trap always relays when adding nanites. With a negative value, then
 * the player will have to pay before the message is sent. If the player doesn't
 * have enough nanites, then ``TurnOn`` will be sent along ``ScriptParams``
 * links that have the data set to $$ErrorOutput$$.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapNanites : public cBaseTrap
{
public:
	cScr_TrapNanites(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  m_bNoCharge(false)
	{ }

private:
	bool m_bNoCharge;

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapNanites","BaseTrap",cScr_TrapNanites)
#endif // SCR_GENSCRIPTS

/**
 * Script: TraitMachineReusable
 * Inherits: BaseScript
 * Messages: FrobWorldEnd, TurnOn, TurnOff, Used
 * Links: Lock
 * Properties: Engine Features\Locked, Player\Traits
 * Schemas: (Event Activate)
 *
 * An upgrade panel that can be used more than once.
 * The machine is usable if it is unlocked. Sending ``TurnOn`` will
 * unlock (enable) the machine and ``TurnOff`` will lock (disable)
 * it. The lock is automatically set after the machine is used.
 *
 * The standard upgrade object has a tweqable joint that can be used
 * to indicate if the panel is active. Use the ``Tweq\Lock`` property
 * to have the joint move when the state changes.
 */
#if !SCR_GENSCRIPTS
class cScr_NewTraitMachine : public cBaseScript
{
public:
	cScr_NewTraitMachine(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	int TraitsUsed(object iPlayer);
	void SetLock(bool bLock);

protected:
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TraitMachineReusable","BaseScript",cScr_NewTraitMachine)
#endif // SCR_GENSCRIPTS

/**
 * Script: PhysARContact
 * Inherits: BaseScript
 * Messages: PhysContactCreate, PhysContactDestroy
 *
 * Stimulates objects that come in contact with this one.
 */
#if !SCR_GENSCRIPTS
class cScr_PhysARContact : public cBaseScript
{
public:
	cScr_PhysARContact(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysContactCreate(sPhysMsg*, cMultiParm&);
	virtual long OnPhysContactDestroy(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("PhysARContact","BaseScript",cScr_PhysARContact)
#endif // SCR_GENSCRIPTS

/**
 * Script: CloneContactFrob
 * Inherits: BaseScript
 * Messages: FrobInvEnd
 * Properties: Engine Features\Stack Count, Renderer\Has Refs
 *
 * Stimulates the player when it is frobbed in inventory. Most often used
 * to give health points.
 */
#if !SCR_GENSCRIPTS
class cScr_CloneContactFrob : public cBaseScript
{
public:
	cScr_CloneContactFrob(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CloneContactFrob","BaseScript",cScr_CloneContactFrob)
#endif // SCR_GENSCRIPTS


#endif // _DARKGAME == 3

#endif // SS2SCRIPTS_H
#ifdef SCR_GENSCRIPTS
#undef SS2SCRIPTS_H
#endif
