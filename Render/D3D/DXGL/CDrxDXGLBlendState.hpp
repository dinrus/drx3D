// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLBlendState.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11BlendState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLBLENDSTATE__
#define __DRXDXGLBLENDSTATE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SBlendState;
class CContext;
}

class CDrxDXGLBlendState : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLBlendState, D3D11BlendState)

	CDrxDXGLBlendState(const D3D11_BLEND_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLBlendState();

	bool Initialize(CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext);
	bool Apply(NDrxOpenGL::CContext* pContext);

	// Implementation of ID3D11BlendState
	void GetDesc(D3D11_BLEND_DESC* pDesc);
protected:
	D3D11_BLEND_DESC         m_kDesc;
	NDrxOpenGL::SBlendState* m_pGLState;
};

#endif //__DRXDXGLBLENDSTATE__
