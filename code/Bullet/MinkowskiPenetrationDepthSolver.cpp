#include <drx3D/Physics/Collision/NarrowPhase/MinkowskiPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>

#define NUM_UNITSPHERE_POINTS 42

bool MinkowskiPenetrationDepthSolver::calcPenDepth(SimplexSolverInterface& simplexSolver,
													 const ConvexShape* convexA, const ConvexShape* convexB,
													 const Transform2& transA, const Transform2& transB,
													 Vec3& v, Vec3& pa, Vec3& pb,
													 class IDebugDraw* debugDraw)
{
	(void)v;

	bool check2d = convexA->isConvex2d() && convexB->isConvex2d();

	struct IntermediateResult : public DiscreteCollisionDetectorInterface::Result
	{
		IntermediateResult() : m_hasResult(false)
		{
		}

		Vec3 m_normalOnBInWorld;
		Vec3 m_pointInWorld;
		Scalar m_depth;
		bool m_hasResult;

		virtual void setShapeIdentifiersA(i32 partId0, i32 index0)
		{
			(void)partId0;
			(void)index0;
		}
		virtual void setShapeIdentifiersB(i32 partId1, i32 index1)
		{
			(void)partId1;
			(void)index1;
		}
		void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
		{
			m_normalOnBInWorld = normalOnBInWorld;
			m_pointInWorld = pointInWorld;
			m_depth = depth;
			m_hasResult = true;
		}
	};

	//just take fixed number of orientation, and sample the penetration depth in that direction
	Scalar minProj = Scalar(DRX3D_LARGE_FLOAT);
	Vec3 minNorm(Scalar(0.), Scalar(0.), Scalar(0.));
	Vec3 minA, minB;
	Vec3 separatingAxisInA, separatingAxisInB;
	Vec3 pInA, qInB, pWorld, qWorld, w;

#ifndef __SPU__
#define USE_BATCHED_SUPPORT 1
#endif
#ifdef USE_BATCHED_SUPPORT

	Vec3 supportVerticesABatch[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2];
	Vec3 supportVerticesBBatch[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2];
	Vec3 separatingAxisInABatch[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2];
	Vec3 separatingAxisInBBatch[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2];
	i32 i;

	i32 numSampleDirections = NUM_UNITSPHERE_POINTS;

	for (i = 0; i < numSampleDirections; i++)
	{
		Vec3 norm = getPenetrationDirections()[i];
		separatingAxisInABatch[i] = (-norm) * transA.getBasis();
		separatingAxisInBBatch[i] = norm * transB.getBasis();
	}

	{
		i32 numPDA = convexA->getNumPreferredPenetrationDirections();
		if (numPDA)
		{
			for (i32 i = 0; i < numPDA; i++)
			{
				Vec3 norm;
				convexA->getPreferredPenetrationDirection(i, norm);
				norm = transA.getBasis() * norm;
				getPenetrationDirections()[numSampleDirections] = norm;
				separatingAxisInABatch[numSampleDirections] = (-norm) * transA.getBasis();
				separatingAxisInBBatch[numSampleDirections] = norm * transB.getBasis();
				numSampleDirections++;
			}
		}
	}

	{
		i32 numPDB = convexB->getNumPreferredPenetrationDirections();
		if (numPDB)
		{
			for (i32 i = 0; i < numPDB; i++)
			{
				Vec3 norm;
				convexB->getPreferredPenetrationDirection(i, norm);
				norm = transB.getBasis() * norm;
				getPenetrationDirections()[numSampleDirections] = norm;
				separatingAxisInABatch[numSampleDirections] = (-norm) * transA.getBasis();
				separatingAxisInBBatch[numSampleDirections] = norm * transB.getBasis();
				numSampleDirections++;
			}
		}
	}

	convexA->batchedUnitVectorGetSupportingVertexWithoutMargin(separatingAxisInABatch, supportVerticesABatch, numSampleDirections);
	convexB->batchedUnitVectorGetSupportingVertexWithoutMargin(separatingAxisInBBatch, supportVerticesBBatch, numSampleDirections);

