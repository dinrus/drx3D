#ifndef DRX3D_COLLISION__DISPATCHER_H
#define DRX3D_COLLISION__DISPATCHER_H

#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>

#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>

#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

class IDebugDraw;
class OverlappingPairCache;
class PoolAllocator;
class CollisionConfiguration;

#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>

#define USE_DISPATCH_REGISTRY_ARRAY 1

class CollisionDispatcher;
///user can override this nearcallback for collision filtering and more finegrained control over collision detection
typedef void (*NearCallback)(BroadphasePair& collisionPair, CollisionDispatcher& dispatcher, const DispatcherInfo& dispatchInfo);

//Dispatcher supports algorithms that handle ConvexConvex and ConvexConcave collision pairs.
///Time of Impact, Closest Points and Penetration Depth.
class CollisionDispatcher : public Dispatcher
{
protected:
	i32 m_dispatcherFlags;

	AlignedObjectArray<PersistentManifold*> m_manifoldsPtr;

	NearCallback m_nearCallback;

	PoolAllocator* m_collisionAlgorithmPoolAllocator;

	PoolAllocator* m_persistentManifoldPoolAllocator;

	CollisionAlgorithmCreateFunc* m_doubleDispatchContactPoints[MAX_BROADPHASE_COLLISION_TYPES][MAX_BROADPHASE_COLLISION_TYPES];

	CollisionAlgorithmCreateFunc* m_doubleDispatchClosestPoints[MAX_BROADPHASE_COLLISION_TYPES][MAX_BROADPHASE_COLLISION_TYPES];

	CollisionConfiguration* m_collisionConfiguration;

public:
	enum DispatcherFlags
	{
		CD_STATIC_STATIC_REPORTED = 1,
		CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD = 2,
		CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION = 4
	};

	i32 getDispatcherFlags() const
	{
		return m_dispatcherFlags;
	}

	void setDispatcherFlags(i32 flags)
	{
		m_dispatcherFlags = flags;
	}

	///registerCollisionCreateFunc allows registration of custom/alternative collision create functions
	void registerCollisionCreateFunc(i32 proxyType0, i32 proxyType1, CollisionAlgorithmCreateFunc* createFunc);

	void registerClosestPointsCreateFunc(i32 proxyType0, i32 proxyType1, CollisionAlgorithmCreateFunc* createFunc);

	i32 getNumManifolds() const
	{
		return i32(m_manifoldsPtr.size());
	}

	PersistentManifold** getInternalManifoldPointer()
	{
		return m_manifoldsPtr.size() ? &m_manifoldsPtr[0] : 0;
	}

	PersistentManifold* getManifoldByIndexInternal(i32 index)
	{
		Assert(index>=0);
		Assert(index<m_manifoldsPtr.size());
		return m_manifoldsPtr[index];
	}

	const PersistentManifold* getManifoldByIndexInternal(i32 index) const
	{
		Assert(index>=0);
		Assert(index<m_manifoldsPtr.size());
		return m_manifoldsPtr[index];
	}

	CollisionDispatcher(CollisionConfiguration* collisionConfiguration);

	virtual ~CollisionDispatcher();

	virtual PersistentManifold* getNewManifold(const CollisionObject2* b0, const CollisionObject2* b1);

	virtual void releaseManifold(PersistentManifold* manifold);

	virtual void clearManifold(PersistentManifold* manifold);

	CollisionAlgorithm* findAlgorithm(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, PersistentManifold* sharedManifold, eDispatcherQueryType queryType);

	virtual bool needsCollision(const CollisionObject2* body0, const CollisionObject2* body1);

	virtual bool needsResponse(const CollisionObject2* body0, const CollisionObject2* body1);

	virtual void dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& dispatchInfo, Dispatcher* dispatcher);

	void setNearCallback(NearCallback nearCallback)
	{
		m_nearCallback = nearCallback;
	}

	NearCallback getNearCallback() const
	{
		return m_nearCallback;
	}

	//by default, drx3D will use this near callback
	static void defaultNearCallback(BroadphasePair& collisionPair, CollisionDispatcher& dispatcher, const DispatcherInfo& dispatchInfo);

	virtual uk allocateCollisionAlgorithm(i32 size);

	virtual void freeCollisionAlgorithm(uk ptr);

	CollisionConfiguration* getCollisionConfiguration()
	{
		return m_collisionConfiguration;
	}

	const CollisionConfiguration* getCollisionConfiguration() const
	{
		return m_collisionConfiguration;
	}

	void setCollisionConfiguration(CollisionConfiguration* config)
	{
		m_collisionConfiguration = config;
	}

	virtual PoolAllocator* getInternalManifoldPool()
	{
		return m_persistentManifoldPoolAllocator;
	}

	virtual const PoolAllocator* getInternalManifoldPool() const
	{
		return m_persistentManifoldPoolAllocator;
	}
};

#endif  //DRX3D_COLLISION__DISPATCHER_H
