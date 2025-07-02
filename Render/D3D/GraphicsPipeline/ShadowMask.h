// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/StandardGraphicsPipeline.h>

namespace ShadowMaskInternal
{
	class CSunShadows;
	class CLocalLightShadows;
}

class CShadowMaskStage : public CGraphicsPipelineStage
{
	friend class ShadowMaskInternal::CSunShadows;
	friend class ShadowMaskInternal::CLocalLightShadows;

public:
	CShadowMaskStage();

	void Init() final;
	void Prepare();

	void Execute();

	void OnCVarsChanged(const CCVarUpdateRecorder& cvarUpdater) final;

private:
	std::unique_ptr<ShadowMaskInternal::CSunShadows>        m_pSunShadows;
	std::unique_ptr<ShadowMaskInternal::CLocalLightShadows> m_pLocalLightShadows;

	std::vector<CPrimitiveRenderPass>                       m_maskGenPasses;
	CPrimitiveRenderPass                                    m_debugCascadesPass;

	CTexture*                                               m_pShadowMaskRT;
	CConstantBufferPtr                                      m_pPerViewConstantBuffer;

	SRenderViewInfo                                         m_viewInfo[2];
	size_t                                                  m_viewInfoCount;

	i32                                                     m_sunShadowPrimitives;
	i32                                                     m_localLightPrimitives;
};
