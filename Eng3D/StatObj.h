// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   statobj.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef STAT_OBJ_H
#define STAT_OBJ_H

#if DRX_PLATFORM_DESKTOP
	#define TRACE_CGF_LEAKS
#endif

class CIndexedMesh;
class CRenderObject;
class CContentCGF;
struct CNodeCGF;
struct CMaterialCGF;
struct phys_geometry;
struct IIndexedMesh;
struct IParticleEffect;

#include <drx3D/Eng3D/DinrusX3dEngBase.h>
#include <drx3D/CoreX/Containers/DrxArray.h>

#include <drx3D/Eng3D/IStatObj.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Eng3D/RenderMeshUtils.h>
#include <drx3D/CoreX/Math/GeomQuery.h>

#define MAX_PHYS_GEOMS_TYPES 4

struct SDeformableMeshData
{
	IGeometry*    pInternalGeom;
	i32*          pVtxMap;
	u32* pUsedVtx;
	i32*          pVtxTri;
	i32*          pVtxTriBuf;
	float*        prVtxValency;
	Vec3*         pPrevVtx;
	float         kViscosity;
};

struct SSpine
{
	~SSpine() { delete[] pVtx; delete[] pVtxCur; delete[] pSegDim; }
	SSpine() { pVtx = 0; pVtxCur = 0; pSegDim = 0; bActive = false; nVtx = 0; len = 0; navg = Vec3(0, 0, 0); idmat = 0; iAttachSpine = 0; iAttachSeg = 0; }

	bool  bActive;
	Vec3* pVtx;
	Vec3* pVtxCur;
	Vec4* pSegDim;
	i32   nVtx;
	float len;
	Vec3  navg;
	i32   idmat;
	i32   iAttachSpine;
	i32   iAttachSeg;
};

class CStatObjFoliage : public IFoliage, public DinrusX3dEngBase
{
public:
	CStatObjFoliage()
	{
		m_next = 0;
		m_prev = 0;
		m_lifeTime = 0;
		m_ppThis = 0;
		m_pStatObj = 0;
		m_pRopes = 0;
		m_pRopesActiveTime = 0;
		m_nRopes = 0;
		m_nRefCount = 1;
		m_timeIdle = 0;
		m_pVegInst = 0;
		m_pTrunk = 0;
		m_pSkinningTransformations[0] = 0;
		m_pSkinningTransformations[1] = 0;
		m_iActivationSource = 0;
		m_flags = 0;
		m_bGeomRemoved = 0;
		m_bEnabled = 1;
		m_timeInvisible = 0;
		m_bDelete = 0;
		m_pRenderObject = 0;
		m_minEnergy = 0.0f;
		m_stiffness = 0.0f;
		arrSkinningRendererData[0].pSkinningData = NULL;
		arrSkinningRendererData[0].nFrameID = 0;
		arrSkinningRendererData[1].pSkinningData = NULL;
		arrSkinningRendererData[1].nFrameID = 0;
		arrSkinningRendererData[2].pSkinningData = NULL;
		arrSkinningRendererData[2].nFrameID = 0;
	}
	~CStatObjFoliage();
	virtual void             AddRef()  { m_nRefCount++; }
	virtual void             Release() { if (--m_nRefCount <= 0) m_bDelete = 2; }

	virtual i32              Serialize(TSerialize ser);
	virtual void             SetFlags(i32 flags);
	virtual i32              GetFlags()                    { return m_flags; }
	virtual IRenderNode*     GetIRenderNode()              { return m_pVegInst; }
	virtual i32              GetBranchCount()              { return m_nRopes; }
	virtual IPhysicalEntity* GetBranchPhysics(i32 iBranch) { return (u32)iBranch < (u32)m_nRopes ? m_pRopes[iBranch] : 0; }

	virtual SSkinningData*   GetSkinningData(const Matrix34& RenderMat34, const SRenderingPassInfo& passInfo);

	u32                   ComputeSkinningTransformationsCount();
	void                     ComputeSkinningTransformations(u32 nList);

	void                     OnHit(struct EventPhysCollision* pHit);
	void                     Update(float dt, const CCamera& rCamera);
	void                     BreakBranch(i32 idx);

