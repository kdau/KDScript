/******************************************************************************
 *  T2Scripts.h
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
#ifndef T2SCRIPTS_H
#define T2SCRIPTS_H

#if (_DARKGAME == 2)

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
 * Script: Burplauncher
 * Inherits: BaseScript
 * Links: Firer
 * Messages: Slain
 *
 * Destroys emitted objects when this one is slain.
 *
 * This script is used for burricks' burp projectile. Each projectile continually
 * emits a cloud of gas. When the projectile is slain, the entire gas cloud
 * should be destroyed with it.
 */
#if !SCR_GENSCRIPTS
class cScr_SlayFirer : public cBaseScript
{
public:
	cScr_SlayFirer(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Burplauncher","BaseScript",cScr_SlayFirer)
#endif // SCR_GENSCRIPTS

/**
 * Script: CollapseFloor
 * Inherits: BasePPlateScript
 * Messages: PressurePlateActive
 * Links: Corpse, Flinderize
 *
 * A collapsing floor is a pressure plate that will disappear when depressed.
 * Use a ``Corpse`` or ``Flinderize`` links to have a solid object be created
 * in its place that will fall to the ground.
 */
#if !SCR_GENSCRIPTS
class cScr_CollapseFloor : public cBasePPlateScript
{
public:
	cScr_CollapseFloor(const char* pszName, int iHostObjId)
		: cBasePPlateScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnPressurePlateActive(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*,cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CollapseFloor","BaseScript",cScr_CollapseFloor)
#endif // SCR_GENSCRIPTS

/**
 * Script: Elemental
 * Inherits: BaseAIScript
 * Messages: Damage, Slain, Die
 * Links: ParticleAttachement
 * Properties: Shape\Scale, Tweq\Scale, Game\Damage Model\Hit Points
 * Parameters: curve(integer) - Adjust the rate of change relative to health. See ``TrapPhantom``.
 *
 * An elemental AI grows and shrinks based on its health. When it is ``Slain``,
 * a ``Tweq\Scale`` is activated so it shrinks before being destroyed.
 *
 * To get the complete __Thief 1__ behavior, use ``SlayHaltSpeech`` as well.
 */
#if !SCR_GENSCRIPTS
class cScr_Elemental : public cBaseAIScript
{
public:
	cScr_Elemental(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	void UpdateScale();
	void NotifyParticles(const char* pszMsg);

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnDamage(sDamageScrMsg*, cMultiParm&);
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Elemental","BaseAIScript",cScr_Elemental)
#endif // SCR_GENSCRIPTS

/**
 * Script: FireElement
 * Inherits: Elemental
 * Messages: Damage, Alertness, Mood0, Mood1, Mood2, Mood3
 * Properties: Renderer\Dynamic Light, Renderer\Extra Light, Renderer\Transparency
 * Metaproperties: BigHeatSource
 *
 * Changes color and transparency based on alertness level. A ''Mood'' message
 * is sent to ``ParticleAttachement`` links that matches the level. On high alert,
 * the metaproperty ``BigHeatSource`` is added to the object.
 */
#if !SCR_GENSCRIPTS
class cScr_FireElement : public cScr_Elemental
{
public:
	cScr_FireElement(const char* pszName, int iHostObjId)
		: cScr_Elemental(pszName, iHostObjId)
	{ }

protected:
	virtual long OnAlertness(sAIAlertnessMsg*, cMultiParm&);
	virtual long OnDamage(sDamageScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FireElement","Elemental",cScr_FireElement)
#endif // SCR_GENSCRIPTS

/**
 * Script: FireElemSparx
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff, Mood0, Mood1, Mood2, Mood3, Die
 *
 * Controls particle SFX. The ''Mood'' messages change the color of
 * the particles. The object is destroyed when ``Die`` is received.
 */
#if !SCR_GENSCRIPTS
class cScr_FireSparx : public cBaseScript
{
public:
	cScr_FireSparx(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void SetColor(int iColor);

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FireElemSparx","BaseScript",cScr_FireSparx)
#endif // SCR_GENSCRIPTS

/**
 * Script: HotPlate
 * Inherits: BaseScript
 * Messages: PhysContactCreate, PhysContactDestroy, HotPlateHeat
 * Properties: Renderer\Extra Light, Script\Timing
 * Metaproperties: HotPlateHeat, HotPlateCool
 * Stims: WaterStim
 * SeeAlso: HotPlateControl
 *
 * Initiate stimulus contact with object that touch this one.
 * When it gets the message ``HotPlateHeat``, the ``Renderer\Extra Light``
 * property will be set according to the first message argument. If it is
 * greater than 0, the metaproperty ``HotPlateHeat`` is added.
 *
 * If the object is hit with a ``WaterStim``, it will force the object to be
 * turned off for a period of time. You should create a receptron for
 * ``WaterStim`` set to the appropriate sensitivity and the reaction
 * ''Send to Scripts''. To turn off the object, a clone of the controller is
 * created with the tweq configured to send diminishing ``HotPlateHeat``
 * messages. Messages that would increase the value of ``Renderer\Extra Light``
 * are ignored. The object will be disabled until it receives a ``TurnOff``
 * message followed by a ``TurnOn``. If you set the ``Script\Timing``
 * property, it will count that many ``TurnOff`` messages before activating
 * again.
 */
#if !SCR_GENSCRIPTS
class cScr_HotPlate : public cBaseScript
{
public:
	cScr_HotPlate(const char* pszName, int iHostObjId);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysContactCreate(sPhysMsg*, cMultiParm&);
	virtual long OnPhysContactDestroy(sPhysMsg*, cMultiParm&);
	virtual long OnHotPlateHeat(sScrMsg*, cMultiParm&);
	virtual long OnWaterStimStimulus(sStimMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);

private:
	static long HandleHotPlateHeat(cScript*, sScrMsg*, sMultiParm*);
	static long HandleWaterStimStimulus(cScript*, sScrMsg*, sMultiParm*);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("HotPlate","BaseScript",cScr_HotPlate)
#endif // SCR_GENSCRIPTS

/**
 * Script: HotPlateControl
 * Inherits: BaseScript
 * Messages: TweqComplete, HotPlateHeat, Heat?
 * Links: ControlDevice
 * Properties: Tweq\Flicker, Tweq\Joints, Shape\Joint Positions
 * SeeAlso: HotPlate
 *
 * A flicker tweq periodically checks the position of the first joint on this object.
 * Each linked object will get the message ``HotPlateHeat`` with the joint position
 * in the first message argument. When the direction of the joint changes from
 * reverse to forward or forward to reverse then the message ``TurnOn`` or
 * ``TurnOff`` will be sent.
 */
#if !SCR_GENSCRIPTS
class cScr_HotPlateCtrl : public cBaseScript
{
public:
	cScr_HotPlateCtrl(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("HotPlateControl","BaseScript",cScr_HotPlateCtrl)
#endif // SCR_GENSCRIPTS

/**
 * Script: ModelByCount
 * Inherits: BaseScript
 * Messages: Contained, Create
 * Properties: Engine Features\Stack Count, Tweq\Models
 *
 * Changes the shape of the object based on the stack count. This
 * makes a combining object look different when more pieces are
 * picked up. But it does not change the shape when it is dropped
 * or used. So it should only be used for objects that cannot be dropped
 * or frobbed in inventory.
 */
#if !SCR_GENSCRIPTS
class cScr_ModelByCount : public cBaseScript
{
public:
	cScr_ModelByCount(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId),
		  bUpdating(false)
	{ }

private:
	void UpdateModel(void);

	bool bUpdating;

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
	//virtual long OnCombine(sCombineScrMsg*, cMultiParm&);
	virtual long OnCreate(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ModelByCount","BaseScript",cScr_ModelByCount)
#endif // SCR_GENSCRIPTS

/**
 * Script: SecureDoor
 * Inherits: BaseDoorScript
 * Messages: DoorOpening, DoorClose
 * Links: AIWatchObj
 * Objects: Human
 * Parameter: watcher(object) - Archetype of objects that will be linked to the door. Default is ``Human``.
 *
 * Creates ``AIWatchObj`` links when a door is opened or closed. If the door
 * starts closed the links are added when the door is opened, and vice-versa.
 */
#if !SCR_GENSCRIPTS
class cScr_SecureDoor : public cBaseDoorScript
{
public:
	cScr_SecureDoor(const char* pszName, int iHostObjId)
		: cBaseDoorScript(pszName, iHostObjId)
	{ }

private:
	eDoorState GetDoorState(void);
	cAnsiStr GetWatchers(void);

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnDoorOpening(sDoorMsg*, cMultiParm&);
	virtual long OnDoorClose(sDoorMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("SecureDoor","BaseDoorScript",cScr_SecureDoor)
#endif // SCR_GENSCRIPTS

/**
 * Script: StickyVines
 * Inherits: BaseScript
 * Messages: PhysCollision
 * Objects: JunkEarthWebs
 *
 * When the object collides with the ``Player``, a ``JunkEarthWebs`` object
 * is created and added to his inventory.
 */
#if !SCR_GENSCRIPTS
class cScr_VineShot : public cBaseScript
{
public:
	cScr_VineShot(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("StickyVines","BaseScript",cScr_VineShot)
#endif // SCR_GENSCRIPTS

/**
 * Script: JunkVines
 * Inherits: BaseScript
 * Messages: Contained
 *
 * Initiates stimulus contact when it is in the ``Player``'s inventory.
 */
#if !SCR_GENSCRIPTS
class cScr_JunkVines : public cBaseScript
{
public:
	cScr_JunkVines(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("JunkVines","BaseScript",cScr_JunkVines)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapCreate
 * Inherits: BaseTrap
 * Links: Contains
 *
 * When turned on, contained objects are teleported here then the
 * links are destroyed.
 */
#if !SCR_GENSCRIPTS
class cScr_CreateTrap : public cBaseTrap
{
public:
	cScr_CreateTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	static int LinkIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapCreate","BaseTrap",cScr_CreateTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: WatchMe
 * Inherits: BaseScript
 * Messages: BeginScript
 * Links: AIWatchObj
 * Parameter: watcher(object) - Archetype of objects that will be linked to this one. Default is ``Human``.
 *
 * Creates ``AIWatchObj`` links when the object is created.
 */
#if !SCR_GENSCRIPTS
class cScr_WatchMe : public cBaseScript
{
public:
	cScr_WatchMe(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("WatchMe","BaseScript",cScr_WatchMe)
#endif // SCR_GENSCRIPTS

/**
 * Script: WindowShade
 * Inherits: BaseTrap
 * Messages: Toggle, Slain
 * Links: ControlDevice, ParticleAttachement
 * Properties: Renderer\Anim Light, Script\TerrReplaceOn, Script\TerrReplaceOff, Script\TerrReplaceDestroy
 *
 * A breakable animated light for windows. Textures on nearby terrain are changed to
 * match the light state. When the light is slain, a broken texture will permanently
 * replace the on and off textures. Linked objects and SFX are also triggered when
 * the light changes.
 */
#if !SCR_GENSCRIPTS
class cScr_WindowShade : public cBaseTrap
{
public:
	cScr_WindowShade(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_WindowShade,m_iOnMode,iHostObjId),
		  SCRIPT_VAROBJ(cScr_WindowShade,m_iOffMode,iHostObjId),
		  SCRIPT_VAROBJ(cScr_WindowShade,m_bBroken,iHostObjId)
	{ }

private:
	script_int m_iOnMode;
	script_int m_iOffMode;
	script_int m_bBroken;

protected:
	void InitModes(void);
	void TurnLightOn(void);
	void TurnLightOff(void);
	bool IsLightOn(void);
	void Trigger(bool bTurnOn);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("WindowShade","BaseTrap",cScr_WindowShade)
#endif // SCR_GENSCRIPTS

/**
 * Script: ControlWindowShade
 * Inherits: WindowShade
 * Properties: Script\Timing
 * Parameters: auto_time_max(time), auto_time_min(time)
 *
 * Window light that automatically toggles at random intervals. Periodically,
 * the script turns the light on or off. The ``Script\Timing`` property is
 * the probability that the light will be turned on. The time period until
 * the next change is randomly chosen from 5 to 15 seconds. The range of
 * time can be configured with the $$auto_time_max$$ and $$auto_time_min$$
 * parameters.
 */
#if !SCR_GENSCRIPTS
class cScr_WindowShadeRandom : public cScr_WindowShade
{
public:
	cScr_WindowShadeRandom(const char* pszName, int iHostObjId)
		: cScr_WindowShade(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_WindowShadeRandom,m_hTimer,iHostObjId)
	{ }

private:
	script_handle<tScrTimer> m_hTimer;

protected:
	void SetAutoTimer(bool);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ControlWindowShade","WindowShade",cScr_WindowShadeRandom)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapSlayer
 * Inherits: BaseTrap
 * Links: ControlDevice
 * Properties: Game\Damage Model\Hit Points
 * Stims: BashStim
 * Parameter: damage(object) - Type of damage stimulus.
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
 * Don't use this for doors or pressure plates that have the ''Blocks vision''
 * flag set.
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
 * Script: CorpseFrobFixed
 * Inherits: BaseAIScript
 * Messages: AIModeChange, Slain, FrobWorldEnd
 * Metaproperties: FrobInert
 *
 * Makes AI frobbable when they are disabled. This script corrects a bug
 * in ``CorpseFrobHack`` when searching a corpse. Objects contained
 * by the AI that can't be picked up will be ignored.
 */
#if !SCR_GENSCRIPTS
class cScr_NewCorpseFrob : public cBaseAIScript
{
public:
	cScr_NewCorpseFrob(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnAIModeChange(sAIModeChangeMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CorpseFrobFixed","BaseAIScript",cScr_NewCorpseFrob)
#endif // SCR_GENSCRIPTS

/**
 * Script: FireShadowEcology
 * Inherits: BaseScript
 * Messages: TweqComplete
 * Links: ControlDevice, Firer
 * Properties: Tweq\Flicker
 * Schemas: (Event Activate)
 *
 * Respawn a creature after it dies. A ``Tweq\Flicker`` fires to test if the
 * creature needs to be spawned. The tweq flags can be set to only fire when
 * the player is not looking at the spawn point. That way the creature will
 * only appear offscreen. The creature archetype to spawn is linked to with
 * ``ControlDevice``. When spawned, a ``Firer`` link is set from the creature
 * to the spawn point. A new creature is spawned only when there is no ``Firer``
 * link to this object. Sends ``TurnOn`` when a new creature is created.
 */
#if !SCR_GENSCRIPTS
class cScr_SpawnEcology : public cBaseScript
{
public:
	cScr_SpawnEcology(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void MakeFirer(object iSpawnObj);
	void AttemptSpawn(void);

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FireShadowEcology","BaseScript",cScr_SpawnEcology)
#endif // SCR_GENSCRIPTS

/**
 * Script: FireShadowFlee
 * Inherits: BaseScript
 * Messages: Slain
 * Links: CorpsePart
 * Properties: Creature\TimeWarp
 * Metaproperties: M-FireShadowFlee
 *
 * A creature that flees when killed. On ``Slain``, the creature drops its
 * ``CorpsePart`` linked objects and gets the ``M-FireShadowFlee`` metaproperty.
 * The creature speed is accelerated until it disappears from the player's
 * view, or after 15 seconds. Then it is destroyed.
 */
#if !SCR_GENSCRIPTS
class cScr_CorpseFlee : public cBaseScript
{
public:
	cScr_CorpseFlee(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FireShadowFlee","BaseScript",cScr_CorpseFlee)
#endif // SCR_GENSCRIPTS


/*** miss03 ***/

/**
 * Script: ReallyLocked
 * Inherits: BaseScript
 * Messages: NowLocked, NowUnlocked
 * Metaproperties: FrobInert
 *
 * Makes an object unfrobbable while it is locked.
 */
#if !SCR_GENSCRIPTS
class cScr_LockFrobInert : public cBaseScript
{
public:
	cScr_LockFrobInert(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnNowLocked(sScrMsg*, cMultiParm&);
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ReallyLocked","BaseScript",cScr_LockFrobInert)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryBase
 * Inherits: BaseScript
 * Messages: TweqComplete, TurnOn, TurnOff, SynchUp, Target?, Report
 * Links: ControlDevice, FrobProxy, Owns
 * Properties: Tweq\Joints, Tweq\JointState
 * Schemas: (Event StateChange, DirectionState ??direction??)
 * SeeAlso: FactoryLever, FactoryMold, FactoryCauldron, FactoryGauge, FactoryWorker
 *
 * A factory is a group of levers, molds, cauldrons, and gauges that
 * can be used to manufacture new objects. This script controls the
 * joint tweqs on the objects. If the object has a ``FrobProxy`` link
 * then the joint tweq on the proxy object is kept in synch with this one.
 *
 * Most factory objects will send ``TurnOn`` to ``ControlDevice`` links
 * when the tweq moves forward, and ``TurnOff`` when moving backward.
 * After an action completes the ``Report`` is sent to objects that link to
 * this one with ``Owns``. The ``FactoryWorker`` script uses this to
 * automate actions.
 */
#if !SCR_GENSCRIPTS
class cScr_Factory : public cBaseScript
{
public:
	cScr_Factory(const char* pszName, int iHostObjId);

protected:
	// Compatible with StdLever TargState, I hope?
	enum eLeverDirection
	{
		kLeverDirectionBackward = 0,
		kLeverDirectionForward = 1
	};

	virtual void BeginGoForward(void)
	{ }
	virtual void EndGoForward(void)
	{ }
	virtual void BeginGoBackward(void)
	{ }
	virtual void EndGoBackward(void)
	{ }

	virtual eLeverDirection NextDirection(void);
	virtual void DoReport(void);
	virtual char const * ReportType(void);

	bool IsLocked(void);
	void DoTrigger(bool bTurnOn);
	void DoTrigger(bool bTurnOn, bool bReverse, const char* pszParam);
	void DoTrigger(const char* pszMsg, bool bReverse, const char* pszParam);
	void DoGoForward(void);
	void DoGoBackward(void);
	void DoSetState(eLeverDirection dir);

private:
	static long HandleReport(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleSynchUp(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleTarget(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);

	static int TriggerIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnReport(sScrMsg*, cMultiParm&);
	virtual long OnSynchUp(sScrMsg*, cMultiParm&);
	virtual long OnTarget(sScrMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryBase","BaseScript",cScr_Factory)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryLever
 * Inherits: FactoryBase
 * Alias: FactoryCauldronLever, FactoryMoldLever
 * Messages: FrobWorldEnd
 * Links: ScriptParams(LockMe), ScriptParams(ErrorOutput), Lock
 * SeeAlso: ReallyLocked
 *
 * Levers in a factory can be frobbed. Using a lever sends ``TurnOn`` to $$LockMe$$
 * links. Trying to frob a locked lever will send ``TurnOn`` to $$ErrorOutput$$ links.
 *
 * === How to Use ===
 * One lever should control the cauldron, and another controls the mold. Each
 * lever has a ``FnordLock`` associated with it. The cauldron lever links to the
 * lock for the mold lever with a $$LockMe$$ link. The mold lever locks the
 * cauldron lever's lock. The mold also has a $$LockMe$$ link to the cauldron
 * lever's lock. Both levers link to an ``alarmlite`` with $$ErrorOutput$$.
 */
#if !SCR_GENSCRIPTS
class cScr_FactoryLever : public cScr_Factory
{
public:
	cScr_FactoryLever(const char* pszName, int iHostObjId)
		: cScr_Factory(pszName, iHostObjId)
	{ }

protected:
	virtual void BeginGoForward(void);
	virtual void EndGoForward(void);
	virtual void EndGoBackward(void);
	virtual char const * ReportType(void);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryLever","FactoryBase",cScr_FactoryLever)
GEN_ALIAS("FactoryCauldronLever","FactoryBase",cScr_FactoryLever,Cauldron)
GEN_ALIAS("FactoryMoldLever","FactoryBase",cScr_FactoryLever,Mold)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryMold
 * Inherits: FactoryLever
 * Messages: FrobToolEnd, Contained, SocketMe, UnsocketMe, Full, Full?
 * Links: ScriptParams(Mold), Owns, Contains
 * Properties: Physics\Model\Attributes
 * SeeAlso: MoldSocket
 *
 * A mold can be moved onto a socket where it will create an object. Using the
 * mold as a tool will send ``SocketMe`` to the socket. ``UnsocketMe`` is
 * sent when the mold is picked up.
 *
 * On a socket the mold will activate $$LockMe$$ links when it is open. The
 * mold needs to have been filled by receiving the message ``Full``. A new object
 * will be created and teleported to the $$COG Offset$$ of the mold. The object
 * to create is found from a ``Contains`` link that can be on an archetype of
 * the mold.
 *
 * === How to Use ===
 * The mold is controlled by a mold lever. Link to the archetype of a product
 * with ``Contains``. Set the $$COG Offset$$ field of ``Physics\Model\Attributes``
 * to where the product should appear. If it is a movable mold, the ``Engine Features\FrobInfo``
 * property should have a $$WorldAction$$ of ''Move,Script'' and $$InvAction$$ of ''Script''.
 * An unmovable mold has only a $$WorldAction$$ of ''Script''.
 *
 * When the mold is in-place, it is controlled by the mold lever. A ``Lock`` link goes
 * to the ``FnordLock`` that also locks the mold lever. There is a $$LockMe$$ link
 * to the cauldron lock. The mold can link with ``ControlDevice`` to a ``SteamPuffSpout``.
 * If there is a socket, the mold links to it with ``Owns``.
 */
#if !SCR_GENSCRIPTS
class cScr_FactoryMold : public cScr_FactoryLever
{
public:
	cScr_FactoryMold(const char* pszName, int iHostObjId)
		: cScr_FactoryLever(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_FactoryGauge,m_iState,iHostObjId)
	{ }

protected:
	virtual void BeginGoForward(void);
	virtual void EndGoForward(void);
	virtual void BeginGoBackward(void);
	virtual void EndGoBackward(void);
	virtual char const * ReportType(void);

private:
	script_int m_iState;

	static int ProductOwnerIter(ILinkSrv*, ILinkQuery*, IScript*, void*);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnFrobToolEnd(sFrobMsg*, cMultiParm&);
	virtual long OnFrobWorldEnd(sFrobMsg*, cMultiParm&);
	virtual long OnFrobWorldBegin(sFrobMsg*, cMultiParm&);
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryMold","FactoryLever",cScr_FactoryMold)
#endif // SCR_GENSCRIPTS

/**
 * Script: MoldSocket
 * Inherits: BaseScript
 * Messages: SocketMe, UnsocketMe
 * Links: ScriptParams, Owns
 * Properties: Physics\Model\Attributes, Engine Features\KeySrc, Engine Features\KeyDst
 *
 * Movable molds are placed on a socket before they can be used. The socket holds
 * template links that are transfered to the mold when it is placed on the socket. These
 * links are ``ScriptParams`` with the data set to the flavor of the real link. The template
 * for a ``ScriptParams`` link can be named ''SP:??data??'' (or ''~SP:??data??'') and the link
 * data will also be set on the new link.
 *
 * When a mold is frobbed on a socket, the ``SocketMe`` message is sent to try using
 * the mold. If the ``Engine Features\KeyDst`` property is set on the socket, then the
 * mold must have a matching ``Engine Features\KeySrc`` property. Otherwise any mold
 * can be used. The mold will be teleported to the location of the socket plus the
 * $$COG Offset$$ from the ``Physics\Model\Attributes`` property. An ``Owns`` link
 * is created from the mold to the socket.
 *
 * === How to Use ===
 * The mold socket has ``ScriptParams`` links to all the objects that a mold would
 * link to. The link data is the type of link. For the mold lever, set the data to $$~ControlDevice$$.
 * To the ``FnordLock`` that also locks the mold lever, the data is $$Lock$$. to the
 * ``FnordLock`` for the cauldron lever, $$SP:LockMe$$. Link to the gauge with the
 * data $$~SP:Mold$$.
 */
#if !SCR_GENSCRIPTS
class cScr_MoldSocket : public cBaseScript
{
public:
	cScr_MoldSocket(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }


protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("MoldSocket","BaseScript",cScr_MoldSocket)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryCauldron
 * Inherits: FactoryBase
 * Messages: GoForward, GoReverse, Halt
 * Links: ScriptParams(Synch), ScriptParams(Sparks)
 * Properties: Tweq\Models, Renderer\AnimLight
 * Schemas: cauldron_lp, lava_pour, cauldron_pivot
 *
 * A cauldron manipulates the model tweq and light as well as the joints of the
 * object. Another tweqable object can be linked to with $$Synch$$ and it will
 * receive ``GoForward`` and ``GoReverse`` messages when the cauldron
 * is moving. An object linked with $$Sparks$$ will be turned on when the
 * cauldron is activated.
 *
 * Schemas with the names ``cauldron_lp``, ``cauldron_pivot``, and ``lava_pour``
 * are played when the cauldron is inactive, moving, and activated respectively.
 *
 * === How to Use ===
 * The cauldron is controlled by a cauldron lever. It links with ``ControlDevice``
 * to a gauge. There can be a $$Synch$$ link to a ``MovingGear`` and a $$Sparks$$
 * link to a ``SparkShower``.
 */
#if !SCR_GENSCRIPTS
class cScr_FactoryCauldron : public cScr_Factory
{
public:
	cScr_FactoryCauldron(const char* pszName, int iHostObjId)
		: cScr_Factory(pszName, iHostObjId)
	{ }

protected:
	virtual void BeginGoForward(void);
	virtual void EndGoForward(void);
	virtual void BeginGoBackward(void);
	virtual void EndGoBackward(void);
	virtual char const * ReportType(void);

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryCauldron","FactoryBase",cScr_FactoryCauldron)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryGauge
 * Inherits: FactoryBase
 * Messages: Halt, Full
 * Links: ScriptParams(Mold)
 * Properties: Tweq\Joints
 * Parameters: rates(vector), overflow(number)
 *
 * A guage has two stages. When activated, it moves to the end of the first stage,
 * then sends ``Full`` to the $$Mold$$ link. The guage will stop moving when it
 * gets the ``Halt`` message and turning it on again will resume from that position.
 * The tweq continues past the end of the first stage until the second stage ends.
 * Then it automatically reverses and sends ``TurnOn`` along ``ControlDevice``
 * links.
 *
 * The speed and positions of the first stage are set in the ``Tweq\Joints`` property.
 * For the second stage, the value of the $$overflow$$ parameter is added to the
 * end of the first stage. The default overflow is ''20''. The speed of the tweq is
 * different for each stage, and also when moving backward. The $$rates$$
 * parameter is three numbers for the first, second, and reverse speeds. The default
 * speeds are ''1.75,0.5,30''.
 *
 * === How to Use ===
 * Set the joint tweq to the normal speed and the maximum position where the
 * mold will be filled. The gauge is controlled by the cauldron. It links to an
 * ``alarmlite`` with ``ControlDevice``. Use an ``Inverter`` trap to link to the
 * cauldron lever. When a mold is in-place, there is a $$Mold$$ link to the mold.
 */
#if !SCR_GENSCRIPTS
class cScr_FactoryGauge : public cScr_Factory
{
public:
	cScr_FactoryGauge(const char* pszName, int iHostObjId)
		: cScr_Factory(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_FactoryGauge,m_iState,iHostObjId),
		  SCRIPT_VAROBJ(cScr_FactoryGauge,m_fMidJoint,iHostObjId)
	{ }

protected:
	virtual void BeginGoForward(void);
	virtual void EndGoForward(void);
	virtual void BeginGoBackward(void);
	virtual void EndGoBackward(void);
	virtual char const * ReportType(void);

private:
	script_int m_iState;
	script_float m_fMidJoint;

	float GetRate(int state);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryGauge","FactoryBase",cScr_FactoryGauge)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryLight
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff
 * Properties: Renderer\Render Type, Renderer\AnimLight, Script\Timing
 * Schemas: (Event Activate)
 *
 * The light will automatically turn off when turned on. It will be active for
 * the duration of ``Script\Timing`` which defaults to 550 ms. The ``Renderer\AnimLight``
 * property is optional. Without it, the object will just be rendered at full-brightness.
 * If the ``Renderer\AnimLight`` property is used, you still need the ``AnimLight``
 * script to activate the light.
 */
#if !SCR_GENSCRIPTS
class cScr_LightBlinker : public cBaseScript
{
public:
	cScr_LightBlinker(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_LightBlinker,m_hTimer,iHostObjId)
	{ }

private:
	script_handle<tScrTimer> m_hTimer;

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryLight","BaseScript",cScr_LightBlinker)
#endif // SCR_GENSCRIPTS

/**
 * Script: FactoryWork
 * Inherits: BaseAIScript
 * Messages: ObjActResult, MotionEnd, Alertness, AIModeChange, Slain, Report
 * Links: Owns, Route, ScriptParams(WithHammer)
 * Metaproperties: M-FactoryWorker, M-WithHammer
 * Objects: TrolPt, Molds, Hammers, SmeltingCauldr
 * Parameters: frobmotion(string), pickupmotion(string), dropmotion(string)
 * SeeAlso: FactoryLever, FactoryMold, FactoryCauldron, FactoryGauge
 *
 * A factory worker is an AI that moves to factory objects and frobs them to
 * create copies of an object, usually a hammer. The script should be in a
 * metaproperty named ``M-FactoryWorker``. When the AI is alerted, it will
 * stop work by removing the metaproperty.
 *
 * The model for the AI should not be carring a hammer. An alternate model
 * that shows the hammer is set in the metaproperty ``M-WithHammer``.
 * If the AI is alerted while it isn't holding a hammer, it will go to the marker
 * where the hammers are dropped then pick one up. A different hammer-holding
 * metaproperty can be used by linking to the metaproperty with a ``ScriptParams``
 * link that has the data set to $$WithHammer$$. This link can be set on the
 * archetype of the AI.
 *
 * The AI will play motions when it does an action. If you are using custom
 * motions, you can set which to use with parameters. The $$frobmotion$$
 * parameter is played when the AI frobs a lever. $$Pickupmotion$$ and
 * $$dropmotion$$ are played when the AI gets or drops a hammer.
 *
 * === How to Use ===
 * Create a ``TrolPt`` that the worker will return to after completing an action.
 * Link to the marker with ``Owns``. Create another ``TrolPt`` that the worker
 * will place the hammer at. Link from the first marker to the second with ``Route``.
 * Link the worker to the mold and the cauldron with ``Owns``.
 */
#if !SCR_GENSCRIPTS
class cScr_FactoryWork : public cBaseAIScript
{
public:
	cScr_FactoryWork(const char* pszName, int iHostObjId)
		: cBaseAIScript(pszName, iHostObjId),
		  SCRIPT_VAROBJ(cScr_FactoryWork,m_iCurrentAction,iHostObjId),
		  SCRIPT_VAROBJ(cScr_FactoryWork,m_iWayPtDest,iHostObjId)
	{ }

private:
	script_int m_iCurrentAction;
	script_int m_iWayPtDest;

protected:
	object GetPath(void);
	object GetOwned(object iArchetype);
	object WhatControls(object iDevice);
	bool IsLeverOn(object iDevice);
	bool IsMoldFull(object iDevice);
	bool IsContained(object iHammer);
	bool IsLocked(object iDevice);

	void GotoWayPt(object iTarget, eAIActionPriority iSpeed=kNormalPriorityAction);
	void PlayMotion(const char* pszType, const char* pszDefault);
	void FrobLever(object iTarget);
	void ChangeHammerModel(bool bWithHammer);
	void PickUp(object iHammer);
	void MakeHammer(void);
	void DecideAction(void);

	enum
	{
		kWorkDecideAction = 0,
		kWorkMoldLeverOff,
		kWorkCauldLeverOn,
		kWorkCauldLeverOff,
		kWorkMoldLeverOn,
		kWorkPickUpHammer,
		kWorkDropHammer,
		kWorkKeepHammer
	};

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnAIModeChange(sAIModeChangeMsg*, cMultiParm&);
	virtual long OnAlertness(sAIAlertnessMsg*, cMultiParm&);
	virtual long OnObjActResult(sAIObjActResultMsg*, cMultiParm&);
	virtual long OnMotionEnd(sBodyMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg* pMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("FactoryWork","BaseAIScript",cScr_FactoryWork)
#endif // SCR_GENSCRIPTS

/**
 * Script: Prisoner
 * Inherits: BaseScript
 * Messages: TurnOn
 * Metaproperties: M-Escapee
 * Signals: Escape
 *
 * When turned on, the signal ``Escape`` is sent to AI that have the
 * metaproperty ``M-Escapee``.
 */
#if !SCR_GENSCRIPTS
class cScr_Prisoner : public cBaseScript
{
public:
	cScr_Prisoner(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Prisoner","BaseScript",cScr_Prisoner)
#endif // SCR_GENSCRIPTS

/**
 * Script: ResetLockbox
 * Inherits: BaseScript
 * Messages: NowUnlocked, TweqComplete
 * Properties: Tweq\Flicker
 *
 * A lock that automatically resets itself. Set the ``Tweq\Flicker``
 * property to the time delay. The tweq is activated when unlocked,
 * and the object is locked when the tweq completes.
 */
#if !SCR_GENSCRIPTS
class cScr_LockResetter : public cBaseScript
{
public:
	cScr_LockResetter(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void StartTimer(void);

protected:
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
	virtual long OnTweqComplete(sTweqMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ResetLockbox","BaseScript",cScr_LockResetter)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrapSecureDoor
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff
 * Links: AIWatchObj
 * Properties: AI\Utility\Watch: Watch link defaults, Engine Features\Suspicious
 * Parameters: watcher(object) - Archetype to create ``AIWatchObj`` links on. Default is ``Human``.
 *
 * Trap that makes the object suspicious when turned on. The suspiciousness
 * can be activated in one of two ways. If the property ``Engine Features\Suspicious``
 * is on the object, then the flag is simply toggled. Otherwise, ``AIWatchObj`` links
 * are created from all ``Human`` AI to this one.
 */
#if !SCR_GENSCRIPTS
class cScr_SuspiciousTrap : public cBaseScript
{
public:
	cScr_SuspiciousTrap(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	cAnsiStr GetWatchers(void);

protected:
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapSecureDoor","BaseScript",cScr_SuspiciousTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: TrigOBBPlayerStuff
 * Inherits: BaseTrap
 * Messages: PhysEnter
 * Links: Firer
 *
 * Triggers when the ``Player`` or something the player threw enters the bounding-box.
 */
#if !SCR_GENSCRIPTS
class cScr_OBBPlayerStuff : public cBaseTrap
{
public:
	cScr_OBBPlayerStuff(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysEnter(sPhysMsg*, cMultiParm&);
	virtual long OnPhysExit(sPhysMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrigOBBPlayerStuff","BaseTrap",cScr_OBBPlayerStuff)
#endif // SCR_GENSCRIPTS

/**
 * Script: CuttyCell
 * Inherits: BaseRoomScript
 * Messages: PlayerRoomEnter, PlayerRoomExit, Presence, Absence
 * Links: ControlDevice
 * SeeAlso: ConvControl
 *
 * Send ``Presence`` along ``ControlDevice`` links when the ``Player`` enters
 * a room. Send ``Absence`` when he leaves.
 */
#if !SCR_GENSCRIPTS
class cScr_ConvRoomPlayer : public cBaseRoomScript
{
public:
	cScr_ConvRoomPlayer(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnPlayerRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnPlayerRoomExit(sRoomMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CuttyCell","BaseRoomScript",cScr_ConvRoomPlayer)
#endif // SCR_GENSCRIPTS

/**
 * Script: NearCuttyCell
 * Inherits: BaseRoomScript
 * Messages: CreatureRoomEnter, CreatureRoomExit, Obituary, Intrusion, Privacy
 * Links: ControlDevice, Population
 * Metaproperties: M-NotifyRegion
 * SeeAlso: ConvControl
 *
 * Activates when a hostile AI enters the room and deactivates when they have all left.
 * The message ``Intrusion`` is sent along ``ControlDevice`` links on entry and
 * ``Privacy`` is sent on exit. The ``M-NotifyRegion`` metaproperty is used so
 * dead or knocked-out AI are not counted.
 */
#if !SCR_GENSCRIPTS
class cScr_ConvRoomOpponent : public cBaseRoomScript, protected cTrackPopulation
{
public:
	cScr_ConvRoomOpponent(const char* pszName, int iHostObjId)
		: cBaseRoomScript(pszName, iHostObjId),
		  cTrackPopulation(iHostObjId)
	{ }

private:
	bool IsOpponent(object iObj);

protected:
	virtual long OnCreatureRoomEnter(sRoomMsg*, cMultiParm&);
	virtual long OnCreatureRoomExit(sRoomMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("NearCuttyCell","BaseRoomScript",cScr_ConvRoomOpponent)
#endif // SCR_GENSCRIPTS

/**
 * Script: ConvControl
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff, NowLocked, NowUnlocked, Presence, Absence, Intrusion, Privacy, ConvNext, ConvEnd
 * Links: ControlDevice
 * SeeAlso: ConvSpeaker, CuttyCell, NearCuttyCell
 *
 * Allows for a long conversation that can be interrupted then resumed where
 * it left off. The controller links to conversation traps with ``ControlDevice``.
 * When turned on, the conversation with the lowest link ID is activated. The last
 * action in the conversation should be to send ``ConvNext`` or ``ConvEnd``
 * to the controller so it can remove the used link. When ``ConvNext`` is used,
 * the controller will activate the next link unless the controller has been turned off.
 * The ``ConvEnd`` message turns off the controller.
 *
 * The controller can be locked to interrupt the conversation. It must be both
 * turned on and unlocked to be active. When unlocked, the next conversation
 * link will be activated if necessary.
 *
 * The message ``Presence`` is the same as ``TurnOn`` and ``Absence`` is
 * the same as ``TurnOff``. The message ``Intrusion`` is as if the controller was
 * locked, and ``Privacy`` like it was unlocked.
 */
#if !SCR_GENSCRIPTS
class cScr_ConvControl : public cBaseScript
{
public:
	cScr_ConvControl(const char* pszName, int iHostObjId);

private:
	script_int m_active;
	script_int m_busy;
	script_int m_intruded;

	void KillPrevLink(void);
	void RunNextLink(void);

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnTurnOn(sScrMsg*, cMultiParm&);
	virtual long OnTurnOff(sScrMsg*, cMultiParm&);
	virtual long OnNowLocked(sScrMsg*, cMultiParm&);
	virtual long OnNowUnlocked(sScrMsg*, cMultiParm&);
	virtual long OnConvNext(sScrMsg*, cMultiParm&);
	virtual long OnConvEnd(sScrMsg*, cMultiParm&);

private:
	static long HandlePresence(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleAbsence(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleIntrusion(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandlePrivacy(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleConvNext(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);
	static long HandleConvEnd(cScript* pScript, sScrMsg* pMsg, sMultiParm* pReply);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ConvControl","BaseScript",cScr_ConvControl)
#endif // SCR_GENSCRIPTS

/**
 * Script: ConvSpeaker
 * Inherits: BaseScript
 * SeeAlso: ConvControl
 *
 * Extra actions for conversations and pseudo-scripts. Put this script on the AI and
 * use the ''Script message'' action. Conversation argument 1 is the name of the message
 * and the other two arguments are used by the message.
 *
 * ^ Message ^ Arguments ^ Action  ^
 * |CompleteGoal |goals |Mark a list of objectives as completed. |
 * |FailGoal |goals |Mark a list of objectives failed. |
 * |CancelGoal |goals |Mark a list of objectives as invalid. |
 * |ShowGoal |goals |Show the listed objectives. |
 * |HideGoal |goals |Hide the listed objectives. |
 * |SwapGoal |goals, goals |Hides the objectives in the first argument and shows the objectives in the second argument. |
 * |IsGoalShown |goals |Test if the objectives are all visible. |
 * |IsGoalComplete |goals |Test if the objectives are completed. |
 * |PlayVO |schema |Play a voice-over schema. |
 * |PlayAmbient |schema |Play an ambient schema. |
 * |Slay |object (opt.) |Slay an object. The name ''self'' or a blank argument will slay the AI. |
 * |ConvNext |object (opt.) |Send the message ``ConvNext`` to the controller. |
 * |ConvEnd |object (opt.) |Send the message ``ConvEnd`` to the controller. |
 *
 * Goal arguments are a list of comma-separated numbers. You cannot list more than
 * 24 goals.
 *
 * The ``ConvNext`` and ``ConvEnd`` actions send a message to a controller object.
 * If the argument is blank, the script will try to locate the object that controls this
 * conversation.
 */
#if !SCR_GENSCRIPTS
class cScr_ConvSpeaker : public cBaseScript
{
public:
	cScr_ConvSpeaker(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	object FindController(void);
	int GetObjList(const char* arg, int objs[24]);

	bool DoObjComplete(const char* arg);
	bool DoObjFail(const char* arg);
	bool DoObjInvalid(const char* arg);
	bool DoObjShow(const char* arg);
	bool DoObjHide(const char* arg);
	bool DoObjReplace(const char* arg1, const char* arg2);
	bool DoObjIsShown(const char* arg);
	bool DoObjIsComplete(const char* arg);
	bool DoVoiceOver(const char* arg);
	bool DoAmbient(const char* arg);
	bool DoSlay(const char* arg);
	bool DoConvNext(const char* arg);
	bool DoConvEnd(const char* arg);

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);

};
#else // SCR_GENSCRIPTS
GEN_FACTORY("ConvSpeaker","BaseScript",cScr_ConvSpeaker)
#endif // SCR_GENSCRIPTS


/*** miss04 ***/

/**
 * Script: HeStartedIt
 * Inherits: BaseScript
 * Messages: Damage, Slain
 * Metaproperties: M-Swaying Burrick
 * Stims: Knockout
 *
 * Activates if the ``Player`` damages, slays, or knocks-out the AI.
 * The metaproperty ``M-Swaying Burrick`` will be removed from all
 * ``Creature`` objects.
 *
 * The AI must have a receptron for ``Knockout`` that has the action
 * ''Send to Scripts''.
 */
#if !SCR_GENSCRIPTS
class cScr_PickAFight : public cBaseScript
{
public:
	cScr_PickAFight(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

private:
	void CheckCulpability(object iSource);

protected:
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
	virtual long OnDamage(sDamageScrMsg*, cMultiParm&);
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("HeStartedIt","BaseScript",cScr_PickAFight)
#endif // SCR_GENSCRIPTS

/**
 * Script: Horn
 * Inherits: BaseScript
 * Messages: Contained
 * Properties: AmbientHacked
 * Metaproperties: M-Swaying Burrick
 *
 * Activates when picked up. The metaproperty ``M-Swaying Burrick`` is
 * removed from all ``Creature`` objects, and the ambient schemas of the
 * object are turned off.
 */
#if !SCR_GENSCRIPTS
class cScr_AccornOfQuintuplets : public cBaseScript
{
public:
	cScr_AccornOfQuintuplets(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnContained(sContainedScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("Horn","BaseScript",cScr_AccornOfQuintuplets)
#endif // SCR_GENSCRIPTS

/**
 * Script: MagicBones
 * Inherits: BaseScript
 * Messages: Slain, PhysCollision, BoneSlain
 * Links: Owns
 * SeeAlso: MagicCoffin
 *
 * Bone objects are meant to be placed on a coffin object. The coffin has ``Owns``
 * links to all the bones. When the bone touches the coffin, it is slain and the coffin
 * notified with ``BoneSlain``.
 */
#if !SCR_GENSCRIPTS
class cScr_MagicBone : public cBaseScript
{
public:
	cScr_MagicBone(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnBeginScript(sScrMsg*, cMultiParm&);
	virtual long OnEndScript(sScrMsg*, cMultiParm&);
	virtual long OnPhysCollision(sPhysMsg*, cMultiParm&);
	virtual long OnSlain(sSlayMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("MagicBones","BaseScript",cScr_MagicBone)
#endif // SCR_GENSCRIPTS

/**
 * Script: MagicCoffin
 * Inherits: BaseTrap
 * Messages: BoneSlain
 * Links: Owns
 * SeeAlso: MagicBones
 *
 * Listens for the ``BoneSlain`` message. When all the ``Owns`` links from
 * this object have been removed, then ``TurnOn`` is relayed along ``ControlDevice``
 * links.
 */
#if !SCR_GENSCRIPTS
class cScr_MagicCoffin : public cBaseTrap
{
public:
	cScr_MagicCoffin(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("MagicCoffin","BaseTrap",cScr_MagicCoffin)
#endif // SCR_GENSCRIPTS


/*** miss05 ***/

/**
 * Script: TrapAIWake
 * Inherits: BaseTrap
 * Messages: TurnOn, WakeyWakey
 * Links: ControlDevice
 * SeeAlso: WakeableAI
 *
 * Sends the message ``WakeyWakey`` along ``ControlDevice`` links when turned on.
 */
#if !SCR_GENSCRIPTS
class cScr_WakeTrap : public cBaseTrap
{
public:
	cScr_WakeTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("TrapAIWake","BaseTrap",cScr_WakeTrap)
#endif // SCR_GENSCRIPTS

/**
 * Script: WakeableAI
 * Inherits: BaseScript
 * Messages: WakeyWakey
 * Properties: AI\State\Current mode
 * SeeAlso: TrapAIWake
 *
 * Activates frozen AI when the message ``WakeyWakey`` is received.
 * The AI mode is changed from ''Asleep'' to ''Normal''.
 */
#if !SCR_GENSCRIPTS
class cScr_WakeAI : public cBaseScript
{
public:
	cScr_WakeAI(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
	{ }

protected:
	virtual long OnMessage(sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("WakeableAI","BaseScript",cScr_WakeAI)
#endif // SCR_GENSCRIPTS

/**
 * Script: CleanObjDestroy
 * Inherits: BaseTrap
 * Messages: TurnOn
 * Links: ControlDevice
 *
 * Sneakily destroy objects behind the player's back. An object will not be
 * destroyed if it is in the player's field of view, or is held by the player.
 * The trap keeps trying to destroy objects until they are all gone.
 * Doors, AI, and containers are destroyed cleanly, including the objects
 * in the containers.
 */
#if !SCR_GENSCRIPTS
class cScr_PoliteDestroyTrap : public cBaseTrap
{
public:
	cScr_PoliteDestroyTrap(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
	{ }

private:
	bool AttemptDestroy(void);

protected:
	virtual long OnTimer(sScrTimerMsg*, cMultiParm&);
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("CleanObjDestroy","BaseTrap",cScr_PoliteDestroyTrap)
#endif // SCR_GENSCRIPTS

#endif // _DARKGAME == 2

#endif // T2SCRIPTS_H
#ifdef SCR_GENSCRIPTS
#undef T2SCRIPTS_H
#endif
