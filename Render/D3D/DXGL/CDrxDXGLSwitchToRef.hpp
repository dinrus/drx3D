// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSwitchToRef.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11SwitchToRef
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSWITCHTOREF__
#define __DRXDXGLSWITCHTOREF__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

class CDrxDXGLSwitchToRef : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLSwitchToRef, D3D11SwitchToRef)

	CDrxDXGLSwitchToRef(CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLSwitchToRef();

	// ID3D11SwitchToRef implementation
	BOOL SetUseRef(BOOL UseRef);
	BOOL GetUseRef();
};

#endif //__DRXDXGLSWITCHTOREF__
