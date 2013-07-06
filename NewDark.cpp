/******************************************************************************
 *  NewDark.cpp: scripts exposing NewDark script-only features
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

#include "NewDark.h"
#include <ScriptLib.h>
#include "utils.h"



/* KDTransitionTrap */

cScr_TransitionTrap::cScr_TransitionTrap (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId),
	  // associate to final script's name to avoid collisions
	  timer (pszName, "transition_timer", iHostObjId),
	  time_remaining (pszName, "transition_remaining", iHostObjId)
{}

long
cScr_TransitionTrap::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	if (OnPrepare (bState))
	{
		Begin ();
		return S_OK;
	}
	else
		return S_FALSE;
}

long
cScr_TransitionTrap::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "Increment") &&
	    pMsg->data.type == kMT_String && !strcmp (pMsg->data, Name ()))
	{
		Increment ();
		return S_OK;
	}
	return cBaseTrap::OnTimer (pMsg, mpReply);
}

void
cScr_TransitionTrap::Begin ()
{
	if (timer.Valid ()) // stop any previous transition
	{
		KillTimedMessage (timer);
		timer.Clear ();
	}

	time_remaining = GetObjectParamTime (ObjId (), "transition", 0);
	Increment ();
}

float
cScr_TransitionTrap::GetProgress ()
{
	float total = GetObjectParamTime (ObjId (), "transition", 0);
	if (!time_remaining.Valid ())
		return 0.0;
	else if (total == 0.0 || time_remaining == 0)
		return 1.0;
	else
		return (total - float (time_remaining)) / total;
}

float
cScr_TransitionTrap::Interpolate (float start, float end)
{
	return start + GetProgress () * (end - start);
}

ulong
cScr_TransitionTrap::InterpolateColor (ulong start, ulong end)
{
	return AverageColors (start, end, GetProgress ());
}

void
cScr_TransitionTrap::Increment ()
{
	if (OnIncrement () && time_remaining > 0)
	{
		time_remaining =
			std::max (0, time_remaining - GetIncrementDelta ());
		timer = SetTimedMessage ("Increment", GetIncrementDelta (),
			kSTM_OneShot, Name ());
	}
	else
	{
		timer.Clear ();
		time_remaining.Clear ();
	}
}



/* KDGetInfo */

cScr_GetInfo::cScr_GetInfo (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_GetInfo::OnBeginScript (sScrMsg*, cMultiParm&)
{
	// not everything is available yet, so wait for next message cycle
	SimplePost (ObjId (), ObjId (), "UpdateVariables");
	return S_OK;
}

long
cScr_GetInfo::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->message, "UpdateVariables"))
	{
		UpdateVariables ();
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

long
cScr_GetInfo::OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg, cMultiParm&)
{
	if (pMsg->fResuming)
		UpdateVariables ();
	return S_OK;
}

long
cScr_GetInfo::OnEndScript (sScrMsg*, cMultiParm&)
{
	DeleteVariables ();
	return S_OK;
}

void
cScr_GetInfo::UpdateVariables ()
{
	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	SService<IEngineSrv> pES (g_pScriptManager);
	SService<IQuestSrv> pQS (g_pScriptManager);
	SService<IVersionSrv> pVS (g_pScriptManager);

	pQS->Set ("info_directx_version", pES->IsRunningDX6 () ? 6 : 9,
		kQuestDataMission);

	int display_height = 0, display_width = 0;
	pES->GetCanvasSize (display_width, display_height);
	pQS->Set ("info_display_height", display_height, kQuestDataMission);
	pQS->Set ("info_display_width", display_width, kQuestDataMission);

	int value = 0;
	if (pES->ConfigGetInt ("sfx_eax", value))
		pQS->Set ("info_has_eax", value, kQuestDataMission);
	if (pES->ConfigGetInt ("fogging", value))
		pQS->Set ("info_has_fog", value, kQuestDataMission);
	if (pES->ConfigGetInt ("game_hardware", value))
		pQS->Set ("info_has_hw3d", value, kQuestDataMission);
	if (pES->ConfigGetInt ("enhanced_sky", value))
		pQS->Set ("info_has_sky", value, kQuestDataMission);
	if (pES->ConfigGetInt ("render_weather", value))
		pQS->Set ("info_has_weather", value, kQuestDataMission);

	pQS->Set ("info_mission", pDGS->GetCurrentMission (), kQuestDataMission);

	pQS->Set ("info_mode", pVS->IsEditor (), kQuestDataMission);

	int version_major = 0, version_minor = 0;
	pVS->GetVersion (version_major, version_minor);
	pQS->Set ("info_version_major", version_major, kQuestDataMission);
	pQS->Set ("info_version_minor", version_minor, kQuestDataMission);
}

void
cScr_GetInfo::DeleteVariables ()
{
	SService<IQuestSrv> pQS (g_pScriptManager);
	pQS->Delete ("info_directx_version");
	pQS->Delete ("info_display_height");
	pQS->Delete ("info_display_width");
	pQS->Delete ("info_has_eax");
	pQS->Delete ("info_has_fog");
	pQS->Delete ("info_has_hw3d");
	pQS->Delete ("info_has_sky");
	pQS->Delete ("info_has_weather");
	pQS->Delete ("info_mission");
	pQS->Delete ("info_mode");
	pQS->Delete ("info_version_major");
	pQS->Delete ("info_version_minor");
}



