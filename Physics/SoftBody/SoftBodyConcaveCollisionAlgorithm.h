#ifndef DRX3D_SOFT_BODY_CONCAVE_COLLISION_ALGORITHM_H
#define DRX3D_SOFT_BODY_CONCAVE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class Dispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
class SoftBody;
class CollisionShape;

#include <drx3D/Maths/Linear/HashMap.h>

#include <drx3D/Physics/Collision/BroadPhase/QuantizedBvh.h>  //for definition of MAX_NUM_PARTS_IN_BITS

struct TriIndex
{
	i32 m_PartIdTriangleIndex;
	class CollisionShape* m_childShape;

	TriIndex(i32 partId, i32 triangleIndex, CollisionShape* shape)
	{
		m_PartIdTriangleIndex = (partId << (31 - MAX_NUM_PARTS_IN_BITS)) | triangleIndex;
		m_childShape = shape;
	}

	i32 getTriangleIndex() const
	{
		// Get only the lower bits where the triangle index is stored
		u32 x = 0;
		u32 y = (~(x & 0)) << (31 - MAX_NUM_PARTS_IN_BITS);
		return (m_PartIdTriangleIndex & ~(y));
	}
	i32 getPartId() const
	{
		// Get only the highest bits where the part index is stored
		return (m_PartIdTriangleIndex >> (31 - MAX_NUM_PARTS_IN_BITS));
	}
	i32 getUid() const
	{
		return m_PartIdTriangleIndex;
	}
};

///For each triangle in the concave mesh that overlaps with the AABB of a soft body (m_softBody), processTriangle is called.
class SoftBodyTriangleCallback : public TriangleCallback
{
	SoftBody* m_softBody;
	const CollisionObject2* m_triBody;

	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	ManifoldResult* m_resultOut;

	Dispatcher* m_dispatcher;
	const DispatcherInfo* m_dispatchInfoPtr;
	Scalar m_collisionMarginTriangle;

	HashMap<HashKey<TriIndex>, TriIndex> m_shapeCache;

public:
	i32 m_triangleCount;

	//	PersistentManifold*	m_manifoldPtr;

	SoftBodyTriangleCallback(Dispatcher* dispatcher, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	void setTimeStepAndCounters(Scalar collisionMarginTriangle, const CollisionObject2Wrapper* triObjWrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual ~SoftBodyTriangleCallback();

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex);

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

/// SoftBodyConcaveCollisionAlgorithm  supports collision between soft body shapes and (concave) trianges meshes.
class SoftBodyConcaveCollisionAlgorithm : public CollisionAlgorithm
{
	bool m_isSwapped;

	SoftBodyTriangleCallback m_SoftBodyTriangleCallback;

public:
	SoftBodyConcaveCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	virtual ~SoftBodyConcaveCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		//we don't add any manifolds
	}

	void clearCache();

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SoftBodyConcaveCollisionAlgorithm));
			return new (mem) SoftBodyConcaveCollisionAlgorithm(ci, body0Wrap, body1Wrap, false);
		}
	};

	struct SwappedCreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SoftBodyConcaveCollisionAlgorithm));
			return new (mem) SoftBodyConcaveCollisionAlgorithm(ci, body0Wrap, body1Wrap, true);
		}
	};
};

#endif  //DRX3D_SOFT_BODY_CONCAVE_COLLISION_ALGORITHM_H
