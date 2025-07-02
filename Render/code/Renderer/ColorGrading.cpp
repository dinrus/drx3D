// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/ColorGrading.h>

#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/PostProcess/PostEffects.h>

tukk COLORCHART_DEF_TEX = "%ENGINE%/EngineAssets/Textures/default_cch.tif";
const ETEX_Format COLORCHART_FORMAT = eTF_R8G8B8A8;

CColorGradingStage::CColorGradingStage()
	: m_pChartIdentity(nullptr)
	, m_pChartStatic(nullptr)
	, m_pChartToUse(nullptr)
	, m_slicesVertexBuffer(~0u)
	, m_colorGradingPrimitive(CRenderPrimitive::eFlags_ReflectShaderConstants)
{
	m_pMergeLayers.fill(nullptr);

	// Preallocate 2 merge primitives (i.e. up to 8 blending layers)
	m_mergeChartsPrimitives.emplace_back(CRenderPrimitive::eFlags_ReflectShaderConstants);
	m_mergeChartsPrimitives.emplace_back(CRenderPrimitive::eFlags_ReflectShaderConstants);
}

void CColorGradingStage::Init()
{
	CColorGradingController* pCtrl = gcpRendD3D->m_pColorGradingControllerD3D;
	i32k chartSize = pCtrl->GetColorChartSize();

	// load default color chart
	i32 defaultChartID = pCtrl->LoadColorChart(COLORCHART_DEF_TEX);
	if (defaultChartID >= 0) 
	{
		m_pChartIdentity.Assign_NoAddRef(CTexture::GetByID(defaultChartID));
	}
	else
	{
		static bool bPrint = true;
		if (bPrint) iLog->LogError("Failed to initialize Color Grading: Default color chart is missing");
		bPrint = false;
	}

	// allocate rendertargets
	for (i32 i = 0; i < m_pMergeLayers.size(); ++i)
	{
		tukk rtName = i == 0 ? "ColorGradingMergeLayer0" : "ColorGradingMergeLayer1";

		CTexture* pRT = CTexture::GetOrCreateRenderTarget(rtName, chartSize*chartSize, chartSize, Clr_Empty, eTT_2D, FT_NOMIPS | FT_DONT_STREAM | FT_STATE_CLAMP, COLORCHART_FORMAT);
		if (CTexture::IsTextureExist(pRT))
		{
			m_pMergeLayers[i].Assign_NoAddRef(pRT);
		}
	}

	// prepare passes
	{
		D3DViewPort viewport;
		viewport.TopLeftX = viewport.TopLeftY = 0.0f;
		viewport.Width = (float)m_pMergeLayers[0]->GetWidth();
		viewport.Height = (float)m_pMergeLayers[0]->GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_mergePass.SetRenderTarget(0, m_pMergeLayers[0]);
		m_mergePass.SetViewport(viewport);

		m_colorGradingPass.SetRenderTarget(0, m_pMergeLayers[1]);
		m_colorGradingPass.SetViewport(viewport);
	}

	// prepare vertex buffer
	{
		m_vecSlicesData.resize(6 * chartSize);

		const float fQuadSize = 1.0f / chartSize;
		const float fTexCoordSize = 1.f / chartSize;
		for (u32 iQuad = 0; iQuad < chartSize; ++iQuad)
		{
			const float fQuadShift = (float)iQuad / chartSize;
			const float fBlue = (float)iQuad / (chartSize - 1.f);

			m_vecSlicesData[iQuad * 6 + 0].xyz = Vec3(fQuadShift + fQuadSize, 1.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 0].st = Vec2(fQuadShift + fTexCoordSize, 1.f);
			m_vecSlicesData[iQuad * 6 + 0].color.dcolor = ColorF(1.f, 1.f, fBlue).pack_argb8888();
			m_vecSlicesData[iQuad * 6 + 1].xyz = Vec3(fQuadShift + fQuadSize, 0.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 1].st = Vec2(fQuadShift + fTexCoordSize, 0.f);
			m_vecSlicesData[iQuad * 6 + 1].color.dcolor = ColorF(1.f, 0.f, fBlue).pack_argb8888();
			m_vecSlicesData[iQuad * 6 + 2].xyz = Vec3(fQuadShift + 0.f, 1.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 2].st = Vec2(fQuadShift + 0.f, 1.f);
			m_vecSlicesData[iQuad * 6 + 2].color.dcolor = ColorF(0.f, 1.f, fBlue).pack_argb8888();

			m_vecSlicesData[iQuad * 6 + 3].xyz = Vec3(fQuadShift + 0.f, 1.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 3].st = Vec2(fQuadShift + 0.f, 1.f);
			m_vecSlicesData[iQuad * 6 + 3].color.dcolor = ColorF(0.f, 1.f, fBlue).pack_argb8888();
			m_vecSlicesData[iQuad * 6 + 4].xyz = Vec3(fQuadShift + fQuadSize, 0.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 4].st = Vec2(fQuadShift + fTexCoordSize, 0.f);
			m_vecSlicesData[iQuad * 6 + 4].color.dcolor = ColorF(1.f, 0.f, fBlue).pack_argb8888();
			m_vecSlicesData[iQuad * 6 + 5].xyz = Vec3(fQuadShift + 0.f, 0.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 5].st = Vec2(fQuadShift + 0.f, 0.f);
			m_vecSlicesData[iQuad * 6 + 5].color.dcolor = ColorF(0.f, 0.f, fBlue).pack_argb8888();
		}

		m_slicesVertexBuffer = gcpRendD3D->m_DevBufMan.Create(BBT_VERTEX_BUFFER, BU_STATIC, m_vecSlicesData.size() * sizeof(m_vecSlicesData[0]));
		gcpRendD3D->m_DevBufMan.UpdateBuffer(m_slicesVertexBuffer, m_vecSlicesData);
	}
}

