// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxVKGIOutput.hpp>

#include <drx3D/Render/CDrxVKGIAdapter.hpp>

CDrxVKGIOutput* CDrxVKGIOutput::Create(CDrxVKGIAdapter* pAdapter, UINT Output)
{
	return new CDrxVKGIOutput(pAdapter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxVKGIOutput::CDrxVKGIOutput(CDrxVKGIAdapter* pAdapter)
	: m_pAdapter(pAdapter)
{
	VK_FUNC_LOG();
}

CDrxVKGIOutput::~CDrxVKGIOutput()
{
	VK_FUNC_LOG();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CDrxVKGIOutput::FindClosestMatchingMode(_In_ const DXGI_MODE_DESC* pModeToMatch, _Out_ DXGI_MODE_DESC* pClosestMatch, _In_opt_ IUnknown* pConcernedDevice)
{
	*pClosestMatch = *pModeToMatch;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxVKGIOutput::TakeOwnership(_In_ IUnknown* pDevice, BOOL Exclusive)
{
	return S_OK;
}
