#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

#include <drx3D/Maths/Linear/Quat.h>

Vec3 SphereShape::localGetSupportingVertexWithoutMargin(const Vec3& vec) const
{
	(void)vec;
	return Vec3(Scalar(0.), Scalar(0.), Scalar(0.));
}

void SphereShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
	(void)vectors;

	for (i32 i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i].setVal(Scalar(0.), Scalar(0.), Scalar(0.));
	}
}

Vec3 SphereShape::localGetSupportingVertex(const Vec3& vec) const
{
	Vec3 supVertex;
	supVertex = localGetSupportingVertexWithoutMargin(vec);

	Vec3 vecnorm = vec;
	if (vecnorm.length2() < (SIMD_EPSILON * SIMD_EPSILON))
	{
		vecnorm.setVal(Scalar(-1.), Scalar(-1.), Scalar(-1.));
	}
	vecnorm.normalize();
	supVertex += getMargin() * vecnorm;
	return supVertex;
}

//broken due to scaling
void SphereShape::getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
{
	const Vec3& center = t.getOrigin();
	Vec3 extent(getMargin(), getMargin(), getMargin());
	aabbMin = center - extent;
	aabbMax = center + extent;
}

void SphereShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
	Scalar elem = Scalar(0.4) * mass * getMargin() * getMargin();
	inertia.setVal(elem, elem, elem);
}
