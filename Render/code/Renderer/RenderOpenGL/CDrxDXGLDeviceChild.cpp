// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDeviceChild.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11DeviceChild
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLDeviceChild.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>

CDrxDXGLDeviceChild::CDrxDXGLDeviceChild(CDrxDXGLDevice* pDevice)
	: m_pDevice(pDevice)
{
	DXGL_INITIALIZE_INTERFACE(D3D11DeviceChild)
	if (m_pDevice != NULL)
		m_pDevice->AddRef();
}

CDrxDXGLDeviceChild::~CDrxDXGLDeviceChild()
{
	if (m_pDevice != NULL)
		m_pDevice->Release();
}

void CDrxDXGLDeviceChild::SetDevice(CDrxDXGLDevice* pDevice)
{
	if (m_pDevice != pDevice)
	{
		if (m_pDevice != NULL)
			m_pDevice->Release();
		m_pDevice = pDevice;
		if (pDevice != NULL)
			m_pDevice->AddRef();
	}
}

////////////////////////////////////////////////////////////////////////////////
// ID3D11DeviceChild implementation
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLDeviceChild::GetDevice(ID3D11Device** ppDevice)
{
	CDrxDXGLDevice::ToInterface(ppDevice, m_pDevice);
}

HRESULT CDrxDXGLDeviceChild::GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData)
{
	return m_kPrivateDataContainer.GetPrivateData(guid, pDataSize, pData);
}

HRESULT CDrxDXGLDeviceChild::SetPrivateData(REFGUID guid, UINT DataSize, ukk pData)
{
	return m_kPrivateDataContainer.SetPrivateData(guid, DataSize, pData);
}

HRESULT CDrxDXGLDeviceChild::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
{
	return m_kPrivateDataContainer.SetPrivateDataInterface(guid, pData);
}
