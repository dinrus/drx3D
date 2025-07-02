#ifndef DRX3D_SPHERE_TRIANGLE_COLLISION_ALGORITHM_H
#define DRX3D_SPHERE_TRIANGLE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
class PersistentManifold;
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

/// SphereSphereCollisionAlgorithm  provides sphere-sphere collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
/// Also provides the most basic sample for custom/user CollisionAlgorithm
class SphereTriangleCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;
	bool m_swapped;

public:
	SphereTriangleCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool swapped);

	SphereTriangleCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci)
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

	virtual ~SphereTriangleCollisionAlgorithm();

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SphereTriangleCollisionAlgorithm));

			return new (mem) SphereTriangleCollisionAlgorithm(ci.m_manifold, ci, body0Wrap, body1Wrap, m_swapped);
		}
	};
};

#endif  //DRX3D_SPHERE_TRIANGLE_COLLISION_ALGORITHM_H
