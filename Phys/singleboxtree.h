// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef singleboxtree_h
#define singleboxtree_h

class CSingleBoxTree : public CBVTree {
public:
	ILINE CSingleBoxTree() { m_nPrims=1; m_bIsConvex=0; m_pGeom=0; }
	virtual i32 GetType() { return BVT_SINGLEBOX; }
	virtual float Build(CGeometry *pGeom);
	virtual void SetGeomConvex() { m_bIsConvex = 1; }
	void SetBox(box *pbox);
	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl);
	virtual void GetBBox(box *pbox);
	virtual i32 MaxPrimsInNode() { return m_nPrims; }
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0);
	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual i32 GetNodeContentsIdx(i32 iNode, i32 &iStartPrim) { iStartPrim=0; return m_nPrims; }
	virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest);

	virtual void GetMemoryStatistics(IDrxSizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm, CGeometry *pGeom);

	CGeometry *m_pGeom;
	box m_Box;
	i32 m_nPrims;
	i32 m_bIsConvex;
};

#endif
