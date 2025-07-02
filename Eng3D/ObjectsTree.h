// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INCLUDE_DRX3DENGINE_OBJECTTREE_H
#define __INCLUDE_DRX3DENGINE_OBJECTTREE_H

#pragma once

#define OCTREENODE_RENDER_FLAG_OBJECTS               1
#define OCTREENODE_RENDER_FLAG_OCCLUDERS             2
#define OCTREENODE_RENDER_FLAG_CASTERS               4
#define OCTREENODE_RENDER_FLAG_OBJECTS_ONLY_ENTITIES 8

#define OCTREENODE_CHUNK_VERSION                     5

constexpr u32 kPassCullMainBitId = 0;
constexpr u32 kPassCullMainMask = BIT(kPassCullMainBitId);

enum ELoadObjectsMode { LOM_LOAD_ALL, LOM_LOAD_ONLY_NON_STREAMABLE, LOM_LOAD_ONLY_STREAMABLE };

class CBrush;
class COctreeNode;
template<class T, size_t overAllocBytes> class PodArray;
struct ILightSource;
struct IParticleEmitter;

///////////////////////////////////////////////////////////////////////////////
// data to be pushed to occlusion culler
struct DRX_ALIGN(16) SCheckOcclusionJobData
{
	enum JobTypeT { QUIT, OCTREE_NODE, TERRAIN_NODE };

	SCheckOcclusionJobData()
	{
	}

	static SCheckOcclusionJobData CreateQuitJobData();
	static SCheckOcclusionJobData CreateOctreeJobData(COctreeNode * pOctTreeNode, i32 nRenderMask, const Vec3 &rAmbColor, u32 passCullMask, const SRenderingPassInfo &passInfo);
	static SCheckOcclusionJobData CreateTerrainJobData(CTerrainNode * pTerrainNode, const AABB &rAABB, float fDistance, u32 passCullMask);

	JobTypeT type; // type to indicate with which data the union is filled
	union
	{
		// data for octree nodes
		struct
		{
			COctreeNode* pOctTreeNode;
			i32          nRenderMask;
			float        vAmbColor[3];
		} octTreeData;

		// data for terrain nodes
		struct
		{
			CTerrainNode* pTerrainNode;
			float         vAABBMin[3];
			float         vAABBMax[3];
			float         fDistance;
		} terrainData;
	};
	// common data
	SRendItemSorter rendItemSorter; // ensure order octree traversal oder even with parallel execution
	const CCamera* pCam;            // store camera to handle vis areas correctly
	std::vector<SRenderingPassInfo>* pShadowPasses = nullptr;
	u32 passCullMask;
};

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionJobData SCheckOcclusionJobData::CreateQuitJobData()
{
	SCheckOcclusionJobData jobData;
	jobData.type = QUIT;
	return jobData;
}

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionJobData SCheckOcclusionJobData::CreateOctreeJobData(COctreeNode* pOctTreeNode, i32 nRenderMask, const Vec3& rAmbColor, u32 passCullMask, const SRenderingPassInfo& passInfo)
{
	SCheckOcclusionJobData jobData;
	jobData.type = OCTREE_NODE;
	jobData.octTreeData.pOctTreeNode = pOctTreeNode;
	jobData.octTreeData.nRenderMask = nRenderMask;
	jobData.octTreeData.vAmbColor[0] = rAmbColor.x;
	jobData.octTreeData.vAmbColor[1] = rAmbColor.y;
	jobData.octTreeData.vAmbColor[2] = rAmbColor.z;
	jobData.passCullMask = passCullMask;
	jobData.rendItemSorter = passInfo.GetRendItemSorter();
	jobData.pCam = &passInfo.GetCamera();
	jobData.pShadowPasses = passInfo.GetShadowPasses();
	return jobData;
}

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionJobData SCheckOcclusionJobData::CreateTerrainJobData(CTerrainNode* pTerrainNode, const AABB& rAABB, float fDistance, u32 passCullMask)
{
	SCheckOcclusionJobData jobData;
	jobData.type = TERRAIN_NODE;
	jobData.passCullMask = passCullMask;
	jobData.terrainData.pTerrainNode = pTerrainNode;
	jobData.terrainData.vAABBMin[0] = rAABB.min.x;
	jobData.terrainData.vAABBMin[1] = rAABB.min.y;
	jobData.terrainData.vAABBMin[2] = rAABB.min.z;
	jobData.terrainData.vAABBMax[0] = rAABB.max.x;
	jobData.terrainData.vAABBMax[1] = rAABB.max.y;
	jobData.terrainData.vAABBMax[2] = rAABB.max.z;
	jobData.terrainData.fDistance = fDistance;
	return jobData;
}

