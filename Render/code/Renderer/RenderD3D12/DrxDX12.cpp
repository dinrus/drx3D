// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DrxDX12.hpp>

HRESULT WINAPI DX12CreateDXGIFactory1(REFIID riid, uk * ppFactory)
{
	*ppFactory = CDrxDX12GIFactory::Create();
	return *ppFactory ? 0 : -1;
}

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
  ID3D11DeviceContext** ppImmediateContext)
{
	*ppDevice = CDrxDX12Device::Create(reinterpret_cast<CDrxDX12GIAdapter*>(pAdapter), pFeatureLevel);

	if (!*ppDevice)
	{
		return -1;
	}

	(*ppDevice)->GetImmediateContext(ppImmediateContext);

	if (!*ppImmediateContext)
	{
		return -1;
	}

	return 0;
}
