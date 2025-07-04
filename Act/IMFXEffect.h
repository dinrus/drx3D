// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IMFXEffect.h
//  Version:     v1.00
//  Created:     28/11/2006 by JohnN/AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Virtual base class for all derived effects
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __IMFXEFFECT_H__
#define __IMFXEFFECT_H__

#pragma once

#include <drx3D/Act/IMaterialEffects.h>

typedef string TMFXNameId; // we use strings, and with clever assigning we minimize duplicates

struct IMFXEffect : public _reference_target_t
{
	virtual ~IMFXEffect() {};

	virtual void Execute(const SMFXRunTimeEffectParams& params) = 0;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) = 0;
	virtual void SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue) = 0;
	virtual void GetResources(SMFXResourceList& resourceList) const = 0;
	virtual void PreLoadAssets() = 0;
	virtual void ReleasePreLoadAssets() = 0;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

#endif
