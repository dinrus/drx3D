#include <drx3D/Physics/Collision/NarrowPhase/ContinuousConvexCollision.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/NarrowPhase/PointCollector.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>

ContinuousConvexCollision::ContinuousConvexCollision(const ConvexShape* convexA, const ConvexShape* convexB, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* penetrationDepthSolver)
	: m_simplexSolver(simplexSolver),
	  m_penetrationDepthSolver(penetrationDepthSolver),
	  m_convexA(convexA),
	  m_convexB1(convexB),
	  m_planeShape(0)
{
}

ContinuousConvexCollision::ContinuousConvexCollision(const ConvexShape* convexA, const StaticPlaneShape* plane)
	: m_simplexSolver(0),
	  m_penetrationDepthSolver(0),
	  m_convexA(convexA),
	  m_convexB1(0),
	  m_planeShape(plane)
{
}

/// This maximum should not be necessary. It allows for untested/degenerate cases in production code.
/// You don't want your game ever to lock-up.
#define MAX_ITERATIONS 64

void ContinuousConvexCollision::computeClosestPoints(const Transform2& transA, const Transform2& transB, PointCollector& pointCollector)
{
	if (m_convexB1)
	{
		m_simplexSolver->reset();
		GjkPairDetector gjk(m_convexA, m_convexB1, m_convexA->getShapeType(), m_convexB1->getShapeType(), m_convexA->getMargin(), m_convexB1->getMargin(), m_simplexSolver, m_penetrationDepthSolver);
		GjkPairDetector::ClosestPointInput input;
		input.m_transformA = transA;
		input.m_transformB = transB;
		gjk.getClosestPoints(input, pointCollector, 0);
	}
	else
	{
		//convex versus plane
		const ConvexShape* convexShape = m_convexA;
		const StaticPlaneShape* planeShape = m_planeShape;

		const Vec3& planeNormal = planeShape->getPlaneNormal();
		const Scalar& planeConstant = planeShape->getPlaneConstant();

		Transform2 convexWorldTransform = transA;
		Transform2 convexInPlaneTrans;
		convexInPlaneTrans = transB.inverse() * convexWorldTransform;
		Transform2 planeInConvex;
		planeInConvex = convexWorldTransform.inverse() * transB;

		Vec3 vtx = convexShape->localGetSupportingVertex(planeInConvex.getBasis() * -planeNormal);

		Vec3 vtxInPlane = convexInPlaneTrans(vtx);
		Scalar distance = (planeNormal.dot(vtxInPlane) - planeConstant);

		Vec3 vtxInPlaneProjected = vtxInPlane - distance * planeNormal;
		Vec3 vtxInPlaneWorld = transB * vtxInPlaneProjected;
		Vec3 normalOnSurfaceB = transB.getBasis() * planeNormal;

		pointCollector.addContactPoint(
			normalOnSurfaceB,
			vtxInPlaneWorld,
			distance);
	}
}

bool ContinuousConvexCollision::calcTimeOfImpact(
	const Transform2& fromA,
	const Transform2& toA,
	const Transform2& fromB,
	const Transform2& toB,
	CastResult& result)
{
	/// compute linear and angular velocity for this interval, to interpolate
	Vec3 linVelA, angVelA, linVelB, angVelB;
	Transform2Util::calculateVelocity(fromA, toA, Scalar(1.), linVelA, angVelA);
	Transform2Util::calculateVelocity(fromB, toB, Scalar(1.), linVelB, angVelB);

	Scalar boundingRadiusA = m_convexA->getAngularMotionDisc();
	Scalar boundingRadiusB = m_convexB1 ? m_convexB1->getAngularMotionDisc() : 0.f;

	Scalar maxAngularProjectedVelocity = angVelA.length() * boundingRadiusA + angVelB.length() * boundingRadiusB;
	Vec3 relLinVel = (linVelB - linVelA);

	Scalar relLinVelocLength = (linVelB - linVelA).length();

	if ((relLinVelocLength + maxAngularProjectedVelocity) == 0.f)
		return false;

	Scalar lambda = Scalar(0.);

	Vec3 n;
	n.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
	bool hasResult = false;
	Vec3 c;

	Scalar lastLambda = lambda;
	//Scalar epsilon = Scalar(0.001);

	i32 numIter = 0;
	//first solution, using GJK

	Scalar radius = 0.001f;
	//	result.drawCoordSystem(sphereTr);

	PointCollector pointCollector1;

	{
		computeClosestPoints(fromA, fromB, pointCollector1);

		hasResult = pointCollector1.m_hasResult;
		c = pointCollector1.m_pointInWorld;
	}

	if (hasResult)
	{
		Scalar dist;
		dist = pointCollector1.m_distance + result.m_allowedPenetration;
		n = pointCollector1.m_normalOnBInWorld;
		Scalar projectedLinearVelocity = relLinVel.dot(n);
		if ((projectedLinearVelocity + maxAngularProjectedVelocity) <= SIMD_EPSILON)
			return false;

		//not close enough
		while (dist > radius)
		{
			if (result.m_debugDrawer)
			{
				result.m_debugDrawer->drawSphere(c, 0.2f, Vec3(1, 1, 1));
			}
			Scalar dLambda = Scalar(0.);

			projectedLinearVelocity = relLinVel.dot(n);

			//don't report time of impact for motion away from the contact normal (or causes minor penetration)
			if ((projectedLinearVelocity + maxAngularProjectedVelocity) <= SIMD_EPSILON)
				return false;

			dLambda = dist / (projectedLinearVelocity + maxAngularProjectedVelocity);

			lambda += dLambda;

			if (lambda > Scalar(1.) || lambda < Scalar(0.))
				return false;

			//todo: next check with relative epsilon
			if (lambda <= lastLambda)
			{
				return false;
				//n.setVal(0,0,0);
				//break;
			}
			lastLambda = lambda;

			//interpolate to next lambda
			Transform2 interpolatedTransA, interpolatedTransB, relativeTrans;

			Transform2Util::integrateTransform(fromA, linVelA, angVelA, lambda, interpolatedTransA);
			Transform2Util::integrateTransform(fromB, linVelB, angVelB, lambda, interpolatedTransB);
			relativeTrans = interpolatedTransB.inverseTimes(interpolatedTransA);

			if (result.m_debugDrawer)
			{
				result.m_debugDrawer->drawSphere(interpolatedTransA.getOrigin(), 0.2f, Vec3(1, 0, 0));
			}

			result.DebugDraw(lambda);

			PointCollector pointCollector;
			computeClosestPoints(interpolatedTransA, interpolatedTransB, pointCollector);

			if (pointCollector.m_hasResult)
			{
				dist = pointCollector.m_distance + result.m_allowedPenetration;
				c = pointCollector.m_pointInWorld;
				n = pointCollector.m_normalOnBInWorld;
			}
			else
			{
				result.reportFailure(-1, numIter);
				return false;
			}

			numIter++;
			if (numIter > MAX_ITERATIONS)
			{
				result.reportFailure(-2, numIter);
				return false;
			}
		}

		result.m_fraction = lambda;
		result.m_normal = n;
		result.m_hitPoint = c;
		return true;
	}

	return false;
}