void CColorGradingStage::PreparePrimitives(CColorGradingController& controller, const SColorGradingMergeParams& mergeParams)
{
	m_pChartToUse = nullptr;

	if (!m_pMergeLayers[0] || !m_pMergeLayers[1] || m_slicesVertexBuffer == ~0u)
		return;

	m_pChartToUse = GetStaticColorChart();
	if (m_pChartToUse != nullptr)
		return;

	// init passes
	m_mergePass.BeginAddingPrimitives();
	m_colorGradingPass.BeginAddingPrimitives();

	DrxAutoCriticalSectionNoRecursive layersLock(controller.GetLayersLock());

	// prepare chart blending primitives
	{
		i32k numLayers = controller.m_layers.size();
		for (i32 i = m_mergeChartsPrimitives.size(), end = numLayers/4+1; i < end; ++i)
			m_mergeChartsPrimitives.emplace_back(CRenderPrimitive::eFlags_ReflectShaderConstants);

		i32 numMergePasses = 0;
		for (size_t curLayer = 0; curLayer < numLayers; )
		{
			CTexture* layerTex[4] = { nullptr, nullptr, nullptr, nullptr };
			Vec4 layerBlendAmount(0, 0, 0, 0);

			i32 numLayersPerPass = 0;

			// group up to four layers
			for (; curLayer < numLayers && numLayersPerPass < 4; ++curLayer)
			{
				auto& layer = controller.m_layers[curLayer];
				if (layer.m_blendAmount > 0.001f)
				{
					layerTex[numLayersPerPass] = CTexture::GetByID(layer.m_texID);
					layerBlendAmount[numLayersPerPass] = layer.m_blendAmount;

					++numLayersPerPass;
				}
			}

			if (numLayersPerPass)
			{
				static CDrxNameTSCRC techName("MergeColorCharts");

				uint64 rtFlags = 0;
				rtFlags |= ((numLayersPerPass - 1) & 1) ? g_HWSR_MaskBit[HWSR_SAMPLE0] : 0;
				rtFlags |= ((numLayersPerPass - 1) & 2) ? g_HWSR_MaskBit[HWSR_SAMPLE1] : 0;

				CRenderPrimitive& prim = m_mergeChartsPrimitives[numMergePasses];
				prim.SetTechnique(CShaderMan::s_shPostEffectsGame, techName, rtFlags);
				prim.SetFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
				prim.SetRenderState(GS_NODEPTHTEST | (numMergePasses ? GS_BLSRC_ONE | GS_BLDST_ONE : 0));
				prim.SetCustomVertexStream(m_slicesVertexBuffer, EDefaultInputLayouts::P3F_C4B_T2F, sizeof(SVF_P3F_C4B_T2F));
				prim.SetDrawInfo(eptTriangleList, 0, 0, 6 * controller.GetColorChartSize());
				for (i32 i = 0; i < numLayersPerPass; ++i)
					prim.SetTexture(i, layerTex[i]);
				prim.Compile(m_mergePass);

				prim.GetConstantUpr().BeginNamedConstantUpdate();

				static CDrxNameR nameLayerBlendAmount("LayerBlendAmount");
				static CDrxNameR nameLayerSize("LayerSize");

				prim.GetConstantUpr().SetNamedConstant(nameLayerBlendAmount, layerBlendAmount, eHWSC_Pixel);
				Vec4 layerSize((float)m_pMergeLayers[0]->GetWidth(), (float)m_pMergeLayers[0]->GetHeight(), 0, 0);
				prim.GetConstantUpr().SetNamedConstant(nameLayerSize, layerSize, eHWSC_Pixel);

				prim.GetConstantUpr().EndNamedConstantUpdate(&m_mergePass.GetViewport());

				m_mergePass.AddPrimitive(&prim);
				++numMergePasses;
			}

		}

		m_pChartToUse = numMergePasses ? m_pMergeLayers[0] : m_pChartIdentity;
	}

	// prepare color grading primitive
	if (m_pChartToUse)
	{
		static CDrxNameTSCRC techName("CombineColorGradingWithColorChart");
		uint64 rtFlags = mergeParams.nFlagsShaderRT & ~g_HWSR_MaskBit[HWSR_SAMPLE1];

		m_colorGradingPrimitive.SetFlags(CRenderPrimitive::eFlags_ReflectShaderConstants_PS);
		m_colorGradingPrimitive.SetTechnique(CShaderMan::s_shPostEffectsGame, techName, rtFlags);
		m_colorGradingPrimitive.SetRenderState(GS_NODEPTHTEST);
		m_colorGradingPrimitive.SetSampler(0, EDefaultSamplerStates::LinearClamp);
		m_colorGradingPrimitive.SetPrimitiveType(CRenderPrimitive::ePrim_Custom);
		m_colorGradingPrimitive.SetCustomVertexStream(m_slicesVertexBuffer, EDefaultInputLayouts::P3F_C4B_T2F, sizeof(SVF_P3F_C4B_T2F));
		m_colorGradingPrimitive.SetDrawInfo(eptTriangleList, 0, 0, 6 * controller.GetColorChartSize());
		m_colorGradingPrimitive.SetTexture(0, m_pChartToUse);
		m_colorGradingPrimitive.Compile(m_colorGradingPass);

		auto& constantUpr = m_colorGradingPrimitive.GetConstantUpr();

		constantUpr.BeginNamedConstantUpdate();

		static CDrxNameR pParamName0("ColorGradingParams0");
		static CDrxNameR pParamName1("ColorGradingParams1");
		static CDrxNameR pParamName2("ColorGradingParams2");
		static CDrxNameR pParamName3("ColorGradingParams3");
		static CDrxNameR pParamName4("ColorGradingParams4");
		static CDrxNameR pParamMatrix("mColorGradingMatrix");

		constantUpr.SetNamedConstant(pParamName0, mergeParams.pLevels[0], eHWSC_Pixel);
		constantUpr.SetNamedConstant(pParamName1, mergeParams.pLevels[1], eHWSC_Pixel);
		constantUpr.SetNamedConstant(pParamName2, mergeParams.pFilterColor, eHWSC_Pixel);
		constantUpr.SetNamedConstant(pParamName3, mergeParams.pSelectiveColor[0], eHWSC_Pixel);
		constantUpr.SetNamedConstant(pParamName4, mergeParams.pSelectiveColor[1], eHWSC_Pixel);
		constantUpr.SetNamedConstantArray(pParamMatrix, mergeParams.pColorMatrix, 3, eHWSC_Pixel);

		constantUpr.EndNamedConstantUpdate(&m_colorGradingPass.GetViewport());

		m_colorGradingPass.AddPrimitive(&m_colorGradingPrimitive);

		m_pChartToUse = m_pMergeLayers[1];
	}
}


