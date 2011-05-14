/******************************************************************************
 *  T1Scripts.h
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
#ifndef T1SCRIPTS_H
#define T1SCRIPTS_H

#if (_DARKGAME == 1)

#ifndef SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/scrservices.h>
#include <lg/links.h>
#include "BaseScript.h"
#include "BaseTrap.h"
#endif // SCR_GENSCRIPTS


/**
 * Script: TrapOffFilter
 * Inherits: BaseScript
 * Messages: TurnOff
 * Links: ControlDevice
 *
 * Relays only ``TurnOff`` messages.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapOffFilter : public cBaseScript
{
public:
	cScr_TrapOffFilter(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapOffFilter","BaseScript",cScr_TrapOffFilter)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapOnce
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff
 * Links: ControlDevice
 *
 * Will relay a single ``TurnOn`` or ``TurnOff`` message. Subsequent
 * messages are ignored.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapOnce : public cBaseScript
{
public:
	cScr_TrapOnce(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapOnce","BaseScript",cScr_TrapOnce)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRevert
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff, Timer
 * Links: ControlDevice
 * Properties: Script\Timing
 *
 * Relays ``TurnOn`` and ``TurnOff``. After the time set in ``Script\Timing``,
 * the opposite message will be sent. If a message is sent to the trap before it
 * has timed-out, the previous message will be ignored and the timer reset.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapRevert : public cBaseScript
{
public:
	cScr_TrapRevert(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRevert","BaseScript",cScr_TrapRevert)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapDirection
 * Inherits: BaseTrap
 * Messages: GoForward, GoReverse
 * Links: ControlDevice
 *
 * When turned on, the trap sends ``GoForward`` along ``ControlDevice`` links.
 * ``GoReverse`` is sent when it is turned off.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapDirection : public cBaseTrap
{
public:
	cScr_TrapDirection(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapDirection","BaseTrap",cScr_TrapDirection)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapPatroller
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Properties: AI\Ability Settings\Patrol: Does patrol
 *
 * Sets the ``Patrol: Does patrol`` property of objects linked with ``ControlDevice``.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapPatroller : public cBaseTrap
{
public:
	cScr_TrapPatroller(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapPatroller","BaseTrap",cScr_TrapPatroller)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapNonFinalComplete
 * Inherits: BaseTrap
 * Messages: QuestChange
 * Quest Vars: goal_state_??n??, goal_visible_??n??
 *
 * Trigger when all required objectives have been completed. Objectives are
 * ignored if they are hidden, optional, or marked as final.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapNonFinalComplete : public cBaseTrap
{
public:
	cScr_TrapNonFinalComplete(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	bool EvaluateGoals(int num_goals);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnQuestChange(sQuestMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapNonFinalComplete","BaseTrap",cScr_TrapNonFinalComplete)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapTexture
 * Inherits: BaseTrap
 * Properties: Script\TerrReplaceOn, Script\TerrReplaceOff, Engine Features\Retexture Radius
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
 * Script: TrigOBBPlayer
 * Inherits: BaseTrap
 * Messages: PhysEnter, PhysExit
 * Schemas: (Event Activate), (Event Deactivate)
 *
 * Sends ``TurnOn`` when the ``Player`` object enters the bounding-box,
 * and ``TurnOff`` when he exits. An environmental schema will be played if
 * the tags are defined for the bounding-box object.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigOBBPlayer : public cBaseTrap
{
public:
	cScr_TrigOBBPlayer(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysEnter(sPhysMsg*, cMultiParm&);
	virtual long OnPhysExit(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigOBBPlayer","BaseTrap",cScr_TrigOBBPlayer)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomPopChange
 * Inherits: BaseRoomScript
 * Messages: CreatureRoomEnter, CreatureRoomExit, PlayerRoomEnter, PlayerRoomExit
 * Links: ControlDevice
 *
 * Sends ``TurnOn`` when anyone enters a room, and ``TurnOff`` when anyone leaves.
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
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigRoomPopChange","BaseRoomScript",cScr_TrigRoomPopChange)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigRoomObject
 * Inherits: BaseRoomScript
 * Messages: ObjRoomTransit
 * Links: ControlDevice, Route
 *
 * Trigger when this object is inside a designated room. Link from the object to
 * any number of rooms using ``Route`` links. The script sends ``TurnOn`` along
 * ``ControlDevice`` links when the object enters any of the rooms. ``TurnOff``
 * is sent when it exits the room. Moving between two rooms that are both linked
 * will do nothing.
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
 * the ``Player``.
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

#if !SCR_GENSCRIPTS
/**
 * Script: BaseFindSecret
 * Properties: Dark GameSys\Stats
 * Quest Vars: DrSSecrets, DrSScrtCnt
 *
 * Manipulates the quest variables $$DrSSecrets$$ and $$DrSScrtCnt$$ for objects
 * that have the ''Hidden'' flag of ``Dark GameSys\Stats`` set. When an object
 * is found, the ''Hidden'' flag is cleared so it will no longer be considered secret.
 */
