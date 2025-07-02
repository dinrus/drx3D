// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDX12GIOutput.hpp>

#include <drx3D/Render/CDrxDX12GIAdapter.hpp>

CDrxDX12GIOutput* CDrxDX12GIOutput::Create(CDrxDX12GIAdapter* pAdapter, UINT Output)
{
	IDXGIOutputToCall* pOutput;
	if (!SUCCEEDED(pAdapter->GetDXGIAdapter()->EnumOutputs(Output, &pOutput)))
		return nullptr;

	IDXGIOutput4ToCall* pOutput4;
	pOutput->QueryInterface(__uuidof(IDXGIOutput4ToCall), (uk *)&pOutput4);

	return pOutput4 ? DX12_NEW_RAW(CDrxDX12GIOutput(pAdapter, pOutput4)) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxDX12GIOutput::CDrxDX12GIOutput(CDrxDX12GIAdapter* pAdapter, IDXGIOutput4ToCall* pOutput)
	: Super()
	, m_pAdapter(pAdapter)
	, m_pDXGIOutput4(pOutput)
{
	DX12_FUNC_LOG
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __d3d11_x_h__
HRESULT STDMETHODCALLTYPE CDrxDX12GIOutput::FindClosestMatchingMode(_In_ const DXGI_MODE_DESC* pModeToMatch, _Out_ DXGI_MODE_DESC* pClosestMatch, _In_opt_ IUnknown* pConcernedDevice)
{
	return m_pDXGIOutput4->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice ? reinterpret_cast<IUnknown*>(static_cast<CDrxDX12Device*>(pConcernedDevice)->GetD3D12Device()) : nullptr);
}

HRESULT STDMETHODCALLTYPE CDrxDX12GIOutput::TakeOwnership(_In_ IUnknown* pDevice, BOOL Exclusive)
{
	return m_pDXGIOutput4->TakeOwnership(pDevice ? reinterpret_cast<IUnknown*>(static_cast<CDrxDX12Device*>(pDevice)->GetD3D12Device()) : nullptr, Exclusive);
}
#endif

#ifdef __dxgi1_2_h__
HRESULT STDMETHODCALLTYPE CDrxDX12GIOutput::FindClosestMatchingMode1(_In_ const DXGI_MODE_DESC1* pModeToMatch, _Out_ DXGI_MODE_DESC1* pClosestMatch, _In_opt_ IUnknown* pConcernedDevice)
{
	return m_pDXGIOutput4->FindClosestMatchingMode1(pModeToMatch, pClosestMatch, pConcernedDevice ? reinterpret_cast<IUnknown*>(static_cast<CDrxDX12Device*>(pConcernedDevice)->GetD3D12Device()) : nullptr);
}
#endif

#ifdef __dxgi1_3_h__
HRESULT STDMETHODCALLTYPE CDrxDX12GIOutput::CheckOverlaySupport(_In_ DXGI_FORMAT EnumFormat, _In_ IUnknown* pConcernedDevice, _Out_ UINT* pFlags)
{
	return m_pDXGIOutput4->CheckOverlaySupport(EnumFormat, reinterpret_cast<IUnknown*>(static_cast<CDrxDX12Device*>(pConcernedDevice)->GetD3D12Device()), pFlags);
}
#endif

#ifdef __dxgi1_4_h__
HRESULT STDMETHODCALLTYPE CDrxDX12GIOutput::CheckOverlayColorSpaceSupport(_In_ DXGI_FORMAT Format, _In_ DXGI_COLOR_SPACE_TYPE ColorSpace, _In_ IUnknown* pConcernedDevice, _Out_ UINT* pFlags)
{
	return m_pDXGIOutput4->CheckOverlayColorSpaceSupport(Format, ColorSpace, reinterpret_cast<IUnknown*>(static_cast<CDrxDX12Device*>(pConcernedDevice)->GetD3D12Device()), pFlags);
}
#endif
