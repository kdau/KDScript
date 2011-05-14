/******************************************************************************
 *  CommonScripts.cpp
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
#include "CommonScripts.h"
#include "ScriptModule.h"

#include <lg/interface.h>
#include <lg/scrmanagers.h>
#include <lg/scrservices.h>
#include <lg/links.h>

#include "ScriptLib.h"
#include "utils.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cmath>

using namespace std;


/***
 * BaseQVarText
 */
int cQVarText::Bracket(cAnsiStr& strBracket, const cAnsiStr& strText, int iStart)
{
	if (strText.GetAt(iStart) != '[')
	{
		strBracket.Empty();
		return -1;
	}
	uint iEnd, iPos = iStart+1, iStack = 1;
	while (iStack > 0)
	{
		iEnd = strText.FindOneOf("[]", iPos);
		if (iEnd == cAnsiStr::MaxPos)
		{
			strBracket.Empty();
			return -1;
		}
		switch (strText.GetAt(iEnd))
		{
		case '[':
			++iStack;
			break;
		case ']':
			--iStack;
			break;
		}
		iPos = iEnd + 1;
	}
	strText.AllocCopy(strBracket, iEnd-iStart-1, 0, iStart+1);
	return iEnd;
}

void cQVarText::Display(object iFrobber)
{
	cAnsiStr strText = GetBookText(m_iObjId);
	if (strText.IsEmpty())
		return;

	SService<IQuestSrv> pQS(g_pScriptManager);
	char szVal[12];
	cAnsiStr strQVar;
	uint iPos = 0;
	uint iSub, iEnd;
	while ((iSub = strText.Find('%', iPos)) != cAnsiStr::MaxPos)
	{
		switch (strText.GetAt(iSub+1))
		{
		case '{':
		{
			iEnd = strText.Find('}', iSub+2);
			if (iEnd > iSub+2 && iEnd != cAnsiStr::MaxPos)
			{
				strText.AllocCopy(strQVar, iEnd-(iSub+2), 0, iSub+2);
				sprintf(szVal, "%d", pQS->Get(strQVar));
				strText.Replace(szVal, iSub, (iEnd-iSub)+1);
				iPos = iSub + strlen(szVal);
			}
			else
				iPos = iSub + 1;
			break;
		}
		case '?':
		{
			if (strText.GetAt(iSub+2) == '{'
			 && (iEnd = strText.Find('}', iSub+3)) > iSub+3
			 && iEnd != cAnsiStr::MaxPos)
			{
				int iAStart, iAEnd, iBStart, iBEnd;
				cAnsiStr strRepA, strRepB;
				iAStart = iEnd + 1;
				iAEnd = Bracket(strRepA, strText, iAStart);
				if (iAEnd > iAStart)
				{
					iBStart = iAEnd + 1;
					iBEnd = Bracket(strRepB, strText, iBStart);
					if (iBEnd <= iBStart)
						iBEnd = iAEnd;

					strText.AllocCopy(strQVar, iEnd-(iSub+3), 0, iSub+3);
					if (pQS->Get(strQVar))
						strText.Replace(strRepA, iSub, (iBEnd-iSub)+1);
					else
						strText.Replace(strRepB, iSub, (iBEnd-iSub)+1);
					iPos = iSub;
				}
				else
				{
					strText.Remove(iSub, (iEnd-iSub)+1);
					iPos = iSub;
				}
			}
			else
				iPos = iSub + 1;
			break;
		}
		case '<':
		case '>':
		case '=':
		{
			int iNum;
			char* pszNumEnd;
			iNum = strtol(static_cast<const char*>(strText)+iSub+2, &pszNumEnd, 10);
			if (pszNumEnd && *pszNumEnd == '{')
			{
				uint iQVStart = pszNumEnd - static_cast<const char*>(strText) + 1;
				iEnd = strText.Find('}', iQVStart);
				if (iEnd > iQVStart && iEnd != cAnsiStr::MaxPos)
				{
					int iAStart, iAEnd, iBStart, iBEnd;
					cAnsiStr strRepA, strRepB;
					iAStart = iEnd + 1;
					iAEnd = Bracket(strRepA, strText, iAStart);
					if (iAEnd > iAStart)
					{
						iBStart = iAEnd + 1;
						iBEnd = Bracket(strRepB, strText, iBStart);
						if (iBEnd <= iBStart)
							iBEnd = iAEnd;

						strText.AllocCopy(strQVar, iEnd-iQVStart, 0, iQVStart);
						bool bAorB = false;
						switch (strText[iSub+1])
						{
						case '<': bAorB = iNum > pQS->Get(strQVar); break;
						case '>': bAorB = iNum < pQS->Get(strQVar); break;
						case '=': bAorB = iNum == pQS->Get(strQVar); break;
						}
						if (bAorB)
							strText.Replace(strRepA, iSub, (iBEnd-iSub)+1);
						else
							strText.Replace(strRepB, iSub, (iBEnd-iSub)+1);
						iPos = iSub;
					}
					else
					{
						strText.Remove(iSub, (iEnd-iSub)+1);
						iPos = iSub;
					}
				}
				else
					iPos = iSub + 1;
			}
			else
				iPos = iSub + 1;
			break;
		}
		default:
			iPos = iSub + 1;
			break;
		}
	}

#if (_DARKGAME == 3)
	SService<IShockGameSrv> pShock(g_pScriptManager);
	pShock->AddText(strText, iFrobber, CalcTextTime(strText, 500));
#else
	SService<IDarkUISrv> pUI(g_pScriptManager);
	pUI->TextMessage(strText, 0, CalcTextTime(strText, 500));
	iFrobber = iFrobber;
#endif
}


