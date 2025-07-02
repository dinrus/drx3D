// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDeviceContext.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11DeviceContext
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLBlendState.hpp>
#include <drx3D/Render/CDrxDXGLBuffer.hpp>
#include <drx3D/Render/CDrxDXGLDepthStencilView.hpp>
#include <drx3D/Render/CDrxDXGLDeviceContext.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/CDrxDXGLBlendState.hpp>
#include <drx3D/Render/CDrxDXGLDepthStencilState.hpp>
#include <drx3D/Render/CDrxDXGLInputLayout.hpp>
#include <drx3D/Render/CDrxDXGLQuery.hpp>
#include <drx3D/Render/CDrxDXGLRasterizerState.hpp>
#include <drx3D/Render/CDrxDXGLRenderTargetView.hpp>
#include <drx3D/Render/CDrxDXGLSamplerState.hpp>
#include <drx3D/Render/CDrxDXGLShader.hpp>
#include <drx3D/Render/CDrxDXGLShaderResourceView.hpp>
#include <drx3D/Render/CDrxDXGLUnorderedAccessView.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>
#include <drx3D/Render/Implementation/GLFormat.hpp>
#include <drx3D/Render/Implementation/GLView.hpp>

#define DXGL_CHECK_HAZARDS         0
#define DXGL_CHECK_PIPELINE        0
#define DXGL_CHECK_CURRENT_CONTEXT 0

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLDeviceContext
////////////////////////////////////////////////////////////////////////////////

CDrxDXGLDeviceContext::CDrxDXGLDeviceContext()
	: m_pContext(nullptr)
	, m_uStencilRef(0)
	, m_uSampleMask(0xFFFFFFFF)
	, m_eIndexBufferFormat(DXGI_FORMAT_UNKNOWN)
	, m_uIndexBufferOffset(0)
	, m_ePrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, m_uNumViewports(0)
	, m_uNumScissorRects(0)
	, m_bPredicateValue(false)
{
	DXGL_INITIALIZE_INTERFACE(D3D11DeviceContext)

	m_auBlendFactor[0] = 1.0f;
	m_auBlendFactor[1] = 1.0f;
	m_auBlendFactor[2] = 1.0f;
	m_auBlendFactor[3] = 1.0f;

	for (u32 uVB = 0; uVB < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++uVB)
	{
		m_auVertexBufferStrides[uVB] = 0;
		m_auVertexBufferOffsets[uVB] = 0;
	}

	for (u32 uSO = 0; uSO < D3D11_SO_BUFFER_SLOT_COUNT; ++uSO)
	{
		m_auStreamOutputBufferOffsets[uSO] = 0;
	}

	m_kStages.resize(NDrxOpenGL::eST_NUM);
	std::vector<SStage>::iterator kStageIter(m_kStages.begin());
	const std::vector<SStage>::iterator kStageEnd(m_kStages.end());
	while (kStageIter != kStageEnd)
	{
		memset(kStageIter->m_auConstantBufferOffsets, 0, sizeof(kStageIter->m_auConstantBufferOffsets));
		memset(kStageIter->m_auConstantBufferSizes, 0, sizeof(kStageIter->m_auConstantBufferSizes));
		++kStageIter;
	}
}

CDrxDXGLDeviceContext::~CDrxDXGLDeviceContext()
{
	Shutdown();
}

bool CDrxDXGLDeviceContext::Initialize(CDrxDXGLDevice* pDevice)
{
	SetDevice(pDevice);

	m_spDefaultBlendState = CreateDefaultBlendState(pDevice);
	m_spDefaultDepthStencilState = CreateDefaultDepthStencilState(pDevice);
	m_spDefaultRasterizerState = CreateDefaultRasterizerState(pDevice);
	m_spDefaultSamplerState = CreateDefaultSamplerState(pDevice);

	NDrxOpenGL::CDevice* pGLDevice(pDevice->GetGLDevice());
	m_pContext = pGLDevice->AllocateContext();
	pGLDevice->BindContext(m_pContext);

	bool bResult =
	  m_spDefaultBlendState->Initialize(pDevice, m_pContext) &&
	  m_spDefaultDepthStencilState->Initialize(pDevice, m_pContext) &&
	  m_spDefaultRasterizerState->Initialize(pDevice, m_pContext) &&
	  m_spDefaultSamplerState->Initialize(m_pContext);

	pGLDevice->UnbindContext(m_pContext);

	return bResult;
}

void CDrxDXGLDeviceContext::Shutdown()
{
	if (m_pContext != NULL)
	{
		m_pContext->GetDevice()->FreeContext(m_pContext);
		m_pContext = NULL;
	}
	m_pDevice = NULL;
}

NDrxOpenGL::CContext* CDrxDXGLDeviceContext::GetGLContext()
{
	return m_pContext;
}

_smart_ptr<CDrxDXGLBlendState> CDrxDXGLDeviceContext::CreateDefaultBlendState(CDrxDXGLDevice* pDevice)
{
	// Default D3D11_BLEND_DESC values from DXSDK
	D3D11_BLEND_DESC kDesc;
	ZeroMemory(&kDesc, sizeof(kDesc));
	kDesc.AlphaToCoverageEnable = FALSE;
	kDesc.IndependentBlendEnable = FALSE;
	kDesc.RenderTarget[0].BlendEnable = FALSE;
	kDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	kDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	kDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	kDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	kDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	kDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	kDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	return new CDrxDXGLBlendState(kDesc, pDevice);
}

_smart_ptr<CDrxDXGLDepthStencilState> CDrxDXGLDeviceContext::CreateDefaultDepthStencilState(CDrxDXGLDevice* pDevice)
{
	// Default D3D11_DEPTH_STENCIL_DESC values from DXSDK
	D3D11_DEPTH_STENCIL_DESC kDesc;
	kDesc.DepthEnable = TRUE;
	kDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	kDesc.DepthFunc = D3D11_COMPARISON_LESS;
	kDesc.StencilEnable = FALSE;
	kDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	kDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	kDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	kDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	kDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	kDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	kDesc.BackFace = kDesc.FrontFace;

	return new CDrxDXGLDepthStencilState(kDesc, pDevice);
}

_smart_ptr<CDrxDXGLRasterizerState> CDrxDXGLDeviceContext::CreateDefaultRasterizerState(CDrxDXGLDevice* pDevice)
{
	// Default D3D11_RASTERIZER_DESC values from DXSDK
	D3D11_RASTERIZER_DESC kDesc;
	kDesc.FillMode = D3D11_FILL_SOLID;
	kDesc.CullMode = D3D11_CULL_BACK;
	kDesc.FrontCounterClockwise = FALSE;
	kDesc.DepthBias = 0;
	kDesc.SlopeScaledDepthBias = 0.0f;
	kDesc.DepthBiasClamp = 0.0f;
	kDesc.DepthClipEnable = TRUE;
	kDesc.ScissorEnable = FALSE;
	kDesc.MultisampleEnable = FALSE;
	kDesc.AntialiasedLineEnable = FALSE;

	return new CDrxDXGLRasterizerState(kDesc, pDevice);
}

_smart_ptr<CDrxDXGLSamplerState> CDrxDXGLDeviceContext::CreateDefaultSamplerState(CDrxDXGLDevice* pDevice)
{
	// Default D3D11_SAMPLER_DESC values from DXSDK
	D3D11_SAMPLER_DESC kDesc;
	kDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	kDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	kDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	kDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	kDesc.MinLOD = -FLT_MAX;
	kDesc.MaxLOD = FLT_MAX;
	kDesc.MipLODBias = 0.0f;
	kDesc.MaxAnisotropy = 1;
	kDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	kDesc.BorderColor[0] = 1.0f;
	kDesc.BorderColor[1] = 1.0f;
	kDesc.BorderColor[2] = 1.0f;
	kDesc.BorderColor[3] = 1.0f;

	return new CDrxDXGLSamplerState(kDesc, pDevice);
}

