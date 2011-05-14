/******************************************************************************
 *  BaseTrap.h
 *
 *  This file is part of Public Scripts
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
#ifndef BASETRAP_H
#define BASETRAP_H

#if !SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/types.h>
#include <lg/defs.h>
#include "BaseScript.h"
#endif

/**
 * Script: BaseTrap
 * Inherits: BaseScript
 * Messages: TurnOn, TurnOff
 * Links: ControlDevice, SwitchLink, Lock
 * Properties: Script\Trap Control Flags, Script\Timing, Script\Delay Time, Engine Features\Locked
 * Parameter: tcf(string) - Alternative trap flags. !+ NoOn, !- NoOff, <> Invert, 01 Once
 */
#if !SCR_GENSCRIPTS
class cBaseTrap : public cBaseScript
{
public:
	cBaseTrap(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId),
		  m_iTiming(0), m_iFlags(0)
	{ }

protected:
	//virtual void InitScript(void);

	virtual long OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply);
	virtual long OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply);

private:
	int m_iTiming;
	ulong m_iFlags;

protected:
	virtual long OnSwitch(bool, sScrMsg*, cMultiParm&)
		{ return 0; }

	void InitTrapVars(void);

	int GetTiming(void)
		{ return m_iTiming; }
	void SetTiming(int t)
		{ m_iTiming = t; }
	ulong GetFlags(void)
		{ return m_iFlags; }
	bool GetFlag(ulong f)
		{ return m_iFlags & f; }
	void SetFlags(ulong f)
		{ m_iFlags = f; }
	void SetFlag(ulong f)
		{ m_iFlags |= f; }
	void UnsetFlag(ulong f)
		{ m_iFlags &= ~f; }

	bool IsLocked(void);
	void SetLock(bool bLock);

	void DoTrigger(bool bTurnOn, object iFrobber = 0);
	void DirectTrigger(bool bTurnOn, object iFrobber = 0);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("BaseTrap","BaseScript",cBaseTrap)
#endif // SCR_GENSCRIPTS

#endif // BASETRAP_H
