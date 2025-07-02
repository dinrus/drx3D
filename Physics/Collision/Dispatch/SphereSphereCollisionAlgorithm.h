#ifndef DRX3D_SPHERE_SPHERE_COLLISION_ALGORITHM_H
#define DRX3D_SPHERE_SPHERE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

class PersistentManifold;

/// SphereSphereCollisionAlgorithm  provides sphere-sphere collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
/// Also provides the most basic sample for custom/user CollisionAlgorithm
class SphereSphereCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;

public:
	SphereSphereCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap);

	SphereSphereCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
		: ActivatingCollisionAlgorithm(ci) {}

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr && m_ownManifold)
		{
			manifoldArray.push_back(m_manifoldPtr);
		}
	}

	virtual ~SphereSphereCollisionAlgorithm();

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0Wrap, const CollisionObject2Wrapper* col1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SphereSphereCollisionAlgorithm));
			return new (mem) SphereSphereCollisionAlgorithm(0, ci, col0Wrap, col1Wrap);
		}
	};
};

#endif  //DRX3D_SPHERE_SPHERE_COLLISION_ALGORITHM_H
