// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBinding.h
//  Version:     v1.00
//  Created:     9/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptBinding_h__
#define __ScriptBinding_h__
#pragma once

class CScriptableBase;

//////////////////////////////////////////////////////////////////////////
class CScriptBindings
{
public:
	CScriptBindings();
	virtual ~CScriptBindings();

	void         Init(ISystem* pSystem, IScriptSystem* pSS);
	void         Done();

	void         LoadScriptedSurfaceTypes(tukk sFolder, bool bReload);
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) const;

private:
	std::vector<std::unique_ptr<CScriptableBase>> m_binds;

	class CScriptSurfaceTypesLoader*              m_pScriptSurfaceTypes;
};

#endif // __ScriptBinding_h__