/***
 * BaseQuestVariable
 */
int cQVarProcessor::TrapProcess(bool bPositive, char cOp, int iArg, int iVal)
{
	// flag high-bit for inverse operation
	switch (cOp | (bPositive?0:0x80))
	{
	  case '=':
		iVal = iArg;
		break;
	  case '!':
	  case '|':
		iVal |= iArg;
		break;
	  case ('!'+0x80):
	  case ('|'+0x80):
		iVal &= ~iArg;
		break;
	  case '+':
	  case ('-'+0x80):
		iVal += iArg;
		break;
	  case '-':
	  case ('+'+0x80):
		iVal -= iArg;
		break;
	  case '*':
	  case ('/'+0x80):
	  case ('%'+0x80):
		iVal *= iArg;
		break;
	  case '/':
	  case ('*'+0x80):
		iVal /= iArg;
		break;
	  case '%':
		iVal %= iArg;
		break;
	  case '{':
	  case ('}'+0x80):
		iVal <<= iArg;
		break;
	  case '}':
	  case ('{'+0x80):
		iVal >>= iArg;
		break;
	  case '"':
		iVal = (iVal * 10) + (iArg % 10);
		break;
	  case '#':
		iVal = ((iVal * 10) + (iArg % 10)) % 10000;
		break;
	  case ('"'+0x80):
	  case ('#'+0x80):
		iVal /= 10;
		break;
	  case '?':
	  {
		SService<IDataSrv> pDS(g_pScriptManager);
		iVal += pDS->RandInt(0,iArg);
		break;
	  }
	  case ('?'+0x80):
	  {
		SService<IDataSrv> pDS(g_pScriptManager);
		iVal -= pDS->RandInt(0,iArg);
		break;
	  }
	  case 'd':
	  {
		SService<IDataSrv> pDS(g_pScriptManager);
		iVal += pDS->RandInt(1,iArg);
		break;
	  }
	  case ('d'+0x80):
	  {
		SService<IDataSrv> pDS(g_pScriptManager);
		iVal -= pDS->RandInt(1,iArg);
		break;
	  }
	}
	return iVal;
}

bool cQVarProcessor::TrigProcess(char cOp, int iArg, int iVal, char const* pszArg)
{
	switch (cOp)
	{
	case '=':
		return iVal == iArg;
	case '<':
		return iVal < iArg;
	case '>':
		return iVal > iArg;
	case '&':
		return iVal & iArg;
	case '|':
		return (iVal & iArg) == iArg;
	case '%':
		return (iArg != 0) && ((iVal % iArg) == 0);
	case '"':
	{
		char const* digit = pszArg;
		while (isdigit(*digit)) ++digit;
		if (digit > pszArg)
		{
			double mod = pow(10.0, digit - pszArg);
			return iVal % int(mod) == iArg;
		}
	}
	}
	return false;
}

