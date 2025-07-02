// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIFactory.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for IDXGIFactory
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLGIAdapter.hpp>
#include <drx3D/Render/CDrxDXGLGIFactory.hpp>
#include <drx3D/Render/CDrxDXGLSwapChain.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLGIFactory::CDrxDXGLGIFactory()
{
	DXGL_INITIALIZE_INTERFACE(DXGIFactory)
	DXGL_INITIALIZE_INTERFACE(DXGIFactory1)
}

CDrxDXGLGIFactory::~CDrxDXGLGIFactory()
{
}

bool CDrxDXGLGIFactory::Initialize()
{
	std::vector<NDrxOpenGL::SAdapterPtr> kAdapters;
	if (!NDrxOpenGL::DetectAdapters(kAdapters))
		return false;

	u32 uAdapter;
	for (uAdapter = 0; uAdapter < kAdapters.size(); ++uAdapter)
	{
		_smart_ptr<CDrxDXGLGIAdapter> spAdapter(new CDrxDXGLGIAdapter(this, kAdapters.at(uAdapter).get()));
		if (!spAdapter->Initialize())
			return false;
		m_kAdapters.push_back(spAdapter);
	}

	return true;
}

template<typename AdapterInterface>
HRESULT EnumAdaptersInternal(UINT Adapter, AdapterInterface** ppAdapter, const std::vector<_smart_ptr<CDrxDXGLGIAdapter>>& kAdapters)
{
	if (Adapter < kAdapters.size())
	{
		CDrxDXGLGIAdapter::ToInterface(ppAdapter, kAdapters.at(Adapter));
		return S_OK;
	}
	*ppAdapter = NULL;
	return DXGI_ERROR_NOT_FOUND;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIFactory implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIFactory::EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter)
{
	return EnumAdaptersInternal(Adapter, ppAdapter, m_kAdapters);
}

HRESULT CDrxDXGLGIFactory::MakeWindowAssociation(HWND WindowHandle, UINT Flags)
{
	DXGL_TODO("Implement ALT+ENTER handling in OpenGL if required")
	Flags;

	m_hWindowHandle = WindowHandle;
	return S_OK;
}

HRESULT CDrxDXGLGIFactory::GetWindowAssociation(HWND* pWindowHandle)
{
	*pWindowHandle = m_hWindowHandle;
	return S_OK;
}

HRESULT CDrxDXGLGIFactory::CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
#if DXGL_FULL_EMULATION
	uk pvD3D11Device;
	if (FAILED(pDevice->QueryInterface(__uuidof(ID3D11Device), &pvD3D11Device)) || pvD3D11Device == NULL)
	{
		DXGL_ERROR("CDrxDXGLGIFactory::CreateSwapChain - device type is not compatible with swap chain creation");
		return E_FAIL;
	}
	CDrxDXGLDevice* pDXGLDevice(CDrxDXGLDevice::FromInterface(static_cast<ID3D11Device*>(pvD3D11Device)));

	_smart_ptr<CDrxDXGLSwapChain> spSwapChain(new CDrxDXGLSwapChain(pDXGLDevice, *pDesc));
	if (!spSwapChain->Initialize())
		return false;
	CDrxDXGLSwapChain::ToInterface(ppSwapChain, spSwapChain);
	spSwapChain->AddRef();

	return S_OK;
#else
	DXGL_ERROR("CDrxDXGLGIFactory::CreateSwapChain is not supported, use D3D11CreateDeviceAndSwapChain");
	return E_FAIL;
#endif
}

HRESULT CDrxDXGLGIFactory::CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter** ppAdapter)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIFactory1 implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIFactory::EnumAdapters1(UINT Adapter, IDXGIAdapter1** ppAdapter)
{
	return EnumAdaptersInternal(Adapter, ppAdapter, m_kAdapters);
}

BOOL CDrxDXGLGIFactory::IsCurrent()
{
	DXGL_NOT_IMPLEMENTED
	return false;
}
