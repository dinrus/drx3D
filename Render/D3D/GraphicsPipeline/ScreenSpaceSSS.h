// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

// Screen Space Subsurface Scattering
class CScreenSpaceSSSStage : public CGraphicsPipelineStage
{
public:
	void Execute(CTexture* pIrradianceTex);

private:
	CFullscreenPass m_passH;
	CFullscreenPass m_passV;
};