	CStatObjFoliage*  m_next, * m_prev;
	i32               m_nRefCount;
	i32               m_flags;
	CStatObj*         m_pStatObj;
	IPhysicalEntity** m_pRopes;
	float*            m_pRopesActiveTime;
	IPhysicalEntity*  m_pTrunk;
	i16             m_nRopes;
	i16             m_bEnabled;
	float             m_timeIdle, m_lifeTime;
	IFoliage**        m_ppThis;
	QuatTS*           m_pSkinningTransformations[2];
	i32               m_iActivationSource;
	i32               m_bGeomRemoved;
	IRenderNode*      m_pVegInst;
	CRenderObject*    m_pRenderObject;
	float             m_timeInvisible;
	float             m_minEnergy;
	float             m_stiffness;
	i32               m_bDelete;
	// history for skinning data, needed for motion blur
	struct { SSkinningData* pSkinningData; i32 nFrameID; } arrSkinningRendererData[3]; // tripple buffered for motion blur
};

struct SClothTangentVtx
{
	i32  ivtxT;   // for each vertex, specifies the iThisVtx->ivtxT edge, which is the closest to the vertex's tangent vector
	Vec3 edge;    // that edge's projection on the vertex's normal basis
	i32  sgnNorm; // sign of phys normal * normal from the basis
};

struct SSkinVtx
{
	i32      bVolumetric;
	i32      idx[4];
	float    w[4];
	Matrix33 M;
};

struct SDelayedSkinParams
{
	Matrix34   mtxSkelToMesh;
	IGeometry* pPhysSkel;
	float      r;
};

struct SPhysGeomThunk
{
	phys_geometry* pgeom;
	i32            type;
	void           GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//		pSizer->AddObject(pgeom);
	}
};

struct SPhysGeomArray
{
	phys_geometry* operator[](i32 idx) const
	{
		if (idx < PHYS_GEOM_TYPE_DEFAULT)
			return idx < (i32)m_array.size() ? m_array[idx].pgeom : 0;
		else
		{
			i32 i;
			for (i = m_array.size() - 1; i >= 0 && m_array[i].type != idx; i--);
			return i >= 0 ? m_array[i].pgeom : 0;
		}
	}
	void SetPhysGeom(phys_geometry* pgeom, i32 idx = PHYS_GEOM_TYPE_DEFAULT, i32 type = PHYS_GEOM_TYPE_DEFAULT)
	{
		i32 i;
		if (idx < PHYS_GEOM_TYPE_DEFAULT)
			i = idx, idx = type;
		else
			for (i = 0; i < (i32)m_array.size() && m_array[i].type != idx; i++);
		if (pgeom)
		{
			if (i >= (i32)m_array.size())
				m_array.resize(i + 1);
			m_array[i].pgeom = pgeom;
			m_array[i].type = idx;
		}
		else if (i < (i32)m_array.size())
			m_array.erase(m_array.begin() + i);
	}
	i32  GetGeomCount() const { return m_array.size(); }
	i32  GetGeomType(i32 idx) { return idx >= PHYS_GEOM_TYPE_DEFAULT ? idx : m_array[idx].type; }
	std::vector<SPhysGeomThunk> m_array;
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_array);
	}
};

struct SSyncToRenderMeshContext
{
	Vec3*                         vmin, * vmax;
	i32                           iVtx0;
	i32                           nVtx;
	strided_pointer<Vec3>         pVtx;
	i32*                          pVtxMap;
	i32                           mask;
	float                         rscale;
	SClothTangentVtx*             ctd;
	strided_pointer<Vec3>         pMeshVtx;
	strided_pointer<SPipTangents> pTangents;
	strided_pointer<Vec3>         pNormals; // TODO: change Vec3 to SPipNormal
	CStatObj*                     pObj;
	JobUpr::SJobState         jobState;

	void                          Set(Vec3* _vmin, Vec3* _vmax, i32 _iVtx0, i32 _nVtx, strided_pointer<Vec3> _pVtx, i32* _pVtxMap
	                                  , i32 _mask, float _rscale, SClothTangentVtx* _ctd, strided_pointer<Vec3> _pMeshVtx
	                                  , strided_pointer<SPipTangents> _pTangents, strided_pointer<Vec3> _pNormals, CStatObj* _pObj)
	{
		vmin = _vmin;
		vmax = _vmax;
		iVtx0 = _iVtx0;
		nVtx = _nVtx;
		pVtx = _pVtx;
		pVtxMap = _pVtxMap;
		mask = _mask;
		rscale = _rscale;
		ctd = _ctd;
		pMeshVtx = _pMeshVtx;
		pTangents = _pTangents;
		pNormals = _pNormals;
		pObj = _pObj;
	}
};

