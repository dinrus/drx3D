// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class COmniCameraStage : public CGraphicsPipelineStage
{
public:
	COmniCameraStage() = default;

	void Execute();
	bool IsEnabled() const;

protected:
	CTexture* m_pOmniCameraTexture = nullptr;
	CTexture* m_pOmniCameraCubeFaceStagingTexture = nullptr;

	CFullscreenPass m_cubemapToScreenPass;
	CDownsamplePass m_downsamplePass;
};
