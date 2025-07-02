// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDevice.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11Device
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLBlendState.hpp>
#include <drx3D/Render/CDrxDXGLBuffer.hpp>
#include <drx3D/Render/CDrxDXGLDepthStencilState.hpp>
#include <drx3D/Render/CDrxDXGLDepthStencilView.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/CDrxDXGLDeviceContext.hpp>
#include <drx3D/Render/CDrxDXGLGIAdapter.hpp>
#include <drx3D/Render/CDrxDXGLInputLayout.hpp>
#include <drx3D/Render/CDrxDXGLQuery.hpp>
#include <drx3D/Render/CDrxDXGLRasterizerState.hpp>
#include <drx3D/Render/CDrxDXGLRenderTargetView.hpp>
#include <drx3D/Render/CDrxDXGLSamplerState.hpp>
#include <drx3D/Render/CDrxDXGLSwapChain.hpp>
#include <drx3D/Render/CDrxDXGLShader.hpp>
#include <drx3D/Render/CDrxDXGLShaderResourceView.hpp>
#include <drx3D/Render/CDrxDXGLTexture1D.hpp>
#include <drx3D/Render/CDrxDXGLTexture2D.hpp>
#include <drx3D/Render/CDrxDXGLTexture3D.hpp>
#include <drx3D/Render/CDrxDXGLUnorderedAccessView.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>
#include <drx3D/Render/Implementation/GLFormat.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>
#include <drx3D/Render/Implementation/GLShader.hpp>

CDrxDXGLDevice::CDrxDXGLDevice(CDrxDXGLGIAdapter* pAdapter, D3D_FEATURE_LEVEL eFeatureLevel)
	: m_spAdapter(pAdapter)
	, m_eFeatureLevel(eFeatureLevel)
{
	DXGL_INITIALIZE_INTERFACE(DXGIDevice)
	DXGL_INITIALIZE_INTERFACE(D3D11Device)

	CDrxDXGLDeviceContext * pImmediateContext(new CDrxDXGLDeviceContext());
	m_spImmediateContext = pImmediateContext;
	pImmediateContext->Release();
}

CDrxDXGLDevice::~CDrxDXGLDevice()
{
	m_spImmediateContext->Shutdown();
}

#if !DXGL_FULL_EMULATION

HRESULT CDrxDXGLDevice::QueryInterface(REFIID riid, uk * ppvObject)
{
	if (SingleInterface<ID3D11Device>::Query(this, riid, ppvObject) ||
	    SingleInterface<CDrxDXGLDevice>::Query(this, riid, ppvObject))
		return S_OK;
	#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT
	return E_NOINTERFACE;
	#else
	return CDrxDXGLBase::QueryInterface(riid, ppvObject);
	#endif
}

#endif //!DXGL_FULL_EMULATION

bool CDrxDXGLDevice::Initialize(const DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	NDrxOpenGL::SFeatureSpec kFeatureSpec;
	if (!NDrxOpenGL::FeatureLevelToFeatureSpec(kFeatureSpec, m_eFeatureLevel))
		return false;

	NDrxOpenGL::SAdapter* pGLAdapter(m_spAdapter->GetGLAdapter());
	kFeatureSpec.m_kFeatures = pGLAdapter->m_kFeatures & kFeatureSpec.m_kFeatures;

	NDrxOpenGL::TNativeDisplay kNativeDisplay(NULL);
	if (pDesc == NULL || ppSwapChain == NULL)
	{
#if DXGL_FULL_EMULATION
		NDrxOpenGL::SPixelFormatSpec kStandardPixelFormatSpec;
		NDrxOpenGL::GetStandardPixelFormatSpec(kStandardPixelFormatSpec);
		m_spGLDevice = new NDrxOpenGL::CDevice(pGLAdapter, kFeatureSpec, kStandardPixelFormatSpec);
#else
		return false;
#endif //DXGL_FULL_EMULATION
	}
	else
	{
		NDrxOpenGL::SFrameBufferSpec kFrameBufferSpec;
		if (!NDrxOpenGL::SwapChainDescToFrameBufferSpec(kFrameBufferSpec, *pDesc) ||
		    !NDrxOpenGL::GetNativeDisplay(kNativeDisplay, pDesc->OutputWindow))
			return false;

		m_spGLDevice = new NDrxOpenGL::CDevice(pGLAdapter, kFeatureSpec, kFrameBufferSpec);
	}

	if (!m_spGLDevice->Initialize(kNativeDisplay))
		return false;

#if DXGL_FULL_EMULATION
	if (ppSwapChain != NULL && pDesc != NULL)
#endif //!DXGL_FULL_EMULATION
	{
		CDrxDXGLSwapChain* pDXGLSwapChain(new CDrxDXGLSwapChain(this, *pDesc));
		CDrxDXGLSwapChain::ToInterface(ppSwapChain, pDXGLSwapChain);

		if (!pDXGLSwapChain->Initialize())
			return false;
	}

	return m_spImmediateContext->Initialize(this);
}

