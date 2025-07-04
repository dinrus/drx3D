// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/PrimitiveRenderPass.h>
#include <drx3D/Render/FullscreenPass.h>
//#include <drx3D/Render/UtilityPasses.h>

#if !DRX_PLATFORM_ORBIS
	#define FEATURE_RENDER_CLIPVOLUME_GEOMETRY_SHADER
#endif

class CClipVolumesStage : public CGraphicsPipelineStage
{
public:
	static u32k MaxDeferredClipVolumes = BIT_STENCIL_INSIDE_CLIPVOLUME - 1; // Keep in sync with MAX_CLIPVOLUMES in DeferredShading.cfx / ShadowBlur.cfx

public:
	CClipVolumesStage();
	virtual ~CClipVolumesStage();

	CGpuBuffer* GetClipVolumeInfoBuffer          ()       { return &m_clipVolumeInfoBuf; }
	CTexture*   GetClipVolumeStencilVolumeTexture() const { return m_pClipVolumeStencilVolumeTex; }

	bool        IsOutdoorVisible() const { DRX_ASSERT(m_bClipVolumesValid); return m_bOutdoorVisible; }

	void Init();
	void Destroy();
	void Update() final;
	bool IsStageActive(EShaderRenderingFlags flags) const final
	{
		if (flags & EShaderRenderingFlags::SHDF_FORWARD_MINIMAL)
			return false;

		return true;
	}

	void Prepare();
	void Execute();

public:
	struct SClipVolumeInfo
	{
		float data;
	};

	void GenerateClipVolumeInfo();

private:
	void PrepareVolumetricFog();
	void ExecuteVolumetricFog();

private:
	CPrimitiveRenderPass m_stencilPass;
	CPrimitiveRenderPass m_blendValuesPass;
	CFullscreenPass      m_stencilResolvePass;

#ifdef FEATURE_RENDER_CLIPVOLUME_GEOMETRY_SHADER
	CPrimitiveRenderPass m_volumetricStencilPass;
#else
	std::vector<std::unique_ptr<CPrimitiveRenderPass>> m_volumetricStencilPassArray;
	std::vector<std::unique_ptr<CFullscreenPass>>      m_resolveVolumetricStencilPassArray;
#endif
	std::vector<std::unique_ptr<CFullscreenPass>>      m_jitteredDepthPassArray;

	CRenderPrimitive                m_stencilPrimitives[MaxDeferredClipVolumes * 2];
	CRenderPrimitive                m_blendPrimitives[MaxDeferredClipVolumes];
#ifdef FEATURE_RENDER_CLIPVOLUME_GEOMETRY_SHADER
	CRenderPrimitive                m_stencilPrimitivesVolFog[MaxDeferredClipVolumes * 2];
#endif

	DRX_ALIGN(16) Vec4              m_clipVolumeShaderParams[MaxDeferredClipVolumes];
	u32                          m_nShaderParamCount;

	CGpuBuffer                      m_clipVolumeInfoBuf;

	CTexture*                       m_pBlendValuesRT;
	CTexture*                       m_pDepthTarget;

	CTexture*                       m_pClipVolumeStencilVolumeTex;
#ifdef FEATURE_RENDER_CLIPVOLUME_GEOMETRY_SHADER
	CTexture*                       m_depthTargetVolFog;
#else
	std::vector<CTexture*>          m_pClipVolumeStencilVolumeTexArray;
#endif
	std::vector<ResourceViewHandle> m_depthTargetArrayVolFog;

	i32                           m_cleared;
	float                           m_nearDepth;
	float                           m_raymarchDistance;

	bool                            m_bClipVolumesValid;
	bool                            m_bBlendPassReady;
	bool                            m_bOutdoorVisible;
};
