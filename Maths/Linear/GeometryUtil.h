#ifndef DRX3D_GEOMETRY_UTIL_H
#define DRX3D_GEOMETRY_UTIL_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///The GeometryUtil helper class provides a few methods to convert between plane equations and vertices.
class GeometryUtil
{
public:
	static void getPlaneEquationsFromVertices(AlignedObjectArray<Vec3>& vertices, AlignedObjectArray<Vec3>& planeEquationsOut);

	static void getVerticesFromPlaneEquations(const AlignedObjectArray<Vec3>& planeEquations, AlignedObjectArray<Vec3>& verticesOut);

	static bool isInside(const AlignedObjectArray<Vec3>& vertices, const Vec3& planeNormal, Scalar margin);

	static bool isPointInsidePlanes(const AlignedObjectArray<Vec3>& planeEquations, const Vec3& point, Scalar margin);

	static bool areVerticesBehindPlane(const Vec3& planeNormal, const AlignedObjectArray<Vec3>& vertices, Scalar margin);
};

#endif  //DRX3D_GEOMETRY_UTIL_H
