// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CMobileCompositionStage : public CGraphicsPipelineStage
{
public:
	void Init() final;
	void Execute();

private:
	CDepthDownsamplePass m_passDepthDownsample2;
	CDepthDownsamplePass m_passDepthDownsample4;
	CDepthDownsamplePass m_passDepthDownsample8;
	
	CFullscreenPass m_passLighting;
	CFullscreenPass m_passTonemappingTAA;
};