struct DRX_ALIGN(8) CStatObj: public IStatObj, public IStreamCallback, public stl::intrusive_linked_list_node<CStatObj>, public DinrusX3dEngBase
{
	CStatObj();
	~CStatObj();

public:
	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	 i32 m_nUsers; // reference counter

	u32 m_nLastDrawMainFrameId;

	_smart_ptr<IRenderMesh> m_pRenderMesh;

	DrxCriticalSection m_streamingMeshLock;
	_smart_ptr<IRenderMesh> m_pStreamedRenderMesh;
	_smart_ptr<IRenderMesh> m_pMergedRenderMesh;

	// Used by hierarchical breaking to hide sub-objects that initially must be hidden.
	hidemask m_nInitialSubObjHideMask;

	CIndexedMesh* m_pIndexedMesh;
	 i32 m_lockIdxMesh;

	string m_szFileName;
	string m_szGeomName;
	string m_szProperties;
	string m_szStreamingDependencyFilePath;

	i32 m_nLoadedTrisCount;
	i32 m_nLoadedVertexCount;
	i32 m_nRenderTrisCount;
	i32 m_nRenderMatIds;
	float m_fGeometricMeanFaceArea;
	float m_fLodDistance;
	Vec3 m_depthSortOffset;

	// Default material.
	_smart_ptr<IMaterial> m_pMaterial;

	// Billboard material and mesh
	_smart_ptr<IMaterial> m_pBillboardMaterial;

	float m_fRadiusHors;
	float m_fRadiusVert;

	AABB m_AABB;
	Vec3 m_vVegCenter;

	SPhysGeomArray m_arrPhysGeomInfo;
	ITetrLattice* m_pLattice;
	IStatObj* m_pLastBooleanOp;
	float m_lastBooleanOpScale;

	_smart_ptr<CStatObj>* m_pLODs;
	CStatObj* m_pLod0;                 // Level 0 stat object. (Pointer to the original object of the LOD)
	u32 m_nMinUsableLod0 : 8; // What is the minimal LOD that can be used as LOD0.
	u32 m_nMaxUsableLod0 : 8; // What is the maximal LOD that can be used as LOD0.
	u32 m_nMaxUsableLod  : 8; // What is the maximal LOD that can be used.
	u32 m_nLoadedLodsNum : 8; // How many lods loaded.

	string m_cgfNodeName;

	//////////////////////////////////////////////////////////////////////////
	// Externally set flags from enum EStaticObjectFlags.
	//////////////////////////////////////////////////////////////////////////
	i32 m_nFlags;

	//////////////////////////////////////////////////////////////////////////
	// Internal Flags.
	//////////////////////////////////////////////////////////////////////////
	u32 m_bCheckGarbage : 1;
	u32 m_bCanUnload : 1;
	u32 m_bLodsLoaded : 1;
	u32 m_bDefaultObject : 1;
	u32 m_bOpenEdgesTested : 1;
	u32 m_bSubObject : 1;          // This is sub object.
	u32 m_bVehicleOnlyPhysics : 1; // Object can be used for collisions with vehicles only
	u32 m_bBreakableByGame : 1;    // material is marked as breakable by game
	u32 m_bSharesChildren : 1;     // means its subobjects belong to another parent statobj
	u32 m_bHasDeformationMorphs : 1;
	u32 m_bTmpIndexedMesh : 1; // indexed mesh is temporary and can be deleted after MakeRenderMesh
	u32 m_bUnmergable : 1;     // Set if sub objects cannot be merged together to the single render merge.
	u32 m_bMerged : 1;         // Set if sub objects merged together to the single render merge.
	u32 m_bMergedLODs : 1;     // Set if m_pLODs were created while merging LODs
	u32 m_bLowSpecLod0Set : 1;
	u32 m_bHaveOcclusionProxy : 1; // If this stat object or its childs have occlusion proxy.
	u32 m_bLodsAreLoadedFromSeparateFile : 1;
	u32 m_bNoHitRefinement : 1;       // doesn't refine bullet hits against rendermesh
	u32 m_bDontOccludeExplosions : 1; // don't act as an explosion occluder in physics
	u32 m_hasClothTangentsData : 1;
	u32 m_hasSkinInfo : 1;
	u32 m_bMeshStrippedCGF : 1; // This CGF was loaded from the Mesh Stripped CGF, (in Level Cache)
	u32 m_isDeformable : 1;     // This cgf is deformable in the sense that it has a special renderpath
	u32 m_isProxyTooBig : 1;
	u32 m_bHasStreamOnlyCGF : 1;

