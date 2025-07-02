#ifndef DRX3D_GHOST_OBJECT_H
#define DRX3D_GHOST_OBJECT_H

#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCallback.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>

class ConvexShape;

class Dispatcher;

///The GhostObject can keep track of all objects that are overlapping
///By default, this overlap is based on the AABB
///This is useful for creating a character controller, collision sensors/triggers, explosions etc.
///We plan on adding rayTest and other queries for the GhostObject
ATTRIBUTE_ALIGNED16(class)
GhostObject : public CollisionObject2
{
protected:
	AlignedObjectArray<CollisionObject2*> m_overlappingObjects;

public:
	GhostObject();

	virtual ~GhostObject();

	void convexSweepTest(const class ConvexShape* castShape, const Transform2& convexFromWorld, const Transform2& convexToWorld, CollisionWorld::ConvexResultCallback& resultCallback, Scalar allowedCcdPenetration = 0.f) const;

	void rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, CollisionWorld::RayResultCallback& resultCallback) const;

	///this method is mainly for expert/internal use only.
	virtual void addOverlappingObjectInternal(BroadphaseProxy * otherProxy, BroadphaseProxy* thisProxy = 0);
	///this method is mainly for expert/internal use only.
	virtual void removeOverlappingObjectInternal(BroadphaseProxy * otherProxy, Dispatcher * dispatcher, BroadphaseProxy* thisProxy = 0);

	i32 getNumOverlappingObjects() const
	{
		return m_overlappingObjects.size();
	}

	CollisionObject2* getOverlappingObject(i32 index)
	{
		return m_overlappingObjects[index];
	}

	const CollisionObject2* getOverlappingObject(i32 index) const
	{
		return m_overlappingObjects[index];
	}

	AlignedObjectArray<CollisionObject2*>& getOverlappingPairs()
	{
		return m_overlappingObjects;
	}

	const AlignedObjectArray<CollisionObject2*> getOverlappingPairs() const
	{
		return m_overlappingObjects;
	}

	//
	// internal cast
	//

	static const GhostObject* upcast(const CollisionObject2* colObj)
	{
		if (colObj->getInternalType() == CO_GHOST_OBJECT)
			return (const GhostObject*)colObj;
		return 0;
	}
	static GhostObject* upcast(CollisionObject2 * colObj)
	{
		if (colObj->getInternalType() == CO_GHOST_OBJECT)
			return (GhostObject*)colObj;
		return 0;
	}
};

class PairCachingGhostObject : public GhostObject
{
	HashedOverlappingPairCache* m_hashPairCache;

public:
	PairCachingGhostObject();

	virtual ~PairCachingGhostObject();

	///this method is mainly for expert/internal use only.
	virtual void addOverlappingObjectInternal(BroadphaseProxy* otherProxy, BroadphaseProxy* thisProxy = 0);

	virtual void removeOverlappingObjectInternal(BroadphaseProxy* otherProxy, Dispatcher* dispatcher, BroadphaseProxy* thisProxy = 0);

	HashedOverlappingPairCache* getOverlappingPairCache()
	{
		return m_hashPairCache;
	}
};

///The GhostPairCallback interfaces and forwards adding and removal of overlapping pairs from the BroadphaseInterface to GhostObject.
class GhostPairCallback : public OverlappingPairCallback
{
public:
	GhostPairCallback()
	{
	}

	virtual ~GhostPairCallback()
	{
	}

	virtual BroadphasePair* addOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
	{
		CollisionObject2* colObj0 = (CollisionObject2*)proxy0->m_clientObject;
		CollisionObject2* colObj1 = (CollisionObject2*)proxy1->m_clientObject;
		GhostObject* ghost0 = GhostObject::upcast(colObj0);
		GhostObject* ghost1 = GhostObject::upcast(colObj1);
		if (ghost0)
			ghost0->addOverlappingObjectInternal(proxy1, proxy0);
		if (ghost1)
			ghost1->addOverlappingObjectInternal(proxy0, proxy1);
		return 0;
	}

	virtual uk removeOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1, Dispatcher* dispatcher)
	{
		CollisionObject2* colObj0 = (CollisionObject2*)proxy0->m_clientObject;
		CollisionObject2* colObj1 = (CollisionObject2*)proxy1->m_clientObject;
		GhostObject* ghost0 = GhostObject::upcast(colObj0);
		GhostObject* ghost1 = GhostObject::upcast(colObj1);
		if (ghost0)
			ghost0->removeOverlappingObjectInternal(proxy1, dispatcher, proxy0);
		if (ghost1)
			ghost1->removeOverlappingObjectInternal(proxy0, dispatcher, proxy1);
		return 0;
	}

	virtual void removeOverlappingPairsContainingProxy(BroadphaseProxy* /*proxy0*/, Dispatcher* /*dispatcher*/)
	{
		Assert(0);
		//need to keep track of all ghost objects and call them here
		//m_hashPairCache->removeOverlappingPairsContainingProxy(proxy0,dispatcher);
	}
};

#endif
