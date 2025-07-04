// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(FEATURE_SVO_GI)

	#include <drx3D/Plugins/concqueue/concqueue.hpp>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

	#pragma pack(push,4)

//#pragma optimize("",off)

	#define SVO_NODE_BRICK_SIZE        2
	#define SVO_VOX_BRICK_MAX_SIZE     16 // maximum size of voxel brick 3d array data
	#define SVO_BRICK_ALLOC_CHUNK_SIZE 16 // size of allocation granularity
	#define SVO_ATLAS_DIM_MAX_XY       (CVoxelSegment::m_voxTexPoolDimXY / SVO_BRICK_ALLOC_CHUNK_SIZE)
	#define SVO_ATLAS_DIM_MAX_Z        (CVoxelSegment::m_voxTexPoolDimZ / SVO_BRICK_ALLOC_CHUNK_SIZE)
	#define SVO_ATLAS_DIM_BRICKS_XY    (CVoxelSegment::m_voxTexPoolDimXY / SVO_VOX_BRICK_MAX_SIZE)
	#define SVO_ATLAS_DIM_BRICKS_Z     (CVoxelSegment::m_voxTexPoolDimZ / SVO_VOX_BRICK_MAX_SIZE)
	#define SVO_ROOTLESS_PARENT_SIZE   512.f
	#define SVO_STREAM_QUEUE_MAX_SIZE  12

typedef u16 ObjectLayerIdType;
const ObjectLayerIdType kInvalidObjectLayerId = 0;
const ObjectLayerIdType kAllObjectLayersId = kInvalidObjectLayerId;

struct SVoxBrick
{
	enum ESubSetType
	{
		OPA3D,
		COLOR,
		NORML,
		MAX_NUM,
	};

	ColorB* pData[MAX_NUM] = { 0 };
};

template<class T>
class PodArrayRT : public PodArray<T>
{
public:
	PodArrayRT()
	{
		m_bModified = false;
		m_textureId = 0;
	}
	DrxReadModifyLock m_Lock;
	bool              m_bModified;
	i32               m_textureId;
};

template<class Key, class T>
class CLockedMap : public std::map<Key, T>
{
public:
	DrxReadModifyLock m_Lock;
};

struct SObjInfo
{
	SObjInfo() { ZeroStruct(*this); }

	static i32 Compare(ukk v1, ukk v2)
	{
		SObjInfo* p[2] = { (SObjInfo*)v1, (SObjInfo*)v2 };

		if (p[0]->maxViewDist < p[1]->maxViewDist)
			return 1;
		if (p[0]->maxViewDist > p[1]->maxViewDist)
			return -1;

		return 0;
	}

	Matrix34          matObjInv;
	Matrix34          matObj;
	float             objectScale;
	IMaterial*        pMat;
	CStatObj*         pStatObj;
	bool              bIndoor;
	bool              bVegetation;
	ObjectLayerIdType objectLayerId;
	float             maxViewDist;
};

struct SVoxSegmentFileHeader
{
	Vec4_tpl<byte> cropTexSize; // and objLayersNum in w
	Vec4_tpl<byte> cropBoxMin;  // and subSetsNum in w
	Vec4_tpl<byte> dummy;
};

// SSuperMesh index type
	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
typedef u32 SMINDEX;
	#else
typedef u16 SMINDEX;
	#endif

struct SRayHitTriangleIndexed
{
	SRayHitTriangleIndexed() { ZeroStruct(*this); arrVertId[0] = arrVertId[1] = arrVertId[2] = (SMINDEX) ~0; }
	Vec3              vFaceNorm;
	#ifdef FEATURE_SVO_GI_USE_MESH_RT
	uint              globalId;
	#endif
	u8             triArea;
	u8             opacity;
	u8             hitObjectType;
	SMINDEX           arrVertId[3];
	u16            materialID;
	ObjectLayerIdType objectLayerId;
};

struct SRayHitVertex
{
	Vec3   v;
	Vec2   t;
	ColorB c;
};

struct SSuperMesh
{
	SSuperMesh();
	~SSuperMesh();

	struct SSvoMatInfo
	{
		SSvoMatInfo(){ ZeroStruct(*this); }
		IMaterial* pMat;
		ColorB*    pTexRgb;
		u16     textureWidth;
		u16     textureHeight;
		inline bool operator==(const SSvoMatInfo& other) const { return pMat == other.pMat; }
	};

	static i32k hashDim = 8;
	void AddSuperTriangle(SRayHitTriangle& htIn, PodArray<SMINDEX> arrVertHash[hashDim][hashDim][hashDim], ObjectLayerIdType nObjLayerId);
	void AddSuperMesh(SSuperMesh& smIn, float vertexOffset);
	void Clear(PodArray<SMINDEX>* parrVertHash);

