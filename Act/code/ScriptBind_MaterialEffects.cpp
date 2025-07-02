// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_MaterialEffects.cpp
//  Version:     v1.00
//  Created:     03/22/2007 by MichaelR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: MaterialEffects ScriptBind. MaterialEffects should be used
//               from C++ if possible. Use this ScriptBind for legacy stuff.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ScriptBind_MaterialEffects.h>
#include <drx3D/Act/MaterialEffects.h>

//------------------------------------------------------------------------
CScriptBind_MaterialEffects::CScriptBind_MaterialEffects(ISystem* pSystem, CMaterialEffects* pMFX)
{
	m_pSystem = pSystem;
	m_pMFX = pMFX;
	assert(m_pMFX != 0);

	Init(gEnv->pScriptSystem, m_pSystem);
	SetGlobalName("MaterialEffects");

	RegisterGlobals();
	RegisterMethods();
}

//------------------------------------------------------------------------
CScriptBind_MaterialEffects::~CScriptBind_MaterialEffects()
{
}

//------------------------------------------------------------------------
void CScriptBind_MaterialEffects::RegisterGlobals()
{
	gEnv->pScriptSystem->SetGlobalValue("MFX_INVALID_EFFECTID", InvalidEffectId);
}

//------------------------------------------------------------------------
void CScriptBind_MaterialEffects::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_MaterialEffects::

	SCRIPT_REG_TEMPLFUNC(GetEffectId, "customName, surfaceIndex2");
	SCRIPT_REG_TEMPLFUNC(GetEffectIdByLibName, "LibName, FGFXName");
	SCRIPT_REG_TEMPLFUNC(PrintEffectIdByMatIndex, "MatName1, MatName2");
	SCRIPT_REG_TEMPLFUNC(ExecuteEffect, "effectId, paramsTable");
}

//------------------------------------------------------------------------
i32 CScriptBind_MaterialEffects::GetEffectId(IFunctionHandler* pH, tukk customName, i32 surfaceIndex2)
{
	TMFXEffectId effectId = m_pMFX->GetEffectId(customName, surfaceIndex2);

	return pH->EndFunction(effectId);
}

//------------------------------------------------------------------------
i32 CScriptBind_MaterialEffects::GetEffectIdByLibName(IFunctionHandler* pH, tukk LibName, tukk FGFXName)
{
	TMFXEffectId effectId = m_pMFX->GetEffectIdByName(LibName, FGFXName);

	return pH->EndFunction(effectId);
}

//------------------------------------------------------------------------
i32 CScriptBind_MaterialEffects::PrintEffectIdByMatIndex(IFunctionHandler* pH, i32 materialIndex1, i32 materialIndex2)
{
	TMFXEffectId effectId = m_pMFX->InternalGetEffectId(materialIndex1, materialIndex2);

	DrxLogAlways("Requested MaterialEffect ID: %u", effectId);

	return pH->EndFunction(effectId);
}

//------------------------------------------------------------------------
i32 CScriptBind_MaterialEffects::ExecuteEffect(IFunctionHandler* pH, i32 effectId, SmartScriptTable paramsTable)
{
	if (effectId == InvalidEffectId)
		return pH->EndFunction(false);

	// minimalistic implementation.. extend if you need it
	SMFXRunTimeEffectParams params;
	paramsTable->GetValue("pos", params.pos);
	paramsTable->GetValue("normal", params.normal);
	paramsTable->GetValue("scale", params.scale);
	paramsTable->GetValue("angle", params.angle);

	bool res = m_pMFX->ExecuteEffect(effectId, params);

	return pH->EndFunction(res);
}
