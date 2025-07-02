// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLSamplerState.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11SamplerState
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSAMPLERSTATE__
#define __DRXDXGLSAMPLERSTATE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLDeviceChild.hpp>

namespace NDrxOpenGL
{
struct SSamplerState;
class CContext;
}

class CDrxDXGLSamplerState : public CDrxDXGLDeviceChild
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLSamplerState, D3D11SamplerState)

	CDrxDXGLSamplerState(const D3D11_SAMPLER_DESC& kDesc, CDrxDXGLDevice* pDevice);
	virtual ~CDrxDXGLSamplerState();

	bool Initialize(NDrxOpenGL::CContext* pContext);
	void Apply(u32 uStage, u32 uSlot, NDrxOpenGL::CContext* pContext);

	// Implementation of ID3D11SamplerState
	void GetDesc(D3D11_SAMPLER_DESC* pDesc);
protected:
	D3D11_SAMPLER_DESC         m_kDesc;
	NDrxOpenGL::SSamplerState* m_pGLState;
};

#endif //__DRXDXGLSAMPLERSTATE__
