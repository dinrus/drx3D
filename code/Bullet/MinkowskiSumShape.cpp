
#include <drx3D/Physics/Collision/Shapes/MinkowskiSumShape.h>

MinkowskiSumShape::MinkowskiSumShape(const ConvexShape* shapeA, const ConvexShape* shapeB)
	: ConvexInternalShape(),
	  m_shapeA(shapeA),
	  m_shapeB(shapeB)
{
	m_shapeType = MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE;
	m_transA.setIdentity();
	m_transB.setIdentity();
}

Vec3 MinkowskiSumShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	Vec3 supVertexA = m_transA(m_shapeA->localGetSupportingVertexWithoutMargin(vec * m_transA.getBasis()));
	Vec3 supVertexB = m_transB(m_shapeB->localGetSupportingVertexWithoutMargin(-vec * m_transB.getBasis()));
	return supVertexA - supVertexB;
}

void MinkowskiSumShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	///@todo: could make recursive use of batching. probably this shape is not used frequently.
	for (i32 i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i] = localGetSupportingVertexWithoutMargin(vectors[i]);
	}
}

Scalar MinkowskiSumShape::getMargin() const
{
	return m_shapeA->getMargin() + m_shapeB->getMargin();
}

void MinkowskiSumShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	(void)mass;
	//inertia of the AABB of the Minkowski sum
	Transform2 identity;
	identity.setIdentity();
	Vec3 aabbMin, aabbMax;
	getAabb(identity, aabbMin, aabbMax);

	Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);

	Scalar margin = getMargin();

	Scalar lx = Scalar(2.) * (halfExtents.x() + margin);
	Scalar ly = Scalar(2.) * (halfExtents.y() + margin);
	Scalar lz = Scalar(2.) * (halfExtents.z() + margin);
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(0.08333333);

	inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));
}
