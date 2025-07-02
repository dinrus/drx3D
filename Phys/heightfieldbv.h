// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef heightfieldbv_h
#define heightfieldbv_h
#pragma once

class CHeightfieldBV : public CBVTree {
public:
	CHeightfieldBV() { m_pUsedTriMap=0; m_minHeight=0.f; m_maxHeight=0.f; m_pMesh=0; m_phf=0; }
	virtual ~CHeightfieldBV() { if (m_pUsedTriMap) delete[] m_pUsedTriMap; }
	virtual i32 GetType() { return BVT_HEIGHTFIELD; }

	virtual float Build(CGeometry *pGeom);
	void SetHeightfield(heightfield *phf);
	virtual void GetBBox(box *pbox);
	virtual i32 MaxPrimsInNode() { return m_PatchSize.x*m_PatchSize.y*2; }
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) {}
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) {}
	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest);

	CTriMesh *m_pMesh;
	heightfield *m_phf;
	Vec2i m_PatchStart;
	Vec2i m_PatchSize;
	float m_minHeight,m_maxHeight;
	u32 *m_pUsedTriMap;
};

struct InitHeightfieldGlobals { InitHeightfieldGlobals(); };

void project_box_on_grid(box *pbox,grid *pgrid, geometry_under_test *pGTest, i32 &ix,i32 &iy,i32 &sx,i32 &sy,float &minz);


#endif
