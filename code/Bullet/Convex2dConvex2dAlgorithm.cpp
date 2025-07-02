#include <drx3D/Physics/Collision/Dispatch/Convex2dConvex2dAlgorithm.h>

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>

#include <drx3D/Physics/Collision/NarrowPhase/ConvexPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/ContinuousConvexCollision.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkConvexCast.h>

#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

#include <drx3D/Physics/Collision/NarrowPhase/MinkowskiPenetrationDepthSolver.h>

#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

Convex2dConvex2dAlgorithm::CreateFunc::CreateFunc(SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* pdSolver)
{
	m_simplexSolver = simplexSolver;
	m_pdSolver = pdSolver;
}

Convex2dConvex2dAlgorithm::CreateFunc::~CreateFunc()
{
}

Convex2dConvex2dAlgorithm::Convex2dConvex2dAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* pdSolver, i32 /* numPerturbationIterations */, i32 /* minimumPointsPerturbationThreshold */)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_simplexSolver(simplexSolver),
	  m_pdSolver(pdSolver),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_lowLevelOfDetail(false)
{
	(void)body0Wrap;
	(void)body1Wrap;
}

Convex2dConvex2dAlgorithm::~Convex2dConvex2dAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void Convex2dConvex2dAlgorithm ::setLowLevelOfDetail(bool useLowLevel)
{
	m_lowLevelOfDetail = useLowLevel;
}

extern Scalar gContactBreakingThreshold;

//
// Convex-Convex collision algorithm
//
void Convex2dConvex2dAlgorithm ::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
	{
		//swapped?
		m_manifoldPtr = m_dispatcher->getNewManifold(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject());
		m_ownManifold = true;
	}
	resultOut->setPersistentManifold(m_manifoldPtr);

	//comment-out next line to test multi-contact generation
	//resultOut->getPersistentManifold()->clearManifold();

	const ConvexShape* min0 = static_cast<const ConvexShape*>(body0Wrap->getCollisionShape());
	const ConvexShape* min1 = static_cast<const ConvexShape*>(body1Wrap->getCollisionShape());

	Vec3 normalOnB;
	Vec3 pointOnBWorld;

	{
		GjkPairDetector::ClosestPointInput input;

		GjkPairDetector gjkPairDetector(min0, min1, m_simplexSolver, m_pdSolver);
		//TODO: if (dispatchInfo.m_useContinuous)
		gjkPairDetector.setMinkowskiA(min0);
		gjkPairDetector.setMinkowskiB(min1);

		{
			input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactBreakingThreshold();
			input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;
		}

		input.m_transformA = body0Wrap->getWorldTransform();
		input.m_transformB = body1Wrap->getWorldTransform();

		gjkPairDetector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);

		Vec3 v0, v1;
		Vec3 sepNormalWorldSpace;
	}

	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
}

Scalar Convex2dConvex2dAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	///Rather then checking ALL pairs, only calculate TOI when motion exceeds threshold

	///Linear motion for one of objects needs to exceed m_ccdSquareMotionThreshold
	///col0->m_worldTransform,
	Scalar resultFraction = Scalar(1.);

	Scalar squareMot0 = (col0->getInterpolationWorldTransform().getOrigin() - col0->getWorldTransform().getOrigin()).length2();
	Scalar squareMot1 = (col1->getInterpolationWorldTransform().getOrigin() - col1->getWorldTransform().getOrigin()).length2();

	if (squareMot0 < col0->getCcdSquareMotionThreshold() &&
		squareMot1 < col1->getCcdSquareMotionThreshold())
		return resultFraction;

	//An adhoc way of testing the Continuous Collision Detection algorithms
	//One object is approximated as a sphere, to simplify things
	//Starting in penetration should report no time of impact
	//For proper CCD, better accuracy and handling of 'allowed' penetration should be added
	//also the mainloop of the physics should have a kind of toi queue (something like Brian Mirtich's application of Timewarp for Rigidbodies)

	/// Convex0 against sphere for Convex1
	{
		ConvexShape* convex0 = static_cast<ConvexShape*>(col0->getCollisionShape());

		SphereShape sphere1(col1->getCcdSweptSphereRadius());  //todo: allow non-zero sphere sizes, for better approximation
		ConvexCast::CastResult result;
		VoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		GjkConvexCast ccd1(convex0, &sphere1, &voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(), col0->getInterpolationWorldTransform(),
								  col1->getWorldTransform(), col1->getInterpolationWorldTransform(), result))
		{
			//store result.m_fraction in both bodies

			if (col0->getHitFraction() > result.m_fraction)
				col0->setHitFraction(result.m_fraction);

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction(result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;
		}
	}

	/// Sphere (for convex0) against Convex1
	{
		ConvexShape* convex1 = static_cast<ConvexShape*>(col1->getCollisionShape());

		SphereShape sphere0(col0->getCcdSweptSphereRadius());  //todo: allow non-zero sphere sizes, for better approximation
		ConvexCast::CastResult result;
		VoronoiSimplexSolver voronoiSimplex;
		//SubsimplexConvexCast ccd0(&sphere,min0,&voronoiSimplex);
		///Simplification, one object is simplified as a sphere
		GjkConvexCast ccd1(&sphere0, convex1, &voronoiSimplex);
		//ContinuousConvexCollision ccd(min0,min1,&voronoiSimplex,0);
		if (ccd1.calcTimeOfImpact(col0->getWorldTransform(), col0->getInterpolationWorldTransform(),
								  col1->getWorldTransform(), col1->getInterpolationWorldTransform(), result))
		{
			//store result.m_fraction in both bodies

			if (col0->getHitFraction() > result.m_fraction)
				col0->setHitFraction(result.m_fraction);

			if (col1->getHitFraction() > result.m_fraction)
				col1->setHitFraction(result.m_fraction);

			if (resultFraction > result.m_fraction)
				resultFraction = result.m_fraction;
		}
	}

	return resultFraction;
}
