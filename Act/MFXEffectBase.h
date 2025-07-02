// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ----------------------------------------------------------------------------------------
//  Имя файла:   MFXEffectBase.h
//  Описание: Basic and partial implemenation of IMFXEffect which serves as a base for concrete implementations
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MFX_EFFECT_BASE_H_
#define _MFX_EFFECT_BASE_H_

#pragma once

#include <drx3D/CoreX/DrxFlags.h>
#include "IMFXEffect.h"

class CMFXEffectBase : public IMFXEffect
{
public:

	CMFXEffectBase(u16k _typeFilterFlag);

	//IMFXEffect (partial implementation)
	virtual void SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue) override;
	virtual void PreLoadAssets() override;
	virtual void ReleasePreLoadAssets() override;
	//~IMFXEffect

	bool CanExecute(const SMFXRunTimeEffectParams& params) const;

private:
	CDrxFlags<u16> m_runtimeExecutionFilter;
};

typedef _smart_ptr<CMFXEffectBase> TMFXEffectBasePtr;

#endif // _MFX_EFFECT_BASE_H_