class cBaseFindSecret
{
public:
	cBaseFindSecret(int iHostObjId)
		: m_iObjId(iHostObjId)
	{ }

private:
	int m_iObjId;

protected:
	void InitHidden(void);
	void FindHidden(void);
};
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapFindSecret
 * Inherits: BaseTrap, BaseFindSecret
 *
 * Marks a secret as found when the trap is turned on.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapFindSecret : public cBaseTrap, protected cBaseFindSecret
{
public:
	cScr_TrapFindSecret(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cBaseFindSecret(iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapFindSecret","BaseTrap",cScr_TrapFindSecret)
#endif // SCR_GENSCRIPTS

/**
 * Script: FrobFind
 * Inherits: BaseScript, BaseFindSecret
 * Messages: FrobWorldEnd
 *
 * Marks a secret as found when the object is frobbed.
 */
#if !SCR_GENSCRIPTS
class cScr_FrobFind : public cBaseScript, protected cBaseFindSecret
{
public:
	cScr_FrobFind(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId),
		  cBaseFindSecret(iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FrobFind","BaseScript",cScr_FrobFind)
#endif // SCR_GENSCRIPTS

/**
 * Script: SlayFind
 * Inherits: BaseScript, BaseFindSecret
 * Messages: Slain
 *
 * Marks a secret as found when the object is destroyed. Useful with banners.
 */
#if !SCR_GENSCRIPTS
class cScr_SlayFind : public cBaseScript, protected cBaseFindSecret
{
public:
	cScr_SlayFind(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId),
		  cBaseFindSecret(iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("SlayFind","BaseScript",cScr_SlayFind)
#endif // SCR_GENSCRIPTS

/**
 * Script: HiddenDoor
 * Inherits: BaseDoorScript, BaseFindSecret
 * Messages: DoorOpen
 *
 * Marks a secret as found when the door is opened.
 */
#if !SCR_GENSCRIPTS
class cScr_HiddenDoor : public cBaseDoorScript, protected cBaseFindSecret
{
public:
	cScr_HiddenDoor(const char* pszName, int iHostObjId)
		: cBaseDoorScript(pszName, iHostObjId),
		  cBaseFindSecret(iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnDoorOpen(sDoorMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("HiddenDoor","BaseDoorScript",cScr_HiddenDoor)
#endif // SCR_GENSCRIPTS

/**
 * Script: NonAutoDoor
 * Inherits: BaseDoorScript
 * Messages: DoorOpen, DoorClose, DoorOpening, DoorClosing, DoorHalt, TurnOn, TurnOff, FrobWorldEnd, Slain, NowLocked, NowUnlocked, SynchUp, PlayerToolFrob
 * Links: ScriptParams(Double)
 * Properties: Door\Rotating, Door\Translating, Engine Features\Locked
 * Schemas: (Event StateChange, OpenState ??state??, OldOpenState ??state??), (Event Reject, Operation OpenDoor)
 *
 * A door that does not automatically open when it is unlocked. In all other respects,
 * it behaves the same way as the ``StdDoor`` script.
 */
#if !SCR_GENSCRIPTS
class cScr_NonAutoDoor : public cBaseDoorScript
{
public:
	cScr_NonAutoDoor(const char* pszName, int iHostObjId)
		: cBaseDoorScript(pszName, iHostObjId)
	{ }

private:
	eDoorState TargetState(eDoorState);
	cAnsiStr StateChangeTags(sDoorMsg::eDoorAction,sDoorMsg::eDoorAction);
	void PingDoubles(void);

protected:
	virtual long OnNowLocked(sScrMsg*, cMultiParm&);
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnDoorOpen(sDoorMsg*, cMultiParm&);
	virtual long OnDoorClose(sDoorMsg*, cMultiParm&);
	virtual long OnDoorOpening(sDoorMsg*, cMultiParm&);
	virtual long OnDoorClosing(sDoorMsg*, cMultiParm&);
	virtual long OnDoorHalt(sDoorMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NonAutoDoor","BaseDoorScript",cScr_NonAutoDoor)
#endif // SCR_GENSCRIPTS

/**
 * Script: TransformLock
 * Inherits: BaseScript
 * Messages: NowUnlocked
 * Properties: Tweq\Models
 *
 * Activates the model tweq of an object when it is unlocked. The key region mask
 * is cleared so the object can't be locked again.
 */
#if !SCR_GENSCRIPTS
class cScr_TransformLock : public cBaseScript
{
public:
	cScr_TransformLock(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TransformLock","BaseScript",cScr_TransformLock)
#endif // SCR_GENSCRIPTS

/**
 * Script: CleanUpAttack
 * Inherits: BaseAIScript
 * Messages: AIModeChange, AbortAttack
 *
 * Sends the message ``AbortAttack`` to itself when the AI is slain or knocked-out.
 * This solves a problem where particle SFX remain active if an AI is killed while in
 * the process of attacking.
 */
#if !SCR_GENSCRIPTS
class cScr_CleanUpAttack : public cBaseAIScript
{
public:
	cScr_CleanUpAttack(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnAIModeChange(sAIModeChangeMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CleanUpAttack","BaseAIScript",cScr_CleanUpAttack)
#endif // SCR_GENSCRIPTS

/**
 * Script: NoPingBack
 * Inherits: BaseAIScript
 * Messages: AIModeChange
 * Metaproperties: NoPingBack
 *
 * Add the metaproperty ``NoPingBack`` when an AI is killed or knocked-out.
 * This prevents bodies from setting-off mines and other things.
 */
#if !SCR_GENSCRIPTS
class cScr_NoPingBack : public cBaseAIScript
{
public:
	cScr_NoPingBack(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnAIModeChange(sAIModeChangeMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NoPingBack","BaseAIScript",cScr_NoPingBack)
#endif // SCR_GENSCRIPTS

/**
 * Script: NoticesPlayerBumps
 * Inherits: BaseAIScript
 * Messages: PhysCollision
 * Properties: AI\AI Core\Alertness cap
 *
 * Briefly raises an AI's alertness if it is touched by the ``Player``.
 */
#if !SCR_GENSCRIPTS
class cScr_NoticesPlayerBumps : public cBaseAIScript
{
public:
	cScr_NoticesPlayerBumps(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NoticesPlayerBumps","BaseAIScript",cScr_NoticesPlayerBumps)
#endif // SCR_GENSCRIPTS

/**
 * Script: ShutUpYerDead
 * Inherits: BaseScript
 * Messages: AIModeChange, IgnorePotion
 * Links: AIAwareness
 *
 * Resets an AI's awareness of other objects. The message ``IgnorePotion`` is sent
 * to the AI for the ``QuaffHeal`` script.
 *
 * This script should be set in a metaproperty that gets added to an AI when it is
 * knocked-out.
 */
#if !SCR_GENSCRIPTS
class cScr_ShutUpYerDead : public cBaseScript
{
public:
	cScr_ShutUpYerDead(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ShutUpYerDead","BaseScript",cScr_ShutUpYerDead)
#endif // SCR_GENSCRIPTS

/**
 * Script: SlayHaltSpeech
 * Inherits: BaseScript
 * Messages: Slain
 *
 * Stops all speech schemas by this AI when it is killed.
 */
#if !SCR_GENSCRIPTS
class cScr_SlayHaltSpeech : public cBaseScript
{
public:
	cScr_SlayHaltSpeech(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("SlayHaltSpeech","BaseScript",cScr_SlayHaltSpeech)
#endif // SCR_GENSCRIPTS

/**
 * Script: TimedPotion
 * Inherits: BaseScript
 * Messages: FrobInvEnd, StartPotion
 * Links: ScriptParams(Potion)
 * Properties: Script\Timing, Engine Features\Combine Type
 * Metaproperties: M-QuaffHeal
 * Schemas: (Event Deactivate)
 *
 * Frobbing a timed potion starts a timer for either 8.6 seconds or the time
 * in ``Script\Timing``. An invisible clone of the object is created for applying
 * the effect of the potion. For each frob the message ``StartPotion`` is sent
 * to the clone. When the timer from the last frob expires, the clone is destroyed
 * and a schema is played.
 */
#if !SCR_GENSCRIPTS
class cScr_TimedPotion : public cBaseScript
{
public:
	cScr_TimedPotion(const char* pszName, int iHostObjId);

private:
	static long HandleStartPotion(cScript*, sScrMsg*, sMultiParm*);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
	virtual long OnStartPotion(sScrMsg*, cMultiParm&);

	virtual void StartPotion(object, ulong) { }
	virtual void StopPotion(object) { }
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TimedPotion","BaseScript",cScr_TimedPotion)
#endif // SCR_GENSCRIPTS

/**
 * Script: Invisible
 * Inherits: BaseScript
 * Messages: Invisible
 * Links: AIAttack
 * Properties: AI\State\Current visibility
 * SeeAlso: InvisiPotion
 *
 * Makes the player invisible. Responds to the message ``Invisible`` by setting
 * the current visibility to 0. All ``AIAttack`` links to this object are removed.
 *
 * Put this in the metaproperty ``M-Invisible`` to make the ``InvisiPotion``
 * script work.
 */
#if !SCR_GENSCRIPTS
class cScr_Invisible : public cBaseScript
{
public:
	cScr_Invisible(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Invisible","BaseScript",cScr_Invisible)
#endif // SCR_GENSCRIPTS

/**
 * Script: InvisiPotion
 * Inherits: TimedPotion
 * Messages: Invisible, Visible
 * Properties: AI\State\Current visibility
 * Metaproperties: M-Invisible
 * SeeAlso: Invisible
 *
 * A potion for making the player invisible. When the potion is frobbed,
 * the metaproperty ``M-Invisible`` is added to the ``Player`` then
 * the message ``Invisible`` is sent to the ``Player``. The metaproperty
 * is removed when the potion stops and the message ``Visible`` is sent
 * to the ``Player``.
 */
#if !SCR_GENSCRIPTS
class cScr_InvisiPotion : public cScr_TimedPotion
{
public:
	cScr_InvisiPotion(const char* pszName, int iHostObjId)
		: cScr_TimedPotion(pszName, iHostObjId)
	{ }

protected:
	virtual void StartPotion(object iFrobber, ulong iDuration);
	virtual void StopPotion(object iFrobber);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("InvisiPotion","TimedPotion",cScr_InvisiPotion)
#endif // SCR_GENSCRIPTS

/**
 * Script: LoGravPotion
 * Inherits: TimedPotion
 * Properties: Physics\Model\Attributes
 *
 * Potion that divides the player's weight and speed in half.
 */
#if !SCR_GENSCRIPTS
class cScr_LoGravPotion : public cScr_TimedPotion
{
public:
	cScr_LoGravPotion(const char* pszName, int iHostObjId)
		: cScr_TimedPotion(pszName, iHostObjId)
	{ }

protected:
	virtual void StartPotion(object iFrobber, ulong iDuration);
	virtual void StopPotion(object iFrobber);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("LoGravPotion","TimedPotion",cScr_LoGravPotion)
#endif // SCR_GENSCRIPTS

/**
 * Script: LastMissionLoot
 * Inherits: BaseScript
 * Messages: Difficulty
 * Properties: Dark Gamesys\Loot
 * Quest Vars: total_loot
 *
 * Sets the ``Dark Gamesys\Loot`` property to the value of the $$total_loot$$
 * quest variable. Activates in response to the ``Difficulty`` message, so the
 * quest variable holds the loot value from the previous mission.
 */
#if !SCR_GENSCRIPTS
class cScr_LastMissionLoot : public cBaseScript
{
public:
	cScr_LastMissionLoot(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnDifficulty(sDiffScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("LastMissionLoot","BaseScript",cScr_LastMissionLoot)
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
 * Script: ReGravitize
 * Inherits: BaseScript
 * Messages: PhysCollision
 * Properties: Physics\Model\Attributes
 *
 * When the object collides with something else, its gravity is set to about 3.25.
 */
#if !SCR_GENSCRIPTS
class cScr_ReGravitize : public cBaseScript
{
public:
	cScr_ReGravitize(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ReGravitize","BaseScript",cScr_ReGravitize)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapSlayer
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Properties: Game\Damage Model\Hit Points
 * Stims: BashStim
 *
 * Slay linked objects by damaging them with a ``BashStim``.
 */
#if !SCR_GENSCRIPTS
class cScr_SlayTrap : public cBaseTrap
{
public:
	cScr_SlayTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapSlayer","BaseTrap",cScr_SlayTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapTerminator
 * Inherits: BaseTrap
 * Links: ControlDevice
 *
 * Simply destroys linked objects. Doesn't use ``Slain``.
 */
#if !SCR_GENSCRIPTS
class cScr_ReallyDestroy : public cBaseTrap
{
public:
	cScr_ReallyDestroy(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapTerminator","BaseTrap",cScr_ReallyDestroy)
#endif // SCR_GENSCRIPTS

/**
 * Script: NumberButton
 * Inherits: BaseScript
 * Messages: FrobWorldEnd, PhysCollision
 *
 * A button that sets the last digit in a quest variable.
 */
#if !SCR_GENSCRIPTS
class cScr_NumberButton : public cBaseScript
{
public:
	cScr_NumberButton(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	virtual void ActivateButton(object iFrobber);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NumberButton","BaseScript",cScr_NumberButton)
#endif // SCR_GENSCRIPTS


#endif // _DARKGAME == 1

#endif // T1SCRIPTS_H
#ifdef SCR_GENSCRIPTS
#undef T1SCRIPTS_H
#endif
