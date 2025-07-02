#ifndef DRX3D_OVERLAPPING_PAIR_CACHE_H
#define DRX3D_OVERLAPPING_PAIR_CACHE_H

#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCallback.h>

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
class Dispatcher;

typedef AlignedObjectArray<BroadphasePair> BroadphasePairArray;

struct OverlapCallback
{
	virtual ~OverlapCallback()
	{
	}
	//return true for deletion of the pair
	virtual bool processOverlap(BroadphasePair& pair) = 0;
};

struct OverlapFilterCallback
{
	virtual ~OverlapFilterCallback()
	{
	}
	// return true when pairs need collision
	virtual bool needBroadphaseCollision(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1) const = 0;
};

i32k DRX3D_NULL_PAIR = 0xffffffff;

///The OverlappingPairCache provides an interface for overlapping pair management (add, remove, storage), used by the BroadphaseInterface broadphases.
///The HashedOverlappingPairCache and SortedOverlappingPairCache classes are two implementations.
class OverlappingPairCache : public OverlappingPairCallback
{
public:
	virtual ~OverlappingPairCache() {}  // this is needed so we can get to the derived class destructor

	virtual BroadphasePair* getOverlappingPairArrayPtr() = 0;

	virtual const BroadphasePair* getOverlappingPairArrayPtr() const = 0;

	virtual BroadphasePairArray& getOverlappingPairArray() = 0;

	virtual void cleanOverlappingPair(BroadphasePair& pair, Dispatcher* dispatcher) = 0;

	virtual i32 getNumOverlappingPairs() const = 0;
	virtual bool needsBroadphaseCollision(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1) const = 0;
	virtual OverlapFilterCallback* getOverlapFilterCallback() = 0;
	virtual void cleanProxyFromPairs(BroadphaseProxy* proxy, Dispatcher* dispatcher) = 0;

	virtual void setOverlapFilterCallback(OverlapFilterCallback* callback) = 0;

	virtual void processAllOverlappingPairs(OverlapCallback*, Dispatcher* dispatcher) = 0;

	virtual void processAllOverlappingPairs(OverlapCallback* callback, Dispatcher* dispatcher, const struct DispatcherInfo& /*dispatchInfo*/)
	{
		processAllOverlappingPairs(callback, dispatcher);
	}
	virtual BroadphasePair* findPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1) = 0;

	virtual bool hasDeferredRemoval() = 0;

	virtual void setInternalGhostPairCallback(OverlappingPairCallback* ghostPairCallback) = 0;

	virtual void sortOverlappingPairs(Dispatcher* dispatcher) = 0;
};

/// Hash-space based Pair Cache, thanks to Erin Catto, Box2D, http://www.box2d.org, and Pierre Terdiman, Codercorner, http://codercorner.com

