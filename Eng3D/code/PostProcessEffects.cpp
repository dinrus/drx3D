// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   PostProcessEffects.cpp : 3d engine post processing acess/scripts interfaces

   Revision история:
* 23/02/2005: Re-factored/Converted to drx3D 2.0 by Tiago Sousa
* Created by Tiago Sousa

   Notes:
* Check PostEffects.h for list of available effects

   =============================================================================*/

#include <drx3D/Eng3D/StdAfx.h>

void C3DEngine::SetPostEffectParam(tukk pParam, float fValue, bool bForceValue) const
{
	if (pParam && GetRenderer())
		GetRenderer()->EF_SetPostEffectParam(pParam, fValue, bForceValue);
}

void C3DEngine::GetPostEffectParam(tukk pParam, float& fValue) const
{
	if (pParam && GetRenderer())
		GetRenderer()->EF_GetPostEffectParam(pParam, fValue);
}

void C3DEngine::SetPostEffectParamVec4(tukk pParam, const Vec4& pValue, bool bForceValue) const
{
	if (pParam && GetRenderer())
		GetRenderer()->EF_SetPostEffectParamVec4(pParam, pValue, bForceValue);
}

void C3DEngine::GetPostEffectParamVec4(tukk pParam, Vec4& pValue) const
{
	if (pParam && GetRenderer())
		GetRenderer()->EF_GetPostEffectParamVec4(pParam, pValue);
}

void C3DEngine::SetPostEffectParamString(tukk pParam, tukk pszArg) const
{
	if (pParam && pszArg && GetRenderer())
		GetRenderer()->EF_SetPostEffectParamString(pParam, pszArg);
}

void C3DEngine::GetPostEffectParamString(tukk pParam, tukk & pszArg) const
{
	if (pParam && GetRenderer())
		GetRenderer()->EF_GetPostEffectParamString(pParam, pszArg);
}

i32 C3DEngine::GetPostEffectID(tukk pPostEffectName)
{
	return GetRenderer() ? GetRenderer()->EF_GetPostEffectID(pPostEffectName) : 0;
}

void C3DEngine::ResetPostEffects(bool bOnSpecChange) const
{
	if (GetRenderer())
	{
		GetRenderer()->EF_ResetPostEffects(bOnSpecChange);
	}
}
