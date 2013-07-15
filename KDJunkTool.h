/******************************************************************************
 *  KDJunkTool.h
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

#ifndef KDJUNKTOOL_H
#define KDJUNKTOOL_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#endif // SCR_GENSCRIPTS

#if !SCR_GENSCRIPTS
class cScr_JunkTool : public virtual cBaseScript
{
public:
	cScr_JunkTool (const char* pszName, int iHostObjId);

protected:
	virtual long OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeSelect (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeFocus (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnFrobInvEnd (sFrobMsg* pMsg, cMultiParm& mpReply);
	virtual long OnFrobToolEnd (sFrobMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnTimer (sScrTimerMsg* pMsg, cMultiParm& mpReply);
	virtual long OnSlain (sSlayMsg* pMsg, cMultiParm& mpReply);
	virtual long OnDestroy (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	object GetAvatarContainer ();

	void StartJunk (object avatar);
	void EndJunk (object avatar);

	object CreateFrobbable (object avatar);
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDJunkTool","BaseScript",cScr_JunkTool)
#endif // SCR_GENSCRIPTS

#endif // KDJUNKTOOL_H

