/******************************************************************************
 *  scrversion.cpp
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
#define _SHOCKINTERFACES 1
#include "scrversion.h"
#include "ScriptModule.h"

#include <lg/objstd.h>
#include <lg/interface.h>
#include <lg/types.h>
#include <lg/defs.h>
#include <lg/scrmsgs.h>
#include <lg/scrservices.h>
#include <lg/objects.h>
#include <lg/properties.h>
#include <lg/links.h>

#include "ScriptLib.h"

#include <new>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <memory>
#include <malloc.h>

#include <windows.h>
#include <winver.h>


static char* GetCfgFromFile(const char* pszName, const char* pszFile);
static char* GetScriptPaths(void);
static char* FindScriptModule(const char* pszName, const char* pszPaths);
static bool CheckFileVersion(const char* pszFile, ulong dwVersHigh, ulong dwVersLow);
static void DoSuccess(int iObjId);
static void DoFailure(int iObjId);
static char* GetObjectParamsCompatible(int iObjId);
static void RelayCompatible(const char* pszMsg, int iObjId);
static int ShowBookCompatible(int iObjId, unsigned long ulTime);


const char* cScriptModule::sm_ScriptModuleName = "version";
const sScrClassDesc cScriptModule::sm_ScriptsArray[] = {
	{ sm_ScriptModuleName, "VersionCheck", "CustomScript", cScr_VersionCheck::MakeVersionCheck },
};
const unsigned int cScriptModule::sm_ScriptsArraySize = 1;


IScript* cScr_VersionCheck::MakeVersionCheck(const char* pszName, int iHostObjId)
{
	cScr_VersionCheck* pscrRet = new(std::nothrow) cScr_VersionCheck(pszName, iHostObjId);
	return static_cast<IScript*>(pscrRet);
}

long __stdcall cScr_VersionCheck::ReceiveMessage(sScrMsg* pMsg, sMultiParm*, eScrTraceAction)
{
	try
	{
		if (!::_stricmp(pMsg->message, "Sim"))
		{
			if (!static_cast<sSimMsg*>(pMsg)->fStarting)
			{
				// not sure if this is really necessary, but just to be safe...
				if (m_iTextType == 2)
				{
					// TDP has, rather annoyingly, all the Shock interfaces.
					// So we use a member variable. It's non-persistent though.
					// Let's just pretend the user isn't so demented as to
					// save the game in this state.
					SService<IShockGameSrv> pSGS(g_pScriptManager);
					pSGS->OverlayChange(41,0);
				}
				else if (m_iTextType == 1)
				{
					SService<IDarkUISrv> pUI(g_pScriptManager);
					pUI->TextMessage("", 0, 1);
				}
				return 0;
			}

			std::auto_ptr<char> pszScriptPaths(GetScriptPaths());
			std::auto_ptr<char> pszParams(GetObjectParamsCompatible(ObjId()));
			char* pszScript;
			char* pszToken = pszParams.get();
			for (pszScript = strsep(&pszToken, ";"); pszScript; pszScript = strsep(&pszToken, ";"))
			{
				if (!*pszScript)
					continue;
				ulong scrVersHigh = 0, scrVersLow = 0;
				char* pszVers = ::strchr(pszScript, '=');
				if (pszVers)
				{
					char* pt = pszVers + 1;
					scrVersHigh = ::strtoul(pt, &pt, 10) << 16;
					if (pt && *pt == '.')
					{
						scrVersHigh |= ::strtoul(pt+1, &pt, 10);
						if (pt && *pt == '.')
						{
							scrVersLow = ::strtoul(pt+1, &pt, 10) << 16;
							if (pt && *pt == '.')
								scrVersLow |= ::strtoul(pt+1, &pt, 10);
						}
					}
					*pszVers = '\0';
				}
				std::auto_ptr<char> pszScriptFile(FindScriptModule(pszScript, pszScriptPaths.get()));
				if (!pszScriptFile.get())
				{
					DoFailure(ObjId());
					return 0;
				}
				if (scrVersHigh == 0 && scrVersLow == 0)
				{
					continue;
				}
				if (!CheckFileVersion(pszScriptFile.get(), scrVersHigh, scrVersLow))
				{
					DoFailure(ObjId());
					return 0;
				}
			}
			DoSuccess(ObjId());
		} // "Sim"
		else if (!::_stricmp(pMsg->message, "Timer"))
		{
			if (!::_stricmp(static_cast<sScrTimerMsg*>(pMsg)->name, "ErrorText"))
			{
				m_iTextType = ShowBookCompatible(ObjId(), 0x7FFFFFFFUL);
				return 0;
			}
		}
		}
	catch (...)
	{
	}
	return 0;
}


char* GetCfgFromFile(const char* pszName, const char* pszFile)
{
	char buffer[140];
	char* value = NULL;
	HANDLE hCfgFile = ::CreateFileA(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCfgFile != INVALID_HANDLE_VALUE)
	{
		ulong dwCfgLen = ::strlen(pszName);
		DWORD dwBytes = 0;
		DWORD dwAvail = sizeof(buffer);
		char* startp = buffer;
		while (::ReadFile(hCfgFile, startp, dwAvail-1, &dwBytes, NULL))
		{
			char* line = buffer;
			char* endp = startp + dwBytes;
			*endp = '\0';
			while (line < endp)
			{
				char* endl = ::strchr(line, '\n');
				if (!endl)
				{
					if (dwBytes != 0)
					{
						dwBytes = endp - line;
						::memmove(buffer, line, dwBytes);
						startp = buffer + dwBytes;
						dwAvail = sizeof(buffer) - dwBytes;
						goto parseCfgContinue;
					}
				}
				else
					*endl++ = '\0';

				if (!::strnicmp(line,pszName,dwCfgLen) && ::isspace(line[dwCfgLen]))
				{
					char* l = &line[dwCfgLen+1];
					while (::isspace(*l)) ++l;
					char* k = l + strlen(l) - 1;
					while (::isspace(*k)) --k;
					++k;
					value = new(std::nothrow) char[k - l + 1];
					if (value)
					{
						::strncpy(value,l,k - l);
						value[k-l] = '\0';
					}
					goto parseCfgEnd;
				}

				if ((line = endl) == NULL)
					break;
			}

			if (dwBytes == 0)
				break;
			dwAvail = sizeof(buffer);
			startp = buffer;
	parseCfgContinue:
			continue;
		}
	parseCfgEnd:
		::CloseHandle(hCfgFile);
	}
	return value;
}

// script_module_path
char* GetScriptPaths(void)
{
	char* game = GetCfgFromFile("game", "cam.cfg");
	if (!game)
		return NULL;
	char* cfg = reinterpret_cast<char*>(::alloca(24 + ::strlen(game)));
	::strcpy(cfg,game);
	::strcat(cfg,"_include_install_cfg");
	delete[] game;
	char* cfgfile = GetCfgFromFile(cfg, "cam.cfg");
	if (!cfgfile)
		return NULL;

	char* paths = GetCfgFromFile("script_module_path", cfgfile);
	delete[] cfgfile;

	return paths;
}

char* FindScriptModule(const char* pszName, const char* pszPaths)
{
	char filepath[160];
	if (!pszPaths || !*pszPaths)
	{
		::strcpy(filepath, pszName);
		if (!::strchr(filepath, '.'))
		{
			::strcat(filepath, ".osm");
		}
		if (::GetFileAttributesA(filepath) != 0xFFFFFFFFUL)
		{
			char* ret = new(std::nothrow) char[::strlen(filepath)+1];
			if (ret)
				::strcpy(ret,filepath);
			return ret;
		}
		return NULL;
	}
	char* scrpaths = reinterpret_cast<char*>(::alloca(::strlen(pszPaths) + 1));
	::strcpy(scrpaths,pszPaths);
	char* tok = scrpaths;
	for (char* path = strsep(&tok, "+"); path; path = strsep(&tok, "+"))
	{
		while (::isspace(*path)) ++path;
		if (!*path)
			continue;
		::strcpy(filepath, path);
		::strcat(filepath, "\\");
		::strcat(filepath, pszName);
		if (!::strchr(pszName, '.'))
			::strcat(filepath, ".osm");
		if (::GetFileAttributesA(filepath) != 0xFFFFFFFFUL)
		{
			char* ret = new(std::nothrow) char[::strlen(filepath)+1];
			if (ret)
				::strcpy(ret,filepath);
			return ret;
		}
	}
	return NULL;
}

bool CheckFileVersion(const char* pszFile, ulong dwVersHigh, ulong dwVersLow)
{
	DWORD z;
	unsigned int len = ::GetFileVersionInfoSizeA(const_cast<LPSTR>(pszFile), &z);
	if (len)
	{
		char* buffer = reinterpret_cast<char*>(::alloca(len));
		VS_FIXEDFILEINFO* pFileVers;
		::GetFileVersionInfoA(const_cast<LPSTR>(pszFile), z, len, reinterpret_cast<void*>(buffer));
		len = 0;
		::VerQueryValueA(reinterpret_cast<void*>(buffer), "\\", reinterpret_cast<void**>(&pFileVers), &len);
		if (len > 0)
		{
			if ( (pFileVers->dwFileVersionMS > dwVersHigh)
			  || (pFileVers->dwFileVersionMS == dwVersHigh
			   && pFileVers->dwFileVersionLS >= dwVersLow)
			)
				return true;
		}
	}
	return false;
}

void DoSuccess(int iObjId)
{
	RelayCompatible("TurnOn", iObjId);
	SInterface<IObjectSystem> pOS(g_pScriptManager);
	pOS->Destroy(iObjId);
}

void DoFailure(int iObjId)
{
	RelayCompatible("TurnOff", iObjId);
	//m_iTextType = ShowBookCompatible(iObjId, 0x7FFFFFFFUL);
	g_pScriptManager->SetTimedMessage2(iObjId, "ErrorText", 288, kSTM_OneShot, 0);
	//SInterface<IObjectSystem> pOS(g_pScriptManager);
	//pOS->Destroy(iObjId);
}

/* IPropertySrv isn't the same in all game versions,
 * nor is the property name always the same.
 * So let's try to even out those differences, even
 * if it is more awkward.
 */
