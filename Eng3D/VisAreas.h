// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   visareas.h
//  Version:     v1.00
//  Created:     18/12/2002 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: visibility areas header
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef VisArea_H
#define VisArea_H

#include "BasicArea.h"
#include <drx3D/Eng3D/PolygonClipContext.h>

// Unique identifier for each VisArea instance
typedef u32 VisAreaId;

typedef uint64       VisAreaGUID;

#define ReservedVisAreaBytes 384

enum EVisAreaColdDataType
{
	eCDT_Generic = 0,
	eCDT_Portal
};

struct SGenericColdData
{
	SGenericColdData()
	{
		m_dataType = eCDT_Generic;
		m_sName[0] = 0;
	}

	ILINE void ResetGenericData()
	{
		m_dataType = eCDT_Generic;
		m_sName[0] = 0;
	}

	EVisAreaColdDataType m_dataType;
	char                 m_sName[64];
};

struct SPortalColdData : public SGenericColdData
{
	SPortalColdData()
	{
		m_dataType = eCDT_Portal;
	}

	ILINE void ResetPortalData()
	{
		m_dataType = eCDT_Portal;
	}

	OcclusionTestClient m_occlusionTestClient;
};

struct CVisArea : public IVisArea, public CBasicArea
{
	static void StaticReset();

	// editor interface
	virtual void Update(const Vec3* pPoints, i32 nCount, tukk szName, const SVisAreaInfo& info);

	CVisArea();
	CVisArea(VisAreaGUID visGUID);
	virtual ~CVisArea();

	bool                    operator==(const CVisArea& v1) const { return &v1 == this; }

	void                    Init();
	ILINE SGenericColdData* GetColdData()                               { return m_pVisAreaColdData; }
	ILINE void              SetColdDataPtr(SGenericColdData* pColdData) { m_pVisAreaColdData = pColdData; }
	bool                    IsSphereInsideVisArea(const Vec3& vPos, const f32 fRadius);
	bool                    IsPointInsideVisArea(const Vec3& vPos) const;
	bool                    IsBoxOverlapVisArea(const AABB& objBox);
	bool                    ClipToVisArea(bool bInside, Sphere& sphere, Vec3 const& vNormal);
	bool                    FindVisArea(IVisArea* pAnotherArea, i32 nMaxRecursion, bool bSkipDisabledPortals);
	bool                    FindVisAreaReqursive(IVisArea* pAnotherArea, i32 nMaxReqursion, bool bSkipDisabledPortals, StaticDynArray<CVisArea*, 1024>& arrVisitedParents);
	bool                    GetDistanceThruVisAreas(AABB vCurBoxIn, IVisArea* pTargetArea, const AABB& targetBox, i32 nMaxReqursion, float& fResDist);
	bool                    GetDistanceThruVisAreasReq(AABB vCurBoxIn, float fCurDistIn, IVisArea* pTargetArea, const AABB& targetBox, i32 nMaxReqursion, float& fResDist, CVisArea* pPrevArea, i32 nCallID);
	void                    FindSurroundingVisArea(i32 nMaxReqursion, bool bSkipDisabledPortals, PodArray<IVisArea*>* pVisitedAreas = NULL, i32 nMaxVisitedAreas = 0, i32 nDeepness = 0);
	void                    FindSurroundingVisAreaReqursive(i32 nMaxReqursion, bool bSkipDisabledPortals, PodArray<IVisArea*>* pVisitedAreas, i32 nMaxVisitedAreas, i32 nDeepness, PodArray<CVisArea*>* pUnavailableAreas);
	i32                     GetVisFrameId();
	Vec3                    GetConnectionNormal(CVisArea* pPortal);
	void                    PreRender(i32 nReqursionLevel, CCamera CurCamera, CVisArea* pParent, CVisArea* pCurPortal, bool* pbOutdoorVisible, PodArray<CCamera>* plstOutPortCameras, bool* pbSkyVisible, bool* pbOceanVisible, PodArray<CVisArea*>& lstVisibleAreas, const SRenderingPassInfo& passInfo);
	void                    UpdatePortalCameraPlanes(CCamera& cam, Vec3* pVerts, bool bMergeFrustums, const SRenderingPassInfo& passInfo);
	i32                     GetVisAreaConnections(IVisArea** pAreas, i32 nMaxConnNum, bool bSkipDisabledPortals = false);
	i32                     GetRealConnections(IVisArea** pAreas, i32 nMaxConnNum, bool bSkipDisabledPortals = false);
	bool                    IsPortalValid();
	bool                    IsPortalIntersectAreaInValidWay(CVisArea* pPortal);
	bool                    IsPortal() const;
	bool                    IsShapeClockwise();
	bool                    IsAffectedByOutLights() const { return m_bAffectedByOutLights; }
	bool                    IsActive()                    { return m_bActive || (GetCVars()->e_Portals == 4); }
	void                    UpdateGeometryBBox();
	void                    UpdateClipVolume();
	void                    UpdatePortalBlendInfo(const SRenderingPassInfo& passInfo);
	void                    DrawAreaBoundsIntoCBuffer(class CCullBuffer* pCBuffer);
	void                    ClipPortalVerticesByCameraFrustum(PodArray<Vec3>* pPolygon, const CCamera& cam);
	void                    GetMemoryUsage(IDrxSizer* pSizer);
	bool                    IsConnectedToOutdoor() const;
	bool                    IsIgnoringGI() const        { return m_bIgnoreGI; }
	bool                    IsIgnoringOutdoorAO() const { return m_bIgnoreOutdoorAO; }