	i32 m_idmatBreakable; // breakable id for the physics
	//////////////////////////////////////////////////////////////////////////

	// streaming
	i32 m_nRenderMeshMemoryUsage;
	i32 m_nMergedMemoryUsage;
	i32 m_arrRenderMeshesPotentialMemoryUsage[2];
	IReadStreamPtr m_pReadStream;
	u32 m_nModificationId; // used to detect the cases when dependent permanent render objects have to be updated

#if !defined (_RELEASE) || defined(ENABLE_STATOSCOPE_RELEASE)
	static float s_fStreamingTime;
	static i32 s_nBandwidth;
	float m_fStreamingStart;
#endif

#ifdef OBJMAN_STREAM_STATS
	i32 m_nStatoscopeState;
#endif

	//////////////////////////////////////////////////////////////////////////

	u16* m_pMapFaceToFace0;
	union
	{
		SClothTangentVtx* m_pClothTangentsData;
		SSkinVtx*         m_pSkinInfo;
	};
	SDelayedSkinParams* m_pDelayedSkinParams;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Bendable Foliage.
	//////////////////////////////////////////////////////////////////////////
	SSpine* m_pSpines;
	i32 m_nSpines;
	struct SMeshBoneMapping_uint8* m_pBoneMapping;
	std::vector<u16> m_chunkBoneIds;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// for debug purposes
	//////////////////////////////////////////////////////////////////////////
#ifdef TRACE_CGF_LEAKS
	string m_sLoadingCallstack;
#endif

private:

	// Returns a list of all CStatObj instances contained within this object (all sub-objects plus the parent object itself).
	std::vector<CStatObj*> GatherAllObjects();

	//////////////////////////////////////////////////////////////////////////
	// Sub objects.
	//////////////////////////////////////////////////////////////////////////
	std::vector<SSubObject> m_subObjects;
	CStatObj* m_pParentObject;       // Parent object (Must not be smart pointer).
	CStatObj* m_pClonedSourceObject; // If this is cloned object, pointer to original source object (Must not be smart pointer).
	i32 m_nSubObjectMeshCount;
	i32 m_nNodeCount;

	CGeomExtents m_Extents;           // Cached extents for random pos generation.

	//////////////////////////////////////////////////////////////////////////
	// Special AI/Physics parameters.
	//////////////////////////////////////////////////////////////////////////
	float m_aiVegetationRadius;
	float m_phys_mass;
	float m_phys_density;

	//////////////////////////////////////////////////////////////////////////
	// used only in the editor
	//////////////////////////////////////////////////////////////////////////

	SSyncToRenderMeshContext* m_pAsyncUpdateContext;

	//////////////////////////////////////////////////////////////////////////
	// METHODS.
	//////////////////////////////////////////////////////////////////////////
public:
	//////////////////////////////////////////////////////////////////////////
	// Fast non virtual access functions.
	ILINE IStatObj::SSubObject& SubObject(i32 nIndex)  { return m_subObjects[nIndex]; };
	ILINE i32                   SubObjectCount() const { return m_subObjects.size(); };
	//////////////////////////////////////////////////////////////////////////

	virtual bool IsUnloadable() const final { return m_bCanUnload; }
	void DisableStreaming();

	virtual IIndexedMesh*    GetIndexedMesh(bool bCreatefNone = false) final;
	virtual IIndexedMesh*    CreateIndexedMesh() final;
	void ReleaseIndexedMesh(bool bRenderMeshUpdated = false);
	virtual ILINE const Vec3 GetVegCenter() final          { return m_vVegCenter; }

	virtual void             SetFlags(i32 nFlags) final		{ m_nFlags = nFlags; IncrementModificationId(); }
	virtual i32              GetFlags() const final         { return m_nFlags; };

	virtual u32     GetVehicleOnlyPhysics() final { return m_bVehicleOnlyPhysics; };
	virtual i32              GetIDMatBreakable() final     { return m_idmatBreakable; };
	virtual u32     GetBreakableByGame() final    { return m_bBreakableByGame; };

