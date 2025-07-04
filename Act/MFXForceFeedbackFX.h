// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MFXForceFeedbackEffect.h
//  Version:     v1.00
//  Created:     8/4/2010 by Benito Gangoso Rodriguez
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __MFXFORCEFEEDBACKEFFECT_H__
	#define __MFXFORCEFEEDBACKEFFECT_H__

	#include "MFXEffectBase.h"

struct SMFXForceFeedbackParams
{
	string forceFeedbackEventName;
	float  intensityFallOffMinDistanceSqr;
	float  intensityFallOffMaxDistanceSqr;

	SMFXForceFeedbackParams()
		: intensityFallOffMinDistanceSqr(0.0f)
		, intensityFallOffMaxDistanceSqr(25.0f)
	{

	}
};

class CMFXForceFeedbackEffect :
	public CMFXEffectBase
{
public:
	CMFXForceFeedbackEffect();
	virtual ~CMFXForceFeedbackEffect();

	//IMFXEffect
	virtual void Execute(const SMFXRunTimeEffectParams& params) override;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) override;
	virtual void GetResources(SMFXResourceList& resourceList) const override;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;
	//~IMFXEffect

protected:
	SMFXForceFeedbackParams m_forceFeedbackParams;
};

#endif
