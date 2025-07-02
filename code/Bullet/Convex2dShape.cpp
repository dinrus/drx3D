#include <drx3D/Physics/Collision/Shapes/Convex2dShape.h>

Convex2dShape::Convex2dShape(ConvexShape* convexChildShape) : ConvexShape(), m_childConvexShape(convexChildShape)
{
	m_shapeType = CONVEX_2D_SHAPE_PROXYTYPE;
}

Convex2dShape::~Convex2dShape()
{
}

Vec3 Convex2dShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	return m_childConvexShape->localGetSupportingVertexWithoutMargin(vec);
}

void Convex2dShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	m_childConvexShape->batchedUnitVectorGetSupportingVertexWithoutMargin(vectors, supportVerticesOut, numVectors);
}

Vec3 Convex2dShape::localGetSupportingVertex(const Vec3& vec) const
{
	return m_childConvexShape->localGetSupportingVertex(vec);
}

void Convex2dShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	///this linear upscaling is not realistic, but we don't deal with large mass ratios...
	m_childConvexShape->calculateLocalInertia(mass, inertia);
}

///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
void Convex2dShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	m_childConvexShape->getAabb(t, aabbMin, aabbMax);
}

void Convex2dShape::getAabbSlow(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	m_childConvexShape->getAabbSlow(t, aabbMin, aabbMax);
}

void Convex2dShape::setLocalScaling(const Vec3& scaling)
{
	m_childConvexShape->setLocalScaling(scaling);
}

const Vec3& Convex2dShape::getLocalScaling() const
{
	return m_childConvexShape->getLocalScaling();
}

void Convex2dShape::setMargin(Scalar margin)
{
	m_childConvexShape->setMargin(margin);
}
Scalar Convex2dShape::getMargin() const
{
	return m_childConvexShape->getMargin();
}

i32 Convex2dShape::getNumPreferredPenetrationDirections() const
{
	return m_childConvexShape->getNumPreferredPenetrationDirections();
}

void Convex2dShape::getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const
{
	m_childConvexShape->getPreferredPenetrationDirection(index, penetrationVector);
}
