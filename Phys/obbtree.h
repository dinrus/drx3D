// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef obbtree_h
#define obbtree_h
#pragma once

struct OBBnode {
    Vec3 axes[3];
    Vec3 center;
    Vec3 size;
    i32 iparent;
    i32 ichild;
    i32 ntris;

    AUTO_STRUCT_INFO;
};

class CTriMesh;

class COBBTree : public CBVTree {
public:
    // cppcheck-suppress uninitMemberVar
    COBBTree() { m_nMinTrisPerNode=2; m_nMaxTrisPerNode=4; m_maxSkipDim=0; m_pNodes=0; m_pTri2Node=0; }
    virtual ~COBBTree() {
        if (m_pNodes) delete[] m_pNodes; m_pNodes=0;
        if (m_pTri2Node) delete[] m_pTri2Node; m_pTri2Node=0;
    }
    virtual i32 GetType() { return BVT_OBB; }

    virtual float Build(CGeometry *pMesh);
    virtual void SetGeomConvex();

    void SetParams(i32 nMinTrisPerNode,i32 nMaxTrisPerNode, float skipdim);
    float BuildNode(i32 iNode, i32 iTriStart,i32 nTris, i32 nDepth);
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
    virtual void ReleaseLastBVs(i32 iCaller=0);
    virtual void ReleaseLastSweptBVs(i32 iCaller=0);
    virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
        geometry_under_test *pGTest,geometry_under_test *pGTestOp);
    virtual i32 GetNodeContentsIdx(i32 iNode, i32 &iStartPrim) { iStartPrim = m_pNodes[iNode].ichild; return m_pNodes[iNode].ntris; }
    virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest);
    virtual float GetMaxSkipDim() { return m_maxSkipDim/max(max(m_pNodes[0].size.x,m_pNodes[0].size.y),m_pNodes[0].size.z); }

    virtual void GetMemoryStatistics(IDrxSizer *pSizer);
    virtual void Save(CMemStream &stm);
    virtual void Load(CMemStream &stm, CGeometry *pGeom);

    virtual i32 SanityCheck();

    CTriMesh *m_pMesh;
    OBBnode *m_pNodes;
    i32 m_nNodes;
    i32 m_nNodesAlloc;
    index_t *m_pTri2Node;
    i32 m_nMaxTrisInNode;

    i32 m_nMinTrisPerNode;
    i32 m_nMaxTrisPerNode;
    float m_maxSkipDim;
    i32 *m_pMapVtxUsed;
    Vec3 *m_pVtxUsed;
};

#endif
