// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#if VK_USE_DXGI

#include <drx3D/Render/CDrxVKGIOutput_DXGI.hpp>
#include <drx3D/Render/CDrxVKGIAdapter_DXGI.hpp>

CDrxVKGIOutput_DXGI* CDrxVKGIOutput_DXGI::Create(CDrxVKGIAdapter_DXGI* Adapter, _smart_ptr<IDXGIOutputToCall> pNativeDxgiOutput, UINT Output)
{
	if (pNativeDxgiOutput)
	{
		return new CDrxVKGIOutput_DXGI(Adapter, pNativeDxgiOutput, Output);
	}

	return nullptr;
}

CDrxVKGIOutput_DXGI::CDrxVKGIOutput_DXGI(CDrxVKGIAdapter_DXGI* pAdapter, _smart_ptr<IDXGIOutputToCall> pNativeDxgiOutput, UINT Output)
	: CDrxVKGIOutput(pAdapter)
	, m_pNativeDxgiOutput(std::move(pNativeDxgiOutput))
{}

HRESULT CDrxVKGIOutput_DXGI::GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, _Inout_ UINT* pNumModes, _Out_writes_to_opt_(*pNumModes, *pNumModes)  DXGI_MODE_DESC* pDesc)
{
	return m_pNativeDxgiOutput->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
}

HRESULT CDrxVKGIOutput_DXGI::FindClosestMatchingMode(_In_ const DXGI_MODE_DESC* pModeToMatch, _Out_ DXGI_MODE_DESC* pClosestMatch, _In_opt_ IUnknown* pConcernedDevice)
{
	return m_pNativeDxgiOutput->FindClosestMatchingMode(pModeToMatch, pClosestMatch, nullptr);
}

HRESULT CDrxVKGIOutput_DXGI::GetDesc(DXGI_OUTPUT_DESC* pDesc)
{
	return m_pNativeDxgiOutput->GetDesc(pDesc);
}

#endif