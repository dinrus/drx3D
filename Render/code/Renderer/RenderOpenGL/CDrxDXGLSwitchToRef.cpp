// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSwitchToRef.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11SwitchToRef
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLSwitchToRef.hpp>
#include <drx3D/Render/Implementation/GLCommon.hpp>

CDrxDXGLSwitchToRef::CDrxDXGLSwitchToRef(CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
{
	DXGL_INITIALIZE_INTERFACE(D3D11SwitchToRef)
}

CDrxDXGLSwitchToRef::~CDrxDXGLSwitchToRef()
{
}

////////////////////////////////////////////////////////////////////////////////
// ID3D11SwitchToRef implementation
////////////////////////////////////////////////////////////////////////////////

BOOL CDrxDXGLSwitchToRef::SetUseRef(BOOL UseRef)
{
	DXGL_NOT_IMPLEMENTED
	return false;
}

BOOL CDrxDXGLSwitchToRef::GetUseRef()
{
	DXGL_NOT_IMPLEMENTED
	return false;
}
