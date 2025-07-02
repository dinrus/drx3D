// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef bvtree_h
#define bvtree_h


////////////////////////// bounding volumes ////////////////////////

struct BV {
	i32 type;
	i32 iNode;
	inline operator primitive*();
};
struct BV_Primitive : public BV { primitive p; };
inline BV::operator primitive*() { return &static_cast<BV_Primitive *>(this)->p; }

struct BBox : BV {
	box abox;
};

struct BVheightfield : BV {
	heightfield hf;
};

struct BVvoxelgrid : BV {
	voxelgrid voxgrid;
};

struct BVray : BV {
	ray aray;
};

class CGeometry;
class CBVTree;

struct surface_desc {
	Vec3 n;
	i32 idx;
	i32 iFeature;
};
struct edge_desc {
	Vec3 dir;
	Vec3 n[2];
	i32 idx;
	i32 iFeature;
};

struct geometry_under_test {
	CGeometry *pGeometry;
	CBVTree *pBVtree;
	i32 *pUsedNodesMap;
	i32 *pUsedNodesIdx;
	i32 nUsedNodes;
	i32 nMaxUsedNodes;
	i32 bStopIntersection;
	i32 bCurNodeUsed;

	Matrix33 R,R_rel;
	Vec3 offset,offset_rel;
	float scale,rscale, scale_rel,rscale_rel;
	i32 bTransformUpdated;

	Vec3 v;
	Vec3 w,centerOfMass;
	Vec3 centerOfRotation;
	intersection_params *pParams;

	Vec3 axisContactNormal;
	Vec3 sweepdir,sweepdir_loc;
	float sweepstep,sweepstep_loc;
	Vec3 ptOutsidePivot;

	i32 typeprim;
	primitive *primbuf; // used to get node contents
	primitive *primbuf1;// used to get unprojection candidates
	i32 szprimbuf,szprimbuf1;
	i32 *iFeature_buf; // feature that led to this primitive
	char *idbuf; // ids of unprojection candidates
	i32 szprim;

	surface_desc *surfaces;	// the last potentially surfaces
	edge_desc *edges;	// the last potentially contacting edges
	i32 nSurfaces,nEdges;
	float minAreaEdge;

	geom_contact *contacts;
	i32 *pnContacts;
	i32 nMaxContacts;

	i32 iCaller;
};

enum BVtreetypes { BVT_OBB=0, BVT_AABB=1, BVT_SINGLEBOX=2, BVT_RAY=3, BVT_HEIGHTFIELD=4, BVT_VOXEL=5 };

class CBVTree {
public:
	virtual ~CBVTree() {}
	virtual i32 GetType() = 0;
	virtual void GetBBox(box *pbox) {}
	virtual i32 MaxPrimsInNode() { return 1; }
	virtual float Build(CGeometry *pGeom) = 0;
	virtual void SetGeomConvex() {}

	virtual i32 PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl) {
		pGTest->pUsedNodesMap = 0;
		pGTest->pUsedNodesIdx = 0;
		pGTest->nMaxUsedNodes = 0;
		pGTest->nUsedNodes = -1;
		return 1;
	}

	virtual void CleanupAfterIntersectionTest(geometry_under_test *pGTest) {}
	virtual void GetNodeBV(BV *&pBV, i32 iNode=0, i32 iCaller=0) = 0;
	virtual void GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) = 0;
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode=0, i32 iCaller=0) = 0;
	virtual void GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode=0, i32 iCaller=0) = 0;
	virtual float SplitPriority(const BV* pBV) { return 0.0f; }
	virtual void GetNodeChildrenBVs(const Matrix33 &Rw,const Vec3 &offsw,float scalew, const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0) {}
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0) {}
	virtual void GetNodeChildrenBVs(const BV *pBV_parent, const Vec3 &sweepdir,float sweepstep, BV *&pBV_child1,BV *&pBV_child2, i32 iCaller=0) {}
	virtual void ReleaseLastBVs(i32 iCaller=0) {}
	virtual void ReleaseLastSweptBVs(i32 iCaller=0) {}
	virtual void ResetCollisionArea() {}
	virtual float GetMaxSkipDim() { return 0; }

	virtual void GetMemoryStatistics(IDrxSizer *pSizer) {}
	virtual void Save(CMemStream &stm) {}
	virtual void Load(CMemStream &stm, CGeometry *pGeom) {}

	virtual i32 SanityCheck() { return 1; }

	virtual i32 GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal,
		geometry_under_test *pGTest,geometry_under_test *pGTestOp) = 0;

	virtual i32 GetNodeContentsIdx(i32 iNode, i32 &iStartPrim) { iStartPrim=0; return 1; }
	virtual void MarkUsedTriangle(i32 itri, geometry_under_test *pGTest) {}
};

#endif
