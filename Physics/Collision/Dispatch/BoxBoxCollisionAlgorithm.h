#ifndef DRX3D_BOX_BOX__COLLISION_ALGORITHM_H
#define DRX3D_BOX_BOX__COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>

class PersistentManifold;

///box-box collision detection
class BoxBoxCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;

public:
	BoxBoxCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
		: ActivatingCollisionAlgorithm(ci) {}

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	BoxBoxCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

	virtual ~BoxBoxCollisionAlgorithm();

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr && m_ownManifold)
		{
			manifoldArray.push_back(m_manifoldPtr);
		}
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			i32 bbsize = sizeof(BoxBoxCollisionAlgorithm);
			uk ptr = ci.m_dispatcher1->allocateCollisionAlgorithm(bbsize);
			return new (ptr) BoxBoxCollisionAlgorithm(0, ci, body0Wrap, body1Wrap);
		}
	};
};

#endif  //DRX3D_BOX_BOX__COLLISION_ALGORITHM_H
