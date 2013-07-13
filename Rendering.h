/******************************************************************************
 *  Rendering.h: scripts affecting weather, precipitation, and textures
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

#ifndef RENDERING_H
#define RENDERING_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#include "BaseTrap.h"
#include "scriptvars.h"
#include "utils.h"
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_TransitionTrap : public virtual cBaseTrap
{
public:
	cScr_TransitionTrap (const char* pszName, int iHostObjId);

protected:
	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);

	virtual bool OnPrepare (bool state);
	void Begin ();
	virtual bool OnIncrement ();

	float GetProgress ();
	float Interpolate (float start, float end);
	ulong InterpolateColor (ulong start, ulong end);

private:
	static const int INCREMENT_TIME;
	void Increment ();

	script_handle<tScrTimer> timer;
	script_int time_total, time_remaining;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTransitionTrap","BaseTrap",cScr_TransitionTrap)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_TrapEnvMap : public virtual cBaseTrap
{
public:
	cScr_TrapEnvMap (const char* pszName, int iHostObjId);

protected:
	enum
	{
		GLOBAL_ZONE = 0,
		MIN_ZONE = 0,
		MAX_ZONE = 63
	};

	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapEnvMap","BaseTrap",cScr_TrapEnvMap)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class DarkFog
{
public:
	enum Zone
	{
		FOG_DISABLED = -1,
		GLOBAL_ZONE = 0,
		ZONE_1,
		ZONE_2,
		ZONE_3,
		ZONE_4,
		ZONE_5,
		ZONE_6,
		ZONE_7,
		ZONE_8,
		MIN_ZONE = 1,
		MAX_ZONE = 8
	};

	static ulong GetColor (int zone);
	static float GetDistance (int zone);
	static void Set (int zone, ulong color, float distance);

protected:
	static float FakeDistance (float distance, bool final);
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_TrapFog : public cScr_TransitionTrap, public DarkFog
{
public:
	cScr_TrapFog (const char* pszName, int iHostObjId);

protected:
	virtual bool OnPrepare (bool state);
	virtual bool OnIncrement ();

private:
	script_int zone; // Zone
	script_int start_color, end_color; // ulong
	script_float start_dist, end_dist;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapFog","KDTransitionTrap",cScr_TrapFog)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_SyncGlobalFog : public cScr_TransitionTrap, public DarkFog
{
public:
	cScr_SyncGlobalFog (const char* pszName, int iHostObjId);

protected:
	virtual long OnObjRoomTransit (sRoomMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);

	void Sync (ulong color, float distance, bool sync_color = true);
	virtual bool OnIncrement ();

private:
	script_int last_room_zone; // Zone
	script_int start_color, end_color; // ulong
	script_float start_dist, end_dist;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDSyncGlobalFog","KDTransitionTrap",cScr_SyncGlobalFog)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct DarkWeather
{
	DarkWeather ();
	void GetFromMission ();
	void SetInMission () const;

	enum PrecipType
	{
		PRECIP_SNOW = 0,
		PRECIP_RAIN = 1
	} precip_type;
	float precip_freq;
	float precip_speed;
	float vis_dist;
	float rend_radius;
	float alpha;
	float brightness;
	float snow_jitter;
	float rain_length;
	float splash_freq;
	float splash_radius;
	float splash_height;
	float splash_duration;
	cScrStr texture;
	cScrVec wind;
};
#endif // !SCR_GENSCRIPTS



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



#endif // RENDERING_H

