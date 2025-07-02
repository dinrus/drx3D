#ifndef DRX3D_GJK_CONVEX_CAST_H
#define DRX3D_GJK_CONVEX_CAST_H

#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Collision/NarrowPhase/ConvexCast.h>
class ConvexShape;
class MinkowskiSumShape;
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>

///GjkConvexCast performs a raycast on a convex object using support mapping.
class GjkConvexCast : public ConvexCast
{
	SimplexSolverInterface* m_simplexSolver;
	const ConvexShape* m_convexA;
	const ConvexShape* m_convexB;

public:
	GjkConvexCast(const ConvexShape* convexA, const ConvexShape* convexB, SimplexSolverInterface* simplexSolver);

	/// cast a convex against another convex object
	virtual bool calcTimeOfImpact(
		const Transform2& fromA,
		const Transform2& toA,
		const Transform2& fromB,
		const Transform2& toB,
		CastResult& result);
};

#endif  //DRX3D_GJK_CONVEX_CAST_H