ATTRIBUTE_ALIGNED16(class)
HashedOverlappingPairCache : public OverlappingPairCache
{
	BroadphasePairArray m_overlappingPairArray;
	OverlapFilterCallback* m_overlapFilterCallback;

protected:
	AlignedObjectArray<i32> m_hashTable;
	AlignedObjectArray<i32> m_next;
	OverlappingPairCallback* m_ghostPairCallback;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	HashedOverlappingPairCache();
	virtual ~HashedOverlappingPairCache();

	void removeOverlappingPairsContainingProxy(BroadphaseProxy * proxy, Dispatcher * dispatcher);

	virtual uk removeOverlappingPair(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1, Dispatcher * dispatcher);

	SIMD_FORCE_INLINE bool needsBroadphaseCollision(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1) const
	{
		if (m_overlapFilterCallback)
			return m_overlapFilterCallback->needBroadphaseCollision(proxy0, proxy1);

		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		return collides;
	}

	// Add a pair and return the new pair. If the pair already exists,
	// no new pair is created and the old one is returned.
	virtual BroadphasePair* addOverlappingPair(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1)
	{
		if (!needsBroadphaseCollision(proxy0, proxy1))
			return 0;

		return internalAddPair(proxy0, proxy1);
	}

	void cleanProxyFromPairs(BroadphaseProxy * proxy, Dispatcher * dispatcher);

	virtual void processAllOverlappingPairs(OverlapCallback*, Dispatcher * dispatcher);

	virtual void processAllOverlappingPairs(OverlapCallback * callback, Dispatcher * dispatcher, const struct DispatcherInfo& dispatchInfo);

	virtual BroadphasePair* getOverlappingPairArrayPtr()
	{
		return &m_overlappingPairArray[0];
	}

	const BroadphasePair* getOverlappingPairArrayPtr() const
	{
		return &m_overlappingPairArray[0];
	}

	BroadphasePairArray& getOverlappingPairArray()
	{
		return m_overlappingPairArray;
	}

	const BroadphasePairArray& getOverlappingPairArray() const
	{
		return m_overlappingPairArray;
	}

	void cleanOverlappingPair(BroadphasePair & pair, Dispatcher * dispatcher);

	BroadphasePair* findPair(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1);

	i32 GetCount() const { return m_overlappingPairArray.size(); }
	//	BroadphasePair* GetPairs() { return m_pairs; }

	OverlapFilterCallback* getOverlapFilterCallback()
	{
		return m_overlapFilterCallback;
	}

	void setOverlapFilterCallback(OverlapFilterCallback * callback)
	{
		m_overlapFilterCallback = callback;
	}

	i32 getNumOverlappingPairs() const
	{
		return m_overlappingPairArray.size();
	}

private:
	BroadphasePair* internalAddPair(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1);

	void growTables();

	SIMD_FORCE_INLINE bool equalsPair(const BroadphasePair& pair, i32 proxyId1, i32 proxyId2)
	{
		return pair.m_pProxy0->getUid() == proxyId1 && pair.m_pProxy1->getUid() == proxyId2;
	}

	/*
	// Thomas Wang's hash, see: http://www.concentric.net/~Ttwang/tech/inthash.htm
	// This assumes proxyId1 and proxyId2 are 16-bit.
	SIMD_FORCE_INLINE i32 getHash(i32 proxyId1, i32 proxyId2)
	{
		i32 key = (proxyId2 << 16) | proxyId1;
		key = ~key + (key << 15);
		key = key ^ (key >> 12);
		key = key + (key << 2);
		key = key ^ (key >> 4);
		key = key * 2057;
		key = key ^ (key >> 16);
		return key;
	}
	*/

	SIMD_FORCE_INLINE u32 getHash(u32 proxyId1, u32 proxyId2)
	{
		u32 key = proxyId1 | (proxyId2 << 16);
		// Thomas Wang's hash

		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}

	SIMD_FORCE_INLINE BroadphasePair* internalFindPair(BroadphaseProxy * proxy0, BroadphaseProxy * proxy1, i32 hash)
	{
		i32 proxyId1 = proxy0->getUid();
		i32 proxyId2 = proxy1->getUid();
#if 0  // wrong, 'equalsPair' use unsorted uids, copy-past devil striked again. Nat.
		if (proxyId1 > proxyId2) 
			Swap(proxyId1, proxyId2);
#endif

		i32 index = m_hashTable[hash];

		while (index != DRX3D_NULL_PAIR && equalsPair(m_overlappingPairArray[index], proxyId1, proxyId2) == false)
		{
			index = m_next[index];
		}

		if (index == DRX3D_NULL_PAIR)
		{
			return nullptr;
		}

		Assert(index < m_overlappingPairArray.size());

		return &m_overlappingPairArray[index];
	}

	virtual bool hasDeferredRemoval()
	{
		return false;
	}

	virtual void setInternalGhostPairCallback(OverlappingPairCallback * ghostPairCallback)
	{
		m_ghostPairCallback = ghostPairCallback;
	}

	virtual void sortOverlappingPairs(Dispatcher * dispatcher);
};

//SortedOverlappingPairCache maintains the objects with overlapping AABB
///Typically managed by the Broadphase, Axis3Sweep or SimpleBroadphase
class SortedOverlappingPairCache : public OverlappingPairCache
{
protected:
	//avoid brute-force finding all the time
	BroadphasePairArray m_overlappingPairArray;

	//during the dispatch, check that user doesn't destroy/create proxy
	bool m_blockedForChanges;

	///by default, do the removal during the pair traversal
	bool m_hasDeferredRemoval;

	//if set, use the callback instead of the built in filter in needBroadphaseCollision
	OverlapFilterCallback* m_overlapFilterCallback;

	OverlappingPairCallback* m_ghostPairCallback;

public:
	SortedOverlappingPairCache();
	virtual ~SortedOverlappingPairCache();