NDrxOpenGL::CDevice* CDrxDXGLDevice::GetGLDevice()
{
	return m_spGLDevice;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIObject overrides
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLDevice::GetParent(REFIID riid, uk * ppParent)
{
	IUnknown* pAdapterInterface;
	CDrxDXGLBase::ToInterface(&pAdapterInterface, m_spAdapter);
	if (pAdapterInterface->QueryInterface(riid, ppParent) == S_OK && ppParent != NULL)
		return S_OK;
#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT && !DXGL_FULL_EMULATION
	return E_FAIL;
#else
	return CDrxDXGLGIObject::GetParent(riid, ppParent);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIDevice implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLDevice::GetAdapter(IDXGIAdapter** pAdapter)
{
	if (m_spAdapter == NULL)
		return E_FAIL;
	CDrxDXGLGIAdapter::ToInterface(pAdapter, m_spAdapter);
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateSurface(const DXGI_SURFACE_DESC* pDesc, UINT NumSurfaces, DXGI_USAGE Usage, const DXGI_SHARED_RESOURCE* pSharedResource, IDXGISurface** ppSurface)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::QueryResourceResidency(IUnknown* const* ppResources, DXGI_RESIDENCY* pResidencyStatus, UINT NumResources)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::SetGPUThreadPriority(INT Priority)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::GetGPUThreadPriority(INT* pPriority)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// ID3D11Device implementation
////////////////////////////////////////////////////////////////////////////////

struct SAutoContext
{
	SAutoContext(NDrxOpenGL::CDevice* pDevice)
#if !OGL_SINGLE_CONTEXT
		: m_pDevice(pDevice)
#endif
	{
#if OGL_SINGLE_CONTEXT
		m_pContext = pDevice->GetCurrentContext();
#else
		m_pContext = m_pDevice->ReserveContext();
		if (m_pContext == NULL)
			DXGL_FATAL("This OpenGL renderer version does not support using the device from this thread");
#endif
	}

	~SAutoContext()
	{
#if !OGL_SINGLE_CONTEXT
		if (m_pContext != NULL)
			m_pDevice->ReleaseContext();
#endif
	}

	NDrxOpenGL::CContext* operator*()
	{
		return m_pContext;
	}

	operator bool() const
	{
#if OGL_SINGLE_CONTEXT
		return m_pContext != NULL;
#else
		return true;
#endif
	}

	NDrxOpenGL::CContext* m_pContext;
#if !OGL_SINGLE_CONTEXT
	NDrxOpenGL::CDevice*  m_pDevice;
#endif
};

HRESULT CDrxDXGLDevice::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer)
{
	if (ppBuffer == NULL)
	{
		// In this case the method should perform parameter validation and return the result
		DXGL_NOT_IMPLEMENTED
		return E_FAIL;
	}

	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext)
	{
		NDrxOpenGL::SBufferPtr spGLBuffer(NDrxOpenGL::CreateBuffer(*pDesc, pInitialData, *kAutoContext));
		if (spGLBuffer == NULL)
			return E_FAIL;
		CDrxDXGLBuffer::ToInterface(ppBuffer, new CDrxDXGLBuffer(*pDesc, spGLBuffer, this));
	}
#if OGL_SINGLE_CONTEXT
	else
	{
		if (pInitialData != NULL)
		{
			NDrxOpenGL::SInitialDataCopyPtr spInitialDataCopy = NDrxOpenGL::CreateBufferInitialDataCopy(*pDesc, pInitialData);
			if (spInitialDataCopy == NULL)
				return E_FAIL;
			CDrxDXGLBuffer::ToInterface(ppBuffer, new CDrxDXGLBuffer(*pDesc, spInitialDataCopy, this));
		}
		else
			CDrxDXGLBuffer::ToInterface(ppBuffer, new CDrxDXGLBuffer(*pDesc, NDrxOpenGL::SInitialDataCopyPtr(), this));
	}
#endif
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D)
{
	if (ppTexture1D == NULL)
	{
		// In this case the method should perform parameter validation and return the result
		DXGL_NOT_IMPLEMENTED
		return E_FAIL;
	}

	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext)
	{
		NDrxOpenGL::STexturePtr spGLTexture(NDrxOpenGL::CreateTexture1D(*pDesc, pInitialData, *kAutoContext));
		if (spGLTexture == NULL)
			return E_FAIL;
		CDrxDXGLTexture1D::ToInterface(ppTexture1D, new CDrxDXGLTexture1D(*pDesc, spGLTexture, this));
	}
#if OGL_SINGLE_CONTEXT
	else
	{
		if (pInitialData != NULL)
		{
			NDrxOpenGL::SInitialDataCopyPtr spInitialDataCopy = NDrxOpenGL::CreateTexture1DInitialDataCopy(*pDesc, pInitialData);
			if (spInitialDataCopy == NULL)
				return E_FAIL;
			CDrxDXGLTexture1D::ToInterface(ppTexture1D, new CDrxDXGLTexture1D(*pDesc, spInitialDataCopy, this));
		}
		else
			CDrxDXGLTexture1D::ToInterface(ppTexture1D, new CDrxDXGLTexture1D(*pDesc, NDrxOpenGL::SInitialDataCopyPtr(), this));
	}
#endif
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
	if (ppTexture2D == NULL)
	{
		// In this case the method should perform parameter validation and return the result
		DXGL_NOT_IMPLEMENTED
		return E_FAIL;
	}

	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext)
	{
		NDrxOpenGL::STexturePtr spGLTexture(NDrxOpenGL::CreateTexture2D(*pDesc, pInitialData, *kAutoContext));
		if (spGLTexture == NULL)
			return E_FAIL;
		CDrxDXGLTexture2D::ToInterface(ppTexture2D, new CDrxDXGLTexture2D(*pDesc, spGLTexture, this));
	}