#if DXGL_CHECK_HAZARDS
void CheckHazard(u32 uRTVIndex, CDrxDXGLRenderTargetView* pRTView, CDrxDXGLResource* pRTVResource, u32 uSRVIndex, CDrxDXGLShaderResourceView* pSRView, CDrxDXGLResource* pSRVResource, u32 uStage)
{
	if (pRTVResource == pSRVResource)
	{
		DXGL_WARNING("Hazard detected: render target view %d and shader resource view %d in stage %d refer to the same resource", uRTVIndex, uSRVIndex, uStage);
	}
}

void CheckHazard(u32 uDSVIndex, CDrxDXGLDepthStencilView* pDSView, CDrxDXGLResource* pDSVResource, u32 uSRVIndex, CDrxDXGLShaderResourceView* pSRView, CDrxDXGLResource* pSRVResource, u32 uStage)
{
	if (pDSVResource == pSRVResource)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC kDSVDesc;
		pDSView->GetDesc(&kDSVDesc);
		if ((kDSVDesc.Flags & D3D11_DSV_READ_ONLY_DEPTH) == 0 || (kDSVDesc.Flags & D3D11_DSV_READ_ONLY_STENCIL) == 0)
		{
			DXGL_ERROR("Hazard detected: writable depth stencil view and shader resource view %d in stage %d refer to the same resource", uSRVIndex, uStage);
		}
	}
}

template<typename OMView>
void CheckHazards(_smart_ptr<OMView>* pOMViews, u32 uNumOMViews, _smart_ptr<CDrxDXGLShaderResourceView>* pSRViews, u32 uNumSRViews, u32 uStage)
{
	u32 uOMView;
	for (uOMView = 0; uOMView < uNumOMViews; ++uOMView)
	{
		OMView* pOMView(pOMViews[uOMView]);
		if (pOMView != NULL)
		{
			ID3D11Resource* pOMVResource(NULL);
			pOMView->GetResource(&pOMVResource);

			u32 uSRView;
			for (uSRView = 0; uSRView < uNumSRViews; ++uSRView)
			{
				CDrxDXGLShaderResourceView* pSRView(pSRViews[uSRView]);
				if (pSRView != NULL)
				{
					ID3D11Resource* pSRVResource(NULL);
					pSRView->GetResource(&pSRVResource);

					CheckHazard(uOMView, pOMView, pOMVResource, uSRView, pSRView, pSRVResource, uStage);

					pSRVResource->Release();
				}
			}

			pOMVResource->Release();
		}
	}
}

#endif //DXGL_CHECK_HAZARDS

#if DXGL_CHECK_PIPELINE
template<typename Stage>
void CheckRequiredStage(std::vector<Stage>& kStages, u32 uRequiredStage)
{
	if (kStages.size() <= uRequiredStage || kStages.at(uRequiredStage).m_spShader == NULL)
	{
		DXGL_ERROR("Required pipeline stage %d is not bound to a valid shader");
	}
}
template<typename Stage>
void CheckPipeline(std::vector<Stage>& kStages)
{
	CheckRequiredStage(kStages, NDrxOpenGL::eST_Vertex);
	CheckRequiredStage(kStages, NDrxOpenGL::eST_Fragment);
}
#else
template<typename Stage>
ILINE void CheckPipeline(std::vector<Stage>& kStages) {}
#endif //DXGL_CHECK_PIPELINE

#if DXGL_FULL_EMULATION
void CheckCurrentContext(NDrxOpenGL::CContext* pContext)
{
	if (pContext->GetDevice()->GetCurrentContext() != pContext)
	{
		pContext->GetDevice()->BindContext(pContext);
	}
}
#elif DXGL_CHECK_CURRENT_CONTEXT
void CheckCurrentContext(NDrxOpenGL::CContext* pContext)
{
	if (pContext->GetDevice()->GetCurrentContext() != pContext)
	{
		DXGL_ERROR("Device context has not been bound to this thread");
	}
}
#else
ILINE void CheckCurrentContext(NDrxOpenGL::CContext*) {}
#endif

////////////////////////////////////////////////////////////////////////////////
// ID3D11DeviceContext implementation
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLDeviceContext::SetShaderResources(u32 uStage, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
	CheckCurrentContext(m_pContext);

	if (uStage >= m_kStages.size())
	{
		for (u32 uView = 0; uView < NumViews; ++uView)
		{
			if (ppShaderResourceViews[uView] != NULL)
			{
				DXGL_ERROR("CDrxDXGLDeviceContext::SetShaderResources: shader stage is not supported, setting will be ignored");
				break;
			}
		}
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uView = 0; uView < NumViews; ++uView)
	{
		u32 uSlot(StartSlot + uView);
		CDrxDXGLShaderResourceView* pDXGLShaderResourceView(CDrxDXGLShaderResourceView::FromInterface(ppShaderResourceViews[uView]));
		if (kStage.m_aspShaderResourceViews[uSlot] != pDXGLShaderResourceView)
		{
			kStage.m_aspShaderResourceViews[uSlot] = pDXGLShaderResourceView;
#if OGL_SINGLE_CONTEXT
			m_pContext->SetShaderResourceView(pDXGLShaderResourceView == NULL ? NULL : pDXGLShaderResourceView->GetGLView(m_pContext), uStage, uSlot);
#else
			m_pContext->SetShaderResourceView(pDXGLShaderResourceView == NULL ? NULL : pDXGLShaderResourceView->GetGLView(), uStage, uSlot);
#endif
		}
	}

#if DXGL_CHECK_HAZARDS
	CheckHazards(m_aspRenderTargetViews, DXGL_ARRAY_SIZE(m_aspRenderTargetViews), kStage.m_aspShaderResourceViews, DXGL_ARRAY_SIZE(kStage.m_aspShaderResourceViews), uStage);
	CheckHazards(&m_spDepthStencilView, 1, kStage.m_aspShaderResourceViews, DXGL_ARRAY_SIZE(kStage.m_aspShaderResourceViews), uStage);
#endif //DXGL_CHECK_HAZARDS
}

void CDrxDXGLDeviceContext::SetUnorderedAccessViews(u32 uStage, UINT StartSlot, UINT NumViews, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews)
{
	CheckCurrentContext(m_pContext);

	if (uStage >= m_kStages.size())
	{
		for (u32 uView = 0; uView < NumViews; ++uView)
		{
			if (ppUnorderedAccessViews[uView] != NULL)
			{
				DXGL_ERROR("CDrxDXGLDeviceContext::SetUnorderedAccessViews: shader stage is not supported, setting will be ignored");
				break;
			}
		}
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uView = 0; uView < NumViews; ++uView)
	{
		u32 uSlot(StartSlot + uView);
		CDrxDXGLUnorderedAccessView* pDXGLUnorderedAccessView(CDrxDXGLUnorderedAccessView::FromInterface(ppUnorderedAccessViews[uView]));
		if (kStage.m_aspUnorderedAccessViews[uSlot] != pDXGLUnorderedAccessView)
		{
			kStage.m_aspUnorderedAccessViews[uSlot] = pDXGLUnorderedAccessView;
#if OGL_SINGLE_CONTEXT
			m_pContext->SetUnorderedAccessView(pDXGLUnorderedAccessView == NULL ? NULL : pDXGLUnorderedAccessView->GetGLView(m_pContext), uStage, uSlot);
#else
			m_pContext->SetUnorderedAccessView(pDXGLUnorderedAccessView == NULL ? NULL : pDXGLUnorderedAccessView->GetGLView(), uStage, uSlot);
#endif
		}
	}

#if DXGL_CHECK_HAZARDS
	CheckHazards(m_aspRenderTargetViews, DXGL_ARRAY_SIZE(m_aspRenderTargetViews), kStage.m_aspUnorderedAccessViews, DXGL_ARRAY_SIZE(kStage.m_aspUnorderedAccessViews), uStage);
	CheckHazards(&m_spDepthStencilView, 1, kStage.m_aspUnorderedAccessViews, DXGL_ARRAY_SIZE(kStage.m_aspUnorderedAccessViews), uStage);
#endif //DXGL_CHECK_HAZARDS
}

