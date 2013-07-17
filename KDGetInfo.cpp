/******************************************************************************
 *  KDGetInfo.cpp
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

#include "KDGetInfo.h"
#include <ScriptLib.h>
#include "utils.h"

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

#if (_DARKGAME == 2)
	if (pVS->IsEditor () == 0)
		pQS->Set ("info_mission", pDGS->GetCurrentMission (),
			kQuestDataMission);
#endif

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

