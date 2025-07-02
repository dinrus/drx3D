// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLInputLayout.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11InputLayout
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLINPUTLAYOUT__
#define __DRXDXGLINPUTLAYOUT__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SInputLayout;
}

class CDrxDXGLInputLayout : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLInputLayout, D3D11InputLayout)

	CDrxDXGLInputLayout(NDrxOpenGL::SInputLayout* pGLLayout, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLInputLayout();

	NDrxOpenGL::SInputLayout* GetGLLayout();
private:
	_smart_ptr<NDrxOpenGL::SInputLayout> m_spGLLayout;
};

#endif //__DRXDXGLINPUTLAYOUT__
