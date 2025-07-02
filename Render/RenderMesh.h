// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   RenderMesh.h
//  Version:     v1.00
//  Created:     01/07/2009 by Andrey Honich.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __RENDERMESH_H__
#define __RENDERMESH_H__

#include <drx3D/CoreX/Containers/intrusive_list.hpp>
#include <drx3D/CoreX/Memory/Pool/PoolAlloc.h>
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/CoreX/Containers/VectorSet.h>
#include <drx3D/CoreX/Math/GeomQuery.h>

#include <drx3D/Render/ComputeSkinningStorage.h>

// Enable the below to get fatal error is some holds a rendermesh buffer lock for longer than 1 second
//#define RM_CATCH_EXCESSIVE_LOCKS

#define DELETE_SUBSET_MESHES_AFTER_NOTUSED_FRAMES 30

struct SMeshSubSetIndicesJobEntry
{
	JobUpr::SJobState jobState;
	_smart_ptr<IRenderMesh> m_pSrcRM;							// source mesh to create a new index mesh from
	_smart_ptr<IRenderMesh> m_pIndexRM;						// when finished: newly created index mesh for this mask, else NULL
	hidemask m_nMeshSubSetMask;						// mask to use

	void CreateSubSetRenderMesh();

	SMeshSubSetIndicesJobEntry()
		: jobState()
		, m_pSrcRM()
		, m_pIndexRM()
		, m_nMeshSubSetMask(0)
	{}

	SMeshSubSetIndicesJobEntry(SMeshSubSetIndicesJobEntry&& other)
		: jobState(other.jobState)
		, m_pSrcRM(std::move(other.m_pSrcRM))
		, m_pIndexRM(std::move(other.m_pIndexRM))
		, m_nMeshSubSetMask(other.m_nMeshSubSetMask)
	{}
};

class RenderMesh_hash_int32
{
public:
	ILINE size_t operator()(i32 key) const
	{
		return stl::hash_uint32()((u32)key);
	}
};

struct SMeshStream
{
  buffer_handle_t m_nID;   // device buffer handle from device buffer manager 
  uk m_pUpdateData;     // system buffer for updating (used for async. mesh updates)
  uk m_pLockedData;     // locked device buffer data (hmm, not a good idea to store)
  u32 m_nLockFlags : 16;
  u32 m_nLockCount : 16;
  u32 m_nElements;
  i32  m_nFrameAccess;
  i32  m_nFrameRequest;
  i32  m_nFrameUpdate;
  i32  m_nFrameCreate;

  SMeshStream()
  {
    m_nID = ~0u;
    m_pUpdateData = NULL;
    m_pLockedData = NULL;

		// m_nFrameRequest MUST be <= m_nFrameUpdate initially to prevent a case where LockIB(FSL_READ) will pull the GPU buffer to the CPU (because FreeIB happened in the past)
		// In this case, while thread X is halfway through memcpy of the data as part of LockIB, the RenderThread sees m_pUpdateData without FSL_WRITE flag, and re-uploads the incompletely copied data.
		m_nFrameRequest = -1;
    m_nFrameUpdate = -1;
    m_nFrameAccess = -1;
		m_nFrameCreate = -1;
    m_nLockFlags = 0;
    m_nLockCount = 0;
    m_nElements = 0;
  }

  ~SMeshStream() { memset(this, 0x0, sizeof(*this)); }
};

// CRenderMesh::m_nFlags 
#define FRM_RELEASED              BIT(0)
#define FRM_DEPRECTATED_FLAG      BIT(1)
#define FRM_READYTOUPLOAD         BIT(2)
#define FRM_ALLOCFAILURE          BIT(3)
#define FRM_SKINNED               BIT(4)
#define FRM_SKINNEDNEXTDRAW       BIT(5) // no proper support yet for objects that can be skinned and not skinned.
#define FRM_ENABLE_NORMALSTREAM   BIT(6)
#define FRM_SKINNED_EIGHT_WEIGHTS BIT(7)

#if defined(FEATURE_SVO_GI)
  #define MAX_RELEASED_MESH_FRAMES (4) // GI voxelization threads may keep using render mesh for several frames
#else
  #define MAX_RELEASED_MESH_FRAMES (2)
#endif

