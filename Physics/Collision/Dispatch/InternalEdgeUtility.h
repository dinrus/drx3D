
#ifndef DRX3D_INTERNAL_EDGE_UTILITY_H
#define DRX3D_INTERNAL_EDGE_UTILITY_H

#include <drx3D/Maths/Linear/HashMap.h>
#include <drx3D/Maths/Linear/Vec3.h>

#include <drx3D/Physics/Collision/Shapes/TriangleInfoMap.h>

///The InternalEdgeUtility helps to avoid or reduce artifacts due to wrong collision normals caused by internal edges.
///See also http://code.google.com/p/bullet/issues/detail?id=27

class BvhTriangleMeshShape;
class CollisionObject2;
struct CollisionObject2Wrapper;
class ManifoldPoint;
class IDebugDraw;
class HeightfieldTerrainShape;

enum InternalEdgeAdjustFlags
{
	DRX3D_TRIANGLE_CONVEX_BACKFACE_MODE = 1,
	DRX3D_TRIANGLE_CONCAVE_DOUBLE_SIDED = 2,  //double sided options are experimental, single sided is recommended
	DRX3D_TRIANGLE_CONVEX_DOUBLE_SIDED = 4
};

///Call GenerateInternalEdgeInfo to create triangle info, store in the shape 'userInfo'
void GenerateInternalEdgeInfo(BvhTriangleMeshShape* trimeshShape, TriangleInfoMap* triangleInfoMap);

void GenerateInternalEdgeInfo(HeightfieldTerrainShape* trimeshShape, TriangleInfoMap* triangleInfoMap);

///Call the FixMeshNormal to adjust the collision normal, using the triangle info map (generated using GenerateInternalEdgeInfo)
///If this info map is missing, or the triangle is not store in this map, nothing will be done
void AdjustInternalEdgeContacts(ManifoldPoint& cp, const CollisionObject2Wrapper* trimeshColObj0Wrap, const CollisionObject2Wrapper* otherColObj1Wrap, i32 partId0, i32 index0, i32 normalAdjustFlags = 0);

///Enable the DRX3D_INTERNAL_EDGE_DEBUG_DRAW define and call SetDebugDrawer, to get visual info to see if the internal edge utility works properly.
///If the utility doesn't work properly, you might have to adjust the threshold values in TriangleInfoMap
//#define DRX3D_INTERNAL_EDGE_DEBUG_DRAW

#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
void SetDebugDrawer(IDebugDraw* debugDrawer);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

#endif  //DRX3D_INTERNAL_EDGE_UTILITY_H
