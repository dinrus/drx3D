// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLQuery.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Query
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLQUERY__
#define __DRXDXGLQUERY__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SQuery;
class CContext;
}

class CDrxDXGLQuery : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLQuery, D3D11Query)
#if DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLQuery, D3D11Asynchronous)
#endif //DXGL_FULL_EMULATION

	CDrxDXGLQuery(const D3D11_QUERY_DESC& kDesc, NDrxOpenGL::SQuery* pGLQuery, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLQuery();

#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SQuery* GetGLQuery(NDrxOpenGL::CContext* pContext);
#else
	NDrxOpenGL::SQuery* GetGLQuery();
#endif

	// ID3D11Asynchronous implementation
	UINT GetDataSize(void);

	// ID3D11Query implementation
	void GetDesc(D3D11_QUERY_DESC* pDesc);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLQuery>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLDeviceChild::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
private:
	D3D11_QUERY_DESC               m_kDesc;
	_smart_ptr<NDrxOpenGL::SQuery> m_spGLQuery;
};

#endif //__DRXDXGLQUERY__
