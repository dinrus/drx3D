// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLQuery.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11Query
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLQuery.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>

CDrxDXGLQuery::CDrxDXGLQuery(const D3D11_QUERY_DESC& kDesc, NDrxOpenGL::SQuery* pGLQuery, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_kDesc(kDesc)
	, m_spGLQuery(pGLQuery)
{
	DXGL_INITIALIZE_INTERFACE(D3D11Asynchronous)
	DXGL_INITIALIZE_INTERFACE(D3D11Query)
}

CDrxDXGLQuery::~CDrxDXGLQuery()
{
}

#if OGL_SINGLE_CONTEXT

NDrxOpenGL::SQuery* CDrxDXGLQuery::GetGLQuery(NDrxOpenGL::CContext* pContext)
{
	IF_UNLIKELY (!m_spGLQuery)
	{
		m_spGLQuery = NDrxOpenGL::CreateQuery(m_kDesc, pContext);
		if (!m_spGLQuery)
			DXGL_FATAL("Deferred query creation failed");
	}
	return m_spGLQuery;
}

#else

NDrxOpenGL::SQuery* CDrxDXGLQuery::GetGLQuery()
{
	return m_spGLQuery;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// ID3D11Asynchronous implementation
////////////////////////////////////////////////////////////////////////////////

UINT CDrxDXGLQuery::GetDataSize(void)
{
	return m_spGLQuery->GetDataSize();
}

////////////////////////////////////////////////////////////////////////////////
// ID3D11Query implementation
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLQuery::GetDesc(D3D11_QUERY_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
}
