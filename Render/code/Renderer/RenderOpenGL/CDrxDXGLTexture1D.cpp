// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLTexture1D.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11Texture1D
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLTexture1D.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLTexture1D::CDrxDXGLTexture1D(const D3D11_TEXTURE1D_DESC& kDesc, NDrxOpenGL::STexture* pGLTexture, CDrxDXGLDevice* pDevice)
	: CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION_TEXTURE1D, pGLTexture, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Texture1D)
}

#if OGL_SINGLE_CONTEXT

CDrxDXGLTexture1D::CDrxDXGLTexture1D(const D3D11_TEXTURE1D_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice)
	: CDrxDXGLTextureBase(D3D11_RESOURCE_DIMENSION_TEXTURE1D, pInitialData, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Texture1D)
}

#endif

CDrxDXGLTexture1D::~CDrxDXGLTexture1D()
{
}

#if OGL_SINGLE_CONTEXT

void CDrxDXGLTexture1D::Initialize()
{
	NDrxOpenGL::CContext* pContext = m_pDevice->GetGLDevice()->GetCurrentContext();
	if (m_spInitialDataCopy)
	{
		m_spGLResource = NDrxOpenGL::CreateTexture1D(m_kDesc, m_spInitialDataCopy->m_akSubresourceData, pContext);
		m_spInitialDataCopy = NULL;
	}
	else
		m_spGLResource = NDrxOpenGL::CreateTexture1D(m_kDesc, NULL, pContext);

	if (m_spGLResource == NULL)
		DXGL_FATAL("Deferred 1D texture creation failed");
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation of ID3D11Texture1D
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLTexture1D::GetDesc(D3D11_TEXTURE1D_DESC* pDesc)
{
	*pDesc = m_kDesc;
}
