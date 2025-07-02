// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLView.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrapper for ID3D11View
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLView.hpp>
#include <drx3D/Render/CDrxDXGLResource.hpp>
#include <drx3D/Render/Implementation/GLResource.hpp>

CDrxDXGLView::CDrxDXGLView(CDrxDXGLResource* pResource, CDrxDXGLDevice* pDevice)
	: CDrxDXGLDeviceChild(pDevice)
	, m_spResource(pResource)
{
	DXGL_INITIALIZE_INTERFACE(D3D11View)
}

CDrxDXGLView::~CDrxDXGLView()
{
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of ID3D11View
////////////////////////////////////////////////////////////////////////////////

void CDrxDXGLView::GetResource(ID3D11Resource** ppResource)
{
	if (m_spResource != NULL)
		m_spResource->AddRef();
	CDrxDXGLResource::ToInterface(ppResource, m_spResource);
}