void CColorGradingStage::Execute()
{
	if (!CRenderer::CV_r_colorgrading || !CRenderer::CV_r_colorgrading_charts || !gcpRendD3D->m_pColorGradingControllerD3D)
	{
		m_pChartToUse = nullptr;
		return;
	}
	
	if (CColorGrading* pColorGrading = (CColorGrading*)PostEffectMgr()->GetEffect(EPostEffectID::ColorGrading))
	{
		PROFILE_LABEL_SCOPE("COLORGRADING");

		SColorGradingMergeParams mergeParams;
		pColorGrading->UpdateParams(mergeParams, false);
		
		if (gEnv->IsCutscenePlaying() || (RenderView()->GetFrameId() % max(1, CRenderer::CV_r_ColorgradingChartsCache)) == 0 || IsRenderPassesDirty())
		{
			PreparePrimitives(*gcpRendD3D->m_pColorGradingControllerD3D, mergeParams);
		}
		
		if (m_mergePass.GetPrimitiveCount() > 0)
			m_mergePass.Execute();

		if (m_colorGradingPass.GetPrimitiveCount() > 0)
			m_colorGradingPass.Execute();
	}
}

CVertexBuffer CColorGradingStage::GetSlicesVB() const
{ 
	CVertexBuffer result((uk )&m_vecSlicesData[0], EDefaultInputLayouts::P3F_C4B_T2F, m_vecSlicesData.size());
	return result;
}

bool CColorGradingStage::IsRenderPassesDirty()
{
	bool isDirty = false;
	if (m_colorGradingPrimitive.IsDirty())
		isDirty = true;
	for (auto& prim : m_mergeChartsPrimitives)
	{
		if (prim.IsDirty())
		{
			isDirty = true;
			break;
		}
	}
	return isDirty;
}