void CDrxDXGLDeviceContext::SetShader(u32 uStage, CDrxDXGLShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
{
	if (ppClassInstances != NULL && NumClassInstances != 0)
		DXGL_WARNING("Class instances not supported");

	if (uStage >= m_kStages.size())
	{
		if (pShader != NULL)
			DXGL_ERROR("CDrxDXGLDeviceContext::SetShader: shader stage is not supported, setting will be ignored");
		return;
	}

	if (m_kStages[uStage].m_spShader != pShader)
	{
		CheckCurrentContext(m_pContext);
		m_kStages[uStage].m_spShader = pShader;
		m_pContext->SetShader(pShader == NULL ? NULL : pShader->GetGLShader(), uStage);
	}
}

void CDrxDXGLDeviceContext::SetSamplers(u32 uStage, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
{
	CheckCurrentContext(m_pContext);

	if (uStage >= m_kStages.size())
	{
		for (u32 uSampler = 0; uSampler < NumSamplers; ++uSampler)
		{
			if (ppSamplers[uSampler] != NULL)
			{
				DXGL_ERROR("CDrxDXGLDeviceContext::SetSamplers: shader stage is not supported, setting will be ignored");
				break;
			}
		}
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uSampler = 0; uSampler < NumSamplers; ++uSampler)
	{
		u32 uSlot(uSampler + StartSlot);

		CDrxDXGLSamplerState* pSamplerState(CDrxDXGLSamplerState::FromInterface(ppSamplers[uSampler]));
		if (pSamplerState == NULL)
			pSamplerState = m_spDefaultSamplerState.get();

		if (pSamplerState != kStage.m_aspSamplerStates[uSlot])
		{
			kStage.m_aspSamplerStates[uSlot] = pSamplerState;
			pSamplerState->Apply(uStage, uSlot, m_pContext);
		}
	}
}

void CDrxDXGLDeviceContext::SetConstantBuffers(u32 uStage, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	CheckCurrentContext(m_pContext);

	if (uStage >= m_kStages.size())
	{
		for (u32 uBuffer = 0; uBuffer < NumBuffers; ++uBuffer)
		{
			if (ppConstantBuffers[uBuffer] != NULL)
			{
				DXGL_ERROR("CDrxDXGLDeviceContext::SetConstantBuffers: shader stage is not supported, setting will be ignored");
				break;
			}
		}
		return;
	}

	assert(uStage < m_kStages.size());
	SStage& kStage(m_kStages[uStage]);
	for (u32 uBuffer = 0; uBuffer < NumBuffers; ++uBuffer)
	{
		u32 uSlot(StartSlot + uBuffer);
		CDrxDXGLBuffer* pConstantBuffer(CDrxDXGLBuffer::FromInterface(ppConstantBuffers[uBuffer]));
		u32 uOffset(pFirstConstant == NULL ? 0 : pFirstConstant[uBuffer] * 16);
		u32 uSize(pNumConstants == NULL ? 0 : pNumConstants[uBuffer] * 16);
		if (kStage.m_aspConstantBuffers[uSlot] != pConstantBuffer ||
		    kStage.m_auConstantBufferOffsets[uSlot] != uOffset ||
		    kStage.m_auConstantBufferSizes[uSlot] != uSize)
		{
			kStage.m_aspConstantBuffers[uSlot] = pConstantBuffer;
			kStage.m_auConstantBufferOffsets[uSlot] = uOffset;
			kStage.m_auConstantBufferSizes[uSlot] = uSize;
			if (pConstantBuffer == NULL)
			{
				m_pContext->SetConstantBuffer(NULL, NDrxOpenGL::SBufferRange(uOffset, uSize), uStage, uSlot);
			}
			else
			{
				NDrxOpenGL::SBuffer* pGLBuffer(pConstantBuffer->GetGLBuffer());
				if (uSize == 0)
					uSize = pGLBuffer->m_uSize;
				m_pContext->SetConstantBuffer(pGLBuffer, NDrxOpenGL::SBufferRange(uOffset, uSize), uStage, uSlot);
			}
		}
	}
}

void CDrxDXGLDeviceContext::GetShaderResources(u32 uStage, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
{
	if (uStage >= m_kStages.size())
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::GetShaderResources: shader stage is not supported, no entries returned");
		for (u32 uView = 0; uView < NumViews; ++uView)
			ppShaderResourceViews[uView] = NULL;
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uView = 0; uView < NumViews; ++uView)
	{
		u32 uSlot(StartSlot + uView);
		CDrxDXGLShaderResourceView::ToInterface(ppShaderResourceViews + uView, kStage.m_aspShaderResourceViews[uSlot]);
		if (ppShaderResourceViews[uView] != NULL)
			ppShaderResourceViews[uView]->AddRef();
	}
}

void CDrxDXGLDeviceContext::GetUnorderedAccesses(u32 uStage, UINT StartSlot, UINT NumViews, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
	if (uStage >= m_kStages.size())
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::GetUnorderedAccesses: shader stage is not supported, no entries returned");
		for (u32 uView = 0; uView < NumViews; ++uView)
			ppUnorderedAccessViews[uView] = NULL;
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uView = 0; uView < NumViews; ++uView)
	{
		u32 uSlot(StartSlot + uView);
		CDrxDXGLUnorderedAccessView::ToInterface(ppUnorderedAccessViews + uView, kStage.m_aspUnorderedAccessViews[uSlot]);
		if (ppUnorderedAccessViews[uView] != NULL)
			ppUnorderedAccessViews[uView]->AddRef();
	}
}

void CDrxDXGLDeviceContext::GetShader(u32 uStage, CDrxDXGLShader** ppShader, ID3D11ClassInstance** ppClassInstances, UINT* NumClassInstances)
{
	if (ppClassInstances != NULL)
		DXGL_WARNING("Class instances not supported");
	if (NumClassInstances != NULL)
		*NumClassInstances = 0;

	if (uStage >= m_kStages.size())
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::GetShader: shader stage is not supported, no shader returned");
		*ppShader = NULL;
		return;
	}

	*ppShader = m_kStages[uStage].m_spShader.get();
	if (*ppShader != NULL)
		(*ppShader)->AddRef();
}

void CDrxDXGLDeviceContext::GetSamplers(u32 uStage, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
{
	if (uStage >= m_kStages.size())
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::GetSamplers: shader stage is not supported, no entries returned");
		for (u32 uSampler = 0; uSampler < NumSamplers; ++uSampler)
			ppSamplers[uSampler] = NULL;
		return;
	}

	SStage& kStage(m_kStages[uStage]);
	for (u32 uSampler = 0; uSampler < NumSamplers; ++uSampler)
	{
		u32 uSlot(uSampler + StartSlot);

		CDrxDXGLSamplerState::ToInterface(ppSamplers + uSampler, kStage.m_aspSamplerStates[uSlot]);
		if (ppSamplers[uSampler] != NULL)
			ppSamplers[uSampler]->AddRef();
	}
}

