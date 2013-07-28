/******************************************************************************
 *  KDOptionalReverse.cpp: DarkObjectives, KDOptionalReverse
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

#include "KDOptionalReverse.h"
#include <ScriptLib.h>
#include "utils.h"



/* DarkObjectives */

#define MAX_QVAR 32
#define NO_OBJECTIVE UINT_MAX

bool
DarkObjectives::Exists (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_state_%d", objective);
	return pQS->Exists (qvar);
}

eGoalState
DarkObjectives::GetState (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_state_%u", objective);
	int result = pQS->Get (qvar);
	return (result >= kGoalIncomplete && result <= kGoalFailed)
		? eGoalState (result) : kGoalInactive;
}

void
DarkObjectives::SetState (uint objective, eGoalState state)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_state_%u", objective);
	pQS->Set (qvar, state, kQuestDataMission);
}

bool
DarkObjectives::IsVisible (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_visible_%u", objective);
	return pQS->Get (qvar);
}

void
DarkObjectives::SetVisible (uint objective, bool visible)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_visible_%u", objective);
	pQS->Set (qvar, visible, kQuestDataMission);
}

bool
DarkObjectives::IsFinal (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_final_%u", objective);
	return pQS->Get (qvar);
}

bool
DarkObjectives::IsIrreversible (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_irreversible_%u", objective);
	return pQS->Get (qvar);
}

bool
DarkObjectives::IsReverse (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_reverse_%u", objective);
	return pQS->Get (qvar);
}

#if (_DARKGAME == 2)

bool
DarkObjectives::IsOptional (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_optional_%u", objective);
	return pQS->Get (qvar);
}

bool
DarkObjectives::IsBonus (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_bonus_%u", objective);
	return pQS->Get (qvar);
}

#endif // _DARKGAME == 2

eGoalType
DarkObjectives::GetType (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_type_%u", objective);
	int result = pQS->Get (qvar);
	return (result >= kGoalNone && result <= kGoalGoTo)
		? eGoalType (result) : kGoalNone;
}

void
DarkObjectives::Subscribe (object host, uint objective, const char* field)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_%s_%u", field, objective);
	pQS->SubscribeMsg (host, qvar, kQuestDataMission);
}

void
DarkObjectives::Unsubscribe (object host, uint objective, const char* field)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_%s_%u", field, objective);
	pQS->UnsubscribeMsg (host, qvar);
}

void
DarkObjectives::DebugObjectives ()
{
	DebugPrintf ("Dumping objectives...");
	DebugPrintf ("###  State Vis  Fin Irr Rev Opt Bon  Type");
	DebugPrintf ("===  ===== ===  === === === === ===  ========");
	for (uint objective = 0; Exists (objective); ++objective)
	{
		char state = '?';
		switch (GetState (objective))
		{
		case kGoalIncomplete: state = '-'; break;
		case kGoalComplete: state = '+'; break;
		case kGoalInactive: state = '/'; break;
		case kGoalFailed: state = 'X'; break;
		}

		const char* type = "????";
		switch (GetType (objective))
		{
		case kGoalNone: type = "none"; break;
		case kGoalTake: type = "take"; break;
		case kGoalSlay: type = "slay"; break;
		case kGoalLoot: type = "loot"; break;
		case kGoalGoTo: type = "goto"; break;
		}

		DebugPrintf ("%03u  %d (%c)  %c    %c   %c   %c   %c   %c   %d (%s)",
			objective,
			GetState (objective), state,
			IsVisible (objective) ? '+' : '-',
			IsFinal (objective) ? '+' : '-',
			IsIrreversible (objective) ? '+' : '-',
			IsReverse (objective) ? '+' : '-',
#if (_DARKGAME == 2)
			IsOptional (objective) ? '+' : '-',
			IsBonus (objective) ? '+' : '-',
#else
			'/', '/',
#endif
			GetType (objective), type
		);
	}
}



/* KDOptionalReverse */

cScr_OptionalReverse::cScr_OptionalReverse (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId)
{}

#if (_DARKGAME == 2)

long
cScr_OptionalReverse::OnSim (sSimMsg* pMsg, cMultiParm&)
{
	if (!pMsg->fStarting) return S_FALSE;

	// subscribe to negations
	for (uint objective = 0; Exists (objective); ++objective)
		if (GetNegation (objective) != NO_OBJECTIVE)
			Subscribe (ObjId (), objective, "state");

	return S_OK;
}

long
cScr_OptionalReverse::OnQuestChange (sQuestMsg* pMsg, cMultiParm&)
{
	if (pMsg->m_oldValue == pMsg->m_newValue)
		return S_FALSE;

	uint objective = NO_OBJECTIVE;
	if (sscanf (pMsg->m_pName, "goal_state_%u", &objective) != 1)
		return S_FALSE;

	return UpdateNegation (objective, false) ? S_OK : S_FALSE;
}

long
cScr_OptionalReverse::OnEndScript (sScrMsg*, cMultiParm&)
{
	// fix what VictoryCheck did
	for (uint objective = 0; Exists (objective); ++objective)
		UpdateNegation (objective, true);

	return S_OK;
}

uint
cScr_OptionalReverse::GetNegation (uint objective)
{
	SService<IQuestSrv> pQS (g_pScriptManager); char qvar[MAX_QVAR];
	snprintf (qvar, MAX_QVAR, "goal_negation_%u", objective);
	return pQS->Exists (qvar) ? pQS->Get (qvar) : NO_OBJECTIVE;
}

bool
cScr_OptionalReverse::UpdateNegation (uint objective, bool final)
{
	uint negation = GetNegation (objective);
	if (negation == NO_OBJECTIVE)
		return false;

	eGoalState obj_state = GetState (objective), neg_state;
	switch (obj_state)
	{
	case kGoalIncomplete:
		neg_state = final
			? kGoalComplete // conditions were never met
			: kGoalIncomplete; // still waiting for result
		break;
	case kGoalComplete: // conditions were met, so objective breached
	case kGoalInactive: // cancelled by something other than VictoryCheck
		neg_state = kGoalInactive;
		break;
	case kGoalFailed: // this shouldn't happen, but hey
	default: // neither should this, of course
		neg_state = kGoalFailed;
		break;
	}

	SetState (negation, neg_state);
	return true;
}

#else // _DARKGAME != 2

long
cScr_OptionalReverse::OnSim (sSimMsg*, cMultiParm&)
{
	DebugPrintf ("Error: This script is not available for this game.");
	return S_FALSE;
}

#endif // _DARKGAME == 2

