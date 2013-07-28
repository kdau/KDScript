/******************************************************************************
 *  KDTransitionTrap.cpp
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

#include "KDTransitionTrap.h"
#include <ScriptLib.h>
#include "utils.h"

const int
cScr_TransitionTrap::INCREMENT_TIME = 50;

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
	if (OnPrepare (bState))
	{
		Begin ();
		return S_OK;
	}
	else
		return S_FALSE;
}

long
cScr_TransitionTrap::OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply)
{
	if (!strcmp (pMsg->name, "Increment") &&
	    pMsg->data.type == kMT_String && !strcmp (pMsg->data, Name ()))
	{
		Increment ();
		return S_OK;
	}
	return cBaseTrap::OnTimer (pMsg, mpReply);
}

bool
cScr_TransitionTrap::OnPrepare (bool)
{
	// trap behavior requires an override of this method
	return false;
}

void
cScr_TransitionTrap::Begin ()
{
	if (timer.Valid ()) // stop any previous transition
	{
		KillTimedMessage (timer);
		timer.Clear ();
	}

	time_remaining = GetObjectParamTime (ObjId (), "transition", 0);
	Increment ();
}

bool
cScr_TransitionTrap::OnIncrement ()
{
	DebugPrintf ("Error: OnIncrement unimplemented in script inheriting "
		"from TransitionTrap.");
	return false;
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

float
cScr_TransitionTrap::Interpolate (float start, float end)
{
	return start + GetProgress () * (end - start);
}

ulong
cScr_TransitionTrap::InterpolateColor (ulong start, ulong end)
{
	return AverageColors (start, end, GetProgress ());
}

void
cScr_TransitionTrap::Increment ()
{
	if (OnIncrement () && time_remaining > 0)
	{
		time_remaining =
			std::max (0, time_remaining - INCREMENT_TIME);
		timer = SetTimedMessage ("Increment", INCREMENT_TIME,
			kSTM_OneShot, Name ());
	}
	else
	{
		timer.Clear ();
		time_remaining.Clear ();
	}
}

