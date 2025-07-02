// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDepthStencilView.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11DepthStencilView
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLDEPTHSTENCILVIEW__
#define __DRXDXGLDEPTHSTENCILVIEW__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLView.hpp>

namespace NDrxOpenGL
{
struct SOutputMergerView;
class CContext;
}

class CDrxDXGLDepthStencilView : public CDrxDXGLView
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDepthStencilView, D3D11DepthStencilView)

	CDrxDXGLDepthStencilView(CDrxDXGLResource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLDepthStencilView();

	bool Initialize(NDrxOpenGL::CContext* pContext);

#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SOutputMergerView* GetGLView(NDrxOpenGL::CContext* pContext);
#else
	NDrxOpenGL::SOutputMergerView* GetGLView();
#endif

	// Implementation of ID3D11DepthStencilView
	void GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc);
protected:
	D3D11_DEPTH_STENCIL_VIEW_DESC             m_kDesc;
	_smart_ptr<NDrxOpenGL::SOutputMergerView> m_spGLView;
};

#endif //__DRXDXGLDEPTHSTENCILVIEW__
