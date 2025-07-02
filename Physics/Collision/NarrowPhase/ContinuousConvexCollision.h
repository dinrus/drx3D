#ifndef DRX3D_CONTINUOUS_COLLISION_CONVEX_CAST_H
#define DRX3D_CONTINUOUS_COLLISION_CONVEX_CAST_H

#include <drx3D/Physics/Collision/NarrowPhase/ConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
class ConvexPenetrationDepthSolver;
class ConvexShape;
class StaticPlaneShape;

/// ContinuousConvexCollision implements angular and linear time of impact for convex objects.
/// Based on Brian Mirtich's Conservative Advancement idea (PhD thesis).
/// Algorithm operates in worldspace, in order to keep in between motion globally consistent.
/// It uses GJK at the moment. Future improvement would use minkowski sum / supporting vertex, merging innerloops
class ContinuousConvexCollision : public ConvexCast
{
	SimplexSolverInterface* m_simplexSolver;
	ConvexPenetrationDepthSolver* m_penetrationDepthSolver;
	const ConvexShape* m_convexA;
	//second object is either a convex or a plane (code sharing)
	const ConvexShape* m_convexB1;
	const StaticPlaneShape* m_planeShape;

	void computeClosestPoints(const Transform2& transA, const Transform2& transB, struct PointCollector& pointCollector);

public:
	ContinuousConvexCollision(const ConvexShape* shapeA, const ConvexShape* shapeB, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* penetrationDepthSolver);

	ContinuousConvexCollision(const ConvexShape* shapeA, const StaticPlaneShape* plane);

	virtual bool calcTimeOfImpact(
		const Transform2& fromA,
		const Transform2& toA,
		const Transform2& fromB,
		const Transform2& toB,
		CastResult& result);
};

#endif  //DRX3D_CONTINUOUS_COLLISION_CONVEX_CAST_H