	PodArrayRT<SRayHitTriangleIndexed>* m_pTrisInArea;
	PodArrayRT<Vec3>*                   m_pFaceNormals;
	PodArrayRT<SRayHitVertex>*          m_pVertInArea;
	PodArrayRT<SSvoMatInfo>*            m_pMatsInArea;
	AABB                                m_boxTris;

protected:
	i32 FindVertex(const Vec3& rPos, const Vec2 rTC, PodArray<SMINDEX> arrVertHash[hashDim][hashDim][hashDim], PodArrayRT<SRayHitVertex>& vertsInArea);
	i32 AddVertex(const SRayHitVertex& rVert, PodArray<SMINDEX> arrVertHash[hashDim][hashDim][hashDim], PodArrayRT<SRayHitVertex>& vertsInArea);
	bool m_bExternalData;
};

struct SBuildVoxelsParams
{
	i32                  X0;
	i32                  X1;
	PodArray<i32>*       pNodeTrisXYZ;
	PodArray<CVisArea*>* arrPortals;
	PodArray<i32>*       pTrisInt;
};

class CVoxelSegment;

class CVoxStreamEngine
{
public:

	struct SVoxStreamItem
	{
		CVoxelSegment* pObj;
		uint           requestFrameId;
	};

	class CVoxStreamEngineThread final : public IThread
	{
	public:
		CVoxStreamEngineThread(CVoxStreamEngine* pStreamingEngine);
		virtual void ThreadEntry();
		void         SignalStopWork();

	private:
		CVoxStreamEngine* m_pStreamingEngine;
		bool              m_bRun;
	};

	CVoxStreamEngine();
	~CVoxStreamEngine();

	void DecompressVoxStreamItem(const SVoxStreamItem item);
	void ProcessSyncCallBacks();
	bool StartRead(CVoxelSegment* pObj, int64 fileOffset, i32 bytesToRead);

public:
	BoundMPMC<SVoxStreamItem> m_arrForFileRead;
	BoundMPMC<SVoxStreamItem> m_arrForSyncCallBack;
	DrxSemaphore              m_fileReadSemaphore;

private:
	std::vector<CVoxStreamEngineThread*> m_workerThreads;
};

class CVoxelSegment : public DinrusX3dEngBase, public SSuperMesh, public IStreamCallback
{
public:

	CVoxelSegment(class CSvoNode* pNode, bool bDumpToDiskInUse = false, EFileStreamingStatus eStreamingStatus = ecss_Ready, bool bDroppedOnDisk = false);
	~CVoxelSegment();
	static i32   GetSubSetsNum() { return GetCVars()->e_svoTI_IntegrationMode ? SVoxBrick::MAX_NUM : 1;  }
	bool         CheckUpdateBrickRenderData(bool bJustCheck);
	bool         LoadVoxels(byte* pData, i32 size);
	void         SaveVoxels(PodArray<byte>& arrData);
	bool         StartStreaming(CVoxStreamEngine* pVoxStreamEngine);
	bool         UpdateBrickRenderData();
	ColorF       GetBilinearAt(float iniX, float iniY, const ColorB* pImg, i32 dimW, i32 dimH, float multiplier);
	ColorF       GetColorF_255(i32 x, i32 y, const ColorB* pImg, i32 imgSizeW, i32 imgSizeH);
	ColorF       ProcessMaterial(const SRayHitTriangleIndexed& tr, const Vec3& voxBox);
	const AABB&  GetBoxOS()   { return m_boxOS; }
	float        GetBoxSize() { return (m_boxOS.max.z - m_boxOS.min.z); }
	i32          StoreIndicesIntoPool(const PodArray<i32>& nodeTInd, i32& countStored);
	i32        GetID()      { return m_segmentID; }
	static AABB  GetChildBBox(const AABB& parentBox, i32 childId);
	static i32   GetBrickPoolUsageLoadedMB();
	static i32   GetBrickPoolUsageMB();
	static i32 ComparemLastVisFrameID(ukk v1, ukk v2);
	static void  CheckAllocateBrick(ColorB*& pPtr, i32 elems, bool bClean = false);
	static void  CheckAllocateTexturePool();
	static void  FreeBrick(ColorB*& pPtr);
	static void  MakeFolderName(char szFolder[256], bool bCreateDirectory = false);
	static void  SetVoxCamera(const CCamera& newCam);
	static void  UpdateObjectLayersInfo();
	static void  ErrorTerminate(tukk format, ...);
	Vec3i        GetDxtDim();
	void         AddTriangle(const SRayHitTriangleIndexed& ht, i32 trId, PodArray<i32>*& rpNodeTrisXYZ, PodArrayRT<SRayHitVertex>* pVertInArea);
	void         CheckStoreTextureInPool(SShaderItem* pShItem, u16& nTexW, u16& nTexH, i32** ppSysTexId);
	void         ComputeDistancesFast_MinDistToSurf(ColorB* pTex3dOptRGBA, ColorB* pTex3dOptNorm, ColorB* pTex3dOptOpac, i32 threadId);
	void         CropVoxTexture(i32 threadId, bool bCompSurfDist);
	void         DebugDrawVoxels();
	void         FindTrianglesForVoxelization(PodArray<i32>*& rpNodeTrisXYZ);
	static bool  CheckCollectObjectsForVoxelization(const AABB& nodeBox, PodArray<SObjInfo>* parrObjects, bool& bThisIsAreaParent, bool& bThisIsLowLodNode, bool bAllowStartStreaming);
	void         FreeAllBrickData();
	void         FreeBrickLayers();
	void         FreeRenderData();
	void         PropagateDirtyFlag();
	void         ReleaseAtlasBlock();
	void         RenderMesh(PodArray<SVF_P3F_C4B_T2F>& arrVertsOut);
	void         SetBoxOS(const AABB& box) { m_boxOS = box; }
	void         SetID(i32 nID)          { m_segmentID = nID; }
	void         StoreAreaTrisIntoTriPool(PodArray<SRayHitTriangle>& allTrisInLevel);
	void         StreamAsyncOnComplete(IReadStream* pStream, unsigned nError) override;
	void         StreamOnComplete(IReadStream* pStream, unsigned nError) override;
	void         UnloadStreamableData();
	void         UpdateMeshRenderData();
	void         UpdateNodeRenderData();
	void         UpdateVoxRenderData();
	void         VoxelizeMeshes(i32 threadId, bool bMT = false);
	void         CombineLayers();
	void         BuildVoxels(SBuildVoxelsParams params);
	i32          CompressToDxt(ColorB* pImgSource, byte*& pDxtOut, i32 threadId);
	static void  SaveCompTexture(ukk data, size_t size, uk userData);

