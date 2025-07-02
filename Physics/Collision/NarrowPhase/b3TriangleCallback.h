#ifndef D3_TRIANGLE_CALLBACK_H
#define D3_TRIANGLE_CALLBACK_H

#include <drx3D/Common/b3Vec3.h>

///The b3TriangleCallback provides a callback for each overlapping triangle when calling processAllTriangles.
///This callback is called by processAllTriangles for all b3ConcaveShape derived class, such as  b3BvhTriangleMeshShape, b3StaticPlaneShape and b3HeightfieldTerrainShape.
class b3TriangleCallback
{
public:
	virtual ~b3TriangleCallback();
	virtual void processTriangle(b3Vec3* triangle, i32 partId, i32 triangleIndex) = 0;
};

class b3InternalTriangleIndexCallback
{
public:
	virtual ~b3InternalTriangleIndexCallback();
	virtual void internalProcessTriangleIndex(b3Vec3* triangle, i32 partId, i32 triangleIndex) = 0;
};

#endif  //D3_TRIANGLE_CALLBACK_H
