#ifndef DRX3D_CONVEX_PLANE_COLLISION_ALGORITHM_H
#define DRX3D_CONVEX_PLANE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
class PersistentManifold;
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

#include <drx3D/Maths/Linear/Vec3.h>

/// SphereBoxCollisionAlgorithm  provides sphere-box collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
class ConvexPlaneCollisionAlgorithm : public CollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;
	bool m_isSwapped;
	i32 m_numPerturbationIterations;
	i32 m_minimumPointsPerturbationThreshold;

public:
	ConvexPlaneCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped, i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold);

	virtual ~ConvexPlaneCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	void collideSingleContact(const Quat& perturbeRot, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr && m_ownManifold)
		{
			manifoldArray.push_back(m_manifoldPtr);
		}
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		i32 m_numPerturbationIterations;
		i32 m_minimumPointsPerturbationThreshold;

		CreateFunc()
			: m_numPerturbationIterations(1),
			  m_minimumPointsPerturbationThreshold(0)
		{
		}

		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(ConvexPlaneCollisionAlgorithm));
			if (!m_swapped)
			{
				return new (mem) ConvexPlaneCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, false, m_numPerturbationIterations, m_minimumPointsPerturbationThreshold);
			}
			else
			{
				return new (mem) ConvexPlaneCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, true, m_numPerturbationIterations, m_minimumPointsPerturbationThreshold);
			}
		}
	};
};

#endif  //DRX3D_CONVEX_PLANE_COLLISION_ALGORITHM_H