	for (i = 0; i < numSampleDirections; i++)
	{
		Vec3 norm = getPenetrationDirections()[i];
		if (check2d)
		{
			norm[2] = 0.f;
		}
		if (norm.length2() > 0.01)
		{
			separatingAxisInA = separatingAxisInABatch[i];
			separatingAxisInB = separatingAxisInBBatch[i];

			pInA = supportVerticesABatch[i];
			qInB = supportVerticesBBatch[i];

			pWorld = transA(pInA);
			qWorld = transB(qInB);
			if (check2d)
			{
				pWorld[2] = 0.f;
				qWorld[2] = 0.f;
			}

			w = qWorld - pWorld;
			Scalar delta = norm.dot(w);
			//find smallest delta
			if (delta < minProj)
			{
				minProj = delta;
				minNorm = norm;
				minA = pWorld;
				minB = qWorld;
			}
		}
	}
#else

	i32 numSampleDirections = NUM_UNITSPHERE_POINTS;

#ifndef __SPU__
	{
		i32 numPDA = convexA->getNumPreferredPenetrationDirections();
		if (numPDA)
		{
			for (i32 i = 0; i < numPDA; i++)
			{
				Vec3 norm;
				convexA->getPreferredPenetrationDirection(i, norm);
				norm = transA.getBasis() * norm;
				getPenetrationDirections()[numSampleDirections] = norm;
				numSampleDirections++;
			}
		}
	}

	{
		i32 numPDB = convexB->getNumPreferredPenetrationDirections();
		if (numPDB)
		{
			for (i32 i = 0; i < numPDB; i++)
			{
				Vec3 norm;
				convexB->getPreferredPenetrationDirection(i, norm);
				norm = transB.getBasis() * norm;
				getPenetrationDirections()[numSampleDirections] = norm;
				numSampleDirections++;
			}
		}
	}
#endif  // __SPU__

	for (i32 i = 0; i < numSampleDirections; i++)
	{
		const Vec3& norm = getPenetrationDirections()[i];
		separatingAxisInA = (-norm) * transA.getBasis();
		separatingAxisInB = norm * transB.getBasis();
		pInA = convexA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
		qInB = convexB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);
		pWorld = transA(pInA);
		qWorld = transB(qInB);
		w = qWorld - pWorld;
		Scalar delta = norm.dot(w);
		//find smallest delta
		if (delta < minProj)
		{
			minProj = delta;
			minNorm = norm;
			minA = pWorld;
			minB = qWorld;
		}
	}
#endif  //USE_BATCHED_SUPPORT

	//add the margins

	minA += minNorm * convexA->getMarginNonVirtual();
	minB -= minNorm * convexB->getMarginNonVirtual();
	//no penetration
	if (minProj < Scalar(0.))
		return false;

	Scalar extraSeparation = 0.5f;  ///scale dependent
	minProj += extraSeparation + (convexA->getMarginNonVirtual() + convexB->getMarginNonVirtual());

//#define DEBUG_DRAW 1
#ifdef DEBUG_DRAW
	if (debugDraw)
	{
		Vec3 color(0, 1, 0);
		debugDraw->drawLine(minA, minB, color);
		color = Vec3(1, 1, 1);
		Vec3 vec = minB - minA;
		Scalar prj2 = minNorm.dot(vec);
		debugDraw->drawLine(minA, minA + (minNorm * minProj), color);
	}
#endif  //DEBUG_DRAW

	GjkPairDetector gjkdet(convexA, convexB, &simplexSolver, 0);

	Scalar offsetDist = minProj;
	Vec3 offset = minNorm * offsetDist;

	GjkPairDetector::ClosestPointInput input;

	Vec3 newOrg = transA.getOrigin() + offset;

	Transform2 displacedTrans = transA;
	displacedTrans.setOrigin(newOrg);

	input.m_transformA = displacedTrans;
	input.m_transformB = transB;
	input.m_maximumDistanceSquared = Scalar(DRX3D_LARGE_FLOAT);  //minProj;

	IntermediateResult res;
	gjkdet.setCachedSeparatingAxis(-minNorm);
	gjkdet.getClosestPoints(input, res, debugDraw);

	Scalar correctedMinNorm = minProj - res.m_depth;

	//the penetration depth is over-estimated, relax it
	Scalar penetration_relaxation = Scalar(1.);
	minNorm *= penetration_relaxation;

	if (res.m_hasResult)
	{
		pa = res.m_pointInWorld - minNorm * correctedMinNorm;
		pb = res.m_pointInWorld;
		v = minNorm;

#ifdef DEBUG_DRAW
		if (debugDraw)
		{
			Vec3 color(1, 0, 0);
			debugDraw->drawLine(pa, pb, color);
		}
#endif  //DEBUG_DRAW
	}
	return res.m_hasResult;
}