#if OGL_SINGLE_CONTEXT
	else
	{
		if (pInitialData != NULL)
		{
			NDrxOpenGL::SInitialDataCopyPtr spInitialDataCopy = NDrxOpenGL::CreateTexture2DInitialDataCopy(*pDesc, pInitialData);
			if (spInitialDataCopy == NULL)
				return E_FAIL;
			CDrxDXGLTexture2D::ToInterface(ppTexture2D, new CDrxDXGLTexture2D(*pDesc, spInitialDataCopy, this));
		}
		else
			CDrxDXGLTexture2D::ToInterface(ppTexture2D, new CDrxDXGLTexture2D(*pDesc, NDrxOpenGL::SInitialDataCopyPtr(), this));
		return S_OK;
	}
#endif
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D)
{
	if (ppTexture3D == NULL)
	{
		// In this case the method should perform parameter validation and return the result
		DXGL_NOT_IMPLEMENTED
		return E_FAIL;
	}

	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext)
	{
		NDrxOpenGL::STexturePtr spGLTexture(NDrxOpenGL::CreateTexture3D(*pDesc, pInitialData, *kAutoContext));
		if (spGLTexture == NULL)
			return E_FAIL;
		CDrxDXGLTexture3D::ToInterface(ppTexture3D, new CDrxDXGLTexture3D(*pDesc, spGLTexture, this));
	}
#if OGL_SINGLE_CONTEXT
	else
	{
		if (pInitialData != NULL)
		{
			NDrxOpenGL::SInitialDataCopyPtr spInitialDataCopy = NDrxOpenGL::CreateTexture3DInitialDataCopy(*pDesc, pInitialData);
			if (spInitialDataCopy == NULL)
				return E_FAIL;
			CDrxDXGLTexture3D::ToInterface(ppTexture3D, new CDrxDXGLTexture3D(*pDesc, spInitialDataCopy, this));
		}
		else
			CDrxDXGLTexture3D::ToInterface(ppTexture3D, new CDrxDXGLTexture3D(*pDesc, NDrxOpenGL::SInitialDataCopyPtr(), this));
		return S_OK;
	}
