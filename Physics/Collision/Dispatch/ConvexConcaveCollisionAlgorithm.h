#ifndef DRX3D_CONVEX_CONCAVE_COLLISION_ALGORITHM_H
#define DRX3D_CONVEX_CONCAVE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class Dispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>

///For each triangle in the concave mesh that overlaps with the AABB of a convex (m_convexProxy), processTriangle is called.
ATTRIBUTE_ALIGNED16(class)
ConvexTriangleCallback : public TriangleCallback
{
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	const CollisionObject2Wrapper* m_convexBodyWrap;
	const CollisionObject2Wrapper* m_triBodyWrap;

	ManifoldResult* m_resultOut;
	Dispatcher* m_dispatcher;
	const DispatcherInfo* m_dispatchInfoPtr;
	Scalar m_collisionMarginTriangle;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_triangleCount;

	PersistentManifold* m_manifoldPtr;

	ConvexTriangleCallback(Dispatcher * dispatcher, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	void setTimeStepAndCounters(Scalar collisionMarginTriangle, const DispatcherInfo& dispatchInfo, const CollisionObject2Wrapper* convexBodyWrap, const CollisionObject2Wrapper* triBodyWrap, ManifoldResult* resultOut);

	void clearWrapperData()
	{
		m_convexBodyWrap = 0;
		m_triBodyWrap = 0;
	}
	virtual ~ConvexTriangleCallback();

	virtual void processTriangle(Vec3 * triangle, i32 partId, i32 triangleIndex);

	void clearCache();

	SIMD_FORCE_INLINE const Vec3& getAabbMin() const
	{
		return m_aabbMin;
	}
	SIMD_FORCE_INLINE const Vec3& getAabbMax() const
	{
		return m_aabbMax;
	}
};

/// ConvexConcaveCollisionAlgorithm  supports collision between convex shapes and (concave) trianges meshes.
ATTRIBUTE_ALIGNED16(class)
ConvexConcaveCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	ConvexTriangleCallback m_ConvexTriangleCallback;

	bool m_isSwapped;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConvexConcaveCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	virtual ~ConvexConcaveCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	Scalar calculateTimeOfImpact(CollisionObject2 * body0, CollisionObject2 * body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray & manifoldArray);

	void clearCache();

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(ConvexConcaveCollisionAlgorithm));
			return new (mem) ConvexConcaveCollisionAlgorithm(ci, body0Wrap, body1Wrap, false);
		}
	};

	struct SwappedCreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(ConvexConcaveCollisionAlgorithm));
			return new (mem) ConvexConcaveCollisionAlgorithm(ci, body0Wrap, body1Wrap, true);
		}
	};
};

#endif  //DRX3D_CONVEX_CONCAVE_COLLISION_ALGORITHM_H
