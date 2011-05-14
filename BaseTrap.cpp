/******************************************************************************
 *  BaseTrap.cpp
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
#include "BaseTrap.h"
#include "ScriptModule.h"

#include <lg/interface.h>
#include <lg/scrmanagers.h>
#include <lg/scrservices.h>

#include "ScriptLib.h"

#include <cstring>

long cBaseTrap::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
#if (_DARKGAME != 3)
	if (pSimMsg->fStarting)
		FixupPlayerLinks();
#endif
	return cBaseScript::OnSim(pSimMsg, mpReply);
}

long cBaseTrap::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitTrapVars();
	if (GetFlag(kTrapFlagNoOn))
		return 0;
	if (IsLocked())
		return 0;
	long iRet = OnSwitch(!GetFlag(kTrapFlagInvert), pMsg, mpReply);
	if (iRet == 0)
	{
		if (GetTiming() > 0)
		{
			g_pScriptManager->SetTimedMessage2(ObjId(), "TrapTimer",
					GetTiming(), kSTM_OneShot, GetFlag(kTrapFlagInvert)?1:0);
		}
		if (GetFlag(kTrapFlagOnce))
			SetLock(true);
	}
	return iRet;
}

long cBaseTrap::OnTurnOff(sScrMsg* pMsg, cMultiParm& mpReply)
{
	InitTrapVars();
	if (GetFlag(kTrapFlagNoOff))
		return 0;
	if (IsLocked())
		return 0;
	long iRet = OnSwitch(GetFlag(kTrapFlagInvert), pMsg, mpReply);
	if (iRet == 0)
	{
		if (GetFlag(kTrapFlagOnce))
			SetLock(true);
	}
	return iRet;
}

long cBaseTrap::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
	if (!::strcmp(pTimerMsg->name, "TrapTimer"))
	{
		m_iTiming = 0;
		m_iFlags = 0;
		OnSwitch(pTimerMsg->data, pTimerMsg, mpReply);
		return 0;
	}
	return cBaseScript::OnTimer(pTimerMsg, mpReply);
}

#if (_DARKGAME == 3)
#define TIMINGPROP	"DelayTime"
#define TRAPFLAGSPROP	"TripFlags"
#else
#define TIMINGPROP	"ScriptTiming"
#define TRAPFLAGSPROP	"TrapFlags"
#endif

void cBaseTrap::InitTrapVars(void)
{
	SService<IPropertySrv> pPropSrv(g_pScriptManager);
	if (pPropSrv->Possessed(ObjId(), TIMINGPROP))
	{
		cMultiParm mpTiming;
		pPropSrv->Get(mpTiming, ObjId(), TIMINGPROP, NULL);
#if (_DARKGAME == 3)
		m_iTiming = float(mpTiming) * 1000.0f;
#else
		m_iTiming = mpTiming;
#endif
	}
	char* pszTCF = GetObjectParamString(ObjId(), "tcf");
	if (pszTCF)
	{
		ulong f = 0;
		uint l = ::strlen(pszTCF);
		unsigned short* p = reinterpret_cast<unsigned short*>(pszTCF);
		for (uint n = 0; n < l; n+=2)
		{
			switch (*p++)
			{
			case 0x2B21: // !+
				f |= kTrapFlagNoOn;
				break;
			case 0x2D21: // !-
				f |= kTrapFlagNoOff;
				break;
			case 0x3E3C: // <>
				f |= kTrapFlagInvert;
				break;
			case 0x3130: // 01
				f |= kTrapFlagOnce;
				break;
			}
		}
		m_iFlags = f;
		g_pMalloc->Free(pszTCF);
	}
	else
	{
		if (pPropSrv->Possessed(ObjId(), TRAPFLAGSPROP))
		{
			cMultiParm mpTrapFlags;
			pPropSrv->Get(mpTrapFlags, ObjId(), TRAPFLAGSPROP, NULL);
#if (_DARKGAME == 3)
			ulong iFlags = mpTrapFlags;
			m_iFlags = ((iFlags & 0x18) >> 3)
				 | ((iFlags & 0x2) << 1)
				 | ((iFlags & 0x1) << 3);
#else
			m_iFlags = mpTrapFlags;
#endif
		}
	}
}

bool cBaseTrap::IsLocked(void)
{
	SService<ILockSrv> pLock(g_pScriptManager);
	return pLock->IsLocked(ObjId());
}

void cBaseTrap::SetLock(bool bLock)
{
	SService<IPropertySrv> pPropSrv(g_pScriptManager);

	object iLock = GetOneLinkDest("Lock", ObjId());
	if (iLock)
	{
		if (pPropSrv->Possessed(ObjId(), "Locked"))
			pPropSrv->Remove(ObjId(), "Locked");

		if (! pPropSrv->Possessed(iLock, "Locked"))
			pPropSrv->Add(iLock, "Locked");
		pPropSrv->SetSimple(iLock, "Locked", int(bLock));
	}
	else
	{
		if (! pPropSrv->Possessed(ObjId(), "Locked"))
			pPropSrv->Add(ObjId(), "Locked");
		pPropSrv->SetSimple(ObjId(), "Locked", int(bLock));
	}
}

void cBaseTrap::DoTrigger(bool bTurnOn, object iFrobber)
{
	InitTrapVars();
	if (!IsLocked())
	{
		CDSend(bTurnOn ? "TurnOn" : "TurnOff", ObjId(), int(iFrobber));
		if (GetFlag(kTrapFlagOnce))
			SetLock(true);
	}
}

void cBaseTrap::DirectTrigger(bool bTurnOn, object iFrobber)
{
	CDSend(bTurnOn ? "TurnOn" : "TurnOff", ObjId(), int(iFrobber));
}

