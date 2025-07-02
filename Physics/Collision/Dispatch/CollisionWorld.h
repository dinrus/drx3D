#ifndef DRX3D_COLLISION_WORLD_H
#define DRX3D_COLLISION_WORLD_H

class CollisionShape;
class ConvexShape;
class BroadphaseInterface;
class Serializer;

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///CollisionWorld - это интерфейс и контейнер для обнаружения столкновения
class CollisionWorld
{
protected:
	AlignedObjectArray<CollisionObject2*> m_collisionObjects;

	Dispatcher* m_dispatcher1;

	DispatcherInfo m_dispatchInfo;

	BroadphaseInterface* m_broadphasePairCache;

	IDebugDraw* m_debugDrawer;

	///m_forceUpdateAllAabbs can be set to false as an optimization to only update active object AABBs
	///it is true by default, because it is error-prone (setting the position of static objects wouldn't update their AABB)
	bool m_forceUpdateAllAabbs;

	void serializeCollisionObjects(Serializer* serializer);

	void serializeContactManifolds(Serializer* serializer);

public:
	//this constructor doesn't own the dispatcher and paircache/broadphase
	CollisionWorld(Dispatcher* dispatcher, BroadphaseInterface* broadphasePairCache, CollisionConfiguration* collisionConfiguration);

	virtual ~CollisionWorld();

	void setBroadphase(BroadphaseInterface* pairCache)
	{
		m_broadphasePairCache = pairCache;
	}

	const BroadphaseInterface* getBroadphase() const
	{
		return m_broadphasePairCache;
	}

	BroadphaseInterface* getBroadphase()
	{
		return m_broadphasePairCache;
	}

	OverlappingPairCache* getPairCache()
	{
		return m_broadphasePairCache->getOverlappingPairCache();
	}

	Dispatcher* getDispatcher()
	{
		return m_dispatcher1;
	}

	const Dispatcher* getDispatcher() const
	{
		return m_dispatcher1;
	}

	void updateSingleAabb(CollisionObject2* colObj);

	virtual void updateAabbs();

	///the computeOverlappingPairs is usually already called by performDiscreteCollisionDetection (or stepSimulation)
	///it can be useful to use if you perform ray tests without collision detection/simulation
	virtual void computeOverlappingPairs();

	virtual void setDebugDrawer(IDebugDraw* debugDrawer)
	{
		m_debugDrawer = debugDrawer;
	}

	virtual IDebugDraw* getDebugDrawer()
	{
		return m_debugDrawer;
	}

	virtual void debugDrawWorld();

	virtual void debugDrawObject(const Transform2& worldTransform2, const CollisionShape* shape, const Vec3& color);

	///LocalShapeInfo gives extra information for complex shapes
	///Currently, only TriangleMeshShape is available, so it just contains triangleIndex and subpart
	struct LocalShapeInfo
	{
		i32 m_shapePart;
		i32 m_triangleIndex;

		//const CollisionShape*	m_shapeTemp;
		//const Transform2*	m_shapeLocalTransform2;
	};

	struct LocalRayResult
	{
		LocalRayResult(const CollisionObject2* collisionObject,
					   LocalShapeInfo* localShapeInfo,
					   const Vec3& hitNormalLocal,
					   Scalar hitFraction)
			: m_collisionObject(collisionObject),
			  m_localShapeInfo(localShapeInfo),
			  m_hitNormalLocal(hitNormalLocal),
			  m_hitFraction(hitFraction)
		{
		}

		const CollisionObject2* m_collisionObject;
		LocalShapeInfo* m_localShapeInfo;
		Vec3 m_hitNormalLocal;
		Scalar m_hitFraction;
	};

	///RayResultCallback is used to report new raycast results
	struct RayResultCallback
	{
		Scalar m_closestHitFraction;
		const CollisionObject2* m_collisionObject;
		i32 m_collisionFilterGroup;
		i32 m_collisionFilterMask;
		//@BP Mod - Custom flags, currently used to enable backface culling on tri-meshes, see RaycastCallback.h. Apply any of the EFlags defined there on m_flags here to invoke.
		u32 m_flags;

		virtual ~RayResultCallback()
		{
		}
		bool hasHit() const
		{
			return (m_collisionObject != 0);
		}

		RayResultCallback()
			: m_closestHitFraction(Scalar(1.)),
			  m_collisionObject(0),
			  m_collisionFilterGroup(BroadphaseProxy::DefaultFilter),
			  m_collisionFilterMask(BroadphaseProxy::AllFilter),
			  //@BP Mod
			  m_flags(0)
		{
		}

		virtual bool needsCollision(BroadphaseProxy* proxy0) const
		{
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			return collides;
		}

		virtual Scalar addSingleResult(LocalRayResult& rayResult, bool normalInWorldSpace) = 0;
	};

	struct ClosestRayResultCallback : public RayResultCallback
	{
		ClosestRayResultCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld)
			: m_rayFromWorld(rayFromWorld),
			  m_rayToWorld(rayToWorld)
		{
		}

