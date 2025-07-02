// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTextureBase.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL common base class for textures
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLTEXTUREBASE__
#define __DRXDXGLTEXTUREBASE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLResource.hpp>

namespace NDrxOpenGL
{
struct STexture;
};

class CDrxDXGLTextureBase : public CDrxDXGLResource
{
public:
	virtual ~CDrxDXGLTextureBase();

	NDrxOpenGL::STexture* GetGLTexture();
protected:
	CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice);
#endif
};

#endif //__DRXDXGLTEXTUREBASE__