/* KDSyncGlobalFog */

cScr_SyncGlobalFog::cScr_SyncGlobalFog (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId),
	  cScr_TransitionTrap (pszName, iHostObjId),
	  SCRIPT_VAROBJ (SyncGlobalFog, last_room_zone, iHostObjId),
	  SCRIPT_VAROBJ (SyncGlobalFog, start_color, iHostObjId),
	  SCRIPT_VAROBJ (SyncGlobalFog, end_color, iHostObjId),
	  SCRIPT_VAROBJ (SyncGlobalFog, start_dist, iHostObjId),
	  SCRIPT_VAROBJ (SyncGlobalFog, end_dist, iHostObjId)
{}

long
cScr_SyncGlobalFog::OnObjRoomTransit (sRoomMsg* pMsg, cMultiParm&)
{
	if (pMsg->ObjType != sRoomMsg::kPlayer)
		return S_FALSE; // we're not the player yet

	SService<IPropertySrv> pPS (g_pScriptManager);
	cMultiParm _new_zone;
	pPS->Get (_new_zone, pMsg->ToObjId, "Weather", "fog");
	if (_new_zone.type != kMT_Int) return S_FALSE;
	// disabled = -1, global = 1, zone_1 =2, etc.
	int new_zone = int (_new_zone) - 1;

	if (last_room_zone.Valid () && last_room_zone == new_zone)
		return S_FALSE; // no change

	ulong color; float dist; bool sync_color;
	if (new_zone == -1) // disabled
	{
		color = 0;
		dist = 0.0;
		sync_color = false;
	}
	else if (new_zone >= 1 && new_zone <= 8)
	{
		int red, green, blue;
		SService<IEngineSrv> pES (g_pScriptManager);
		pES->GetFogZone (new_zone, red, green, blue, dist);
		color = makecolor (red, green, blue);
		sync_color = true;
	}
	else
		return S_FALSE;

	last_room_zone = new_zone;
	Sync (color, dist, sync_color);
	return S_OK;
}

long
cScr_SyncGlobalFog::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->message, "FogZoneChanging") && last_room_zone.Valid () &&
	    pMsg->data.type == kMT_Int && last_room_zone == int (pMsg->data) &&
	    pMsg->data2.type == kMT_Int && pMsg->data3.type == kMT_Float)
	{
		Sync ((int) pMsg->data2, (float) pMsg->data3);
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

void
cScr_SyncGlobalFog::Sync (ulong color, float dist, bool sync_color)
{
	sync_color &= GetObjectParamBool (ObjId (), "sync_fog_color", true);
	bool sync_dist = (dist >= 0.0) &&
		GetObjectParamBool (ObjId (), "sync_fog_dist", true);

	SService<IEngineSrv> pES (g_pScriptManager);
	int start_red, start_green, start_blue; float _start_dist;
	pES->GetFog (start_red, start_green, start_blue, _start_dist);
	start_color = makecolor (start_red, start_green, start_blue);
	start_dist = _start_dist;

	end_color = sync_color ? color : start_color;

	float _ed, mult = GetObjectParamFloat (ObjId (), "fog_dist_mult", 1.0),
		add = GetObjectParamFloat (ObjId (), "fog_dist_add", 0.0);
	end_dist = _ed = (sync_dist || start_dist == 0.0 || dist == 0.0)
		? (dist == 0.0 ? dist : dist * mult + add) : start_dist;

	Begin ();
}

bool
cScr_SyncGlobalFog::OnIncrement ()
{
	ulong color = InterpolateColor (start_color, end_color);

	float fake_end_dist = (end_dist == 0.0 && GetProgress () < 1.0)
		? 100000.0 : end_dist;
	float dist = Interpolate (start_dist, fake_end_dist);

	SService<IEngineSrv> pES (g_pScriptManager);
	pES->SetFog (getred (color), getgreen (color), getblue (color), dist);

	return true;
}



/* KDTrapEnvMap */

cScr_TrapEnvMap::cScr_TrapEnvMap (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId)
{}

long
cScr_TrapEnvMap::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	int zone = GetObjectParamInt (ObjId (), "env_map_zone", 0);
	char* texture = GetObjectParamString (ObjId (),
		bState ? "env_map_on" : "env_map_off", NULL);

	if (zone < 0 || zone > 63 || !texture) return S_FALSE;

	SService<IEngineSrv> pES (g_pScriptManager);
	pES->SetEnvMapZone (zone, texture);
	g_pMalloc->Free (texture);

	return S_OK;
}



/* KDTrapFog */

cScr_TrapFog::cScr_TrapFog (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId),
	  cScr_TransitionTrap (pszName, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, zone, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_color, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_color, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_dist, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_dist, iHostObjId)
{}

