// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/D3D/Vulkan/DrxVulkan.hpp>
#include <drx3D/Render/D3D/Vulkan/VKInstance.hpp>

#if VK_USE_DXGI
	#include <drx3D/Render/D3D/Vulkan/DXGI/CDrxVKGIFactory_DXGI.hpp>
#endif


HRESULT WINAPI D3DCreateBlob(SIZE_T NumBytes, ID3DBlob** ppBuffer)
{
	*ppBuffer = new CDrxVKBlob(NumBytes);
	return *ppBuffer ? (*ppBuffer)->GetBufferPointer() ? S_OK : (delete *ppBuffer, E_OUTOFMEMORY) : E_FAIL;
}

HRESULT WINAPI CreateDXGIFactory1(REFIID riid, uk * ppFactory)
{
#if VK_USE_DXGI
	*ppFactory = CDrxVKGIFactory_DXGI::Create();
#else
	*ppFactory = CDrxVKGIFactory::Create();
#endif

	return *ppFactory ? 0 : -1;
}

HRESULT WINAPI VKCreateDevice(
	IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	ID3D11Device** ppDevice,
	D3D_FEATURE_LEVEL* pFeatureLevel,
	ID3D11DeviceContext** ppImmediateContext)
{
	_smart_ptr<NDrxVulkan::CDevice> pDevice = pAdapter->GetFactory()->GetVkInstance()->CreateDevice(pAdapter->GetDeviceIndex());
	if (!pDevice)
	{
		return S_FALSE;
	}

	if (ppImmediateContext)
	{
		pDevice->AddRef();
		*ppImmediateContext = pDevice.get();
	}

	*ppDevice = pDevice.ReleaseOwnership();
	*pFeatureLevel = D3D_FEATURE_LEVEL_12_1;
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(USE_SDL2_VIDEO)
bool VKCreateSDLWindow(tukk szTitle, u32 uWidth, u32 uHeight, bool bFullScreen, HWND* pHandle)
{
	return NDrxVulkan::CInstance::CreateSDLWindow(szTitle, uWidth, uHeight, bFullScreen, pHandle);
}

void VKDestroySDLWindow(HWND kHandle)
{
	NDrxVulkan::CInstance::DestroySDLWindow(kHandle);
}
#endif

namespace NDrxVulkan
{
	VkFormat ConvertFormat(DXGI_FORMAT format)
	{
		if (format > DRX_ARRAY_COUNT(s_FormatWithSize))
		{
			format = DXGI_FORMAT_UNKNOWN;
		}
		return s_FormatWithSize[format].Format;
	}

	DXGI_FORMAT ConvertFormat(VkFormat format)
	{
		if (format > DRX_ARRAY_COUNT(s_VkFormatToDXGI))
		{
			format = VK_FORMAT_UNDEFINED;
		}
		return s_VkFormatToDXGI[format];
	}
}
