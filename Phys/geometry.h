// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef geometry_h
#define geometry_h
#pragma once

#include "overlapchecks.h"

struct tritem {
	i32 itri;
	i32 itri_parent;
	i32 ivtx0;
};
struct vtxitem {
	i32 ivtx;
	i32 id;
	i32 ibuddy[2];
};

struct BBoxExt : BBox {
	box aboxStatic;
};


struct intersData {
	indexed_triangle IdxTriBuf[256];
	i32 IdxTriBufPos;
	cylinder CylBuf[2];
	i32 CylBufPos;
	sphere SphBuf[2];
	i32 SphBufPos;
	box BoxBuf[2];
	i32 BoxBufPos;
	ray RayBuf[2];

	surface_desc SurfaceDescBuf[64];
	i32 SurfaceDescBufPos;

	edge_desc EdgeDescBuf[64];
	i32 EdgeDescBufPos;

	i32 iFeatureBuf[64];
	i32 iFeatureBufPos;

	char IdBuf[128];
	i32 IdBufPos;

	i32 UsedNodesMap[8192];
	i32 UsedNodesMapPos;

	i32 UsedNodesIdx[64];
	i32 UsedNodesIdxPos;

	geom_contact Contacts[64];
	i32 nTotContacts;

	geom_contact_area AreaBuf[32];
	i32 nAreas;
	Vec3 AreaPtBuf[256];
	i32 AreaPrimBuf0[256],AreaFeatureBuf0[256],AreaPrimBuf1[256],AreaFeatureBuf1[256];
	i32 nAreaPt;

	Vec3 BrdPtBuf[2048];
	i32 BrdPtBufPos,BrdPtBufStart;
	i32 BrdiTriBuf[2048][2];
	float BrdSeglenBuf[2048];
	i32 UsedVtxMap[4096];
	i32 UsedTriMap[4096];
	Vec2 PolyPtBuf[1024];
	i32 PolyVtxIdBuf[1024];
	i32 PolyEdgeIdBuf[1024];
	i32 PolyPtBufPos;

	tritem TriQueue[512];
	vtxitem VtxList[512];

	Vec2 BoxCont[8];
	i32 BoxVtxId[8],BoxEdgeId[8];
	char BoxIdBuf[3];
	surface_desc BoxSurfaceBuf[3];
	edge_desc BoxEdgeBuf[3];

	Vec2 CylCont[32];
	i32 CylContId[32];
	char CylIdBuf[1];
	surface_desc CylSurfaceBuf[1];
	edge_desc CylEdgeBuf[1];

	BBox BBoxBuf[128];
	i32 BBoxBufPos;
	BBoxExt BBoxExtBuf[64];
	i32 BBoxExtBufPos;

	BVheightfield BVhf;

	BVvoxelgrid BVvox;

	BVray BVRay;
	COverlapChecker Overlapper;
	 i32 lockIntersect;
};


extern intersData g_idata[];
#define G(vname) (g_idata[iCaller].vname)

i32k k_BBoxBufSize = sizeof(g_idata[0].BBoxBuf)/sizeof(g_idata[0].BBoxBuf[0]);
i32k k_BBoxExtBufSize = sizeof(g_idata[0].BBoxExtBuf)/sizeof(g_idata[0].BBoxExtBuf[0]);

#define g_ContiPart ((i32(*)[2])g_idata+iCaller)
#define g_IdxTriBuf (g_idata[iCaller].IdxTriBuf)
#define g_IdxTriBufPos (g_idata[iCaller].IdxTriBufPos)
#define g_CylBuf (g_idata[pGTest->iCaller].CylBuf)
#define g_CylBufPos (g_idata[pGTest->iCaller].CylBufPos)
#define g_SphBuf (g_idata[pGTest->iCaller].SphBuf)
#define g_SphBufPos (g_idata[pGTest->iCaller].SphBufPos)
#define g_BoxBuf (g_idata[pGTest->iCaller].BoxBuf)
#define g_BoxBufPos (g_idata[pGTest->iCaller].BoxBufPos)
#define g_RayBuf (g_idata[pGTest->iCaller].RayBuf)

