// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLBuffer.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11Buffer
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLBuffer.hpp>
#include <drx3D/Render/CDrxDXGLDeviceContext.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLBuffer::CDrxDXGLBuffer(const D3D11_BUFFER_DESC& kDesc, NDrxOpenGL::SBuffer* pGLBuffer, CDrxDXGLDevice* pDevice)
	: CDrxDXGLResource(D3D11_RESOURCE_DIMENSION_BUFFER, pGLBuffer, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Buffer)
}

#if OGL_SINGLE_CONTEXT

CDrxDXGLBuffer::CDrxDXGLBuffer(const D3D11_BUFFER_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialData, CDrxDXGLDevice* pDevice)
	: CDrxDXGLResource(D3D11_RESOURCE_DIMENSION_BUFFER, pInitialData, pDevice)
	, m_kDesc(kDesc)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Buffer)
}

#endif

CDrxDXGLBuffer::~CDrxDXGLBuffer()
{
}

#if OGL_SINGLE_CONTEXT

void CDrxDXGLBuffer::Initialize()
{
	NDrxOpenGL::CContext* pContext = m_pDevice->GetGLDevice()->GetCurrentContext();
	if (m_spInitialDataCopy)
	{
		m_spGLResource = NDrxOpenGL::CreateBuffer(m_kDesc, m_spInitialDataCopy->m_akSubresourceData, pContext);
		m_spInitialDataCopy = NULL;
	}
	else
		m_spGLResource = NDrxOpenGL::CreateBuffer(m_kDesc, NULL, pContext);

	if (m_spGLResource == NULL)
		DXGL_FATAL("Deferred buffer creation failed");
}

#endif

NDrxOpenGL::SBuffer* CDrxDXGLBuffer::GetGLBuffer()
{
#if OGL_SINGLE_CONTEXT
	IF_UNLIKELY (!m_spGLResource)
		Initialize();
#endif
	return static_cast<NDrxOpenGL::SBuffer*>(m_spGLResource.get());
}

////////////////////////////////////////////////////////////////////////////////
// ID3D11Buffer implementation
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLBuffer::GetDesc(D3D11_BUFFER_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
}
