// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#if VK_USE_DXGI

#include <drx3D/Render/CDrxVKGIAdapter_DXGI.hpp>
#include <drx3D/Render/CDrxVKGIFactory_DXGI.hpp>
#include <drx3D/Render/CDrxVKGIOutput_DXGI.hpp>

CDrxVKGIAdapter_DXGI* CDrxVKGIAdapter_DXGI::Create(CDrxVKGIFactory* factory, _smart_ptr<IDXGIAdapterToCall> pNativeDxgiAdapter, uint index)
{
	if (pNativeDxgiAdapter)
	{
		return new CDrxVKGIAdapter_DXGI(factory, pNativeDxgiAdapter, index);
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxVKGIAdapter_DXGI::CDrxVKGIAdapter_DXGI(CDrxVKGIFactory* pFactory, _smart_ptr<IDXGIAdapterToCall> pNativeDxgiAdapter, uint index)
	: CDrxVKGIAdapter(pFactory, index)
	, m_pNativeDxgiAdapter(std::move(pNativeDxgiAdapter))
{}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxVKGIAdapter_DXGI::EnumOutputs(UINT Output, _COM_Outptr_ IDXGIOutput** ppOutput)
{
	VK_FUNC_LOG();

	IDXGIOutputToCall* pNativeOutputRaw;
	HRESULT hr = m_pNativeDxgiAdapter->EnumOutputs(Output, &pNativeOutputRaw);

	if (hr== S_OK)
	{
		_smart_ptr<IDXGIOutputToCall> pNativeOutput;
		pNativeOutput.Assign_NoAddRef(pNativeOutputRaw);

		*ppOutput = CDrxVKGIOutput_DXGI::Create(this, pNativeOutput, Output);
	}

	return hr;
}

HRESULT CDrxVKGIAdapter_DXGI::GetDesc1(
	_Out_ DXGI_ADAPTER_DESC1* pDesc)
{
	return m_pNativeDxgiAdapter->GetDesc1(pDesc);
}

#endif