void CDrxDXGLDeviceContext::GetConstantBuffers(u32 uStage, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers, UINT* pFirstConstant, UINT* pNumConstants)
{
	if (uStage >= m_kStages.size())
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::GetConstantBuffers: shader stage is not supported, no entries returned");
		for (u32 uBuffer = 0; uBuffer < NumBuffers; ++uBuffer)
		{
			ppConstantBuffers[uBuffer] = NULL;
			if (pFirstConstant != NULL)
				pFirstConstant[uBuffer] = 0;
			if (pNumConstants != NULL)
				pNumConstants[uBuffer] = 0;
		}
		return;
	}

	assert(uStage < m_kStages.size());
	SStage& kStage(m_kStages[uStage]);
	for (u32 uBuffer = 0; uBuffer < NumBuffers; ++uBuffer)
	{
		u32 uSlot(StartSlot + uBuffer);

		CDrxDXGLBuffer::ToInterface(ppConstantBuffers + uBuffer, kStage.m_aspConstantBuffers[uSlot]);
		if (pFirstConstant != NULL)
			pFirstConstant[uBuffer] = kStage.m_auConstantBufferOffsets[uSlot];
		if (pNumConstants != NULL)
			pNumConstants[uBuffer] = kStage.m_auConstantBufferSizes[uSlot];
		if (ppConstantBuffers[uBuffer] != NULL)
			ppConstantBuffers[uBuffer]->AddRef();
	}
}

