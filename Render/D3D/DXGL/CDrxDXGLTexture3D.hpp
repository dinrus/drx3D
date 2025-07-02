// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTexture3D.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Texture3D
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLTEXTURE3D__
#define __DRXDXGLTEXTURE3D__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLTextureBase.hpp>

class CDrxDXGLTexture3D : public CDrxDXGLTextureBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLTexture3D, D3D11Texture3D)

	CDrxDXGLTexture3D(const D3D11_TEXTURE3D_DESC& kDesc, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLTexture3D(const D3D11_TEXTURE3D_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice);
#endif
	virtual ~CDrxDXGLTexture3D();

#if OGL_SINGLE_CONTEXT
	virtual void Initialize();
#endif

	// Implementation of ID3D11Texture3D
	void GetDesc(D3D11_TEXTURE3D_DESC* pDesc);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject)
	{
		if (SingleInterface<CDrxDXGLTexture3D>::Query(this, riid, ppvObject))
			return S_OK;
		return CDrxDXGLTextureBase::QueryInterface(riid, ppvObject);
	}
#endif //!DXGL_FULL_EMULATION
private:
	D3D11_TEXTURE3D_DESC m_kDesc;
};

#endif //__DRXDXGLTEXTURE3D__
