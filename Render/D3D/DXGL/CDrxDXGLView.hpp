// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLView.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11View
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLVIEW__
#define __DRXDXGLVIEW__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

class CDrxDXGLResource;

class CDrxDXGLView : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLView, D3D11View)
	virtual ~CDrxDXGLView();

	inline CDrxDXGLResource* GetGLResource() { return m_spResource; }

	// Implementation of ID3D11View
	void GetResource(ID3D11Resource** ppResource);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLView>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLDeviceChild::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
protected:
	CDrxDXGLView(CDrxDXGLResource* pResource, CDrxDXGLDevice* pDevice);

	_smart_ptr<CDrxDXGLResource> m_spResource;
};

#endif //__DRXDXGLVIEW__
