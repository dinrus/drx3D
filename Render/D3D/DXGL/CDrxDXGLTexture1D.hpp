// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTexture1D.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Texture1D
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLTEXTURE1D__
#define __DRXDXGLTEXTURE1D__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLTextureBase.hpp>

class CDrxDXGLTexture1D : public CDrxDXGLTextureBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLTexture1D, D3D11Texture1D)

	CDrxDXGLTexture1D(const D3D11_TEXTURE1D_DESC& kDesc, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLTexture1D(const D3D11_TEXTURE1D_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice);
#endif
	virtual ~CDrxDXGLTexture1D();

#if OGL_SINGLE_CONTEXT
	virtual void Initialize();
#endif

	// Implementation of ID3D11Texture1D
	void GetDesc(D3D11_TEXTURE1D_DESC* pDesc);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLTexture1D>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLTextureBase::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
private:
	D3D11_TEXTURE1D_DESC m_kDesc;
};

#endif //__DRXDXGLTEXTURE1D__
