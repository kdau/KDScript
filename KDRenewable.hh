/******************************************************************************
 *  KDRenewable.h
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

#ifndef KDRENEWABLE_H
#define KDRENEWABLE_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#endif // SCR_GENSCRIPTS

#if !SCR_GENSCRIPTS
class cScr_Renewable : public virtual cBaseScript
{
public:
	cScr_Renewable (const char* pszName, int iHostObjId);

protected:
	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);

private:
	static const int DEFAULT_TIMING;

	bool Renew ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDRenewable","BaseScript",cScr_Renewable)
#endif // SCR_GENSCRIPTS

#endif // KDRENEWABLE_H

