
#ifndef DRX3D_CONVEX_PENETRATION_DEPTH_H
#define DRX3D_CONVEX_PENETRATION_DEPTH_H

class Vec3;
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
class ConvexShape;
class Transform2;

///ConvexPenetrationDepthSolver provides an interface for penetration depth calculation.
class ConvexPenetrationDepthSolver
{
public:
	virtual ~ConvexPenetrationDepthSolver(){};
	virtual bool calcPenDepth(SimplexSolverInterface& simplexSolver,
							  const ConvexShape* convexA, const ConvexShape* convexB,
							  const Transform2& transA, const Transform2& transB,
							  Vec3& v, Vec3& pa, Vec3& pb,
							  class IDebugDraw* debugDraw) = 0;
};
#endif  //DRX3D_CONVEX_PENETRATION_DEPTH_H
