#ifndef GIMPACT_MASS_UTIL_H
#define GIMPACT_MASS_UTIL_H

#include <drx3D/Maths/Linear/Transform2.h>

SIMD_FORCE_INLINE Vec3 gim_inertia_add_transformed(
	const Vec3& source_inertia, const Vec3& added_inertia, const Transform2& transform)
{
	Matrix3x3 rotatedTensor = transform.getBasis().scaled(added_inertia) * transform.getBasis().transpose();

	Scalar x2 = transform.getOrigin()[0];
	x2 *= x2;
	Scalar y2 = transform.getOrigin()[1];
	y2 *= y2;
	Scalar z2 = transform.getOrigin()[2];
	z2 *= z2;

	Scalar ix = rotatedTensor[0][0] * (y2 + z2);
	Scalar iy = rotatedTensor[1][1] * (x2 + z2);
	Scalar iz = rotatedTensor[2][2] * (x2 + y2);

	return Vec3(source_inertia[0] + ix, source_inertia[1] + iy, source_inertia[2] + iz);
}

SIMD_FORCE_INLINE Vec3 gim_get_point_inertia(const Vec3& point, Scalar mass)
{
	Scalar x2 = point[0] * point[0];
	Scalar y2 = point[1] * point[1];
	Scalar z2 = point[2] * point[2];
	return Vec3(mass * (y2 + z2), mass * (x2 + z2), mass * (x2 + y2));
}

#endif  //GIMPACT_MESH_SHAPE_H