struct SSetMeshIntData
{
	CMesh *m_pMesh;
	char *m_pVBuff;
	SPipTangents *m_pTBuff;
	SPipQTangents *m_pQTBuff;
	SVF_P3F *m_pVelocities;
	u32 m_nVerts;
	u32 m_nInds;
	vtx_idx *m_pInds;
	const Vec3 *m_pPosOffset;
	u32 m_flags;
 	Vec3 *m_pNormalsBuff;
};

class CRenderMesh : public IRenderMesh
{
  friend class CREMeshImpl;

public:

	static void ClearJobResources();

private:
  friend class CD3D9Renderer;

	SMeshStream  m_IBStream;
	SMeshStream* m_VBStream[VSF_NUM];
	struct SBoneIndexStream
	{
		buffer_handle_t buffer; 
		u32 guid; 
		u32 refcount; 
	}; 

	struct SBoneIndexStreamRequest
	{
		SBoneIndexStreamRequest(i32 _frameId, u32 _guid, SVF_W4B_I4S *_pStream, SMeshBoneMapping_u16 *_pExtraStream) :
			pStream(_pStream), 
			pExtraStream(_pExtraStream), 
			guid(_guid), refcount(1), frameId(_frameId)
		{}

		SVF_W4B_I4S *pStream;
		SMeshBoneMapping_u16 *pExtraStream;
		u32 guid; 
		u32 refcount; 
		i32    frameId;
	};

	std::vector<SBoneIndexStream> m_RemappedBoneIndices;
	std::vector<SBoneIndexStreamRequest> m_CreatedBoneIndices;
	std::vector<u32> m_DeletedBoneIndices;	// guid

	u32 m_nInds;
	u32 m_nVerts;
  i32   m_nRefCounter;
	InputLayoutHandle m_eVF;          // Base stream vertex format (optional streams are hardcoded: VSF_)

  Vec3 *m_pCachePos;         // float positions (cached)
  i32   m_nFrameRequestCachePos;
 
	Vec2 *m_pCacheUVs;         // float UVs (cached)
	i32   m_nFrameRequestCacheUVs;

  CRenderMesh *m_pVertexContainer;
  PodArray<CRenderMesh*>  m_lstVertexContainerUsers;

#ifdef RENDER_MESH_TRIANGLE_HASH_MAP_SUPPORT
  typedef std::unordered_map<i32, PodArray<std::pair<i32, i32>>, RenderMesh_hash_int32> TrisMap;
  TrisMap * m_pTrisMap;
#endif

  SRecursiveSpinLock m_sResLock;

  i32	m_nThreadAccessCounter; // counter to ensure that no system rendermesh streams are freed since they are in use
	 i32  m_asyncUpdateState[2];
	i32  m_asyncUpdateStateCounter[2];

	ERenderPrimitiveType m_nPrimetiveType;
	ERenderMeshType m_eType;
	u16 m_nFlags														: 8;          // FRM_
  i16  m_nLod                             : 4;          // used for LOD debug visualization
  bool m_keepSysMesh                        : 1;
  bool m_nFlagsCachePos                     : 1;          // only checked for FSL_WRITE, which can be represented as a single bit
	bool m_nFlagsCacheUVs											: 1;

public:
	enum ESizeUsageArg
	{
		SIZE_ONLY_SYSTEM = 0,
		SIZE_VB = 1,
		SIZE_IB = 2,
	};

private:
  SMeshStream* GetVertexStream(i32 nStream, u32 nFlags = 0);
  SMeshStream* GetVertexStream(i32 nStream, u32 nFlags = 0) const { return m_VBStream[nStream]; }
  bool UpdateVidIndices(SMeshStream& IBStream, bool stall=true);

  bool CreateVidVertices(i32 nVerts=0, InputLayoutHandle eVF=InputLayoutHandle::Unspecified, i32 nStream=VSF_GENERAL);
  bool UpdateVidVertices(i32 nStream);

  bool CopyStreamToSystemForUpdate(SMeshStream& MS, size_t nSize);

  void ReleaseVB(i32 nStream);
  void ReleaseIB();

  void InitTriHash(IMaterial * pMaterial);

  bool CreateCachePos(byte *pSrc, u32 nStrideSrc, u32 nFlags);
  bool PrepareCachePos();
	bool CreateCacheUVs(byte *pSrc, u32 nStrideSrc, u32 nFlags);	

