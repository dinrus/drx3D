// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef capsulegeom_h
#define capsulegeom_h
#pragma once

class CCapsuleGeom : public CCylinderGeom {
public:
	CCapsuleGeom() {}
	CCapsuleGeom* CreateCapsule(capsule *pcaps);
	virtual i32 GetType() { return GEOM_CAPSULE; }
	virtual void SetData(const primitive* pcaps) { CreateCapsule((capsule*)pcaps); }
	virtual i32 PreparePrimitive(geom_world_data *pgwd,primitive *&pprim,i32 iCaller=0) {
		CCylinderGeom::PreparePrimitive(pgwd,pprim,iCaller); return capsule::type;
	}
	virtual i32 CalcPhysicalProperties(phys_geometry *pgeom);
	virtual i32 FindClosestPoint(geom_world_data *pgwd, i32 &iPrim,i32 &iFeature, const Vec3 &ptdst0,const Vec3 &ptdst1,
		Vec3 *ptres, i32 nMaxIters);
	virtual i32 PointInsideStatus(const Vec3 &pt);
	virtual float CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, Vec3 &massCenter);
	virtual void CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, Vec3 &dPres,Vec3 &dLres);
	virtual i32 DrawToOcclusionCubemap(const geom_world_data *pgwd, i32 iStartPrim,i32 nPrims, i32 iPass, SOcclusionCubeMap* cubeMap);
	virtual i32 UnprojectSphere(Vec3 center,float r,float rsep, contact *pcontact);
	virtual float GetVolume() { return sqr(m_cyl.r)*m_cyl.hh*(g_PI*2) + (4.0f/3*g_PI)*cube(m_cyl.r); }
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts=false);
	virtual i32 GetUnprojectionCandidates(i32 iop,const contact *pcontact, primitive *&pprim,i32 *&piFeature, geometry_under_test *pGTest);
};

#endif