	virtual bool IsDeformable() final;

	// Loader
	bool LoadCGF(tukk filename, bool bLod, u64 nLoadingFlags, ukk pData, i32k nDataSize);
	bool LoadCGF_Int(tukk filename, bool bLod, u64 nLoadingFlags, ukk pData, i32k nDataSize);

	//////////////////////////////////////////////////////////////////////////
	virtual void SetMaterial(IMaterial * pMaterial) final;
	virtual IMaterial* GetMaterial() const final { return m_pMaterial; }
	//////////////////////////////////////////////////////////////////////////

	IMaterial * GetBillboardMaterial() { return m_pBillboardMaterial; }

	void RenderInternal(CRenderObject * pRenderObject, hidemask nSubObjectHideMask, const CLodValue &lodValue, const SRenderingPassInfo &passInfo);
	void RenderObjectInternal(CRenderObject * pRenderObject, i32 nLod, u8 uLodDissolveRef, bool dissolveOut, const SRenderingPassInfo &passInfo);
	void RenderSubObject(CRenderObject * pRenderObject, i32 nLod,
	                     i32 nSubObjId, const Matrix34A &renderTM, const SRenderingPassInfo &passInfo);
	void RenderSubObjectInternal(CRenderObject * pRenderObject, i32 nLod, const SRenderingPassInfo &passInfo);
	virtual void Render(const SRendParams &rParams, const SRenderingPassInfo &passInfo) final;
	void RenderRenderMesh(CRenderObject * pObj, struct SInstancingInfo* pInstInfo, const SRenderingPassInfo &passInfo);
	virtual phys_geometry* GetPhysGeom(i32 nGeomType = PHYS_GEOM_TYPE_DEFAULT) const final { return m_arrPhysGeomInfo[nGeomType]; }
	virtual void           SetPhysGeom(phys_geometry* pPhysGeom, i32 nGeomType = PHYS_GEOM_TYPE_DEFAULT) final
	{
		if (m_arrPhysGeomInfo[nGeomType])
			GetPhysicalWorld()->GetGeomUpr()->UnregisterGeometry(m_arrPhysGeomInfo[nGeomType]);
		m_arrPhysGeomInfo.SetPhysGeom(pPhysGeom, nGeomType);
	}
	virtual IPhysicalEntity* GetPhysEntity() const final               { return NULL; }
	virtual ITetrLattice*    GetTetrLattice() final                    { return m_pLattice; }

	virtual float            GetAIVegetationRadius() const final       { return m_aiVegetationRadius; }
	virtual void             SetAIVegetationRadius(float radius) final { m_aiVegetationRadius = radius; }

	//! Refresh object ( reload shaders or/and object geometry )
	virtual void Refresh(i32 nFlags) final;

	virtual IRenderMesh* GetRenderMesh() const final { return m_pRenderMesh; };
	void SetRenderMesh(IRenderMesh * pRM);

	virtual tukk GetFilePath() final                       { return (m_szFileName); }
	virtual void        SetFilePath(tukk szFileName) final { m_szFileName = szFileName; }
	virtual tukk GetGeoName() final                        { return (m_szGeomName); }
	virtual void        SetGeoName(tukk szGeoName) final   { m_szGeomName = szGeoName; }
	virtual bool IsSameObject(tukk szFileName, tukk szGeomName) final;

	//set object's min/max bbox
	virtual void SetBBoxMin(const Vec3& vBBoxMin) final { m_AABB.min = vBBoxMin; }
	virtual void SetBBoxMax(const Vec3& vBBoxMax) final { m_AABB.max = vBBoxMax; }
	virtual AABB GetAABB() const final                  { return m_AABB; }

	virtual float GetExtent(EGeomForm eForm) final;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const final;

	virtual Vec3 GetHelperPos(tukk szHelperName) final;
	virtual const Matrix34& GetHelperTM(tukk szHelperName) final;

	virtual float           GetRadiusVert() const final { return m_fRadiusVert; }
	virtual float           GetRadiusHors() const final { return m_fRadiusHors; }

	virtual i32 AddRef() final;
	virtual i32 Release() final;
	virtual i32  GetRefCount() const final { return m_nUsers; }

	virtual bool IsDefaultObject() final   { return (m_bDefaultObject); }

	i32          GetLoadedTrisCount()         { return m_nLoadedTrisCount; }
	i32          GetRenderTrisCount()         { return m_nRenderTrisCount; }

