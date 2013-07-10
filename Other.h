/******************************************************************************
 *  Other.h: all other scripts
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

#ifndef OTHER_H
#define OTHER_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#include "BaseTrap.h"
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_Carried : public virtual cBaseScript
{
public:
	cScr_Carried (const char* pszName, int iHostObjId);

protected:
	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
	virtual long OnCreate (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	void Drop ();
	void FixPhysics ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDCarried","BaseScript",cScr_Carried)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_Carrier : public virtual cBaseAIScript
{
public:
	cScr_Carrier (const char* pszName, int iHostObjId);

protected:
	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
	virtual long OnCreate (sScrMsg* pMsg, cMultiParm& mpReply);

	virtual long OnAIModeChange (sAIModeChangeMsg* pMsg,
		cMultiParm& mpReply);
	virtual long OnAlertness (sAIAlertnessMsg* pMsg, cMultiParm& mpReply);

private:
	void CreateAttachments ();
	void CreateAttachment (object archetype, int joint);

	void NotifyCarried (const char* message,
		const cMultiParm& data = cMultiParm::Undef);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDCarrier","BaseAIScript",cScr_Carrier)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_GetInfo : public virtual cBaseScript
{
public:
	cScr_GetInfo (const char* pszName, int iHostObjId);

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnDarkGameModeChange (sDarkGameModeScrMsg* pMsg,
		cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	void UpdateVariables ();
	void DeleteVariables ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDGetInfo","BaseScript",cScr_GetInfo)
#endif // SCR_GENSCRIPTS



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



#endif // OTHER_H

