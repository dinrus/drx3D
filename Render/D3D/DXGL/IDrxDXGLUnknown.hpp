// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IDrxDXGLUnknown.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Contains the cross platform replacement for COM IUnknown
//               interface on non-windows platforms
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLUNKNOWN__
#define __DRXDXGLUNKNOWN__

#if !DRX_PLATFORM_WINDOWS

	#include <drx3D/Render/D3D/DXGL/DrxDXGLGuid.hpp>

struct IUnknown
{
public:
	#if !DXGL_FULL_EMULATION
	virtual ~IUnknown(){};
	#endif //!DXGL_FULL_EMULATION

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

#endif

#endif
