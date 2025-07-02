// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/D3DColorGradingController.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/D3DPostProcess.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Render/D3D/GraphicsPipeline/ColorGrading.h>

i32k COLORCHART_SIZE = 16;
i32k COLORCHART_ALIGNED_SIZE = 16;
i32k COLORCHART_WIDTH = COLORCHART_SIZE * COLORCHART_SIZE;
i32k COLORCHART_RENDERTARGET_WIDTH = COLORCHART_SIZE * COLORCHART_ALIGNED_SIZE;
i32k COLORCHART_HEIGHT = COLORCHART_SIZE;
u32k COLORCHART_TEXFLAGS = FT_NOMIPS | FT_DONT_STREAM | FT_STATE_CLAMP;



CColorGradingController::CColorGradingController()
	: m_layers()
	, m_pChartIdentity(nullptr)
	, m_pChartStatic(nullptr)
	, m_pChartToUse(nullptr)
{
}

CColorGradingController::~CColorGradingController()
{
}

void CColorGradingController::FreeMemory()
{
	DrxAutoCriticalSectionNoRecursive lock(m_layersLock);

	stl::reconstruct(m_layers);
	m_pChartToUse = nullptr;
	m_pChartStatic = nullptr;
	m_pChartIdentity = nullptr;
}

bool CColorGradingController::ValidateColorChart(const CTexture* pChart) const
{
	if (!CTexture::IsTextureExist(pChart))
		return false;

	if (pChart->IsNoTexture())
		return false;

	if (pChart->GetWidth() != COLORCHART_WIDTH || pChart->GetHeight() != COLORCHART_HEIGHT)
		return false;

	return true;
}

CTexture* CColorGradingController::LoadColorChartInt(tukk pChartFilePath) const
{
	if (!pChartFilePath || !pChartFilePath[0])
		return 0;

	CTexture* pChart = (CTexture*)gRenDev->EF_LoadTexture(pChartFilePath, COLORCHART_TEXFLAGS);
	if (!ValidateColorChart(pChart))
	{
		SAFE_RELEASE(pChart);
		return 0;
	}
	return pChart;
}

i32 CColorGradingController::LoadColorChart(tukk pChartFilePath) const
{
	CTexture* pChart = LoadColorChartInt(pChartFilePath);
	return pChart ? pChart->GetID() : -1;
}

void CColorGradingController::UnloadColorChart(i32 texID) const
{
	CTexture* pChart = CTexture::GetByID(texID);
	SAFE_RELEASE(pChart);
}

void CColorGradingController::SetLayers(const SColorChartLayer* pLayerInfo, u32 numLayers) threadsafe
{
	DrxAutoCriticalSectionNoRecursive lock(m_layersLock);

	m_layers.reserve(numLayers);
	m_layers.resize(0);

	if (numLayers)
	{
		float blendSum = 0;
		for (size_t i = 0; i < numLayers; ++i)
		{
			const SColorChartLayer& l = pLayerInfo[i];
			if (l.m_texID > 0 && l.m_blendAmount > 0)
			{
				const CTexture* pChart = CTexture::GetByID(l.m_texID);
				if (ValidateColorChart(pChart))
				{
					m_layers.push_back(l);
					blendSum += l.m_blendAmount;
				}
			}
		}

		const size_t numActualLayers = m_layers.size();
		if (numActualLayers)
		{
			if (numActualLayers > 1)
			{
				float normalizeBlendAmount = (float) (1.0 / (double) blendSum);
				for (size_t i = 0; i < numActualLayers; ++i)
					m_layers[i].m_blendAmount *= normalizeBlendAmount;
			}
			else
				m_layers[0].m_blendAmount = 1;
		}
	}
}

bool CColorGradingController::InitResources()
{
	auto pColorGradingStage = static_cast<CColorGradingStage*>(gRenDev->GetGraphicsPipeline().GetStage(eStage_ColorGrading));

	m_pChartIdentity  = pColorGradingStage->GetIdentityColorChart();
	m_pChartStatic    = pColorGradingStage->GetStaticColorChart();
	m_pMergeLayers[0] = pColorGradingStage->GetMergeLayers()[0];
	m_pMergeLayers[1] = pColorGradingStage->GetMergeLayers()[1];

	if (!m_pChartIdentity || !m_pMergeLayers[0] || !m_pMergeLayers[1])
		return false;

	return true;
}

