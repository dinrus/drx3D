// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCENETREE_H__
#define __SCENETREE_H__

#if defined(FEATURE_SVO_GI)

	#pragma pack(push,4)

typedef std::unordered_map<uint64, std::pair<byte, byte>> PvsMap;
typedef std::set<class CVoxelSegment*>                    VsSet;

template<class T, i32 maxElemsInChunk> class CCustomSVOPoolAllocator
{
public:
	CCustomSVOPoolAllocator() { m_numElements = 0; }

	~CCustomSVOPoolAllocator()
	{
		Reset();
	}

	void Reset()
	{
		for (i32 i = 0; i < m_chunksPool.Count(); i++)
		{
			delete[](byte*)m_chunksPool[i];
			m_chunksPool[i] = NULL;
		}
		m_numElements = 0;
	}

	void ReleaseElement(T* pElem)
	{
		if (pElem)
			m_freeElements.Add(pElem);
	}

	T* GetNewElement()
	{
		if (m_freeElements.Count())
		{
			T* pPtr = m_freeElements.Last();
			m_freeElements.DeleteLast();
			return pPtr;
		}

		i32 poolId = m_numElements / maxElemsInChunk;
		i32 elemId = m_numElements - poolId * maxElemsInChunk;
		m_chunksPool.PreAllocate(poolId + 1, poolId + 1);
		if (!m_chunksPool[poolId])
			m_chunksPool[poolId] = (T*)new byte[maxElemsInChunk * sizeof(T)];
		m_numElements++;
		return &m_chunksPool[poolId][elemId];
	}

	i32 GetCount()         { return m_numElements - m_freeElements.Count(); }
	i32 GetCapacity()      { return m_chunksPool.Count() * maxElemsInChunk; }
	i32 GetCapacityBytes() { return GetCapacity() * sizeof(T); }

private:

	i32          m_numElements;
	PodArray<T*> m_chunksPool;
	PodArray<T*> m_freeElements;
};

class CSvoNode
{
public:

	CSvoNode(const AABB& box, CSvoNode* pParent);
	~CSvoNode();

	static uk         operator new(size_t);
	static void          operator delete(uk ptr);

	void                 CheckAllocateChilds();
	void                 DeleteChilds();
	void                 Render(PodArray<struct SPvsItem>* pSortedPVS, uint64 nodeKey, i32 treeLevel, PodArray<SVF_P3F_C4B_T2F>& arrVertsOut, PodArray<class CVoxelSegment*> arrForStreaming[SVO_STREAM_QUEUE_MAX_SIZE][SVO_STREAM_QUEUE_MAX_SIZE]);
	bool                 IsStreamingInProgress();
	void                 GetTrisInAreaStats(i32& trisCount, i32& vertCount, i32& trisBytes, i32& vertBytes, i32& maxVertPerArea, i32& matsCount);
	void                 GetVoxSegMemUsage(i32& allocated);
	AABB                 GetChildBBox(i32 childId);
	void                 CheckAllocateSegment(i32 lod);
	void                 OnStatLightsChanged(const AABB& objBox);
	class CVoxelSegment* AllocateSegment(i32 cloudId, i32 stationId, i32 lod, EFileStreamingStatus eStreamingStatus, bool bDroppedOnDisk);
	u32               SaveNode(PodArray<byte>& arrData, u32& nNodesCounter, IDrxArchive* pArchive, u32& totalSizeCounter);
	void                 MakeNodeFilePath(tuk szFileName);
	bool                 CheckReadyForRendering(i32 treeLevel, PodArray<CVoxelSegment*> arrForStreaming[SVO_STREAM_QUEUE_MAX_SIZE][SVO_STREAM_QUEUE_MAX_SIZE]);
	CSvoNode*            FindNodeByPosition(const Vec3& vPosWS, i32 treeLevelToFind, i32 treeLevelCur);
	void                 UpdateNodeRenderDataPtrs();
	void                 RegisterMovement(const AABB& objBox);
	Vec3i                GetStatGeomCheckSumm();
	CSvoNode*            FindNodeByPoolAffset(i32 allocatedAtlasOffset);
	static bool          IsStreamingActive();

	AABB                       m_nodeBox;
	CSvoNode**                 m_ppChilds;
	std::pair<u32, u32>* m_pChildFileOffsets;
	CSvoNode*                  m_pParent;
	CVoxelSegment*             m_pSeg;
	uint                       m_requestSegmentUpdateFrametId;
	bool                       m_arrChildNotNeeded[8];
	bool                       m_bForceRecreate;
};

class CPointTreeNode
{
public:
	bool TryInsertPoint(i32 pointId, const Vec3& vPos, const AABB& nodeBox, i32 recursionLevel = 0);
	bool IsThereAnyPointInTheBox(const AABB& testBox, const AABB& nodeBox);
	bool GetAllPointsInTheBox(const AABB& testBox, const AABB& nodeBox, PodArray<i32>& arrIds);
	AABB GetChildBBox(i32 childId, const AABB& nodeBox);
	void Clear();
	CPointTreeNode() { m_ppChilds = 0; m_pPoints = 0; }
	~CPointTreeNode() { Clear(); }