#define _IMPLEMENT_COMMON_SHADER_SETTERS(_Prefix, _ShaderInterface, _ShaderType, _Stage)                                                                                                        \
  void CDrxDXGLDeviceContext::_Prefix ## SetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const* ppShaderResourceViews)                                             \
  {                                                                                                                                                                                             \
    SetShaderResources(_Stage, StartSlot, NumViews, ppShaderResourceViews);                                                                                                                     \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## SetShader(_ShaderInterface * pShader, ID3D11ClassInstance * const* ppClassInstances, UINT NumClassInstances)                                           \
  {                                                                                                                                                                                             \
    SetShader(_Stage, _ShaderType::FromInterface(pShader), ppClassInstances, NumClassInstances);                                                                                                \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## SetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const* ppSamplers)                                                                  \
  {                                                                                                                                                                                             \
    SetSamplers(_Stage, StartSlot, NumSamplers, ppSamplers);                                                                                                                                    \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## SetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const* ppConstantBuffers)                                                           \
  {                                                                                                                                                                                             \
    SetConstantBuffers(_Stage, StartSlot, NumBuffers, ppConstantBuffers);                                                                                                                       \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## SetConstantBuffers1(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const* ppConstantBuffers, const UINT * pFirstConstant, const UINT * pNumConstants) \
  {                                                                                                                                                                                             \
    SetConstantBuffers(_Stage, StartSlot, NumBuffers, ppConstantBuffers, pFirstConstant, pNumConstants);                                                                                        \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## GetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * *ppShaderResourceViews)                                                   \
  {                                                                                                                                                                                             \
    GetShaderResources(_Stage, StartSlot, NumViews, ppShaderResourceViews);                                                                                                                     \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## GetShader(_ShaderInterface * *ppShader, ID3D11ClassInstance * *ppClassInstances, UINT * NumClassInstances)                                             \
  {                                                                                                                                                                                             \
    CDrxDXGLShader* pShader(NULL);                                                                                                                                                              \
    GetShader(_Stage, &pShader, ppClassInstances, NumClassInstances);                                                                                                                           \
    _ShaderType::ToInterface(ppShader, static_cast<_ShaderType*>(pShader));                                                                                                                     \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## GetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * *ppSamplers)                                                                        \
  {                                                                                                                                                                                             \
    GetSamplers(_Stage, StartSlot, NumSamplers, ppSamplers);                                                                                                                                    \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## GetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * *ppConstantBuffers)                                                                 \
  {                                                                                                                                                                                             \
    GetConstantBuffers(_Stage, StartSlot, NumBuffers, ppConstantBuffers);                                                                                                                       \
  }                                                                                                                                                                                             \
  void CDrxDXGLDeviceContext::_Prefix ## GetConstantBuffers1(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * *ppConstantBuffers, UINT * pFirstConstant, UINT * pNumConstants)                   \
  {                                                                                                                                                                                             \
    GetConstantBuffers(_Stage, StartSlot, NumBuffers, ppConstantBuffers, pFirstConstant, pNumConstants);                                                                                        \
  }
_IMPLEMENT_COMMON_SHADER_SETTERS(VS, ID3D11VertexShader, CDrxDXGLVertexShader, NDrxOpenGL::eST_Vertex)
_IMPLEMENT_COMMON_SHADER_SETTERS(PS, ID3D11PixelShader, CDrxDXGLPixelShader, NDrxOpenGL::eST_Fragment)
#if DXGL_SUPPORT_GEOMETRY_SHADERS
_IMPLEMENT_COMMON_SHADER_SETTERS(GS, ID3D11GeometryShader, CDrxDXGLGeometryShader, NDrxOpenGL::eST_Geometry)
#else
_IMPLEMENT_COMMON_SHADER_SETTERS(GS, ID3D11GeometryShader, CDrxDXGLGeometryShader, NDrxOpenGL::eST_NUM)
#endif
#if DXGL_SUPPORT_TESSELLATION
_IMPLEMENT_COMMON_SHADER_SETTERS(HS, ID3D11HullShader, CDrxDXGLHullShader, NDrxOpenGL::eST_TessControl)
_IMPLEMENT_COMMON_SHADER_SETTERS(DS, ID3D11DomainShader, CDrxDXGLDomainShader, NDrxOpenGL::eST_TessEvaluation)
#else
_IMPLEMENT_COMMON_SHADER_SETTERS(HS, ID3D11HullShader, CDrxDXGLHullShader, NDrxOpenGL::eST_NUM)
_IMPLEMENT_COMMON_SHADER_SETTERS(DS, ID3D11DomainShader, CDrxDXGLDomainShader, NDrxOpenGL::eST_NUM)
#endif
#if DXGL_SUPPORT_COMPUTE
_IMPLEMENT_COMMON_SHADER_SETTERS(CS, ID3D11ComputeShader, CDrxDXGLComputeShader, NDrxOpenGL::eST_Compute)
#else
_IMPLEMENT_COMMON_SHADER_SETTERS(CS, ID3D11ComputeShader, CDrxDXGLComputeShader, NDrxOpenGL::eST_NUM)
#endif
#undef _IMPLEMENT_COMMON_SHADER_SETTERS

void CDrxDXGLDeviceContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	CheckCurrentContext(m_pContext);
	CheckPipeline(m_kStages);
	m_pContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
}

void CDrxDXGLDeviceContext::Draw(UINT VertexCount, UINT StartVertexLocation)
{
	CheckCurrentContext(m_pContext);
	CheckPipeline(m_kStages);
	m_pContext->Draw(VertexCount, StartVertexLocation);
}

HRESULT CDrxDXGLDeviceContext::Map(ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
{
	CheckCurrentContext(m_pContext);
	NDrxOpenGL::SResource* pGLResource(CDrxDXGLResource::FromInterface(pResource)->GetGLResource());
	if (pGLResource->m_pfMapSubresource)
	{
		return (*pGLResource->m_pfMapSubresource)(pGLResource, Subresource, MapType, MapFlags, pMappedResource, m_pContext) ? S_OK : E_FAIL;
	}
	else
	{
		DXGL_NOT_IMPLEMENTED
		return E_FAIL;
	}
}

void CDrxDXGLDeviceContext::Unmap(ID3D11Resource* pResource, UINT Subresource)
{
	CheckCurrentContext(m_pContext);
	NDrxOpenGL::SResource* pGLResource(CDrxDXGLResource::FromInterface(pResource)->GetGLResource());
	if (pGLResource->m_pfUnmapSubresource)
	{
		(*pGLResource->m_pfUnmapSubresource)(pGLResource, Subresource, m_pContext);
	}
	else
	{
		DXGL_NOT_IMPLEMENTED
	}
}

void CDrxDXGLDeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout)
{
	CDrxDXGLInputLayout* pDXGLInputLayout(CDrxDXGLInputLayout::FromInterface(pInputLayout));
	if (m_spInputLayout != pDXGLInputLayout)
	{
		CheckCurrentContext(m_pContext);
		m_pContext->SetInputLayout(pDXGLInputLayout == NULL ? NULL : pDXGLInputLayout->GetGLLayout());
		m_spInputLayout = pDXGLInputLayout;
	}
}

void CDrxDXGLDeviceContext::IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
{
	CheckCurrentContext(m_pContext);
	u32 uSlot;
	for (uSlot = 0; uSlot < NumBuffers; ++uSlot)
	{
		u32 uSlotIndex(StartSlot + uSlot);
		CDrxDXGLBuffer* pDXGLVertexBuffer(CDrxDXGLBuffer::FromInterface(ppVertexBuffers[uSlot]));
		if (m_aspVertexBuffers[uSlotIndex] != pDXGLVertexBuffer ||
		    m_auVertexBufferStrides[uSlotIndex] != pStrides[uSlot] ||
		    m_auVertexBufferOffsets[uSlotIndex] != pOffsets[uSlot])
		{
			m_aspVertexBuffers[uSlotIndex] = pDXGLVertexBuffer;
			m_auVertexBufferStrides[uSlotIndex] = pStrides[uSlot];
			m_auVertexBufferOffsets[uSlotIndex] = pOffsets[uSlot];
			m_pContext->SetVertexBuffer(uSlotIndex, pDXGLVertexBuffer == NULL ? 0 : pDXGLVertexBuffer->GetGLBuffer(), pStrides[uSlot], pOffsets[uSlot]);
		}
	}
}

void CDrxDXGLDeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
{
	CDrxDXGLBuffer* pDXGLIndexBuffer(CDrxDXGLBuffer::FromInterface(pIndexBuffer));

	m_spIndexBuffer = pDXGLIndexBuffer;
	m_eIndexBufferFormat = Format;
	m_uIndexBufferOffset = Offset;

	CheckCurrentContext(m_pContext);
	if (pDXGLIndexBuffer == NULL)
		m_pContext->SetIndexBuffer(NULL, GL_NONE, 0, 0);
	else
	{
		const NDrxOpenGL::SGIFormatInfo* pFormatInfo;
		NDrxOpenGL::EGIFormat eGIFormat(NDrxOpenGL::GetGIFormat(Format));
		if (eGIFormat == NDrxOpenGL::eGIF_NUM ||
		    (pFormatInfo = NDrxOpenGL::GetGIFormatInfo(eGIFormat)) == NULL ||
		    pFormatInfo->m_pTexture == NULL ||
		    pFormatInfo->m_pUncompressed == NULL)
		{
			DXGL_ERROR("Invalid format for index buffer");
			return;
		}
		m_pContext->SetIndexBuffer(pDXGLIndexBuffer->GetGLBuffer(), pFormatInfo->m_pTexture->m_eDataType, pFormatInfo->m_pUncompressed->GetPixelBytes(), Offset);
	}
}

void CDrxDXGLDeviceContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	CheckCurrentContext(m_pContext);
	CheckPipeline(m_kStages);
	m_pContext->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void CDrxDXGLDeviceContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	CheckCurrentContext(m_pContext);
	CheckPipeline(m_kStages);
	m_pContext->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void CDrxDXGLDeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology)
{
	CheckCurrentContext(m_pContext);
	m_ePrimitiveTopology = Topology;
	m_pContext->SetPrimitiveTopology(Topology);
}

void CDrxDXGLDeviceContext::Begin(ID3D11Asynchronous* pAsync)
{
	CheckCurrentContext(m_pContext);
#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery(m_pContext));
#else
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery());
#endif
	if (pQuery)
		pQuery->Begin();
}

void CDrxDXGLDeviceContext::End(ID3D11Asynchronous* pAsync)
{
	CheckCurrentContext(m_pContext);
#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery(m_pContext));
#else
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery());
#endif
	if (pQuery)
		pQuery->End();
}

HRESULT CDrxDXGLDeviceContext::GetData(ID3D11Asynchronous* pAsync, uk pData, UINT DataSize, UINT GetDataFlags)
{
	CheckCurrentContext(m_pContext);
#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery(m_pContext));
#else
	NDrxOpenGL::SQuery* pQuery(CDrxDXGLQuery::FromInterface(pAsync)->GetGLQuery());
#endif
	if (pQuery)
		return pQuery->GetData(pData, DataSize, (GetDataFlags & D3D11_ASYNC_GETDATA_DONOTFLUSH) == 0 ? true : false) ? S_OK : E_FAIL;
	return E_FAIL;
}

void CDrxDXGLDeviceContext::SetPredication(ID3D11Predicate* pPredicate, BOOL PredicateValue)
{
	if (pPredicate != NULL)
	{
		DXGL_NOT_IMPLEMENTED
	}
	m_spPredicate = CDrxDXGLQuery::FromInterface(pPredicate);
	m_bPredicateValue = PredicateValue == TRUE;
}

void CDrxDXGLDeviceContext::OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView)
{
	OMSetRenderTargetsAndUnorderedAccessViews(NumViews, ppRenderTargetViews, pDepthStencilView, NumViews, 0, NULL, NULL);
}

void CDrxDXGLDeviceContext::OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
	NDrxOpenGL::SOutputMergerView* apGLRenderTargetViews[DXGL_ARRAY_SIZE(m_aspRenderTargetViews)];
	NDrxOpenGL::SOutputMergerView* pGLDepthStencilView(NULL);

	for (u32 uRTV = 0; uRTV < NumRTVs; ++uRTV)
	{
		CDrxDXGLRenderTargetView* pDXGLRenderTargetView(CDrxDXGLRenderTargetView::FromInterface(ppRenderTargetViews[uRTV]));
		m_aspRenderTargetViews[uRTV] = pDXGLRenderTargetView;
#if OGL_SINGLE_CONTEXT
		apGLRenderTargetViews[uRTV] = pDXGLRenderTargetView == NULL ? NULL : pDXGLRenderTargetView->GetGLView(m_pContext);
#else
		apGLRenderTargetViews[uRTV] = pDXGLRenderTargetView == NULL ? NULL : pDXGLRenderTargetView->GetGLView();
#endif
	}
	if (UAVStartSlot == NumRTVs)
	{
		SetUnorderedAccessViews(NDrxOpenGL::eST_Fragment, 0, NumUAVs, ppUnorderedAccessViews);
	}
	else
	{
		DXGL_ERROR("CDrxDXGLDeviceContext::OMSetRenderTargetsAndUnorderedAccessViews - UAVStartSlot is expected to be equal to NumRTVs");
		NumUAVs = 0;
	}

	for (u32 uRTV = NumRTVs; uRTV < DXGL_ARRAY_SIZE(m_aspRenderTargetViews); ++uRTV)
	{
		m_aspRenderTargetViews[uRTV] = NULL;
		apGLRenderTargetViews[uRTV] = NULL;
	}

	CDrxDXGLDepthStencilView* pDXGLDepthStencilView(CDrxDXGLDepthStencilView::FromInterface(pDepthStencilView));
	m_spDepthStencilView = pDXGLDepthStencilView;
	if (pDXGLDepthStencilView != NULL)
#if OGL_SINGLE_CONTEXT
		pGLDepthStencilView = pDXGLDepthStencilView->GetGLView(m_pContext);
#else
		pGLDepthStencilView = pDXGLDepthStencilView->GetGLView();
#endif

#if DXGL_CHECK_HAZARDS
	for (u32 uStage = 0; uStage < m_kStages.size(); ++uStage)
	{
		CheckHazards(m_aspRenderTargetViews, DXGL_ARRAY_SIZE(m_aspRenderTargetViews), m_kStages.at(uStage).m_aspShaderResourceViews, DXGL_ARRAY_SIZE(m_kStages.at(uStage).m_aspShaderResourceViews), uStage);
		CheckHazards(&m_spDepthStencilView, 1, m_kStages.at(uStage).m_aspShaderResourceViews, DXGL_ARRAY_SIZE(m_kStages.at(uStage).m_aspShaderResourceViews), uStage);
	}
#endif //DXGL_CHECK_HAZARDS

	CheckCurrentContext(m_pContext);

	m_pContext->SetRenderTargets(NumRTVs, apGLRenderTargetViews, pGLDepthStencilView);
}

void CDrxDXGLDeviceContext::OMSetBlendState(ID3D11BlendState* pBlendState, const FLOAT BlendFactor[4], UINT SampleMask)
{
	CheckCurrentContext(m_pContext);

	CDrxDXGLBlendState* pDXGLBlendState(
	  pBlendState == NULL ?
	  m_spDefaultBlendState.get() :
	  CDrxDXGLBlendState::FromInterface(pBlendState));

	if (pDXGLBlendState != m_spBlendState)
	{
		m_spBlendState = pDXGLBlendState;
		pDXGLBlendState->Apply(m_pContext);
	}

	m_uSampleMask = SampleMask;
	if (BlendFactor == NULL)
	{
		m_auBlendFactor[0] = 1.0f;
		m_auBlendFactor[1] = 1.0f;
		m_auBlendFactor[2] = 1.0f;
		m_auBlendFactor[3] = 1.0f;
	}
	else
	{
		m_auBlendFactor[0] = BlendFactor[0];
		m_auBlendFactor[1] = BlendFactor[1];
		m_auBlendFactor[2] = BlendFactor[2];
		m_auBlendFactor[3] = BlendFactor[3];
	}

	m_pContext->SetBlendColor(m_auBlendFactor[0], m_auBlendFactor[1], m_auBlendFactor[2], m_auBlendFactor[3]);
	m_pContext->SetSampleMask(m_uSampleMask);
}

void CDrxDXGLDeviceContext::OMSetDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
{
	CheckCurrentContext(m_pContext);

	CDrxDXGLDepthStencilState* pDXGLDepthStencilState(
	  pDepthStencilState == NULL ?
	  m_spDefaultDepthStencilState.get() :
	  CDrxDXGLDepthStencilState::FromInterface(pDepthStencilState));

	if (pDXGLDepthStencilState != m_spDepthStencilState || m_uStencilRef != StencilRef)
	{
		m_spDepthStencilState = pDXGLDepthStencilState;
		m_uStencilRef = StencilRef;
		pDXGLDepthStencilState->Apply(StencilRef, m_pContext);
	}
}

void CDrxDXGLDeviceContext::SOSetTargets(UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets)
{
	for (u32 uBuffer = 0; uBuffer < D3D11_SO_BUFFER_SLOT_COUNT; ++uBuffer)
	{
		CDrxDXGLBuffer* pDXGLSOBuffer(NULL);
		u32 uOffset(0);
		if (uBuffer < NumBuffers)
		{
			pDXGLSOBuffer = CDrxDXGLBuffer::FromInterface(ppSOTargets[uBuffer]);
			uOffset = pOffsets[uBuffer];
		}

		if (m_aspStreamOutputBuffers[uBuffer] != pDXGLSOBuffer ||
		    m_auStreamOutputBufferOffsets[uBuffer] != uOffset)
		{
			DXGL_NOT_IMPLEMENTED
			  m_aspStreamOutputBuffers[uBuffer] = pDXGLSOBuffer;
			m_auStreamOutputBufferOffsets[uBuffer] = uOffset;
		}
	}
}

void CDrxDXGLDeviceContext::DrawAuto(void)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::DrawIndexedInstancedIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::DrawInstancedIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
#if DXGL_SUPPORT_COMPUTE
	CheckCurrentContext(m_pContext);
	m_pContext->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
#else
	DXGL_ERROR("CDrxDXGLDeviceContext::Dispatch is not supported in this configuration");
#endif
}

void CDrxDXGLDeviceContext::DispatchIndirect(ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState)
{
	CDrxDXGLRasterizerState* pDXGLRasterizerState(
	  pRasterizerState == NULL ?
	  m_spDefaultRasterizerState.get() :
	  CDrxDXGLRasterizerState::FromInterface(pRasterizerState));

	if (pDXGLRasterizerState != m_spRasterizerState)
	{
		CheckCurrentContext(m_pContext);
		m_spRasterizerState = pDXGLRasterizerState;
		m_spRasterizerState->Apply(m_pContext);
	}
}

void CDrxDXGLDeviceContext::RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports)
{
	m_uNumViewports = NumViewports;
	NDrxOpenGL::Memcpy(m_akViewports, pViewports, min(sizeof(m_akViewports), sizeof(pViewports[0]) * NumViewports));

	CheckCurrentContext(m_pContext);
	m_pContext->SetViewports(NumViewports, pViewports);
}

