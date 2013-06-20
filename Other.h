/******************************************************************************
 *  Other.h: miscellaneous useful scripts
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
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_ShortText : public virtual cBaseScript
{
public:
	cScr_ShortText (const char* pszName, int iHostObjId);

protected:
	virtual long OnFrobWorldEnd (sFrobMsg* pMsg, cMultiParm& mpReply);
	virtual long OnWorldSelect (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	void DisplayMessage ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDShortText","BaseScript",cScr_ShortText)
#endif // SCR_GENSCRIPTS



#endif // OTHER_H

