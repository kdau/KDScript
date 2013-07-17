/******************************************************************************
 *  KDTransitionTrap.h
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

#ifndef KDTRANSITIONTRAP_H
#define KDTRANSITIONTRAP_H

#if !SCR_GENSCRIPTS
#include "BaseTrap.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS

#if !SCR_GENSCRIPTS
class cScr_TransitionTrap : public virtual cBaseTrap
{
public:
	cScr_TransitionTrap (const char* pszName, int iHostObjId);

protected:
	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);

	virtual bool OnPrepare (bool state);
	void Begin ();
	virtual bool OnIncrement ();

	float GetProgress ();
	float Interpolate (float start, float end);
	ulong InterpolateColor (ulong start, ulong end);

private:
	static const int INCREMENT_TIME;
	void Increment ();

	script_handle<tScrTimer> timer;
	script_int time_total, time_remaining;
};
#else // SCR_GENSCRIPTS
// Included for hierarchy only. KDTransitionTrap should not be instantiated.
GEN_FACTORY("KDTransitionTrap","BaseTrap",cScr_TransitionTrap)
#endif // SCR_GENSCRIPTS

#endif // KDTRANSITIONTRAP_H

