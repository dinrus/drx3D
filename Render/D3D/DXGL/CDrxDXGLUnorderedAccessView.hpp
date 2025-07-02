// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLUnorderedAccessView.hpp
//  Version:     v1.00
//  Created:     18/06/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11UnorderedAccessView
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLUNORDEREDACCESSVIEW__
#define __DRXDXGLUNORDEREDACCESSVIEW__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLView.hpp>

namespace NDrxOpenGL
{
struct SShaderView;
class CContext;
}

class CDrxDXGLUnorderedAccessView : public CDrxDXGLView
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLUnorderedAccessView, D3D11UnorderedAccessView)

	CDrxDXGLUnorderedAccessView(CDrxDXGLResource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLUnorderedAccessView();

	bool Initialize(NDrxOpenGL::CContext* pContext);

#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SShaderView* GetGLView(NDrxOpenGL::CContext* pContext);
#else
	NDrxOpenGL::SShaderView* GetGLView();
#endif

	// Implementation of ID3D11UnorderedAccessView
	void GetDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc);
protected:
	D3D11_UNORDERED_ACCESS_VIEW_DESC    m_kDesc;
	_smart_ptr<NDrxOpenGL::SShaderView> m_spGLView;
};

#endif //__DRXDXGLUNORDEREDACCESSVIEW__