bool CColorGradingController::Update(const SColorGradingMergeParams* pMergeParams)
{
	m_pChartToUse = nullptr;

	if (!gRenDev->CV_r_colorgrading_charts)
		return true;

	if (m_pChartToUse = m_pChartStatic)
		return true;

	if (!InitResources())
	{
		m_pChartToUse = m_pChartIdentity;
		return m_pChartToUse != nullptr;
	}

	gRenDev->m_cEF.mfRefreshSystemShader("PostEffectsGame", CShaderMan::s_shPostEffectsGame);

	const uint64 sample0 = g_HWSR_MaskBit[HWSR_SAMPLE0];
	const uint64 sample1 = g_HWSR_MaskBit[HWSR_SAMPLE1];
	const uint64 sample2 = g_HWSR_MaskBit[HWSR_SAMPLE2];
	const uint64 sample5 = g_HWSR_MaskBit[HWSR_SAMPLE5];

	auto pColorGradingStage = static_cast<CColorGradingStage*>(gRenDev->GetGraphicsPipeline().GetStage(eStage_ColorGrading));

	// OLD PIPELINE
	ASSERT_LEGACY_PIPELINE

		/*

	// merge layers
	const size_t numLayers = m_layers.size();
	if (!numLayers)
	{
		m_pChartToUse = m_pChartIdentity;
	}
	else
	{
		CTexture* pNewMergeResult = m_pMergeLayers[0];
		gRenDev->FX_PushRenderTarget(0, pNewMergeResult, 0);

		i32 numMergePasses = 0;
		for (size_t curLayer = 0; curLayer < numLayers; )
		{
			size_t mergeLayerIdx[4] = { ~0U, ~0U, ~0U, ~0U };
			i32 numLayersPerPass = 0;

			for (; curLayer < numLayers && numLayersPerPass < 4; ++curLayer)
			{
				if (m_layers[curLayer].m_blendAmount > 0.001f)
					mergeLayerIdx[numLayersPerPass++] = curLayer;
			}

			if (numLayersPerPass)
			{
				const uint64 nResetFlags = ~(sample0 | sample1 | sample2);
				gRenDev->m_RP.m_FlagsShader_RT &= nResetFlags;
				if ((numLayersPerPass - 1) & 1)
					gRenDev->m_RP.m_FlagsShader_RT |= sample0;
				if ((numLayersPerPass - 1) & 2)
					gRenDev->m_RP.m_FlagsShader_RT |= sample1;

				CShader* pSh = CShaderMan::s_shPostEffectsGame;

				PROFILE_LABEL_SCOPE("MergeColorCharts");
				static CDrxNameTSCRC techName("MergeColorCharts");
				SD3DPostEffectsUtils::ShBeginPass(pSh, techName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

				Vec4 layerBlendAmount(0, 0, 0, 0);
				for (i32 i = 0; i < numLayersPerPass; ++i)
				{
					const SColorChartLayer& l = m_layers[mergeLayerIdx[i]];
					CTexture* pChart = CTexture::GetByID(l.m_texID);
					pChart->Apply(i, EDefaultSamplerStates::PointClamp);
					layerBlendAmount[i] = l.m_blendAmount;
				}

				static CDrxNameR semLayerBlendAmount("LayerBlendAmount");
				SD3DPostEffectsUtils::ShSetParamPS(semLayerBlendAmount, layerBlendAmount);

				static CDrxNameR semLayerSize("LayerSize");
				Vec4 layerSize((float)pNewMergeResult->GetWidth(), (float)pNewMergeResult->GetHeight(), 0, 0);
				SD3DPostEffectsUtils::ShSetParamPS(semLayerSize, layerSize);

				gRenDev->FX_SetState(GS_NODEPTHTEST | (numMergePasses ? GS_BLSRC_ONE | GS_BLDST_ONE : 0));
				gRenDev->SetCullMode(R_CULL_NONE);

				auto slicesVB = pColorGradingStage->GetSlicesVB();
				gcpRendD3D->DrawPrimitivesInternal(&slicesVB, COLORCHART_SIZE * 6, eptTriangleList);

				SD3DPostEffectsUtils::ShEndPass();
				++numMergePasses;

				gRenDev->m_RP.m_FlagsShader_RT &= nResetFlags;
			}
		}

		m_pChartToUse = numMergePasses ? pNewMergeResult : m_pChartIdentity;

		gRenDev->FX_PopRenderTarget(0);
	}

	// combine merged layers with color grading stuff
	if (m_pChartToUse && pMergeParams)
	{
		PROFILE_LABEL_SCOPE("CombineColorGradingWithColorChart");

		uint64 nSaveFlagsShader_RT = gRenDev->m_RP.m_FlagsShader_RT;
		gRenDev->m_RP.m_FlagsShader_RT = pMergeParams->nFlagsShaderRT;
		gRenDev->m_RP.m_FlagsShader_RT &= ~(sample1 | sample5);

		CTexture* pNewMergeResult = m_pMergeLayers[1];
		gRenDev->FX_PushRenderTarget(0, pNewMergeResult, 0);
		CShader* pSh = CShaderMan::s_shPostEffectsGame;

		static CDrxNameTSCRC techName("CombineColorGradingWithColorChart");
		SD3DPostEffectsUtils::ShBeginPass(pSh, techName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

		m_pChartToUse->Apply(0, EDefaultSamplerStates::LinearClamp);

		static CDrxNameR pParamName0("ColorGradingParams0");
		static CDrxNameR pParamName1("ColorGradingParams1");
		static CDrxNameR pParamName2("ColorGradingParams2");
		static CDrxNameR pParamName3("ColorGradingParams3");
		static CDrxNameR pParamName4("ColorGradingParams4");
		static CDrxNameR pParamMatrix("mColorGradingMatrix");

		pSh->FXSetPSFloat(pParamName0, &pMergeParams->pLevels[0], 1);
		pSh->FXSetPSFloat(pParamName1, &pMergeParams->pLevels[1], 1);
		pSh->FXSetPSFloat(pParamName2, &pMergeParams->pFilterColor, 1);
		pSh->FXSetPSFloat(pParamName3, &pMergeParams->pSelectiveColor[0], 1);
		pSh->FXSetPSFloat(pParamName4, &pMergeParams->pSelectiveColor[1], 1);
		pSh->FXSetPSFloat(pParamMatrix, &pMergeParams->pColorMatrix[0], 3);

		gRenDev->FX_SetState(GS_NODEPTHTEST);
		gRenDev->SetCullMode(R_CULL_NONE);

		auto slicesVB = pColorGradingStage->GetSlicesVB();
		gcpRendD3D->DrawPrimitivesInternal(&slicesVB, COLORCHART_SIZE * 6, eptTriangleList);

		SD3DPostEffectsUtils::ShEndPass();

		m_pChartToUse = pNewMergeResult;

		gRenDev->FX_PopRenderTarget(0);

		gRenDev->m_RP.m_FlagsShader_RT = nSaveFlagsShader_RT;
	}
	*/

	return m_pChartToUse != 0;
}

