/******************************************************************************
 *  scriptvars.h
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

#ifndef SCRIPTVARS_H
#define SCRIPTVARS_H

#include <cstring>
#include <lg/types.h>
#include <lg/scrmanagers.h>

extern IScriptMan* g_pScriptManager;
extern IMalloc* g_pMalloc;


#define SCRIPT_VAR(className,varName) varName(#className, #varName)
#define SCRIPT_VAROBJ(className,varName, objId) varName(#className, #varName, objId)


class script_var
{
protected:
	bool		m_bDynName;
	sScrDatumTag	m_tag;

public:
	~script_var()
	{
		if (m_bDynName)
			delete[] m_tag.pszName;
	}
	script_var()
	{
		m_bDynName = false;
	}
	script_var(const char* pszScriptName, const char* pszVarName)
	{
		m_bDynName = false;
		m_tag.pszClass = pszScriptName;
		m_tag.pszName = pszVarName;
	}
	script_var(const char* pszScriptName, const char* pszVarName, int iObj)
	{
		m_bDynName = false;
		m_tag.pszClass = pszScriptName;
		m_tag.pszName = pszVarName;
		m_tag.objId = iObj;
	}

	void SetObj(int iObj)
	{
		m_tag.objId = iObj;
	}

	void SetDynTag(const char* pszDynName)
	{
		char* pszNew = new char[::strlen(pszDynName)+1];
		if (pszNew)
		{
			::strcpy(pszNew, pszDynName);
			if (m_bDynName)
				delete[] m_tag.pszName;
			m_tag.pszName = pszNew;
			m_bDynName = true;
		}
	}
	void Clear()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->ClearScriptData(&m_tag, &param);
	}
	bool Valid()
	{
		return g_pScriptManager->IsScriptDataSet(&m_tag);
	}
	script_var& operator= (const script_var& rCpy)
	{
		if (m_bDynName)
		{
			delete[] m_tag.pszName;
			m_bDynName = false;
		}
		m_tag = rCpy.m_tag;
		if (rCpy.m_bDynName)
		{
			SetDynTag(rCpy.m_tag.pszName);
		}
		return *this;
	}
};


class script_int : public script_var
{
public:
	script_int() : script_var()
	{ }
	script_int(const char* pszScriptName, const char* pszVarName)
		: script_var(pszScriptName,pszVarName)
	{ }
	script_int(const char* pszScriptName, const char* pszVarName, int iObj)
		: script_var(pszScriptName,pszVarName,iObj)
	{ }

	void Init(int iVal = 0)
	{
		if (!g_pScriptManager->IsScriptDataSet(&m_tag))
			*this = iVal;
	}

	operator int()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->GetScriptData(&m_tag, &param);
		return param.i;
	}
	script_int& operator= (int iVal)
	{
		sMultiParm param;
		param.type = kMT_Int;
		param.i = iVal;
		g_pScriptManager->SetScriptData(&m_tag, &param);
		return *this;
	}
	script_int& operator+= (int iVal)
	{
		return *this = *this + iVal;
	}
	script_int& operator-= (int iVal)
	{
		return *this = *this - iVal;
	}
	script_int& operator++ ()
	{
		return *this = *this + 1;
	}
	script_int operator++ (int)
	{
		script_int __tmp = *this;
		*this = *this + 1;
		return __tmp;
	}
	script_int& operator-- ()
	{
		return *this = *this - 1;
	}
	script_int operator-- (int)
	{
		script_int __tmp = *this;
		*this = *this - 1;
		return __tmp;
	}
};


class script_float : public script_var
{
public:
	script_float() : script_var()
	{ }
	script_float(const char* pszScriptName, const char* pszVarName)
		: script_var(pszScriptName,pszVarName)
	{ }
	script_float(const char* pszScriptName, const char* pszVarName, int iObj)
		: script_var(pszScriptName,pszVarName,iObj)
	{ }

	void Init(float fVal = 0)
	{
		if (!g_pScriptManager->IsScriptDataSet(&m_tag))
			*this = fVal;
	}

	operator float()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->GetScriptData(&m_tag, &param);
		return param.f;
	}
	script_float& operator= (float fVal)
	{
		sMultiParm param;
		param.type = kMT_Float;
		param.f = fVal;
		g_pScriptManager->SetScriptData(&m_tag, &param);
		return *this;
	}
	script_float& operator+= (float fVal)
	{
		return *this = *this + fVal;
	}
	script_float& operator-= (float fVal)
	{
		return *this = *this - fVal;
	}
	script_float& operator++ ()
	{
		return *this = *this + 1;
	}
	script_float operator++ (int)
	{
		script_float __tmp = *this;
		*this = *this + 1;
		return __tmp;
	}
	script_float& operator-- ()
	{
		return *this = *this - 1;
	}
	script_float operator-- (int)
	{
		script_float __tmp = *this;
		*this = *this - 1;
		return __tmp;
	}
};


class script_str : public script_var
{
	cScrStr m_szVal;

public:
	~script_str()
	{
		m_szVal.Free();
	}
	script_str() : script_var(), m_szVal()
	{ }
	script_str(const char* pszScriptName, const char* pszVarName)
		: script_var(pszScriptName,pszVarName), m_szVal()
	{ }
	script_str(const char* pszScriptName, const char* pszVarName, int iObj)
		: script_var(pszScriptName,pszVarName,iObj), m_szVal()
	{ }

	void Init(const char* pszVal = NULL)
	{
		if (!g_pScriptManager->IsScriptDataSet(&m_tag))
		{
			*this = pszVal;
		}
	}

	operator const char*()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->GetScriptData(&m_tag, &param);
		m_szVal.Free();
		m_szVal = param.psz;
		return m_szVal;
	}

	script_str& operator= (const char* pszVal)
	{
		sMultiParm param;
		param.type = kMT_String;
		param.psz = const_cast<char*>(IF_NOT(pszVal,""));
		g_pScriptManager->SetScriptData(&m_tag, &param);
		return *this;
	}

	void Clear()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->ClearScriptData(&m_tag, &param);
		if (param.type != kMT_Undef)
			g_pMalloc->Free(param.psz);
	}
};


class script_vec : public script_var
{
	cScrVec m_vVal;

public:
	script_vec() : script_var(), m_vVal()
	{ }
	script_vec(const char* pszScriptName, const char* pszVarName)
		: script_var(pszScriptName,pszVarName), m_vVal()
	{ }
	script_vec(const char* pszScriptName, const char* pszVarName, int iObj)
		: script_var(pszScriptName,pszVarName,iObj), m_vVal()
	{ }

	void Init(const mxs_vector* pvVal = NULL)
	{
		if (!g_pScriptManager->IsScriptDataSet(&m_tag))
		{
			*this = pvVal;
		}
	}

	operator const mxs_vector*()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->GetScriptData(&m_tag, &param);
		m_vVal = *param.pVector;
		g_pMalloc->Free(param.pVector);
		return &m_vVal;
	}

	script_vec& operator= (const mxs_vector* pvVal)
	{
		cScrVec v;
		if (pvVal)
			v = *pvVal;
		sMultiParm param;
		param.type = kMT_Vector;
		param.pVector = &v;
		g_pScriptManager->SetScriptData(&m_tag, &param);
		return *this;
	}

	void Clear()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->ClearScriptData(&m_tag, &param);
		if (param.type != kMT_Undef)
			g_pMalloc->Free(param.pVector);
	}
};


template<typename _Type>
class script_handle : public script_var
{
public:
	script_handle() : script_var()
	{ }
	script_handle(const char* pszScriptName, const char* pszVarName)
		: script_var(pszScriptName,pszVarName)
	{ }
	script_handle(const char* pszScriptName, const char* pszVarName, int iObj)
		: script_var(pszScriptName,pszVarName,iObj)
	{ }

	// gcc is being a snotty bitch over type casting.
	// so I figure that we want to initialize handles to NULL anyway
	void Init()
	{
		if (!g_pScriptManager->IsScriptDataSet(&m_tag))
		{
			sMultiParm param;
			param.type = kMT_Int;
			param.i = 0;
			g_pScriptManager->SetScriptData(&m_tag, &param);
		}
	}

	operator _Type()
	{
		sMultiParm param;
		param.type = kMT_Undef;
		g_pScriptManager->GetScriptData(&m_tag, &param);
		return reinterpret_cast<_Type>(param.i);
	}
	script_handle<_Type>& operator= (_Type hVal)
	{
		sMultiParm param;
		param.type = kMT_Int;
		param.i = reinterpret_cast<int>(hVal);
		g_pScriptManager->SetScriptData(&m_tag, &param);
		return *this;
	}
};


#endif // SCRIPTVARS_H
