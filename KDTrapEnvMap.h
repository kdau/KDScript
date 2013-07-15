/******************************************************************************
 *  KDTrapEnvMap.h
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

#ifndef KDTRAPENVMAP_H
#define KDTRAPENVMAP_H

#if !SCR_GENSCRIPTS
#include "BaseTrap.h"
#endif // SCR_GENSCRIPTS

#if !SCR_GENSCRIPTS
class cScr_TrapEnvMap : public virtual cBaseTrap
{
public:
	cScr_TrapEnvMap (const char* pszName, int iHostObjId);

protected:
	enum
	{
		GLOBAL_ZONE = 0,
		MIN_ZONE = 0,
		MAX_ZONE = 63
	};

	virtual long OnSwitch (bool bState, sScrMsg* pMsg, cMultiParm& mpReply);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDTrapEnvMap","BaseTrap",cScr_TrapEnvMap)
#endif // SCR_GENSCRIPTS

#endif // KDTRAPENVMAP_H