	//Internal versions of funcs - no lock
	bool UpdateVertices_Int(ukk pVertBuffer, i32 nVertCount, i32 nOffset, i32 nStream, u32 copyFlags);
  bool UpdateIndices_Int(const vtx_idx *pNewInds, i32 nInds, i32 nOffsInd, u32 copyFlags);
  size_t SetMesh_Int( CMesh &mesh, i32 nSecColorsSetOffset, u32 flags, const Vec3 *pPosOffset);	

#ifdef MESH_TESSELLATION_RENDERER
	template<class VertexFormat, class VecPos, class VecUV>
	bool UpdateUVCoordsAdjacency(SMeshStream& IBStream);

	template<class VertexFormat, class VecPos, class VecUV>
	static void BuildAdjacency(const VertexFormat *pVerts, uint nVerts, const vtx_idx *pIndexBuffer, uint nTrgs, std::vector<VecUV> &pTxtAdjBuffer);
#endif

	void Cleanup();

public:

	void AddShadowPassMergedChunkIndicesAndVertices(CRenderChunk *pCurrentChunk, IMaterial *pMaterial, i32 &rNumVertices, i32 &rNumIndices );
	static bool RenderChunkMergeAbleInShadowPass(CRenderChunk *pPreviousChunk, CRenderChunk *pCurrentChunk, IMaterial *pMaterial );

	inline void PrefetchVertexStreams() const { for (i32 i = 0; i < VSF_NUM; DrxPrefetch(m_VBStream[i++])); }

	void SetMesh_IntImpl( SSetMeshIntData data );

	//! constructor
	//! /param szSource this pointer is stored - make sure the memory stays
	CRenderMesh(tukk szType, tukk szSourceName, bool bLock=false);
  CRenderMesh();

	//! destructor
	~CRenderMesh();

	virtual bool CanUpdate() final { return (m_nFlags & FRM_ALLOCFAILURE) == 0; }
	virtual bool CanRender() final { return (m_nFlags & FRM_ALLOCFAILURE) == 0 && CheckStreams(); }

	inline bool IsSkinned() const
	{
		return (m_nFlags & (FRM_SKINNED | FRM_SKINNEDNEXTDRAW)) != 0;
	}

	virtual void AddRef() final
	{
#   if !defined(_RELEASE)
		if (m_nFlags & FRM_RELEASED)
			DrxFatalError("CRenderMesh::AddRef() mesh already in the garbage list (resurrecting deleted mesh)");
#   endif
		DrxInterlockedIncrement(&m_nRefCounter);
	}
	virtual i32 Release() final;
  void ReleaseForce()
  {
    while (true)
    {
      i32 nRef = Release();
#if !defined(_RELEASE) && defined(_DEBUG)
			IF (nRef < 0, 0)
			__debugbreak();
#endif
			if (nRef == 0)
        return;
    }
  }

	// ----------------------------------------------------------------
	// Helper functions
	inline i32 GetStreamStride(i32 nStream) const
	{
		InputLayoutHandle eVF = m_eVF;

		if (nStream != VSF_GENERAL)
		{
			switch (nStream)
			{
		#ifdef TANG_FLOATS
				case VSF_TANGENTS       : eVF = EDefaultInputLayouts::T4F_B4F; break;
				case VSF_QTANGENTS      : eVF = EDefaultInputLayouts::Q4F; break;
		#else
				case VSF_TANGENTS       : eVF = EDefaultInputLayouts::T4S_B4S; break;
				case VSF_QTANGENTS      : eVF = EDefaultInputLayouts::Q4S; break;
		#endif
				case VSF_HWSKIN_INFO    : eVF = EDefaultInputLayouts::W4B_I4S; break;
				case VSF_VERTEX_VELOCITY: eVF = EDefaultInputLayouts::V3F; break;
				case VSF_NORMALS        : eVF = EDefaultInputLayouts::N3F; break;
				default:
					DrxWarning(EValidatorModule::VALIDATOR_MODULE_RENDERER, EValidatorSeverity::VALIDATOR_WARNING, "Unknown nStream");
					return 0;
			}
		}

		u16 Stride = CDeviceObjectFactory::GetInputLayoutDescriptor(eVF)->m_Strides[0];
		assert(Stride != 0);

		return Stride;
	}

