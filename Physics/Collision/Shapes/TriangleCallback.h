#ifndef DRX3D_TRIANGLE_CALLBACK_H
#define DRX3D_TRIANGLE_CALLBACK_H

#include <drx3D/Maths/Linear/Vec3.h>

///The TriangleCallback provides a callback for each overlapping triangle when calling processAllTriangles.
///This callback is called by processAllTriangles for all ConcaveShape derived class, such as  BvhTriangleMeshShape, StaticPlaneShape and HeightfieldTerrainShape.
class TriangleCallback
{
public:
	virtual ~TriangleCallback();
	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex) = 0;
};

class InternalTriangleIndexCallback
{
public:
	virtual ~InternalTriangleIndexCallback();
	virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex) = 0;
};

#endif  //DRX3D_TRIANGLE_CALLBACK_H