	// Load LODs
	virtual void SetLodObject(i32 nLod, IStatObj * pLod) final;
	bool LoadLowLODS_Prep(bool bUseStreaming, u64 nLoadingFlags);
	CStatObj* LoadLowLODS_Load(i32 nLodLevel, bool bUseStreaming, u64 nLoadingFlags, ukk pData, i32 nDataLen);
	void LoadLowLODS_Finalize(i32 nLoadedLods, CStatObj * loadedLods[MAX_STATOBJ_LODS_NUM]);
	void LoadLowLODs(bool bUseStreaming, u64 nLoadingFlags);
	// Free render resources for unused upper LODs.
	void CleanUnusedLods();

	virtual void FreeIndexedMesh() final;
	bool RenderDebugInfo(CRenderObject * pObj, const SRenderingPassInfo &passInfo);

	//! Release method.
	virtual void GetMemoryUsage(class IDrxSizer* pSizer) const final;

	void ShutDown();
	void Init();

	//  void CheckLoaded();
	virtual IStatObj* GetLodObject(i32 nLodLevel, bool bReturnNearest = false) final;
	virtual IStatObj* GetLowestLod() final;

	virtual i32 FindNearestLoadedLOD(i32 nLodIn, bool bSearchUp = false) final;
	virtual i32 FindHighestLOD(i32 nBias) final;

	// interface IStreamCallback -----------------------------------------------------

	virtual void StreamAsyncOnComplete(IReadStream * pStream, unsigned nError) final;
	virtual void StreamOnComplete(IReadStream * pStream, unsigned nError) final;
	void IncrementModificationId();
	u32 GetModificationId() { return m_nModificationId; }

	// -------------------------------------------------------------------------------

	virtual void StartStreaming(bool bFinishNow, IReadStream_AutoPtr * ppStream) final;
	void UpdateStreamingPrioriryInternal(const Matrix34A &objMatrix, float fDistance, bool bFullUpdate);

	void MakeCompiledFileName(tuk szCompiledFileName, i32 nMaxLen);

	virtual bool IsPhysicsExist() const final;
	bool IsSphereOverlap(const Sphere &sSphere);
	virtual void Invalidate(bool bPhysics = false, float tolerance = 0.05f) final;

	void AnalyzeFoliage(IRenderMesh * pRenderMesh, CContentCGF * pCGF);
	void FreeFoliageData();
	virtual void CopyFoliageData(IStatObj * pObjDst, bool bMove = false, IFoliage * pSrcFoliage = 0, i32* pVtxMap = 0, primitives::box * pMoveBoxes = 0, i32 nMovedBoxes = -1) final;
	virtual i32 PhysicalizeFoliage(IPhysicalEntity * pTrunk, const Matrix34 &mtxWorld, IFoliage * &pRes, float lifeTime = 0.0f, i32 iSource = 0) final;
	i32 SerializeFoliage(TSerialize ser, IFoliage * pFoliage);

	virtual IStatObj* UpdateVertices(strided_pointer<Vec3> pVtx, strided_pointer<Vec3> pNormals, i32 iVtx0, i32 nVtx, i32* pVtxMap = 0, float rscale = 1.f) final;
	bool              HasSkinInfo(float skinRadius = -1.0f)                                                { return m_hasSkinInfo && m_pSkinInfo && (skinRadius < 0.0f || m_pSkinInfo[m_nLoadedVertexCount].w[0] == skinRadius); }
	void PrepareSkinData(const Matrix34 &mtxSkelToMesh, IGeometry * pPhysSkel, float r = 0.0f);
	virtual IStatObj* SkinVertices(strided_pointer<Vec3> pSkelVtx, const Matrix34& mtxSkelToMesh) final { return SkinVertices(pSkelVtx, mtxSkelToMesh, NULL);  }
	IStatObj*         SkinVertices(strided_pointer<Vec3> pSkelVtx, const Matrix34& mtxSkelToMesh,  i32* ready);

