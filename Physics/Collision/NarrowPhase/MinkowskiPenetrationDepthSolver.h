#ifndef DRX3D_MINKOWSKI_PENETRATION_DEPTH_SOLVER_H
#define DRX3D_MINKOWSKI_PENETRATION_DEPTH_SOLVER_H

#include <drx3D/Physics/Collision/NarrowPhase/ConvexPenetrationDepthSolver.h>

///MinkowskiPenetrationDepthSolver implements bruteforce penetration depth estimation.
///Implementation is based on sampling the depth using support mapping, and using GJK step to get the witness points.
class MinkowskiPenetrationDepthSolver : public ConvexPenetrationDepthSolver
{
protected:
	static Vec3* getPenetrationDirections();

public:
	virtual bool calcPenDepth(SimplexSolverInterface& simplexSolver,
							  const ConvexShape* convexA, const ConvexShape* convexB,
							  const Transform2& transA, const Transform2& transB,
							  Vec3& v, Vec3& pa, Vec3& pb,
							  class IDebugDraw* debugDraw);
};

#endif  //DRX3D_MINKOWSKI_PENETRATION_DEPTH_SOLVER_H
