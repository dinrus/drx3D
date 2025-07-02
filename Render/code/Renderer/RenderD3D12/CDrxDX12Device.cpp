// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12Device.hpp>

#include <drx3D/Render/CDrxDX12DeviceContext.hpp>

#include <drx3D/Render/DX12/GI/CDrxDX12GIAdapter.hpp>

#include <drx3D/Render/DX12/Resource/Misc/CDrxDX12Buffer.hpp>
#include <drx3D/Render/DX12/Resource/Misc/CDrxDX12InputLayout.hpp>
#include <drx3D/Render/DX12/Resource/Misc/CDrxDX12Query.hpp>
#include <drx3D/Render/DX12/Resource/Misc/CDrxDX12Shader.hpp>

#include <drx3D/Render/DX12/Resource/State/CDrxDX12BlendState.hpp>
#include <drx3D/Render/DX12/Resource/State/CDrxDX12DepthStencilState.hpp>
#include <drx3D/Render/DX12/Resource/State/CDrxDX12RasterizerState.hpp>
#include <drx3D/Render/DX12/Resource/State/CDrxDX12SamplerState.hpp>

#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture1D.hpp>
#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture2D.hpp>
#include <drx3D/Render/DX12/Resource/Texture/CDrxDX12Texture3D.hpp>

#include <drx3D/Render/DX12/Resource/View/CDrxDX12DepthStencilView.hpp>
#include <drx3D/Render/DX12/Resource/View/CDrxDX12RenderTargetView.hpp>
#include <drx3D/Render/DX12/Resource/View/CDrxDX12ShaderResourceView.hpp>
#include <drx3D/Render/DX12/Resource/View/CDrxDX12UnorderedAccessView.hpp>