	//////////////////////////////////////////////////////////////////////////
	// Sub objects.
	//////////////////////////////////////////////////////////////////////////
	virtual i32                   GetSubObjectCount() const final { return m_subObjects.size(); }
	virtual void SetSubObjectCount(i32 nCount) final;
	virtual IStatObj::SSubObject* FindSubObject(tukk sNodeName) final;
	virtual IStatObj::SSubObject* FindSubObject_StrStr(tukk sNodeName) final;
	virtual IStatObj::SSubObject* FindSubObject_CGA(tukk sNodeName) final;
	virtual IStatObj::SSubObject* GetSubObject(i32 nIndex) final
	{
		if (nIndex >= 0 && nIndex < (i32)m_subObjects.size())
			return &m_subObjects[nIndex];
		else
			return 0;
	}
	virtual bool RemoveSubObject(i32 nIndex) final;
	virtual IStatObj* GetParentObject() const final      { return m_pParentObject; }
	virtual IStatObj* GetCloneSourceObject() const final { return m_pClonedSourceObject; }
	virtual bool      IsSubObject() const final          { return m_bSubObject; };
	virtual bool CopySubObject(i32 nToIndex, IStatObj * pFromObj, i32 nFromIndex) final;
	virtual i32 PhysicalizeSubobjects(IPhysicalEntity * pent, const Matrix34 * pMtx, float mass, float density = 0.0f, i32 id0 = 0,
	                                  strided_pointer<i32> pJointsIdMap = 0, tukk szPropsOverride = 0, i32 idbodyArtic = -1) final;
	virtual IStatObj::SSubObject& AddSubObject(IStatObj* pStatObj) final;
	virtual i32 Physicalize(IPhysicalEntity * pent, pe_geomparams * pgp, i32 id = -1, tukk szPropsOverride = 0) final;
	//////////////////////////////////////////////////////////////////////////

	virtual bool SaveToCGF(tukk sFilename, IChunkFile * *pOutChunkFile = NULL, bool bHavePhiscalProxy = false) final;

	//virtual IStatObj* Clone(bool bCloneChildren=true, bool nDynamic=false);
	virtual IStatObj* Clone(bool bCloneGeometry, bool bCloneChildren, bool bMeshesOnly) final;

	virtual i32 SetDeformationMorphTarget(IStatObj * pDeformed) final;
	virtual i32 SubobjHasDeformMorph(i32 iSubObj);
	virtual IStatObj* DeformMorph(const Vec3& pt, float r, float strength, IRenderMesh* pWeights = 0) final;

	virtual IStatObj* HideFoliage() final;

	virtual i32 Serialize(TSerialize ser) final;

	// Get object properties as loaded from CGF.
	virtual tukk GetProperties() final                  { return m_szProperties.c_str(); };
	virtual void        SetProperties(tukk props) final { m_szProperties = props; ParseProperties(); }

	virtual bool GetPhysicalProperties(float& mass, float& density) final;

	virtual IStatObj* GetLastBooleanOp(float& scale) final { scale = m_lastBooleanOpScale; return m_pLastBooleanOp; }

	// Intersect ray with static object.
	// Ray must be in object local space.
	virtual bool RayIntersection(SRayHitInfo & hitInfo, IMaterial * pCustomMtl = 0) final;
	virtual bool LineSegIntersection(const Lineseg &lineSeg, Vec3 & hitPos, i32& surfaceTypeId) final;

	virtual void DebugDraw(const SGeometryDebugDrawInfo &info) final;
	virtual void GetStatistics(SStatistics & stats) final;
	//////////////////////////////////////////////////////////////////////////

	IParticleEffect* GetSurfaceBreakageEffect(tukk sType);

	virtual hidemask GetInitialHideMask() final { return m_nInitialSubObjHideMask; }

	virtual void     SetStreamingDependencyFilePath(tukk szFileName) final
	{
		const bool streamingDependencyLoop = CheckForStreamingDependencyLoop(szFileName);
		if (streamingDependencyLoop)
		{
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "StatObj '%s' cannot set '%s' as a streaming dependency as it would result in a looping dependency.", GetFilePath(), szFileName);
			return;
		}

