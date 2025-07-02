// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef D3D_RENDER_PIPELINE_H
#define D3D_RENDER_PIPELINE_H

#include <drx3D/Render/RenderPipeline.h>

class CD3D9Renderer;
class IDrxSizer;

struct SRenderStatePassD3D
{
	u32                       m_nState;

	CHWShader_D3D::SHWSInstance* m_pShaderVS;
	CHWShader_D3D::SHWSInstance* m_pShaderPS;
};

struct SRenderStateD3D : public SRenderState
{
public:
	TArray<SRenderStatePassD3D> m_Passes;

public:
	void Execute();
};

class CRenderPipelineD3D : public CRenderPipeline
{
public:

public:
	static CRenderPipelineD3D* Create(CD3D9Renderer& renderer)
	{
		return(new CRenderPipelineD3D(renderer));
	}

	i32 CV_r_renderPipeline;

public:
	virtual void RT_CreateGPUStates(IRenderState* pRootState);
	virtual void RT_ReleaseGPUStates(IRenderState* pRootState);

public:
	~CRenderPipelineD3D();

	void                  FreeMemory();

	i32                   GetDeviceDataSize();
	void                  ReleaseDeviceObjects();
	HRESULT               RestoreDeviceObjects();
	void                  GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual SRenderState* CreateRenderState();

	uk                 operator new(size_t s)
	{
		u8* p = (u8*) malloc(s + 16 + 8);
		memset(p, 0, s + 16 + 8);
		u8* pRet = (u8*) ((size_t) (p + 16 + 8) & ~0xF);
		((u8**) pRet)[-1] = p;
		return pRet;
	}

	void operator delete(uk p)
	{
		free(((u8**)p)[-1]);
	}

private:
	CRenderPipelineD3D(CD3D9Renderer& renderer);

private:
	CD3D9Renderer& m_renderer;

};

#endif // D3D_RENDER_PIPELINE_H