char* GetObjectParamsCompatible(int iObjId)
{
	SInterface<IPropertyManager> pPM(g_pScriptManager);
	SInterface<IStringProperty> pProp = static_cast<IStringProperty*>(pPM->GetPropertyNamed("DesignNote"));
	if (-1 == pProp->GetID())
	{
		pProp.reset(static_cast<IStringProperty*>(pPM->GetPropertyNamed("ObjList")));
		if (-1 == pProp->GetID())
			return NULL;
	}

	if (!pProp->IsRelevant(iObjId))
	{
		return NULL;
	}

	char* pRet = NULL;
	const char* pszValue;
	pProp->Get(iObjId, &pszValue);
	if (pszValue)
	{
		pRet = new(std::nothrow) char[::strlen(pszValue)+1];
		if (pRet)
			::strcpy(pRet, pszValue);
	}
	return pRet;
}

// And because Thief 1 has a different call signature with no way around it...
// Script modules aren't allowed to create their own cScrMsg objects, or this
// would be much easier.
static void DoPostMessage(int iSrc, int iDest, const char* pszMsg)
{
#ifdef __GNUC__
	asm("push edi\n"
	"\tmov edi,esp\n"
	"\tsub esp,0x28\n"
	"\tlea eax,[edi-0x8]\n"
	"\tmov dword ptr [edi-0x8],0x0\n"
	"\tmov dword ptr [edi-0x4],0x0\n"
	"\tmov dword ptr [esp+0x1C],0x8\n"
	"\tmov dword ptr [esp+0x18],eax\n"
	"\tmov dword ptr [esp+0x14],eax\n"
	"\tmov dword ptr [esp+0x10],eax\n"
	"\tmov eax,%0\n"
	"\tmov ecx,%3\n"
	"\tmov edx,dword ptr [eax]\n"
	"\tmov dword ptr [esp+0xC],ecx\n"
	"\tmov ecx,%2\n"
	"\tmov dword ptr [esp+0x8],ecx\n"
	"\tmov ecx,%1\n"
	"\tmov dword ptr [esp+0x4],ecx\n"
	"\tmov dword ptr [esp],eax\n"
	"\tcall dword ptr [edx+0x6C]\n"
	"\tmov esp,edi\n"
	"\tpop edi\n"
	:: "m"(g_pScriptManager), "g"(iSrc), "g"(iDest), "g"(pszMsg)
	);
#else
	_asm {
		push edi
		mov  edi,esp
		sub  esp,0x28
		lea  eax,[edi-0x8]
		mov  dword ptr [edi-0x8],0x0
		mov  dword ptr [edi-0x4],0x0
		mov  dword ptr [esp+0x1C],kScrMsgPostToOwner
		mov  dword ptr [esp+0x18],eax
		mov  dword ptr [esp+0x14],eax
		mov  dword ptr [esp+0x10],eax
		mov  eax,g_pScriptManager
		mov  ecx,pszMsg
		mov  edx,dword ptr [eax]
		mov  dword ptr [esp+0xC],ecx
		mov  ecx,dword ptr iDest
		mov  dword ptr [esp+0x8],ecx
		mov  ecx,dword ptr iSrc
		mov  dword ptr [esp+0x4],ecx
		mov  dword ptr [esp],eax
		call dword ptr [edx+0x6C]
		mov  esp,edi
		pop  edi
	}
#endif
}

