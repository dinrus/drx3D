// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSamplerState.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11SamplerState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLSamplerState.hpp>
#include <drx3D/Render/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Implementation/GLState.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

CDrxDXGLSamplerState::CDrxDXGLSamplerState(const D3D11_SAMPLER_DESC& kDesc, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_kDesc(kDesc)
	, m_pGLState(NULL)
{
	DXGL_INITIALIZE_INTERFACE(D3D11SamplerState)
}

CDrxDXGLSamplerState::~CDrxDXGLSamplerState()
{
	delete m_pGLState;
}

bool CDrxDXGLSamplerState::Initialize(NDrxOpenGL::CContext* pContext)
{
	assert(m_pGLState == NULL);
	m_pGLState = new NDrxOpenGL::SSamplerState;
	return NDrxOpenGL::InitializeSamplerState(m_kDesc, *m_pGLState, pContext);
}

void CDrxDXGLSamplerState::Apply(u32 uStage, u32 uSlot, NDrxOpenGL::CContext* pContext)
{
	IF_UNLIKELY (!m_pGLState)
	{
		if (!Initialize(pContext))
			DXGL_FATAL("Deferred sampler state creation failed");
	}
	pContext->SetSampler(m_pGLState, uStage, uSlot);
}

////////////////////////////////////////////////////////////////
// Implementation of ID3D11SamplerState
////////////////////////////////////////////////////////////////

void CDrxDXGLSamplerState::GetDesc(D3D11_SAMPLER_DESC* pDesc)
{
	(*pDesc) = m_kDesc;
}
