/******************************************************************************
 *  KDTrapWeather.h: DarkWeather, KDTrapWeather
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

#ifndef KDTRAPWEATHER_H
#define KDTRAPWEATHER_H

#if !SCR_GENSCRIPTS
#include "KDTransitionTrap.h"
#include "scriptvars.h"
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
	cScrVec wind;

private:
	cScrStr texture;
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



#endif // KDTRAPWEATHER_H

