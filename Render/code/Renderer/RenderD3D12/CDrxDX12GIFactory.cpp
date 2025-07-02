// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12GIFactory.hpp>
#include <drx3D/Render/CDrxDX12GIAdapter.hpp>
#include <drx3D/Render/CDrxDX12SwapChain.hpp>

#include <drx3D/Render/DX12/Device/CDrxDX12Device.hpp>
#include <drx3D/Render/DX12/Device/CDrxDX12DeviceContext.hpp>

#include <drx3D/Render/DX12/API/DX12SwapChain.hpp>

CDrxDX12GIFactory* CDrxDX12GIFactory::Create()
{
	IDXGIFactory4ToCall* pDXGIFactory4 = nullptr;

#if DRX_PLATFORM_DESKTOP
	if (S_OK != CreateDXGIFactory1(IID_GFX_ARGS(&pDXGIFactory4)))
#endif
	{
		DX12_ASSERT("Failed to create underlying DXGI factory!");
		return nullptr;
	}

	return DX12_NEW_RAW(CDrxDX12GIFactory(pDXGIFactory4));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12GIFactory::CDrxDX12GIFactory(IDXGIFactory4ToCall* pDXGIFactory4)
	: Super()
	, m_pDXGIFactory4(pDXGIFactory4)
{
	DX12_FUNC_LOG

}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::EnumAdapters(UINT Adapter, _Out_ IDXGIAdapter** ppAdapter)
{
	DX12_FUNC_LOG
	* ppAdapter = CDrxDX12GIAdapter::Create(this, Adapter);
	return *ppAdapter ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::MakeWindowAssociation(HWND WindowHandle, UINT Flags)
{
	DX12_FUNC_LOG
	return m_pDXGIFactory4->MakeWindowAssociation(WindowHandle, Flags);
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::GetWindowAssociation(_Out_ HWND* pWindowHandle)
{
	DX12_FUNC_LOG
	return m_pDXGIFactory4->GetWindowAssociation(pWindowHandle);
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::CreateSwapChain(_In_ IGfxUnknown* pDevice, _In_ DXGI_SWAP_CHAIN_DESC* pDesc, _Out_ IDXGISwapChain** ppSwapChain)
{
	DX12_FUNC_LOG
	* ppSwapChain = CDrxDX12SwapChain::Create(static_cast<CDrxDX12Device*>(pDevice), m_pDXGIFactory4, pDesc);
	return *ppSwapChain ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::CreateSoftwareAdapter(HMODULE Module, _Out_ IDXGIAdapter** ppAdapter)
{
	DX12_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIFactory::EnumAdapters1(UINT Adapter, _Out_ IDXGIAdapter1** ppAdapter)
{
	DX12_FUNC_LOG
	* ppAdapter = CDrxDX12GIAdapter::Create(this, Adapter);
	return *ppAdapter ? S_OK : E_FAIL;
}

BOOL STDMETHODCALLTYPE CDrxDX12GIFactory::IsCurrent()
{
	DX12_FUNC_LOG
	return m_pDXGIFactory4->IsCurrent();
}