	AABB                                                 m_boxClipped;
	AABB                                                 m_boxOS;
	bool                                                 m_bStatLightsChanged;
	byte                                                 m_bChildOffsetsDirty;
	class CSvoNode*                                      m_pNode;
	SVoxBrick                                            m_voxData;
	CVoxelSegment*                                       m_pParentCloud;
	EFileStreamingStatus                                 m_eStreamingStatus;
	float                                                m_maxAlphaInBrick;
	i32                                                  m_allocatedAtlasOffset;
	i32                                                  m_fileStreamSize;
	i32                                                m_arrChildOffset[8];
	i32                                                m_segmentID;
	int64                                                m_fileStreamOffset64;
	PodArray<i32>                                        m_nodeTrisAllMerged;

	static CCamera                                       m_voxCam;
	static class CBlockPacker3D*                         m_pBlockPacker;
	static DrxCriticalSection                            m_csLockBrick;
	static i32                                           m_addPolygonToSceneCounter;
	static i32                                           m_checkReadyCounter;
	static i32                                           m_segmentsCounter;
	static i32                                           m_postponedCounter;
	static i32                                           m_currPassMainFrameID;
	static i32                                           m_maxBrickUpdates;
	static i32                                           m_nextSegmentId;
	static i32                                           m_poolUsageBytes;
	static i32                                           m_poolUsageItems;
	static i32                                           m_svoDataPoolsCounter;
	static i32                                           m_voxTrisCounter;
	static i32                                           m_voxTexPoolDimXY;
	static i32                                           m_voxTexPoolDimZ;
	static i32                                         m_streamingTasksInProgress;
	static bool                                          m_bUpdateBrickRenderDataPostponed;
	static i32                                         m_updatesInProgressBri;
	static i32                                         m_updatesInProgressTex;
	static PodArray<CVoxelSegment*>                      m_arrLoadedSegments;
	static SRenderingPassInfo*                           m_pCurrPassInfo;
	static CLockedMap<ITexture*, _smart_ptr<ITexture>>   m_arrLockedTextures;
	static CLockedMap<IMaterial*, _smart_ptr<IMaterial>> m_arrLockedMaterials;
	static std::map<CStatObj*, float>                    m_cgfTimeStats;
	static DrxReadModifyLock                             m_cgfTimeStatsLock;
	static bool                                          m_bExportMode;
	static bool                                          m_bExportAbortRequested;
	static i32                                           m_exportVisitedAreasCounter;
	static i32                                           m_exportVisitedNodesCounter;
	static PodArray<C3DEngine::SLayerActivityInfo>       m_arrObjectLayersInfo;
	static uint                                          m_arrObjectLayersInfoVersion;
	struct SBlockMinMax*                                 m_pBlockInfo;
	SVF_P3F_C4B_T2F                                      m_vertForGS;
	uint                                                 m_lastRendFrameId;
	uint                                                 m_lastTexUpdateFrameId;
	u16                                               m_solidVoxelsNum;
	u8                                                m_dwChildTrisTest;
	Vec3i                                                m_vCropBoxMin;
	Vec3                                                 m_vSegOrigin;
	Vec3i                                                m_vCropTexSize;
	Vec3i                                                m_vStaticGeomCheckSumm;
	Vec3i                                                m_vStatLightsCheckSumm;
	DrxReadModifyLock                                    m_superMeshLock;
	std::map<ObjectLayerIdType, SVoxBrick>               m_objLayerMap;
};

inline uint GetCurrPassMainFrameID() { return CVoxelSegment::m_currPassMainFrameID; }

	#pragma pack(pop)

#endif

