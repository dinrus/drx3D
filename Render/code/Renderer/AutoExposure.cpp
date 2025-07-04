// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/AutoExposure.h>

#include <drx3D/Render/DriverD3D.h>

void GetSampleOffsets_Downscale4x4Bilinear(u32 width, u32 height, Vec4 avSampleOffsets[])
{
	float tU = 1.0f / (float)width;
	float tV = 1.0f / (float)height;

	// Sample from the 16 surrounding points.  Since bilinear filtering is being used, specific the coordinate
	// exactly halfway between the current texel center (k-1.5) and the neighboring texel center (k-0.5)

	i32 index = 0;
	for (i32 y = 0; y < 4; y += 2)
	{
		for (i32 x = 0; x < 4; x += 2, index++)
		{
			avSampleOffsets[index].x = (x - 1.f) * tU;
			avSampleOffsets[index].y = (y - 1.f) * tV;
			avSampleOffsets[index].z = 0;
			avSampleOffsets[index].w = 1;
		}
	}
}

void CAutoExposureStage::MeasureLuminance()
{
	PROFILE_LABEL_SCOPE("MEASURE_LUMINANCE");

	CD3D9Renderer* pRenderer = gcpRendD3D;

	i32 curTexture = NUM_HDR_TONEMAP_TEXTURES - 1;

	float tU = 1.0f / (3.0f * CRendererResources::s_ptexHDRToneMaps[curTexture]->GetWidth());
	float tV = 1.0f / (3.0f * CRendererResources::s_ptexHDRToneMaps[curTexture]->GetHeight());

	Vec4 sampleOffsets[16];
	u32 index = 0;
	for (i32 x = -1; x <= 1; x++)
	{
		for (i32 y = -1; y <= 1; y++)
		{
			sampleOffsets[index].x = x * tU;
			sampleOffsets[index].y = y * tV;
			sampleOffsets[index].z = 0;
			sampleOffsets[index].w = 1;
			index++;
		}
	}

	CShader* pShader = CShaderMan::s_shHDRPostProcess;

	// Initial downsampling
	{
		if (m_passLuminanceInitial.IsDirty())
		{
			static CDrxNameTSCRC techLumInitial("HDRSampleLumInitial");
			m_passLuminanceInitial.SetPrimitiveFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
			m_passLuminanceInitial.SetPrimitiveType(CRenderPrimitive::ePrim_ProceduralTriangle);
			m_passLuminanceInitial.SetTechnique(pShader, techLumInitial, 0);
			m_passLuminanceInitial.SetRenderTarget(0, CRendererResources::s_ptexHDRToneMaps[curTexture]);
			m_passLuminanceInitial.SetState(GS_NODEPTHTEST);
			m_passLuminanceInitial.SetFlags(CPrimitiveRenderPass::ePassFlags_RequireVrProjectionConstants);

			m_passLuminanceInitial.SetTexture(0, CRendererResources::s_ptexHDRTargetScaled[1]);
			m_passLuminanceInitial.SetTexture(1, CRendererResources::s_ptexSceneNormalsMap);
			m_passLuminanceInitial.SetTexture(2, CRendererResources::s_ptexSceneDiffuse);
			m_passLuminanceInitial.SetTexture(3, CRendererResources::s_ptexSceneSpecular);
			m_passLuminanceInitial.SetSampler(0, EDefaultSamplerStates::LinearClamp);
		}

		static CDrxNameR sampleLumOffsets0Name("SampleLumOffsets0");
		static CDrxNameR sampleLumOffsets1Name("SampleLumOffsets1");

		m_passLuminanceInitial.BeginConstantUpdate();

		float s1 = 1.0f / (float) CRendererResources::s_ptexHDRTargetScaled[1]->GetWidth();
		float t1 = 1.0f / (float) CRendererResources::s_ptexHDRTargetScaled[1]->GetHeight();

		// Use rotated grid
		Vec4 sampleLumOffsets0 = Vec4(s1 * 0.95f, t1 * 0.25f, -s1 * 0.25f, t1 * 0.96f);
		Vec4 sampleLumOffsets1 = Vec4(-s1 * 0.96f, -t1 * 0.25f, s1 * 0.25f, -t1 * 0.96f);

		m_passLuminanceInitial.SetConstant(sampleLumOffsets0Name, sampleLumOffsets0, eHWSC_Pixel);
		m_passLuminanceInitial.SetConstant(sampleLumOffsets1Name, sampleLumOffsets1, eHWSC_Pixel);

		m_passLuminanceInitial.Execute();
	}

	// Iterative downsampling
	for (curTexture = curTexture - 1; curTexture >= 0; --curTexture)
	{
		CFullscreenPass& passLuminanceIteration = m_passLuminanceIteration[curTexture];

		if (passLuminanceIteration.IsDirty())
		{
			uint64 rtMask = 0;
			if (!curTexture)
				rtMask |= g_HWSR_MaskBit[HWSR_SAMPLE0];
			if (curTexture == 1)
				rtMask |= g_HWSR_MaskBit[HWSR_SAMPLE1];

			static CDrxNameTSCRC techLumIterative("HDRSampleLumIterative");
			passLuminanceIteration.SetPrimitiveFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
			passLuminanceIteration.SetPrimitiveType(CRenderPrimitive::ePrim_ProceduralTriangle);
			passLuminanceIteration.SetTechnique(pShader, techLumIterative, rtMask);
			passLuminanceIteration.SetRenderTarget(0, CRendererResources::s_ptexHDRToneMaps[curTexture]);
			passLuminanceIteration.SetState(GS_NODEPTHTEST);
			passLuminanceIteration.SetTexture(0, CRendererResources::s_ptexHDRToneMaps[curTexture + 1]);
			passLuminanceIteration.SetSampler(0, EDefaultSamplerStates::LinearClamp);
		}

		static CDrxNameR param1Name("SampleOffsets");

		passLuminanceIteration.BeginConstantUpdate();

		GetSampleOffsets_Downscale4x4Bilinear(CRendererResources::s_ptexHDRToneMaps[curTexture + 1]->GetWidth(), CRendererResources::s_ptexHDRToneMaps[curTexture + 1]->GetHeight(), sampleOffsets);
		passLuminanceIteration.SetConstantArray(param1Name, sampleOffsets, 4, eHWSC_Pixel);

		passLuminanceIteration.Execute();
	}

	GetDeviceObjectFactory().GetCoreCommandList().GetCopyInterface()->Copy(
		CRendererResources::s_ptexHDRToneMaps[0]->GetDevTexture(),
		CRendererResources::s_ptexHDRMeasuredLuminance[gcpRendD3D->RT_GetCurrGpuID()]->GetDevTexture()
	);
}

