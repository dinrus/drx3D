// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTexture2D.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Texture2D
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLTEXTURE2D__
#define __DRXDXGLTEXTURE2D__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLTextureBase.hpp>

class CDrxDXGLTexture2D : public CDrxDXGLTextureBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLTexture2D, D3D11Texture2D)

	CDrxDXGLTexture2D(const D3D11_TEXTURE2D_DESC& kDesc, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLTexture2D(const D3D11_TEXTURE2D_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice);
#endif
	virtual ~CDrxDXGLTexture2D();

#if OGL_SINGLE_CONTEXT
	virtual void Initialize();
#endif

	// Implementation of ID3D11Texture2D
	void GetDesc(D3D11_TEXTURE2D_DESC* pDesc);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLTexture2D>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLTextureBase::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
private:
	D3D11_TEXTURE2D_DESC m_kDesc;
};

#endif //__DRXDXGLTEXTURE2D__
