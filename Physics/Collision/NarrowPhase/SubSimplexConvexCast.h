
#ifndef DRX3D_SUBSIMPLEX_CONVEX_CAST_H
#define DRX3D_SUBSIMPLEX_CONVEX_CAST_H

#include <drx3D/Physics/Collision/NarrowPhase/ConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
class ConvexShape;

/// SubsimplexConvexCast implements Gino van den Bergens' paper
///"Ray Casting against bteral Convex Objects with Application to Continuous Collision Detection"
/// GJK based Ray Cast, optimized version
/// Objects should not start in overlap, otherwise results are not defined.
class SubsimplexConvexCast : public ConvexCast
{
	SimplexSolverInterface* m_simplexSolver;
	const ConvexShape* m_convexA;
	const ConvexShape* m_convexB;

public:
	SubsimplexConvexCast(const ConvexShape* shapeA, const ConvexShape* shapeB, SimplexSolverInterface* simplexSolver);

	//virtual ~SubsimplexConvexCast();
	///SimsimplexConvexCast calculateTimeOfImpact calculates the time of impact+normal for the linear cast (sweep) between two moving objects.
	///Precondition is that objects should not penetration/overlap at the start from the interval. Overlap can be tested using GjkPairDetector.
	virtual bool calcTimeOfImpact(
		const Transform2& fromA,
		const Transform2& toA,
		const Transform2& fromB,
		const Transform2& toB,
		CastResult& result);
};

#endif  //DRX3D_SUBSIMPLEX_CONVEX_CAST_H