#define g_SurfaceDescBuf (g_idata[pGTest->iCaller].SurfaceDescBuf)
#define g_SurfaceDescBufPos (g_idata[pGTest->iCaller].SurfaceDescBufPos)

#define g_EdgeDescBuf (g_idata[pGTest->iCaller].EdgeDescBuf)
#define g_EdgeDescBufPos (g_idata[pGTest->iCaller].EdgeDescBufPos)

#define g_iFeatureBuf (g_idata[pGTest->iCaller].iFeatureBuf)
#define g_iFeatureBufPos (g_idata[pGTest->iCaller].iFeatureBufPos)

#define g_IdBuf (g_idata[pGTest->iCaller].IdBuf)
#define g_IdBufPos (g_idata[pGTest->iCaller].IdBufPos)

#define g_UsedNodesMap (g_idata[pGTest->iCaller].UsedNodesMap)
#define g_UsedNodesMapPos (g_idata[pGTest->iCaller].UsedNodesMapPos)

#define g_UsedNodesIdx (g_idata[pGTest->iCaller].UsedNodesIdx)
#define g_UsedNodesIdxPos (g_idata[pGTest->iCaller].UsedNodesIdxPos)

#define g_AreaBuf (g_idata[iCaller].AreaBuf)
#define g_AreaPtBuf (g_idata[iCaller].AreaPtBuf)
#define g_AreaPrimBuf0 (g_idata[iCaller].AreaPrimBuf0)
#define g_AreaFeatureBuf0 (g_idata[iCaller].AreaFeatureBuf0)
#define g_AreaPrimBuf1 (g_idata[iCaller].AreaPrimBuf1)
#define g_AreaFeatureBuf1 (g_idata[iCaller].AreaFeatureBuf1)

#define g_BrdPtBuf (g_idata[iCaller].BrdPtBuf)
#define g_BrdPtBufStart (g_idata[iCaller].BrdPtBufStart)
#define g_BrdiTriBuf (g_idata[iCaller].BrdiTriBuf)

#define g_BrdSeglenBuf (g_idata[iCaller].BrdSeglenBuf)
#define g_UsedVtxMap (g_idata[pGTest->iCaller].UsedVtxMap)
#define g_UsedTriMap (g_idata[pGTest->iCaller].UsedTriMap)
#define g_PolyPtBuf (g_idata[pGTest->iCaller].PolyPtBuf)
#define g_PolyVtxIdBuf (g_idata[pGTest->iCaller].PolyVtxIdBuf)
#define g_PolyEdgeIdBuf (g_idata[pGTest->iCaller].PolyEdgeIdBuf)
#define g_PolyPtBufPos (g_idata[pGTest->iCaller].PolyPtBufPos)

#define g_TriQueue (g_idata[pGTest->iCaller].TriQueue)
#define g_VtxList (g_idata[pGTest->iCaller].VtxList)

#define g_BoxCont (g_idata[pGTest->iCaller].BoxCont)
#define g_BoxVtxId (g_idata[pGTest->iCaller].BoxVtxId)
#define g_BoxEdgeId (g_idata[pGTest->iCaller].BoxEdgeId)
#define g_BoxIdBuf (g_idata[pGTest->iCaller].BoxIdBuf)
#define g_BoxSurfaceBuf (g_idata[pGTest->iCaller].BoxSurfaceBuf)
#define g_BoxEdgeBuf (g_idata[pGTest->iCaller].BoxEdgeBuf)

#define g_CylCont (g_idata[pGTest->iCaller].CylCont)
#define g_CylContId (g_idata[pGTest->iCaller].CylContId)
#define g_CylIdBuf (g_idata[pGTest->iCaller].CylIdBuf)
#define g_CylSurfaceBuf (g_idata[pGTest->iCaller].CylSurfaceBuf)
#define g_CylEdgeBuf (g_idata[pGTest->iCaller].CylEdgeBuf)

