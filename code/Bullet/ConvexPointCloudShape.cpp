#include <drx3D/Physics/Collision/Shapes/ConvexPointCloudShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Maths/Linear/Quat.h>

void ConvexPointCloudShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling;
	recalcLocalAabb();
}

#ifndef __SPU__
Vec3 ConvexPointCloudShape::localGetSupportingVertexWithoutMargin(const Vec3& vec0) const
{
	Vec3 supVec(Scalar(0.), Scalar(0.), Scalar(0.));
	Scalar maxDot = Scalar(-DRX3D_LARGE_FLOAT);

	Vec3 vec = vec0;
	Scalar lenSqr = vec.length2();
	if (lenSqr < Scalar(0.0001))
	{
		vec.setVal(1, 0, 0);
	}
	else
	{
		Scalar rlen = Scalar(1.) / Sqrt(lenSqr);
		vec *= rlen;
	}

	if (m_numPoints > 0)
	{
		// Here we take advantage of dot(a*b, c) = dot( a, b*c) to do less work. Note this transformation is true mathematically, not numerically.
		//    Vec3 scaled = vec * m_localScaling;
		i32 index = (i32)vec.maxDot(&m_unscaledPoints[0], m_numPoints, maxDot);  //FIXME: may violate encapsulation of m_unscaledPoints
		return getScaledPoint(index);
	}

	return supVec;
}

void ConvexPointCloudShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	for (i32 j = 0; j < numVectors; j++)
	{
		const Vec3& vec = vectors[j] * m_localScaling;  // dot( a*c, b) = dot(a, b*c)
		Scalar maxDot;
		i32 index = (i32)vec.maxDot(&m_unscaledPoints[0], m_numPoints, maxDot);
		supportVerticesOut[j][3] = Scalar(-DRX3D_LARGE_FLOAT);
		if (0 <= index)
		{
			//WARNING: don't swap next lines, the w component would get overwritten!
			supportVerticesOut[j] = getScaledPoint(index);
			supportVerticesOut[j][3] = maxDot;
		}
	}
}

Vec3 ConvexPointCloudShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 supVertex = localGetSupportingVertexWithoutMargin(vec);

	if (getMargin() != Scalar(0.))
	{
		Vec3 vecnorm = vec;
		if (vecnorm.length2() < (SIMD_EPSILON * SIMD_EPSILON))
		{
			vecnorm.setVal(Scalar(-1.), Scalar(-1.), Scalar(-1.));
		}
		vecnorm.normalize();
		supVertex += getMargin() * vecnorm;
	}
	return supVertex;
}

#endif

//currently just for debugging (drawing), perhaps future support for algebraic continuous collision detection
//Please note that you can debug-draw ConvexHullShape with the Raytracer Demo
i32 ConvexPointCloudShape::getNumVertices() const
{
	return m_numPoints;
}

i32 ConvexPointCloudShape::getNumEdges() const
{
	return 0;
}

void ConvexPointCloudShape::getEdge(i32 i, Vec3& pa, Vec3& pb) const
{
	Assert(0);
}

void ConvexPointCloudShape::getVertex(i32 i, Vec3& vtx) const
{
	vtx = m_unscaledPoints[i] * m_localScaling;
}

i32 ConvexPointCloudShape::getNumPlanes() const
{
	return 0;
}

void ConvexPointCloudShape::getPlane(Vec3&, Vec3&, i32) const
{
	Assert(0);
}

//not yet
bool ConvexPointCloudShape::isInside(const Vec3&, Scalar) const
{
	Assert(0);
	return false;
}
