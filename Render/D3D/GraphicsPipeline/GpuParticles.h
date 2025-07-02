// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>

class CComputeParticlesStage : public CGraphicsPipelineStage
{
public:
	CComputeParticlesStage();
	~CComputeParticlesStage();

	void Init() final;

	void Update() override;
	void PreDraw();
	void PostDraw();

	gpu_pfx2::CUpr* GetGpuParticleUpr() { return m_pGpuParticleUpr.get(); }
private:
	std::unique_ptr<gpu_pfx2::CUpr> m_pGpuParticleUpr;
	i32 m_oldFrameIdExecute;
	i32 m_oldFrameIdPreDraw;
	i32 m_oldFrameIdPostDraw;
};
