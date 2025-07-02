#ifndef DRX3D_CONVEX_CONVEX_MPR_ALGORITHM_H
#define DRX3D_CONVEX_CONVEX_MPR_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Maths/Linear/Transform2Util.h>  //for ConvexSeparatingDistanceUtil

class ConvexPenetrationDepthSolver;

///Enabling USE_SEPDISTANCE_UTIL2 requires 100% reliable distance computation. However, when using large size ratios GJK can be imprecise
///so the distance is not conservative. In that case, enabling this USE_SEPDISTANCE_UTIL2 would result in failing/missing collisions.
///Either improve GJK for large size ratios (testing a 100 units versus a 0.1 unit object) or only enable the util
///for certain pairs that have a small size ratio

///The convexConvexAlgorithm collision algorithm implements time of impact, convex closest points and penetration depth calculations between two convex objects.
///Multiple contact points are calculated by perturbing the orientation of the smallest object orthogonal to the separating normal.
///This idea was described by Gino van den Bergen in this forum topic http://www.bulletphysics.com/drx3D/phpBB3/viewtopic.php?f=4&t=288&p=888#p888
class ConvexConvexMprAlgorithm : public ActivatingCollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;
	///cache separating vector to speedup collision detection

public:
	ConvexConvexMprAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

	virtual ~ConvexConvexMprAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		///should we use m_ownManifold to avoid adding duplicates?
		if (m_manifoldPtr && m_ownManifold)
			manifoldArray.push_back(m_manifoldPtr);
	}

	const PersistentManifold* getManifold()
	{
		return m_manifoldPtr;
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		CreateFunc();

		virtual ~CreateFunc();

		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(ConvexConvexMprAlgorithm));
			return new (mem) ConvexConvexMprAlgorithm(ci.m_manifold, ci, body0Wrap, body1Wrap);
		}
	};
};

#endif  //DRX3D_CONVEX_CONVEX_MPR_ALGORITHM_H
