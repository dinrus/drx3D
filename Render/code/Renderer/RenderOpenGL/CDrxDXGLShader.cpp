// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLShader.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for D3D11 shader interfaces
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLShader.hpp>
#include <drx3D/Render/Implementation/GLShader.hpp>

CDrxDXGLShader::CDrxDXGLShader(NDrxOpenGL::SShader* pGLShader, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_spGLShader(pGLShader)
{
}

CDrxDXGLShader::~CDrxDXGLShader()
{
}

NDrxOpenGL::SShader* CDrxDXGLShader::GetGLShader()
{
	return m_spGLShader.get();
}