///////////////////////////////////////////////////////////////////////////////
// data written by occlusion culler jobs, to control main thread 3dengine side rendering
struct DRX_ALIGN(16) SCheckOcclusionOutput
{
	enum JobTypeT { ROAD_DECALS, COMMON, TERRAIN, DEFORMABLE_BRUSH };

	static SCheckOcclusionOutput CreateDecalsAndRoadsOutput(IRenderNode * pObj, PodArray<SRenderLight*>* pAffectingLights, const Vec3 &rAmbColor, const AABB &rObjBox, float fEntDistance, bool bCheckPerObjectOcclusion, const SRenderingPassInfo &passInfo);
	static SCheckOcclusionOutput CreateCommonObjectOutput(IRenderNode * pObj, PodArray<SRenderLight*>* pAffectingLights, const Vec3 &rAmbColor, const AABB &rObjBox, float fEntDistance, SSectorTextureSet * pTerrainTexInfo, u32 passCullMask, const SRenderingPassInfo &passInfo);
	static SCheckOcclusionOutput CreateTerrainOutput(CTerrainNode * pTerrainNode, u32 passCullMask, const SRenderingPassInfo &passInfo);
	static SCheckOcclusionOutput CreateDeformableBrushOutput(CBrush * pBrush, CRenderObject * pObj, i32 nLod, const SRenderingPassInfo &passInfo);

	JobTypeT type;
	union
	{
		//VEGETATION,ROAD_DECALS,COMMON Data
		struct
		{
			IRenderNode*             pObj;
			PodArray<SRenderLight*>* pAffectingLights;
			SSectorTextureSet*       pTerrainTexInfo;
			float                    fEntDistance;
			bool                     bCheckPerObjectOcclusion;
		} common;

		//TERRAIN Data
		struct
		{
			CTerrainNode* pTerrainNode;
		} terrain;

		//DEFORMABLE_BRUSH Data
		struct
		{
			CBrush*        pBrush;
			CRenderObject* pRenderObject;
			i32            nLod;
		} deformable_brush;

		//BRUSH Data
		struct
		{
			CBrush*        pBrush;
			CRenderObject* pRenderObject;
			i16          nLodA;
			i16          nLodB;
			u8          nDissolveRef;
		} brush;
	};

	Vec3 vAmbColor;
	AABB objBox;
	SRendItemSorter rendItemSorter;
	u32 passCullMask;
};

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionOutput SCheckOcclusionOutput::CreateDecalsAndRoadsOutput(IRenderNode* pObj, PodArray<SRenderLight*>* pAffectingLights, const Vec3& rAmbColor, const AABB& rObjBox, float fEntDistance, bool bCheckPerObjectOcclusion, const SRenderingPassInfo& passInfo)
{
	SCheckOcclusionOutput outputData;
	outputData.type = ROAD_DECALS;
	outputData.rendItemSorter = passInfo.GetRendItemSorter();
	outputData.vAmbColor = rAmbColor;
	outputData.objBox = rObjBox;

	outputData.common.pObj = pObj;
	outputData.common.pAffectingLights = pAffectingLights;
	outputData.common.pTerrainTexInfo = NULL;
	outputData.common.fEntDistance = fEntDistance;
	outputData.common.bCheckPerObjectOcclusion = bCheckPerObjectOcclusion;

	return outputData;
}

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionOutput SCheckOcclusionOutput::CreateCommonObjectOutput(IRenderNode* pObj, PodArray<SRenderLight*>* pAffectingLights, const Vec3& rAmbColor, const AABB& rObjBox, float fEntDistance, SSectorTextureSet* pTerrainTexInfo, u32 passCullMask, const SRenderingPassInfo& passInfo)
{
	SCheckOcclusionOutput outputData;
	outputData.type = COMMON;
	outputData.rendItemSorter = passInfo.GetRendItemSorter();
	outputData.vAmbColor = rAmbColor;
	outputData.objBox = rObjBox;
	outputData.passCullMask = passCullMask;
	outputData.common.pObj = pObj;
	outputData.common.pAffectingLights = pAffectingLights;
	outputData.common.fEntDistance = fEntDistance;
	outputData.common.pTerrainTexInfo = pTerrainTexInfo;
	return outputData;
}

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionOutput SCheckOcclusionOutput::CreateTerrainOutput(CTerrainNode* pTerrainNode, u32 passCullMask, const SRenderingPassInfo& passInfo)
{
	SCheckOcclusionOutput outputData;
	outputData.type = TERRAIN;
	outputData.rendItemSorter = passInfo.GetRendItemSorter();
	outputData.passCullMask = passCullMask;
	outputData.terrain.pTerrainNode = pTerrainNode;

	return outputData;
}

