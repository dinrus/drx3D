// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTexture3D.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11Texture3D
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLTexture3D.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLTexture3D::CDrxDXGLTexture3D(const D3D11_TEXTURE3D_DESC& kDesc, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice)
	: CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION_TEXTURE3D, pGLTexture, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Texture3D)
}

#if OGL_SINGLE_CONTEXT

CDrxDXGLTexture3D::CDrxDXGLTexture3D(const D3D11_TEXTURE3D_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice)
	: CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION_TEXTURE3D, pInitialData, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Texture3D)
}

#endif

CDrxDXGLTexture3D::~CDrxDXGLTexture3D()
{
}

#if OGL_SINGLE_CONTEXT

void CDrxDXGLTexture3D::Initialize()
{
	NDrxOpenGL::CContext* pContext = m_pDevice->GetGLDevice()->GetCurrentContext();
	if (m_spInitialDataCopy)
	{
		m_spGLResource = NDrxOpenGL::CreateTexture3D(m_kDesc, m_spInitialDataCopy->m_akSubresourceData, pContext);
		m_spInitialDataCopy = NULL;
	}
	else
		m_spGLResource = NDrxOpenGL::CreateTexture3D(m_kDesc, NULL, pContext);

	if (m_spGLResource == NULL)
		DXGL_FATAL("Deferred 3D texture creation failed");
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation of ID3D11Texture3D
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLTexture3D::GetDesc(D3D11_TEXTURE3D_DESC* pDesc)
{
	*pDesc = m_kDesc;
}
