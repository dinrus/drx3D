// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern i32 g_nPrintDX12;

#ifdef _DEBUG
	#define DX12_LOG(cond, ...) \
		do { if (cond || g_nPrintDX12) { DrxLog("DX12 Log: " __VA_ARGS__); } } while (0)
	#define DX12_ERROR(...) \
		do { DrxLog("DX12 Error: " __VA_ARGS__); } while (0)
	#define DX12_ASSERT(cond, ...) \
		do { if (!(cond)) { DX12_ERROR(__VA_ARGS__); DRX_ASSERT_MESSAGE(0, __VA_ARGS__); } } while (0)
	#define DX12_WARNING(cond, ...) \
		do { if (!(cond)) { DX12_LOG(__VA_ARGS__); } } while (0)
#else
	#define DX12_LOG(cond, ...) do {} while (0)
	#define DX12_ERROR(...)     do {} while (0)
	#define DX12_ASSERT(cond, ...)
	#define DX12_WARNING(cond, ...)
#endif

#define DX12_NOT_IMPLEMENTED DX12_ASSERT(0, "Not implemented!");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/D3D/DX12/API/DX12Base.hpp>
#include <drx3D/Render/D3D/DX12/API/DX12PSO.hpp>

// DX11 emulation
#include <drx3D/Render/D3D/DX12/Misc/SDrxDX11PipelineState.hpp>
#include <drx3D/Render/D3D/DX12/Device/CDrxDX12Device.hpp>
#include <drx3D/Render/D3D/DX12/Device/CDrxDX12DeviceContext.hpp>
#include <drx3D/Render/D3D/DX12/Device/CDrxDX12DeviceChild.hpp>
#include <drx3D/Render/D3D/DX12/Resource/State/CDrxDX12SamplerState.hpp>
#include <drx3D/Render/D3D/DX12/Resource/CDrxDX12View.hpp>
#include <drx3D/Render/D3D/DX12/Resource/View/CDrxDX12DepthStencilView.hpp>
#include <drx3D/Render/D3D/DX12/Resource/View/CDrxDX12RenderTargetView.hpp>
#include <drx3D/Render/D3D/DX12/Resource/View/CDrxDX12ShaderResourceView.hpp>
#include <drx3D/Render/D3D/DX12/Resource/View/CDrxDX12UnorderedAccessView.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Misc/CDrxDX12Buffer.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Texture/CDrxDX12Texture1D.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Texture/CDrxDX12Texture2D.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Texture/CDrxDX12Texture3D.hpp>
#include <drx3D/Render/D3D/DX12/Resource/CDrxDX12Asynchronous.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Misc/CDrxDX12Query.hpp>
#include <drx3D/Render/D3D/DX12/Resource/Misc/CDrxDX12InputLayout.hpp>

// DXGI emulation
#include <drx3D/Render/D3D/DX12/GI/CDrxDX12GIOutput.hpp>
#include <drx3D/Render/D3D/DX12/GI/CDrxDX12GIAdapter.hpp>
#include <drx3D/Render/D3D/DX12/GI/CDrxDX12SwapChain.hpp>
#include <drx3D/Render/D3D/DX12/GI/CDrxDX12GIFactory.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI DX12CreateDXGIFactory1(REFIID riid, uk * ppFactory);

HRESULT WINAPI DX12CreateDevice(
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
