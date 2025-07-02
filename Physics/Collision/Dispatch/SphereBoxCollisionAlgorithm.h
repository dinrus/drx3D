#ifndef DRX3D_SPHERE_BOX_COLLISION_ALGORITHM_H
#define DRX3D_SPHERE_BOX_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
class PersistentManifold;
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

#include <drx3D/Maths/Linear/Vec3.h>

/// SphereBoxCollisionAlgorithm  provides sphere-box collision detection.
/// Other features are frame-coherency (persistent data) and collision response.
class SphereBoxCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;
	bool m_isSwapped;

public:
	SphereBoxCollisionAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	virtual ~SphereBoxCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr && m_ownManifold)
		{
			manifoldArray.push_back(m_manifoldPtr);
		}
	}

	bool getSphereDistance(const CollisionObject2Wrapper* boxObjWrap, Vec3& v3PointOnBox, Vec3& normal, Scalar& penetrationDepth, const Vec3& v3SphereCenter, Scalar fRadius, Scalar maxContactDistance);

	Scalar getSpherePenetration(Vec3 const& boxHalfExtent, Vec3 const& sphereRelPos, Vec3& closestPoint, Vec3& normal);

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SphereBoxCollisionAlgorithm));
			if (!m_swapped)
			{
				return new (mem) SphereBoxCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, false);
			}
			else
			{
				return new (mem) SphereBoxCollisionAlgorithm(0, ci, body0Wrap, body1Wrap, true);
			}
		}
	};
};

#endif  //DRX3D_SPHERE_BOX_COLLISION_ALGORITHM_H
