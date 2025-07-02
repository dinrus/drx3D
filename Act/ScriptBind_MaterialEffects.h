// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_MaterialEffects.cpp
//  Version:     v1.00
//  Created:     03/22/2007 by MichaelR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: MaterialEffects ScriptBind
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __SCRIPTBIND_MaterialEffects_H__
#define __SCRIPTBIND_MaterialEffects_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

class CMaterialEffects;

class CScriptBind_MaterialEffects : public CScriptableBase
{
public:
	CScriptBind_MaterialEffects(ISystem* pSystem, CMaterialEffects* pDS);
	virtual ~CScriptBind_MaterialEffects();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	void RegisterGlobals();
	void RegisterMethods();

	i32  GetEffectId(IFunctionHandler* pH, tukk customName, i32 surfaceIndex2);
	i32  GetEffectIdByLibName(IFunctionHandler* pH, tukk LibName, tukk FGFXName);
	i32  PrintEffectIdByMatIndex(IFunctionHandler* pH, i32 materialIndex1, i32 materialIndex2);
	i32  ExecuteEffect(IFunctionHandler* pH, i32 effectId, SmartScriptTable paramsTable);

private:
	ISystem*          m_pSystem;
	CMaterialEffects* m_pMFX;
};

#endif //__SCRIPTBIND_MaterialEffects_H__