  inline u32 _GetFlags() const { return m_nFlags; }
  inline i32 GetStreamSize(i32 nStream, i32 nVerts=0) const { return GetStreamStride(nStream) * (nVerts ? nVerts : m_nVerts); }
  inline const buffer_handle_t _GetVBStream(i32 nStream) const { if (!m_VBStream[nStream]) return ~0u; return m_VBStream[nStream]->m_nID; }
  inline const buffer_handle_t _GetIBStream() const { return m_IBStream.m_nID; }
  inline bool _NeedsVBStream(i32 nStream) const { return m_VBStream[nStream] && m_VBStream[nStream]->m_pUpdateData && (m_VBStream[nStream]->m_nFrameRequest > m_VBStream[nStream]->m_nFrameUpdate); }
  inline bool _NeedsIBStream() const { return m_IBStream.m_pUpdateData && (m_IBStream.m_nFrameRequest > m_IBStream.m_nFrameUpdate); }
  inline bool _HasVBStream(i32 nStream) const { return m_VBStream[nStream] && m_VBStream[nStream]->m_nID!=~0u; }
  inline bool _HasIBStream() const { return m_IBStream.m_nID!=~0u; }
  inline i32 _IsVBStreamLocked(i32 nStream) const { if (!m_VBStream[nStream]) return 0; return (m_VBStream[nStream]->m_nLockFlags & FSL_LOCKED); }
  inline i32 _IsIBStreamLocked() const { return m_IBStream.m_nLockFlags & FSL_LOCKED; }
  inline InputLayoutHandle _GetVertexFormat() const { return m_eVF; }
  inline void _SetVertexFormat(InputLayoutHandle eVF) { m_eVF = eVF; }
  inline i32 _GetNumVerts() const { return m_nVerts; }
  inline void _SetNumVerts(i32 nVerts) { m_nVerts = max(nVerts, 0); }
  inline i32 _GetNumInds() const { return m_nInds; }
  inline void _SetNumInds(i32 nInds) { m_nInds = nInds; }
	inline const ERenderPrimitiveType _GetPrimitiveType() const                               { return m_nPrimetiveType; }
	inline void                       _SetPrimitiveType(const ERenderPrimitiveType nPrimType) { m_nPrimetiveType = nPrimType; }
  inline void _SetRenderMeshType(ERenderMeshType eType) { m_eType = eType; }
  inline CRenderMesh *_GetVertexContainer()
  {
    if (m_pVertexContainer)
      return m_pVertexContainer;
    return this;
  }

  D3DBuffer* _GetD3DVB(i32 nStream, buffer_size_t* offs) const;
  D3DBuffer* _GetD3DIB(buffer_size_t* offs) const;

  buffer_size_t Size(u32 nFlags) const;
	void Size(u32 nFlags, IDrxSizer *pSizer ) const;

  uk LockVB(i32 nStream, u32 nFlags, i32 nOffset=0, i32 nVerts=0, i32 *nStride=NULL, bool prefetchIB=false, bool inplaceCachePos=false);

	template<class T>
	T* GetStridedArray(strided_pointer<T>& arr, EStreamIDs stream)
	{
		arr.data = (T*)LockVB(stream, FSL_READ, 0, 0, &arr.iStride);
		assert(!arr.data || arr.iStride >= sizeof(T));
		return arr.data;
	}

	template<class T>
	T* GetStridedArray(strided_pointer<T>& arr, EStreamIDs stream, i32 dataType)
	{
		if (GetStridedArray(arr, stream))
		{
			const auto vertexFormatDescriptor = CDeviceObjectFactory::GetInputLayoutDescriptor(_GetVertexFormat());
			int8 offset = vertexFormatDescriptor ? vertexFormatDescriptor->m_Offsets[dataType] : -1;
			if (offset < 0)
				arr.data = nullptr;
			else
				arr.data = (T*)((tuk)arr.data + offset);
		}
		return arr.data;
	}

  vtx_idx *LockIB(u32 nFlags, i32 nOffset=0, i32 nInds=0);
  void UnlockVB(i32 nStream);
  void UnlockIB();

  bool RT_CheckUpdate(CRenderMesh *pVContainer, InputLayoutHandle eVF, u32 nStreamMask, bool bTessellation = false);
  void RT_SetMeshCleanup();
  void RT_AllocationFailure(tukk sPurpose, u32 nSize);

  void AssignChunk(CRenderChunk *pChunk, class CREMeshImpl *pRE);
  void InitRenderChunk( CRenderChunk &rChunk );

  void FreeVB(i32 nStream);
  void FreeIB();
  void FreeDeviceBuffers(bool bRestoreSys);
  void FreeSystemBuffers();
  void FreePreallocatedData();

