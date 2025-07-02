// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDepthStencilState.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11DepthStencilState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLDEPTHSTENCILSTATE__
#define __DRXDXGLDEPTHSTENCILSTATE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SDepthStencilState;
class CContext;
}

class CDrxDXGLDepthStencilState : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDepthStencilState, D3D11DepthStencilState)

	CDrxDXGLDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLDepthStencilState();

	bool Initialize(CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext);
	bool Apply(u32 uStencilReference, NDrxOpenGL::CContext* pContext);

	// Implementation of ID3D11DepthStencilState
	void GetDesc(D3D11_DEPTH_STENCIL_DESC* pDesc);
protected:
	D3D11_DEPTH_STENCIL_DESC        m_kDesc;
	NDrxOpenGL::SDepthStencilState* m_pGLState;
};

#endif //__DRXDXGLDEPTHSTENCILSTATE__
