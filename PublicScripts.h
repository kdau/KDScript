/******************************************************************************
 *  PublicScripts.h
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
#ifndef PUBLICSCRIPTS_H
#define PUBLICSCRIPTS_H

#if !SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/scrservices.h>
#include <lg/links.h>
#include <lg/tools.h>
#include <darkhook.h>
#include "BaseScript.h"
#include "BaseTrap.h"
#include "CommonScripts.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapMetaProp
 * Inherits: BaseTrap
 * Link: ScriptParams(S??code??) - Link to metaproperty.
 * Link: ScriptParams(A@??code??) - Add to linked object. Code matches $$S$$ link.
 * Link: ScriptParams(R@??code??) - Remove from object when turned on.
 *
 * Manipulates metaproperties. When turned on, $$A$$ linked objects have a metaproperty
 * added, and $$R$$ linked objects have a metaproperty removed. The metaproperty
 * can be named with $$@??code??$$ (the code is any identifier that is convenient for you)
 * and a $$S??code??$$ link will point to the metaproperty. Or just name the metaproperty
 * directly: ''aFrobInert''. Turning off the trap removes $$A$$ from links and adds to $$R$$ links.
 */
#if !SCR_GENSCRIPTS
class cScr_MPTrap : public cBaseTrap
{
public:
	cScr_MPTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapMetaProp","BaseTrap",cScr_MPTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: ToolMP
 * Inherits: BaseScript
 * Link: ScriptParams(S??code??) - Link to metaproperty.
 * Link: ScriptParams(A@??code??) - Add metaproperty when frobbed on linked object.
 * Link: ScriptParams(R@??code??) - Remove metaproperty when frobbed.
 * Schemas: (Event Activate)
 * Parameter: effect(string) - If set to ''source'', the metaproperties are changed on the source of the frob.
 * SeeAlso: TrapMetaProp
 *
 * Manipulates metaproperties on an object when it is frobbed using the script object as a tool.
 * The $$A$$ and $$R$$ links can link to an archetype and the script will work with any object
 * of that type.
 */
#if !SCR_GENSCRIPTS
class cScr_MPTool : public cBaseScript
{
public:
	cScr_MPTool(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnFrobToolEnd(sFrobMsg* pFrobMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ToolMP","BaseScript",cScr_MPTool)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapScrMsgRelay
 * Inherits: BaseTrap
 * Link: ScriptParams(??msg??) - Send message in data to the linked object.
 * Parameter: status_index(integer) - 1 (default, 2, or 3. Sets data to 1 when turned on, 0 when off.
 * Parameter: data(string) - Extra message data. Sets first data not used by $$status_index$$.
 * Parameter: data1(string) - First extra data.
 * Parameter: data2(string) - Second extra data.
 * Parameter: data3(string) - Third extra data.
 *
 * Trap that sends an arbitrary message when turned on or off. Each ``ScriptParams``
 * link names the message to send to the destination of the link. The message is sent
 * with the first data argument set to whether the trap was turned on or off. The data
 * argument to use can be changed with $$status_index$$. If this is not 1, 2, or 3 then
 * no status is sent and the trap only responds to ``TurnOn``.
 *
 * The message data arguments not being used for status can be set with other parameters.
 * The parameter string starts with a letter to tell what type of value is being used.
 *   ''i'' - An integer.
 *   ''f'' - A real number.
 *   ''v'' - A real vector. Enter three numbers separated with commas.
 *   ''s'' - A string.
 * If the first letter doesn't match, the script will guess if the parameter is a number or a string.
 */
#if !SCR_GENSCRIPTS
class cScr_SMRelay : public cBaseTrap
{
public:
	cScr_SMRelay(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapScrMsgRelay","BaseTrap",cScr_SMRelay)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapSMTrans
 * Inherits: BaseTrap
 * Link: ScriptParams(??msg??) - Send ``TurnOn`` to the linked object when ??msg?? is received.
 *
 * Trigger when any message is received. Each object linked to with ``ScriptParams``
 * will get ``TurnOn`` when the message named in the link is received by the script.
 *
 * Only the ''Invert'' trap control flag can be used. The other flags and the lock
 * ignored by this script. The object the script is used on will likely want to use the
 * lock and the timing properties.
 */
#if !SCR_GENSCRIPTS
class cScr_SMTrans : public cBaseTrap
{
public:
	cScr_SMTrans(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply);

public:
	STDMETHOD(ReceiveMessage)(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapSMTrans","BaseTrap",cScr_SMTrans)
#endif // SCR_GENSCRIPTS

/**
 * Script: Forwarder
 * Inherits: BaseTrap
 * Link: ScriptParams(??msg??)
 * SeeAlso: TrapScrMsgRelay, TrapSMTrans
 *
 * Relays a message to objects that are linked with a ``ScriptParams`` link having
 * the name of the message. None of the trap control flags, lock, or timing have
 * any meaning to this script.
 *
 * The data for a ``ScriptParams`` links can only be 15 characters long. Many
 * messages will exceed this limit. There are two ways to work around this:
 * use an indirect name, or use a shortcut name. An indirect name uses the
 * ``Editor\Design Note`` property of an object to provide the full name.
 * The link data starts with the character $$#$$ then the name of the other
 * object. The entire ``Editor\Design Note`` property is the full name; it is
 * not treated as a parameter list.
 *
 * Message names in a ``ScriptParams`` link can also be given as shortcuts.
 *   $$.FIB$$ - FrobInvBegin
 *   $$.FIE$$ - FrobInvEnd
 *   $$.FTB$$ - FrobToolBegin
 *   $$.FTE$$ - FrobToolEnd
 *   $$.FWB$$ - FrobWorldBegin
 *   $$.FWE$$ - FrobWorldEnd
 *   $$.IDF$$ - InvDeFocus
 *   $$.IDS$$ - InvDeSelect
 *   $$.IF$$ - InvFocus
 *   $$.IS$$ - InvSelect
 *   $$.WDF$$ - WorldDeFocus
 *   $$.WDS$$ - WorldDeSelect
 *   $$.WF$$ - WorldFocus
 *   $$.WS$$ - WorldSelect
 *   $$.CRI$$ - CreatureRoomEnter
 *   $$.CRO$$ - CreatureRoomExit
 *   $$.ORI$$ - ObjectRoomEnter
 *   $$.ORO$$ - ObjectRoomExit
 *   $$.ORT$$ - ObjectRoomTransit
 *   $$.PRI$$ - PlayerRoomEnter
 *   $$.PRO$$ - PlayerRoomExit
 *   $$.RPRI$$ - RemotePlayerRoomEnter
 *   $$.RPRO$$ - RemotePlayerRoomExit
 *   $$.PC$$ - PhysCollision
 *   $$.PCC$$ - PhysContactCreate
 *   $$.PCD$$ - PhysContactDestroy
 *   $$.PI$$ - PhysEnter
 *   $$.PO$$ - PhysExit
 *   $$.PFA$$ - PhysFellAsleep
 *   $$.PMN$$ - PhysMadeNonPhysical
 *   $$.PMP$$ - PhysMadePhysical
 *   $$.PWU$$ - PhysWokeUp
 *   $$.PPA$$ - PressurePlateActivating
 *   $$.PPD$$ - PressurePlateDeactivating
 *   $$.PPU$$ - PressurePlateInactive
 *   $$.PPD$$ - PressurePlateActive
 *   $$.ME$$ - MotionEnd
 *   $$.MF$$ - MotionFlagReached
 *   $$.MS$$ - MotionStart
 *   $$.MTWP$$ - MovingTerrainWaypoint
 *   $$.WPR$$ - WaypointReached
 *   $$.DGMC$$ - DarkGameModeChange
 *   $$.MT$$ - MediumTransition
 *   $$.PSC$$ - PickStateChange
 * If the message name begins with an exclamation mark ($$!$$) then the
 * rest of the name is a stimulus and the word ''Stimulus'' will be added to
 * complete the message name.
 */
#if !SCR_GENSCRIPTS
class cScr_Forwarder : public cBaseTrap
{
public:
	cScr_Forwarder(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	virtual long OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply);

public:
	STDMETHOD(ReceiveMessage)(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Forwarder","BaseTrap",cScr_Forwarder)
#endif // SCR_GENSCRIPTS

/**
 * Script: Validator
 * Inherits: BaseTrap
 * Message: Validate - Set which link will get the next relay. First data argument is index number.
 * Links: ScriptParams(??index??)
 * Parameter: order(string) - Set to ''increment'' or ''step'' to have the trap automatically change the relay index after each trigger.
 * Parameter: increment(integer) - Value to change the current index when $$order$$ is set to ''increment''.
 * Parameter: rollover(integer) - Reset the index to 0 when incremented gets to this value or greater.
 *
 * Relay ``TurnOn`` and ``TurnOff`` to one of many destinations. ``ScriptParams`` links
 * with the data set to an integer index identify the destinations. Initially, the link index 0 is
 * triggered. The index can be changed manually by sending a ``Validate`` message. If the
 * parameter $$order$$ is set to ''increment'', the index is changed after each relay.
 * Parameters $$increment$$ and $$rollover$$ control how the index is incremented.
 * Setting the parameter $$order$$ to ''step'' is like ''increment'' except when using $$rollover$$
 * and $$increment$$ is greater than 1. When the increment passes the rollover it will continue
 * skipping values from 0. For example, 3 links with increment by 2 will go 0,2,0,2,0,2
 * when using ''increment'', and 0,2,1,0,2,1 with ''step''.
 */
#if !SCR_GENSCRIPTS
class cScr_Validator : public cBaseTrap
{
public:
	cScr_Validator(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_Validator, m_iValidateParam, iHostObjId)
	{ }

private:
	script_int m_iValidateParam;

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Validator","BaseTrap",cScr_Validator)
#endif // SCR_GENSCRIPTS

/**
 * Script: LinkTemplate
 * Inherits: BaseTrap
 * Parameter: object(object) - Object that links will be created from.
 * Parameter: flavor(string) - Type of link to move.
 * Parameter: dest(object) - Destination of the links.
 * Parameter: on_create(boolean) - Just make the new links, don't destroy the template links.
 * Parameter: off_destroy(boolean) - Only delete links when turned off.
 * Parameter: singleton(boolean) - Only allow one of the link type to be made from the object.
 *
 * Manipulates links. The $$object$$ parameter identifies the source that the links will be
 * added to or removed from. Template links are created from the script object to destinations.
 * When turned on, identical links will be made using the source object instead of the
 * script object. Turning off the trap removes the links and sets them on the script object
 * again. $$Flavor$$ and $$dest$$ can be set to restrict which links to use.
 *
 * When a link is created, the template link is removed. Links that are removed when the
 * trap is turned off have a duplicate re-made as a template. The parameter $$on_create$$
 * will not delete the template links when a new one is created. $$Off_destroy$$ deletes
 * links without creating links on the script object.
 *
 * Creating multiple links of the same flavor can be avoided by setting the $$singleton$$ parameter.
 */
#if !SCR_GENSCRIPTS
class cScr_LinkTrap : public cBaseTrap
{
public:
	cScr_LinkTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("LinkTemplate","BaseTrap",cScr_LinkTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapTeleportDelta
 * Inherits: BaseTrap
 * Messages: NetTeleport
 * Links: ControlDevice, ScriptParams(dm)
 *
 * Teleports the object linked with ``ControlDevice`` to somewhere nearby
 * this object. The $$dm$$ link identifies a reference object. The new position
 * is the same distance and direction from this object as the old position was
 * from the reference object. If the reference object is rotated differently than
 * the teleport destination, then the controlled object will be rotated to match.
 */
#if !SCR_GENSCRIPTS
class cScr_DeltaTeleport : public cBaseTrap
{
public:
	cScr_DeltaTeleport(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	void Teleport(object iObj);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
#if (_DARKGAME == 3)
	virtual long OnMessage(sScrMsg*, cMultiParm&);
#endif
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapTeleportDelta","BaseTrap",cScr_DeltaTeleport)
#endif // SCR_GENSCRIPTS

/**
 * Script: RandomRelay
 * Inherits: BaseTrap
 * Links: ControlDevice, SwitchLink, ScriptParams(??weight??)
 * Parameter: reusable(boolean) - Like the trap control flag ''Once''.
 * Parameter: preserve(boolean) - Don't remove the link that is used.
 * Parameter: weighted(boolean) - Relay along ``ScriptParams`` links where some can be more likely than others.
 * Parameter: rechargeable(boolean) - Keep links, but don't reuse a link until all others have been triggered. Requires $$weighted$$ and implies $$preserve$$.
 *
 * Randomly picks a link and relays the ``TurnOn`` or ``TurnOff`` along it.
 *
 * A weighted relay lets you use an uneven probability of choosing links. Instead of
 * ``ControlDevice`` or ``SwitchLink``, create ``ScriptParams`` links and set
 * the data of each to a number greater than zero. Links with higher numbers are
 * more likely to be chosen than lower numbers.
 */
#if !SCR_GENSCRIPTS
class cScr_RelayRandom : public cBaseTrap
{
public:
	cScr_RelayRandom(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("RandomRelay","BaseTrap",cScr_RelayRandom)
GEN_ALIAS("RandomRelay1x2","BaseTrap",cScr_RelayRandom,1x2)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigSim
 * Inherits: BaseTrap
 * Messages: Sim
 *
 * Generate a trigger when the game starts.
 * Since the script has no use after it activates, the object will be destroyed
 * to save object ID space. Objects that descend from the ``physical``
 * archetype will not be destroyed.
 */
#if !SCR_GENSCRIPTS
class cScr_SimTrigger : public cBaseTrap
{
public:
	cScr_SimTrigger(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigSim","BaseTrap",cScr_SimTrigger)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapStim
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Stims: ScriptStim
 * Parameter: stim(object) - Stimulus to use instead of ``ScriptStim``.
 * Parameter: intensity(number) - Stimulus intensity.
 *
 * Applies a stimulus to linked objects when triggered.
 * Turning off the trap sends a negative intensity.
 */
#if !SCR_GENSCRIPTS
class cScr_StimTrap : public cBaseTrap
{
public:
	cScr_StimTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv* pLS, ILinkQuery* pLQ, IScript* pScript, void* pData);

protected:
	void StimLinks(float fScale, bool bSelf = false);

	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapStim","BaseTrap",cScr_StimTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TimedStimulator
 * Inherits: TrapStim
 * Messages: Stage
 * Link: ScriptParams(??interval??) - Supported for compatibility. Use parameters instead.
 * Parameter: interval(time) - Delay between stims.
 * Parameter: stage(integer) - Initial multiplier.
 * Parameter: device(boolean) - Only start the timer in response to ``TurnOn``.
 *
 * Applies a stimulus to ``ControlDevice`` linked object many times until the trap
 * is turned off. Each successive stim will multiply the intensity (see ``TrapStim``)
 * by an increasing amount.
 *
 * When turned off, the trap will save the current stage. Turning it on again will
 * restart from that value. Send the message ``Stage`` to manually reset the current
 * value of the stimulator. The first message data is the new multiplier.
 *
 * The $$device$$ parameter controls whether the timer can be activated automatically.
 * Normally, when an object is created in Dromed, the timer will wait for a ``TurnOn``
 * message to activate. If the object is created after the game starts, then the timer
 * activates automatically. To make an emitted object wait for ``TurnOn``, you must
 * set the $$device$$ parameter to ''true''.
 */
#if !SCR_GENSCRIPTS
class cScr_StimRepeater : public cScr_StimTrap
{
public:
	cScr_StimRepeater(const char* pszName, int iHostObjId)
		: cScr_StimTrap(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_StimRepeater, m_hTimer, iHostObjId),
		  SCRIPT_VAROBJ(cScr_StimRepeater, m_iScale, iHostObjId),
		  SCRIPT_VAROBJ(cScr_StimRepeater, m_iInterval, iHostObjId)
	{ }

private:
	script_handle<tScrTimer> m_hTimer;
	script_int m_iScale;
	script_int m_iInterval;

	void GetLinkParams(int* piInterval, int* piInitial);
	void StartTimer(void);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TimedStimulator","TrapStim",cScr_StimRepeater)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigOBBSpec
 * Inherits: BaseTrap, BasePopulation
 * Messages: PhysEnter, PhysExit, Obituary, PMResize
 * Links: ScriptParams(arc), ScriptParams(cd), Population, ScriptParams(Population)
 * Metaproperties: M-NotifyRegion
 * SeeAlso: PhysModelCorrect
 *
 * Triggers when objects of a specific type enter the bounding box.
 * The $$arc$$ link identifies the type of objects that will trigger the trap.
 * Every time an object enters or exits the bounding box, the $$cd$$ links
 * will be triggered with ``TurnOn`` or ``TurnOff``. ``ControlDevice`` links
 * are turned on when the first object enters, and turned off when the last
 * object exits.
 *
 * Objects in the bounding box are tracked with ``Population`` links and the
 * metaproperty ``M-NotifyRegion``. Objects that are slain while being tracked
 * should send an ``Obituary`` message to the trap. __System Shock 2__ will use
 * ``ScriptParams`` links with the data set to ''Population''.
 */
#if !SCR_GENSCRIPTS
class cScr_OBBSpec : public cBaseTrap, protected cTrackPopulation
{
public:
	cScr_OBBSpec(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cTrackPopulation(iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnPhysEnter(sPhysMsg*, cMultiParm&);
	virtual long OnPhysExit(sPhysMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);

	void SpecTrigger(bool bTurnOn, object iSource);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigOBBSpec","BaseTrap",cScr_OBBSpec)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigBodyPickup
 * Inherits: BaseScript
 * Messages: Contained
 *
 * Turns on when the object is picked up. Turns off when dropped.
 */
#if !SCR_GENSCRIPTS
class cScr_CorpseTrigger : public cBaseScript
{
public:
	cScr_CorpseTrigger(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigBodyPickup","BaseScript",cScr_CorpseTrigger)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigDamageRTC
 * Inherits: BaseScript
 * Messages: Damage
 *
 * Triggers the object that caused damage. The first message data argument
 * is the amount of damage.
 */
#if !SCR_GENSCRIPTS
class cScr_DamageRTC : public cBaseScript
{
public:
	cScr_DamageRTC(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnDamage(sDamageScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigDamageRTC","BaseScript",cScr_DamageRTC)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapHP
 * Inherits: BaseTrap
 * Parameter: target(object) - Victim.
 * Parameter: culprit(object) - What causes the damage. (optional)
 * Parameter: damage(object) - Type of damage stimulus. (optional)
 * Parameter: hitcount(number) - How much healing or damage to cause.
 *
 * Increases the hit-points of $$target$$ when turned on. Damages the object by the same amount when turned off.
 */
#if !SCR_GENSCRIPTS
class cScr_HPTrap : public cBaseTrap
{
public:
	cScr_HPTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapHP","BaseTrap",cScr_HPTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: AIItemGiver
 * Inherits: BaseScript
 * Messages: TransferItem
 *
 * Takes an item contained by this object and gives it to another object.
 * The script can be used in a conversation or response so the AI running
 * the pseudo-script will give an item it holds.
 *
 * The first data argument of the ``TransferItem`` message is the name
 * of the object that will become the new container for the item. The second
 * argument is the name of the item. If either object can't be found, or the
 * AI isn't the current container for the item, then the pseudo-script will abort.
 *
 * This script may be unstable in __System Shock 2__.
 */
#if !SCR_GENSCRIPTS
class cScr_TransferItem : public cBaseScript
{
public:
	cScr_TransferItem(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("AIItemGiver","BaseScript",cScr_TransferItem)
#endif // SCR_GENSCRIPTS

/**
 * Script: IntrinsicText
 * Inherits: BaseTrap
 * Properties: Book\Text
 * Parameters: text(string), clr(color), clr_red(integer), clr_green(integer), clr_blue(integer), time(time)
 *
 * Displays text on-screen when turned on. If the ``Book\Text`` property is set,
 * the first page of the book file will be shown. Otherwise, the text is given in the
 * design note.
 */
#if !SCR_GENSCRIPTS
class cScr_QuickText : public cBaseTrap
{
public:
	cScr_QuickText(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	void ScanText(char* psz);
	void DisplayText(void);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("IntrinsicText","BaseTrap",cScr_QuickText)
#endif // SCR_GENSCRIPTS

/**
 * Script: IntrinsicPlaque
 * Inherits: IntrinsicText
 * Messages: FrobWorldEnd
 *
 * Displays text on-screen when frobbed.
 */
#if !SCR_GENSCRIPTS
class cScr_FrobText : public cScr_QuickText
{
public:
	cScr_FrobText(const char* pszName, int iHostObjId)
		: cScr_QuickText(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&)
	{ return 0; }
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("IntrinsicPlaque","IntrinsicText",cScr_FrobText)
#endif // SCR_GENSCRIPTS

/**
 * Script: IntrinsicCover
 * Inherits: IntrinsicText
 * Messages: WorldFocus
 *
 * Displays text on-screen when highlighted. The ``WorldAction`` of the property
 * ``Engine Features\FrobInfo`` must be set to ``FocusScript``.
 */
#if !SCR_GENSCRIPTS
class cScr_FocusText : public cScr_QuickText
{
public:
	cScr_FocusText(const char* pszName, int iHostObjId)
		: cScr_QuickText(pszName, iHostObjId)
	{ }

protected:
	virtual long OnWorldSelect(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&)
	{ return 0; }
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("IntrinsicCover","IntrinsicText",cScr_FocusText)
#endif // SCR_GENSCRIPTS

/**
 * Script: InvSchema
 * Inherits: BaseScript
 * Messages: FrobInvEnd
 * Links: SoundDescription
 * Properties: Sound\Object Sound
 *
 * Play a sound when an inventory object is frobbed. In __Thief__, use a ``SoundDescription``
 * link to the schema. In __System Shock 2__, set the ``Sound\Object Sound`` property.
 *
 * The schema will be played as an ambient sound if it is frobbed by the ``Player``.
 */
#if !SCR_GENSCRIPTS
class cScr_InvSchema : public cBaseScript
{
public:
	cScr_InvSchema(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("InvSchema","BaseScript",cScr_InvSchema)
#endif // SCR_GENSCRIPTS

/**
 * Script: Sippable
 * Inherits: BaseScript
 * Messages: FrobInvEnd, Contained
 * Properties: Inventory\Object Name, Obj\Object name
 * Parameter: sips(number) - How many times the object can be frobbed.
 *
 * The object can be frobbed a limited number of times. After all the sips have
 * been used, it is destroyed. Put the string ''%i'' somewhere in the inventory
 * name of the object and it will be replaced with the number of sips remaining.
 */
#if !SCR_GENSCRIPTS
class cScr_Sippable : public cBaseScript
{
public:
	cScr_Sippable(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void SetSipsLeft(int iSips);
	int GetSipsLeft(void);

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Sippable","BaseScript",cScr_Sippable)
#endif // SCR_GENSCRIPTS

/**
 * Script: Zapper
 * Inherits: BaseAIScript
 * Messages: EndAttack
 * Links: AITarget
 * Stims: MagicZapStim, Anti-Human
 * Schemas: ghmagic, hit_magic, dam_sting, exp_cryopsi
 * Parameters: zap_stim(object), zap_strength(number), no_zap_sound(boolean)
 *
 * Stimulates an object when it is attacked by an AI. Uses a $$MagicZapStim$$ in
 * __Thief__ or $$Anti-Human$$ in __System Shock 2__, or the stimulus in the
 * $$zap_stim$$ parameter.
 *
 * Unless $$no_zap_sound$$ is set, a sound is played where the target is. One
 * schema is used when the target is the ``Player``, and another for other
 * objects. In __Thief__ the sounds are ``ghmagic`` and ``hit_magic``.
 * In __System Shock 2__ the sounds are ``dam_sting`` and ``exp_cryopsi``.
 */
#if !SCR_GENSCRIPTS
class cScr_Zapper : public cBaseAIScript
{
public:
	cScr_Zapper(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnEndAttack(sAttackMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Zapper","BaseAIScript",cScr_Zapper)
#endif // SCR_GENSCRIPTS

/**
 * Script: PhysModelCorrect
 * Inherits: BaseScript
 * Messages: Sim, TweqComplete, PMResize
 * Properties: Physics\Model, Shape\Scale
 *
 * Automatically adjusts the physical dimensions of an object to match the
 * ``Shape\Scale`` property. Activates when the game starts, when a
 * scale tweq completes, and when the ``PMResize`` message is received.
 */
#if !SCR_GENSCRIPTS
class cScr_PhysScale : public cBaseScript
{
public:
	cScr_PhysScale(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void DoResize(void);

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("PhysModelCorrect","BaseScript",cScr_PhysScale)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapPhantom
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Properties: Renderer\Transpncy
 * Parameters: alpha_min(number), alpha_max(number), rate(number), fade_time(time), curve(integer)
 *
 * Gradually changes the ``Renderer\Transpncy`` of an object when turned on or off.
 * Turning on the trap will increase the value, turning off decreases it. When the alpha
 * reaches $$alpha_max$$ or $$alpha_min$$ the script stops and sends ``TurnOn`` or
 * ``TurnOff``. The $$fade_time$$ parameter sets the time it will take to fade from
 * $$alpha_min$$ to $$alpha_max$$. Or you can set the speed as alpha-per-second in
 * the $$rate$$ parameter.
 *
 * The $$curve$$ parameter changes the algorithm used to calculate the alpha value.
 * The default is to scale linearly to the time. Other options :
 *   1 - square
 *   2 - square root
 *   3 - logarithm
 *   4 - exponential (base 10)
 *   5 - natural logarithm
 *   6 - exponential (base e)
 */
#if !SCR_GENSCRIPTS
class cScr_AlphaTrap : public cBaseTrap
{
public:
	cScr_AlphaTrap(const char* pszName, int iHostObjId);

private:
	//script_handle<tScrTimer> m_hFadeTimer;
	script_int m_bActive;
	script_int m_iSign;
	script_int m_iStartTime;

	static const int sm_iFadeResolution;

	float m_fAlphaMin, m_fAlphaMax;
	int m_iFadeTime, m_iCurve;

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapPhantom","BaseTrap",cScr_AlphaTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapMotion
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Parameters: mot(string)
 *
 * Make an AI play a motion. Link to the AI with ``ControlDevice``.
 * The parameter is the name of the motion file (without a ''.mi'' extension),
 * not the motion schema. However, only motions that are part of a motion
 * schema will be recognized.
 */
#if !SCR_GENSCRIPTS
class cScr_MotionTrap : public cBaseTrap
{
public:
	cScr_MotionTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapMotion","BaseTrap",cScr_MotionTrap)
#endif // SCR_GENSCRIPTS

#if (_DARKGAME == 2) || (_DARKGAME == 3)
/**
 * Script: TrapCamShift
 * Inherits: BaseTrap
 *
 * Changes the player's viewpoint to this object while the trap is turned on.
 * In __System Shock 2__ it is necessary to assign a name to the object.
 * This script isn't supported in __Thief 1__.
 *
 * If the object has a physics model, then the camera view will have a lens-like
 * border. Setting the ``Physics\Model\Type`` to ''None'' (which is the same
 * as deleting the property) will fill the entire screen with the object view.
 */
#if !SCR_GENSCRIPTS
class cScr_CameraTrap : public cBaseTrap
{
public:
	cScr_CameraTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapCamShift","BaseTrap",cScr_CameraTrap)
#endif // SCR_GENSCRIPTS
#endif // (_DARKGAME == 2) || (_DARKGAME == 3)


/**
 * Script: TrigAIAwareShift
 * Inherits: BaseScript
 * Link: AIAwareness
 * Link: ScriptParams(af??code??) - Link to object that can trigger awareness.
 * Link: ScriptParams(av??code??) - Relay to these links when the AI can see the object.
 * Link: ScriptParams(aa??code??) - Relay to these links when the AI can hear the object.
 *
 * Triggers when an AI becomes aware of other objects. Create a ``ScriptParams`` link
 * to each object to watch for and set the link data to ''af'' plus a code that identifies
 * the object. When the AI becomes aware of the object it will send ``TurnOn`` to
 * ``ScriptParams`` links that have the same code. If the AI can hear the object, links
 * that begin with ''aa'' are used. If it can see the object, links that begin with ''av'' are used.
 * ``TurnOff`` will be sent when the AI loses contact with the object.
 *
 * This script requires the DarkHook library.
 */
#if !SCR_GENSCRIPTS
class cScr_TrackAwareness : public cBaseScript
{
public:
	cScr_TrackAwareness(const char* pszName, int iHostObjId);

private:
	script_int m_iPrevVis;
	script_int m_iPrevAud;

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnNotify(sDHNotifyMsg*, cMultiParm&);

private:
	static long HandleDHNotify(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigAIAwareShift","BaseScript",cScr_TrackAwareness)
#endif // SCR_GENSCRIPTS

/**
 * Script: Spy
 *
 * Developers can use this script to trace messages received by an object.
 * The information is displayed on-screen and also printed to the monolog.
 */
#if !SCR_GENSCRIPTS
class cScr_Spy : public cScript
{
public:
	cScr_Spy(const char* pszName, int iHostObjId)
		: cScript(pszName, iHostObjId)
	{ }

	STDMETHOD(ReceiveMessage)(sScrMsg* pMsg, sMultiParm* pReply, eScrTraceAction eTrace);

private:
	cAnsiStr FormatMultiParm(const sMultiParm& mp, const char* pszExtra);
	cAnsiStr FormatObject(object iObjId);
	inline const char* ContainsEvent(int event);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Spy","CustomScript",cScr_Spy)
#endif // SCR_GENSCRIPTS


/**
 * Script: TrapTimer
 * Inherits: BaseTrap
 * Properties: Script\Timing, Script\Delay Time
 *
 * Relays messages after a delay.
 */
#if !SCR_GENSCRIPTS
class cScr_Delayer : public cBaseTrap
{
public:
	cScr_Delayer(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	void DelayMessage(const char* pszMsg);

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapTimer","BaseTrap",cScr_Delayer)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapFlipFlop
 * Inherits: BaseTrap
 *
 * Alternately relays ``TurnOn`` and ``TurnOff`` when it receives ``TurnOn``.
 * The trap control flags and delay time have no meaning for this script.
 */
#if !SCR_GENSCRIPTS
class cScr_FlipFlop : public cBaseTrap
{
public:
	cScr_FlipFlop(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_FlipFlop,m_iState,iHostObjId)
	{ }

private:
	script_int m_iState;

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapFlipFlop","BaseTrap",cScr_FlipFlop)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapCinema
 * Inherits: BaseTrap
 * Parameter: movie(string)
 *
 * Plays a video from the ''Movies'' folder.
 */
#if !SCR_GENSCRIPTS
class cScr_PlayMovie : public cBaseTrap
{
public:
	cScr_PlayMovie(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapCinema","BaseTrap",cScr_PlayMovie)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapQVarText
 * Inherits: BaseTrap, BaseQVarText
 *
 * Shows text on-screen when turned on. The text will be formatted with quest
 * variable substitutions.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapQVText : public cBaseTrap, protected cQVarText
{
public:
	cScr_TrapQVText(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId), cQVarText(iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapQVarText","BaseTrap",cScr_TrapQVText)
#endif // SCR_GENSCRIPTS

/**
 * Script: QVarPlaque
 * Inherits: BaseScript, BaseQVarText
 * Messages: FrobWorldEnd
 *
 * Shows text on-screen when frobbed. The text will be formatted with quest
 * variable substitutions.
 */
#if !SCR_GENSCRIPTS
class cScr_FrobQVText : public cBaseScript, protected cQVarText
{
public:
	cScr_FrobQVText(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId), cQVarText(iHostObjId)
	{ }

protected:
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("QVarPlaque","BaseScript",cScr_FrobQVText)
#endif // SCR_GENSCRIPTS

/**
 * Script: QVarScroll
 * Inherits: BaseScript, BaseQVarText
 * Messages: FrobInvEnd
 *
 * Shows text on-screen when frobbed in inventory. The text will be formatted
 * with quest variable substitutions.
 */
#if !SCR_GENSCRIPTS
class cScr_InvQVText : public cBaseScript, protected cQVarText
{
public:
	cScr_InvQVText(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId), cQVarText(iHostObjId)
	{ }

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("QVarScroll","BaseScript",cScr_InvQVText)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapMissionQVar
 * Inherits: BaseTrap, BaseQuestVariable
 * Properties: Trap\Quest Var, Script\QB Name
 * Parameter: initqv(integer) - Set the quest variable when the mission starts.
 *
 * Trap for setting a quest variable for a single mission.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapQVarMis : public cBaseTrap, protected cQVarProcessor
{
public:
	cScr_TrapQVarMis(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);

	virtual int GetQVar(const char* pszName);
	virtual void SetQVar(const char* pszName, int iVal);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapMissionQVar","BaseTrap",cScr_TrapQVarMis)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapCampaignQVar
 * Inherits: TrapMissionQVar
 * Properties: Trap\Quest Var, Script\QB Name
 *
 * Sets a quest variable that is shared between missions.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapQVarCmp : public cScr_TrapQVarMis
{
public:
	cScr_TrapQVarCmp(const char* pszName, int iHostObjId)
		: cScr_TrapQVarMis(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);

	virtual void SetQVar(const char* pszName, int iVal);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapCampaignQVar","TrapMissionQVar",cScr_TrapQVarCmp)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigQuestVar
 * Inherits: BaseTrap, BaseQuestVariable
 * Properties: Trap\Quest Var, Script\QB Name
 *
 * Triggers when the value of a quest variable matches a condition.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigQVar : public cBaseTrap, protected cQVarProcessor
{
public:
	cScr_TrigQVar(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnQuestChange(sQuestMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigQuestVar","BaseTrap",cScr_TrigQVar)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigQVarChange
 * Inherits: BaseTrap
 * Properties: Trap\Quest Var, Script\QB Name
 *
 * Triggers when the value of a quest variable changes.
 * The property is just the name of the variable.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigQVarChange : public cBaseTrap
{
public:
	cScr_TrigQVarChange(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	char* GetQVar(void);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnQuestChange(sQuestMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigQVarChange","BaseTrap",cScr_TrigQVarChange)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapQVarFilter
 * Inherits: BaseTrap, BaseQuestVariable
 * Properties: Trap\Quest Var, Script\QB Name
 *
 * Relay that works only when the value of a quest variable matches a condition.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapQVarRelay : public cBaseTrap, protected cQVarProcessor
{
public:
	cScr_TrapQVarRelay(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapQVarFilter","BaseTrap",cScr_TrapQVarRelay)
#endif // SCR_GENSCRIPTS

/**
 * Script: BasePostRead
 * Inherits: BaseTrap, BaseQuestVariable
 * Links: SoundDescription
 * Properties: Book\Text, Book\Art, Script\Use Msg, Trap\Quest Var, Script\QB Name, Script\Timing, Script\Delay Time, Sound\Object Sound
 * SeeAlso: PostReadPlaque, PostReadScroll
 *
 * Display text then, after it has been displayed, trigger and perform some action.
 * When there is a ``SoundDescription`` link or ``Sound\Object Sound`` property,
 * it will play the schema as a voice-over. If there is a ``Trap\Quest Var`` or
 * ``Script\QB Name`` property, then the variable is set. The quest variable will be changed
 * after the schema is played, but the script needs to know how long to wait for the sound
 * to finish playing. Setting a quest variable can cause another sound to play, which would
 * interfere with the voice-over. Set the time to wait in the ``Script\Timing`` or
 * ``Script\Delay Time`` property.
 */
#if !SCR_GENSCRIPTS
class cScr_PostReader : public cBaseTrap, protected cQVarProcessor
{
public:
	cScr_PostReader(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	bool DoVoiceOver(object iFrobber);
	bool DoQVar(void);

protected:
	void Read(object iFrobber);

	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BasePostRead","BaseTrap",cScr_PostReader)
#endif // SCR_GENSCRIPTS

/**
 * Script: PostReadPlaque
 * Inherits: BasePostRead
 * Messages: FrobWorldEnd
 *
 * Displays text when frobbed, then triggers after it is shown.
 */
#if !SCR_GENSCRIPTS
class cScr_FrobPostRead : public cScr_PostReader
{
public:
	cScr_FrobPostRead(const char* pszName, int iHostObjId)
		: cScr_PostReader(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("PostReadPlaque","BasePostRead",cScr_FrobPostRead)
GEN_ALIAS("PostReadBook","BasePostRead",cScr_FrobPostRead,Book)
#endif // SCR_GENSCRIPTS

/**
 * Script: PostReadScroll
 * Inherits: BasePostRead
 * Messages: FrobInvEnd
 *
 * Displays text when frobbed in inventory, then triggers after it is shown.
 */
#if !SCR_GENSCRIPTS
class cScr_InvPostRead : public cScr_PostReader
{
public:
	cScr_InvPostRead(const char* pszName, int iHostObjId)
		: cScr_PostReader(pszName, iHostObjId)
	{ }

protected:
	virtual long OnFrobInvEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("PostReadScroll","BasePostRead",cScr_InvPostRead)
#endif // SCR_GENSCRIPTS


/**
 * Script: TrigWaypoint
 * Inherits: BaseMovingTerrainScript
 * Messages: WaypointReached
 *
 * Use on a moving terrain waypoint. Sends ``TurnOn`` when the moving terrain
 * reaches this object.
 */
#if !SCR_GENSCRIPTS
class cScr_TrigTerr : public cBaseMovingTerrainScript
{
public:
	cScr_TrigTerr(const char* pszName, int iHostObjId)
		: cBaseMovingTerrainScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnWaypointReached(sWaypointMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigWaypoint","BaseMovingTerrainScript",cScr_TrigTerr)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapDestroyDoor
 * Inherits: BaseTrap
 * Links: ControlDevice, SwitchLink
 * Properties: Door\Rotating, Door\Translating
 *
 * Destroy trap that first opens doors so they don't leave behind a black rectangle.
 */
#if !SCR_GENSCRIPTS
class cScr_DoorKillerTrap : public cBaseTrap
{
public:
	cScr_DoorKillerTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapDestroyDoor","BaseTrap",cScr_DoorKillerTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigOBBCreature
 * Inherits: TrigOBBSpec
 * Messages: PhysEnter, PhysExit, Obituary
 * Links: Population, ScriptParams(Population)
 * Metaproperties: M-NotifyRegion
 * Objects: Creature, Monsters
 *
 * Bounds trigger that reacts to objects that are a ``Creature`` or ``Monsters``.
 */
#if !SCR_GENSCRIPTS
class cScr_OBBCret : public cScr_OBBSpec
{
public:
	cScr_OBBCret(const char* pszName, int iHostObjId)
		: cScr_OBBSpec(pszName, iHostObjId)
	{ }

protected:
	virtual long OnPhysEnter(sPhysMsg*, cMultiParm&);
	virtual long OnPhysExit(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigOBBCreature","TrigOBBSpec",cScr_OBBCret)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapFader
 * Inherits: BaseTrap
 * Messages: NetFade
 * Parameter: fade_time(time) - Speed of the fade.
 * Parameter: fade_color(color) - Screen color. __System Shock 2__ only.
 *
 * Turns the screen black when turned on. Turning off the trap will restore the
 * screen. In __Thief 1__, the screen jumps back immediately, __Thief 2__ will
 * gradually fade in, but has the side-effect of clearing the currently selected
 * item and weapon. __System Shock 2__ doesn't have side-effects and also
 * can fade to any color.
 *
 * Fading is disabled when the configuration variable $$no_endgame$$ is set.
 * This is a commonly enabled option in Dromed for testing mission objectives.
 * But it's easy to forget and you may wonder why the script isn't working.
 */
#if !SCR_GENSCRIPTS
class cScr_FadeTrap : public cBaseTrap
{
public:
	cScr_FadeTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
#if (_DARKGAME == 3)
	virtual long OnMessage(sScrMsg*, cMultiParm&);
#endif
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapFader","BaseTrap",cScr_FadeTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: Transmogrify
 * Inherits: BaseScript
 * Messages: Contained, NetTransmogrify
 * Links: Transmute, Mutate
 * Properties: Engine Features\Stack Count
 *
 * When the object is picked-up by the ``Player``, create a different object
 * and add that to the inventory instead. The ``Engine Features\Stack Count``
 * is copied to the new item. The old object is destroyed.
 *
 * Link to the archetype of the new object using ``Transmute``. In __System Shock 2__
 * use ``Mutate`` instead. The script will search its parents for the link,
 * so you can create many mutatable objects from a single archetype or metaproperty.
 */
#if !SCR_GENSCRIPTS
class cScr_Transmogrify : public cBaseScript
{
public:
	cScr_Transmogrify(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void DoContain(object iPlayer);

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
//#if (_DARKGAME == 3)
	virtual long OnMessage(sScrMsg*, cMultiParm&);
//#endif
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Transmogrify","BaseScript",cScr_Transmogrify)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRequireOne
 * Inherits: BaseTrap, BaseRequirement
 * Links: ScriptParams(Require)
 *
 * Counts how many different devices have sent ``TurnOn`` to the trap
 * and triggers when it is an odd number. This is the traditional three-way
 * switch, but can work with more than just two levers. When any one of
 * the levers is frobbed the trap will change from off to on or vice-versa.
 */
#if !SCR_GENSCRIPTS
class cScr_RequireOneTrap : public cBaseTrap, protected cRequirement
{
public:
	cScr_RequireOneTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cRequirement(iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRequireOne","BaseTrap",cScr_RequireOneTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapRequireOneOnly
 * Inherits: BaseTrap, BaseRequirement
 * Links: ScriptParams(Require)
 *
 * Triggers when only one device has sent ``TurnOn`` to the trap.
 */
#if !SCR_GENSCRIPTS
class cScr_RequireSingleTrap : public cBaseTrap, protected cRequirement
{
public:
	cScr_RequireSingleTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  cRequirement(iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapRequireOneOnly","BaseTrap",cScr_RequireSingleTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapSquelch
 * Inherits: BaseTrap
 * Properties: Script\Timing
 *
 * Ignores rapidly repeating messages. If a trigger is sending more than one
 * ``TurnOn`` or ``TurnOff`` at a time, this script will relay just the first
 * message and stop others from going through for a period of time.
 */
#if !SCR_GENSCRIPTS
class cScr_Squelch : public cBaseTrap
{
public:
	cScr_Squelch(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapSquelch","BaseTrap",cScr_Squelch)
#endif // SCR_GENSCRIPTS

/**
 * Script: PropertyScript
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Parameter: property(string) - Name of the property to modify.
 * Parameter: fields(string) - List of property fields.
 * Parameter: onvalue(string) - Value or list of values to set when turned on.
 * Parameter: offvalue(string) - Value or list of values to set when turned off.
 *
 * Modifies the value of a property on linked objects. The property name is the
 * short name of the property as listed by ``list_props``. Simple properties do
 * not need a $$fields$$ parameter and the value parameter is a single string.
 * If the property has multiple fields, then the value parameter is a list of
 * comma-separated values. With the $$fields$$ parameter, only the named fields
 * are modified and the value parameter is a list that corresponds to those fields.
 * Fields that don't have a listed value are unchanged. If either $$onvalue$$ or
 * $$offvalue$$ is omitted, then the property is not changed for that switch.
 *
 * Property fields are named with an abbreviated format. Take the original name,
 * as displayed in the Dromed object editor, and ignore anything that isn't a letter
 * or number. Match the first field that starts with the name given in the $$fields$$
 * parameter. The parameter name can end with a number enclosed in square braces.
 * This is the subscript and will match the n-th field that starts with the name.
 * So ''A[1]'' will match the first field that has the first letter ''A'', and
 * ''A[2]'' will match the second field that starts with ''A''.
 *
 * Values are either a string, number, boolean, or vector. Multiple values are given as
 * a list of values separated by commas. Any value in the list can be enclosed in
 * parentheses. A pair of parentheses with nothing in-between is a null value and indicates
 * that field should not be changed. An empty value not in parentheses is a blank string.
 * A vector is three number separated by commas. Because commas are used between values,
 * a vector in a list must be enclosed in parentheses.
 */
#if !SCR_GENSCRIPTS
class cScr_PropertyTrap : public cBaseTrap
{
public:
	cScr_PropertyTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	bool ParseProperty(int iObjId, IProperty* pProp, const char* pszTypeName,
					char* pszValues, char const* const* pszFields);
	bool ParsePropertyFields(IStructDescTools* pSD, const sStructDesc* pSDesc, void* pData,
					char* pszValues, char const* const* pszFields);
	bool ParseStringProperty(int iObjId, IProperty* pProp, char* pszValue);
	static int LinkIter(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData);
	char const* const* GetFieldNames(char* names);
	int MatchFieldName(const char* name, const sFieldDesc* fields, int num_fields);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("PropertyScript","BaseTrap",cScr_PropertyTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapProperty
 * Inherits: BaseTrap
 * Links: ScriptParams(??property??)
 *
 * Change the properties on linked objects. Sets the property when turned on,
 * or deletes it when turned off. The link data is the short name of the
 * property to set or unset. If the data begins with $$@$$ then the data is
 * the name of an object and all properties from that object will be copied to
 * the linked object when turned on.
 */
#if !SCR_GENSCRIPTS
class cScr_SimplePropertyTrap : public cBaseTrap
{
public:
	cScr_SimplePropertyTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIterOn(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData);
	static int LinkIterOff(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapProperty","BaseTrap",cScr_SimplePropertyTrap)
#endif // SCR_GENSCRIPTS


#endif // PUBLICSCRIPTS_H