#define g_BBoxBuf (g_idata[iCaller].BBoxBuf)
#define g_BBoxBufPos (g_idata[iCaller].BBoxBufPos)
#define g_BBoxExtBuf (g_idata[iCaller].BBoxExtBuf)
#define g_BBoxExtBufPos (g_idata[iCaller].BBoxExtBufPos)

#define g_BVhf (g_idata[iCaller].BVhf)

#define g_BVvox (g_idata[iCaller].BVvox)

#define g_Contacts (g_idata[iCaller].Contacts)
#define g_maxContacts (DRX_ARRAY_COUNT(g_Contacts))
#define g_nTotContacts (g_idata[iCaller].nTotContacts)
#define g_BrdPtBufPos (g_idata[iCaller].BrdPtBufPos)
#define g_nAreas (g_idata[iCaller].nAreas)
#define g_nAreaPt (g_idata[iCaller].nAreaPt)
#define g_BVray (g_idata[iCaller].BVRay)

extern  i32 *g_pLockIntersect;

/*extern indexed_triangle g_IdxTriBuf[256];
extern i32 g_IdxTriBufPos;
extern cylinder g_CylBuf[2];
extern i32 g_CylBufPos;
extern sphere g_SphBuf[2];
extern i32 g_SphBufPos;
extern box g_BoxBuf[2];
extern i32 g_BoxBufPos;
extern ray g_RayBuf[2];

extern surface_desc g_SurfaceDescBuf[64];
extern i32 g_SurfaceDescBufPos;

extern edge_desc g_EdgeDescBuf[64];
extern i32 g_EdgeDescBufPos;

extern i32 g_iFeatureBuf[64];
extern i32 g_iFeatureBufPos;

extern char g_IdBuf[256];
extern i32 g_IdBufPos;

extern i32 g_UsedNodesMap[8192];
extern i32 g_UsedNodesMapPos;
extern i32 g_UsedNodesIdx[64];
extern i32 g_UsedNodesIdxPos;

extern geom_contact g_Contacts[64];
extern i32 g_nTotContacts;

extern geom_contact_area g_AreaBuf[32];
extern i32 g_nAreas;
extern Vec3 g_AreaPtBuf[256];
extern i32 g_AreaPrimBuf0[256],g_AreaFeatureBuf0[256],g_AreaPrimBuf1[256],g_AreaFeatureBuf1[256];
extern i32 g_nAreaPt;

// trimesh-related
extern Vec3 g_BrdPtBuf[2048];
extern i32 g_BrdPtBufPos,g_BrdPtBufStart;
extern i32 g_BrdiTriBuf[2048][2];
extern float g_BrdSeglenBuf[2048];
extern i32 g_UsedVtxMap[4096];
extern i32 g_UsedTriMap[4096];
extern Vec2 g_PolyPtBuf[1024];
extern i32 g_PolyVtxIdBuf[1024];
extern i32 g_PolyEdgeIdBuf[1024];
extern i32 g_PolyPtBufPos;

extern tritem g_TriQueue[512];
extern vtxitem g_VtxList[512];

// boxgeom-related
extern Vec2 g_BoxCont[8];
extern i32 g_BoxVtxId[8],g_BoxEdgeId[8];
extern char g_BoxIdBuf[3];
extern surface_desc g_BoxSurfaceBuf[3];
extern edge_desc g_BoxEdgeBuf[3];

// cylindergeom-related
extern Vec2 g_CylCont[64];
extern i32 g_CylContId[64];
extern char g_CylIdBuf[1];
extern surface_desc g_CylSurfaceBuf[1];
extern edge_desc g_CylEdgeBuf[1];

extern BBox g_BBoxBuf[128];
extern i32 g_BBoxBufPos;
extern BBoxExt g_BBoxExtBuf[64];
extern i32 g_BBoxExtBufPos;

// heightfieldbv-related
extern BVheightfield g_BVhf;

// voxelbv-related
extern BVvoxelgrid g_BVvox;

*/

inline void ResetGlobalPrimsBuffers(i32 iCaller=0)
{
	g_BBoxBufPos = 0;
	G(IdxTriBufPos) = 0;
	G(CylBufPos) = 0;
	G(BoxBufPos) = 0;
	G(SphBufPos) = 0;
}


