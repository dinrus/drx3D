// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef heightfieldgeom_h
#define heightfieldgeom_h
#pragma once

#include "trimesh.h"
#include "heightfieldbv.h"

class CHeightfield : public CTriMesh {
public:
	CHeightfield() { m_pTree=&m_Tree; m_minHeight=m_maxHeight=0; m_nVerticesAlloc=m_nTrisAlloc=0; }
	virtual ~CHeightfield() { m_pTree=0; }

	CHeightfield* CreateHeightfield(heightfield *phf);
	virtual i32 GetType() { return GEOM_HEIGHTFIELD; }
	virtual i32 Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts);
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual i32 PointInsideStatus(const Vec3 &pt);
	virtual i32 FindClosestPoint(geom_world_data *pgwd, i32 &iPrim,i32 &iFeature, const Vec3 &ptdst0,const Vec3 &ptdst1,
		Vec3 *ptres, i32 nMaxIters=10);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const Vec3 &epicenter,float k,float rmin,
		const Vec3 &centerOfMass, Vec3 &P,Vec3 &L);
	virtual i32 IsConvex(float tolerance) { return 0; }
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap);
	virtual void PrepareForRayTest(float raylen) {}
	virtual CBVTree *GetBVTree() { return &m_Tree; }

	virtual const primitive *GetData() { return &m_hf; }
	virtual Vec3 GetCenter() { return m_hf.origin+(Vec3(m_hf.size.x,m_hf.size.y,0)*0.5f)*m_hf.Basis; }

	virtual i32 GetPrimitive(i32 iPrim, primitive *pprim) { *(heightfield*)pprim = m_hf; return sizeof(heightfield); }
	virtual void GetMemoryStatistics(IDrxSizer *pSizer);

	heightfield m_hf;
	CHeightfieldBV m_Tree;
	float m_minHeight,m_maxHeight;
	i32 m_nVerticesAlloc,m_nTrisAlloc;
	Vec3 m_lastOriginOffs;
};

#endif
