#ifndef D3_GEOMETRY_UTIL_H
#define D3_GEOMETRY_UTIL_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

///The b3GeometryUtil helper class provides a few methods to convert between plane equations and vertices.
class b3GeometryUtil
{
public:
	static void getPlaneEquationsFromVertices(b3AlignedObjectArray<b3Vec3>& vertices, b3AlignedObjectArray<b3Vec3>& planeEquationsOut);

	static void getVerticesFromPlaneEquations(const b3AlignedObjectArray<b3Vec3>& planeEquations, b3AlignedObjectArray<b3Vec3>& verticesOut);

	static bool isInside(const b3AlignedObjectArray<b3Vec3>& vertices, const b3Vec3& planeNormal, b3Scalar margin);

	static bool isPointInsidePlanes(const b3AlignedObjectArray<b3Vec3>& planeEquations, const b3Vec3& point, b3Scalar margin);

	static bool areVerticesBehindPlane(const b3Vec3& planeNormal, const b3AlignedObjectArray<b3Vec3>& vertices, b3Scalar margin);
};

#endif  //D3_GEOMETRY_UTIL_H
