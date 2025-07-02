// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CRoadRenderNode : public IRoadRenderNode, public DinrusX3dEngBase
{
public:
	// SDynamicData and SData are helpers for serialization
	struct SData
	{
		float arrTexCoors[2];
		float arrTexCoorsGlobal[2];

		AABB  worldSpaceBBox;

		// Don't use vtx_idx, we need to keep levels exported on PC loadable on lesser platforms
		u32 numVertices;
		u32 numIndices;
		u32 numTangents;

		u32 physicsGeometryCount;

		// Number of vertices that can be used to fully rebuild the road data
		u32 sourceVertexCount;

		AUTO_STRUCT_INFO;
	};

	struct SPhysicsGeometryParams
	{
		Vec3 size;
		Vec3 pos;
		Quat q;

		AUTO_STRUCT_INFO;
	};

	struct SDynamicData
	{
		PodArray<SVF_P3F_C4B_T2S>        vertices;
		PodArray<vtx_idx>                indices;
		PodArray<SPipTangents>           tangents;

		PodArray<SPhysicsGeometryParams> physicsGeometry;
	};

public:
	CRoadRenderNode();
	virtual ~CRoadRenderNode();

	// IRoadRenderNode implementation
	virtual void SetVertices(const Vec3* pVerts, i32 nVertsNum, float fTexCoordBegin, float fTexCoordEnd, float fTexCoordBeginGlobal, float fTexCoordEndGlobal);
	virtual void SetSortPriority(u8 sortPrio);
	virtual void SetIgnoreTerrainHoles(bool bVal);
	virtual void SetPhysicalize(bool bVal) { if (m_bPhysicalize != bVal) { m_bPhysicalize = bVal; ScheduleRebuild(true); } }

	// IRenderNode implementation
	virtual tukk         GetEntityClassName(void) const { return "RoadObjectClass"; }
	virtual tukk         GetName(void) const            { return "RoadObjectName"; }
	virtual Vec3                GetPos(bool) const;
	virtual void                Render(const SRendParams& RendParams, const SRenderingPassInfo& passInfo);

	virtual IPhysicalEntity*    GetPhysics(void) const             { return m_pPhysEnt; }
	virtual void                SetPhysics(IPhysicalEntity* pPhys) { m_pPhysEnt = pPhys; }

	virtual void                SetMaterial(IMaterial* pMat);
	virtual IMaterial*          GetMaterial(Vec3* pHitPos = NULL) const;
	virtual IMaterial*          GetMaterialOverride() { return m_pMaterial; }
	virtual float               GetMaxViewDist();
	virtual EERType             GetRenderNodeType();
	virtual struct IRenderMesh* GetRenderMesh(i32 nLod)     { return nLod == 0 ? m_pRenderMesh.get() : NULL; }
	virtual void                GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual const AABB          GetBBox() const             { return m_serializedData.worldSpaceBBox; }
	virtual void                SetBBox(const AABB& WSBBox) { m_serializedData.worldSpaceBBox = WSBBox; }
	virtual void                FillBBox(AABB& aabb);
	virtual void                OffsetPosition(const Vec3& delta);
	virtual void                GetClipPlanes(Plane* pPlanes, i32 nPlanesNum, i32 nVertId);
	virtual void                GetTexCoordInfo(float* pTexCoordInfo);
	virtual u8               GetSortPriority() { return m_sortPrio; }
	virtual bool                CanExecuteRenderAsJob() final;

	virtual void                SetLayerId(u16 nLayerId) { m_nLayerId = nLayerId; Get3DEngine()->C3DEngine::UpdateObjectsLayerAABB(this); }
	virtual u16              GetLayerId()                { return m_nLayerId; }

	static bool                 ClipTriangle(PodArray<Vec3>& lstVerts, PodArray<vtx_idx>& lstInds, i32 nStartIdxId, Plane* pPlanes);
	using IRenderNode::Physicalize;
	virtual void                Dephysicalize(bool bKeepIfReferenced = false);
	void                        Compile();
	void                        ScheduleRebuild(bool bFullRebuild);
	void                        OnTerrainChanged();

	_smart_ptr<IRenderMesh> m_pRenderMesh;
	_smart_ptr<IMaterial>   m_pMaterial;
	PodArray<Vec3>          m_arrVerts;

	SData                   m_serializedData;
	SDynamicData            m_dynamicData;

	// Set when scheduling rebuild, true if the road was changed and we need to recompile it
	bool             m_bRebuildFull;

	u8            m_sortPrio;
	bool             m_bIgnoreTerrainHoles;
	bool             m_bPhysicalize;

	IPhysicalEntity* m_pPhysEnt;

	u16           m_nLayerId;

	// Temp buffers used during road mesh creation
	static PodArray<Vec3>            s_tempVertexPositions;
	static PodArray<vtx_idx>         s_tempIndices;

	static PodArray<SPipTangents>    s_tempTangents;
	static PodArray<SVF_P3F_C4B_T2S> s_tempVertices;

	static CPolygonClipContext       s_tmpClipContext;

	static void GetMemoryUsageStatic(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "RoadRenderNodeStaticData");

		pSizer->AddObject(s_tempVertexPositions);
		pSizer->AddObject(s_tempIndices);

		pSizer->AddObject(s_tempTangents);
		pSizer->AddObject(s_tempVertices);

		pSizer->AddObject(s_tmpClipContext);
	}

	static void FreeStaticMemoryUsage()
	{
		s_tempVertexPositions.Reset();
		s_tempIndices.Reset();

		s_tempTangents.Reset();
		s_tempVertices.Reset();

		s_tmpClipContext.Reset();
	}
};
