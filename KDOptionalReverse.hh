/******************************************************************************
 *  KDOptionalReverse.h: DarkObjectives, KDOptionalReverse
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

#ifndef KDOPTIONALREVERSE_H
#define KDOPTIONALREVERSE_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class DarkObjectives
{
public:
	bool Exists (uint objective);

	eGoalState GetState (uint objective);
	void SetState (uint objective, eGoalState state);

	bool IsVisible (uint objective);
	void SetVisible (uint objective, bool visible);

	bool IsFinal (uint objective);
	bool IsIrreversible (uint objective);
	bool IsReverse (uint objective);
#if (_DARKGAME == 2)
	bool IsOptional (uint objective);
	bool IsBonus (uint objective);
#endif

	eGoalType GetType (uint objective);

	void Subscribe (object host, uint objective, const char* field);
	void Unsubscribe (object host, uint objective, const char* field);

	void DebugObjectives ();
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_OptionalReverse : public virtual cBaseScript, public DarkObjectives
{
public:
	cScr_OptionalReverse (const char* pszName, int iHostObjId);

protected:
	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
#if (_DARKGAME == 2)
	virtual long OnQuestChange (sQuestMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	uint GetNegation (uint objective);
	bool UpdateNegation (uint objective, bool final);
#endif // _DARKGAME == 2
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDOptionalReverse","BaseScript",cScr_OptionalReverse)
#endif // SCR_GENSCRIPTS



#endif // KDOPTIONALREVERSE_H

