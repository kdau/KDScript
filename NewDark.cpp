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



/* KDGetInfo */

cScr_GetInfo::cScr_GetInfo (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

long
cScr_GetInfo::OnBeginScript (sScrMsg*, cMultiParm&)
{
	UpdateVariables ();
	return 0;
}

long
cScr_GetInfo::OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg, cMultiParm&)
{
	if (pMsg->fResuming)
		UpdateVariables ();
	return 0;
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
	int zone = GetObjectParamInt (ObjId (), "env_map_zone", -1);
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
	  SCRIPT_VAROBJ (TrapFog, timer, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, zone, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, time_total, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_red, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_green, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_blue, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_red, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_green, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_blue, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, start_dist, iHostObjId),
	  SCRIPT_VAROBJ (TrapFog, end_dist, iHostObjId)
{}

long
cScr_TrapFog::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	if (timer.Valid ()) // stop any previous transition
	{
		KillTimedMessage (timer);
		timer.Clear ();
	}

	zone = GetObjectParamInt (ObjId (), "fog_zone", -1);
	time_total = GetObjectParamTime (ObjId (), "fog_time", 0);
	int end_color = GetObjectParamColor (ObjId (),
		bState ? "fog_color_on" : "fog_color_off", -1);
	end_dist = GetObjectParamFloat (ObjId (),
		bState ? "fog_dist_on" : "fog_dist_off", -1.0);

	if (zone < 0 || zone > 8 || end_color < 0 || end_dist < 0.0) return 1;

	end_red = GetRValue (end_color),
	end_green = GetGValue (end_color),
	end_blue = GetBValue (end_color);

	DEBUG_PRINTF ("setting fog for zone %d to RGB %d,%d,%d at distance %f over %d ms",
		(int) zone, (int) end_red, (int) end_green, (int) end_blue,
		(float) end_dist, (int) time_total);

	int sr, sg, sb; float sd;
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->GetFog (sr, sg, sb, sd);
	else
		pES->GetFogZone (zone, sr, sg, sb, sd);
	start_red = sr;
	start_green = sg;
	start_blue = sb;
	start_dist = sd;

	IncrementFog (time_total);

	return 0;
}

long
cScr_TrapFog::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "IncrementFog"))
	{
		if (timer.Valid ()) timer.Clear (); // one-shot
		IncrementFog (pMsg->data.type == kMT_Int
			? (int) pMsg->data : 0);
	}

	return cBaseTrap::OnTimer (pMsg, mpReply);
}

void
cScr_TrapFog::IncrementFog (int time_remaining)
{
	time_remaining =
		std::max (0, std::min ((int) time_total, time_remaining));
	if (time_remaining > 0)
		timer = SetTimedMessage ("IncrementFog", 50, kSTM_OneShot,
			time_remaining - 50);

	float progress = (time_total - time_remaining) / time_total;

	int red = start_red + progress * (end_red - start_red),
		green = start_green + progress * (end_green - start_green),
		blue = start_blue + progress * (end_blue - start_blue);

	float fake_end_dist = (end_dist == 0.0 && progress < 1.0)
		? 100000.0 : end_dist;
	float dist = start_dist + progress * (fake_end_dist - start_dist);

	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == 0)
		pES->SetFog (red, green, blue, dist);
	else
		pES->SetFogZone (zone, red, green, blue, dist);
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

	int precip_type;
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
	: precip_type (0), precip_freq (0.0), precip_speed (0.0), vis_dist (0.0),
	  rend_radius (0.0), alpha (0.0), brightness (0.0), snow_jitter (0.0),
	  rain_length (0.0), splash_freq (0.0), splash_radius (0.0),
	  splash_height (0.0), splash_duration (0.0), texture (),
	  wind (0.0, 0.0, 0.0)
{
	SetFromMission ();
}

void
DarkWeather::SetFromMission ()
{
	SService<IEngineSrv> pES (g_pScriptManager);
	pES->GetWeather (precip_type, precip_freq, precip_speed, vis_dist,
		rend_radius, alpha, brightness, snow_jitter, rain_length,
		splash_freq, splash_radius, splash_height, splash_duration,
		texture, wind);
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
	  cBaseTrap (pszName, iHostObjId)
{}

long
cScr_TrapWeather::OnSwitch (bool bState, sScrMsg*, cMultiParm&)
{
	float freq = GetObjectParamFloat (ObjId (),
		bState ? "precip_freq_on" : "precip_freq_off", -1.0);
	float speed = GetObjectParamFloat (ObjId (),
		bState ? "precip_speed_on" : "precip_speed_off", -1.0);

	if (freq < 0.0 && speed < 0.0) return 1; // either may be set

	DarkWeather weather;

	if (freq >= 0.0)
	{
		DEBUG_PRINTF ("setting precipitation frequency to %f", freq);
		weather.precip_freq = freq;
	}

	if (speed >= 0.0)
	{
		DEBUG_PRINTF ("setting precipitation speed to %f", speed);
		weather.precip_speed = speed;
	}

	weather.ApplyToMission ();
	return 0;
}