#endif
	return S_OK;
}

bool GetStandardViewDesc(CDrxDXGLTexture1D* pTexture, D3D11_SHADER_RESOURCE_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE1D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	if (kTextureDesc.ArraySize > 0)
	{
		kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		kStandardDesc.Texture1DArray.MostDetailedMip = 0;
		kStandardDesc.Texture1DArray.MipLevels = -1;
		kStandardDesc.Texture1DArray.FirstArraySlice = 0;
		kStandardDesc.Texture1DArray.ArraySize = kTextureDesc.ArraySize;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		kStandardDesc.Texture1D.MostDetailedMip = 0;
		kStandardDesc.Texture1DArray.MipLevels = -1;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture2D* pTexture, D3D11_SHADER_RESOURCE_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE2D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	if (kTextureDesc.ArraySize > 1)
	{
		if (kTextureDesc.SampleDesc.Count > 1)
		{
			kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
			kStandardDesc.Texture2DMSArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DMSArray.ArraySize = kTextureDesc.ArraySize;
		}
		else
		{
			kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			kStandardDesc.Texture2DArray.MostDetailedMip = 0;
			kStandardDesc.Texture2DArray.MipLevels = -1;
			kStandardDesc.Texture2DArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DArray.ArraySize = kTextureDesc.ArraySize;
		}
	}
	else if (kTextureDesc.SampleDesc.Count > 1)
	{
		kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		kStandardDesc.Texture2D.MostDetailedMip = 0;
		kStandardDesc.Texture2D.MipLevels = -1;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture3D* pTexture, D3D11_SHADER_RESOURCE_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE3D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	kStandardDesc.Texture3D.MostDetailedMip = 0;
	kStandardDesc.Texture3D.MipLevels = -1;
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLBuffer* pBuffer, D3D11_SHADER_RESOURCE_VIEW_DESC& kStandardDesc)
{
	D3D11_BUFFER_DESC kBufferDesc;
	pBuffer->GetDesc(&kBufferDesc);

	bool bSuccess((kBufferDesc.MiscFlags | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) != 0);
	if (bSuccess)
	{
		kStandardDesc.Format = DXGI_FORMAT_UNKNOWN;
		kStandardDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		kStandardDesc.Buffer.FirstElement = 0;
		kStandardDesc.Buffer.NumElements = kBufferDesc.StructureByteStride;
	}
	else
	{
		DXGL_ERROR("Default shader resource view for a buffer requires element size specification");
	}

	pBuffer->Release();
	return bSuccess;
}

bool GetStandardViewDesc(CDrxDXGLTexture1D* pTexture, D3D11_RENDER_TARGET_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE1D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	if (kTextureDesc.ArraySize > 0)
	{
		kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		kStandardDesc.Texture1DArray.MipSlice = 0;
		kStandardDesc.Texture1DArray.FirstArraySlice = 0;
		kStandardDesc.Texture1DArray.ArraySize = kTextureDesc.ArraySize;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
		kStandardDesc.Texture1D.MipSlice = 0;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture2D* pTexture, D3D11_RENDER_TARGET_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE2D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	if (kTextureDesc.ArraySize > 1)
	{
		if (kTextureDesc.SampleDesc.Count > 1)
		{
			kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
			kStandardDesc.Texture2DMSArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DMSArray.ArraySize = kTextureDesc.ArraySize;
		}
		else
		{
			kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			kStandardDesc.Texture2DArray.MipSlice = 0;
			kStandardDesc.Texture2DArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DArray.ArraySize = kTextureDesc.ArraySize;
		}
	}
	else if (kTextureDesc.SampleDesc.Count > 1)
	{
		kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		kStandardDesc.Texture2D.MipSlice = 0;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture3D* pTexture, D3D11_RENDER_TARGET_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE3D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
	kStandardDesc.Texture3D.MipSlice = 0;
	kStandardDesc.Texture3D.FirstWSlice = 0;
	kStandardDesc.Texture3D.WSize = -1;
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLBuffer* pBuffer, D3D11_RENDER_TARGET_VIEW_DESC& kStandardDesc)
{
	D3D11_BUFFER_DESC kBufferDesc;
	pBuffer->GetDesc(&kBufferDesc);

	bool bSuccess((kBufferDesc.MiscFlags | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) != 0);
	if (bSuccess)
	{
		kStandardDesc.Format = DXGI_FORMAT_UNKNOWN;
		kStandardDesc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
		kStandardDesc.Buffer.FirstElement = 0;
		kStandardDesc.Buffer.NumElements = kBufferDesc.StructureByteStride;
	}
	else
	{
		DXGL_ERROR("Default render target view for a buffer requires element size specification");
	}

	pBuffer->Release();
	return bSuccess;
}

bool GetStandardViewDesc(CDrxDXGLTexture1D* pTexture, D3D11_DEPTH_STENCIL_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE1D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	kStandardDesc.Flags = 0;
	if (kTextureDesc.ArraySize > 0)
	{
		kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		kStandardDesc.Texture1DArray.MipSlice = 0;
		kStandardDesc.Texture1DArray.FirstArraySlice = 0;
		kStandardDesc.Texture1DArray.ArraySize = kTextureDesc.ArraySize;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
		kStandardDesc.Texture1D.MipSlice = 0;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture2D* pTexture, D3D11_DEPTH_STENCIL_VIEW_DESC& kStandardDesc)
{
	D3D11_TEXTURE2D_DESC kTextureDesc;
	pTexture->GetDesc(&kTextureDesc);

	kStandardDesc.Format = kTextureDesc.Format;
	kStandardDesc.Flags = 0;
	if (kTextureDesc.ArraySize > 0)
	{
		if (kTextureDesc.SampleDesc.Count > 1)
		{
			kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
			kStandardDesc.Texture2DMSArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DMSArray.ArraySize = kTextureDesc.ArraySize;
		}
		else
		{
			kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			kStandardDesc.Texture2DArray.MipSlice = 0;
			kStandardDesc.Texture2DArray.FirstArraySlice = 0;
			kStandardDesc.Texture2DArray.ArraySize = kTextureDesc.ArraySize;
		}
	}
	else if (kTextureDesc.SampleDesc.Count > 1)
	{
		kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		kStandardDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		kStandardDesc.Texture2D.MipSlice = 0;
	}
	pTexture->Release();
	return true;
}

bool GetStandardViewDesc(CDrxDXGLTexture3D* pTexture, D3D11_DEPTH_STENCIL_VIEW_DESC&)
{
	DXGL_ERROR("Cannot bind a depth stencil view to a 3D texture");
	pTexture->Release();
	return false;
}

bool GetStandardViewDesc(CDrxDXGLBuffer* pBuffer, D3D11_DEPTH_STENCIL_VIEW_DESC&)
{
	DXGL_ERROR("Cannot bind a depth stencil view to a buffer");
	pBuffer->Release();
	return false;
}

template<typename ViewDesc>
bool GetStandardViewDesc(ID3D11Resource* pResource, ViewDesc& kStandardDesc)
{
	memset(&kStandardDesc, 0, sizeof(kStandardDesc));

	uk pvData(NULL);
	if (!FAILED(pResource->QueryInterface(__uuidof(ID3D11Texture1D), &pvData)) && pvData != NULL)
		return GetStandardViewDesc(CDrxDXGLTexture1D::FromInterface(reinterpret_cast<ID3D11Texture1D*>(pvData)), kStandardDesc);
	if (!FAILED(pResource->QueryInterface(__uuidof(ID3D11Texture2D), &pvData)) && pvData != NULL)
		return GetStandardViewDesc(CDrxDXGLTexture2D::FromInterface(reinterpret_cast<ID3D11Texture2D*>(pvData)), kStandardDesc);
	if (!FAILED(pResource->QueryInterface(__uuidof(ID3D11Texture3D), &pvData)) && pvData != NULL)
		return GetStandardViewDesc(CDrxDXGLTexture3D::FromInterface(reinterpret_cast<ID3D11Texture3D*>(pvData)), kStandardDesc);
	if (!FAILED(pResource->QueryInterface(__uuidof(ID3D11Buffer), &pvData)) && pvData != NULL)
		return GetStandardViewDesc(CDrxDXGLBuffer::FromInterface(reinterpret_cast<ID3D11Buffer*>(pvData)), kStandardDesc);

	DXGL_ERROR("Unknown resource type for standard view description");
	return false;
}

HRESULT CDrxDXGLDevice::CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC kStandardDesc;
	if (pDesc == NULL)
	{
		if (!GetStandardViewDesc(pResource, kStandardDesc))
			return E_INVALIDARG;
		pDesc = &kStandardDesc;
	}
	assert(pDesc != NULL);

	CDrxDXGLShaderResourceView* pSRView(new CDrxDXGLShaderResourceView(CDrxDXGLResource::FromInterface(pResource), *pDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!kAutoContext || pSRView->Initialize(*kAutoContext))
	{
		CDrxDXGLShaderResourceView::ToInterface(ppSRView, pSRView);
		return S_OK;
	}

	pSRView->Release();
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC kStandardDesc;
	if (pDesc == NULL)
	{
		if (!GetStandardViewDesc(pResource, kStandardDesc))
			return E_INVALIDARG;
		pDesc = &kStandardDesc;
	}
	assert(pDesc != NULL);

	CDrxDXGLUnorderedAccessView* pUAView(new CDrxDXGLUnorderedAccessView(CDrxDXGLResource::FromInterface(pResource), *pDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!kAutoContext || pUAView->Initialize(*kAutoContext))
	{
		CDrxDXGLUnorderedAccessView::ToInterface(ppUAView, pUAView);
		return S_OK;
	}

	pUAView->Release();
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView)
{
	D3D11_RENDER_TARGET_VIEW_DESC kStandardDesc;
	if (pDesc == NULL)
	{
		if (!GetStandardViewDesc(pResource, kStandardDesc))
			return E_INVALIDARG;
		pDesc = &kStandardDesc;
	}
	assert(pDesc != NULL);

	CDrxDXGLRenderTargetView* pRTView(new CDrxDXGLRenderTargetView(CDrxDXGLResource::FromInterface(pResource), *pDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!kAutoContext || pRTView->Initialize(*kAutoContext))
	{
		CDrxDXGLRenderTargetView::ToInterface(ppRTView, pRTView);
		return S_OK;
	}

	pRTView->Release();
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC kStandardDesc;
	if (pDesc == NULL)
	{
		if (!GetStandardViewDesc(pResource, kStandardDesc))
			return E_INVALIDARG;
		pDesc = &kStandardDesc;
	}
	assert(pDesc != NULL);

	CDrxDXGLDepthStencilView* pDSView(new CDrxDXGLDepthStencilView(CDrxDXGLResource::FromInterface(pResource), *pDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!kAutoContext || pDSView->Initialize(*kAutoContext))
	{
		CDrxDXGLDepthStencilView::ToInterface(ppDepthStencilView, pDSView);
		return S_OK;
	}

	pDSView->Release();
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, ukk pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout)
{
	NDrxOpenGL::TShaderReflection kShaderReflection;
	if (!InitializeShaderReflectionFromInput(&kShaderReflection, pShaderBytecodeWithInputSignature))
		return E_FAIL;

	_smart_ptr<NDrxOpenGL::SInputLayout> spGLInputLayout(NDrxOpenGL::CreateInputLayout(pInputElementDescs, NumElements, kShaderReflection));

	if (spGLInputLayout == NULL)
		return E_FAIL;

	CDrxDXGLInputLayout::ToInterface(ppInputLayout, new CDrxDXGLInputLayout(spGLInputLayout, this));

	return S_OK;
}

_smart_ptr<NDrxOpenGL::SShader> CreateGLShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, NDrxOpenGL::EShaderType eType, NDrxOpenGL::CContext* pContext)
{
	if (pClassLinkage != NULL)
	{
		DXGL_ERROR("Class linkage not supported");
		return NULL;
	}

	_smart_ptr<NDrxOpenGL::SShader> spGLShader(new NDrxOpenGL::SShader());
	spGLShader->m_eType = eType;
	if (!NDrxOpenGL::InitializeShader(spGLShader, pShaderBytecode, BytecodeLength))
		return NULL;
	return spGLShader;
}

template<typename DXGLShader, typename D3DShader>
HRESULT CreateShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, D3DShader** ppShader, NDrxOpenGL::EShaderType eType, CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext)
{
	_smart_ptr<NDrxOpenGL::SShader> spGLShader(CreateGLShader(pShaderBytecode, BytecodeLength, pClassLinkage, eType, pContext));
	if (spGLShader == NULL)
		return E_FAIL;

	DXGLShader::ToInterface(ppShader, new DXGLShader(spGLShader, pDevice));
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateVertexShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLVertexShader, ID3D11VertexShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader, NDrxOpenGL::eST_Vertex, this, *kAutoContext);
}

HRESULT CDrxDXGLDevice::CreateGeometryShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
#if DXGL_SUPPORT_GEOMETRY_SHADERS
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLGeometryShader, ID3D11GeometryShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppGeometryShader, NDrxOpenGL::eST_Geometry, this, *kAutoContext);
#else
	DXGL_ERROR("Geometry shaders are not supported by this GL implementation.");
	return E_FAIL;
#endif
}

HRESULT CDrxDXGLDevice::CreateGeometryShaderWithStreamOutput(ukk pShaderBytecode, SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY* pSODeclaration, UINT NumEntries, const UINT* pBufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreatePixelShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader)
{
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLPixelShader, ID3D11PixelShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader, NDrxOpenGL::eST_Fragment, this, *kAutoContext);
}

HRESULT CDrxDXGLDevice::CreateHullShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader)
{
#if DXGL_SUPPORT_TESSELLATION
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLHullShader, ID3D11HullShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppHullShader, NDrxOpenGL::eST_TessControl, this, *kAutoContext);
#else
	DXGL_ERROR("Hull shaders are not supported by this GL implementation.");
	return E_FAIL;
#endif
}