char* cQVarProcessor::GetQVarParams(object iObjId, char* pcOp, int* piVal, char** pszQVar)
{
	SService<IPropertySrv> pPS(g_pScriptManager);
	if (!pPS->Possessed(iObjId,
#if (_DARKGAME == 3)
		"QBName"
#else
		"TrapQVar"
#endif
	))
		return NULL;
	cMultiParm mpQVar;
	pPS->Get(mpQVar, iObjId,
#if (_DARKGAME == 3)
		"QBName",
#else
		"TrapQVar",
#endif
		NULL);

	const char* pszProp = static_cast<const char*>(mpQVar);
	if (!pszProp)
		return NULL;
	char* pszRet = new char[strlen(pszProp)+1];
	strcpy(pszRet, pszProp);
	if (pcOp)
		*pcOp = *pszProp;
	if (piVal)
		*piVal = strtol(pszProp+1, NULL, 10);
	if (pszQVar)
	{
		char* pszName = strchr(pszRet, ':');
		if (!pszName)
			*pszQVar = NULL;
		else
			*pszQVar = pszName + 1;
	}
	return pszRet;
}

void cQVarProcessor::SetQVar(object, const char* pszName, int iVal, bool bCamp)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	pQS->Set(pszName, iVal, bCamp ? kQuestDataCampaign : kQuestDataMission);
}

int cQVarProcessor::GetQVar(object, const char* pszName)
{
	SService<IQuestSrv> pQS(g_pScriptManager);
	if (pQS->Exists(pszName))
		return pQS->Get(pszName);
	return 0;
}


/***
 * BaseRequirement
 */
uint cRequirement::Requirements(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed("ScriptParams");
	uint nLinks = 0;
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, 0);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Require"))
			++nLinks;
	}

	return nLinks;
}

bool cRequirement::TurnOn(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed("ScriptParams");
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Require"))
			return false;
	}
	link l;
	pLS->Create(l, lk, m_iObjId, iObj);
	if (!l)
		return false;
	pLTS->LinkSetData(l, NULL, "Require");
	return true;
}

bool cRequirement::TurnOff(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed("ScriptParams");
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Require"))
		{
			pLS->Destroy(ls.Link());
			return true;
		}
	}

	return false;
}


/***
 * BasePopulation
 */
uint cTrackPopulation::Population(void)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed(
#if (_DARKGAME == 3)
		"ScriptParams"
#else
		"Population"
#endif
		);
	uint nPopulation = 0;
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, 0);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
#if (_DARKGAME == 3)
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Population"))
#endif
			++nPopulation;
	}

	return nPopulation;
}

bool cTrackPopulation::TrackCreatureEnter(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed(
#if (_DARKGAME == 3)
		"ScriptParams"
#else
		"Population"
#endif
		);
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
#if (_DARKGAME == 3)
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Population"))
#endif
		{
			return false;
		}
	}
	link lPopLink;
	pLS->Create(lPopLink, lk, m_iObjId, iObj);
	if (!lPopLink)
		return false;
#if (_DARKGAME == 3)
	pLTS->LinkSetData(lPopLink, NULL, "Population");
#endif
	AddMetaProperty("M-NotifyRegion", iObj);
	return true;
}

bool cTrackPopulation::TrackCreatureExit(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed(
#if (_DARKGAME == 3)
		"ScriptParams"
#else
		"Population"
#endif
		);
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
#if (_DARKGAME == 3)
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Population"))
#endif
		{
			pLS->Destroy(ls.Link());
			RemoveMetaProperty("M-NotifyRegion", iObj);
			return true;
		}
	}

	return false;
}

bool cTrackPopulation::TrackPlayerEnter(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed(
#if (_DARKGAME == 3)
		"ScriptParams"
#else
		"Population"
#endif
		);
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
#if (_DARKGAME == 3)
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Population"))
#endif
		{
			return false;
		}
	}
	link lPopLink;
	pLS->Create(lPopLink, lk, m_iObjId, iObj);
	if (!lPopLink)
		return false;
#if (_DARKGAME == 3)
	pLTS->LinkSetData(lPopLink, NULL, "Population");
#endif
	return true;
}

bool cTrackPopulation::TrackPlayerExit(object iObj)
{
	SService<ILinkSrv> pLS(g_pScriptManager);
	SService<ILinkToolsSrv> pLTS(g_pScriptManager);

	linkkind lk = pLTS->LinkKindNamed(
#if (_DARKGAME == 3)
		"ScriptParams"
#else
		"Population"
#endif
		);
	linkset ls;
	pLS->GetAll(ls, lk, m_iObjId, iObj);
	for (; ls.AnyLinksLeft(); ls.NextLink())
	{
#if (_DARKGAME == 3)
		if (ls.Data() && !_stricmp(reinterpret_cast<const char*>(ls.Data()), "Population"))
#endif
		{
			pLS->Destroy(ls.query->ID());
			return true;
		}
	}

	return false;
}

