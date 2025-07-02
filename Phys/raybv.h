// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef raybv_h
#define raybv_h
#pragma once

class CRayBV : public CBVTree {
public:
	CRayBV() { m_pGeom=0; m_pray=0; }
	virtual i32 GetType() { return BVT_RAY; }
	virtual float Build(CGeometry *pGeom) { m_pGeom=pGeom; return 0.0f; }
	void SetRay(ray *pray) { m_pray = pray; }
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) {}
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) {}
	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);

	CGeometry *m_pGeom;
	ray *m_pray;
};

#endif

