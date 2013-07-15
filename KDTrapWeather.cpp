/******************************************************************************
 *  KDTrapWeather.cpp: DarkWeather, KDTrapWeather
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDTrapWeather.h"
#include <ScriptLib.h>
#include "utils.h"



/* DarkWeather */

DarkWeather::DarkWeather ()
	: precip_type (PRECIP_SNOW), precip_freq (0.0), precip_speed (0.0),
	  vis_dist (0.0), rend_radius (0.0), alpha (0.0), brightness (0.0),
	  snow_jitter (0.0), rain_length (0.0), splash_freq (0.0),
	  splash_radius (0.0), splash_height (0.0), splash_duration (0.0),
	  texture (), wind (0.0, 0.0, 0.0)
{
	GetFromMission ();
}

void
DarkWeather::GetFromMission ()
{
	SService<IEngineSrv> pES (g_pScriptManager);
	int _precip_type;
	const char* _texture = NULL;
	pES->GetWeather (_precip_type, precip_freq, precip_speed, vis_dist,
		rend_radius, alpha, brightness, snow_jitter, rain_length,
		splash_freq, splash_radius, splash_height, splash_duration,
		_texture, wind);
	precip_type = (PrecipType) _precip_type;
	texture = _texture;
}

void
DarkWeather::SetInMission () const
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
	weather.SetInMission ();
	return true;
}