///////////////////////////////////////////////////////////////////////////////
inline SCheckOcclusionOutput SCheckOcclusionOutput::CreateDeformableBrushOutput(CBrush* pBrush, CRenderObject* pRenderObject, i32 nLod, const SRenderingPassInfo& passInfo)
{
	SCheckOcclusionOutput outputData;
	outputData.type = DEFORMABLE_BRUSH;
	outputData.rendItemSorter = passInfo.GetRendItemSorter();

	outputData.deformable_brush.pBrush = pBrush;
	outputData.deformable_brush.nLod = nLod;
	outputData.deformable_brush.pRenderObject = pRenderObject;

	return outputData;
}

///////////////////////////////////////////////////////////////////////////////
enum EOcTeeNodeListType
{
	eMain,
	eCasters,
	eSprites,
	eLights,
};

template<class T> struct TDoublyLinkedList
{
	T* m_pFirstNode, * m_pLastNode;

	TDoublyLinkedList()
	{
		m_pFirstNode = m_pLastNode = NULL;
	}

	~TDoublyLinkedList()
	{
		assert(!m_pFirstNode && !m_pLastNode);
	}

	void insertAfter(T* pAfter, T* pObj)
	{
		pObj->m_pPrev = pAfter;
		pObj->m_pNext = pAfter->m_pNext;
		if (pAfter->m_pNext == NULL)
			m_pLastNode = pObj;
		else
			pAfter->m_pNext->m_pPrev = pObj;
		pAfter->m_pNext = pObj;
	}

	void insertBefore(T* pBefore, T* pObj)
	{
		pObj->m_pPrev = pBefore->m_pPrev;
		pObj->m_pNext = pBefore;
		if (pBefore->m_pPrev == NULL)
			m_pFirstNode = pObj;
		else
			pBefore->m_pPrev->m_pNext = pObj;
		pBefore->m_pPrev = pObj;
	}

	void insertBeginning(T* pObj)
	{
		if (m_pFirstNode == NULL)
		{
			m_pFirstNode = pObj;
			m_pLastNode = pObj;
			pObj->m_pPrev = NULL;
			pObj->m_pNext = NULL;
		}
		else
			insertBefore(m_pFirstNode, pObj);
	}

	void insertEnd(T* pObj)
	{
		if (m_pLastNode == NULL)
			insertBeginning(pObj);
		else
			insertAfter(m_pLastNode, pObj);
	}

	void remove(T* pObj)
	{
		if (pObj->m_pPrev == NULL)
			m_pFirstNode = pObj->m_pNext;
		else
			pObj->m_pPrev->m_pNext = pObj->m_pNext;

		if (pObj->m_pNext == NULL)
			m_pLastNode = pObj->m_pPrev;
		else
			pObj->m_pNext->m_pPrev = pObj->m_pPrev;

		pObj->m_pPrev = pObj->m_pNext = NULL;
	}

	bool empty() const { return m_pFirstNode == NULL; }
};

