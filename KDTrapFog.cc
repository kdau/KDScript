/******************************************************************************
 *  KDTrapFog.cpp: DarkFog, KDTrapFog, KDSyncGlobalFog
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

#include "KDTrapFog.h"
#include <ScriptLib.h>
#include "utils.h"



/* DarkFog */

ulong
DarkFog::GetColor (int zone)
{
	int r = 0, g = 0, b = 0; float d = 0.0;
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == GLOBAL_ZONE)
		pES->GetFog (r, g, b, d);
	else if (zone >= MIN_ZONE && zone <= MAX_ZONE)
		pES->GetFogZone (zone, r, g, b, d);
	return makecolor (r, g, b);
}

float
DarkFog::GetDistance (int zone)
{
	int r = 0, g = 0, b = 0; float d = 0.0;
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == GLOBAL_ZONE)
		pES->GetFog (r, g, b, d);
	else if (zone >= MIN_ZONE && zone <= MAX_ZONE)
		pES->GetFogZone (zone, r, g, b, d);
	return d;
}

void
DarkFog::Set (int zone, ulong color, float distance)
{
	int r = getred (color), b = getblue (color), g = getgreen (color);
	SService<IEngineSrv> pES (g_pScriptManager);
	if (zone == GLOBAL_ZONE)
		pES->SetFog (r, g, b, distance);
	else if (zone >= MIN_ZONE && zone <= MAX_ZONE)
		pES->SetFogZone (zone, r, g, b, distance);
}

float
DarkFog::InterpolateDistance (float start, float end, float progress)
{
	if (progress == 1.0)
		{} // don't fake distance at end
	else if (start == 0.0)
		start = 50.0 * end;
	else if (end == 0.0)
		end = 50.0 * start;
	return start + progress * (end - start);
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
	int _zone = GetObjectParamInt (ObjId (), "fog_zone", GLOBAL_ZONE);
	ulong _end_color = GetObjectParamColor (ObjId (),
		state ? "fog_color_on" : "fog_color_off", 0);
	float _end_dist = GetObjectParamFloat (ObjId (),
		state ? "fog_dist_on" : "fog_dist_off", -1.0);

	if (_zone < GLOBAL_ZONE || _zone > MAX_ZONE ||
	    (_end_color == 0 && _end_dist < 0.0))
		return false;

	zone = _zone;
	start_color = GetColor (zone);
	start_dist = GetDistance (zone);
	end_color = (_end_color > 0) ? _end_color : start_color;
	end_dist = (_end_dist >= 0.0) ? _end_dist : start_dist;

	// notify SyncGlobalFog if present
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
	float distance =
		InterpolateDistance (start_dist, end_dist, GetProgress ());
	Set (zone, color, distance);
	return true;
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

	ulong color; float distance; bool sync_color;
	if (new_zone == FOG_DISABLED)
	{
		if (!GetObjectParamBool (ObjId (), "sync_fog_disabled", false))
			return S_FALSE;
		color = 0;
		distance = 0.0;
		sync_color = false;
	}
	else if (new_zone >= MIN_ZONE && new_zone <= MAX_ZONE)
	{
		color = GetColor (new_zone);
		distance = GetDistance (new_zone);
		sync_color = true;
	}
	else
		return S_FALSE;

	last_room_zone = new_zone;
	Sync (color, distance, sync_color);
	return S_OK;
}

long
cScr_SyncGlobalFog::OnMessage (sScrMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->message, "FogZoneChanging") &&
	    last_room_zone.Valid () &&
	    pMsg->data.type == kMT_Int && last_room_zone == int (pMsg->data) &&
	    pMsg->data2.type == kMT_Int && pMsg->data3.type == kMT_Float)
	{
		Sync ((int) pMsg->data2, (float) pMsg->data3);
		return S_OK;
	}
	return cBaseScript::OnMessage (pMsg, mpReply);
}

void
cScr_SyncGlobalFog::Sync (ulong color, float dist,
	bool sync_color, bool sync_dist)
{
	sync_color &= GetObjectParamBool (ObjId (), "sync_fog_color", true);
	sync_dist &= (dist >= 0.0) &&
		GetObjectParamBool (ObjId (), "sync_fog_dist", true);

	start_color = GetColor (GLOBAL_ZONE);
	start_dist = GetDistance (GLOBAL_ZONE);
	end_color = sync_color ? color : start_color;

	float mult = GetObjectParamFloat (ObjId (), "fog_dist_mult", 1.0),
		add = GetObjectParamFloat (ObjId (), "fog_dist_add", 0.0);
	end_dist = (sync_dist || start_dist == 0.0 || dist == 0.0)
		? (dist == 0.0 ? dist : dist * mult + add) : start_dist;

	Begin ();
}

bool
cScr_SyncGlobalFog::OnIncrement ()
{
	ulong color = InterpolateColor (start_color, end_color);
	float dist = InterpolateDistance (start_dist, end_dist, GetProgress ());
	Set (GLOBAL_ZONE, color, dist);
	return true;
}

