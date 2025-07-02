// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxVKGIFactory.hpp>
#include <drx3D/Render/CDrxVKGIAdapter.hpp>
#include <drx3D/Render/CDrxVKSwapChain.hpp>
#include <drx3D/Render/D3D/Vulkan/VKInstance.hpp>


CDrxVKGIFactory* CDrxVKGIFactory::Create()
{
	return new CDrxVKGIFactory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxVKGIFactory::CDrxVKGIFactory()
{
	VK_FUNC_LOG();
	auto pInstance = stl::make_unique<NDrxVulkan::CInstance>();
	if (pInstance->Initialize("appName", 1, "drx3D", 1))
	{
		m_pInstance = std::move(pInstance);
	}
}

CDrxVKGIFactory::~CDrxVKGIFactory()
{
	VK_FUNC_LOG();
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::EnumAdapters(UINT Adapter, _Out_ IDXGIAdapter** ppAdapter)
{
	VK_FUNC_LOG();
	return EnumAdapters1(Adapter, ppAdapter);
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::MakeWindowAssociation(HWND WindowHandle, UINT Flags)
{
	VK_FUNC_LOG();
	return S_OK;  //m_pDXGIFactory4->MakeWindowAssociation(WindowHandle, Flags);
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::GetWindowAssociation(_Out_ HWND* pWindowHandle)
{
	VK_FUNC_LOG();
	return S_OK;  //m_pDXGIFactory4->GetWindowAssociation(pWindowHandle);
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::CreateSwapChain(_In_ IUnknown* pDevice, _In_ DXGI_SWAP_CHAIN_DESC* pDesc, _Out_ IDXGISwapChain** ppSwapChain)
{
	VK_FUNC_LOG();

	*ppSwapChain = CDrxVKSwapChain::Create(static_cast<D3DDevice*>(pDevice), pDesc);
	return *ppSwapChain ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::CreateSoftwareAdapter(HMODULE Module, _Out_ IDXGIAdapter** ppAdapter)
{
	VK_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDrxVKGIFactory::EnumAdapters1(UINT Adapter, _Out_ IDXGIAdapter1** ppAdapter)
{
	VK_FUNC_LOG();

	if (Adapter < m_pInstance->GetPhysicalDeviceCount())
	{
		*ppAdapter = CDrxVKGIAdapter::Create(this, Adapter);
		return S_OK;
	}
	else
	{
		return DXGI_ERROR_NOT_FOUND;
	}
}

BOOL STDMETHODCALLTYPE CDrxVKGIFactory::IsCurrent()
{
	VK_FUNC_LOG();
	return 1;  //m_pDXGIFactory4->IsCurrent();
}
