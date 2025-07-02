// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef geoman_h
#define geoman_h
#pragma once

i32k GEOM_CHUNK_SZ = 64;
i32k PHYS_GEOM_VER = 1;

class CTriMesh;
struct SCrackGeom {
	i32 id;
	CTriMesh *pGeom;
	i32 idmat;
	Vec3 pt0;
	Matrix33 Rc;
	float maxedge,rmaxedge;
	float ry3;
	Vec2 pt3;
	Vec3 vtx[3];
};

struct phys_geometry_serialize {
	i32 dummy0;
	Vec3 Ibody;
	quaternionf q;
	Vec3 origin;
	float V;
	i32 nRefCount;
	i32 surface_idx;
	i32 dummy1;
	i32 nMats;

	AUTO_STRUCT_INFO;
};

class CGeomUpr : public IGeomUpr {
public:
	CGeomUpr() { InitGeoman(); }
	~CGeomUpr() { ShutDownGeoman(); }

	void InitGeoman();
	void ShutDownGeoman();

	virtual IGeometry *CreateMesh(strided_pointer<const Vec3> pVertices,strided_pointer<unsigned short> pIndices,char *pMats,i32 *pForeignIdx,i32 nTris,
		i32 flags, float approx_tolerance=0.05f, i32 nMinTrisPerNode=2,i32 nMaxTrisPerNode=4, float favorAABB=1.0f)
	{
		SBVTreeParams params;
		params.nMinTrisPerNode = nMinTrisPerNode; params.nMaxTrisPerNode = nMaxTrisPerNode;
		params.favorAABB = favorAABB;
		return CreateMesh(pVertices,pIndices,pMats,pForeignIdx, nTris, flags & ~mesh_VoxelGrid, approx_tolerance, &params);
	}
	virtual IGeometry *CreateMesh(strided_pointer<const Vec3> pVertices,strided_pointer<unsigned short> pIndices,char *pMats,i32 *pForeignIdx,i32 nTris,
		i32 flags, float approx_tolerance, SMeshBVParams *pParams);
	virtual IGeometry *CreatePrimitive(i32 type, const primitives::primitive *pprim);
	virtual void DestroyGeometry(IGeometry *pGeom);

	virtual phys_geometry *RegisterGeometry(IGeometry *pGeom,i32 defSurfaceIdx=0, i32 *pMatMapping=0,i32 nMats=0);
	virtual i32 AddRefGeometry(phys_geometry *pgeom);
	virtual i32 UnregisterGeometry(phys_geometry *pgeom);
	virtual void SetGeomMatMapping(phys_geometry *pgeom, i32 *pMatMapping, i32 nMats);

	virtual void SaveGeometry(CMemStream &stm, IGeometry *pGeom);
	virtual IGeometry *LoadGeometry(CMemStream &stm, strided_pointer<const Vec3> pVertices, strided_pointer<unsigned short> pIndices, char *pIds);
	virtual void SavePhysGeometry(CMemStream &stm, phys_geometry *pgeom);
	virtual phys_geometry *LoadPhysGeometry(CMemStream &stm, strided_pointer<const Vec3> pVertices, strided_pointer<unsigned short> pIndices, char *pIds);
	virtual IGeometry *CloneGeometry(IGeometry *pGeom);

	virtual ITetrLattice *CreateTetrLattice(const Vec3 *pt,i32 npt, i32k *pTets,i32 nTets);
	virtual i32 RegisterCrack(IGeometry *pGeom, Vec3 *pVtx, i32 idmat);
	virtual void UnregisterCrack(i32 id);
	virtual IGeometry *GetCrackGeom(const Vec3 *pt,i32 idmat, geom_world_data *pgwd);
	virtual void UnregisterAllCracks(void (*OnRemoveGeom)(IGeometry *pGeom)=0) {
		for(i32 i=0;i<m_nCracks;i++) {
			if (OnRemoveGeom) OnRemoveGeom((IGeometry*)m_pCracks[i].pGeom);
			((IGeometry*)m_pCracks[i].pGeom)->Release();
		} m_nCracks=m_idCrack=0;
	}

	virtual IBreakableGrid2d *GenerateBreakableGrid(Vec2 *ptsrc,i32 npt, const Vec2i &nCells, i32 bStaticBorder, i32 seed=-1);
	virtual void ReleaseGeomsImmediately(bool bReleaseImmediately) { m_bReleaseGeomsImmediately = bReleaseImmediately; }

	phys_geometry *GetFreeGeomSlot();
	virtual IPhysicalWorld *GetIWorld() { return 0; }
	void FlushOldGeoms();

	phys_geometry **m_pGeoms;
	i32 m_nGeomChunks,m_nGeomsInLastChunk;
	phys_geometry *m_pFreeGeom;
	i32 m_lockGeoman;

	SCrackGeom *m_pCracks;
	i32 m_nCracks;
	i32 m_idCrack;
	float m_kCrackScale,m_kCrackSkew;
	i32 m_sizeExtGeoms;

	bool m_bReleaseGeomsImmediately;
	class CGeometry *m_oldGeoms;
	 i32 m_lockOldGeoms;
};

#endif