		m_szStreamingDependencyFilePath = szFileName;
	}

	i32 GetMaxUsableLod();
	i32 GetMinUsableLod();
	void RenderStreamingDebugInfo(CRenderObject * pRenderObject, const SRenderingPassInfo& passInfo);
	void RenderCoverInfo(CRenderObject * pRenderObject, const SRenderingPassInfo& passInfo);
	i32 CountChildReferences();
	void ReleaseStreamableContent() final;
	i32 GetStreamableContentMemoryUsage(bool bJustForDebug = false) final;
	virtual SMeshLodInfo ComputeGeometricMean() const final;
	SMeshLodInfo ComputeAndStoreLodDistances();
	virtual float GetLodDistance() const final     { return m_fLodDistance; }
	virtual Vec3  GetDepthSortOffset() const final { return m_depthSortOffset; }
	virtual i32 ComputeLodFromScale(float fScale, float fLodRatioNormalized, float fEntDistance, bool bFoliage, bool bForPrecache) final;
	bool UpdateStreamableComponents(float fImportance, const Matrix34A &objMatrix, bool bFullUpdate, i32 nNewLod);
	void GetStreamableName(string& sName) final
	{
		sName = m_szFileName;
		if (m_szGeomName.length())
		{
			sName += " - ";
			sName += m_szGeomName;
		}
	};
	void GetStreamFilePath(stack_string & strOut);
	void FillRenderObject(const SRendParams &rParams, IRenderNode * pRenderNode, IMaterial * pMaterial,
	                      SInstancingInfo * pInstInfo, CRenderObject * &pObj, const SRenderingPassInfo &passInfo);
	virtual u32 GetLastDrawMainFrameId() final { return m_nLastDrawMainFrameId; }

	// Allow pooled allocs
	static uk operator new(size_t size);
	static void  operator delete(uk pToFree);

	// Used in ObjMan.
	void TryMergeSubObjects(bool bFromStreaming);
	void SavePhysicalizeData(CNodeCGF * pNode);

protected:
	// Called by async stream callback.
	bool LoadStreamRenderMeshes(tukk filename, ukk pData, i32k nDataSize, bool bLod);
	// Called by sync stream complete callback.
	void CommitStreamRenderMeshes();

	void MergeSubObjectsRenderMeshes(bool bFromStreaming, CStatObj * pLod0, i32 nLod);
	void UnMergeSubObjectsRenderMeshes();
	bool CanMergeSubObjects();
	bool IsMatIDReferencedByObj(u16 matID);

	//	bool LoadCGF_Info( tukk filename );
	CStatObj* MakeStatObjFromCgfNode(CContentCGF* pCGF, CNodeCGF* pNode, bool bLod, i32 nLoadingFlags, AABB& commonBBox);
	void ParseProperties();

	void CalcRadiuses();
	void GetStatisticsNonRecursive(SStatistics & stats);

	void PhysicalizeCompiled(CNodeCGF * pNode, i32 bAppend = 0);
	bool PhysicalizeGeomType(i32 nGeomType, CMesh & mesh, float tolerance = 0.05f, i32 bAppend = 0);
	bool RegisterPhysicGeom(i32 nGeomType, phys_geometry * pPhysGeom);
	void AssignPhysGeom(i32 nGeomType, phys_geometry * pPhysGeom, i32 bAppend = 0, i32 bLoading = 0);

	// Creates static object contents from mesh.
	// Return true if successful.
	_smart_ptr<IRenderMesh> MakeRenderMesh(CMesh * pMesh, bool bDoRenderMesh);
	void MakeRenderMesh();

	tukk stristr(tukk szString, tukk szSubstring)
	{
		i32 nSuperstringLength = (i32)strlen(szString);
		i32 nSubstringLength = (i32)strlen(szSubstring);

		for (i32 nSubstringPos = 0; nSubstringPos <= nSuperstringLength - nSubstringLength; ++nSubstringPos)
		{
			if (strnicmp(szString + nSubstringPos, szSubstring, nSubstringLength) == 0)
				return szString + nSubstringPos;
		}
		return NULL;
	}

	bool CheckForStreamingDependencyLoop(tukk szFilenameDependancy) const;
	void CheckCreateBillboardMaterial();
	void CreateBillboardMesh(IMaterial* pMaterial);
};

//////////////////////////////////////////////////////////////////////////
inline void InitializeSubObject(IStatObj::SSubObject& so)
{
	so.localTM.SetIdentity();
	so.name = "";
	so.properties = "";
	so.nType = STATIC_SUB_OBJECT_MESH;
	so.pWeights = 0;
	so.pFoliage = 0;
	so.nParent = -1;
	so.tm.SetIdentity();
	so.bIdentityMatrix = true;
	so.bHidden = false;
	so.helperSize = Vec3(0, 0, 0);
	so.pStatObj = 0;
	so.bShadowProxy = 0;
}

#endif // STAT_OBJ_H