	bool SyncAsyncUpdate(i32 threadId, bool block = true);
	void MarkRenderElementsDirty();

  //===========================================================================================
  // IRenderMesh interface
	virtual tukk GetTypeName() final         { return m_sType; }
	virtual tukk GetSourceName() const final { return m_sSource; }

	virtual i32 GetIndicesCount() final  { return m_nInds; }
	virtual i32 GetVerticesCount() final { return m_nVerts; }

	virtual InputLayoutHandle   GetVertexFormat() final { return m_eVF; }
	virtual ERenderMeshType GetMeshType() final     { return m_eType; }

	virtual void SetSkinned(bool bSkinned = true) final
  {
    if (bSkinned) m_nFlags |=  FRM_SKINNED;
    else          m_nFlags &= ~FRM_SKINNED;
  };
	virtual uint GetSkinningWeightCount() const final;

	virtual float GetGeometricMeanFaceArea() const final { return m_fGeometricMeanFaceArea; }

	virtual void NextDrawSkinned() final { m_nFlags |= FRM_SKINNEDNEXTDRAW; }

  virtual void GenerateQTangents() final;
  virtual void CreateChunksSkinned() final;
  virtual void CopyTo(IRenderMesh *pDst, i32 nAppendVtx=0, bool bDynamic=false, bool fullCopy=true) final;
  virtual void SetSkinningDataVegetation(struct SMeshBoneMapping_uint8 *pBoneMapping) final;
  virtual void SetSkinningDataCharacter(CMesh& mesh, u32 flags, struct SMeshBoneMapping_u16 *pBoneMapping, struct SMeshBoneMapping_u16 *pExtraBoneMapping) final; 
  // Creates an indexed mesh from this render mesh (accepts an optional pointer to an IIndexedMesh object that should be used)
	virtual IIndexedMesh* GetIndexedMesh(IIndexedMesh* pIdxMesh = 0) final;
	virtual i32           GetRenderChunksCount(IMaterial* pMat, i32& nRenderTrisCount) final;

	virtual IRenderMesh* GenerateMorphWeights() final             { return NULL; }
	virtual IRenderMesh* GetMorphBuddy() final                    { return NULL; }
	virtual void         SetMorphBuddy(IRenderMesh* pMorph) final {}

  // Create render buffers from render mesh. Returns the final size of the render mesh or ~0U on failure
	virtual size_t SetMesh(CMesh& mesh, i32 nSecColorsSetOffset, u32 flags, const Vec3* pPosOffset, bool requiresLock) final;

  // Update system vertices buffer
	virtual bool UpdateVertices(ukk pVertBuffer, i32 nVertCount, i32 nOffset, i32 nStream, u32 copyFlags, bool requiresLock = true) final;
  // Update system indices buffer
	virtual bool UpdateIndices(const vtx_idx* pNewInds, i32 nInds, i32 nOffsInd, u32 copyFlags, bool requiresLock = true) final;

	virtual void SetCustomTexID(i32 nCustomTID) final;
	virtual void SetChunk(i32 nIndex, CRenderChunk& chunk) final;
	virtual void SetChunk(IMaterial* pNewMat, i32 nFirstVertId, i32 nVertCount, i32 nFirstIndexId, i32 nIndexCount, float texelAreaDensity, i32 nMatID = 0) final;

	virtual void SetRenderChunks(CRenderChunk* pChunksArray, i32 nCount, bool bSubObjectChunks) final;

	virtual TRenderChunkArray& GetChunks() final           { return m_Chunks; }
	virtual TRenderChunkArray& GetChunksSkinned() final    { return m_ChunksSkinned; }
	virtual TRenderChunkArray& GetChunksSubObjects() final { return m_ChunksSubObjects; }
	virtual IRenderMesh*       GetVertexContainer() final  { return _GetVertexContainer(); }
	virtual void               SetVertexContainer(IRenderMesh* pBuf) final;

	virtual void Render(CRenderObject* pObj, const SRenderingPassInfo& passInfo) final;
	virtual void AddRenderElements(IMaterial* pIMatInfo, CRenderObject* pObj, const SRenderingPassInfo& passInfo, i32 nSortId = EFSLIST_GENERAL, i32 nAW = 1) final;
	virtual void SetREUserData(float* pfCustomData, float fFogScale = 0, float fAlpha = 1) final;
	virtual void AddRE(IMaterial* pMaterial, CRenderObject* pObj, IShader* pEf, const SRenderingPassInfo& passInfo, i32 nList, i32 nAW) final;

