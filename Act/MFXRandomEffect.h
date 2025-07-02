// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MFXRandomEffect.h
//  Version:     v1.00
//  Created:     28/11/2006 by JohnN/AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Random effect (randomly plays one of its child effects)
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __MFXRANDOMEFFECT_H__
#define __MFXRANDOMEFFECT_H__

#pragma once

#include "MFXEffectBase.h"
#include "MFXContainer.h"

class CMFXRandomizerContainer
	: public CMFXContainer
{
public:
	void ExecuteRandomizedEffects(const SMFXRunTimeEffectParams& params);

private:
	TMFXEffectBasePtr ChooseCandidate(const SMFXRunTimeEffectParams& params) const;
};

class CMFXRandomEffect :
	public CMFXEffectBase
{

public:
	CMFXRandomEffect();
	virtual ~CMFXRandomEffect();

	//IMFXEffect
	virtual void Execute(const SMFXRunTimeEffectParams& params) override;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) override;
	virtual void SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue) override;
	virtual void GetResources(SMFXResourceList& resourceList) const override;
	virtual void PreLoadAssets() override;
	virtual void ReleasePreLoadAssets() override;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;
	//~IMFXEffect

private:
	CMFXRandomizerContainer m_container;
};

#endif
