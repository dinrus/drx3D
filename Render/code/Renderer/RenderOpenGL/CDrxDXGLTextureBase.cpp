// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTextureBase.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL common base class for textures
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLTextureBase.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>

CDrxDXGLTextureBase::CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice)
	: CDrxDXGLResource(eDimension, pGLTexture, pDevice)
{
}

#if OGL_SINGLE_CONTEXT

CDrxDXGLTextureBase::CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION eDimension, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice)
	: CDrxDXGLResource(eDimension, pInitialData, pDevice)
{
}

#endif

CDrxDXGLTextureBase::~CDrxDXGLTextureBase()
{
}

NDrxOpenGL::STexture* CDrxDXGLTextureBase::GetGLTexture()
{
#if OGL_SINGLE_CONTEXT
	IF_UNLIKELY (!m_spGLResource)
		Initialize();
#endif
	return static_cast<NDrxOpenGL::STexture*>(m_spGLResource.get());
}
