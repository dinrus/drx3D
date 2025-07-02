// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLRasterizerState.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11RasterizerState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLRasterizerState.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLState.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLRasterizerState::CDrxDXGLRasterizerState(const D3D11_RASTERIZER_DESC& kDesc, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_kDesc(kDesc)
	, m_pGLState(new NDrxOpenGL::SRasterizerState)
{
	DXGL_INITIALIZE_INTERFACE(D3D11RasterizerState)
}

CDrxDXGLRasterizerState::~CDrxDXGLRasterizerState()
{
	delete m_pGLState;
}

bool CDrxDXGLRasterizerState::Initialize(CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext)
{
	return NDrxOpenGL::InitializeRasterizerState(m_kDesc, *m_pGLState, pContext);
}

bool CDrxDXGLRasterizerState::Apply(NDrxOpenGL::CContext* pContext)
{
	return pContext->SetRasterizerState(*m_pGLState);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of ID3D11RasterizerState
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLRasterizerState::GetDesc(D3D11_RASTERIZER_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
}
