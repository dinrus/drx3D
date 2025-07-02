// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/D3DRenderPipeline.h>

IRenderPipeline* CD3D9Renderer::GetIRenderPipeline()
{
	return m_pRenderPipelineD3D;
}

CRenderPipelineD3D::CRenderPipelineD3D(CD3D9Renderer& renderer)
	: m_renderer(renderer)
{
	REGISTER_CVAR2("r_renderPipeline", &CV_r_renderPipeline, 0, VF_NULL, "");
}

CRenderPipelineD3D::~CRenderPipelineD3D()
{
}

void CRenderPipelineD3D::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject((tuk)this - 16, sizeof(*this) + 16);  // adjust for new operator
}

void CRenderPipelineD3D::ReleaseDeviceObjects()
{
}

HRESULT CRenderPipelineD3D::RestoreDeviceObjects()
{
	return S_OK;
}

i32 CRenderPipelineD3D::GetDeviceDataSize()
{
	i32 nSize = 0;
	return nSize;
}

SRenderState* CRenderPipelineD3D::CreateRenderState()
{
	SRenderStateD3D* pRS = new SRenderStateD3D;

	return pRS;
}

//==========================================================================================================

void CRenderPipelineD3D::RT_CreateGPUStates(IRenderState* pRootState)
{
	SRenderStateD3D* pSTD3D = (SRenderStateD3D*)pRootState;
	if (pSTD3D->m_bWasDeleted)
		return;
}
void CRenderPipelineD3D::RT_ReleaseGPUStates(IRenderState* pRootState)
{
	SRenderStateD3D* pSTD3D = (SRenderStateD3D*)pRootState;
}
