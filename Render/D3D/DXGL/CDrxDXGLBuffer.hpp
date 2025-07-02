// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLBuffer.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Buffer
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLBUFFER__
#define __DRXDXGLBUFFER__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLResource.hpp>

namespace NDrxOpenGL
{
struct SBuffer;
}

class CDrxDXGLBuffer : public CDrxDXGLResource
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLBuffer, D3D11Buffer)

	CDrxDXGLBuffer(const D3D11_BUFFER_DESC& kDesc, NDrxOpenGL::SBuffer* pGLBuffer, CDrxDXGLDevice* pDevice);
#if OGL_SINGLE_CONTEXT
	CDrxDXGLBuffer(const D3D11_BUFFER_DESC& kDesc, NDrxOpenGL::SInitialDataCopy* pInitialDataCopy, CDrxDXGLDevice* pDevice);
#endif
	virtual ~CDrxDXGLBuffer();

#if OGL_SINGLE_CONTEXT
	virtual void Initialize();
#endif

	NDrxOpenGL::SBuffer* GetGLBuffer();

	// ID3D11Buffer implementation
	void GetDesc(D3D11_BUFFER_DESC* pDesc);
private:
	D3D11_BUFFER_DESC m_kDesc;
};

#endif //__DRXDXGLBUFFER__
