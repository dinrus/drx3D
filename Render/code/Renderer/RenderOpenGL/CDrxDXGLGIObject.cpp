// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIObject.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for IDXGIObject
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLGIObject.hpp>
#include <drx3D/Render/Implementation/GLCommon.hpp>

CDrxDXGLGIObject::CDrxDXGLGIObject()
{
	DXGL_INITIALIZE_INTERFACE(DXGIObject)
}

CDrxDXGLGIObject::~CDrxDXGLGIObject()
{
}

////////////////////////////////////////////////////////////////////////////////
// IDXGIObject implementation
////////////////////////////////////////////////////////////////////////////////

HRESULT CDrxDXGLGIObject::SetPrivateData(REFGUID Name, UINT DataSize, ukk pData)
{
	return m_kPrivateDataContainer.SetPrivateData(Name, DataSize, pData);
}

HRESULT CDrxDXGLGIObject::SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown)
{
	return m_kPrivateDataContainer.SetPrivateDataInterface(Name, pUnknown);
}

HRESULT CDrxDXGLGIObject::GetPrivateData(REFGUID Name, UINT* pDataSize, uk pData)
{
	return m_kPrivateDataContainer.GetPrivateData(Name, pDataSize, pData);
}

HRESULT CDrxDXGLGIObject::GetParent(REFIID riid, uk * ppParent)
{
	DXGL_TODO("Implement if required")
	* ppParent = NULL;
	return E_FAIL;
}
