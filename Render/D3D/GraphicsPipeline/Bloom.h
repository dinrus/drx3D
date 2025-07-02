// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CBloomStage : public CGraphicsPipelineStage
{
public:
	void Execute();

private:
	CFullscreenPass m_pass1H;
	CFullscreenPass m_pass1V;
	CFullscreenPass m_pass2H;
	CFullscreenPass m_pass2V;
};
