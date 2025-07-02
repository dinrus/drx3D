// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLInputLayout.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11InputLayout
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLInputLayout.hpp>
#include <drx3D/Render/Implementation/GLShader.hpp>

CDrxDXGLInputLayout::CDrxDXGLInputLayout(NDrxOpenGL::SInputLayout* pGLLayout, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_spGLLayout(pGLLayout)
{
	DXGL_INITIALIZE_INTERFACE(D3D11InputLayout)
}

CDrxDXGLInputLayout::~CDrxDXGLInputLayout()
{
}

NDrxOpenGL::SInputLayout* CDrxDXGLInputLayout::GetGLLayout()
{
	return m_spGLLayout;
}
