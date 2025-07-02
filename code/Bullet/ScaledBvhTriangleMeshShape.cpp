#include <drx3D/Physics/Collision/Shapes/ScaledBvhTriangleMeshShape.h>

ScaledBvhTriangleMeshShape::ScaledBvhTriangleMeshShape(BvhTriangleMeshShape* childShape, const Vec3& localScaling)
	: m_localScaling(localScaling), m_bvhTriMeshShape(childShape)
{
	m_shapeType = SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE;
}

ScaledBvhTriangleMeshShape::~ScaledBvhTriangleMeshShape()
{
}

class ScaledTriangleCallback : public TriangleCallback
{
	TriangleCallback* m_originalCallback;

	Vec3 m_localScaling;

public:
	ScaledTriangleCallback(TriangleCallback* originalCallback, const Vec3& localScaling)
		: m_originalCallback(originalCallback),
		  m_localScaling(localScaling)
	{
	}

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		Vec3 newTriangle[3];
		newTriangle[0] = triangle[0] * m_localScaling;
		newTriangle[1] = triangle[1] * m_localScaling;
		newTriangle[2] = triangle[2] * m_localScaling;
		m_originalCallback->processTriangle(&newTriangle[0], partId, triangleIndex);
	}
};

void ScaledBvhTriangleMeshShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	ScaledTriangleCallback scaledCallback(callback, m_localScaling);

	Vec3 invLocalScaling(1.f / m_localScaling.getX(), 1.f / m_localScaling.getY(), 1.f / m_localScaling.getZ());
	Vec3 scaledAabbMin, scaledAabbMax;

	///support negative scaling
	scaledAabbMin[0] = m_localScaling.getX() >= 0. ? aabbMin[0] * invLocalScaling[0] : aabbMax[0] * invLocalScaling[0];
	scaledAabbMin[1] = m_localScaling.getY() >= 0. ? aabbMin[1] * invLocalScaling[1] : aabbMax[1] * invLocalScaling[1];
	scaledAabbMin[2] = m_localScaling.getZ() >= 0. ? aabbMin[2] * invLocalScaling[2] : aabbMax[2] * invLocalScaling[2];
	scaledAabbMin[3] = 0.f;

	scaledAabbMax[0] = m_localScaling.getX() <= 0. ? aabbMin[0] * invLocalScaling[0] : aabbMax[0] * invLocalScaling[0];
	scaledAabbMax[1] = m_localScaling.getY() <= 0. ? aabbMin[1] * invLocalScaling[1] : aabbMax[1] * invLocalScaling[1];
	scaledAabbMax[2] = m_localScaling.getZ() <= 0. ? aabbMin[2] * invLocalScaling[2] : aabbMax[2] * invLocalScaling[2];
	scaledAabbMax[3] = 0.f;

	m_bvhTriMeshShape->processAllTriangles(&scaledCallback, scaledAabbMin, scaledAabbMax);
}

void ScaledBvhTriangleMeshShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	Vec3 localAabbMin = m_bvhTriMeshShape->getLocalAabbMin();
	Vec3 localAabbMax = m_bvhTriMeshShape->getLocalAabbMax();

	Vec3 tmpLocalAabbMin = localAabbMin * m_localScaling;
	Vec3 tmpLocalAabbMax = localAabbMax * m_localScaling;

	localAabbMin[0] = (m_localScaling.getX() >= 0.) ? tmpLocalAabbMin[0] : tmpLocalAabbMax[0];
	localAabbMin[1] = (m_localScaling.getY() >= 0.) ? tmpLocalAabbMin[1] : tmpLocalAabbMax[1];
	localAabbMin[2] = (m_localScaling.getZ() >= 0.) ? tmpLocalAabbMin[2] : tmpLocalAabbMax[2];
	localAabbMax[0] = (m_localScaling.getX() <= 0.) ? tmpLocalAabbMin[0] : tmpLocalAabbMax[0];
	localAabbMax[1] = (m_localScaling.getY() <= 0.) ? tmpLocalAabbMin[1] : tmpLocalAabbMax[1];
	localAabbMax[2] = (m_localScaling.getZ() <= 0.) ? tmpLocalAabbMin[2] : tmpLocalAabbMax[2];

	Vec3 localHalfExtents = Scalar(0.5) * (localAabbMax - localAabbMin);
	Scalar margin = m_bvhTriMeshShape->getMargin();
	localHalfExtents += Vec3(margin, margin, margin);
	Vec3 localCenter = Scalar(0.5) * (localAabbMax + localAabbMin);

	Matrix3x3 abs_b = trans.getBasis().absolute();

	Vec3 center = trans(localCenter);

	Vec3 extent = localHalfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);
	aabbMin = center - extent;
	aabbMax = center + extent;
}

void ScaledBvhTriangleMeshShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling;
}

const Vec3& ScaledBvhTriangleMeshShape::getLocalScaling() const
{
	return m_localScaling;
}

void ScaledBvhTriangleMeshShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	///don't make this a movable object!
	//	Assert(0);
}