void RelayCompatible(const char* pszMsg, int iObjId)
{
	SInterface<ILinkManager> pLM(g_pScriptManager);
	SInterface<IRelation> pRel = pLM->GetRelationNamed("ControlDevice");
	if (! pRel->GetID())
	{
		pRel.reset(pLM->GetRelationNamed("SwitchLink"));
		if (! pRel->GetID())
			return;
	}

	SInterface<ILinkQuery> pLQ = pRel->Query(iObjId, 0);
	if (!pLQ)
		return;

	for (; ! pLQ->Done(); pLQ->Next())
	{
		sLink sl;
		pLQ->Link(&sl);
		//g_pScriptManager->PostMessage2(iObjId, sl.dest, pszMsg, 0, 0, 0);
		DoPostMessage(iObjId, sl.dest, pszMsg);
	}
}

int ShowBookCompatible(int iObjId, unsigned long ulTime)
{
	SInterface<IPropertyManager> pPM(g_pScriptManager);
	SInterface<IStringProperty> pBookProp = static_cast<IStringProperty*>(pPM->GetPropertyNamed("Book"));
	if (-1 == pBookProp->GetID())
	{
		// Must be SShock2
		pBookProp.reset(static_cast<IStringProperty*>(pPM->GetPropertyNamed("UseMsg")));
		if (-1 != pBookProp->GetID()
		 && pBookProp->IsRelevant(iObjId))
		{
			const char* pszBook;
			pBookProp->Get(iObjId, &pszBook);
			SService<IShockGameSrv> pSGS(g_pScriptManager);
			pSGS->TlucTextAdd(pszBook, "error", -1);
			//pSGS->AddTranslatableText(pszBook, "error", 0, ulTime);
			return 2;
		}
		return 0;
	}

	if (pBookProp->IsRelevant(iObjId))
	{
		const char* pszBook;
		pBookProp->Get(iObjId, &pszBook);

		SInterface<IStringProperty> pArtProp = static_cast<IStringProperty*>(pPM->GetPropertyNamed("BookArt"));
		if (-1 != pArtProp->GetID()
		 && pArtProp->IsRelevant(iObjId))
		{
			const char* pszBookArt;
			pArtProp->Get(iObjId, &pszBookArt);
			SService<IDarkUISrv> pUI(g_pScriptManager);
			pUI->ReadBook(pszBook, pszBookArt);
			return 0;
		}

		SService<IDataSrv> pDS(g_pScriptManager);
		char* szBookFile = reinterpret_cast<char*>(::alloca(10 + ::strlen(pszBook)));
		::strcpy(szBookFile, "..\\books\\");
		::strcat(szBookFile, pszBook);
		cScrStr strText;
		pDS->GetString(strText, szBookFile, "page_0", "", "strings");
		if (!strText.IsEmpty())
		{
			SService<IDarkUISrv> pUI(g_pScriptManager);
			pUI->TextMessage(strText, 0, ulTime);
		}
		strText.Free();
		return 1;
	}
	return 0;
}