void CDrxDXGLDeviceContext::RSSetScissorRects(UINT NumRects, const D3D11_RECT* pRects)
{
	m_uNumScissorRects = NumRects;
	if (NumRects > 0)
		NDrxOpenGL::Memcpy(m_akScissorRects, pRects, min(sizeof(m_akScissorRects), sizeof(pRects[0]) * NumRects));

	CheckCurrentContext(m_pContext);
	m_pContext->SetScissorRects(NumRects, pRects);
}

void CDrxDXGLDeviceContext::CopySubresourceRegion(ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
{
	CDrxDXGLResource* pDXGLDstResource(CDrxDXGLResource::FromInterface(pDstResource));
	CDrxDXGLResource* pDXGLSrcResource(CDrxDXGLResource::FromInterface(pSrcResource));

	CheckCurrentContext(m_pContext);
	D3D11_RESOURCE_DIMENSION kDstType, kSrcType;
	pDXGLDstResource->GetType(&kDstType);
	pDXGLSrcResource->GetType(&kSrcType);

	if (kDstType == kSrcType)
	{
		switch (kDstType)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			NDrxOpenGL::CopySubTexture(
			  static_cast<NDrxOpenGL::STexture*>(pDXGLDstResource->GetGLResource()), DstSubresource, DstX, DstY, DstZ,
			  static_cast<NDrxOpenGL::STexture*>(pDXGLSrcResource->GetGLResource()), SrcSubresource, pSrcBox, m_pContext);
			break;
		case D3D11_RESOURCE_DIMENSION_BUFFER:
			NDrxOpenGL::CopySubBuffer(
			  static_cast<NDrxOpenGL::SBuffer*>(pDXGLDstResource->GetGLResource()), DstSubresource, DstX, DstY, DstZ,
			  static_cast<NDrxOpenGL::SBuffer*>(pDXGLSrcResource->GetGLResource()), SrcSubresource, pSrcBox, m_pContext);
			break;
		default:
			assert(false);
			break;
		}
	}
}

void CDrxDXGLDeviceContext::CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
{
	CDrxDXGLResource* pDXGLDstResource(CDrxDXGLResource::FromInterface(pDstResource));
	CDrxDXGLResource* pDXGLSrcResource(CDrxDXGLResource::FromInterface(pSrcResource));

	D3D11_RESOURCE_DIMENSION kDstType, kSrcType;
	pDXGLDstResource->GetType(&kDstType);
	pDXGLSrcResource->GetType(&kSrcType);

	if (kDstType == kSrcType)
	{
		CheckCurrentContext(m_pContext);
		switch (kDstType)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			NDrxOpenGL::CopyTexture(
			  static_cast<NDrxOpenGL::STexture*>(pDXGLDstResource->GetGLResource()),
			  static_cast<NDrxOpenGL::STexture*>(pDXGLSrcResource->GetGLResource()),
			  m_pContext);
			break;
		case D3D11_RESOURCE_DIMENSION_BUFFER:
			NDrxOpenGL::CopyBuffer(
			  static_cast<NDrxOpenGL::SBuffer*>(pDXGLDstResource->GetGLResource()),
			  static_cast<NDrxOpenGL::SBuffer*>(pDXGLSrcResource->GetGLResource()),
			  m_pContext);
			break;
		default:
			assert(false);
			break;
		}
	}
	else
	{
		DXGL_ERROR("CopyResource failed - source and destination are resources of different type");
	}
}

