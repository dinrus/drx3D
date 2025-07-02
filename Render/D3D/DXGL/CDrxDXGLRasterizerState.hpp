// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLRasterizerState.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11RasterizerState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLRASTERIZERSTATE__
#define __DRXDXGLRASTERIZERSTATE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SRasterizerState;
class CContext;
};

class CDrxDXGLRasterizerState : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLRasterizerState, D3D11RasterizerState)

	CDrxDXGLRasterizerState(const D3D11_RASTERIZER_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLRasterizerState();

	bool Initialize(CDrxDXGLDevice* pDevice, NDrxOpenGL::CContext* pContext);
	bool Apply(NDrxOpenGL::CContext* pContext);

	// Implementation of ID3D11RasterizerState
	void GetDesc(D3D11_RASTERIZER_DESC* pDesc);
protected:
	D3D11_RASTERIZER_DESC         m_kDesc;
	NDrxOpenGL::SRasterizerState* m_pGLState;
};

#endif //__DRXDXGLRASTERIZERSTATE__
