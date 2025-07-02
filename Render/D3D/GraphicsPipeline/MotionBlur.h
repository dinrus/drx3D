// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CMotionBlurStage : public CGraphicsPipelineStage
{
public:
	void Execute();

private:
	float ComputeMotionScale();

private:
	CFullscreenPass    m_passPacking;
	CFullscreenPass    m_passTileGen1;
	CFullscreenPass    m_passTileGen2;
	CFullscreenPass    m_passNeighborMax;
	CStretchRectPass   m_passCopy;
	CFullscreenPass    m_passMotionBlur;
};