void CDrxDXGLDeviceContext::UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, ukk pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
{
	CheckCurrentContext(m_pContext);
	NDrxOpenGL::SResource* pGLResource(CDrxDXGLResource::FromInterface(pDstResource)->GetGLResource());
	if (pGLResource->m_pfUpdateSubresource)
	{
		(*pGLResource->m_pfUpdateSubresource)(pGLResource, DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch, m_pContext);
	}
	else
	{
		DXGL_NOT_IMPLEMENTED
	}
}

void CDrxDXGLDeviceContext::CopyStructureCount(ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[4])
{
	CheckCurrentContext(m_pContext);
	CDrxDXGLRenderTargetView* pDXGLRenderTargetView(CDrxDXGLRenderTargetView::FromInterface(pRenderTargetView));
#if OGL_SINGLE_CONTEXT
	m_pContext->ClearRenderTarget(pDXGLRenderTargetView == NULL ? NULL : pDXGLRenderTargetView->GetGLView(m_pContext), ColorRGBA);
#else
	m_pContext->ClearRenderTarget(pDXGLRenderTargetView == NULL ? NULL : pDXGLRenderTargetView->GetGLView(), ColorRGBA);
#endif
}

void CDrxDXGLDeviceContext::ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[4])
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[4])
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
{
	CheckCurrentContext(m_pContext);
	CDrxDXGLDepthStencilView* pDXGLDepthStencilView(CDrxDXGLDepthStencilView::FromInterface(pDepthStencilView));
#if OGL_SINGLE_CONTEXT
	m_pContext->ClearDepthStencil(pDXGLDepthStencilView == NULL ? NULL : pDXGLDepthStencilView->GetGLView(m_pContext), (ClearFlags & D3D11_CLEAR_DEPTH) != 0, (ClearFlags & D3D11_CLEAR_STENCIL) != 0, Depth, Stencil);
#else
	m_pContext->ClearDepthStencil(pDXGLDepthStencilView == NULL ? NULL : pDXGLDepthStencilView->GetGLView(), (ClearFlags & D3D11_CLEAR_DEPTH) != 0, (ClearFlags & D3D11_CLEAR_STENCIL) != 0, Depth, Stencil);
#endif
}

void CDrxDXGLDeviceContext::GenerateMips(ID3D11ShaderResourceView* pShaderResourceView)
{
	CheckCurrentContext(m_pContext);
#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SShaderView* pGLView(CDrxDXGLShaderResourceView::FromInterface(pShaderResourceView)->GetGLView(m_pContext));
#else
	NDrxOpenGL::SShaderView* pGLView(CDrxDXGLShaderResourceView::FromInterface(pShaderResourceView)->GetGLView());
#endif
	switch (pGLView->m_eType)
	{
	case NDrxOpenGL::SShaderView::eSVT_Texture:
		static_cast<NDrxOpenGL::SShaderTextureBasedView*>(pGLView)->GenerateMipmaps(m_pContext);
		break;
	default:
		DXGL_ERROR("Generation of mipmaps for this type of resource view is not supported");
		break;
	}
}

void CDrxDXGLDeviceContext::SetResourceMinLOD(ID3D11Resource* pResource, FLOAT MinLOD)
{
	DXGL_NOT_IMPLEMENTED
}

FLOAT CDrxDXGLDeviceContext::GetResourceMinLOD(ID3D11Resource* pResource)
{
	DXGL_NOT_IMPLEMENTED
	return 0.0f;
}

void CDrxDXGLDeviceContext::ResolveSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::ExecuteCommandList(ID3D11CommandList* pCommandList, BOOL RestoreContextState)
{
	DXGL_NOT_IMPLEMENTED
}

void CDrxDXGLDeviceContext::CSSetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
{
#if DXGL_SUPPORT_COMPUTE
	SetUnorderedAccessViews(NDrxOpenGL::eST_Compute, StartSlot, NumUAVs, ppUnorderedAccessViews);
#else
	DXGL_ERROR("CDrxDXGLDeviceContext::CSSetUnorderedAccessViews is not supported in this configuration");
#endif //DXGL_SUPPORT_COMPUTE
}

void CDrxDXGLDeviceContext::IAGetInputLayout(ID3D11InputLayout** ppInputLayout)
{
	CDrxDXGLInputLayout::ToInterface(ppInputLayout, m_spInputLayout);
	if ((*ppInputLayout) != NULL)
		(*ppInputLayout)->AddRef();
}

void CDrxDXGLDeviceContext::IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
{
	u32 uSlot;
	for (uSlot = 0; uSlot < NumBuffers; ++uSlot)
	{
		u32 uSlotIndex(StartSlot + uSlot);

		CDrxDXGLBuffer::ToInterface(ppVertexBuffers + uSlot, m_aspVertexBuffers[uSlotIndex]);
		if (ppVertexBuffers[uSlot] != NULL)
			ppVertexBuffers[uSlot]->AddRef();
		pStrides[uSlot] = m_auVertexBufferStrides[uSlotIndex];
		pOffsets[uSlot] = m_auVertexBufferOffsets[uSlotIndex];
	}
}

void CDrxDXGLDeviceContext::IAGetIndexBuffer(ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset)
{
	CDrxDXGLBuffer::ToInterface(pIndexBuffer, m_spIndexBuffer);
	if ((*pIndexBuffer) != NULL)
		(*pIndexBuffer)->AddRef();
	*Format = m_eIndexBufferFormat;
	*Offset = m_uIndexBufferOffset;
}

void CDrxDXGLDeviceContext::IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* pTopology)
{
	*pTopology = m_ePrimitiveTopology;
}

void CDrxDXGLDeviceContext::GetPredication(ID3D11Predicate** ppPredicate, BOOL* pPredicateValue)
{
	ID3D11Query* pQuery;
	CDrxDXGLQuery::ToInterface(&pQuery, m_spPredicate);
	*ppPredicate = static_cast<ID3D11Predicate*>(pQuery);
	*pPredicateValue = (m_bPredicateValue ? TRUE : FALSE);
}

void CDrxDXGLDeviceContext::OMGetRenderTargets(UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView)
{
	OMGetRenderTargetsAndUnorderedAccessViews(NumViews, ppRenderTargetViews, ppDepthStencilView, 0, 0, NULL);
}

void CDrxDXGLDeviceContext::OMGetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
	for (u32 uRTV = 0; uRTV < NumRTVs; ++uRTV)
	{
		CDrxDXGLRenderTargetView::ToInterface(ppRenderTargetViews + uRTV, m_aspRenderTargetViews[uRTV]);
		if (ppRenderTargetViews[uRTV] != NULL)
			ppRenderTargetViews[uRTV]->AddRef();
	}
	CDrxDXGLDepthStencilView::ToInterface(ppDepthStencilView, m_spDepthStencilView);
	if ((*ppDepthStencilView) != NULL)
		(*ppDepthStencilView)->AddRef();

	GetUnorderedAccesses(NDrxOpenGL::eST_Fragment, 0, NumUAVs, ppUnorderedAccessViews);
}

