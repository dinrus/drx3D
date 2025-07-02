// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxVKGIAdapter.hpp>

#include <drx3D/Render/CDrxVKGIFactory.hpp>
#include <drx3D/Render/CDrxVKGIOutput.hpp>

CDrxVKGIAdapter* CDrxVKGIAdapter::Create(CDrxVKGIFactory* pFactory, UINT Adapter)
{
	return new CDrxVKGIAdapter(pFactory, Adapter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDrxVKGIAdapter::CDrxVKGIAdapter(CDrxVKGIFactory* pFactory, UINT index)
	: m_pFactory(pFactory),
	m_deviceIndex(index)
{
	VK_FUNC_LOG();
}

CDrxVKGIAdapter::~CDrxVKGIAdapter()
{
	VK_FUNC_LOG();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CDrxVKGIAdapter::EnumOutputs(UINT Output, _COM_Outptr_ IDXGIOutput** ppOutput)
{
	VK_FUNC_LOG();
	* ppOutput = CDrxVKGIOutput::Create(this, Output);
	return *ppOutput ? S_OK : E_FAIL;
}
