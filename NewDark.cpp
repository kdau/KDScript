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

#include <windows.h>
#undef GetClassName // ugh, Windows...



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
	if (!OnPrepare (bState))
		return 1;

	if (timer.Valid ()) // stop any previous transition
	{
		KillTimedMessage (timer);
		timer.Clear ();
	}

	time_remaining = GetObjectParamTime (ObjId (), "transition", 0);
	Increment ();

	return 0;
}

long
cScr_TransitionTrap::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "Increment") &&
	    pMsg->data.type == kMT_String && !strcmp (pMsg->data, Name ()))
	{
		Increment ();
		return 0;
	}
	else
		return cBaseTrap::OnTimer (pMsg, mpReply);
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
	: cBaseScript (pszName, iHostObjId),
	  did_initial_update (false)
{}

long
cScr_GetInfo::OnBeginScript (sScrMsg*, cMultiParm&)
{
	SetTimedMessage ("UpdateVariables", 10, kSTM_OneShot);
	return 0;
}

long
cScr_GetInfo::OnSim (sSimMsg* pMsg, cMultiParm&)
{
	if (pMsg->fStarting && !did_initial_update)
	{
		did_initial_update = true;
		UpdateVariables ();
	}
	return 0;
}

long
cScr_GetInfo::OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg, cMultiParm&)
{
	if (pMsg->fResuming)
		UpdateVariables ();
	return 0;
}

long
cScr_GetInfo::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "UpdateVariables") && !did_initial_update)
	{
		did_initial_update = true;
		UpdateVariables ();
		return 0;
	}
	else
		return cBaseScript::OnTimer (pMsg, mpReply);
}

void
cScr_GetInfo::UpdateVariables ()
{
	DEBUG_STRING ("updating info_* quest variables");

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



/* KDTrapEnvMapTexture */

cScr_TrapEnvMapTexture::cScr_TrapEnvMapTexture
		(const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId)
{}

long
cScr_TrapEnvMapTexture::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	int zone = GetObjectParamInt (ObjId (), "env_map_zone", 0);
	char* texture = GetObjectParamString (ObjId (),
		bState ? "env_map_on" : "env_map_off", NULL);

	if (zone < 0 || zone > 63 || !texture) return 1;

	DEBUG_PRINTF ("setting cubemap texture for enviroment zone %d to `%s'",
		zone, texture);

	SService<IEngineSrv> pES (g_pScriptManager);
	pES->SetEnvMapZone (zone, texture);
	g_pMalloc->Free (texture);

	return 0;
}



/* KDTrapFog */

cScr_TrapFog::cScr_TrapFog (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId),
	  cBaseTrap (pszName, iHostObjId),
	  cScr_TransitionTrap (pszName, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, zone, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_red, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_green, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_blue, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_red, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_green, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_blue, iHostObjId),
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

	int _sr, _sg, _sb; float _sd;
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->GetFog (_sr, _sg, _sb, _sd);
	else
		pES->GetFogZone (zone, _sr, _sg, _sb, _sd);
	start_red = _sr;
	start_green = _sg;
	start_blue = _sb;
	start_dist = _sd;

	end_red = _end_color ? GetRValue (_end_color) : _sr;
	end_green = _end_color ? GetGValue (_end_color) : _sg;
	end_blue = _end_color ? GetBValue (_end_color) : _sb;
	end_dist = (_end_dist >= 0.0) ? _end_dist : _sd;

	DEBUG_PRINTF ("setting fog for zone %d to RGB %d,%d,%d at distance %f",
		(int) zone, (int) end_red, (int) end_green, (int) end_blue,
		(float) end_dist);

	return true;
}

bool
cScr_TrapFog::OnIncrement ()
{
	int red = Interpolate (start_red, end_red),
		green = Interpolate (start_green, end_green),
		blue = Interpolate (start_blue, end_blue);

	float fake_end_dist = (end_dist == 0.0 && GetProgress () < 1.0)
		? 100000.0 : end_dist;
	float dist = Interpolate (start_dist, fake_end_dist);

	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->SetFog (red, green, blue, dist);
	else
		pES->SetFogZone (zone, red, green, blue, dist);

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

	if (next_mission < 1) return 1;

	DEBUG_PRINTF ("setting next mission to %d", next_mission);

	SService<IDarkGameSrv> pDGS (g_pScriptManager);
	pDGS->SetNextMission (next_mission);

	return 0;
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
	{
		DEBUG_PRINTF ("setting precipitation frequency to %f drops/sec",
			_freq);
		end_freq = _freq;
	}
	else
		end_freq = start_freq;

	if (_speed >= 0.0)
	{
		DEBUG_PRINTF ("setting precipitation speed to %f ft/sec",
			_speed);
		end_speed = _speed;
	}
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

