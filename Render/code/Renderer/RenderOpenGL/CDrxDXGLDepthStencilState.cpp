// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDepthStencilState.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11DepthStencilState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLDepthStencilState.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLState.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLDepthStencilState::CDrxDXGLDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& kDesc, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_kDesc(kDesc)
	, m_pGLState(new NDrxOpenGL::SDepthStencilState)
{
	DXGL_INITIALIZE_INTERFACE(D3D11DepthStencilState)
}

CDrxDXGLDepthStencilState::~CDrxDXGLDepthStencilState()
{
	delete m_pGLState;
}

bool CDrxDXGLDepthStencilState::Initialize(CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext)
{
	return NDrxOpenGL::InitializeDepthStencilState(m_kDesc, *m_pGLState, pContext);
}

bool CDrxDXGLDepthStencilState::Apply(u32 uStencilReference, NDrxOpenGL::CContext* pContext)
{
	return pContext->SetDepthStencilState(*m_pGLState, static_cast<GLint>(uStencilReference));
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of ID3D11DepthStencilState
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLDepthStencilState::GetDesc(D3D11_DEPTH_STENCIL_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
}
