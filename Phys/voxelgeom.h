// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef voxelgeom_h
#define voxelgeom_h
#pragma once

class CVoxelGeom : public CTriMesh {
public:
	CVoxelGeom() { m_grid.pCellTris=0; m_grid.pTriBuf=0; }
	virtual ~CVoxelGeom() {
		if (m_grid.pCellTris) delete[] m_grid.pCellTris;
		if (m_grid.pTriBuf) delete[] m_grid.pTriBuf;
		m_pTree=0;
	}

	CVoxelGeom *CreateVoxelGrid(grid3d *pgrid);
	virtual i32 GetType() { return GEOM_VOXELGRID; }
	virtual i32 Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual i32 PointInsideStatus(const Vec3 &pt) { return -1; }
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const Vec3 &epicenter,float k,float rmin,
		const Vec3 &centerOfMass, Vec3 &P,Vec3 &L) {}
	virtual i32 IsConvex(float tolerance) { return 0; }
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap);
	virtual void PrepareForRayTest(float raylen) {}
	virtual CBVTree *GetBVTree() { return &m_Tree; }
	virtual void GetMemoryStatistics(IDrxSizer *pSizer);

	voxelgrid m_grid;
	CVoxelBV m_Tree;
};

#endif
