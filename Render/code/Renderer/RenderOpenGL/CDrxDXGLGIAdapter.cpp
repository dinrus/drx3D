// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIAdapter.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for IDXGIAdapter
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLGIAdapter.hpp>
#include <drx3D/Render/CDrxDXGLGIFactory.hpp>
#include <drx3D/Render/CDrxDXGLGIOutput.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>
#include <drx3D/CoreX/String/UnicodeFunctions.h>

CDrxDXGLGIAdapter::CDrxDXGLGIAdapter(CDrxDXGLGIFactory* pFactory, NDrxOpenGL::SAdapter* pGLAdapter)
	: m_spGLAdapter(pGLAdapter)
	, m_spFactory(pFactory)
{
	DXGL_INITIALIZE_INTERFACE(DXGIAdapter)
	DXGL_INITIALIZE_INTERFACE(DXGIAdapter1)
}

CDrxDXGLGIAdapter::~CDrxDXGLGIAdapter()
{
}

bool CDrxDXGLGIAdapter::Initialize()
{
	memset(&m_kDesc1, 0, sizeof(m_kDesc1));
	Unicode::Convert(m_kDesc.Description, m_spGLAdapter->m_strRenderer);
	NDrxOpenGL::Memcpy(m_kDesc1.Description, m_kDesc.Description, sizeof(m_kDesc1.Description));

	switch (m_spGLAdapter->m_eDriverVendor)
	{
	case NDrxOpenGL::eDV_NVIDIA:
	case NDrxOpenGL::eDV_NOUVEAU:
		m_kDesc.VendorId = 0x10DE;
		break;
	case NDrxOpenGL::eDV_AMD:
	case NDrxOpenGL::eDV_ATI:
		m_kDesc.VendorId = 0x1002;
		break;
	case NDrxOpenGL::eDV_INTEL:
	case NDrxOpenGL::eDV_INTEL_OS:
		m_kDesc.VendorId = 0x8086;
		break;
	default:
		m_kDesc1.VendorId = 0;
		break;
	}

	std::vector<NDrxOpenGL::SOutputPtr> kGLOutputs;
	if (!NDrxOpenGL::DetectOutputs(*m_spGLAdapter, m_spGLAdapter->m_kOutputs))
		return false;

	m_kOutputs.reserve(m_spGLAdapter->m_kOutputs.size());

	std::vector<NDrxOpenGL::SOutputPtr>::const_iterator kGLOutputIter(m_spGLAdapter->m_kOutputs.begin());
	const std::vector<NDrxOpenGL::SOutputPtr>::const_iterator kGLOutputEnd(m_spGLAdapter->m_kOutputs.end());
	for (; kGLOutputIter != kGLOutputEnd; ++kGLOutputIter)
	{
		_smart_ptr<CDrxDXGLGIOutput> spOutput(new CDrxDXGLGIOutput(*kGLOutputIter));
		if (!spOutput->Initialize())
			return false;
		m_kOutputs.push_back(spOutput);
	}

	DXGL_TODO("Detect from available extensions")
	m_eSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	m_kDesc1.DedicatedVideoMemory = m_spGLAdapter->m_uVRAMBytes;

	return true;
}

D3D_FEATURE_LEVEL CDrxDXGLGIAdapter::GetSupportedFeatureLevel()
{
	return m_eSupportedFeatureLevel;
}

NDrxOpenGL::SAdapter* CDrxDXGLGIAdapter::GetGLAdapter()
{
	return m_spGLAdapter.get();
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIObject overrides
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIAdapter::GetParent(REFIID riid, uk * ppParent)
{
	IUnknown* pFactoryInterface;
	CDrxDXGLBase::ToInterface(&pFactoryInterface, m_spFactory);
	if (pFactoryInterface->QueryInterface(riid, ppParent) == S_OK && ppParent != NULL)
		return S_OK;
	return CDrxDXGLGIObject::GetParent(riid, ppParent);
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIAdapter implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIAdapter::EnumOutputs(UINT Output, IDXGIOutput** ppOutput)
{
	if (Output >= m_kOutputs.size())
	{
		ppOutput = NULL;
		return DXGI_ERROR_NOT_FOUND;
	}

	CDrxDXGLGIOutput::ToInterface(ppOutput, m_kOutputs.at(Output));
	(*ppOutput)->AddRef();

	return S_OK;
}

HRESULT CDrxDXGLGIAdapter::GetDesc(DXGI_ADAPTER_DESC* pDesc)
{
	*pDesc = m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLGIAdapter::CheckInterfaceSupport(REFGUID InterfaceName, LARGE_INTEGER* pUMDVersion)
{
	if (InterfaceName == __uuidof(ID3D10Device)
	    || InterfaceName == __uuidof(ID3D11Device)
#if !DXGL_FULL_EMULATION
	    || InterfaceName == __uuidof(CDrxDXGLDevice)
#endif //!DXGL_FULL_EMULATION
	    )
	{
		if (pUMDVersion != NULL)
		{
			DXGL_TODO("Put useful data here");
			pUMDVersion->HighPart = 0;
			pUMDVersion->LowPart = 0;
		}
		return S_OK;
	}
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIAdapter1 implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIAdapter::GetDesc1(DXGI_ADAPTER_DESC1* pDesc)
{
	*pDesc = m_kDesc1;
	return S_OK;
}
