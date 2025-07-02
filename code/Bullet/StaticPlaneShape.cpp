
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>

#include <drx3D/Maths/Linear/Transform2Util.h>

StaticPlaneShape::StaticPlaneShape(const Vec3& planeNormal, Scalar planeConstant)
	: ConcaveShape(), m_planeNormal(planeNormal.normalized()), m_planeConstant(planeConstant), m_localScaling(Scalar(1.), Scalar(1.), Scalar(1.))
{
	m_shapeType = STATIC_PLANE_PROXYTYPE;
	//	Assert( btFuzzyZero(m_planeNormal.length() - Scalar(1.)) );
}

StaticPlaneShape::~StaticPlaneShape()
{
}

void StaticPlaneShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	(void)t;
	/*
	Vec3 infvec (Scalar(DRX3D_LARGE_FLOAT),Scalar(DRX3D_LARGE_FLOAT),Scalar(DRX3D_LARGE_FLOAT));

	Vec3 center = m_planeNormal*m_planeConstant;
	aabbMin = center + infvec*m_planeNormal;
	aabbMax = aabbMin;
	aabbMin.setMin(center - infvec*m_planeNormal);
	aabbMax.setMax(center - infvec*m_planeNormal); 
	*/

	aabbMin.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
	aabbMax.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
}

void StaticPlaneShape::processAllTriangles(TriangleCallback* callback, const Vec3& aabbMin, const Vec3& aabbMax) const
{
	Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);
	Scalar radius = halfExtents.length();
	Vec3 center = (aabbMax + aabbMin) * Scalar(0.5);

	//this is where the triangles are generated, given AABB and plane equation (normal/constant)

	Vec3 tangentDir0, tangentDir1;

	//tangentDir0/tangentDir1 can be precalculated
	PlaneSpace1(m_planeNormal, tangentDir0, tangentDir1);

	Vec3 projectedCenter = center - (m_planeNormal.dot(center) - m_planeConstant) * m_planeNormal;

	Vec3 triangle[3];
	triangle[0] = projectedCenter + tangentDir0 * radius + tangentDir1 * radius;
	triangle[1] = projectedCenter + tangentDir0 * radius - tangentDir1 * radius;
	triangle[2] = projectedCenter - tangentDir0 * radius - tangentDir1 * radius;

	callback->processTriangle(triangle, 0, 0);

	triangle[0] = projectedCenter - tangentDir0 * radius - tangentDir1 * radius;
	triangle[1] = projectedCenter - tangentDir0 * radius + tangentDir1 * radius;
	triangle[2] = projectedCenter + tangentDir0 * radius + tangentDir1 * radius;

	callback->processTriangle(triangle, 0, 1);
}

void StaticPlaneShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	(void)mass;

	//moving concave objects not supported

	inertia.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
}

void StaticPlaneShape::setLocalScaling(const Vec3& scaling)
{
	m_localScaling = scaling;
}
const Vec3& StaticPlaneShape::getLocalScaling() const
{
	return m_localScaling;
}