bool
cScr_TrapFog::OnPrepare (bool state)
{
	int _zone = GetObjectParamInt (ObjId (), "fog_zone", 0);
	ulong _end_color = GetObjectParamColor (ObjId (),
		state ? "fog_color_on" : "fog_color_off", 0);
	float _end_dist = GetObjectParamFloat (ObjId (),
		state ? "fog_dist_on" : "fog_dist_off", -1.0);

	if (_zone < 0 || _zone > 8 || (_end_color == 0 && _end_dist < 0.0))
		return false;

	zone = _zone;

	int start_red, start_green, start_blue; float _start_dist;
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->GetFog (start_red, start_green, start_blue, _start_dist);
	else
		pES->GetFogZone (zone, start_red, start_green, start_blue,
			_start_dist);
	start_color = makecolor (start_red, start_green, start_blue);
	start_dist = _start_dist;

	end_color = _end_color ? _end_color : start_color;
	end_dist = (_end_dist >= 0.0) ? _end_dist : _start_dist;

	// notify KDSyncGlobalFog if present
	object player = StrToObject ("Player");
	if (player)
		SimpleSend (ObjId (), player, "FogZoneChanging", (int) zone,
			(int) end_color, (float) end_dist);

	return true;
}

bool
cScr_TrapFog::OnIncrement ()
{
	ulong color = InterpolateColor (start_color, end_color);

	float fake_end_dist = (end_dist == 0.0 && GetProgress () < 1.0)
		? 100000.0 : end_dist;
	float dist = Interpolate (start_dist, fake_end_dist);

	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->SetFog (getred (color), getgreen (color), getblue (color),
			dist);
	else
		pES->SetFogZone (zone, getred (color), getgreen (color),
			getblue (color), dist);

	return true;
}



/* KDTrapNextMission */

cScr_TrapNextMission::cScr_TrapNextMission (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId)
{}

long
cScr_TrapNextMission::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	int next_mission = GetObjectParamInt (ObjId (),
		bState ? "next_mission_on" : "next_mission_off", 0);

	if (next_mission < 1) return S_FALSE;

	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	pDGS->SetNextMission (next_mission);

	return S_OK;
}



/* DarkWeather */

struct DarkWeather
{
	DarkWeather ();
	void SetFromMission ();
	void ApplyToMission () const;

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

DarkWeather::DarkWeather ()
	: precip_type (PRECIP_SNOW), precip_freq (0.0), precip_speed (0.0),
	  vis_dist (0.0), rend_radius (0.0), alpha (0.0), brightness (0.0),
	  snow_jitter (0.0), rain_length (0.0), splash_freq (0.0),
	  splash_radius (0.0), splash_height (0.0), splash_duration (0.0),
	  texture (), wind (0.0, 0.0, 0.0)
{
	SetFromMission ();
}

void
DarkWeather::SetFromMission ()
{
	SService<IEngineSrv> pES (g_pScriptManager);
	int _type;
	pES->GetWeather (_type, precip_freq, precip_speed, vis_dist,
		rend_radius, alpha, brightness, snow_jitter, rain_length,
		splash_freq, splash_radius, splash_height, splash_duration,
		texture, wind);
	precip_type = (PrecipType) _type;
}

void
DarkWeather::ApplyToMission () const
{
	SService<IEngineSrv> pES (g_pScriptManager);
	pES->SetWeather (precip_type, precip_freq, precip_speed, vis_dist,
		rend_radius, alpha, brightness, snow_jitter, rain_length,
		splash_freq, splash_radius, splash_height, splash_duration,
		texture, wind);
}



/* KDTrapWeather */

cScr_TrapWeather::cScr_TrapWeather (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId),
	  cScr_TransitionTrap (pszName, iHostObjId),
	  SCRIPT_VAROBJ (TrapWeather, start_freq, iHostObjId),
	  SCRIPT_VAROBJ (TrapWeather, end_freq, iHostObjId),
	  SCRIPT_VAROBJ (TrapWeather, start_speed, iHostObjId),
	  SCRIPT_VAROBJ (TrapWeather, end_speed, iHostObjId)
{}

bool
cScr_TrapWeather::OnPrepare (bool state)
{
	float _freq = GetObjectParamFloat (ObjId (),
		state ? "precip_freq_on" : "precip_freq_off", -1.0);
	float _speed = GetObjectParamFloat (ObjId (),
		state ? "precip_speed_on" : "precip_speed_off", -1.0);

	if (_freq < 0.0 && _speed < 0.0) return false;

	DarkWeather current;
	start_freq = current.precip_freq;
	start_speed = current.precip_speed;

	if (_freq >= 0.0)
		end_freq = _freq;
	else
		end_freq = start_freq;

	if (_speed >= 0.0)
		end_speed = _speed;
	else
		end_speed = start_speed;

	return true;
}

bool
cScr_TrapWeather::OnIncrement ()
{
	DarkWeather weather;
	weather.precip_freq = Interpolate (start_freq, end_freq);
	weather.precip_speed = Interpolate (start_speed, end_speed);
	weather.ApplyToMission ();
	return true;
}

