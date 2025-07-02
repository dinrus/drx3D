// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLResource.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Resource
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLRESOURCE__
#define __DRXDXGLRESOURCE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
class CDevice;
struct SResource;
struct SInitialDataCopy;
};

class CDrxDXGLResource : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLResource, D3D11Resource)

	virtual ~CDrxDXGLResource();

#if OGL_SINGLE_CONTEXT
	virtual void Initialize() = 0;
#endif

	ILINE NDrxOpenGL::SResource* GetGLResource()
	{
#if OGL_SINGLE_CONTEXT
		IF_UNLIKELY (!m_spGLResource)
			Initialize();
#endif
		return m_spGLResource;
	}

	// Implementation of ID3D11Resource
	void GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension);
	void SetEvictionPriority(UINT EvictionPriority);
	UINT GetEvictionPriority(void);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLResource>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLDeviceChild::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
protected:
	CDrxDXGLResource(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::SResource* pResource, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLResource(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::SInitialDataCopy* pInitialDataCopy, CDrxDXGLDevice* pDevice);
#endif
protected:
	_smart_ptr<NDrxOpenGL::SResource>        m_spGLResource;
#if OGL_SINGLE_CONTEXT
	_smart_ptr<NDrxOpenGL::SInitialDataCopy> m_spInitialDataCopy;
#endif
	D3D11_RESOURCE_DIMENSION                 m_eDimension;
};

#endif //__DRXDXGLRESOURCE__
