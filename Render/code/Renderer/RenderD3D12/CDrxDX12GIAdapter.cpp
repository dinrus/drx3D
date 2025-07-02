// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12GIAdapter.hpp>

#include <drx3D/Render/CDrxDX12GIFactory.hpp>
#include <drx3D/Render/CDrxDX12GIOutput.hpp>

CDrxDX12GIAdapter* CDrxDX12GIAdapter::Create(CDrxDX12GIFactory* pFactory, UINT Adapter)
{
	IDXGIAdapter1ToCall* pAdapter1;
	pFactory->GetDXGIFactory()->EnumAdapters1(Adapter, &pAdapter1);
	IDXGIAdapter3ToCall* pAdapter3;
	pAdapter1->QueryInterface(__uuidof(IDXGIAdapter3ToCall), (uk *)&pAdapter3);

	return pAdapter3 ? DX12_NEW_RAW(CDrxDX12GIAdapter(pFactory, pAdapter3)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12GIAdapter::CDrxDX12GIAdapter(CDrxDX12GIFactory* pFactory, IDXGIAdapter3ToCall* pAdapter)
	: Super()
	, m_pFactory(pFactory)
	, m_pDXGIAdapter3(pAdapter)
{
	DX12_FUNC_LOG
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region /* IDXGIAdapter implementation */

HRESULT STDMETHODCALLTYPE CDrxDX12GIAdapter::EnumOutputs(UINT Output, _COM_Outptr_ IDXGIOutput** ppOutput)
{
	DX12_FUNC_LOG
	* ppOutput = CDrxDX12GIOutput::Create(this, Output);
	return *ppOutput ? S_OK : E_FAIL;
}

#pragma endregion
