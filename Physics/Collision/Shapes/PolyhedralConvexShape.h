#ifndef DRX3D_POLYHEDRAL_CONVEX_SHAPE_H
#define DRX3D_POLYHEDRAL_CONVEX_SHAPE_H

#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
class ConvexPolyhedron;

///The PolyhedralConvexShape is an internal interface class for polyhedral convex shapes.
ATTRIBUTE_ALIGNED16(class)
PolyhedralConvexShape : public ConvexInternalShape
{
protected:
	ConvexPolyhedron* m_polyhedron;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	PolyhedralConvexShape();

	virtual ~PolyhedralConvexShape();

	///optional method mainly used to generate multiple contact points by clipping polyhedral features (faces/edges)
	///experimental/work-in-progress
	virtual bool initializePolyhedralFeatures(i32 shiftVerticesByMargin = 0);

	virtual void setPolyhedralFeatures(ConvexPolyhedron & polyhedron);

	const ConvexPolyhedron* getConvexPolyhedron() const
	{
		return m_polyhedron;
	}

	//brute force implementations

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual i32 getNumVertices() const = 0;
	virtual i32 getNumEdges() const = 0;
	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const = 0;
	virtual void getVertex(i32 i, Vec3& vtx) const = 0;
	virtual i32 getNumPlanes() const = 0;
	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const = 0;
	//	virtual i32 getIndex(i32 i) const = 0 ;

	virtual bool isInside(const Vec3& pt, Scalar tolerance) const = 0;
};

///The PolyhedralConvexAabbCachingShape adds aabb caching to the PolyhedralConvexShape
class PolyhedralConvexAabbCachingShape : public PolyhedralConvexShape
{
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;
	bool m_isLocalAabbValid;

protected:
	void setCachedLocalAabb(const Vec3& aabbMin, const Vec3& aabbMax)
	{
		m_isLocalAabbValid = true;
		m_localAabbMin = aabbMin;
		m_localAabbMax = aabbMax;
	}

	inline void getCachedLocalAabb(Vec3& aabbMin, Vec3& aabbMax) const
	{
		Assert(m_isLocalAabbValid);
		aabbMin = m_localAabbMin;
		aabbMax = m_localAabbMax;
	}

protected:
	PolyhedralConvexAabbCachingShape();

public:
	inline void getNonvirtualAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax, Scalar margin) const
	{
		//lazy evaluation of local aabb
		Assert(m_isLocalAabbValid);
		Transform2Aabb(m_localAabbMin, m_localAabbMax, margin, trans, aabbMin, aabbMax);
	}

	virtual void setLocalScaling(const Vec3& scaling);

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	void recalcLocalAabb();
};

#endif  //DRX3D_POLYHEDRAL_CONVEX_SHAPE_H
