#include <drx3D/Physics/Collision/Shapes/Box2dShape.h>

//{

void Box2dShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	Transform2Aabb(getHalfExtentsWithoutMargin(), getMargin(), t, aabbMin, aabbMax);
}

void Box2dShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	//Scalar margin = Scalar(0.);
	Vec3 halfExtents = getHalfExtentsWithMargin();

	Scalar lx = Scalar(2.) * (halfExtents.x());
	Scalar ly = Scalar(2.) * (halfExtents.y());
	Scalar lz = Scalar(2.) * (halfExtents.z());

	inertia.setVal(mass / (Scalar(12.0)) * (ly * ly + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + lz * lz),
					 mass / (Scalar(12.0)) * (lx * lx + ly * ly));
}