struct SInstancingInfo
{
	SInstancingInfo() { pStatInstGroup = 0; aabb.Reset(); fMinSpriteDistance = 10000.f; bInstancingInUse = 0; }
	StatInstGroup*                                        pStatInstGroup;
	DynArray<CVegetation*>                                arrInstances;
	stl::aligned_vector<CRenderObject::SInstanceInfo, 16> arrMats;
	stl::aligned_vector<SVegetationSpriteInfo, 16>        arrSprites;
	AABB  aabb;
	float fMinSpriteDistance;
	bool  bInstancingInUse;
};

struct SLayerVisibility
{
	u8k*  pLayerVisibilityMask;
	u16k* pLayerIdTranslation;
};

struct SOctreeLoadObjectsData
{
	COctreeNode*             pNode;
	ptrdiff_t                offset;
	size_t                   size;
	_smart_ptr<IMemoryBlock> pMemBlock;
	byte*                    pObjPtr;
	byte*                    pEndObjPtr;
};

class COctreeNode : public IOctreeNode, DinrusX3dEngBase, IStreamCallback
{
public:

	struct ShadowMapFrustumParams
	{
		SRenderLight*             pLight;
		struct ShadowMapFrustum*  pFr;
		PodArray<SPlaneObject>*   pShadowHull;
		const SRenderingPassInfo* passInfo;
		Vec3                      vCamPos;
		u32                    nRenderNodeFlags;
		bool                      bSun;
	};