	virtual byte* GetPosPtrNoCache(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetPosPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetNormPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetColorPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetUVPtrNoCache(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetUVPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;

	virtual byte* GetTangentPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;
	virtual byte* GetQTangentPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;

	virtual byte* GetHWSkinPtr(i32& nStride, u32 nFlags, i32 nOffset = 0, bool remapped = false) final;
	virtual byte* GetVelocityPtr(i32& nStride, u32 nFlags, i32 nOffset = 0) final;

	virtual void UnlockStream(i32 nStream) final;
	virtual void UnlockIndexStream() final;

	virtual vtx_idx*                              GetIndexPtr(u32 nFlags, i32 nOffset = 0) final;
	virtual const PodArray<std::pair<i32, i32> >* GetTrisForPosition(const Vec3& vPos, IMaterial* pMaterial) final;
	virtual float                                 GetExtent(EGeomForm eForm) final;
	virtual void                                  GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm, SSkinningData const* pSkinning = NULL) final;
	virtual u32*                               GetPhysVertexMap() final { return NULL; }
	virtual bool                                  IsEmpty() final;

	virtual size_t GetMemoryUsage(IDrxSizer* pSizer, EMemoryUsageArgument nType) const final;
	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const final;
	virtual float  GetAverageTrisNumPerChunk(IMaterial* pMat) final;
	virtual i32    GetTextureMemoryUsage(const IMaterial* pMaterial, IDrxSizer* pSizer = NULL, bool bStreamedIn = true) const final;
	// Get allocated only in video memory or only in system memory.
	virtual i32 GetAllocatedBytes(bool bVideoMem) const final;

	virtual void SetBBox(const Vec3& vBoxMin, const Vec3& vBoxMax) final { m_vBoxMin = vBoxMin; m_vBoxMax = vBoxMax; }
	virtual void GetBBox(Vec3& vBoxMin, Vec3& vBoxMax) final             { vBoxMin = m_vBoxMin; vBoxMax = m_vBoxMax; };
	virtual void UpdateBBoxFromMesh() final;

  // Debug draw this render mesh.
	virtual void DebugDraw(const struct SGeometryDebugDrawInfo& info, u32 nVisibleChunksMask = ~0) final;
	virtual void KeepSysMesh(bool keep) final;
	virtual void UnKeepSysMesh() final;
	virtual void LockForThreadAccess() final;
	virtual void UnLockForThreadAccess() final;

	virtual void SetMeshLod(i32 nLod) final { m_nLod = nLod; }

	virtual  i32* SetAsyncUpdateState() final;
	void CreateRemappedBoneIndicesPair(const uint pairGuid, const TRenderChunkArray& Chunks);
 	virtual void CreateRemappedBoneIndicesPair(const DynArray<JointIdType> &arrRemapTable, const uint pairGuid, ukk tag) final;
	virtual void ReleaseRemappedBoneIndicesPair(const uint pairGuid) final;

	virtual void OffsetPosition(const Vec3& delta) final { m_vBoxMin += delta; m_vBoxMax += delta; }

	virtual bool RayIntersectMesh(const Ray& ray, Vec3& hitpos, Vec3& p0, Vec3& p1, Vec3& p2, Vec2& uv0, Vec2& uv1, Vec2& uv2) final;

  IRenderMesh* GetRenderMeshForSubsetMask(SRenderObjData *pOD, hidemask nMeshSubSetMask, IMaterial * pMaterial, const SRenderingPassInfo &passInfo);
	void GarbageCollectSubsetRenderMeshes();
	void CreateSubSetRenderMesh();

  void ReleaseRenderChunks(TRenderChunkArray* pChunks);

	bool GetRemappedSkinningData(u32 guid, SStreamInfo& streamInfo);
	bool FillGeometryInfo(CRenderElement::SGeometryInfo& geomInfo);
	bool CheckStreams();

private:
	void AddHUDRenderElement(CRenderObject* pObj, IMaterial* pMaterial, const SRenderingPassInfo& passInfo);

public:
	// --------------------------------------------------------------
	// Members

	// When modifying or traversing any of the lists below, be sure to always hold the link lock
	static DrxCriticalSection      m_sLinkLock;

	// intrusive list entries - a mesh can be in multiple lists at the same time
	util::list<CRenderMesh>        m_Chain;       // mesh will either be in the mesh list or garbage mesh list
	util::list<CRenderMesh>        m_Dirty[2];    // if linked, mesh has  data (data read back from vram)
	util::list<CRenderMesh>        m_Modified[2]; // if linked, mesh has modified data (to be uploaded to vram)

	// The static list heads, corresponds to the entries above
	static util::list<CRenderMesh> s_MeshList;
	static util::list<CRenderMesh> s_MeshGarbageList[MAX_RELEASED_MESH_FRAMES];
	static util::list<CRenderMesh> s_MeshDirtyList[2];
	static util::list<CRenderMesh> s_MeshModifiedList[2];

	TRenderChunkArray              m_Chunks;
	TRenderChunkArray              m_ChunksSubObjects; // Chunks of sub-objects.
	TRenderChunkArray              m_ChunksSkinned;

	i32                            m_nClientTextureBindID;
	Vec3                           m_vBoxMin;
	Vec3                           m_vBoxMax;

	float                          m_fGeometricMeanFaceArea;
	CGeomExtents                   m_Extents;
	DynArray<PosNorm>              m_PosNorms;

	// Frame id when this render mesh was last rendered.
	u32                         m_nLastRenderFrameID;
	u32                         m_nLastSubsetGCRenderFrameID;

	string                         m_sType;          //!< pointer to the type name in the constructor call
	string                         m_sSource;        //!< pointer to the source  name in the constructor call

	// For debugging purposes to catch longstanding data accesses
# if !defined(_RELEASE) && defined(RM_CATCH_EXCESSIVE_LOCKS)
	float m_lockTime;
# endif

	typedef VectorMap<hidemask,_smart_ptr<IRenderMesh> > MeshSubSetIndices;
	MeshSubSetIndices m_meshSubSetIndices;

	static CThreadSafeRendererContainer<SMeshSubSetIndicesJobEntry> m_meshSubSetRenderMeshJobs[RT_COMMAND_BUF_COUNT];
	static CThreadSafeRendererContainer<CRenderMesh*> m_deferredSubsetGarbageCollection[RT_COMMAND_BUF_COUNT];

#ifdef RENDER_MESH_TRIANGLE_HASH_MAP_SUPPORT
  DrxCriticalSection m_getTrisForPositionLock;
#endif

#ifdef MESH_TESSELLATION_RENDERER
	CGpuBuffer m_adjBuffer;                // buffer containing adjacency information to fix displacement seams
#endif

	CGpuBuffer m_extraBonesBuffer;

	// shared inputs
	std::shared_ptr<compute_skinning::IPerMeshDataSupply> m_computeSkinningDataSupply;
	u32 m_nMorphs;

	void ComputeSkinningCreateSkinningBuffers(const SVF_W4B_I4S* pBoneMapping, const SMeshBoneMapping_u16* pExtraBoneMapping);
	void ComputeSkinningCreateBindPoseAndMorphBuffers(CMesh& mesh);
	SMeshBoneMapping_u16* m_pExtraBoneMapping;

	static void Initialize();
	static void ShutDown();
	static void Tick(uint numFrames = 1);
	static void UpdateModified();
	static void UpdateModifiedMeshes(bool bAcquireLock, i32 threadId);
	static bool ClearStaleMemory(bool bAcquireLock, i32 threadId);
	static void PrintMeshLeaks();
	static void GetPoolStats(SMeshPoolStatistics* stats);

	uk operator new(size_t size);
	void operator delete(uk ptr);

	static void RT_PerFrameTick();
};

//////////////////////////////////////////////////////////////////////
// General VertexBuffer created by CreateVertexBuffer() function
class CVertexBuffer
{
public:
	CVertexBuffer()
	{
		m_eVF = InputLayoutHandle::Unspecified;
		m_nVerts = 0;
	}
	CVertexBuffer(uk pData, InputLayoutHandle eVF, i32 nVerts = 0)
	{
		m_VData = pData;
		m_eVF = eVF;
		m_nVerts = nVerts;
	}

	uk m_VData;
	InputLayoutHandle m_eVF;
	i32 m_nVerts;
};

class CIndexBuffer
{
public:
	CIndexBuffer()
	{
		m_nInds = 0;
	}

	CIndexBuffer(u16* pData)
	{
		m_IData = pData;
		m_nInds = 0;
	}

	uk m_IData;
	i32 m_nInds;
};

#endif // __RenderMesh2_h__
