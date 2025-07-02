
#ifndef DRX3D_GJP_EPA_PENETRATION_DEPTH_H
#define DRX3D_GJP_EPA_PENETRATION_DEPTH_H

#include <drx3D/Physics/Collision/NarrowPhase/ConvexPenetrationDepthSolver.h>

///EpaPenetrationDepthSolver uses the Expanding Polytope Algorithm to
///calculate the penetration depth between two convex shapes.
class GjkEpaPenetrationDepthSolver : public ConvexPenetrationDepthSolver
{
public:
	GjkEpaPenetrationDepthSolver()
	{
	}

	bool calcPenDepth(SimplexSolverInterface& simplexSolver,
					  const ConvexShape* pConvexA, const ConvexShape* pConvexB,
					  const Transform2& transformA, const Transform2& transformB,
					  Vec3& v, Vec3& wWitnessOnA, Vec3& wWitnessOnB,
					  class IDebugDraw* debugDraw);

private:
};

#endif  // DRX3D_GJP_EPA_PENETRATION_DEPTH_H
