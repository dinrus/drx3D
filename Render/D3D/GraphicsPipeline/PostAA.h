// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CPostAAStage : public CGraphicsPipelineStage
{
public:
	// Width and height should be the actual dimensions of the texture to be antialiased, 
	// which might be different than the provided renderview's output resolution (e.g. VR).
	void CalculateJitterOffsets(i32 targetWidth, i32 targetHeight, CRenderView* pTargetRenderView);
	void CalculateJitterOffsets(CRenderView* pRenderView)
	{
		const int32_t w = pRenderView->GetRenderResolution()[0];
		const int32_t h = pRenderView->GetRenderResolution()[1];
		DRX_ASSERT(w > 0 && h > 0);
		if (w > 0 && h > 0)
			CalculateJitterOffsets(w, h, pRenderView);
	}

	void Resize(i32 renderWidth, i32 renderHeight) override;
	void Update() override;
	void Init() override;
	void Execute();

private:
	void ApplySMAA(CTexture*& pCurrRT);
	void ApplyTemporalAA(CTexture*& pCurrRT, CTexture*& pMgpuRT, u32 aaMode);
	void DoFinalComposition(CTexture*& pCurrRT, CTexture* pDestRT, u32 aaMode);

	CTexture* GetAARenderTarget(const CRenderView* pRenderView, bool bCurrentFrame) const;

private:
	_smart_ptr<CTexture> m_pTexAreaSMAA;
	_smart_ptr<CTexture> m_pTexSearchSMAA;
	CTexture*            m_pPrevBackBuffersLeftEye[2];
	CTexture*            m_pPrevBackBuffersRightEye[2];
	i32                  m_lastFrameID;

	CFullscreenPass      m_passSMAAEdgeDetection;
	CFullscreenPass      m_passSMAABlendWeights;
	CFullscreenPass      m_passSMAANeighborhoodBlending;
	CFullscreenPass      m_passTemporalAA;
	CFullscreenPass      m_passComposition;

	bool oldStereoEnabledState{ false };
	i32  oldAAState{ 0 };
};