		Vec3 m_rayFromWorld;  //used to calculate hitPointWorld from hitFraction
		Vec3 m_rayToWorld;

		Vec3 m_hitNormalWorld;
		Vec3 m_hitPointWorld;

		virtual Scalar addSingleResult(LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			//caller already does the filter on the m_closestHitFraction
			Assert(rayResult.m_hitFraction <= m_closestHitFraction);

			m_closestHitFraction = rayResult.m_hitFraction;
			m_collisionObject = rayResult.m_collisionObject;
			if (normalInWorldSpace)
			{
				m_hitNormalWorld = rayResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;
			}
			m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
			return rayResult.m_hitFraction;
		}
	};

	struct AllHitsRayResultCallback : public RayResultCallback
	{
		AllHitsRayResultCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld)
			: m_rayFromWorld(rayFromWorld),
			  m_rayToWorld(rayToWorld)
		{
		}

		AlignedObjectArray<const CollisionObject2*> m_collisionObjects;

		Vec3 m_rayFromWorld;  //used to calculate hitPointWorld from hitFraction
		Vec3 m_rayToWorld;

		AlignedObjectArray<Vec3> m_hitNormalWorld;
		AlignedObjectArray<Vec3> m_hitPointWorld;
		AlignedObjectArray<Scalar> m_hitFractions;

		virtual Scalar addSingleResult(LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			m_collisionObject = rayResult.m_collisionObject;
			m_collisionObjects.push_back(rayResult.m_collisionObject);
			Vec3 hitNormalWorld;
			if (normalInWorldSpace)
			{
				hitNormalWorld = rayResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;
			}
			m_hitNormalWorld.push_back(hitNormalWorld);
			Vec3 hitPointWorld;
			hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
			m_hitPointWorld.push_back(hitPointWorld);
			m_hitFractions.push_back(rayResult.m_hitFraction);
			return m_closestHitFraction;
		}
	};

	struct LocalConvexResult
	{
		LocalConvexResult(const CollisionObject2* hitCollisionObject2,
						  LocalShapeInfo* localShapeInfo,
						  const Vec3& hitNormalLocal,
						  const Vec3& hitPointLocal,
						  Scalar hitFraction)
			: m_hitCollisionObject2(hitCollisionObject2),
			  m_localShapeInfo(localShapeInfo),
			  m_hitNormalLocal(hitNormalLocal),
			  m_hitPointLocal(hitPointLocal),
			  m_hitFraction(hitFraction)
		{
		}

		const CollisionObject2* m_hitCollisionObject2;
		LocalShapeInfo* m_localShapeInfo;
		Vec3 m_hitNormalLocal;
		Vec3 m_hitPointLocal;
		Scalar m_hitFraction;
	};

	///RayResultCallback is used to report new raycast results
	struct ConvexResultCallback
	{
		Scalar m_closestHitFraction;
		i32 m_collisionFilterGroup;
		i32 m_collisionFilterMask;

		ConvexResultCallback()
			: m_closestHitFraction(Scalar(1.)),
			  m_collisionFilterGroup(BroadphaseProxy::DefaultFilter),
			  m_collisionFilterMask(BroadphaseProxy::AllFilter)
		{
		}

		virtual ~ConvexResultCallback()
		{
		}

		bool hasHit() const
		{
			return (m_closestHitFraction < Scalar(1.));
		}

		virtual bool needsCollision(BroadphaseProxy* proxy0) const
		{
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			return collides;
		}

		virtual Scalar addSingleResult(LocalConvexResult& convexResult, bool normalInWorldSpace) = 0;
	};

	struct ClosestConvexResultCallback : public ConvexResultCallback
	{
		ClosestConvexResultCallback(const Vec3& convexFromWorld, const Vec3& convexToWorld)
			: m_convexFromWorld(convexFromWorld),
			  m_convexToWorld(convexToWorld),
			  m_hitCollisionObject2(0)
		{
		}

		Vec3 m_convexFromWorld;  //used to calculate hitPointWorld from hitFraction
		Vec3 m_convexToWorld;

		Vec3 m_hitNormalWorld;
		Vec3 m_hitPointWorld;
		const CollisionObject2* m_hitCollisionObject2;

		virtual Scalar addSingleResult(LocalConvexResult& convexResult, bool normalInWorldSpace)
		{
			//caller already does the filter on the m_closestHitFraction
			Assert(convexResult.m_hitFraction <= m_closestHitFraction);

			m_closestHitFraction = convexResult.m_hitFraction;
			m_hitCollisionObject2 = convexResult.m_hitCollisionObject2;
			if (normalInWorldSpace)
			{
				m_hitNormalWorld = convexResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld = m_hitCollisionObject2->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;
			}
			m_hitPointWorld = convexResult.m_hitPointLocal;
			return convexResult.m_hitFraction;
		}
	};

	///ContactResultCallback is used to report contact points
	struct ContactResultCallback
	{
		i32 m_collisionFilterGroup;
		i32 m_collisionFilterMask;
		Scalar m_closestDistanceThreshold;

		ContactResultCallback()
			: m_collisionFilterGroup(BroadphaseProxy::DefaultFilter),
			  m_collisionFilterMask(BroadphaseProxy::AllFilter),
			  m_closestDistanceThreshold(0)
		{
		}

		virtual ~ContactResultCallback()
		{
		}

		virtual bool needsCollision(BroadphaseProxy* proxy0) const
		{
			bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
			collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);
			return collides;
		}

		virtual Scalar addSingleResult(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, i32 partId0, i32 index0, const CollisionObject2Wrapper* colObj1Wrap, i32 partId1, i32 index1) = 0;
	};

	i32 getNumCollisionObjects() const
	{
		return i32(m_collisionObjects.size());
	}

	/// rayTest performs a raycast on all objects in the CollisionWorld, and calls the resultCallback
	/// This allows for several queries: first hit, all hits, any hit, dependent on the value returned by the callback.
	virtual void rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, RayResultCallback& resultCallback) const;

	/// convexTest performs a swept convex cast on all objects in the CollisionWorld, and calls the resultCallback
	/// This allows for several queries: first hit, all hits, any hit, dependent on the value return by the callback.
	void convexSweepTest(const ConvexShape* castShape, const Transform2& from, const Transform2& to, ConvexResultCallback& resultCallback, Scalar allowedCcdPenetration = Scalar(0.)) const;

	///contactTest performs a discrete collision test between colObj against all objects in the CollisionWorld, and calls the resultCallback.
	///it reports one or more contact points for every overlapping object (including the one with deepest penetration)
	void contactTest(CollisionObject2* colObj, ContactResultCallback& resultCallback);

	///contactTest performs a discrete collision test between two collision objects and calls the resultCallback if overlap if detected.
	///it reports one or more contact points (including the one with deepest penetration)
	void contactPairTest(CollisionObject2* colObjA, CollisionObject2* colObjB, ContactResultCallback& resultCallback);

	/// rayTestSingle performs a raycast call and calls the resultCallback. It is used internally by rayTest.
	/// In a future implementation, we consider moving the ray test as a virtual method in CollisionShape.
	/// This allows more customization.
	static void rayTestSingle(const Transform2& rayFromTrans, const Transform2& rayToTrans,
							  CollisionObject2* collisionObject,
							  const CollisionShape* collisionShape,
							  const Transform2& colObjWorldTransform,
							  RayResultCallback& resultCallback);

	static void rayTestSingleInternal(const Transform2& rayFromTrans, const Transform2& rayToTrans,
									  const CollisionObject2Wrapper* collisionObjectWrap,
									  RayResultCallback& resultCallback);

	/// objectQuerySingle performs a collision detection query and calls the resultCallback. It is used internally by rayTest.
	static void objectQuerySingle(const ConvexShape* castShape, const Transform2& rayFromTrans, const Transform2& rayToTrans,
								  CollisionObject2* collisionObject,
								  const CollisionShape* collisionShape,
								  const Transform2& colObjWorldTransform,
								  ConvexResultCallback& resultCallback, Scalar allowedPenetration);

	static void objectQuerySingleInternal(const ConvexShape* castShape, const Transform2& convexFromTrans, const Transform2& convexToTrans,
										  const CollisionObject2Wrapper* colObjWrap,
										  ConvexResultCallback& resultCallback, Scalar allowedPenetration);

	virtual void addCollisionObject(CollisionObject2* collisionObject, i32 collisionFilterGroup = BroadphaseProxy::DefaultFilter, i32 collisionFilterMask = BroadphaseProxy::AllFilter);

	virtual void refreshBroadphaseProxy(CollisionObject2* collisionObject);

	CollisionObject2Array& getCollisionObjectArray()
	{
		return m_collisionObjects;
	}

	const CollisionObject2Array& getCollisionObjectArray() const
	{
		return m_collisionObjects;
	}

	virtual void removeCollisionObject(CollisionObject2* collisionObject);

	virtual void performDiscreteCollisionDetection();

	DispatcherInfo& getDispatchInfo()
	{
		return m_dispatchInfo;
	}

	const DispatcherInfo& getDispatchInfo() const
	{
		return m_dispatchInfo;
	}

	bool getForceUpdateAllAabbs() const
	{
		return m_forceUpdateAllAabbs;
	}
	void setForceUpdateAllAabbs(bool forceUpdateAllAabbs)
	{
		m_forceUpdateAllAabbs = forceUpdateAllAabbs;
	}

	///Preliminary serialization test for drx3D 2.76. Loading those files requires a separate parser (drx3D/Demos/SerializeDemo)
	virtual void serialize(Serializer* serializer);
};

#endif  //DRX3D_COLLISION_WORLD_H
