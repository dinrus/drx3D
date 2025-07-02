// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MFXFlowGraphEffect.h
//  Version:     v1.00
//  Created:     29/11/2006 by AlexL-Benito GR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __MFXFLOWGRAPHEFFECT_H__
#define __MFXFLOWGRAPHEFFECT_H__

#pragma once

#include "MFXEffectBase.h"

struct SMFXFlowGraphParams
{
	static i32k MAX_CUSTOM_PARAMS = 4;
	string           fgName;
	float            maxdistSq; // max distance (squared) for spawning this effect
	float            params[MAX_CUSTOM_PARAMS];

	SMFXFlowGraphParams()
	{
		maxdistSq = 0.0f;
		memset(&params, 0, sizeof(params));
	}
};

class CMFXFlowGraphEffect :
	public CMFXEffectBase
{
public:
	CMFXFlowGraphEffect();
	virtual ~CMFXFlowGraphEffect();

	//IMFXEffect
	virtual void Execute(const SMFXRunTimeEffectParams& params) override;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) override;
	virtual void SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue) override;
	virtual void GetResources(SMFXResourceList& resourceList) const override;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;
	//~IMFXEffect

private:
	SMFXFlowGraphParams m_flowGraphParams;
};

#endif
