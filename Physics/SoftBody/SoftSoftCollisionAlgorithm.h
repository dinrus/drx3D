#ifndef DRX3D_SOFT_SOFT_COLLISION_ALGORITHM_H
#define DRX3D_SOFT_SOFT_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>

class PersistentManifold;
class SoftBody;

///collision detection between two SoftBody shapes
class SoftSoftCollisionAlgorithm : public CollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;

	//	SoftBody*	m_softBody0;
	//	SoftBody*	m_softBody1;

public:
	SoftSoftCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
		: CollisionAlgorithm(ci) {}

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr && m_ownManifold)
			manifoldArray.push_back(m_manifoldPtr);
	}

	SoftSoftCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

	virtual ~SoftSoftCollisionAlgorithm();

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			i32 bbsize = sizeof(SoftSoftCollisionAlgorithm);
			uk ptr = ci.m_dispatcher1->allocateCollisionAlgorithm(bbsize);
			return new (ptr) SoftSoftCollisionAlgorithm(0, ci, body0Wrap, body1Wrap);
		}
	};
};

#endif  //DRX3D_SOFT_SOFT_COLLISION_ALGORITHM_H
