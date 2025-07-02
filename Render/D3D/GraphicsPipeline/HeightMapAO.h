// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CHeightMapAOStage : public CGraphicsPipelineStage
{
public:
	void Update() final;
	void Execute();

	bool IsValid() const { return m_bHeightMapAOExecuted; }

	const ShadowMapFrustum* GetHeightMapFrustum   () const { DRX_ASSERT(m_bHeightMapAOExecuted); return m_pHeightMapFrustum; }
	CTexture*         GetHeightMapAOScreenDepthTex() const { DRX_ASSERT(m_bHeightMapAOExecuted); return m_pHeightMapAOScreenDepthTex; }
	CTexture*         GetHeightMapAOTex           () const { DRX_ASSERT(m_bHeightMapAOExecuted); return m_pHeightMapAOTex; }

private:
	CFullscreenPass m_passSampling;
	CFullscreenPass m_passSmoothing;
	CMipmapGenPass  m_passMipmapGen;

	bool                    m_bHeightMapAOExecuted = false;
	const ShadowMapFrustum* m_pHeightMapFrustum;
	CTexture*               m_pHeightMapAOScreenDepthTex;
	CTexture*               m_pHeightMapAOTex;
};
