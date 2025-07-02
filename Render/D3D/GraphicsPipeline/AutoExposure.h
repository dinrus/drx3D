// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CAutoExposureStage : public CGraphicsPipelineStage
{
public:
	void Execute();

private:
	void MeasureLuminance();
	void AdjustExposure();

private:
	CFullscreenPass m_passLuminanceInitial;
	CFullscreenPass m_passLuminanceIteration[NUM_HDR_TONEMAP_TEXTURES];
	CFullscreenPass m_passAutoExposure;
};
