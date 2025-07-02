// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef voxelbv_h
#define voxelbv_h
#pragma once

class CVoxelBV : public CBVTree {
public:
	CVoxelBV()
		: m_pMesh(nullptr)
		, m_pgrid(nullptr)
		, m_nTris(0)
	{
		static_assert(DRX_ARRAY_COUNT(m_iBBox) == 2, "Неполноценный размер массива!");
		m_iBBox[0].zero();
		m_iBBox[1].zero();
	}

	virtual ~CVoxelBV() {}
	virtual i32 GetType() { return BVT_VOXEL; }
	float Build(CGeometry *pGeom) { m_pMesh = (CTriMesh*)pGeom; return 0; }

	virtual void GetBBox(box *pbox);
	virtual i32 MaxPrimsInNode() { return m_nTris; }

	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest);
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) { GetNodeBV(pBV); }
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0)
	{ GetNodeBV(Rw,offsw,scalew,pBV,0,iCaller); }
	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest);
	virtual void ResetCollisionArea() { m_iBBox[0].zero(); m_iBBox[1]=m_pgrid->size; }

	CTriMesh *m_pMesh;
	voxelgrid *m_pgrid;
	Vec3i m_iBBox[2];
	i32 m_nTris;
};

#endif
