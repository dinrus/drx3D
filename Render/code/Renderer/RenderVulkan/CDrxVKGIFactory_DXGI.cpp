// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#if VK_USE_DXGI

#include <drx3D/Render/CDrxVKGIFactory_DXGI.hpp>
#include <drx3D/Render/CDrxVKGIAdapter_DXGI.hpp>

CDrxVKGIFactory_DXGI* CDrxVKGIFactory_DXGI::Create()
{
	typedef HRESULT(WINAPI * FP_CreateDXGIFactory1)(REFIID, uk *);
	FP_CreateDXGIFactory1 pCreateDXGIFactory1 = (FP_CreateDXGIFactory1)GetProcAddress(LoadLibraryA("dxgi.dll"), "CreateDXGIFactory1");

	IDXGIFactoryToCall* pNativeFactoryRaw;
	if (pCreateDXGIFactory1(__uuidof(IDXGIFactoryToCall), (uk *)&pNativeFactoryRaw) == S_OK)
	{
		_smart_ptr<IDXGIFactoryToCall> pNativeFactory;
		pNativeFactory.Assign_NoAddRef(pNativeFactoryRaw);

		auto factory = new CDrxVKGIFactory_DXGI(pNativeFactory);
		if (factory->GetVkInstance() == VK_NULL_HANDLE) {
			delete factory;
			factory = nullptr;
		}
		return factory;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxVKGIFactory_DXGI::CDrxVKGIFactory_DXGI(_smart_ptr<IDXGIFactoryToCall> pNativeDxgiFactory)
	: m_pNativeDxgiFactory(std::move(pNativeDxgiFactory))
{
}

HRESULT CDrxVKGIFactory_DXGI::EnumAdapters1(
	UINT Adapter,
	_Out_ IDXGIAdapter1** ppAdapter)
{

	IDXGIAdapterToCall* pNativeAdapterRaw;
	HRESULT hr = m_pNativeDxgiFactory->EnumAdapters1(Adapter, &pNativeAdapterRaw);

	if (hr == S_OK)
	{
		_smart_ptr<IDXGIAdapterToCall> pNativedapter;
		pNativedapter.Assign_NoAddRef(pNativeAdapterRaw);

		*ppAdapter = CDrxVKGIAdapter_DXGI::Create(this, pNativedapter, Adapter);
	}

	return hr;
}

#endif