#ifndef DRX3D_CONVEX_POINT_CLOUD_SHAPE_H
#define DRX3D_CONVEX_POINT_CLOUD_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///The ConvexPointCloudShape implements an implicit convex hull of an array of vertices.
ATTRIBUTE_ALIGNED16(class)
ConvexPointCloudShape : public PolyhedralConvexAabbCachingShape
{
	Vec3* m_unscaledPoints;
	i32 m_numPoints;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConvexPointCloudShape()
	{
		m_localScaling.setVal(1.f, 1.f, 1.f);
		m_shapeType = CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE;
		m_unscaledPoints = 0;
		m_numPoints = 0;
	}

	ConvexPointCloudShape(Vec3 * points, i32 numPoints, const Vec3& localScaling, bool computeAabb = true)
	{
		m_localScaling = localScaling;
		m_shapeType = CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE;
		m_unscaledPoints = points;
		m_numPoints = numPoints;

		if (computeAabb)
			recalcLocalAabb();
	}

	void setPoints(Vec3 * points, i32 numPoints, bool computeAabb = true, const Vec3& localScaling = Vec3(1.f, 1.f, 1.f))
	{
		m_unscaledPoints = points;
		m_numPoints = numPoints;
		m_localScaling = localScaling;

		if (computeAabb)
			recalcLocalAabb();
	}

	SIMD_FORCE_INLINE Vec3* getUnscaledPoints()
	{
		return m_unscaledPoints;
	}

	SIMD_FORCE_INLINE const Vec3* getUnscaledPoints() const
	{
		return m_unscaledPoints;
	}

	SIMD_FORCE_INLINE i32 getNumPoints() const
	{
		return m_numPoints;
	}

	SIMD_FORCE_INLINE Vec3 getScaledPoint(i32 index) const
	{
		return m_unscaledPoints[index] * m_localScaling;
	}

#ifndef __SPU__
	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;
#endif

	//debugging
	virtual tukk getName() const { return "ConvexPointCloud"; }

	virtual i32 getNumVertices() const;
	virtual i32 getNumEdges() const;
	virtual void getEdge(i32 i, Vec3& pa, Vec3& pb) const;
	virtual void getVertex(i32 i, Vec3& vtx) const;
	virtual i32 getNumPlanes() const;
	virtual void getPlane(Vec3 & planeNormal, Vec3 & planeSupport, i32 i) const;
	virtual bool isInside(const Vec3& pt, Scalar tolerance) const;

	///in case we receive negative scaling
	virtual void setLocalScaling(const Vec3& scaling);
};

#endif  //DRX3D_CONVEX_POINT_CLOUD_SHAPE_H