	virtual void processAllOverlappingPairs(OverlapCallback*, Dispatcher* dispatcher);

	uk removeOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1, Dispatcher* dispatcher);

	void cleanOverlappingPair(BroadphasePair& pair, Dispatcher* dispatcher);

	BroadphasePair* addOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1);

	BroadphasePair* findPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1);

	void cleanProxyFromPairs(BroadphaseProxy* proxy, Dispatcher* dispatcher);

	void removeOverlappingPairsContainingProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher);

	inline bool needsBroadphaseCollision(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1) const
	{
		if (m_overlapFilterCallback)
			return m_overlapFilterCallback->needBroadphaseCollision(proxy0, proxy1);

		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		return collides;
	}

	BroadphasePairArray& getOverlappingPairArray()
	{
		return m_overlappingPairArray;
	}

	const BroadphasePairArray& getOverlappingPairArray() const
	{
		return m_overlappingPairArray;
	}

	BroadphasePair* getOverlappingPairArrayPtr()
	{
		return &m_overlappingPairArray[0];
	}

	const BroadphasePair* getOverlappingPairArrayPtr() const
	{
		return &m_overlappingPairArray[0];
	}

	i32 getNumOverlappingPairs() const
	{
		return m_overlappingPairArray.size();
	}

	OverlapFilterCallback* getOverlapFilterCallback()
	{
		return m_overlapFilterCallback;
	}

	void setOverlapFilterCallback(OverlapFilterCallback* callback)
	{
		m_overlapFilterCallback = callback;
	}

	virtual bool hasDeferredRemoval()
	{
		return m_hasDeferredRemoval;
	}

	virtual void setInternalGhostPairCallback(OverlappingPairCallback* ghostPairCallback)
	{
		m_ghostPairCallback = ghostPairCallback;
	}

	virtual void sortOverlappingPairs(Dispatcher* dispatcher);
};

//NullPairCache skips add/removal of overlapping pairs. Userful for benchmarking and unit testing.
class NullPairCache : public OverlappingPairCache
{
	BroadphasePairArray m_overlappingPairArray;

public:
	virtual BroadphasePair* getOverlappingPairArrayPtr()
	{
		return &m_overlappingPairArray[0];
	}
	const BroadphasePair* getOverlappingPairArrayPtr() const
	{
		return &m_overlappingPairArray[0];
	}
	BroadphasePairArray& getOverlappingPairArray()
	{
		return m_overlappingPairArray;
	}

	virtual void cleanOverlappingPair(BroadphasePair& /*pair*/, Dispatcher* /*dispatcher*/)
	{
	}

	virtual i32 getNumOverlappingPairs() const
	{
		return 0;
	}

	virtual void cleanProxyFromPairs(BroadphaseProxy* /*proxy*/, Dispatcher* /*dispatcher*/)
	{
	}

	bool needsBroadphaseCollision(BroadphaseProxy*, BroadphaseProxy*) const
	{
		return true;
	}
	OverlapFilterCallback* getOverlapFilterCallback()
	{
		return 0;
	}
	virtual void setOverlapFilterCallback(OverlapFilterCallback* /*callback*/)
	{
	}

	virtual void processAllOverlappingPairs(OverlapCallback*, Dispatcher* /*dispatcher*/)
	{
	}

	virtual BroadphasePair* findPair(BroadphaseProxy* /*proxy0*/, BroadphaseProxy* /*proxy1*/)
	{
		return 0;
	}

	virtual bool hasDeferredRemoval()
	{
		return true;
	}

	virtual void setInternalGhostPairCallback(OverlappingPairCallback* /* ghostPairCallback */)
	{
	}

	virtual BroadphasePair* addOverlappingPair(BroadphaseProxy* /*proxy0*/, BroadphaseProxy* /*proxy1*/)
	{
		return 0;
	}

	virtual uk removeOverlappingPair(BroadphaseProxy* /*proxy0*/, BroadphaseProxy* /*proxy1*/, Dispatcher* /*dispatcher*/)
	{
		return 0;
	}

	virtual void removeOverlappingPairsContainingProxy(BroadphaseProxy* /*proxy0*/, Dispatcher* /*dispatcher*/)
	{
	}

	virtual void sortOverlappingPairs(Dispatcher* dispatcher)
	{
		(void)dispatcher;
	}
};

#endif  //DRX3D_OVERLAPPING_PAIR_CACHE_H