	struct SPointInfo
	{ Vec3 vPos; i32 id; };
	PodArray<SPointInfo>* m_pPoints;
	CPointTreeNode**      m_ppChilds;
};

struct SBrickSubSet
{
	ColorB arrData[SVO_VOX_BRICK_MAX_SIZE * SVO_VOX_BRICK_MAX_SIZE * SVO_VOX_BRICK_MAX_SIZE];
};

class CSvoEnv : public DinrusX3dEngBase
{
public:

	CSvoEnv(const AABB& worldBox);
	~CSvoEnv();
	bool GetSvoStaticTextures(I3DEngine::SSvoStaticTexInfo& svoInfo, PodArray<I3DEngine::SLightTI>* pLightsTI_S, PodArray<I3DEngine::SLightTI>* pLightsTI_D);
	void GetSvoBricksForUpdate(PodArray<I3DEngine::SSvoNodeInfo>& arrNodeInfo, float nodeSize, PodArray<SVF_P3F_C4B_T2F>* pVertsOut);
	bool Render();
	void ProcessSvoRootTeleport();
	void CheckUpdateMeshPools();
	i32  GetWorstPointInSubSet(i32k start, i32k end);
	void StartupStreamingTimeTest(bool bDone);
	void OnLevelGeometryChanged();
	void ReconstructTree(bool bMultiPoint);
	void AllocateRootNode();
	i32  ExportSvo(IDrxArchive* pArchive);
	void DetectMovement_StaticGeom();
	void DetectMovement_StatLights();
	void CollectLights();
	void CollectAnalyticalOccluders();
	void AddAnalyticalOccluder(IRenderNode* pRN, Vec3 camPos);
	void GetGlobalEnvProbeProperties(_smart_ptr<ITexture>& specEnvCM, float& mult);

	PodArray<I3DEngine::SLightTI>            m_lightsTI_S, m_lightsTI_D;
	PodArray<I3DEngine::SAnalyticalOccluder> m_analyticalOccluders[2];
	Vec4                      m_vSvoOriginAndSize;
	AABB                      m_aabbLightsTI_D;
	SRenderLight*             m_pGlobalEnvProbe;
	DrxCriticalSection        m_csLockGlobalEnvProbe;
	CVoxStreamEngine*         m_pStreamEngine;
	CSvoNode*                 m_pSvoRoot;
	bool                      m_bReady;
	bool                      m_bRootTeleportSkipFrame = false;
	PodArray<CVoxelSegment*>  m_arrForStreaming[SVO_STREAM_QUEUE_MAX_SIZE][SVO_STREAM_QUEUE_MAX_SIZE];
	i32                       m_debugDrawVoxelsCounter;
	i32                       m_nodeCounter;
	i32                       m_dynNodeCounter;
	i32                       m_dynNodeCounter_DYNL;
	PodArray<CVoxelSegment*>  m_arrForBrickUpdate[16];
	DrxCriticalSection        m_csLockTree;
	float                     m_streamingStartTime;
	float                     m_svoFreezeTime;
	i32                       m_arrVoxelizeMeshesCounter[2];
	AABB                      m_worldBox;
	PodArray<SVF_P3F_C4B_T2F> m_arrSvoProxyVertices;
	double                    m_prevCheckVal;
	bool                      m_bFirst_SvoFreezeTime;
	bool                      m_bFirst_StartStreaming;
	bool                      m_bStreamingDonePrev;

	i32                       m_texOpasPoolId;
	i32                       m_texNodePoolId;
	i32                       m_texNormPoolId;
	i32                       m_texRgb0PoolId;
	i32                       m_texRgb1PoolId;
	i32                       m_texRgb2PoolId;
	i32                       m_texRgb3PoolId;
	i32                       m_texRgb4PoolId;
	i32                       m_texDynlPoolId;
	i32                       m_texAldiPoolId;
	#ifdef FEATURE_SVO_GI_USE_MESH_RT
	i32                       m_texTrisPoolId;
	#endif

	ETEX_Format                                       m_voxTexFormat;
	TDoublyLinkedList<CVoxelSegment>                  m_arrSegForUnload;
	CCustomSVOPoolAllocator<struct SBrickSubSet, 128> m_brickSubSetAllocator;
	CCustomSVOPoolAllocator<CSvoNode, 128>            m_nodeAllocator;

	#ifdef FEATURE_SVO_GI_USE_MESH_RT
	PodArrayRT<ColorB> m_arrRTPoolTexs;
	PodArrayRT<Vec4>   m_arrRTPoolTris;
	PodArrayRT<ColorB> m_arrRTPoolInds;
	#endif
};

	#pragma pack(pop)

#endif

#endif