	tukk             GetName()                   { return m_pVisAreaColdData->m_sName; }
#if ENGINE_ENABLE_COMPILATION
	i32                     SaveHeader(byte*& pData, i32& nDataSize);
	i32                     SaveObjetsTree(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo, byte* pHead);
	i32                     GetData(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
	i32                     GetSegmentData(byte*& pData, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
#endif
	template<class T>
	i32                 LoadHeader_T(T*& f, i32& nDataSizeLeft, EEndian eEndian, i32& objBlockSize);
	template<class T>
	i32                 LoadObjectsTree_T(T*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo, i32k objBlockSize);
	template<class T>
	i32                 Load_T(T*& f, i32& nDataSize, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
	i32                 Load(byte*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
	i32                 Load(FILE*& f, i32& nDataSizeLeft, std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
	const AABB*         GetAABBox() const;
	const AABB*         GetStaticObjectAABBox() const;
	void                UpdateOcclusionFlagInTerrain();
	void                AddConnectedAreas(PodArray<CVisArea*>& lstAreas, i32 nMaxRecursion);
	void                GetShapePoints(const Vec3*& pPoints, size_t& nPoints);
	float               GetHeight();
	float               CalcSignedArea();

	bool                CalcPortalBlendPlanes(Vec3 camPos);
	virtual void        GetClipVolumeMesh(_smart_ptr<IRenderMesh>& renderMesh, Matrix34& worldTM) const;
	virtual const AABB& GetClipVolumeBBox() const                       { return *GetStaticObjectAABBox(); }
	virtual u8       GetStencilRef() const                           { return m_nStencilRef; }
	virtual uint        GetClipVolumeFlags() const;
	virtual bool        IsPointInsideClipVolume(const Vec3& vPos) const { return IsPointInsideVisArea(vPos); }

	void                OffsetPosition(const Vec3& delta);
	static VisAreaGUID  GetGUIDFromFile(byte* f, EEndian eEndian);
	VisAreaGUID         GetGUID() const { return m_nVisGUID; }

	const Vec3          GetFinalAmbientColor();

	static PodArray<CVisArea*>     m_lUnavailableAreas;
	static PodArray<Vec3>          s_tmpLstPortVertsClipped;
	static PodArray<Vec3>          s_tmpLstPortVertsSS;
	static PodArray<Vec3>          s_tmpPolygonA;
	static PodArray<IRenderNode*>  s_tmpLstLights;
	static PodArray<CTerrainNode*> s_tmpLstTerrainNodeResult;
	static CPolygonClipContext     s_tmpClipContext;
	static PodArray<CCamera>       s_tmpCameras;
	static i32                     s_nGetDistanceThruVisAreasCallCounter;

	VisAreaGUID                    m_nVisGUID;
	PodArray<CVisArea*>            m_lstConnections;
	Vec3                           m_vConnNormals[2];
	i32                            m_nRndFrameId;
	float                          m_fGetDistanceThruVisAreasMinDistance;
	i32                            m_nGetDistanceThruVisAreasLastCallID;
	float                          m_fPortalBlending;

	PodArray<Vec3>                 m_lstShapePoints;
	float                          m_fHeight;

	_smart_ptr<IRenderMesh>        m_pClipVolumeMesh;

	Vec3                           m_vAmbientColor;
	float                          m_fDistance;
	float                          m_fViewDistRatio;
	CCamera*                       m_arrOcclCamera[MAX_RECURSION_LEVELS];
	i32                            m_lstCurCamerasLen;
	i32                            m_lstCurCamerasCap;
	i32                            m_lstCurCamerasIdx;
	u8                          m_nStencilRef;
	SGenericColdData*              m_pVisAreaColdData;
	bool                           m_bAffectedByOutLights;
	bool                           m_bSkyOnly;
	bool                           m_bOceanVisible;
	bool                           m_bDoubleSide;
	bool                           m_bUseDeepness;
	bool                           m_bUseInIndoors;
	bool                           m_bThisIsPortal;
	bool                           m_bIgnoreSky;
	bool                           m_bActive;
	bool                           m_bIgnoreGI;
	bool                           m_bIgnoreOutdoorAO;
};

struct SAABBTreeNode
{
	SAABBTreeNode(PodArray<CVisArea*>& lstAreas, AABB box, i32 nMaxRecursion = 0);
	~SAABBTreeNode();
	CVisArea*      FindVisarea(const Vec3& vPos);
	SAABBTreeNode* GetTopNode(const AABB& box, uk * pNodeCache);
	bool           IntersectsVisAreas(const AABB& box);
	i32            ClipOutsideVisAreas(Sphere& sphere, Vec3 const& vNormal);
	void           OffsetPosition(const Vec3& delta);

	AABB                nodeBox;
	PodArray<CVisArea*> nodeAreas;
	SAABBTreeNode*      arrChilds[2];
};

struct TTopologicalSorter;

struct CVisAreaSegmentData
{
	// active vis areas in current segment
	std::vector<i32> m_visAreaIndices;
};

struct CVisAreaUpr : public IVisAreaUpr, DinrusX3dEngBase
{
	CVisArea*                   m_pCurArea, * m_pCurPortal;
	PodArray<CVisArea*>         m_lstActiveEntransePortals;

	PodArray<CVisArea*>         m_lstVisAreas;
	PodArray<CVisArea*>         m_lstPortals;
	PodArray<CVisArea*>         m_lstOcclAreas;
	PodArray<CVisArea*>         m_segVisAreas;
	PodArray<CVisArea*>         m_segPortals;
	PodArray<CVisArea*>         m_segOcclAreas;
	PodArray<CVisArea*>         m_lstActiveOcclVolumes;
	PodArray<CVisArea*>         m_lstIndoorActiveOcclVolumes;
	PodArray<CVisArea*>         m_lstVisibleAreas;
	PodArray<CVisArea*>         m_tmpLstUnavailableAreas;
	PodArray<CVisArea*>         m_tmpLstLightBoxAreas;
	bool                        m_bOutdoorVisible;
	bool                        m_bSkyVisible;
	bool                        m_bOceanVisible;
	bool                        m_bSunIsNeeded;
	PodArray<CCamera>           m_lstOutdoorPortalCameras;
	PodArray<IVisAreaCallback*> m_lstCallbacks;
	SAABBTreeNode*              m_pAABBTree;

	CVisAreaUpr();
	~CVisAreaUpr();
	void                 UpdateAABBTree();
	void                 SetCurAreas(const SRenderingPassInfo& passInfo);
	PodArray<CVisArea*>* GetActiveEntransePortals() { return &m_lstActiveEntransePortals; }
	void                 PortalsDrawDebug();
	bool                 IsEntityVisible(IRenderNode* pEnt);
	bool                 IsOutdoorAreasVisible();
	bool                 IsSkyVisible();
	bool                 IsOceanVisible();
	CVisArea*            CreateVisArea(VisAreaGUID visGUID);
	bool                 DeleteVisArea(CVisArea* pVisArea);
	bool                 SetEntityArea(IRenderNode* pEnt, const AABB& objBox, const float fObjRadiusSqr);
	void                 CheckVis(const SRenderingPassInfo& passInfo);
	void                 DrawVisibleSectors(const SRenderingPassInfo& passInfo, u32 passCullMask);
	void                 ActivatePortal(const Vec3& vPos, bool bActivate, tukk szEntityName);
	void                 ActivateOcclusionAreas(IVisAreaTestCallback* pTest, bool bActivate);
	void                 UpdateVisArea(CVisArea* pArea, const Vec3* pPoints, i32 nCount, tukk szName, const SVisAreaInfo& info);
	virtual void         UpdateConnections();
	void                 MoveObjectsIntoList(PodArray<SRNInfo>* plstVisAreasEntities, const AABB* boxArea, bool bRemoveObjects = false);
	IVisArea*            GetVisAreaFromPos(const Vec3& vPos);
	bool                 IntersectsVisAreas(const AABB& box, uk * pNodeCache = 0);
	bool                 ClipOutsideVisAreas(Sphere& sphere, Vec3 const& vNormal, uk pNodeCache = 0);
	bool                 IsEntityVisAreaVisible(IRenderNode* pEnt, i32 nMaxReqursion, const SRenderLight* pLight, const SRenderingPassInfo& passInfo);
	void                 MakeActiveEntransePortalsList(const CCamera* pCamera, PodArray<CVisArea*>& lstActiveEntransePortals, CVisArea* pThisPortal, const SRenderingPassInfo& passInfo);
	void                 MergeCameras(CCamera& cam, const CCamera& camPlus, const SRenderingPassInfo& passInfo);
	void                 DrawOcclusionAreasIntoCBuffer(const SRenderingPassInfo& passInfo);
	bool                 IsValidVisAreaPointer(CVisArea* pVisArea);
	void                 GetStreamingStatus(i32& nLoadedSectors, i32& nTotalSectors);
	void                 GetMemoryUsage(IDrxSizer* pSizer) const;
	bool                 IsOccludedByOcclVolumes(const AABB& objBox, const SRenderingPassInfo& passInfo, bool bCheckOnlyIndoorVolumes = false);
	void                 GetObjectsAround(Vec3 vExploPos, float fExploRadius, PodArray<SRNInfo>* pEntList, bool bSkip_ERF_NO_DECALNODE_DECALS = false, bool bSkipDynamicObjects = false);
	void                 IntersectWithBox(const AABB& aabbBox, PodArray<CVisArea*>* plstResult, bool bOnlyIfVisible);
	template<class T>
	bool                 Load_T(T*& f, i32& nDataSize, struct SVisAreaManChunkHeader* pVisAreaUprChunkHeader, std::vector<struct IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, bool bHotUpdate, SHotUpdateInfo* pExportInfo);
	virtual bool         Load(FILE*& f, i32& nDataSize, struct SVisAreaManChunkHeader* pVisAreaUprChunkHeader, std::vector<struct IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable);
	virtual bool         SetCompiledData(byte* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, bool bHotUpdate, SHotUpdateInfo* pExportInfo);
	virtual bool         GetCompiledData(byte* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, std::vector<struct IStatInstGroup*>** ppStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo);
	virtual i32          GetCompiledDataSize(SHotUpdateInfo* pExportInfo);
	void                 UnregisterEngineObjectsInArea(const SHotUpdateInfo* pExportInfo, PodArray<IRenderNode*>& arrUnregisteredObjects, bool bOnlyEngineObjects);
	void                 PrecacheLevel(bool bPrecacheAllVisAreas, Vec3* pPrecachePoints, i32 nPrecachePointsNum);
	void                 AddLightSource(SRenderLight* pLight, const SRenderingPassInfo& passInfo);
	void                 AddLightSourceReqursive(SRenderLight* pLight, CVisArea* pArea, i32k nDeepness, const SRenderingPassInfo& passInfo);
	bool                 IsEntityVisAreaVisibleReqursive(CVisArea* pVisArea, i32 nMaxReqursion, PodArray<CVisArea*>* pUnavailableAreas, const SRenderLight* pLight, const SRenderingPassInfo& passInfo);
	bool                 IsAABBVisibleFromPoint(AABB& aabb, Vec3 vPos);
	bool                 FindShortestPathToVisArea(CVisArea* pThisArea, CVisArea* pTargetArea, PodArray<CVisArea*>& arrVisitedAreas, i32& nRecursion, const struct Shadowvolume& sv);

	i32                  GetNumberOfVisArea() const;                            // the function give back the accumlated number of visareas and portals
	IVisArea*            GetVisAreaById(i32 nID) const;                         // give back the visarea interface based on the id (0..GetNumberOfVisArea()) it can be a visarea or a portal

	virtual void         AddListener(IVisAreaCallback* pListener);
	virtual void         RemoveListener(IVisAreaCallback* pListener);

	virtual void         CloneRegion(const AABB& region, const Vec3& offset, float zRotation);
	virtual void         ClearRegion(const AABB& region);

	void                 MarkAllSectorsAsUncompiled();
	void                 InitAABBTree();

	// -------------------------------------

	void         GetObjectsByType(PodArray<IRenderNode*>& lstObjects, EERType objType, const AABB* pBBox, bool* pInstStreamReady = NULL, uint64 dwFlags = ~0);
	void         GetObjectsByFlags(uint dwFlags, PodArray<IRenderNode*>& lstObjects);
	i32          GetObjectsCount(EOcTeeNodeListType eListType);
	void         GetStreamedInNodesNum(i32& nAllStreamable, i32& nReady);

	void         GetNearestCubeProbe(float& fMinDistance, i32& nMaxPriority, CLightEntity*& pNearestLight, const AABB* pBBox);

	void         GetObjects(PodArray<IRenderNode*>& lstObjects, const AABB* pBBox);
	CVisArea*    GetCurVisArea() { return m_pCurArea ? m_pCurArea : m_pCurPortal; }
	void         GenerateStatObjAndMatTables(std::vector<IStatObj*>* pStatObjTable, std::vector<IMaterial*>* pMatTable, std::vector<IStatInstGroup*>* pStatInstGroupTable, SHotUpdateInfo* pExportInfo);
	void         OnVisAreaDeleted(IVisArea* pArea);
	void         ActivateObjectsLayer(u16 nLayerId, bool bActivate, bool bPhys, IGeneralMemoryHeap* pHeap, const AABB& layerBox);
	void         PhysicalizeInBox(const AABB&);
	void         DephysicalizeInBox(const AABB&);
	virtual void OffsetPosition(const Vec3& delta);
	void         CleanUpTrees();

private:
	void      DeleteAllVisAreas();

	CVisArea* CreateTypeVisArea();
	CVisArea* CreateTypePortal();
	CVisArea* CreateTypeOcclArea();

	CVisArea* FindVisAreaByGuid(VisAreaGUID guid, PodArray<CVisArea*>& lstVisAreas);
	template<class T>
	void      ResetVisAreaList(PodArray<CVisArea*>& lstVisAreas, PodArray<CVisArea*, ReservedVisAreaBytes>& visAreas, PodArray<T>& visAreaColdData);

	PodArray<CVisArea*, ReservedVisAreaBytes> m_portals;
	PodArray<CVisArea*, ReservedVisAreaBytes> m_visAreas;
	PodArray<CVisArea*, ReservedVisAreaBytes> m_occlAreas;

	PodArray<SGenericColdData>                m_visAreaColdData;
	PodArray<SPortalColdData>                 m_portalColdData;
	PodArray<SGenericColdData>                m_occlAreaColdData;

	PodArray<i32>                             m_arrDeletedVisArea;
	PodArray<i32>                             m_arrDeletedPortal;
	PodArray<i32>                             m_arrDeletedOcclArea;

	struct SActiveVerts
	{
		Vec3 arrvActiveVerts[4];
	};

#if defined(OCCLUSIONCULLER_W)
	std::vector<SActiveVerts> m_allActiveVerts;
#endif
};

#endif // VisArea_H