void CAutoExposureStage::AdjustExposure()
{
	PROFILE_LABEL_SCOPE("EYEADAPTATION");

	CD3D9Renderer* pRenderer = gcpRendD3D;

	// Swap current & last luminance
	i32k lumMask = (i32)(sizeof(CRendererResources::s_ptexHDRAdaptedLuminanceCur) / sizeof(CRendererResources::s_ptexHDRAdaptedLuminanceCur[0])) - 1;
	i32k numTextures = (i32)max(min(gRenDev->GetActiveGPUCount(), (u32)(sizeof(CRendererResources::s_ptexHDRAdaptedLuminanceCur) / sizeof(CRendererResources::s_ptexHDRAdaptedLuminanceCur[0]))), 1u);

	CRendererResources::s_nCurLumTextureIndex++;
	CTexture* pTexPrev = CRendererResources::s_ptexHDRAdaptedLuminanceCur[(CRendererResources::s_nCurLumTextureIndex - numTextures) & lumMask];
	CTexture* pTexCur = CRendererResources::s_ptexHDRAdaptedLuminanceCur[CRendererResources::s_nCurLumTextureIndex & lumMask];
	CRendererResources::s_ptexCurLumTexture = pTexCur;
	assert(pTexCur);

	CShader* pShader = CShaderMan::s_shHDRPostProcess;

	if (m_passAutoExposure.IsDirty(pTexCur->GetTextureID(), pTexPrev->GetTextureID()))
	{
		static CDrxNameTSCRC techAdaptedLum("HDRCalculateAdaptedLum");
		m_passAutoExposure.SetPrimitiveFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
		m_passAutoExposure.SetPrimitiveType(CRenderPrimitive::ePrim_ProceduralTriangle);
		m_passAutoExposure.SetTechnique(pShader, techAdaptedLum, 0);
		m_passAutoExposure.SetRenderTarget(0, pTexCur);
		m_passAutoExposure.SetState(GS_NODEPTHTEST);

		m_passAutoExposure.SetTexture(0, pTexPrev);
		m_passAutoExposure.SetTexture(1, CRendererResources::s_ptexHDRToneMaps[0]);
		m_passAutoExposure.SetSampler(0, EDefaultSamplerStates::PointClamp);
	}

	static CDrxNameR param0Name("ElapsedTime");

	m_passAutoExposure.BeginConstantUpdate();

	Vec4 param0(iTimer->GetFrameTime() * numTextures, 1, 1, 0);
	if (!RenderView()->GetCamera(CCamera::eEye_Left).IsJustActivated() && pRenderer->m_nDisableTemporalEffects == 0)
	{
		param0[1] = 1.0f - expf(-CRenderer::CV_r_HDREyeAdaptationSpeed * param0[0]);
		param0[2] = 1.0f - expf(-CRenderer::CV_r_HDRRangeAdaptationSpeed * param0[0]);
	}
	m_passAutoExposure.SetConstant(param0Name, param0, eHWSC_Pixel);

	m_passAutoExposure.Execute();
}

void CAutoExposureStage::Execute()
{
	MeasureLuminance();
	AdjustExposure();
}
