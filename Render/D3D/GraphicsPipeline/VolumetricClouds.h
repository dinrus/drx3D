// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/ComputeRenderPass.h>

class CVolumetricCloudsStage : public CGraphicsPipelineStage
{
public:
	static bool IsRenderable();
	static Vec4 GetVolumetricCloudShadowParams(const CCamera&, const Vec2& windOffset, const Vec2& vTiling);

public:
	CVolumetricCloudsStage();
	virtual ~CVolumetricCloudsStage();

	void Init() final;
	void Update() final;

	void ExecuteShadowGen();
	void Execute();

private:
	void  ExecuteVolumetricCloudShadowGen();
	void  GenerateCloudShadowGenShaderParam(const Vec3& texSize);

	void  ExecuteComputeDensityAndShadow(const struct VCCloudRenderContext& context);
	void  ExecuteRenderClouds(const struct VCCloudRenderContext& context);
	void  GenerateCloudShaderParam(struct VCCloudRenderContext& context);

	i32 GetBufferIndex(i32k gpuCount, bool bStereoMultiGPURendering) const;
	i32 GetCurrentFrameIndex() const;
	i32 GetPreviousFrameIndex(i32k gpuCount, bool bStereoMultiGPURendering) const;

	bool  AreTexturesValid() const;

	void  GenerateCloudBlockerList();
	void  GenerateCloudBlockerSSList();

private:
	static i32k   MaxFrameNum = 4;
	static u32k  MaxEyeNum = 2;

	_smart_ptr<CTexture> m_pDownscaledMaxTex[MaxEyeNum][2];
	_smart_ptr<CTexture> m_pDownscaledMinTex[MaxEyeNum][2];
	_smart_ptr<CTexture> m_pScaledPrevDepthTex[MaxEyeNum];
	_smart_ptr<CTexture> m_pCloudDepthTex;
	_smart_ptr<CTexture> m_pDownscaledMaxTempTex;
	_smart_ptr<CTexture> m_pDownscaledMinTempTex;
	_smart_ptr<CTexture> m_pDownscaledLeftEyeTex;
	_smart_ptr<CTexture> m_pCloudDensityTex;
	_smart_ptr<CTexture> m_pCloudShadowTex;

	_smart_ptr<CTexture> m_pCloudMiePhaseFuncTex;
	_smart_ptr<CTexture> m_pNoiseTex;

	_smart_ptr<CTexture> m_pVolCloudTex;
	_smart_ptr<CTexture> m_pVolCloudNoiseTex;
	_smart_ptr<CTexture> m_pVolCloudEdgeNoiseTex;

	CComputeRenderPass   m_passGenerateCloudShadow;
	CComputeRenderPass   m_passComputeDensityAndShadow[MaxEyeNum];
	CComputeRenderPass   m_passRenderClouds[MaxEyeNum];
	CFullscreenPass      m_passTemporalReprojectionDepthMax[MaxEyeNum][2];
	CFullscreenPass      m_passTemporalReprojectionDepthMin[MaxEyeNum][2];
	CFullscreenPass      m_passUpscale[MaxEyeNum][2];

	CConstantBufferPtr   m_pCloudShadowConstantBuffer;
	CConstantBufferPtr   m_pRenderCloudConstantBuffer;
	CConstantBufferPtr   m_pReprojectionConstantBuffer;

	Matrix44             m_viewMatrix[MaxEyeNum][MaxFrameNum];
	Matrix44             m_projMatrix[MaxEyeNum][MaxFrameNum];
	int64                m_nUpdateFrameID[MaxEyeNum];
	i32                m_cleared;
	i32                m_tick;

	TArray<Vec4>         m_blockerPosArray;
	TArray<Vec4>         m_blockerParamArray;
	TArray<Vec4>         m_blockerSSPosArray;
	TArray<Vec4>         m_blockerSSParamArray;
};