CTexture* CColorGradingController::GetColorChart() const
{
	auto pColorGradingStage = static_cast<CColorGradingStage*>(gRenDev->GetGraphicsPipeline().GetStage(eStage_ColorGrading));
	return pColorGradingStage->GetColorChart();
}

void CColorGradingController::DrawLayer(float x, float y, float w, float h, CTexture* pChart, float blendAmount, tukk pLayerName) const
{
	// OLD PIPELINE
	ASSERT_LEGACY_PIPELINE

	/*
#if !defined(_RELEASE)
	CShader* pSh = CShaderMan::s_shPostEffectsGame;

	gRenDev->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0];

	// If using merged color grading with color chart disable regular color transformations in display - only need to use color chart
	if (pChart && pChart->GetTexType() == eTT_3D)
		gRenDev->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];

	static CDrxNameTSCRC techName("DisplayColorCharts");
	SD3DPostEffectsUtils::ShBeginPass(pSh, techName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

	if (pChart)
		pChart->Apply(0, EDefaultSamplerStates::PointClamp);

	// NOTE: Get aligned stack-space (pointer and size aligned to manager's alignment requirement)
	DrxStackAllocWithSizeVector(SVF_P3F_C4B_T2F, 4, pVerts, CDeviceBufferUpr::AlignBufferSizeForStreaming);

	pVerts[0].xyz = Vec3(x, y, 0);
	pVerts[0].st = Vec2(0, 1);

	pVerts[1].xyz = Vec3(x + w, y, 0);
	pVerts[1].st = Vec2(1, 1);

	pVerts[2].xyz = Vec3(x, y + h, 0);
	pVerts[2].st = Vec2(0, 0);

	pVerts[3].xyz = Vec3(x + w, y + h, 0);
	pVerts[3].st = Vec2(1, 0);

	gRenDev->FX_Commit();
	gRenDev->FX_SetState(GS_NODEPTHTEST);
	gRenDev->SetCullMode(R_CULL_NONE);

	TempDynVB<SVF_P3F_C4B_T2F>::CreateFillAndBind(pVerts, 4, 0);

	if (!FAILED(gRenDev->FX_SetVertexDeclaration(0, EDefaultInputLayouts::P3F_C4B_T2F)))
		gRenDev->FX_DrawPrimitive(eptTriangleStrip, 0, 4);

	SD3DPostEffectsUtils::ShEndPass();

	float color[4] = { 1, 1, 1, 1 };
	IRenderAuxText::Draw2dLabel(x + w + 10.0f, y, 1.35f, color, false, "%2.1f%%", blendAmount * 100.0f);
	IRenderAuxText::Draw2dLabel(x + w + 55.0f, y, 1.35f, color, false, "%s", pLayerName);
#endif // #if !defined(_RELEASE)
*/
}

