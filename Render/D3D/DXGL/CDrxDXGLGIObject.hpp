// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLGIObject.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for IDXGIObject
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLGIOBJECT__
#define __DRXDXGLGIOBJECT__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLBase.hpp>

class CDrxDXGLGIObject : public CDrxDXGLBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLGIObject, DXGIObject)

	CDrxDXGLGIObject();
	virtual ~CDrxDXGLGIObject();

	// IDXGIObject implementation
	HRESULT SetPrivateData(REFGUID Name, UINT DataSize, ukk pData);
	HRESULT SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown);
	HRESULT GetPrivateData(REFGUID Name, UINT* pDataSize, uk pData);
	HRESULT GetParent(REFIID riid, uk * ppParent);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLGIObject>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLBase::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION

protected:
	CDrxDXGLPrivateDataContainer m_kPrivateDataContainer;
};

#endif //__DRXDXGLGIOBJECT__
