// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLRenderTargetView.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11RenderTargetView
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLRENDERTARGETVIEW__
#define __DRXDXGLRENDERTARGETVIEW__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLView.hpp>

namespace NDrxOpenGL
{
struct SOutputMergerView;
class CContext;
}

class CDrxDXGLRenderTargetView : public CDrxDXGLView
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLRenderTargetView, D3D11RenderTargetView)

	CDrxDXGLRenderTargetView(CDrxDXGLResource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLRenderTargetView();

	bool Initialize(NDrxOpenGL::CContext* pContext);

#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SOutputMergerView* GetGLView(NDrxOpenGL::CContext* pContext);
#else
	NDrxOpenGL::SOutputMergerView* GetGLView();
#endif

	// Implementation of ID3D11RenderTargetView
	void GetDesc(D3D11_RENDER_TARGET_VIEW_DESC* pDesc);
private:
	D3D11_RENDER_TARGET_VIEW_DESC             m_kDesc;
	_smart_ptr<NDrxOpenGL::SOutputMergerView> m_spGLView;
};

#endif //__DRXDXGLRENDERTARGETVIEW__
