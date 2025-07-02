// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   terrain_water.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _TERRAIN_WATER_H_
#define _TERRAIN_WATER_H_

#define CYCLE_BUFFERS_NUM 4

class COcean : public IRenderNode, public DinrusX3dEngBase
{
public:
	COcean(IMaterial* pMat);
	~COcean();

	void          Create(const SRenderingPassInfo& passInfo);
	void          Update(const SRenderingPassInfo& passInfo);
	void          Render(const SRenderingPassInfo& passInfo);

	bool          IsVisible(const SRenderingPassInfo& passInfo);

	void          SetLastFov(float fLastFov) { m_fLastFov = fLastFov; }
	static void   SetTimer(ITimer* pTimer);

	static float  GetWave(const Vec3& pPosition, i32 nFrameID);
	static u32 GetVisiblePixelsCount();
	i32         GetMemoryUsage();

	// fake IRenderNode implementation
	virtual EERType          GetRenderNodeType();
	virtual tukk      GetEntityClassName(void) const                                 { return "Ocean"; }
	virtual tukk      GetName(void) const                                            { return "Ocean"; }
	virtual Vec3             GetPos(bool) const;
	virtual void             Render(const SRendParams&, const SRenderingPassInfo& passInfo) {}
	virtual IPhysicalEntity* GetPhysics(void) const                                         { return 0; }
	virtual void             SetPhysics(IPhysicalEntity*)                                   {}
	virtual void             SetMaterial(IMaterial* pMat)                                   { m_pMaterial = pMat; }
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = NULL) const;
	virtual IMaterial*       GetMaterialOverride()                                          { return m_pMaterial; }
	virtual float            GetMaxViewDist()                                               { return 1000000.f; }
	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const                        {}
	virtual const AABB       GetBBox() const                                                { return AABB(Vec3(-1000000.f, -1000000.f, -1000000.f), Vec3(1000000.f, 1000000.f, 1000000.f)); }
	virtual void             SetBBox(const AABB& WSBBox)                                    {}
	virtual void             FillBBox(AABB& aabb);
	virtual void             OffsetPosition(const Vec3& delta);

private:

	void RenderFog(const SRenderingPassInfo& passInfo);
	void RenderBottomCap(const SRenderingPassInfo& passInfo);

private:

	// Ocean data
	IMaterial*                m_pMaterial;

	i32                     m_nPrevGridDim;
	u32                    m_nVertsCount;
	u32                    m_nIndicesCount;

	i32                     m_nTessellationType;
	i32                     m_nTessellationTiles;

	// Ocean bottom cap data
	_smart_ptr<IMaterial>     m_pBottomCapMaterial;
	_smart_ptr<IRenderMesh>   m_pBottomCapRenderMesh;

	PodArray<SVF_P3F_C4B_T2F> m_pBottomCapVerts;
	PodArray<vtx_idx>         m_pBottomCapIndices;

	// Visibility data
	CCamera                  m_Camera;
	class CREOcclusionQuery* m_pREOcclusionQueries[CYCLE_BUFFERS_NUM];
	IShader*                 m_pShaderOcclusionQuery;
	float                    m_fLastFov;
	float                    m_fLastVisibleFrameTime;
	i32                    m_nLastVisibleFrameId;
	static u32            m_nVisiblePixelsCount;

	float                    m_fRECustomData[12];           // used for passing data to renderer
	float                    m_fREOceanBottomCustomData[8]; // used for passing data to renderer
	bool                     m_bOceanFFT;

	// Ocean fog related members
	CREWaterVolume::SParams      m_wvParams[RT_COMMAND_BUF_COUNT];
	CREWaterVolume::SOceanParams m_wvoParams[RT_COMMAND_BUF_COUNT];

	_smart_ptr<IMaterial>        m_pFogIntoMat;
	_smart_ptr<IMaterial>        m_pFogOutofMat;
	_smart_ptr<IMaterial>        m_pFogIntoMatLowSpec;
	_smart_ptr<IMaterial>        m_pFogOutofMatLowSpec;

	CREWaterVolume*              m_pWVRE[RT_COMMAND_BUF_COUNT];
	std::vector<SVF_P3F_C4B_T2F> m_wvVertices[RT_COMMAND_BUF_COUNT];
	std::vector<u16>          m_wvIndices[RT_COMMAND_BUF_COUNT];

	static ITimer*               m_pOceanTimer;
	static CREWaterOcean*        m_pOceanRE;

};

#endif
