// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef trimesh_h
#define trimesh_h
#pragma once

#include <drx3D/CoreX/Math/GeomQuery.h>

i32k	 mesh_force_AABB = 0x10000000;
i32k  mesh_shared_topology = 0x20000000;
i32k  mesh_should_die = 0x40000000;

struct border_trace {
	Vec3 *pt;
	i32 (*itri)[2];
	float *seglen;
	i32 npt,szbuf;

	Vec3 pt_end;
	i32 itri_end;
	i32 iedge_end;
	float end_dist2;
	i32 bExactBorder;

	i32 iMark,nLoop;

	Vec3r n_sum[2];
	Vec3 n_best;
	i32 ntris[2];
};

struct tri_flags {
	u32 inext : 16;
	u32 iprev : 15;
	u32 bFree : 1;
};

class CTriMesh : public CGeometry {
public:
	static void CleanupGlobalLoadState();

public:
	CTriMesh();
	virtual ~CTriMesh();

	CTriMesh* CreateTriMesh(strided_pointer<const Vec3> pVertices,strided_pointer<unsigned short> pIndices,char *pMats,i32 *pForeignIdx,i32 nTris,
		i32 flags, i32 nMinTrisPerNode=2,i32 nMaxTrisPerNode=4, float favorAABB=1.0f);
	CTriMesh *Clone(CTriMesh *src,i32 flags);
	virtual i32 GetType() { return GEOM_TRIMESH; }
	virtual i32 Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual i32 Subtract(IGeometry *pGeom, geom_world_data *pdata1,geom_world_data *pdata2, i32 bLogUpdates=1);
	virtual i32 Slice(const triangle *pcut, float minlen=0, float minArea=0);
	virtual void GetBBox(box *pbox) { ReadLockCond lock(m_lockUpdate,isneg(m_lockUpdate)^1); m_pTree->GetBBox(pbox); }
	virtual void GetBBox(box *pbox, i32 bThreadSafe) { ReadLockCond lock(m_lockUpdate,(bThreadSafe|isneg(m_lockUpdate))^1); m_pTree->GetBBox(pbox); }
	virtual i32 FindClosestPoint(geom_world_data *pgwd, i32 &iPrim,i32 &iFeature, const Vec3 &ptdst0,const Vec3 &ptdst1,
		Vec3 *ptres, i32 nMaxIters=10);
	virtual i32 CalcPhysicalProperties(phys_geometry *pgeom);
	virtual i32 PointInsideStatus(const Vec3 &pt) { return PointInsideStatusMesh(pt,0); }
	virtual i32 PointInsideStatusMesh(const Vec3 &pt,indexed_triangle *pHitTri);
	virtual void DrawWireframe(IPhysRenderer *pRenderer,geom_world_data *gwd, i32 iLevel, i32 idxColor);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const Vec3 &epicenter,float k,float rmin,
		const Vec3 &centerOfMass, Vec3 &P,Vec3 &L);
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, Vec3 &massCenter);
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, Vec3 &dPres,Vec3 &dLres);
	i32 FloodFill(const Vec3& org, float &V, const Vec3& gravity, float e, Vec2* &ptborder,QuatT &qtBorder,float &depth,i32 bConvexBorder,
		IGeometry **pFloaters,QuatTS *qtsFloaters,i32 nFloaters, float &Vcombined,Vec3 &comCombined);
	void Flip();
	float ComputeVesselVolume(const plane *pplane, IGeometry **pFloaters,QuatTS *qtsFloaters,i32 nFloaters, i32 ihash, index_t *pIndices,Vec3 *pNormals, float dim, float &Vsum,Vec3 &comSum);
	virtual i32 GetPrimitiveId(i32 iPrim,i32 iFeature) { return m_pIds ? m_pIds[iPrim]:-1; }
	virtual i32 GetForeignIdx(i32 iPrim) { return m_pForeignIdx ? m_pForeignIdx[iPrim] : -1; }
	virtual Vec3 GetNormal(i32 iPrim, const Vec3 &pt) { return m_pNormals[iPrim]; }
	virtual i32 IsConvex(float tolerance);
	virtual void PrepareForRayTest(float raylen);
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap);
	virtual CBVTree *GetBVTree() { return m_pTree; }
	virtual i32 GetPrimitiveCount() { return m_nTris; }
	virtual i32 GetPrimitive(i32 iPrim, primitive *pprim) {
		if ((u32)iPrim>=(u32)m_nTris)
			return 0;
		((triangle*)pprim)->pt[0] = m_pVertices[m_pIndices[iPrim*3+0]];
		((triangle*)pprim)->pt[1] = m_pVertices[m_pIndices[iPrim*3+1]];
		((triangle*)pprim)->pt[2] = m_pVertices[m_pIndices[iPrim*3+2]];
		((triangle*)pprim)->n = m_pNormals[iPrim];
		return sizeof(triangle);
	}
	virtual i32 GetFeature(i32 iPrim,i32 iFeature, Vec3 *pt);
	virtual i32 PreparePrimitive(geom_world_data *pgwd,primitive *&pprim,i32 iCaller);
	virtual i32 GetSubtractionsCount() { return m_nSubtracts; }
	virtual IGeometry *GetTriMesh(i32 bClone=1) {
		if (!bClone) return this;
		CTriMesh *pClone = new CTriMesh; pClone->Clone(this,0);
		return pClone;
	}
	virtual i32 UnprojectSphere(Vec3 center,float r,float rsep, contact *pcontact);

	virtual float GetExtent(EGeomForm eForm) const;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;

	virtual void GetMemoryStatistics(IDrxSizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm) { Load(stm,0,0,0); }
	virtual void Load(CMemStream &stm, strided_pointer<const Vec3> pVertices, strided_pointer<unsigned short> pIndices, char *pIds);
	virtual void CompactMemory();
	virtual i32 GetErrorCount() { return m_nErrors; }
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual i32 RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2,
		prim_inters *pinters);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest);
	virtual i32 GetPrimitiveList(i32 iStart,i32 nPrims, i32 typeCollider,primitive *pCollider,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,char *pResId);
	virtual i32 GetUnprojectionCandidates(i32 iop,const contact *pcontact, primitive *&pprim,i32 *&piFeature, geometry_under_test *pGTest);
	virtual i32 PreparePolygon(coord_plane *psurface, i32 iPrim,i32 iFeature, geometry_under_test *pGTest, Vec2 *&ptbuf,
		i32 *&pVtxIdBuf,i32 *&pEdgeIdBuf);
	virtual i32 PreparePolyline(coord_plane *psurface, i32 iPrim,i32 iFeature, geometry_under_test *pGTest, Vec2 *&ptbuf,
		i32 *&pVtxIdBuf,i32 *&pEdgeIdBuf);

	i32 GetEdgeByBuddy(i32 itri,i32 itri_buddy) {
		i32 iedge=0,imask;
		imask = m_pTopology[itri].ibuddy[1]-itri_buddy; imask = imask-1>>31 ^ imask>>31; iedge = 1&imask;
		imask = m_pTopology[itri].ibuddy[2]-itri_buddy; imask = imask-1>>31 ^ imask>>31; iedge = iedge&~imask | 2&imask;
		return iedge;
	}
	i32 GetNeighbouringEdgeId(i32 vtxid, i32 ivtx);
	void PrepareTriangle(i32 itri,triangle *ptri, const geometry_under_test *pGTest);
	i32 TraceTriangleInters(i32 iop, primitive *pprims[], i32 idx_buddy,i32 type_buddy, prim_inters *pinters,
													geometry_under_test *pGTest, border_trace *pborder);
	void HashTrianglesToPlane(const coord_plane &hashplane, const Vec2 &hashsize, grid &hashgrid,index_t *&pHashGrid,index_t *&pHashData,float rcellsize=0,
		float pad=1E-5f, const index_t* pIndices=0,i32 nTris=0,i32 maxhash=0);
	i32 CalculateTopology(index_t *pIndices, i32 bCheckOnly=0);
	i32 BuildIslandMap();
	void RebuildBVTree(CBVTree *pRefTree=0);
	void Empty();
	float GetIslandDisk(i32 matid, const Vec3 &ptref, Vec3 &center,Vec3 &normal,float &peakDist);

	CTriMesh *SplitIntoIslands(plane *pGround,i32 nPlanes,i32 bOriginallyMobile);
	i32 FilterMesh(float minlen,float minangle,i32 bLogUpdates=1);

	void CompactTriangleList(i32 *pTriMap, bool bNoRealloc=false);
	void CollapseTriangleToLine(i32 itri,i32 ivtx, i32 *pTriMap, bop_meshupdate *pmu);
	void RecalcTriNormal(i32 i) {
		m_pNormals[i] = (m_pVertices[m_pIndices[i*3+1]]-m_pVertices[m_pIndices[i*3]] ^
										 m_pVertices[m_pIndices[i*3+2]]-m_pVertices[m_pIndices[i*3]]).normalized();
	}

	virtual const primitive *GetData() { return (mesh_data*)&m_pIndices; }
	virtual void SetData(const primitives::primitive*) { for(i32 i=0;i<m_nTris;i++) RecalcTriNormal(i); RebuildBVTree(); MARK_UNUSED m_V,m_center; }
	virtual float GetVolume();
	virtual Vec3 GetCenter();
	virtual uk GetForeignData(i32 iForeignData=0) {
		return iForeignData==DATA_MESHUPDATE ? m_pMeshUpdate : CGeometry::GetForeignData(iForeignData);
	}
	virtual void SetForeignData(uk pForeignData, i32 iForeignData) {
		if (iForeignData==DATA_MESHUPDATE) {
			m_pMeshUpdate = (bop_meshupdate*)pForeignData;
			if (!pForeignData)
				m_iLastNewTriIdx = BOP_NEWIDX0;
		} else
			CGeometry::SetForeignData(pForeignData,iForeignData);
	}
	virtual void RemapForeignIdx(i32 *pCurForeignIdx, i32 *pNewForeignIdx, i32 nTris);
	virtual void AppendVertices(Vec3 *pVtx,i32 *pVtxMap, i32 nVtx);
	virtual void DestroyAuxilaryMeshData(i32 idata) {
		if (idata & mesh_data_materials && m_pIds) { if (!(m_flags & mesh_shared_mats)) delete[] m_pIds; m_pIds=0; }
		if (idata & mesh_data_foreign_idx && m_pForeignIdx) { if (!(m_flags & mesh_shared_foreign_idx)) delete[] m_pForeignIdx; m_pForeignIdx=0; }
		if (idata & mesh_data_vtxmap && m_pVtxMap) { delete[] m_pVtxMap; m_pVtxMap=0; }
	}

	virtual i32 SanityCheck();

	struct voxgrid_surf	{
		voxgrid_surf() : cells(0),vtx(0),norm(0),nbcnt(0),nbidx(0) {}
		void Free() { delete[] cells;cells=0; delete[] vtx;vtx=0; delete[] norm;norm=0; delete[] nbcnt;nbcnt=0; delete[] nbidx;nbidx=0; }
		Vec3i *cells;
		Vec3 *vtx,*norm;
		i32 *nbcnt,*nbidx;
		i32 ncells;
	};
	template<bool rngchk> struct voxgrid_tpl {
		Vec3i sz;
		Vec3 center;
		quaternionf q;
		float celldim,rcelldim;
		i32 *data;
		Vec3i lastHit;
		Vec3i *flipHist;
		i32 nflips;
		i32 bitVis;
		voxgrid_surf surf;
		bool persistent;
		voxgrid_tpl() : lastHit(1<<10),persistent(false),nflips(0),flipHist(0) { data=0; celldim=0; }
		voxgrid_tpl(const Vec3i &dim,Vec3 c=Vec3(ZERO)) : sz(dim),center(c),lastHit(1<<10),persistent(false),nflips(0),flipHist(0) { center=c; celldim=0; Alloc(); }
		~voxgrid_tpl() { if (persistent) Free(); }
		void Alloc() { i32 alloc=sz.GetVolume()*8+1; memset(data=new i32[alloc],0,alloc*sizeof(data[0])); }
		void Free() { delete[] data; data=0; surf.Free(); delete[] flipHist; }
		i32 &operator()(i32 ix,i32 iy,i32 iz) {
			if (rngchk) {
				i32 mask = (ix+sz.x|sz.x-1-ix|iy+sz.y|sz.y-1-iy|iz+sz.z|sz.z-1-iz)>>31;
				return data[(((iz+sz.z)*sz.y*2+iy+sz.y)*sz.x*2+ix+sz.x&~mask) + (sz.GetVolume()*8&mask)];
			} else
				return data[((iz+sz.z)*sz.y*2+iy+sz.y)*sz.x*2+ix+sz.x];
		}
		i32 &operator()(const Vec3i &idx) { return (*this)(idx.x,idx.y,idx.z); }
		Vec3i RayTrace(const Vec3& org, const Vec3& dir);
		void DrawHelpers(IPhysRenderer *pRenderer, const Vec3& pos,const quaternionf& q,float scale, i32 flags);
	};
	typedef voxgrid_tpl<true> voxgrid;
	#define vox_iterate(vox,ix,iy,iz) for((ix)=-(vox).sz.x;(ix)<(vox).sz.x;(ix)++) for((iy)=-(vox).sz.y;(iy)<(vox).sz.y;(iy)++) for((iz)=-(vox).sz.z;(iz)<(vox).sz.z;(iz)++)
	#define vox_iterate1(vox,ic) for((ic).z=-(vox).sz.z+1;(ic).z<(vox).sz.z-1;(ic).z++) for((ic).y=-(vox).sz.y+1;(ic).y<(vox).sz.y-1;(ic).y++) for((ic).x=-(vox).sz.x+1;(ic).x<(vox).sz.x-1;(ic).x++)
	template<bool rngchk> voxgrid_tpl<rngchk> Voxelize(quaternionf q, float celldim, const index_t *pIndices=0, i32 nTris=0, i32 pad=0);
	virtual i32 Boxify(primitives::box *pboxes,i32 nMaxBoxes, const SBoxificationParams &params);
	virtual i32 Proxify(IGeometry **&pOutGeoms, SProxifyParams *pparams=0);
	voxgrid_tpl<false> *m_voxgrid;

	CBVTree *m_pTree;
	index_t *m_pIndices;
	char *m_pIds;
	i32 *m_pForeignIdx;
	strided_pointer<Vec3> m_pVertices;
	Vec3 *m_pNormals;
	i32 *m_pVtxMap;
	trinfo *m_pTopology;
	i32 m_nTris,m_nVertices;
	mesh_island *m_pIslands;
	i32 m_nIslands;
	tri_flags *m_pTri2Island;
	i32 m_flags;
	i32 *m_pIdxNew2Old;
	i32 m_nMaxVertexValency;
	index_t *m_pHashGrid[3],*m_pHashData[3];
	grid m_hashgrid[3];
	i32 m_nHashPlanes;
	 i32 m_lockHash;
	i32 m_bConvex[4];
	float m_ConvexityTolerance[4];
	i32 m_bMultipart;
	float m_V;
	Vec3 m_center;
	mutable CGeomExtents m_Extents;
	i32 m_nErrors;
	bop_meshupdate *m_pMeshUpdate;
	i32 m_iLastNewTriIdx;
	bop_meshupdate_thunk m_refmu;
	i32 m_nMessyCutCount;
	i32 m_nSubtracts;
	u32 *m_pUsedTriMap;

	i32 m_ivtx0;
	i32 ExpandVtxList();
};


struct InitTriMeshGlobals { InitTriMeshGlobals(); };


#endif
