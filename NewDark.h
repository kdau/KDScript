/******************************************************************************
 *  NewDark.h: scripts exposing NewDark script-only features
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts
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

#ifndef NEWDARK_H
#define NEWDARK_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#include "BaseTrap.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS



/**
 * AbstractScript: KDTransitionTrap
 * Inherits: BaseTrap
 * Parameter: transition (time) - Length of the transition, in milliseconds.
 *
 * Abstract base script for traps that create smooth transitions between two
 * states. The length of the transition in milliseconds is specified in the
 * "transition" parameter. If that parameter is undefined or set to zero, the
 * transition will be instantaneous. The usual trap control flags, locking, and
 * timing (before the start of the transition, not its length) are respected.
 *
 * A concrete script inheriting from TransitionTrap should implement OnPrepare
 * and OnIncrement, making preliminary calculations for the transition in the
 * former and actually performing the change of state in the latter. OnPrepare
 * has an argument "state" which will indicate the trap's new on/off state.
 * The GetProgress method returns a float value ranging linearly from 0.0 at the
 * start of the transition to 1.0 at its end. The GetIncrementDelta method can
 * be overridden to change the time between increments in milliseconds.
 */
#if !SCR_GENSCRIPTS
class cScr_TransitionTrap : public virtual cBaseTrap
{
public:
	cScr_TransitionTrap (const char* pszName, int iHostObjId);

protected:
	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);

	virtual bool OnPrepare (bool state) = 0;
	virtual bool OnIncrement () = 0;

	float GetProgress ();
	float Interpolate (float start, float end)
		{ return start + GetProgress () * (end - start); }

	virtual int GetIncrementDelta () const { return 50; }

private:
	void Increment ();

	script_handle<tScrTimer> timer;
	script_int time_total, time_remaining;
};
#endif // !SCR_GENSCRIPTS



/**
 * Script: KDGetInfo
 * Inherits: BaseScript
 * Quest Vars: (see below)
 *
 * Every time the mission is loaded or game mode is entered, populates a number
 * of mission quest variables with information about the game environment. These
 * values will be available at Sim time, and can also be tracked by TrigQVar.
 *
 *     info_directx_version: the DirectX version in use (6 = old, 9 = new)
 *     info_display_height: the height of the drawing area (screen/window)
 *     info_display_width: the height of the drawing area (screen/window)
 *         (e.g. fullscreen 1920x1080 is height 1080, width 1920)
 *     info_has_eax: whether EAX is enabled (1 = yes, 0 = no)
 *     info_has_fog: whether fogging is enabled (1 = yes, 0 = no)
 *     info_has_hw3d: whether hardware 3D rendering is enabled (1 = yes, 0 = no)
 *     info_has_sky: whether "enhanced" sky is enabled (1 = yes, 0 = no)
 *     info_has_weather: whether weather is enabled (1 = yes, 0 = no)
 *     info_mission: the number of the current mission (e.g. miss20 = 20)
 *     info_mode: the current engine mode:
 *         0 = regular game mode (e.g. thief2.exe)
 *         1 = editor (e.g. dromed.exe) in edit mode
 *         2 = editor in game mode
 *     info_version_major: the major component of the engine version
 *     info_version_minor: the minor component of the engine version
 *         (e.g. version 1.21 is major 1, minor 21)
 */
#if !SCR_GENSCRIPTS
class cScr_GetInfo : public virtual cBaseScript
{
public:
	cScr_GetInfo (const char* pszName, int iHostObjId);

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg,
		cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);

private:
	void UpdateVariables ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDGetInfo","BaseScript",cScr_GetInfo)
#endif // SCR_GENSCRIPTS



