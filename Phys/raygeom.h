// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef raygeom_h
#define raygeom_h
#pragma once

class CRayGeom : public CGeometry {
public:
	CRayGeom() { m_iCollPriority=0; m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f; }
	CRayGeom(const ray *pray) {
		m_iCollPriority=0; m_ray.origin = pray->origin; m_ray.dir = pray->dir; m_dirn = pray->dir.normalized();
		m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f;
	}
	CRayGeom(const Vec3 &origin, const Vec3 &dir) {
		m_iCollPriority=0; m_ray.origin = origin; m_ray.dir = dir; m_dirn = dir.normalized();
		m_Tree.Build(this); m_Tree.SetRay(&m_ray); m_minVtxDist=1.0f;
	}
	ILINE CRayGeom *CreateRay(const Vec3 &origin, const Vec3 &dir, const Vec3 *pdirn=0) {
		Vec3 *pdirnLocal = (Vec3*)pdirn;
    if (pdirnLocal) m_dirn = *pdirn; else m_dirn = dir.normalized();
		m_ray.origin = origin; m_ray.dir = dir;  return this;
	}
	void PrepareRay(ray *pray, geometry_under_test *pGTest);

	virtual i32 GetType() { return GEOM_RAY; }
	virtual i32 IsAPrimitive() { return 1; }
	virtual void SetData(const primitive* pray) { CreateRay(((ray*)pray)->origin, ((ray*)pray)->dir); }

	virtual void GetBBox(box *pbox);
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts);
  virtual i32 RegisterIntersection(primitive *pprim1,primitive *pprim2, geometry_under_test *pGTest1,geometry_under_test *pGTest2,
		prim_inters *pinters);
	virtual i32 GetPrimitiveList(i32 iStart,i32 nPrims, i32 typeCollider,primitive *pCollider,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,char *pResId);
	virtual i32 GetUnprojectionCandidates(i32 iop,const contact *pcontact, primitive *&pprim,i32 *&piFeature, geometry_under_test *pGTest);
	virtual i32 PreparePrimitive(geom_world_data *pgwd,primitive *&pprim,i32 iCaller=0);
	virtual CBVTree *GetBVTree() { return &m_Tree; }
	virtual i32 GetPrimitive(i32 iPrim, primitive *pprim) { *(ray*)pprim = m_ray; return sizeof(ray); }
	virtual void PrepareForRayTest(float raylen) { m_dirn = m_ray.dir.normalized(); }

	virtual const primitive *GetData() { return &m_ray; }
	virtual Vec3 GetCenter() { return m_ray.origin+m_ray.dir*0.5f; }
	virtual i32 GetSizeFast() { return sizeof(*this); }

	ray m_ray;
	Vec3 m_dirn;
	CRayBV m_Tree;
};

#endif