void CDrxDXGLDeviceContext::OMGetBlendState(ID3D11BlendState** ppBlendState, FLOAT BlendFactor[4], UINT* pSampleMask)
{
	CDrxDXGLBlendState::ToInterface(ppBlendState, m_spBlendState);
	if ((*ppBlendState) != NULL)
		(*ppBlendState)->AddRef();
	BlendFactor[0] = m_auBlendFactor[0];
	BlendFactor[1] = m_auBlendFactor[1];
	BlendFactor[2] = m_auBlendFactor[2];
	BlendFactor[3] = m_auBlendFactor[3];
	*pSampleMask = m_uSampleMask;
}

void CDrxDXGLDeviceContext::OMGetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
{
	CDrxDXGLDepthStencilState::ToInterface(ppDepthStencilState, m_spDepthStencilState);
	if ((*ppDepthStencilState) != NULL)
		(*ppDepthStencilState)->AddRef();
	*pStencilRef = m_uStencilRef;
}

void CDrxDXGLDeviceContext::SOGetTargets(UINT NumBuffers, ID3D11Buffer** ppSOTargets)
{
	for (u32 uBuffer = 0; uBuffer < NumBuffers; ++uBuffer)
	{
		CDrxDXGLBuffer::ToInterface(ppSOTargets + uBuffer, m_aspStreamOutputBuffers[uBuffer]);
		if (ppSOTargets[uBuffer] != NULL)
			ppSOTargets[uBuffer]->AddRef();
	}
}

void CDrxDXGLDeviceContext::RSGetState(ID3D11RasterizerState** ppRasterizerState)
{
	CDrxDXGLRasterizerState::ToInterface(ppRasterizerState, m_spRasterizerState);
	if ((*ppRasterizerState) != NULL)
		(*ppRasterizerState)->AddRef();
}

void CDrxDXGLDeviceContext::RSGetViewports(UINT* pNumViewports, D3D11_VIEWPORT* pViewports)
{
	if (pViewports != NULL)
		NDrxOpenGL::Memcpy(pViewports, m_akViewports, min(sizeof(m_akViewports), sizeof(pViewports[0]) * *pNumViewports));
	*pNumViewports = m_uNumViewports;
}

void CDrxDXGLDeviceContext::RSGetScissorRects(UINT* pNumRects, D3D11_RECT* pRects)
{
	if (pRects != NULL)
		NDrxOpenGL::Memcpy(pRects, m_akScissorRects, min(sizeof(m_akScissorRects), sizeof(pRects[0]) * *pNumRects));
	*pNumRects = m_uNumScissorRects;
}

void CDrxDXGLDeviceContext::CSGetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
{
#if DXGL_SUPPORT_COMPUTE
	GetUnorderedAccesses(NDrxOpenGL::eST_Compute, StartSlot, NumUAVs, ppUnorderedAccessViews);
#else
	DXGL_ERROR("CDrxDXGLDeviceContext::CSGetUnorderedAccessViews is not supported in this configuration");
#endif //DXGL_SUPPORT_COMPUTE
}

void CDrxDXGLDeviceContext::ClearState(void)
{
	CheckCurrentContext(m_pContext);

	// Common shader state
	for (u32 uStage = 0; uStage < m_kStages.size(); ++uStage)
	{
		SStage& kStage(m_kStages.at(uStage));

		for (u32 uSRV = 0; uSRV < DXGL_ARRAY_SIZE(kStage.m_aspShaderResourceViews); ++uSRV)
		{
			if (kStage.m_aspShaderResourceViews[uSRV] != NULL)
			{
				m_pContext->SetShaderTexture(NULL, uStage, uSRV);
				kStage.m_aspShaderResourceViews[uSRV] = NULL;
			}
		}

#if DXGL_SUPPORT_SHADER_IMAGES
		for (u32 uUAV = 0; uUAV < DXGL_ARRAY_SIZE(kStage.m_aspUnorderedAccessViews); ++uUAV)
		{
			if (kStage.m_aspUnorderedAccessViews[uUAV] != NULL)
			{
				m_pContext->SetShaderImage(NULL, uStage, uUAV);
				kStage.m_aspUnorderedAccessViews[uUAV] = NULL;
			}
		}
#endif //DXGL_SUPPORT_SHADER_IMAGES

		for (u32 uSampler = 0; uSampler < DXGL_ARRAY_SIZE(kStage.m_aspSamplerStates); ++uSampler)
		{
			if (kStage.m_aspSamplerStates[uSampler] != m_spDefaultSamplerState)
			{
				m_spDefaultSamplerState->Apply(uStage, uSampler, m_pContext);
				kStage.m_aspSamplerStates[uSampler] = m_spDefaultSamplerState;
			}
		}

		for (u32 uCB = 0; uCB < DXGL_ARRAY_SIZE(kStage.m_aspConstantBuffers); ++uCB)
		{
			if (kStage.m_aspConstantBuffers[uCB] != NULL)
			{
				m_pContext->SetConstantBuffer(NULL, NDrxOpenGL::SBufferRange(0, 0), uStage, uCB);
				kStage.m_aspConstantBuffers[uCB] = NULL;
				kStage.m_auConstantBufferOffsets[uCB] = 0;
				kStage.m_auConstantBufferSizes[uCB] = 0;
			}
		}

		if (m_kStages[uStage].m_spShader != NULL)
		{
			m_pContext->SetShader(NULL, uStage);
			m_kStages[uStage].m_spShader = NULL;
		}
	}

	// Vertex buffers
	for (u32 uVB = 0; uVB < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; ++uVB)
	{
		if (m_aspVertexBuffers[uVB] != NULL ||
		    m_auVertexBufferStrides[uVB] != 0 ||
		    m_auVertexBufferOffsets[uVB] != 0)
		{
			m_pContext->SetVertexBuffer(uVB, NULL, 0, 0);
			m_aspVertexBuffers[uVB] = NULL;
			m_auVertexBufferStrides[uVB] = 0;
			m_auVertexBufferOffsets[uVB] = 0;
		}
	}

	// Index buffer
	if (m_spIndexBuffer != NULL ||
	    m_eIndexBufferFormat != DXGI_FORMAT_UNKNOWN ||
	    m_uIndexBufferOffset != 0)
	{
		m_pContext->SetIndexBuffer(NULL, GL_NONE, 0, 0);
		m_spIndexBuffer = NULL;
		m_eIndexBufferFormat = DXGI_FORMAT_UNKNOWN;
		m_uIndexBufferOffset = 0;
	}

	// Input layout
	if (m_spInputLayout != NULL)
	{
		m_pContext->SetInputLayout(NULL);
		m_spInputLayout = NULL;
	}

	// Primitive topology
	if (m_ePrimitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
	{
		m_pContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
		m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}

	// Output merger state
	OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	OMSetDepthStencilState(NULL, 0);
	OMSetRenderTargetsAndUnorderedAccessViews(0, NULL, NULL, 0, 0, NULL, NULL);

	// Rasterizer state
	m_uNumScissorRects = 0;
	m_pContext->SetScissorRects(0, NULL);
	m_uNumViewports;
	m_pContext->SetViewports(0, NULL);
	RSSetState(NULL);

	// Predication
	SetPredication(NULL, FALSE);

	// Stream output
	SOSetTargets(0, NULL, NULL);
}

void CDrxDXGLDeviceContext::Flush(void)
{
	CheckCurrentContext(m_pContext);
	m_pContext->Flush();
}

D3D11_DEVICE_CONTEXT_TYPE CDrxDXGLDeviceContext::GetType(void)
{
	DXGL_TODO("Modify when deferred contexts are supported")
	return D3D11_DEVICE_CONTEXT_IMMEDIATE;
}

UINT CDrxDXGLDeviceContext::GetContextFlags(void)
{
	return 0;
}

HRESULT CDrxDXGLDeviceContext::FinishCommandList(BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}
