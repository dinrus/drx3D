#include "../ConvexConvexMprAlgorithm.h"

//#include <stdio.h>
#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
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
#include <drx3D/Physics/Collision/NarrowPhase/PolyhedralContactClipping.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>
#include <drx3D/Physics/Collision/NarrowPhase/ComputeGjkEpaPenetration.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa3.h>
#include <drx3D/Physics/Collision/NarrowPhase/MprPenetration.h>

//this is just an internal debug variable to switch between GJK+MPR or GJK+EPA
bool gUseMprCollisionFunction = true;

ConvexConvexMprAlgorithm::CreateFunc::CreateFunc()
{
}

ConvexConvexMprAlgorithm::CreateFunc::~CreateFunc()
{
}

ConvexConvexMprAlgorithm::ConvexConvexMprAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	: ActivatingCollisionAlgorithm(ci, body0Wrap, body1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf)
{
	(void)body0Wrap;
	(void)body1Wrap;
}

ConvexConvexMprAlgorithm::~ConvexConvexMprAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

Vec3 BulletShapeSupportFunc(ukk shapeAptr, const Vec3& dir, bool includeMargin)
{
	ConvexShape* shape = (ConvexShape*)shapeAptr;
	if (includeMargin)
	{
		return shape->localGetSupportingVertex(dir);
	}

	return shape->localGetSupportingVertexWithoutMargin(dir);
}

Vec3 BulletShapeCenterFunc(ukk shapeAptr)
{
	return Vec3(0, 0, 0);
}

struct MprConvexWrap
{
	const ConvexShape* m_convex;
	Transform2 m_worldTrans;
	inline Scalar getMargin() const
	{
		return m_convex->getMargin();
	}
	inline Vec3 getObjectCenterInWorld() const
	{
		return m_worldTrans.getOrigin();
	}
	inline const Transform2& getWorldTransform() const
	{
		return m_worldTrans;
	}
	inline Vec3 getLocalSupportWithMargin(const Vec3& dir) const
	{
		return m_convex->localGetSupportingVertex(dir);
	}
	inline Vec3 getLocalSupportWithoutMargin(const Vec3& dir) const
	{
		return m_convex->localGetSupportingVertexWithoutMargin(dir);
	}
};

struct MyDistanceInfo
{
	Vec3 m_pointOnA;
	Vec3 m_pointOnB;
	Vec3 m_normalBtoA;
	Scalar m_distance;
};