/**
 * Script: KDTrapEnvMapTexture
 * Inherits: BaseTrap
 * Parameter: env_zone (integer) - The zone to change the texture for (0-63).
 * Parameter: env_map_on (string) - The texture to set when turned on.
 * Parameter: env_map_off (string) - The texture to set when turned off.
 *
 * When triggered, changes the cubemap texture associated with an environment
 * zone. If the env_zone parameter is 0, the global texture is changed. (The
 * default textures, both global and per zone, are defined in the Mission
 * Variables -> Environment Maps dialog. See the NewDark documentation for more
 * details on environment maps and reflective materials.)
 *
 * When turned on, sets the texture for the specified zone to the value of the
 * env_map_on parameter, if defined; when turned off, sets it to env_map_off.
 * Values should be specified as relative paths, e.g. "tex\envmaps\hallway".
 * The usual trap control flags, locking, and timing are respected.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapEnvMapTexture : public virtual cBaseTrap
{
public:
	cScr_TrapEnvMapTexture (const char* pszName, int iHostObjId);

protected:
	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapEnvMapTexture","BaseTrap",cScr_TrapEnvMapTexture)
#endif // SCR_GENSCRIPTS



/**
 * Script: KDTrapFog
 * Inherits: KDTransitionTrap
 * Parameter: fog_zone (integer) - The zone to change the fog for (0-8).
 * Parameter: fog_color_on (color) - The color of fog to set when turned on.
 * Parameter: fog_dist_on (real) - The distance of fog to set when turned on.
 * Parameter: fog_color_off (color) - The color of fog to set when turned off.
 * Parameter: fog_dist_off (real) - The distance of fog to set when turned off.
 *
 * When triggered, changes the color and distance of fog for the specified fog
 * zone. If the fog_zone parameter is 0 or undefined, the global fog is changed.
 * (The initial fog, both global and per zone, is defined in the Mission
 * Variables -> Fog Zones dialog.) See TransitionTrap for more behavior details.
 *
 * When turned on, sets the fog color and distance for the specified zone to the
 * values of the fog_color_on and fog_dist_on parameters, respectively. (If the
 * color is undefined or black #000000, it will not be changed. If the distance
 * is undefined or less than zero, it will not be changed.) If the specified
 * distance is 0.0, fog is turned off completely; transitions to and from
 * turned-off fog are handled correctly.
 *
 * When turned off, sets the fog color and distance to fog_color_off and
 * fog_dist_off, with the same behaviors as above.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapFog : public cScr_TransitionTrap
{
public:
	cScr_TrapFog (const char* pszName, int iHostObjId);

protected:
	virtual bool OnPrepare (bool state);
	virtual bool OnIncrement ();

private:
	script_int zone,
		start_red, start_green, start_blue,
		end_red, end_green, end_blue;
	script_float start_dist, end_dist;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapFog","KDTransitionTrap",cScr_TrapFog)
#endif // SCR_GENSCRIPTS



/**
 * Script: KDTrapNextMission
 * Inherits: BaseTrap
 * Parameter: next_mission_on (integer) - The mission to set when turned on.
 * Parameter: next_mission_off (integer) - The mission to set when turned off.
 *
 * When triggered, changes which mission will begin after the current mission
 * ends. (By default, the next mission is the mission specified in missflag.str
 * in "miss_N_next" [where N is the current mission], or else one mission past
 * the current mission.)
 *
 * When turned on, sets the next mission to the value of the next_mission_on
 * parameter, if defined; when turned off, sets it to next_mission_off. The
 * usual trap control flags, locking, and timing are respected.
 *
 * IMPORTANT: The engine does not perform any validation on mission numbers.
 * If the next mission is set to a nonexistent mission, the game will crash
 * as soon as it tries to load it. If the chosen mission has the "skip" flag
 * set, it will still be skipped (e.g. choosing 3 in T2 will get mission 4).
 * If the mission calling this script has the "end" flag, this script will have
 * no effect.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapNextMission : public virtual cBaseTrap
{
public:
	cScr_TrapNextMission (const char* pszName, int iHostObjId);

protected:
	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapNextMission","BaseTrap",cScr_TrapNextMission)
#endif // SCR_GENSCRIPTS



/**
 * Script: KDTrapWeather
 * Inherits: KDTransitionTrap
 * Parameter: precip_freq_on (real) - The frequency to set when turned on.
 * Parameter: precip_speed_on (real) - The speed to set when turned on.
 * Parameter: precip_freq_off (real) - The frequency to set when turned off.
 * Parameter: precip_speed_off (real) - The speed to set when turned off.
 *
 * When triggered, changes the frequency and speed of precipitation (weather).
 * The frequency is expressed in number of new drops per second, while the speed
 * is expressed in feet per second. Both values must be greater than or equal
 * to zero. (The default weather, including a number of other values, is defined
 * in the Mission Variables -> Weather dialog.) See TransitionTrap for more
 * behavior details.
 *
 * When turned on, sets the precipitation frequency and speed to the values of
 * the precip_freq_on and precip_speed_on parameters, respectively. (If either
 * is not defined or is less than zero, it will not be changed.) If the defined
 * frequency is 0.0, precipitation will be turned off completely.
 *
 * When turned off, sets the precipitation frequency and speed to
 * precip_freq_off and precip_speed_off, with the same behaviors as above.
 */
#if !SCR_GENSCRIPTS
class cScr_TrapWeather : public cScr_TransitionTrap
{
public:
	cScr_TrapWeather (const char* pszName, int iHostObjId);

protected:
	virtual bool OnPrepare (bool state);
	virtual bool OnIncrement ();

private:
	script_float start_freq, end_freq,
		start_speed, end_speed;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapWeather","KDTransitionTrap",cScr_TrapWeather)
#endif // SCR_GENSCRIPTS



#endif // NEWDARK_H

