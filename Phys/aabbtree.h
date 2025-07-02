// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef aabbtree_h
#define aabbtree_h

struct AABBnode {
	u32 ichild;
	u8 minx;
	u8 maxx;
	u8 miny;
	u8 maxy;
	u8 minz;
	u8 maxz;
	u8 ntris;
	u8 bSingleColl;

	AUTO_STRUCT_INFO;
};

struct AABBnodeV0 {
	u32 minx : 7;
	u32 maxx : 7;
	u32 miny : 7;
	u32 maxy : 7;
	u32 minz : 7;
	u32 maxz : 7;
	u32 ichild : 15;
	u32 ntris : 6;
	u32 bSingleColl : 1;

	AUTO_STRUCT_INFO_LOCAL;
};

class CTriMesh;

class CAABBTree : public CBVTree {
public:
	// cppcheck-suppress uninitMemberVar
	CAABBTree() { m_pNodes=0; m_pTri2Node=0; m_maxDepth=64; }
	virtual ~CAABBTree() {
		if (m_pNodes) delete[] m_pNodes; m_pNodes=0;
		if (m_pTri2Node) delete[] m_pTri2Node; m_pTri2Node=0;
	}

	virtual i32 GetType()  { return BVT_AABB; }

	float Build(CGeometry *pMesh);
	virtual void SetGeomConvex();

	void SetParams(i32 nMinTrisPerNode,i32 nMaxTrisPerNode, float skipdim, const Matrix33 &Basis, i32 planeOptimisation=0);
	float BuildNode(i32 iNode, i32 iTriStart,i32 nTris, Vec3 center,Vec3 size, Vec3 (*bbtri)[3], i32 nDepth);

	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl);
	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest);
	virtual void GetBBox(box *pbox);
	virtual i32 MaxPrimsInNode() { return m_nMaxTrisInNode; }
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0);
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) {}
	virtual float SplitPriority(const BV* pBV);
	virtual void GetNodeChildrenBVs(const Matrix33 &Rw,const Vec3 &offsw,float scalew, const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0);
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0);
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, const Vec3 &sweepdir,float sweepstep, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0);
	virtual void ReleaseLastBVs(i32 iCaller);
	virtual void ReleaseLastSweptBVs(i32 iCaller);
	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp);
	virtual i32 GetNodeContentsIdx(i32 iNode, i32 &iStartPrim) { iStartPrim = m_pNodes[iNode].ichild; return m_pNodes[iNode].ntris; }
	virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest);
	virtual float GetMaxSkipDim() { return m_maxSkipDim/max(max(m_size.x,m_size.y),m_size.z); }
	void GetRootNodeDim(Vec3 &center,Vec3 &size)
	{
		Vec3 ptmin,ptmax;
		ptmin.x = m_pNodes[0].minx*m_size.x*(2.0f/128);
		ptmax.x = (m_pNodes[0].maxx+1)*m_size.x*(2.0f/128);
		ptmin.y = m_pNodes[0].miny*m_size.y*(2.0f/128);
		ptmax.y = (m_pNodes[0].maxy+1)*m_size.y*(2.0f/128);
		ptmin.z = m_pNodes[0].minz*m_size.z*(2.0f/128);
		ptmax.z = (m_pNodes[0].maxz+1)*m_size.z*(2.0f/128);
		center = m_center+((ptmax+ptmin)*0.5f-m_size)*m_Basis;
		size = (ptmax-ptmin)*0.5f;
	}

	virtual void GetMemoryStatistics(IDrxSizer *pSizer);
	virtual void Save(CMemStream &stm);
	virtual void Load(CMemStream &stm, CGeometry *pGeom);

	virtual i32 SanityCheck();

	CTriMesh *m_pMesh;
	AABBnode *m_pNodes;
	i32 m_nNodes,m_nNodesAlloc;
	Vec3 m_center;
	Vec3 m_size;
	Matrix33 m_Basis;
	i16 m_bOriented;
	i16 m_axisSplitMask;
	i32 *m_pTri2Node,m_nBitsLog;
	i32 m_nMaxTrisPerNode,m_nMinTrisPerNode;
	i32 m_nMaxTrisInNode;
	float m_maxSkipDim;
	i32 m_maxDepth;
};

#endif
