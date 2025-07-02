// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLShaderResourceView.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11ShaderResourceView
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSHADERRESOURCEVIEW__
#define __DRXDXGLSHADERRESOURCEVIEW__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLView.hpp>

namespace NDrxOpenGL
{
struct SShaderView;
class CContext;
}

class CDrxDXGLShaderResourceView : public CDrxDXGLView
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLShaderResourceView, D3D11ShaderResourceView)

	CDrxDXGLShaderResourceView(CDrxDXGLResource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLShaderResourceView();

	bool Initialize(NDrxOpenGL::CContext* pContext);

#if OGL_SINGLE_CONTEXT
	NDrxOpenGL::SShaderView*       GetGLView(NDrxOpenGL::CContext* pContext);
#else
	ILINE NDrxOpenGL::SShaderView* GetGLView()
	{
		return m_spGLView;
	}
#endif

	// Implementation of ID3D11ShaderResourceView
	void GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc);
protected:
	D3D11_SHADER_RESOURCE_VIEW_DESC     m_kDesc;
	_smart_ptr<NDrxOpenGL::SShaderView> m_spGLView;
};

#endif //__DRXDXGLSHADERRESOURCEVIEW__
