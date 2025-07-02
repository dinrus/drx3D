#ifndef DRX3D_SOFT_RIGID_COLLISION_ALGORITHM_H
#define DRX3D_SOFT_RIGID_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
class PersistentManifold;
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

#include <drx3D/Maths/Linear/Vec3.h>
class SoftBody;

/// SoftRigidCollisionAlgorithm  provides collision detection between SoftBody and RigidBody
class SoftRigidCollisionAlgorithm : public CollisionAlgorithm
{
	//	bool	m_ownManifold;
	//	PersistentManifold*	m_manifoldPtr;

	//SoftBody*				m_softBody;
	//CollisionObject2*		m_rigidCollisionObject2;

	///for rigid versus soft (instead of soft versus rigid), we use this swapped boolean
	bool m_isSwapped;

public:
	SoftRigidCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* col0, const CollisionObject2Wrapper* col1Wrap, bool isSwapped);

	virtual ~SoftRigidCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		//we don't add any manifolds
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SoftRigidCollisionAlgorithm));
			if (!m_swapped)
			{
				return new (mem) SoftRigidCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, false);
			}
			else
			{
				return new (mem) SoftRigidCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, true);
			}
		}
	};
};

#endif  //DRX3D_SOFT_RIGID_COLLISION_ALGORITHM_H