Vec3* MinkowskiPenetrationDepthSolver::getPenetrationDirections()
{
	static Vec3 sPenetrationDirections[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2] =
		{
			Vec3(Scalar(0.000000), Scalar(-0.000000), Scalar(-1.000000)),
			Vec3(Scalar(0.723608), Scalar(-0.525725), Scalar(-0.447219)),
			Vec3(Scalar(-0.276388), Scalar(-0.850649), Scalar(-0.447219)),
			Vec3(Scalar(-0.894426), Scalar(-0.000000), Scalar(-0.447216)),
			Vec3(Scalar(-0.276388), Scalar(0.850649), Scalar(-0.447220)),
			Vec3(Scalar(0.723608), Scalar(0.525725), Scalar(-0.447219)),
			Vec3(Scalar(0.276388), Scalar(-0.850649), Scalar(0.447220)),
			Vec3(Scalar(-0.723608), Scalar(-0.525725), Scalar(0.447219)),
			Vec3(Scalar(-0.723608), Scalar(0.525725), Scalar(0.447219)),
			Vec3(Scalar(0.276388), Scalar(0.850649), Scalar(0.447219)),
			Vec3(Scalar(0.894426), Scalar(0.000000), Scalar(0.447216)),
			Vec3(Scalar(-0.000000), Scalar(0.000000), Scalar(1.000000)),
			Vec3(Scalar(0.425323), Scalar(-0.309011), Scalar(-0.850654)),
			Vec3(Scalar(-0.162456), Scalar(-0.499995), Scalar(-0.850654)),
			Vec3(Scalar(0.262869), Scalar(-0.809012), Scalar(-0.525738)),
			Vec3(Scalar(0.425323), Scalar(0.309011), Scalar(-0.850654)),
			Vec3(Scalar(0.850648), Scalar(-0.000000), Scalar(-0.525736)),
			Vec3(Scalar(-0.525730), Scalar(-0.000000), Scalar(-0.850652)),
			Vec3(Scalar(-0.688190), Scalar(-0.499997), Scalar(-0.525736)),
			Vec3(Scalar(-0.162456), Scalar(0.499995), Scalar(-0.850654)),
			Vec3(Scalar(-0.688190), Scalar(0.499997), Scalar(-0.525736)),
			Vec3(Scalar(0.262869), Scalar(0.809012), Scalar(-0.525738)),
			Vec3(Scalar(0.951058), Scalar(0.309013), Scalar(0.000000)),
			Vec3(Scalar(0.951058), Scalar(-0.309013), Scalar(0.000000)),
			Vec3(Scalar(0.587786), Scalar(-0.809017), Scalar(0.000000)),
			Vec3(Scalar(0.000000), Scalar(-1.000000), Scalar(0.000000)),
			Vec3(Scalar(-0.587786), Scalar(-0.809017), Scalar(0.000000)),
			Vec3(Scalar(-0.951058), Scalar(-0.309013), Scalar(-0.000000)),
			Vec3(Scalar(-0.951058), Scalar(0.309013), Scalar(-0.000000)),
			Vec3(Scalar(-0.587786), Scalar(0.809017), Scalar(-0.000000)),
			Vec3(Scalar(-0.000000), Scalar(1.000000), Scalar(-0.000000)),
			Vec3(Scalar(0.587786), Scalar(0.809017), Scalar(-0.000000)),
			Vec3(Scalar(0.688190), Scalar(-0.499997), Scalar(0.525736)),
			Vec3(Scalar(-0.262869), Scalar(-0.809012), Scalar(0.525738)),
			Vec3(Scalar(-0.850648), Scalar(0.000000), Scalar(0.525736)),
			Vec3(Scalar(-0.262869), Scalar(0.809012), Scalar(0.525738)),
			Vec3(Scalar(0.688190), Scalar(0.499997), Scalar(0.525736)),
			Vec3(Scalar(0.525730), Scalar(0.000000), Scalar(0.850652)),
			Vec3(Scalar(0.162456), Scalar(-0.499995), Scalar(0.850654)),
			Vec3(Scalar(-0.425323), Scalar(-0.309011), Scalar(0.850654)),
			Vec3(Scalar(-0.425323), Scalar(0.309011), Scalar(0.850654)),
			Vec3(Scalar(0.162456), Scalar(0.499995), Scalar(0.850654))};

	return sPenetrationDirections;
}