HRESULT CDrxDXGLDevice::CreateDomainShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader)
{
#if DXGL_SUPPORT_TESSELLATION
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLDomainShader, ID3D11DomainShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppDomainShader, NDrxOpenGL::eST_TessEvaluation, this, *kAutoContext);
#else
	DXGL_ERROR("Domain shaders are not supported by this GL implementation.");
	return E_FAIL;
#endif
}

HRESULT CDrxDXGLDevice::CreateComputeShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader)
{
#if DXGL_SUPPORT_COMPUTE
	SAutoContext kAutoContext(m_spGLDevice);
	return CreateShader<CDrxDXGLComputeShader, ID3D11ComputeShader>(pShaderBytecode, BytecodeLength, pClassLinkage, ppComputeShader, NDrxOpenGL::eST_Compute, this, *kAutoContext);
#else
	DXGL_ERROR("Compute shaders are not supported by this GL implementation.");
	return E_FAIL;
#endif
}

HRESULT CDrxDXGLDevice::CreateClassLinkage(ID3D11ClassLinkage** ppLinkage)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState)
{
	CDrxDXGLBlendState* pState(new CDrxDXGLBlendState(*pBlendStateDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!pState->Initialize(this, *kAutoContext))
	{
		pState->Release();
		return E_FAIL;
	}

	CDrxDXGLBlendState::ToInterface(ppBlendState, pState);
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState)
{
	CDrxDXGLDepthStencilState* pState(new CDrxDXGLDepthStencilState(*pDepthStencilDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!pState->Initialize(this, *kAutoContext))
	{
		pState->Release();
		return E_FAIL;
	}

	CDrxDXGLDepthStencilState::ToInterface(ppDepthStencilState, pState);
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState)
{
	CDrxDXGLRasterizerState* pState(new CDrxDXGLRasterizerState(*pRasterizerDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (!pState->Initialize(this, *kAutoContext))
	{
		pState->Release();
		return E_FAIL;
	}

	CDrxDXGLRasterizerState::ToInterface(ppRasterizerState, pState);
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState)
{
	CDrxDXGLSamplerState* pState(new CDrxDXGLSamplerState(*pSamplerDesc, this));

	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext && !pState->Initialize(*kAutoContext))
	{
		pState->Release();
		return E_FAIL;
	}

	CDrxDXGLSamplerState::ToInterface(ppSamplerState, pState);
	return S_OK;
}

HRESULT CDrxDXGLDevice::CreateQuery(const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
{
	SAutoContext kAutoContext(m_spGLDevice);
	if (kAutoContext)
	{
		NDrxOpenGL::SQueryPtr spGLQuery(NDrxOpenGL::CreateQuery(*pQueryDesc, *kAutoContext));
		if (spGLQuery == NULL)
			return E_FAIL;
		CDrxDXGLQuery::ToInterface(ppQuery, new CDrxDXGLQuery(*pQueryDesc, spGLQuery, this));
		return S_OK;
	}
	else
	{
		CDrxDXGLQuery::ToInterface(ppQuery, new CDrxDXGLQuery(*pQueryDesc, NULL, this));
		return S_OK;
	}
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreatePredicate(const D3D11_QUERY_DESC* pPredicateDesc, ID3D11Predicate** ppPredicate)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateCounter(const D3D11_COUNTER_DESC* pCounterDesc, ID3D11Counter** ppCounter)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CreateDeferredContext(UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::OpenSharedResource(HANDLE hResource, REFIID ReturnedInterface, uk * ppResource)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CheckFormatSupport(DXGI_FORMAT Format, UINT* pFormatSupport)
{
	NDrxOpenGL::EGIFormat eGIFormat(NDrxOpenGL::GetGIFormat(Format));
	if (eGIFormat == NDrxOpenGL::eGIF_NUM)
	{
		DXGL_ERROR("Unknown DXGI format");
		return E_FAIL;
	}

	(*pFormatSupport) = m_spAdapter->GetGLAdapter()->m_kCapabilities.m_auFormatSupport[eGIFormat];
	return S_OK;
}

HRESULT CDrxDXGLDevice::CheckMultisampleQualityLevels(DXGI_FORMAT Format, UINT SampleCount, UINT* pNumQualityLevels)
{
	NDrxOpenGL::EGIFormat eGIFormat(NDrxOpenGL::GetGIFormat(Format));
	if (eGIFormat != NDrxOpenGL::eGIF_NUM && NDrxOpenGL::CheckFormatMultisampleSupport(m_spAdapter->GetGLAdapter(), eGIFormat, SampleCount))
		*pNumQualityLevels = 1;
	else
		*pNumQualityLevels = 0;
	DXGL_TODO("Check if there's a way to query for specific quality levels");
	return S_OK;
}

void CDrxDXGLDevice::CheckCounterInfo(D3D11_COUNTER_INFO* pCounterInfo)
{
	DXGL_NOT_IMPLEMENTED
}

HRESULT CDrxDXGLDevice::CheckCounter(const D3D11_COUNTER_DESC* pDesc, D3D11_COUNTER_TYPE* pType, UINT* pActiveCounters, LPSTR szName, UINT* pNameLength, LPSTR szUnits, UINT* pUnitsLength, LPSTR szDescription, UINT* pDescriptionLength)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLDevice::CheckFeatureSupport(D3D11_FEATURE Feature, uk pFeatureSupportData, UINT FeatureSupportDataSize)
{
	switch (Feature)
	{
	case D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS:
		{
			D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS* pData(static_cast<D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS*>(pFeatureSupportData));
			bool bComputeShaderSupported(m_spGLDevice->GetFeatureSpec().m_kFeatures.Get(NDrxOpenGL::eF_ComputeShader));
			pData->ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x = bComputeShaderSupported ? TRUE : FALSE;
			return S_OK;
		}
		break;
	default:
		DXGL_TODO("Add supported 11.1 features")
		return E_FAIL;
	}
}

#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT && !DXGL_FULL_EMULATION

HRESULT CDrxDXGLDevice::GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData)
{
	return m_kPrivateDataContainer.GetPrivateData(guid, pDataSize, pData);
}

HRESULT CDrxDXGLDevice::SetPrivateData(REFGUID guid, UINT DataSize, ukk pData)
{
	return m_kPrivateDataContainer.SetPrivateData(guid, DataSize, pData);
}

HRESULT CDrxDXGLDevice::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
	return m_kPrivateDataContainer.SetPrivateDataInterface(guid, pData);
}

#endif

D3D_FEATURE_LEVEL CDrxDXGLDevice::GetFeatureLevel(void)
{
	DXGL_NOT_IMPLEMENTED
	return D3D_FEATURE_LEVEL_11_0;
}

UINT CDrxDXGLDevice::GetCreationFlags(void)
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

HRESULT CDrxDXGLDevice::GetDeviceRemovedReason(void)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

void CDrxDXGLDevice::GetImmediateContext(ID3D11DeviceContext** ppImmediateContext)
{
	m_spImmediateContext->AddRef();
	CDrxDXGLDeviceContext::ToInterface(ppImmediateContext, m_spImmediateContext);
}

HRESULT CDrxDXGLDevice::SetExceptionMode(UINT RaiseFlags)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

UINT CDrxDXGLDevice::GetExceptionMode(void)
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}