	~COctreeNode();
	void                     ResetStaticInstancing();
	bool                     HasChildNodes();
	i32                      CountChildNodes();
	void                     InsertObject(IRenderNode* pObj, const AABB& objBox, const float fObjRadiusSqr, const Vec3& vObjCenter);
	bool                     DeleteObject(IRenderNode* pObj);
	void                     Render_Object_Nodes(bool bNodeCompletelyInFrustum, i32 nRenderMask, const Vec3& vAmbColor, u32 passCullMask, const SRenderingPassInfo& passInfo);
	void                     Render_LightSources(bool bNodeCompletelyInFrustum, const SRenderingPassInfo& passInfo);
	static u32            UpdateCullMask(u32 onePassTraversalFrameId, u32 onePassTraversalShadowCascades, const IRenderNode::RenderFlagsType renderFlags, const SRenderingPassInfo& passInfo, const AABB& nodeBox, const float nodeDistance, const float nodeMaxViewDist, const bool bTestCoverageBuffer,
	                                        bool& bCompletelyInMainFrustum, OcclusionTestClient* occlusionTestClient, u32 passCullMask);
	void                     CheckUpdateStaticInstancing();
	void                     RenderDebug();
	void                     RenderContent(i32 nRenderMask, const Vec3& vAmbColor, u32 passCullMask, const SRenderingPassInfo& passInfo);
	void                     RenderContentJobEntry(i32 nRenderMask, Vec3 vAmbColor, u32 passCullMask, SRenderingPassInfo passInfo);
	void                     RenderVegetations(TDoublyLinkedList<IRenderNode>* lstObjects, u32k passCullMask, i32 nRenderMask, const bool bOcNodeCompletelyInFrustum, PodArray<SRenderLight*>* pAffectingLights, SSectorTextureSet* pTerrainTexInfo, const SRenderingPassInfo& passInfo);
	void                     RenderCommonObjects(TDoublyLinkedList<IRenderNode>* lstObjects, u32k passCullMask, i32 nRenderMask, const Vec3& vAmbColor, const bool bOcNodeCompletelyInFrustum, PodArray<SRenderLight*>* pAffectingLights, SSectorTextureSet* pTerrainTexInfo, const SRenderingPassInfo& passInfo);
	void                     RenderDecalsAndRoads(TDoublyLinkedList<IRenderNode>* lstObjects, u32k passCullMask, i32 nRenderMask, const Vec3& vAmbColor, const bool bOcNodeCompletelyInFrustum, PodArray<SRenderLight*>* pAffectingLights, const SRenderingPassInfo& passInfo);
	void                     RenderBrushes(TDoublyLinkedList<IRenderNode>* lstObjects, u32k passCullMask, const bool bOcNodeCompletelyInFrustum, PodArray<SRenderLight*>* pAffectingLights, SSectorTextureSet* pTerrainTexInfo, const SRenderingPassInfo& passInfo);
	static void              RenderObjectIntoShadowViews(const SRenderingPassInfo& passInfo, float fEntDistance, IRenderNode* pObj, const AABB& objBox, u32k passCullMask);
	static bool              IsShadowCaster(IRenderNode* pObj);
	PodArray<SRenderLight*>* GetAffectingLights(const SRenderingPassInfo& passInfo);
	void                     AddLightSource(SRenderLight* pSource, const SRenderingPassInfo& passInfo);
	void                     CheckInitAffectingLights(const SRenderingPassInfo& passInfo);
	void                     InvalidateCachedShadowData();
	void                     ActivateObjectsLayer(u16 nLayerId, bool bActivate, bool bPhys, IGeneralMemoryHeap* pHeap, const AABB& layerBox);
	void                     GetLayerMemoryUsage(u16 nLayerId, IDrxSizer* pSizer, i32* pNumBrushes, i32* pNumDecals);
	virtual void             MarkAsUncompiled(const ERNListType eListType)       { SetCompiled(eListType, false); }
	void                     MarkAsUncompiled();
	inline bool              IsCompiled(ERNListType eRNListType) const           { return (m_compiledFlag & (1 << eRNListType)) != 0; }
	void                     SetCompiled(ERNListType eRNListType, bool compiled) { m_compiledFlag = (compiled ? (1 << eRNListType) : 0) | (m_compiledFlag & ~(1 << eRNListType)); }
	COctreeNode*             FindNodeContainingBox(const AABB& objBox);
	void                     MoveObjectsIntoList(PodArray<SRNInfo>* plstResultEntities, const AABB* pAreaBox, bool bRemoveObjects = false, bool bSkipDecals = false, bool bSkip_ERF_NO_DECALNODE_DECALS = false, bool bSkipDynamicObjects = false, EERType eRNType = eERType_TypesNum);
	i32                      PhysicalizeInBox(const AABB& bbox);
	i32                      DephysicalizeInBox(const AABB& bbox);
	i32                      PhysicalizeOfType(ERNListType listType, bool bInstant);
	i32                      DePhysicalizeOfType(ERNListType listType, bool bInstant);

#if ENGINE_ENABLE_COMPILATION
	i32 GetData(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
#endif

	const AABB&  GetObjectsBBox() { return m_objectsBox; }
	AABB         GetShadowCastersBox(const AABB* pBBox, const Matrix34* pShadowSpaceTransform);
	void         DeleteObjectsByFlag(i32 nRndFlag);
	void         UnregisterEngineObjectsInArea(const SHotUpdateInfo* pExportInfo, PodArray<IRenderNode*>& arrUnregisteredObjects, bool bOnlyEngineObjects);
	u32       GetLastVisFrameId() { return m_nLastVisFrameId; }
	void         GetObjectsByType(PodArray<IRenderNode*>& lstObjects, EERType objType, const AABB* pBBox, bool* pInstStreamCheckReady = NULL, uint64 dwFlags = ~0, bool bRecursive = true);
	void         GetObjectsByFlags(uint dwFlags, PodArray<IRenderNode*>& lstObjects);

	void         GetNearestCubeProbe(float& fMinDistance, i32& nMaxPriority, CLightEntity*& pNearestLight, const AABB* pBBox);
	void         GetObjects(PodArray<IRenderNode*>& lstObjects, const AABB* pBBox);
	bool         GetShadowCastersTimeSliced(IRenderNode* pIgnoreNode, ShadowMapFrustum* pFrustum, const PodArray<struct SPlaneObject>* pShadowHull, i32 renderNodeExcludeFlags, i32& totalRemainingNodes, i32 nCurLevel, const SRenderingPassInfo& passInfo);
	bool         IsObjectTypeInTheBox(EERType objType, const AABB& WSBBox);
	bool         CleanUpTree();
	i32          GetObjectsCount(EOcTeeNodeListType eListType);
	static i32 SaveObjects_CompareRenderNodes(ukk v1, ukk v2);
	i32 SaveObjects(class CMemoryBlock* pMemBlock, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, const SHotUpdateInfo * pExportInfo);
	i32          LoadObjects(byte* pPtr, byte* pEndPtr, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, i32 nChunkVersion, const SLayerVisibility* pLayerVisibility, ELoadObjectsMode eLoadMode);
	static i32   GetSingleObjectFileDataSize(IRenderNode* pObj, const SHotUpdateInfo* pExportInfo);
	static void  SaveSingleObject(byte*& pPtr, i32& nDatanSize, IRenderNode* pObj, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, const SHotUpdateInfo* pExportInfo);
	static void  LoadSingleObject(byte*& pPtr, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, i32 nChunkVersion, const SLayerVisibility* pLayerVisibility, ELoadObjectsMode eLoadMode, IRenderNode*& pRN);
	static bool  IsObjectStreamable(EERType eType, uint64 dwRndFlags);
	static bool  CheckSkipLoadObject(EERType eType, uint64 dwRndFlags, ELoadObjectsMode eLoadMode);
	bool         IsRightNode(const AABB& objBox, const float fObjRadius, float fObjMaxViewDist);
	void         GetMemoryUsage(IDrxSizer* pSizer) const;
	void         UpdateTerrainNodes(CTerrainNode* pParentNode = 0);

	template<class T>
	i32         Load_T(T*& f, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, AABB* pBox, const SLayerVisibility* pLayerVisibility);
	i32         Load(FILE*& f, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, AABB* pBox, const SLayerVisibility* pLayerVisibility);
	i32         Load(u8*& f, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, AABB* pBox, const SLayerVisibility* pLayerVisibility);
	bool        StreamLoad(u8* pData, i32 nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, AABB* pBox);

	static void FreeLoadingCache();
	void        GenerateStatObjAndMatTables(std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, SHotUpdateInfo* pExportInfo);
	static void ReleaseEmptyNodes();
	static void StaticReset();
	bool        IsEmpty();
	bool        HasObjects();

	// used for streaming
	bool                   UpdateStreamingPriority(PodArray<COctreeNode*>& arrRecursion, float fMinDist, float fMaxDist, bool bFullUpdate, const SObjManPrecacheCamera* pPrecacheCams, size_t nPrecacheCams, const SRenderingPassInfo& passInfo);
	float                  GetNodeStreamingDistance(const SObjManPrecacheCamera* pPrecacheCams, AABB objectsBox, size_t nPrecacheCams, const SRenderingPassInfo& passInfo);
	void                   ReleaseStreamableContent();
	bool                   CheckStartStreaming(bool bFullUpdate);
	virtual void           StreamOnComplete(IReadStream* pStream, unsigned nError);
	template<class T> void StreamOnCompleteReadObjects(T* f, i32 nDataSize);
	void                   StartStreaming(bool bFinishNow, IReadStream_AutoPtr* ppStream);
	template<class T> i32  ReadObjects(T*& f, i32& nDataSize, EEndian eEndian, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, const SLayerVisibility* pLayerVisibilityMask, SOcTreeNodeChunk& chunk, ELoadObjectsMode eLoadMode);
	void                   ReleaseObjects(bool bReleaseOnlyStreamable = false);
	void                   GetStreamedInNodesNum(i32& nAllStreamable, i32& nReady);
	static i32             GetStreamingTasksNum()  { return m_nInstStreamTasksInProgress; }
	static i32             GetStreamedInNodesNum() { return m_arrStreamedInNodes.Count(); }
	static void            StreamingCheckUnload(const SRenderingPassInfo& passInfo, PodArray<SObjManPrecacheCamera>& rStreamPreCacheCameras);

	void                   CheckManageVegetationSprites(float fNodeDistance, i32 nMaxFrames, const SRenderingPassInfo& passInfo);
	AABB                   GetNodeBox() const
	{
		return AABB(
		  m_vNodeCenter - m_vNodeAxisRadius,
		  m_vNodeCenter + m_vNodeAxisRadius);
	}

	void                OffsetObjects(const Vec3& offset);
	void                SetVisArea(CVisArea* pVisArea);
	void                SetTerrainNode(struct CTerrainNode* node) { m_pTerrainNode = node; }
	static void         SetTraversalFrameId(IRenderNode* pObj, u32 onePassTraversalFrameId, i32 shadowFrustumLod);
	static COctreeNode* Create(const AABB& box, struct CVisArea* pVisArea, COctreeNode* pParent = NULL);

protected:
	AABB GetChildBBox(i32 nChildId);
	void CompileObjects(ERNListType eListType);
	void UpdateStaticInstancing();
	void UpdateObjects(IRenderNode* pObj);
	void CompileCharacter(ICharacterInstance* pChar, u8& nInternalFlags);

	// Check if min spec specified in render node passes current server config spec.
	static bool CheckRenderFlagsMinSpec(u32 dwRndFlags);

	void        LinkObject(IRenderNode* pObj, EERType eERType, bool bPushFront = true);
	void        UnlinkObject(IRenderNode* pObj);

	static i32  Cmp_OctreeNodeSize(ukk v1, ukk v2);

private:
	COctreeNode(const AABB& box, struct CVisArea* pVisArea, COctreeNode* pParent);

	float        GetNodeRadius2() const { return m_vNodeAxisRadius.Dot(m_vNodeAxisRadius); }
	COctreeNode* FindChildFor(IRenderNode* pObj, const AABB& objBox, const float fObjRadius, const Vec3& vObjCenter);
	bool         HasAnyRenderableCandidates(const SRenderingPassInfo& passInfo) const;
	void         BuildLoadingDatas(PodArray<SOctreeLoadObjectsData>* pQueue, byte* pOrigData, byte*& pData, i32& nDataSize, EEndian eEndian);
	PodArray<SOctreeLoadObjectsData> m_loadingDatas;

	static const float               fMinShadowCasterViewDist;

	bool                             m_streamComplete;

	IRenderNode::RenderFlagsType     m_renderFlags;
	u32                           m_errTypesBitField;
	AABB                             m_objectsBox;
	float                            m_fObjectsMaxViewDist;
	u32                           m_nLastVisFrameId;

	COctreeNode*                     m_arrChilds[8];
	TDoublyLinkedList<IRenderNode>   m_arrObjects[eRNListType_ListsNum];
	Vec3                             m_vNodeCenter;
	Vec3                             m_vNodeAxisRadius;
	PodArray<SRenderLight*>          m_lstAffectingLights;
	u32                           m_nLightMaskFrameId;
	COctreeNode*                     m_pParent;
	float                            m_fNodeDistance;
	i32                              m_nManageVegetationsFrameId;

	OcclusionTestClient              m_occlusionTestClient;

	u32                           m_compiledFlag: eRNListType_ListsNum;
	u32                           m_bHasLights               : 1;
	u32                           m_bHasRoads                : 1;
	u32                           m_bNodeCompletelyInFrustum : 1;
	u32                           m_bStaticInstancingIsDirty : 1;

	// used for streaming
	i32                           m_nFileDataOffset; // TODO: make it 64bit
	i32                           m_nFileDataSize;
	EFileStreamingStatus          m_eStreamingStatus;
	IReadStreamPtr                m_pReadStream;
	i32                           m_nUpdateStreamingPrioriryRoundId;
	static i32                    m_nInstStreamTasksInProgress;
	static FILE*                  m_pFileForSyncRead;
	static PodArray<COctreeNode*> m_arrStreamedInNodes;

	struct SNodeInstancingInfo
	{
		SNodeInstancingInfo() { pRNode = 0; nodeMatrix.IsIdentity(); }
		Matrix34           nodeMatrix;
		class CVegetation* pRNode;
	};
	std::map<std::pair<IStatObj*, IMaterial*>, PodArray<SNodeInstancingInfo>*>* m_pStaticInstancingInfo;

	float m_fPrevTerrainTexScale;                         // used to detect terrain texturing change and refresh info in object instances

public:
	static PodArray<COctreeNode*> m_arrEmptyNodes;
	static i32                    m_nNodesCounterAll;
	static i32                    m_nNodesCounterStreamable;
	static i32                    m_nInstCounterLoaded;

	 i32                  m_updateStaticInstancingLock;
	u32                        m_onePassTraversalFrameId = 0; // Used to request visiting of the node during one-pass traversal
};

#endif
