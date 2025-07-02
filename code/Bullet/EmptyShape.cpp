
#include <drx3D/Physics/Collision/Shapes/EmptyShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>

EmptyShape::EmptyShape() : ConcaveShape()
{
	m_shapeType = EMPTY_SHAPE_PROXYTYPE;
}

EmptyShape::~EmptyShape()
{
}

///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
void EmptyShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	Vec3 margin(getMargin(), getMargin(), getMargin());

	aabbMin = t.getOrigin() - margin;

	aabbMax = t.getOrigin() + margin;
}

void EmptyShape::calculateLocalInertia(Scalar, Vec3&) const
{
	Assert(0);
}