void CColorGradingController::DrawDebugInfo() const
{
#if !defined(_RELEASE)
	if (gRenDev->CV_r_colorgrading_charts < 2)
		return;

	DrxAutoCriticalSectionNoRecursive lock(m_layersLock);

	const float w = (float) COLORCHART_WIDTH;
	const float h = (float) COLORCHART_HEIGHT;

	float x = 16.0f;
	float y = 16.0f;

	if (!m_pChartStatic)
	{
		for (size_t i = 0, numLayers = m_layers.size(); i < numLayers; ++i)
		{
			const SColorChartLayer& l = m_layers[i];
			CTexture* pChart = CTexture::GetByID(l.m_texID);
			DrawLayer(x, y, w, h, pChart, l.m_blendAmount, PathUtil::GetFile(pChart->GetName()));
			y += h + 4;
		}
		if (GetColorChart())
			DrawLayer(x, y, w, h, GetColorChart(), 1, "FinalChart");
	}
	else
		DrawLayer(x, y, w, h, m_pChartStatic, 1, PathUtil::GetFile(m_pChartStatic->GetName()));
#endif // #if !defined(_RELEASE)
}

bool CColorGradingController::LoadStaticColorChart(tukk pChartFilePath)
{
	bool bResult = true;
	
	SAFE_RELEASE(m_pChartStatic);
	if (pChartFilePath && pChartFilePath[0] != '\0')
	{
		m_pChartStatic = LoadColorChartInt(pChartFilePath);
		bResult = m_pChartStatic != nullptr;	
	}

	auto pColorGradingStage = static_cast<CColorGradingStage*>(gRenDev->GetGraphicsPipeline().GetStage(eStage_ColorGrading));
	pColorGradingStage->SetStaticColorChart(m_pChartStatic);

	return bResult;
}

const CTexture* CColorGradingController::GetStaticColorChart() const
{
	auto pColorGradingStage = static_cast<CColorGradingStage*>(gRenDev->GetGraphicsPipeline().GetStage(eStage_ColorGrading));
	return pColorGradingStage->GetStaticColorChart();
}

i32 CColorGradingController::GetColorChartSize() const
{
	return COLORCHART_SIZE;
}