class CGeometry : public IGeometry {
public:
	CGeometry() { m_bIsConvex=0; m_pForeignData=0; m_iForeignData=0; m_nRefCount=1; m_lockUpdate=-1; m_minVtxDist=0; m_iCollPriority=0; }
	virtual ~CGeometry() {
    if (m_iForeignData==DATA_OWNED_OBJECT && m_pForeignData) ((IOwnedObject*)m_pForeignData)->Release();
  }
	virtual i32 GetType() = 0;
	virtual i32 AddRef() { return DrxInterlockedIncrement(&m_nRefCount); }
	virtual void Release();
	virtual void GetBBox(box *pbox) = 0;
	virtual i32 Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual i32 IntersectLocked(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts,
		WriteLockCond &lock,i32 iCaller);
	virtual i32 CalcPhysicalProperties(phys_geometry *pgeom) { return 0; }
	virtual i32 FindClosestPoint(geom_world_data *pgwd, i32 &iPrim,i32 &iFeature, const Vec3 &ptdst0,const Vec3 &ptdst1,
		Vec3 *ptres, i32 nMaxIters=10) { return 0; }
	virtual i32 PointInsideStatus(const Vec3 &pt) { return -1; }
	virtual void DrawWireframe(IPhysRenderer *pRenderer, geom_world_data *gwd, i32 iLevel, i32 idxColor) { pRenderer->DrawGeometry(this,gwd,idxColor); }
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const Vec3 &epicenter,float k,float rmin,
		const Vec3 &centerOfMass, Vec3 &P,Vec3 &L) {}
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, Vec3 &massCenter) { massCenter.zero(); return 0; }
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, Vec3 &dPres,Vec3 &dLres) { dPres.zero(); dLres.zero(); }
	virtual i32 GetPrimitiveId(i32 iPrim,i32 iFeature) { return -1; }
	virtual i32 GetForeignIdx(i32 iPrim) { return -1; }
	virtual Vec3 GetNormal(i32 iPrim, const Vec3 &pt) { return Vec3(0,0,1); }
	virtual i32 IsConvex(float tolerance) { return 1; }
	virtual void PrepareForRayTest(float raylen) {}
	virtual CBVTree *GetBVTree() { return 0; }
	virtual i32 GetPrimitiveCount() { return 1; }
	virtual i32 IsAPrimitive() { return 0; }
	virtual i32 PreparePrimitive(geom_world_data *pgwd,primitive *&pprim,i32 iCaller=0) { return -1; }
	virtual i32 GetFeature(i32 iPrim,i32 iFeature, Vec3 *pt) { return 0; }
	virtual i32 UnprojectSphere(Vec3 center,float r,float rsep, contact *pcontact) { return 0; }
	virtual i32 Subtract(IGeometry *pGeom, geom_world_data *pdata1,geom_world_data *pdata2, i32 bLogUpdates=1) { return 0; }
	virtual i32 GetSubtractionsCount() { return 0; }
	virtual void SetData(const primitive*) {}
	virtual IGeometry *GetTriMesh(i32 bClone=1) { return 0; }
	i32 SphereCheck(const Vec3 &center, float r, i32 iCaller=get_iCaller());
	inline i32 IntersectQueued(IGeometry *piCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pip, geom_contact *&pcontacts,
		uk pent0,uk pent1,i32 ipart0,i32 ipart1)
	{ return Intersect(piCollider,pdata1,pdata2,pip,pcontacts); }

	virtual float GetVolume() { return 0; }
	virtual Vec3 GetCenter() { return Vec3(ZERO); }
	virtual uk GetForeignData(i32 iForeignData=0) { return iForeignData==m_iForeignData ? m_pForeignData:0; }
	virtual i32 GetiForeignData() { return m_iForeignData; }
	virtual void SetForeignData(uk pForeignData, i32 iForeignData) { m_pForeignData=pForeignData; m_iForeignData=iForeignData; }

	virtual float BuildOcclusionCubemap(geom_world_data *pgwd, i32 iMode, SOcclusionCubeMap* grid0, SOcclusionCubeMap* grid1, i32 nGrow);
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap) { return 0; }


	virtual void GetMemoryStatistics(IDrxSizer *pSizer) {}
	virtual void Save(CMemStream &stm) {}
	virtual void Load(CMemStream &stm) {}
	virtual void Load(CMemStream &stm, strided_pointer<const Vec3> pVertices, strided_pointer<unsigned short> pIndices, char *pIds) { Load(stm); }
	virtual i32 GetErrorCount() { return 0; }
	virtual i32 GetSizeFast() { return 0; }
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false)
	{ return 1; };
	virtual i32 RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2,
		prim_inters *pinters);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest) {}
	virtual i32 GetPrimitiveList(i32 iStart,i32 nPrims, i32 typeCollider,primitive *pCollider,i32 bColliderLocal, geometry_under_test *pGTest,
		geometry_under_test *pGTestOp, primitive *pRes,char *pResId) { return 0; }
	virtual i32 GetUnprojectionCandidates(i32 iop,const contact *pcontact, primitive *&pprim,i32 *&piFeature, geometry_under_test *pGTest) {
		pGTest->nSurfaces=pGTest->nEdges = 0; return 0;
	}
	virtual i32 PreparePolygon(coord_plane *psurface, i32 iPrim,i32 iFeature, geometry_under_test *pGTest, Vec2 *&ptbuf,
		i32 *&pVtxIdBuf,i32 *&pEdgeIdBuf) { return 0; }
	virtual i32 PreparePolyline(coord_plane *psurface, i32 iPrim,i32 iFeature, geometry_under_test *pGTest, Vec2 *&ptbuf,
		i32 *&pVtxIdBuf,i32 *&pEdgeIdBuf) { return 0; }

	virtual void Lock(i32 bWrite=1) {
		if (m_lockUpdate>>31 & bWrite)
			m_lockUpdate = 0;
		if (bWrite) {
			m_lockUpdate += isneg(m_lockUpdate);
			SpinLock(&m_lockUpdate,0,WRITE_LOCK_VAL);
		} else if (m_lockUpdate>=0) {
			AtomicAdd(&m_lockUpdate,1);
			 char *pw=( tuk)&m_lockUpdate+(1+eBigEndian); for(;*pw;);
		}
	}
	virtual void Unlock(i32 bWrite=1) { AtomicAdd(&m_lockUpdate,-1-((WRITE_LOCK_VAL-1)&-bWrite) & ~(m_lockUpdate>>31)); }

	virtual void DestroyAuxilaryMeshData(i32) {}
	virtual void RemapForeignIdx(i32 *pCurForeignIdx, i32 *pNewForeignIdx, i32 nTris) {}
	virtual void AppendVertices(Vec3 *pVtx,i32 *pVtxMap, i32 nVtx) {}
	virtual float GetExtent(EGeomForm eForm) const;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;
	virtual void CompactMemory() { }
	virtual i32 Boxify(primitives::box *pboxes,i32 nMaxBoxes, const SBoxificationParams &params) { return 0; }
	virtual i32 Proxify(IGeometry **&pOutGeoms, SProxifyParams *pparams=0) { return 0; }
	virtual i32 SanityCheck() { return 1; }

	 i32 m_lockUpdate;
	 i32 m_nRefCount;
	float m_minVtxDist;
	i32 m_iCollPriority;
	i32 m_bIsConvex;
	uk m_pForeignData;
	i32 m_iForeignData;
};

class CPrimitive : public CGeometry {
public:
	CPrimitive() { m_bIsConvex=1; }
	virtual i32 Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual i32 IsAPrimitive() { return 1; }
};

void DrawBBox(IPhysRenderer *pRenderer,i32 idxColor, geom_world_data *gwd, CBVTree *pTree,BBox *pbbox,i32 maxlevel,i32 level=0,i32 iCaller=0);
i32 SanityCheckTree(CBVTree* pBVtree, i32 maxDepth);

struct InitGeometryGlobals { InitGeometryGlobals(); };

#endif
