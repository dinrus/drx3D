#ifndef DRX3D_CONVEX_HULL_SHAPE_H
#define DRX3D_CONVEX_HULL_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///The ConvexHullShape implements an implicit convex hull of an array of vertices.
///drx3D provides a general and fast collision detector for convex shapes based on GJK and EPA using localGetSupportingVertex.
ATTRIBUTE_ALIGNED16(class)
ConvexHullShape : public PolyhedralConvexAabbCachingShape
{
protected:
	AlignedObjectArray<Vec3> m_unscaledPoints;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	///this constructor optionally takes in a pointer to points. Each point is assumed to be 3 consecutive Scalar (x,y,z), the striding defines the number of bytes between each point, in memory.
	///It is easier to not pass any points in the constructor, and just add one point at a time, using addPoint.
	//ConvexHullShape make an internal copy of the points.
	ConvexHullShape(const Scalar* points = 0, i32 numPoints = 0, i32 stride = sizeof(Vec3));

	void addPoint(const Vec3& point, bool recalculateLocalAabb = true);

	Vec3* getUnscaledPoints()
	{
		return &m_unscaledPoints[0];
	}

	const Vec3* getUnscaledPoints() const
	{
		return &m_unscaledPoints[0];
	}

	///getPoints is obsolete, please use getUnscaledPoints
	const Vec3* getPoints() const
	{
		return getUnscaledPoints();
	}

	void optimizeConvexHull();

	SIMD_FORCE_INLINE Vec3 getScaledPoint(i32 i) const
	{
		return m_unscaledPoints[i] * m_localScaling;
	}

	SIMD_FORCE_INLINE i32 getNumPoints() const
	{
		return m_unscaledPoints.size();
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void project(const Transform2& trans, const Vec3& dir, Scalar& minProj, Scalar& maxProj, Vec3& witnesPtMin, Vec3& witnesPtMax) const;

	//debugging
	virtual tukk getName() const { return "Convex"; }

	virtual i32 getNumVertices() const;
	virtual i32 getNumEdges() const;
	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const;
	virtual void getVertex(i32 i, Vec3& vtx) const;
	virtual i32 getNumPlanes() const;
	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const;
	virtual bool isInside(const Vec3& pt, Scalar tolerance) const;

	///in case we receive negative scaling
	virtual void setLocalScaling(const Vec3& scaling);

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

// clang-format off

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	ConvexHullShapeData
{
	ConvexInternalShapeData	m_convexInternalShapeData;

	Vec3FloatData	*m_unscaledPointsFloatPtr;
	Vec3DoubleData	*m_unscaledPointsDoublePtr;

	i32		m_numUnscaledPoints;
	char m_padding3[4];

};

// clang-format on

SIMD_FORCE_INLINE i32 ConvexHullShape::calculateSerializeBufferSize() const
{
	return sizeof(ConvexHullShapeData);
}

#endif  //DRX3D_CONVEX_HULL_SHAPE_H
