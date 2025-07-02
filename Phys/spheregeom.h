// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef spheregeom_h
#define spheregeom_h
#pragma once

class CSphereGeom : public CPrimitive {
public:
	CSphereGeom() { m_iCollPriority = 3; m_minVtxDist = 0; }

	CSphereGeom* CreateSphere(sphere *pcyl);

	virtual i32 GetType() { return GEOM_SPHERE; }
	virtual void GetBBox(box *pbox) { m_Tree.GetBBox(pbox); }
	virtual i32 CalcPhysicalProperties(phys_geometry *pgeom);
	virtual i32 FindClosestPoint(geom_world_data *pgwd, i32 &iPrim,i32 &iFeature, const Vec3 &ptdst0,const Vec3 &ptdst1,
		Vec3 *ptres, i32 nMaxIters);
	virtual i32 PointInsideStatus(const Vec3 &pt);
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, Vec3 &massCenter);
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, Vec3 &dPres,Vec3 &dLres);
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const Vec3 &epicenter,float k,float rmin,
		const Vec3 &centerOfMass, Vec3 &P,Vec3 &L);
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap);
	virtual CBVTree *GetBVTree() { return &m_Tree; }
	virtual i32 UnprojectSphere(Vec3 center,float r,float rsep, contact *pcontact);
	virtual i32 GetPrimitive(i32 iPrim, primitive *pprim) { *(sphere*)pprim = m_sphere; return sizeof(sphere); }
	virtual void DrawWireframe(IPhysRenderer *pRenderer, geom_world_data *gwd, i32 iLevel, i32 idxColor);

	virtual const primitive *GetData() { return &m_sphere; }
	virtual void SetData(const primitive* psph) { CreateSphere((sphere*)psph); }
	virtual float GetVolume() { return (4.0f/3)*g_PI*cube(m_sphere.r); }
	virtual Vec3 GetCenter() { return m_sphere.center; }
	virtual float GetExtent(EGeomForm eForm) const;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual i32 PreparePrimitive(geom_world_data *pgwd,primitive *&pprim,i32 iCaller=0);
	virtual i32 GetPrimitiveList(i32 iStart,i32 nPrims, i32 typeCollider,primitive *pCollider,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,char *pResId);
	virtual i32 GetUnprojectionCandidates(i32 iop,const contact *pcontact, primitive *&pprim,i32 *&piFeature, geometry_under_test *pGTest);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm);
	virtual i32 GetSizeFast() { return sizeof(*this); }

	sphere m_sphere;
	CSingleBoxTree m_Tree;
};

#endif
