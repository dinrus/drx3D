// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// Pull in everything needed to implement device objects
#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>
#include <drx3D/Render/D3D/Vulkan/VKDevice.hpp>
#include <drx3D/Render/D3D/Vulkan/VKShader.hpp>
#include <drx3D/Render/D3D/Vulkan/D3DVKConversionUtility.hpp>

// DXGI emulation
#include <drx3D/Render/D3D/Vulkan/CDrxVKShaderReflection.hpp>
#include <drx3D/Render/D3D/Vulkan/CDrxVKGIAdapter.hpp>
#include <drx3D/Render/D3D/Vulkan/CDrxVKGIFactory.hpp>
#include <drx3D/Render/D3D/Vulkan/CDrxVKGIOutput.hpp>
#include <drx3D/Render/D3D/Vulkan/CDrxVKSwapChain.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI D3DCreateBlob(SIZE_T NumBytes, ID3DBlob** ppBuffer);

HRESULT WINAPI CreateDXGIFactory1(REFIID riid, uk * ppFactory);

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
	ID3D11DeviceContext** ppImmediateContext);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(USE_SDL2_VIDEO)
bool VKCreateSDLWindow(tukk szTitle, u32 uWidth, u32 uHeight, bool bFullScreen, HWND* pHandle);
void VKDestroySDLWindow(HWND kHandle);
#endif
