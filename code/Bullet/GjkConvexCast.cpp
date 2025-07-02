
#include <drx3D/Physics/Collision/NarrowPhase/GjkConvexCast.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/NarrowPhase/PointCollector.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define MAX_ITERATIONS 64
#else
#define MAX_ITERATIONS 32
#endif

GjkConvexCast::GjkConvexCast(const ConvexShape* convexA, const ConvexShape* convexB, SimplexSolverInterface* simplexSolver)
	: m_simplexSolver(simplexSolver),
	  m_convexA(convexA),
	  m_convexB(convexB)
{
}

bool GjkConvexCast::calcTimeOfImpact(
	const Transform2& fromA,
	const Transform2& toA,
	const Transform2& fromB,
	const Transform2& toB,
	CastResult& result)
{
	m_simplexSolver->reset();

	/// compute linear velocity for this interval, to interpolate
	//assume no rotation/angular velocity, assert here?
	Vec3 linVelA, linVelB;
	linVelA = toA.getOrigin() - fromA.getOrigin();
	linVelB = toB.getOrigin() - fromB.getOrigin();

	Scalar radius = Scalar(0.001);
	Scalar lambda = Scalar(0.);
	Vec3 v(1, 0, 0);

	i32 maxIter = MAX_ITERATIONS;

	Vec3 n;
	n.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
	bool hasResult = false;
	Vec3 c;
	Vec3 r = (linVelA - linVelB);

	Scalar lastLambda = lambda;
	//Scalar epsilon = Scalar(0.001);

	i32 numIter = 0;
	//first solution, using GJK

	Transform2 identityTrans;
	identityTrans.setIdentity();

	//	result.drawCoordSystem(sphereTr);

	PointCollector pointCollector;

	GjkPairDetector gjk(m_convexA, m_convexB, m_simplexSolver, 0);  //m_penetrationDepthSolver);
	GjkPairDetector::ClosestPointInput input;

	//we don't use margins during CCD
	//	gjk.setIgnoreMargin(true);

	input.m_transformA = fromA;
	input.m_transformB = fromB;
	gjk.getClosestPoints(input, pointCollector, 0);

	hasResult = pointCollector.m_hasResult;
	c = pointCollector.m_pointInWorld;

	if (hasResult)
	{
		Scalar dist;
		dist = pointCollector.m_distance;
		n = pointCollector.m_normalOnBInWorld;

		//not close enough
		while (dist > radius)
		{
			numIter++;
			if (numIter > maxIter)
			{
				return false;  //todo: report a failure
			}
			Scalar dLambda = Scalar(0.);

			Scalar projectedLinearVelocity = r.dot(n);

			dLambda = dist / (projectedLinearVelocity);

			lambda = lambda - dLambda;

			if (lambda > Scalar(1.))
				return false;

			if (lambda < Scalar(0.))
				return false;

			//todo: next check with relative epsilon
			if (lambda <= lastLambda)
			{
				return false;
				//n.setVal(0,0,0);
				break;
			}
			lastLambda = lambda;

			//interpolate to next lambda
			result.DebugDraw(lambda);
			input.m_transformA.getOrigin().setInterpolate3(fromA.getOrigin(), toA.getOrigin(), lambda);
			input.m_transformB.getOrigin().setInterpolate3(fromB.getOrigin(), toB.getOrigin(), lambda);

			gjk.getClosestPoints(input, pointCollector, 0);
			if (pointCollector.m_hasResult)
			{
				if (pointCollector.m_distance < Scalar(0.))
				{
					result.m_fraction = lastLambda;
					n = pointCollector.m_normalOnBInWorld;
					result.m_normal = n;
					result.m_hitPoint = pointCollector.m_pointInWorld;
					return true;
				}
				c = pointCollector.m_pointInWorld;
				n = pointCollector.m_normalOnBInWorld;
				dist = pointCollector.m_distance;
			}
			else
			{
				//??
				return false;
			}
		}

		//is n normalized?
		//don't report time of impact for motion away from the contact normal (or causes minor penetration)
		if (n.dot(r) >= -result.m_allowedPenetration)
			return false;

		result.m_fraction = lambda;
		result.m_normal = n;
		result.m_hitPoint = c;
		return true;
	}

	return false;
}
