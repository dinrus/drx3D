// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>

class CFogStage : public CGraphicsPipelineStage
{
public:
	struct SForwardParams
	{
		Vec4  vfParams;
		Vec4  vfRampParams;
		Vec4  vfSunDir;
		Vec3  vfColGradBase;
		float padding0;
		Vec3  vfColGradDelta;
		float padding1;
		Vec4  vfColGradParams;
		Vec4  vfColGradRadial;
		// Fog shadows
		Vec4  vfShadowDarkening;
		Vec4  vfShadowDarkeningSunAmb;
	};

public:
	void Init() final;
	void Resize(i32 renderWidth, i32 renderHeight) final;
	void OnCVarsChanged(const CCVarUpdateRecorder& cvarUpdater) final;
	void Execute();

	void FillForwardParams(SForwardParams& forwardParams, bool enable = true) const;

private:
	void ExecuteVolumetricFogShadow();
	f32  GetFogCullDistance() const;
	void ResizeResource(i32 resourceWidth, i32 resourceHeight);

private:
	_smart_ptr<CTexture> m_pTexInterleaveSamplePattern;
	_smart_ptr<CTexture> m_pCloudShadowTex;

#if defined(VOLUMETRIC_FOG_SHADOWS)
	CFullscreenPass m_passVolFogShadowRaycast;
	CFullscreenPass m_passVolFogShadowHBlur;
	CFullscreenPass m_passVolFogShadowVBlur;
#endif
	CFullscreenPass m_passFog;
	CFullscreenPass m_passLightning;
};
