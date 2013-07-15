/******************************************************************************
 *  KDTrapFog.h: DarkFog, KDTrapFog, KDSyncGlobalFog
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

#ifndef KDTRAPFOG_H
#define KDTRAPFOG_H

#if !SCR_GENSCRIPTS
#include "KDTransitionTrap.h"
#include "scriptvars.h" //FIXME
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
	static float InterpolateDistance (float start, float end,
		float progress);
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

	void Sync (ulong color, float distance, bool sync_color = true,
		bool sync_distance = true);
	virtual bool OnIncrement ();

private:
	script_int last_room_zone; // Zone
	script_int start_color, end_color; // ulong
	script_float start_dist, end_dist;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDSyncGlobalFog","KDTransitionTrap",cScr_SyncGlobalFog)
#endif // SCR_GENSCRIPTS



#endif // KDTRAPFOG_H

