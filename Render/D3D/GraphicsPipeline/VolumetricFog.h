// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/ComputeRenderPass.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/SceneRenderPass.h>

class CVolumetricFogStage : public CGraphicsPipelineStage
{
public:
	struct SForwardParams
	{
		Vec4 vfSamplingParams;
		Vec4 vfDistributionParams;
		Vec4 vfScatteringParams;
		Vec4 vfScatteringBlendParams;
		Vec4 vfScatteringColor;
		Vec4 vfScatteringSecondaryColor;
		Vec4 vfHeightDensityParams;
		Vec4 vfHeightDensityRampParams;
		Vec4 vfDistanceParams;
		Vec4 vfGlobalEnvProbeParams0;
		Vec4 vfGlobalEnvProbeParams1;
	};

public:
	static i32k ShadowCascadeNum = 3;

	static bool  IsEnabledInFrame();
	static i32 GetVolumeTextureDepthSize();
	static i32 GetVolumeTextureSize(i32 size, i32 scale);
	static float GetDepthTexcoordFromLinearDepthScaled(float linearDepthScaled, float raymarchStart, float invRaymarchDistance, float depthSlicesNum);

public:
	CVolumetricFogStage();
	virtual ~CVolumetricFogStage();

	void Init() final;
	void Update() final;

	void Execute();

	bool CreatePipelineState(const SGraphicsPipelineStateDescription& desc, CDeviceGraphicsPSOPtr& outPSO) const;

	void FillForwardParams(SForwardParams& volFogParams, bool enable = true) const;

	template<class RenderPassType>
	void        BindVolumetricFogResources(RenderPassType& pass, i32 startTexSlot, i32 trilinearClampSamplerSlot);
	const Vec4& GetGlobalEnvProbeShaderParam0() const { return m_globalEnvProbeParam0; }
	const Vec4& GetGlobalEnvProbeShaderParam1() const { return m_globalEnvProbeParam1; }
	CTexture*   GetVolumetricFogTex() const;
	CTexture*   GetGlobalEnvProbeTex0() const;
	CTexture*   GetGlobalEnvProbeTex1() const;
	void        ResetFrame();

private:
	static i32k MaxFrameNum = 4;

private:
	bool      PreparePerPassResources(bool bOnInit);

	void      ExecuteInjectParticipatingMedia(const SScopedComputeCommandList& commandList);
	void      ExecuteVolumetricFog(const SScopedComputeCommandList& commandList);

	u32    GetTemporalBufferId() const;
	CTexture* GetInscatterTex() const;
	CTexture* GetPrevInscatterTex() const;
	CTexture* GetDensityTex() const;
	CTexture* GetPrevDensityTex() const;

	void      GenerateLightList();
	void      GenerateFogVolumeList();

	bool      ReplaceShadowMapWithStaticShadowMap(CShadowUtils::SShadowCascades& shadowCascades, u32 shadowCascadeSlot) const;

	bool      IsVisible() const;
	bool      IsTexturesValid() const;
	void      UpdateFrame();
	void      ExecuteDownscaleShadowmap();
	void      ExecuteBuildLightListGrid(const SScopedComputeCommandList& commandList);
	void      ExecuteDownscaledDepth(const SScopedComputeCommandList& commandList);
	void      ExecuteInjectFogDensity(const SScopedComputeCommandList& commandList);
	void      ExecuteInjectInscatteringLight(const SScopedComputeCommandList& commandList);
	void      ExecuteBlurDensityVolume(const SScopedComputeCommandList& commandList);
	void      ExecuteBlurInscatterVolume(const SScopedComputeCommandList& commandList);
	void      ExecuteTemporalReprojection(const SScopedComputeCommandList& commandList);
	void      ExecuteRaymarchVolumetricFog(const SScopedComputeCommandList& commandList);

private:
	_smart_ptr<CTexture>     m_pVolFogBufDensityColor;
	_smart_ptr<CTexture>     m_pVolFogBufDensity;
	_smart_ptr<CTexture>     m_pVolFogBufEmissive;
	_smart_ptr<CTexture>     m_pInscatteringVolume;
	_smart_ptr<CTexture>     m_pFogInscatteringVolume[2];
	_smart_ptr<CTexture>     m_pFogDensityVolume[2];
	_smart_ptr<CTexture>     m_pMaxDepth;
	_smart_ptr<CTexture>     m_pMaxDepthTemp;
	_smart_ptr<CTexture>     m_pNoiseTexture;
	_smart_ptr<CTexture>     m_pDownscaledShadow[ShadowCascadeNum];
	_smart_ptr<CTexture>     m_pDownscaledShadowTemp;
	_smart_ptr<CTexture>     m_pCloudShadowTex;
	_smart_ptr<CTexture>     m_globalEnvProveTex0;
	_smart_ptr<CTexture>     m_globalEnvProveTex1;

	CGpuBuffer               m_lightCullInfoBuf;
	CGpuBuffer               m_LightShadeInfoBuf;
	CGpuBuffer               m_lightGridBuf;
	CGpuBuffer               m_lightCountBuf;
	CGpuBuffer               m_fogVolumeCullInfoBuf;
	CGpuBuffer               m_fogVolumeInjectInfoBuf;

	CFullscreenPass          m_passDownscaleShadowmap[ShadowCascadeNum];
	CFullscreenPass          m_passDownscaleShadowmap2[ShadowCascadeNum];
	CComputeRenderPass       m_passBuildLightListGrid;
	CComputeRenderPass       m_passDownscaleDepthHorizontal;
	CComputeRenderPass       m_passDownscaleDepthVertical;
	CComputeRenderPass       m_passInjectFogDensity;
	CSceneRenderPass         m_passInjectParticleDensity;
	CComputeRenderPass       m_passInjectInscattering;
	CComputeRenderPass       m_passBlurDensityHorizontal[2];
	CComputeRenderPass       m_passBlurDensityVertical[2];
	CComputeRenderPass       m_passBlurInscatteringHorizontal[2];
	CComputeRenderPass       m_passBlurInscatteringVertical[2];
	CComputeRenderPass       m_passTemporalReprojection[2];
	CComputeRenderPass       m_passRaymarch[2];

	Vec4                     m_globalEnvProbeParam0;
	Vec4                     m_globalEnvProbeParam1;

	CDeviceResourceLayoutPtr m_pSceneRenderResourceLayout;
	CDeviceResourceSetDesc   m_sceneRenderPassResources;
	CDeviceResourceSetPtr    m_pSceneRenderPassResourceSet;
	CConstantBufferPtr       m_pSceneRenderPassCB;

	Matrix44A                m_viewProj[MaxFrameNum];
	i32                    m_cleared;
	u32                   m_numTileLights;
	u32                   m_numFogVolumes;
	int64                    m_frameID;
	i32                    m_tick;
	i32                    m_resourceFrameID;
};
