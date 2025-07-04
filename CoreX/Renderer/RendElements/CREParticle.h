// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CREPARTICLE_H__
#define __CREPARTICLE_H__

#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/CoreX/Memory/MemoryAccess.h>

// forward declarations
class CREParticle;
typedef SVF_P3F_C4B_T4B_N3F2 SVF_Particle;

namespace gpu_pfx2
{
	class CParticleComponentRuntime;
}
class CDeviceGraphicsCommandInterface;

struct SParticleAxes
{
	Vec3 xAxis;
	Vec3 yAxis;
};

struct SParticleColorST
{
	UCol color;
	UCol st;
};

struct SRenderVertices
{
	FixedDynArray<SVF_Particle>     aVertices;
	FixedDynArray<u16>           aIndices;
	FixedDynArray<Vec3>             aPositions;
	FixedDynArray<SParticleAxes>    aAxes;
	FixedDynArray<SParticleColorST> aColorSTs;
	float                           fPixels;

	ILINE SRenderVertices()
		: fPixels(0.f) {}
};

struct SCameraInfo
{
	const CCamera* pCamera;
	IVisArea*      pCameraVisArea;
	bool           bCameraUnderwater;
	bool           bRecursivePass;

	SCameraInfo(const SRenderingPassInfo& passInfo) :
		pCamera(&passInfo.GetCamera()),
		pCameraVisArea(gEnv->p3DEngine->GetVisAreaFromPos(passInfo.GetCamera().GetOccPos())),
		bCameraUnderwater(passInfo.IsCameraUnderWater()),
		bRecursivePass(passInfo.IsRecursivePass())
	{}
};

struct IParticleVertexCreator
{
	//! Create the vertices for the particle emitter.
	virtual void ComputeVertices(const SCameraInfo& camInfo, CREParticle* pRE, uint64 uRenderFlags, float fMaxPixels) = 0;

	virtual ~IParticleVertexCreator() {}
};

class CCompiledParticle;
class CDeviceGraphicsPSO;
typedef std::shared_ptr<CCompiledParticle>  TCompiledParticlePtr;
typedef std::shared_ptr<CDeviceGraphicsPSO> CDeviceGraphicsPSOPtr;

class CREParticle : public CRenderElement
{
public:
	static const uint numBuffers = 3;

	enum EParticleObjFlags
	{
		ePOF_HALF_RES              = BIT(0),
		ePOF_VOLUME_FOG            = BIT(1),
		ePOF_USE_VERTEX_PULL_MODEL = BIT(2),
	};

public:
	CREParticle();

	//! Custom copy constructor required to avoid m_Lock copy.
	CREParticle(const CREParticle& in)
		: m_pCompiledParticle(in.m_pCompiledParticle)
		, m_pVertexCreator(in.m_pVertexCreator)
		, m_pGpuRuntime(in.m_pGpuRuntime)
		, m_nThreadId(in.m_nThreadId)
	{
	}

	static void ResetPool();

	void Reset(IParticleVertexCreator* pVC, i32 nThreadId, uint allocId);
	void SetRuntime(gpu_pfx2::CParticleComponentRuntime* pRuntime);

	//! CRenderElement implementation.
	virtual CRenderElement* mfCopyConstruct() override
	{
		return new CREParticle(*this);
	}
	virtual i32 Size() override
	{
		return sizeof(*this);
	}

	virtual bool Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly) override;
	virtual void DrawToCommandList(CRenderObject* pRenderObject, const struct SGraphicsPipelinePassContext& context, CDeviceCommandList* commandList) override;

	// Additional methods.

	//! Interface to alloc render verts and indices from 3DEngine code.
	virtual SRenderVertices* AllocVertices(i32 nAllocVerts, i32 nAllocInds);
	virtual SRenderVertices* AllocPullVertices(i32 nPulledVerts);

	void                     ComputeVertices(SCameraInfo camInfo, uint64 uRenderFlags);

	bool                     AddedToView() const { return m_addedToView != 0; }
	void                     SetAddedToView() { m_addedToView = 1; }

	void                     mfGetBBox(Vec3& vMins, Vec3& vMaxs) const override  { vMins = m_AABBmin; vMaxs = m_AABBmax; }
	void                     SetBBox(const Vec3& vMins, const Vec3& vMaxs)       { m_AABBmin = vMins; m_AABBmax = vMaxs; }

private:
	CDeviceGraphicsPSOPtr GetGraphicsPSO(CRenderObject* pRenderObject, const struct SGraphicsPipelinePassContext& context) const;
	void                  PrepareDataToRender(CRenderView *pRenderView,CRenderObject* pRenderObject);
	void                  BindPipeline(CRenderObject* pRenderObject, CDeviceGraphicsCommandInterface& commandInterface, CDeviceGraphicsPSOPtr pGraphicsPSO);
	void                  DrawParticles(CRenderObject* pRenderObject, CDeviceGraphicsCommandInterface& commandInterface, i32 frameId);
	void                  DrawParticlesLegacy(CRenderObject* pRenderObject, CDeviceGraphicsCommandInterface& commandInterface, i32 frameId);

	TCompiledParticlePtr                 m_pCompiledParticle;
	IParticleVertexCreator*              m_pVertexCreator;
	gpu_pfx2::CParticleComponentRuntime* m_pGpuRuntime;
	SRenderVertices                      m_RenderVerts;
	u32                               m_nFirstVertex;
	u32                               m_nFirstIndex;
	u32                               m_allocId;
	u16                               m_nThreadId;
	u8                                m_addedToView;

	Vec3                                 m_AABBmin, m_AABBmax;
};

#endif  // __CREPARTICLE_H__
