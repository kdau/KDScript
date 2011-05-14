/******************************************************************************
 *  CommonScripts.h
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
#ifndef COMMONSCRIPTS_H
#define COMMONSCRIPTS_H

#if !SCR_GENSCRIPTS
#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/types.h>
#endif // SCR_GENSCRIPTS

/**
 * Script: BaseQVarText
 * Properties: Book\Text, Script\Use Message
 *
 * Displays text with substitutions using quest variables.
 * The string ''%{??qvar??}'' will be replaced with the value of the
 * quest variable. ''%?{??qvar??}[a][b]'' will print ''a'' if the qvar is
 * non-zero and ''b'' otherwise. ''%>??num??{??qvar??}[a][b]'' is
 * similar, but compares the value to a number. ''>'' can be
 * one of ''>'', ''<'', ''=''. Whenever a conditional substitution
 * is made, the string will be re-parsed, so you can have
 * secondary (and tertiary...) substitutions.
 */
#if !SCR_GENSCRIPTS
class cQVarText
{
public:
	cQVarText(int iHostObjId)
		: m_iObjId(iHostObjId)
	{ }

private:
	int m_iObjId;

protected:
	void Display(object iFrobber);
	int Bracket(cAnsiStr& strBracket, const cAnsiStr& strText, int iStart);
};
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseQuestVariable
 * Properties: Trap\Quest Var, Script\QB Name
 *
 * Common functions for working with quest variables.
 */
#if !SCR_GENSCRIPTS
class cQVarProcessor
{
protected:
	static int TrapProcess(bool bPositive, char cOp, int iArg, int iVal);
	static bool TrigProcess(char cOp, int iArg, int iVal, char const* pszArg);
	/**
	 * GetQVarParams
	 *
	 * Retrieve the name, operator, and argument from the Trap\QuestVar
	 * (or Script\QB Name) property.
	 *
	 * Returns the entire property string (allocated by ``new char[]'').
	 */
	static char* GetQVarParams(object iObjId, char* pcOp, int* piVal, char** pszQVar);
	static int GetQVar(object iObjId, const char* pszName);
#if (_DARKGAME == 3)
	static void SetQVar(object iObjId, const char* pszName, int iVal, bool bCamp=true);
#else
	static void SetQVar(object iObjId, const char* pszName, int iVal, bool bCamp=false);
#endif
};
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BaseRequirement
 * Links: ScriptParams(Require)
 */
#if !SCR_GENSCRIPTS
class cRequirement
{
public:
	cRequirement(int iHostObjId)
		: m_iObjId(iHostObjId)
	{ }

private:
	int m_iObjId;

protected:
	uint Requirements(void);
	bool TurnOn(object iObj);
	bool TurnOff(object iObj);
};
#endif // SCR_GENSCRIPTS

/**
 * AbstractScript: BasePopulation
 * Links: Population, ScriptParams(Population)
 * Metaproperties: M-NotifyRegion
 *
 * Maintains the Population link used by OBB and Room triggers.
 */
#if !SCR_GENSCRIPTS
class cTrackPopulation
{
public:
	cTrackPopulation(int iHostObjId)
		: m_iObjId(iHostObjId)
	{ }

private:
	int m_iObjId;

protected:
	uint Population(void);
	bool TrackCreatureEnter(object iObj);
	bool TrackCreatureExit(object iObj);
	bool TrackPlayerEnter(object iObj);
	bool TrackPlayerExit(object iObj);

};
#endif // SCR_GENSCRIPTS


#endif // PUBLICSCRIPTS_H
