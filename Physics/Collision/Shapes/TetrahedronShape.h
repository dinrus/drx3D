#ifndef DRX3D_SIMPLEX_1TO4_SHAPE
#define DRX3D_SIMPLEX_1TO4_SHAPE

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>

///The BU_Simplex1to4 implements tetrahedron, triangle, line, vertex collision shapes. In most cases it is better to use ConvexHullShape instead.
ATTRIBUTE_ALIGNED16(class)
BU_Simplex1to4 : public PolyhedralConvexAabbCachingShape
{
protected:
	i32 m_numVertices;
	Vec3 m_vertices[4];

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	BU_Simplex1to4();

	BU_Simplex1to4(const Vec3& pt0);
	BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1);
	BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1, const Vec3& pt2);
	BU_Simplex1to4(const Vec3& pt0, const Vec3& pt1, const Vec3& pt2, const Vec3& pt3);

	void reset()
	{
		m_numVertices = 0;
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	void addVertex(const Vec3& pt);

	//PolyhedralConvexShape interface

	virtual i32 getNumVertices() const;

	virtual i32 getNumEdges() const;

	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const;

	virtual void getVertex(i32 i, Vec3& vtx) const;

	virtual i32 getNumPlanes() const;

	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const;

	virtual i32 getIndex(i32 i) const;

	virtual bool isInside(const Vec3& pt, Scalar tolerance) const;

	///getName is for debugging
	virtual tukk getName() const { return "BU_Simplex1to4"; }
};

#endif  //DRX3D_SIMPLEX_1TO4_SHAPE