//
// Convex-Convex collision algorithm
//
void ConvexConvexMprAlgorithm ::processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
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

	GjkPairDetector::ClosestPointInput input;

	VoronoiSimplexSolver vs;
	GjkEpaPenetrationDepthSolver epa;

	if (gUseMprCollisionFunction)
	{
		MprConvexWrap a, b;
		a.m_worldTrans = body0Wrap->getWorldTransform();
		b.m_worldTrans = body1Wrap->getWorldTransform();
		a.m_convex = (const ConvexShape*)body0Wrap->getCollisionShape();
		b.m_convex = (const ConvexShape*)body1Wrap->getCollisionShape();
		VoronoiSimplexSolver simplexSolver;
		simplexSolver.reset();
		GjkCollisionDescription colDesc;
		MyDistanceInfo distInfo;
		i32 res = ComputeGjkDistance(a, b, colDesc, &distInfo);
		if (res == 0)
		{
			//printf("use GJK results in distance %f\n",distInfo.m_distance);
		}
		else
		{
			MprCollisionDescription mprDesc;
			res = ComputeMprPenetration(a, b, mprDesc, &distInfo);

			//printf("use MPR results in distance %f\n",distInfo.m_distance);
		}
		if (res == 0)
		{
#if 0
			printf("Dist=%f,normalOnB[%f,%f,%f],pA=[%f,%f,%f],pB[%f,%f,%f]\n",
				distInfo.m_distance, distInfo.m_normalBtoA[0], distInfo.m_normalBtoA[1], distInfo.m_normalBtoA[2],
				distInfo.m_pointOnA[0], distInfo.m_pointOnA[1], distInfo.m_pointOnA[2],
				distInfo.m_pointOnB[0], distInfo.m_pointOnB[1], distInfo.m_pointOnB[2]);
#endif

			if (distInfo.m_distance <= 0)
			{
				resultOut->addContactPoint(distInfo.m_normalBtoA, distInfo.m_pointOnB, distInfo.m_distance);
			}
			//ASSERT_EQ(0,result);
			//ASSERT_NEAR(Fabs(Scalar(i-z))-Scalar(j)-ssd.m_radiusB, distInfo.m_distance, abs_error);
			//Vec3 computedA = distInfo.m_pointOnB+distInfo.m_distance*distInfo.m_normalBtoA;
			//ASSERT_NEAR(computedA.x(),distInfo.m_pointOnA.x(),abs_error);
			//ASSERT_NEAR(computedA.y(),distInfo.m_pointOnA.y(),abs_error);
			//ASSERT_NEAR(computedA.z(),distInfo.m_pointOnA.z(),abs_error);
		}

#if 0
		CollisionDescription colDesc;
		colDesc.m_objA = min0;
		colDesc.m_objB = min1;
		colDesc.m_localSupportFuncA = &BulletShapeSupportFunc;
		colDesc.m_localSupportFuncB = &BulletShapeSupportFunc;
		colDesc.m_localOriginFuncA = &BulletShapeCenterFunc;
		colDesc.m_localOriginFuncB = &BulletShapeCenterFunc;

		colDesc.m_transformA = body0Wrap->getWorldTransform();
		colDesc.m_transformB = body1Wrap->getWorldTransform();
		colDesc.m_marginA = body0Wrap->getCollisionShape()->getMargin();
		colDesc.m_marginB = body1Wrap->getCollisionShape()->getMargin();
		btDistanceInfo distInfo;
		//i32	result = ComputeGjkEpaPenetration(colDesc, &distInfo);
		//i32	result = ComputeGjkEpaPenetration2(colDesc, &distInfo);
		i32	result = ComputeMprPenetration(colDesc, &distInfo);
		
		if (result==0)
		{
			resultOut->addContactPoint(distInfo.m_normalBtoA,distInfo.m_pointOnB,distInfo.m_distance);
		}

		//bool res = b3MprPenetration(pairIndex,bodyIndexA,bodyIndexB,cpuBodyBuf,convexData,collidable2,cpuVertices,sepAxis,hasSepAxis,depthOut,dirOut,posOut);
	
		/*CollisionDescription colDesc;
		btDistanceInfo distInfo;
		i32	ComputeGjkEpaPenetration(min0, min1, &colDesc, &distInfo);
		*/
#endif
	}
	else
	{
		GjkPairDetector gjkPairDetector(min0, min1, &vs, &epa);  //m_simplexSolver,m_pdSolver);
		//TODO: if (dispatchInfo.m_useContinuous)
		gjkPairDetector.setMinkowskiA(min0);
		gjkPairDetector.setMinkowskiB(min1);

		{
			//if (dispatchInfo.m_convexMaxDistanceUseCPT)
			//{
			//	input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactProcessingThreshold();
			//} else
			//{
			input.m_maximumDistanceSquared = min0->getMargin() + min1->getMargin() + m_manifoldPtr->getContactBreakingThreshold();
			//		}

			input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;
		}

		input.m_transformA = body0Wrap->getWorldTransform();
		input.m_transformB = body1Wrap->getWorldTransform();

		gjkPairDetector.getClosestPoints(input, *resultOut, dispatchInfo.m_debugDraw);
	}
	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
}

Scalar ConvexConvexMprAlgorithm::calculateTimeOfImpact(CollisionObject2* col0, CollisionObject2* col1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	Assert(0);
	return 0;
}