CDrxDX12Device* CDrxDX12Device::Create(CDrxDX12GIAdapter* pAdapter, D3D_FEATURE_LEVEL* pFeatureLevel)
{
	DX12_PTR(NDrxDX12::CDevice) device = NDrxDX12::CDevice::Create(pAdapter, pFeatureLevel);

	if (!device)
	{
		DX12_ERROR("Could not create DX12 Device!");
		return NULL;
	}

	return DX12_NEW_RAW(CDrxDX12Device(device));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12Device::CDrxDX12Device(NDrxDX12::CDevice* device)
	: Super()
	, m_pDevice(device)
{
	DX12_FUNC_LOG
		
#ifdef DX12_LINKEDADAPTER
	// TODO: CVar ...
	if (CRenderer::CV_r_StereoEnableMgpu)
	{
		const UINT numNodes = m_numNodes = m_pDevice->GetNodeCount();
		const UINT allMask = m_allMask = (1UL << numNodes) - 1UL;
		const UINT crtMask = m_crtMask = allMask;
		const UINT visMask = m_visMask = allMask;
		const UINT shrMask = m_shrMask = 1U;

		m_pMainContext = CDrxDX12DeviceContext::Create(this, allMask, false);

		//		m_pNodeContexts.reserve(numNodes);
		//		for (UINT c = 0, n = numNodes; c < n; ++c)
		//			m_pNodeContexts.push_back(CDrxDX12DeviceContext::Create(this, 1UL << c, false));
	}
	else
#endif
	{
		const UINT numNodes = m_numNodes = 1U;
		const UINT allMask = m_allMask = (1UL << numNodes) - 1UL;
		const UINT crtMask = m_crtMask = 1U;
		const UINT visMask = m_visMask = 1U;
		const UINT shrMask = m_shrMask = 1U;

		m_pMainContext = CDrxDX12DeviceContext::Create(this, allMask, false);
	}
	//report the node count used
	gRenDev->m_adapterInfo.nNodeCount = m_numNodes;
}

#pragma region /* ID3D11Device implementation */

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateBuffer(
  _In_ const D3D11_BUFFER_DESC* pDesc,
  _In_opt_ const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Buffer** ppBuffer)
{
	*ppBuffer = CDrxDX12Buffer::Create(this, pDesc, pInitialData);
	return *ppBuffer ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTexture1D(
  _In_ const D3D11_TEXTURE1D_DESC* pDesc,
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize)) const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture1D** ppTexture1D)
{
	DX12_FUNC_LOG
	* ppTexture1D = CDrxDX12Texture1D::Create(this, nullptr, pDesc, pInitialData);
	return *ppTexture1D ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTexture2D(
  _In_ const D3D11_TEXTURE2D_DESC* pDesc,
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize)) const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture2D** ppTexture2D)
{
	DX12_FUNC_LOG
	* ppTexture2D = CDrxDX12Texture2D::Create(this, nullptr, pDesc, pInitialData);
	return *ppTexture2D ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTexture3D(
  _In_ const D3D11_TEXTURE3D_DESC* pDesc,
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels))  const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture3D** ppTexture3D)
{
	DX12_FUNC_LOG
	* ppTexture3D = CDrxDX12Texture3D::Create(this, nullptr, pDesc, pInitialData);
	return *ppTexture3D ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateShaderResourceView(
  _In_ ID3D11Resource* pResource,
  _In_opt_ const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
  _Out_opt_ ID3D11ShaderResourceView** ppSRView)
{
	DX12_FUNC_LOG
	* ppSRView = NULL;

	if (CDrxDX12ShaderResourceView* pResult = CDrxDX12ShaderResourceView::Create(this, pResource, pDesc))
	{
		auto descriptorHandle = GetDX12Device()->CacheShaderResourceView(&pResult->GetDX12View().GetSRVDesc(), DX12_EXTRACT_D3D12RESOURCE(pResource));
		pResult->GetDX12View().SetDescriptorHandle(descriptorHandle);

		if (INVALID_CPU_DESCRIPTOR_HANDLE == descriptorHandle)
			SAFE_RELEASE(pResult);

		*ppSRView = pResult;
	}

	return *ppSRView ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateUnorderedAccessView(
  _In_ ID3D11Resource* pResource,
  _In_opt_ const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc,
  _Out_opt_ ID3D11UnorderedAccessView** ppUAView)
{
	DX12_FUNC_LOG
	* ppUAView = NULL;

	if (CDrxDX12UnorderedAccessView* pResult = CDrxDX12UnorderedAccessView::Create(this, pResource, pDesc))
	{
		auto descriptorHandle = GetDX12Device()->CacheUnorderedAccessView(&pResult->GetDX12View().GetUAVDesc(), DX12_EXTRACT_D3D12RESOURCE(pResource));
		pResult->GetDX12View().SetDescriptorHandle(descriptorHandle);

		if (INVALID_CPU_DESCRIPTOR_HANDLE == descriptorHandle)
			SAFE_RELEASE(pResult);

		*ppUAView = pResult;
	}

	return *ppUAView ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateRenderTargetView(
  _In_ ID3D11Resource* pResource,
  _In_opt_ const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
  _Out_opt_ ID3D11RenderTargetView** ppRTView)
{
	DX12_FUNC_LOG
	* ppRTView = NULL;

	if (CDrxDX12RenderTargetView* pResult = CDrxDX12RenderTargetView::Create(this, pResource, pDesc))
	{
		auto descriptorHandle = GetDX12Device()->CacheRenderTargetView(&pResult->GetDX12View().GetRTVDesc(), DX12_EXTRACT_D3D12RESOURCE(pResource));
		pResult->GetDX12View().SetDescriptorHandle(descriptorHandle);

		if (INVALID_CPU_DESCRIPTOR_HANDLE == descriptorHandle)
			SAFE_RELEASE(pResult);

		*ppRTView = pResult;
	}

	return *ppRTView ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDepthStencilView(
  _In_ ID3D11Resource* pResource,
  _In_opt_ const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
  _Out_opt_ ID3D11DepthStencilView** ppDSView)
{
	DX12_FUNC_LOG
	* ppDSView = NULL;

	if (CDrxDX12DepthStencilView* pResult = CDrxDX12DepthStencilView::Create(this, pResource, pDesc))
	{
		auto descriptorHandle = GetDX12Device()->CacheDepthStencilView(&pResult->GetDX12View().GetDSVDesc(), DX12_EXTRACT_D3D12RESOURCE(pResource));
		pResult->GetDX12View().SetDescriptorHandle(descriptorHandle);

		if (INVALID_CPU_DESCRIPTOR_HANDLE == descriptorHandle)
			SAFE_RELEASE(pResult);

		*ppDSView = pResult;
	}

	return *ppDSView ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateInputLayout(
  _In_reads_(NumElements)  const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
  _In_range_(0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT)  UINT NumElements,
  _In_ ukk pShaderBytecodeWithInputSignature,
  _In_ SIZE_T BytecodeLength,
  _Out_opt_ ID3D11InputLayout** ppInputLayout)
{
	DX12_FUNC_LOG
	* ppInputLayout = CDrxDX12InputLayout::Create(this, pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength);
	return *ppInputLayout ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateVertexShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11VertexShader** ppVertexShader)
{
	DX12_FUNC_LOG
	* ppVertexShader = reinterpret_cast<ID3D11VertexShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppVertexShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateGeometryShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11GeometryShader** ppGeometryShader)
{
	DX12_FUNC_LOG
	* ppGeometryShader = reinterpret_cast<ID3D11GeometryShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppGeometryShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateGeometryShaderWithStreamOutput(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_reads_opt_(NumEntries)  const D3D11_SO_DECLARATION_ENTRY* pSODeclaration,
  _In_range_(0, D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT)  UINT NumEntries,
  _In_reads_opt_(NumStrides)  const UINT* pBufferStrides,
  _In_range_(0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumStrides,
  _In_ UINT RasterizedStream,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11GeometryShader** ppGeometryShader)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreatePixelShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11PixelShader** ppPixelShader)
{
	DX12_FUNC_LOG
	* ppPixelShader = reinterpret_cast<ID3D11PixelShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppPixelShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateHullShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11HullShader** ppHullShader)
{
	DX12_FUNC_LOG
	* ppHullShader = reinterpret_cast<ID3D11HullShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppHullShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDomainShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11DomainShader** ppDomainShader)
{
	DX12_FUNC_LOG
	* ppDomainShader = reinterpret_cast<ID3D11DomainShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppDomainShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateComputeShader(
  _In_ ukk pShaderBytecode,
  _In_ SIZE_T BytecodeLength,
  _In_opt_ ID3D11ClassLinkage* pClassLinkage,
  _Out_opt_ ID3D11ComputeShader** ppComputeShader)
{
	DX12_FUNC_LOG
	* ppComputeShader = reinterpret_cast<ID3D11ComputeShader*>(CDrxDX12Shader::Create(this, pShaderBytecode, BytecodeLength, pClassLinkage));
	return *ppComputeShader ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateClassLinkage(
  _Out_ ID3D11ClassLinkage** ppLinkage)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateBlendState(
  _In_ const D3D11_BLEND_DESC* pBlendStateDesc,
  _Out_opt_ ID3D11BlendState** ppBlendState)
{
	DX12_FUNC_LOG
	* ppBlendState = CDrxDX12BlendState::Create(pBlendStateDesc);
	return *ppBlendState ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDepthStencilState(
  _In_ const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc,
  _Out_opt_ ID3D11DepthStencilState** ppDepthStencilState)
{
	DX12_FUNC_LOG
	* ppDepthStencilState = CDrxDX12DepthStencilState::Create(pDepthStencilDesc);
	return *ppDepthStencilState ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateRasterizerState(
  _In_ const D3D11_RASTERIZER_DESC* pRasterizerDesc,
  _Out_opt_ ID3D11RasterizerState** ppRasterizerState)
{
	DX12_FUNC_LOG
	* ppRasterizerState = CDrxDX12RasterizerState::Create(pRasterizerDesc);
	return *ppRasterizerState ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateSamplerState(
  _In_ const D3D11_SAMPLER_DESC* pSamplerDesc,
  _Out_opt_ ID3D11SamplerState** ppSamplerState)
{
	DX12_FUNC_LOG
	* ppSamplerState = NULL;

	if (CDrxDX12SamplerState* pResult = CDrxDX12SamplerState::Create(pSamplerDesc))
	{
		auto descriptorHandle = GetDX12Device()->CacheSampler(&pResult->GetDX12SamplerState().GetSamplerDesc());
		pResult->GetDX12SamplerState().SetDescriptorHandle(descriptorHandle);

		if (INVALID_CPU_DESCRIPTOR_HANDLE == descriptorHandle)
			SAFE_RELEASE(pResult);

		*ppSamplerState = pResult;
	}

	return *ppSamplerState ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateQuery(
  _In_ const D3D11_QUERY_DESC* pQueryDesc,
  _Out_opt_ ID3D11Query** ppQuery)
{
	*ppQuery = CDrxDX12Query::Create(GetD3D12Device(), pQueryDesc);
	return *ppQuery ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreatePredicate(
  _In_ const D3D11_QUERY_DESC* pPredicateDesc,
  _Out_opt_ ID3D11Predicate** ppPredicate)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateCounter(
  _In_ const D3D11_COUNTER_DESC* pCounterDesc,
  _Out_opt_ ID3D11Counter** ppCounter)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDeferredContext(
  UINT ContextFlags,
  _Out_opt_ ID3D11DeviceContext** ppDeferredContext)
{
	DX12_FUNC_LOG

	if (ppDeferredContext)
	{
		*ppDeferredContext = new CDrxDX12DeviceContext(this, 0, true);
		return S_OK;
	}

	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::OpenSharedResource(
  _In_ HANDLE hResource,
  _In_ REFIID ReturnedInterface,
  _Out_optvoid** ppResource)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CheckFormatSupport(
  _In_ DXGI_FORMAT Format,
  _Out_ UINT* pFormatSupport)
{
	DX12_FUNC_LOG

	D3D12_FEATURE_DATA_FORMAT_SUPPORT data;
	data.Format = Format;

	if (S_OK != m_pDevice->GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &data, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)))
		return S_FALSE;

	*pFormatSupport = data.Support1;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CheckMultisampleQualityLevels(
  _In_ DXGI_FORMAT Format,
  _In_ UINT SampleCount,
  _Out_ UINT* pNumQualityLevels)
{
	DX12_FUNC_LOG

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS data;
	data.Format = Format;
	data.SampleCount = SampleCount;
	data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	if (S_OK != m_pDevice->GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)))
		return S_FALSE;

	*pNumQualityLevels = data.NumQualityLevels;
	return S_OK;
}

void STDMETHODCALLTYPE CDrxDX12Device::CheckCounterInfo(
  _Out_ D3D11_COUNTER_INFO* pCounterInfo)
{
	DX12_FUNC_LOG

}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CheckCounter(
  _In_ const D3D11_COUNTER_DESC* pDesc,
  _Out_ D3D11_COUNTER_TYPE* pType,
  _Out_ UINT* pActiveCounters,
  _Out_writes_opt_(*pNameLength)  LPSTR szName,
  _Inout_opt_ UINT* pNameLength,
  _Out_writes_opt_(*pUnitsLength)  LPSTR szUnits,
  _Inout_opt_ UINT* pUnitsLength,
  _Out_writes_opt_(*pDescriptionLength)  LPSTR szDescription,
  _Inout_opt_ UINT* pDescriptionLength)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CheckFeatureSupport(
  D3D11_FEATURE Feature,
  _Out_writes_bytes_(FeatureSupportDataSize)  uk pFeatureSupportData,
  UINT FeatureSupportDataSize)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::GetPrivateData(
  _In_ REFGUID guid,
  _Inout_ UINT* pDataSize,
  _Out_writes_bytes_opt_(*pDataSize)  uk pData)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::SetPrivateData(
  _In_ REFGUID guid,
  _In_ UINT DataSize,
  _In_reads_bytes_opt_(DataSize)  ukk pData)
{
	DX12_FUNC_LOG
	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::SetPrivateDataInterface(
  _In_ REFGUID guid,
  _In_opt_ const IUnknown* pData)
{
	DX12_FUNC_LOG
	return -1;
}

D3D_FEATURE_LEVEL STDMETHODCALLTYPE CDrxDX12Device::GetFeatureLevel()
{
	DX12_FUNC_LOG
	return D3D_FEATURE_LEVEL_11_1;
}

UINT STDMETHODCALLTYPE CDrxDX12Device::GetCreationFlags()
{
	DX12_FUNC_LOG
	return 0;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::GetDeviceRemovedReason()
{
	DX12_FUNC_LOG
	return -1;
}

void STDMETHODCALLTYPE CDrxDX12Device::GetImmediateContext(
  _Out_ ID3D11DeviceContext** ppImmediateContext)
{
	DX12_FUNC_LOG
	if (ppImmediateContext)
	{
		*ppImmediateContext = GetDeviceContext();
		(*ppImmediateContext)->AddRef();
	}
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::SetExceptionMode(
  UINT RaiseFlags)
{
	DX12_FUNC_LOG
	return -1;
}

UINT STDMETHODCALLTYPE CDrxDX12Device::GetExceptionMode()
{
	DX12_FUNC_LOG
	return 0;
}

#pragma endregion

#pragma region /* ID3D11Device1 implementation */

void STDMETHODCALLTYPE CDrxDX12Device::GetImmediateContext1(
  _Out_ ID3D11DeviceContext1** ppImmediateContext)
{
	DX12_FUNC_LOG

	if (ppImmediateContext)
	{
		*ppImmediateContext = GetDeviceContext();
		(*ppImmediateContext)->AddRef();
	}
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDeferredContext1(
  UINT ContextFlags,
  /* [annotation] */
  _COM_Outptr_opt_ ID3D11DeviceContext1** ppDeferredContext)
{
	DX12_FUNC_LOG

	if (ppDeferredContext)
	{
		*ppDeferredContext = new CDrxDX12DeviceContext(this, 0, true);
		return S_OK;
	}

	return -1;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateBlendState1(
  /* [annotation] */
  _In_ const D3D11_BLEND_DESC1* pBlendStateDesc,
  /* [annotation] */
  _COM_Outptr_opt_ ID3D11BlendState1** ppBlendState)
{
	DX12_FUNC_LOG
	  DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateRasterizerState1(
  /* [annotation] */
  _In_ const D3D11_RASTERIZER_DESC1* pRasterizerDesc,
  /* [annotation] */
  _COM_Outptr_opt_ ID3D11RasterizerState1** ppRasterizerState)
{
	DX12_FUNC_LOG
	  DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateDeviceContextState(
  UINT Flags,
  /* [annotation] */
  _In_reads_(FeatureLevels)  const D3D_FEATURE_LEVEL* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  REFIID EmulatedInterface,
  /* [annotation] */
  _Out_opt_ D3D_FEATURE_LEVEL* pChosenFeatureLevel,
  /* [annotation] */
  _Out_opt_ ID3DDeviceContextState** ppContextState)
{
	DX12_FUNC_LOG
	  DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::OpenSharedResource1(
  /* [annotation] */
  _In_ HANDLE hResource,
  /* [annotation] */
  _In_ REFIID returnedInterface,
  /* [annotation] */
  _COM_Outptrvoid** ppResource)
{
	DX12_FUNC_LOG
	  DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::OpenSharedResourceByName(
  /* [annotation] */
  _In_ LPCWSTR lpName,
  /* [annotation] */
  _In_ DWORD dwDesiredAccess,
  /* [annotation] */
  _In_ REFIID returnedInterface,
  /* [annotation] */
  _COM_Outptrvoid** ppResource)
{
	DX12_FUNC_LOG
	  DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

#pragma endregion

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTarget1D(
  _In_ const D3D11_TEXTURE1D_DESC* pDesc,
  _In_ const FLOAT cClearValue[4],
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize)) const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture1D** ppTexture1D)
{
	DX12_FUNC_LOG
	* ppTexture1D = CDrxDX12Texture1D::Create(this, cClearValue, pDesc, pInitialData);
	return *ppTexture1D ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTarget2D(
  _In_ const D3D11_TEXTURE2D_DESC* pDesc,
  _In_ const FLOAT cClearValue[4],
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels * pDesc->ArraySize)) const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture2D** ppTexture2D)
{
	DX12_FUNC_LOG
	* ppTexture2D = CDrxDX12Texture2D::Create(this, cClearValue, pDesc, pInitialData);
	return *ppTexture2D ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateTarget3D(
  _In_ const D3D11_TEXTURE3D_DESC* pDesc,
  _In_ const FLOAT cClearValue[4],
  _In_reads_opt_(_Inexpressible_(pDesc->MipLevels))  const D3D11_SUBRESOURCE_DATA* pInitialData,
  _Out_opt_ ID3D11Texture3D** ppTexture3D)
{
	DX12_FUNC_LOG
	* ppTexture3D = CDrxDX12Texture3D::Create(this, cClearValue, pDesc, pInitialData);
	return *ppTexture3D ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateNullResource(
  _In_ D3D11_RESOURCE_DIMENSION eType,
  _Out_opt_ ID3D11Resource** ppNullResource)
{
	DX12_FUNC_LOG
	switch (eType)
	{
	case D3D11_RESOURCE_DIMENSION_BUFFER:
		*ppNullResource = CDrxDX12Buffer::Create(this);
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		*ppNullResource = CDrxDX12Texture1D::Create(this);
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		*ppNullResource = CDrxDX12Texture2D::Create(this);
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		*ppNullResource = CDrxDX12Texture3D::Create(this);
		break;
	default:
		*ppNullResource = nullptr;
		break;
	}
	return *ppNullResource ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::ReleaseNullResource(
  _In_ ID3D11Resource* pNullResource)
{
	pNullResource->Release();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::CreateStagingResource(
  _In_ ID3D11Resource* pInputResource,
  _Out_opt_ ID3D11Resource** ppStagingResource,
  _In_ BOOL Upload)
{
	IDrxDX12Resource* dx12Resource = DX12_EXTRACT_IDRXDX12RESOURCE(pInputResource);
	NDrxDX12::CResource& rResource = dx12Resource->GetDX12Resource();
	ID3D12Resource* d3d12Resource = rResource.GetD3D12Resource();

	D3D12_RESOURCE_STATES initialState = Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;
	ID3D12Resource* stagingResource = nullptr;
	HRESULT result = GetDX12Device()->CreateOrReuseStagingResource(d3d12Resource, &stagingResource, Upload);

	if (result == S_OK && stagingResource != nullptr)
	{
		*ppStagingResource = CDrxDX12Buffer::Create(this, stagingResource, initialState);
		stagingResource->Release();

		return S_OK;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE CDrxDX12Device::ReleaseStagingResource(
  _In_ ID3D11Resource* pStagingResource)
{
	IDrxDX12Resource* dx12Resource = DX12_EXTRACT_IDRXDX12RESOURCE(pStagingResource);
	NDrxDX12::CResource& rResource = dx12Resource->GetDX12Resource();
	ID3D12Resource* d3d12Resource = rResource.GetD3D12Resource();

	GetDX12Device()->ReleaseLater(rResource.GetFenceValues(CMDTYPE_ANY), d3d12Resource);

	pStagingResource->Release();
	return S_OK;
}
