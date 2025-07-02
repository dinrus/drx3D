#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>

#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

#include <stdio.h>

HashedOverlappingPairCache::HashedOverlappingPairCache() : m_overlapFilterCallback(0),
															   m_ghostPairCallback(0)
{
	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve(initialAllocatedSize);
	growTables();
}

HashedOverlappingPairCache::~HashedOverlappingPairCache()
{
}

void HashedOverlappingPairCache::cleanOverlappingPair(BroadphasePair& pair, Dispatcher* dispatcher)
{
	if (pair.m_algorithm && dispatcher)
	{
		{
			pair.m_algorithm->~CollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
			pair.m_algorithm = 0;
		}
	}
}

void HashedOverlappingPairCache::cleanProxyFromPairs(BroadphaseProxy* proxy, Dispatcher* dispatcher)
{
	class CleanPairCallback : public OverlapCallback
	{
		BroadphaseProxy* m_cleanProxy;
		OverlappingPairCache* m_pairCache;
		Dispatcher* m_dispatcher;

	public:
		CleanPairCallback(BroadphaseProxy* cleanProxy, OverlappingPairCache* pairCache, Dispatcher* dispatcher)
			: m_cleanProxy(cleanProxy),
			  m_pairCache(pairCache),
			  m_dispatcher(dispatcher)
		{
		}
		virtual bool processOverlap(BroadphasePair& pair)
		{
			if ((pair.m_pProxy0 == m_cleanProxy) ||
				(pair.m_pProxy1 == m_cleanProxy))
			{
				m_pairCache->cleanOverlappingPair(pair, m_dispatcher);
			}
			return false;
		}
	};

	CleanPairCallback cleanPairs(proxy, this, dispatcher);

	processAllOverlappingPairs(&cleanPairs, dispatcher);
}

void HashedOverlappingPairCache::removeOverlappingPairsContainingProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher)
{
	class RemovePairCallback : public OverlapCallback
	{
		BroadphaseProxy* m_obsoleteProxy;

	public:
		RemovePairCallback(BroadphaseProxy* obsoleteProxy)
			: m_obsoleteProxy(obsoleteProxy)
		{
		}
		virtual bool processOverlap(BroadphasePair& pair)
		{
			return ((pair.m_pProxy0 == m_obsoleteProxy) ||
					(pair.m_pProxy1 == m_obsoleteProxy));
		}
	};

	RemovePairCallback removeCallback(proxy);

	processAllOverlappingPairs(&removeCallback, dispatcher);
}

BroadphasePair* HashedOverlappingPairCache::findPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0->getUid();
	i32 proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2)
		Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	if (hash >= m_hashTable.size())
	{
		return NULL;
	}

	i32 index = m_hashTable[hash];
	while (index != DRX3D_NULL_PAIR && equalsPair(m_overlappingPairArray[index], proxyId1, proxyId2) == false)
	{
		index = m_next[index];
	}

	if (index == DRX3D_NULL_PAIR)
	{
		return NULL;
	}

	Assert(index < m_overlappingPairArray.size());

	return &m_overlappingPairArray[index];
}

//#include <stdio.h>

void HashedOverlappingPairCache::growTables()
{
	i32 newCapacity = m_overlappingPairArray.capacity();

	if (m_hashTable.size() < newCapacity)
	{
		//grow hashtable and next table
		i32 curHashtableSize = m_hashTable.size();

		m_hashTable.resize(newCapacity);
		m_next.resize(newCapacity);

		i32 i;

		for (i = 0; i < newCapacity; ++i)
		{
			m_hashTable[i] = DRX3D_NULL_PAIR;
		}
		for (i = 0; i < newCapacity; ++i)
		{
			m_next[i] = DRX3D_NULL_PAIR;
		}

		for (i = 0; i < curHashtableSize; i++)
		{
			const BroadphasePair& pair = m_overlappingPairArray[i];
			i32 proxyId1 = pair.m_pProxy0->getUid();
			i32 proxyId2 = pair.m_pProxy1->getUid();
			/*if (proxyId1 > proxyId2) 
				Swap(proxyId1, proxyId2);*/
			i32 hashValue = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask
			m_next[i] = m_hashTable[hashValue];
			m_hashTable[hashValue] = i;
		}
	}
}

BroadphasePair* HashedOverlappingPairCache::internalAddPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0->getUid();
	i32 proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2) 
		Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask

	BroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
	if (pair != NULL)
	{
		return pair;
	}
	/*for(i32 i=0;i<m_overlappingPairArray.size();++i)
		{
		if(	(m_overlappingPairArray[i].m_pProxy0==proxy0)&&
			(m_overlappingPairArray[i].m_pProxy1==proxy1))
			{
			printf("Adding duplicated %u<>%u\r\n",proxyId1,proxyId2);
			internalFindPair(proxy0, proxy1, hash);
			}
		}*/
	i32 count = m_overlappingPairArray.size();
	i32 oldCapacity = m_overlappingPairArray.capacity();
	uk mem = &m_overlappingPairArray.expandNonInitializing();

	//this is where we add an actual pair, so also call the 'ghost'
	if (m_ghostPairCallback)
		m_ghostPairCallback->addOverlappingPair(proxy0, proxy1);

	i32 newCapacity = m_overlappingPairArray.capacity();

	if (oldCapacity < newCapacity)
	{
		growTables();
		//hash with new capacity
		hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));
	}

	pair = new (mem) BroadphasePair(*proxy0, *proxy1);
	//	pair->m_pProxy0 = proxy0;
	//	pair->m_pProxy1 = proxy1;
	pair->m_algorithm = 0;
	pair->m_internalTmpValue = 0;

	m_next[count] = m_hashTable[hash];
	m_hashTable[hash] = count;

	return pair;
}

uk HashedOverlappingPairCache::removeOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1, Dispatcher* dispatcher)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0->getUid();
	i32 proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2)
		Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	BroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
	if (pair == NULL)
	{
		return 0;
	}

	cleanOverlappingPair(*pair, dispatcher);

	uk userData = pair->m_internalInfo1;

	Assert(pair->m_pProxy0->getUid() == proxyId1);
	Assert(pair->m_pProxy1->getUid() == proxyId2);

	i32 pairIndex = i32(pair - &m_overlappingPairArray[0]);
	Assert(pairIndex < m_overlappingPairArray.size());

	// Remove the pair from the hash table.
	i32 index = m_hashTable[hash];
	Assert(index != DRX3D_NULL_PAIR);

	i32 previous = DRX3D_NULL_PAIR;
	while (index != pairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != DRX3D_NULL_PAIR)
	{
		Assert(m_next[previous] == pairIndex);
		m_next[previous] = m_next[pairIndex];
	}
	else
	{
		m_hashTable[hash] = m_next[pairIndex];
	}

	// We now move the last pair into spot of the
	// pair being removed. We need to fix the hash
	// table indices to support the move.

	i32 lastPairIndex = m_overlappingPairArray.size() - 1;

	if (m_ghostPairCallback)
		m_ghostPairCallback->removeOverlappingPair(proxy0, proxy1, dispatcher);

	// If the removed pair is the last pair, we are done.
	if (lastPairIndex == pairIndex)
	{
		m_overlappingPairArray.pop_back();
		return userData;
	}

	// Remove the last pair from the hash table.
	const BroadphasePair* last = &m_overlappingPairArray[lastPairIndex];
	/* missing swap here too, Nat. */
	i32 lastHash = static_cast<i32>(getHash(static_cast<u32>(last->m_pProxy0->getUid()), static_cast<u32>(last->m_pProxy1->getUid())) & (m_overlappingPairArray.capacity() - 1));

	index = m_hashTable[lastHash];
	Assert(index != DRX3D_NULL_PAIR);

	previous = DRX3D_NULL_PAIR;
	while (index != lastPairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != DRX3D_NULL_PAIR)
	{
		Assert(m_next[previous] == lastPairIndex);
		m_next[previous] = m_next[lastPairIndex];
	}
	else
	{
		m_hashTable[lastHash] = m_next[lastPairIndex];
	}

	// Copy the last pair into the remove pair's spot.
	m_overlappingPairArray[pairIndex] = m_overlappingPairArray[lastPairIndex];

	// Insert the last pair into the hash table
	m_next[pairIndex] = m_hashTable[lastHash];
	m_hashTable[lastHash] = pairIndex;

	m_overlappingPairArray.pop_back();

	return userData;
}
//#include <stdio.h>
#include <drx3D/Maths/Linear/Quickprof.h>
void HashedOverlappingPairCache::processAllOverlappingPairs(OverlapCallback* callback, Dispatcher* dispatcher)
{
	DRX3D_PROFILE("HashedOverlappingPairCache::processAllOverlappingPairs");
	i32 i;

	//	printf("m_overlappingPairArray.size()=%d\n",m_overlappingPairArray.size());
	for (i = 0; i < m_overlappingPairArray.size();)
	{
		BroadphasePair* pair = &m_overlappingPairArray[i];
		if (callback->processOverlap(*pair))
		{
			removeOverlappingPair(pair->m_pProxy0, pair->m_pProxy1, dispatcher);
		}
		else
		{
			i++;
		}
	}
}

struct MyPairIndex
{
	i32 m_orgIndex;
	i32 m_uidA0;
	i32 m_uidA1;
};

class MyPairIndeSortPredicate
{
public:
	bool operator()(const MyPairIndex& a, const MyPairIndex& b) const
	{
		i32k uidA0 = a.m_uidA0;
		i32k uidB0 = b.m_uidA0;
		i32k uidA1 = a.m_uidA1;
		i32k uidB1 = b.m_uidA1;
		return uidA0 > uidB0 || (uidA0 == uidB0 && uidA1 > uidB1);
	}
};

void HashedOverlappingPairCache::processAllOverlappingPairs(OverlapCallback* callback, Dispatcher* dispatcher, const struct DispatcherInfo& dispatchInfo)
{
	if (dispatchInfo.m_deterministicOverlappingPairs)
	{
		BroadphasePairArray& pa = getOverlappingPairArray();
		AlignedObjectArray<MyPairIndex> indices;
		{
			DRX3D_PROFILE("sortOverlappingPairs");
			indices.resize(pa.size());
			for (i32 i = 0; i < indices.size(); i++)
			{
				const BroadphasePair& p = pa[i];
				i32k uidA0 = p.m_pProxy0 ? p.m_pProxy0->m_uniqueId : -1;
				i32k uidA1 = p.m_pProxy1 ? p.m_pProxy1->m_uniqueId : -1;

				indices[i].m_uidA0 = uidA0;
				indices[i].m_uidA1 = uidA1;
				indices[i].m_orgIndex = i;
			}
			indices.quickSort(MyPairIndeSortPredicate());
		}
		{
			DRX3D_PROFILE("HashedOverlappingPairCache::processAllOverlappingPairs");
			i32 i;
			for (i = 0; i < indices.size();)
			{
				BroadphasePair* pair = &pa[indices[i].m_orgIndex];
				if (callback->processOverlap(*pair))
				{
					removeOverlappingPair(pair->m_pProxy0, pair->m_pProxy1, dispatcher);
				}
				else
				{
					i++;
				}
			}
		}
	}
	else
	{
		processAllOverlappingPairs(callback, dispatcher);
	}
}

void HashedOverlappingPairCache::sortOverlappingPairs(Dispatcher* dispatcher)
{
	///need to keep hashmap in sync with pair address, so rebuild all
	BroadphasePairArray tmpPairs;
	i32 i;
	for (i = 0; i < m_overlappingPairArray.size(); i++)
	{
		tmpPairs.push_back(m_overlappingPairArray[i]);
	}

	for (i = 0; i < tmpPairs.size(); i++)
	{
		removeOverlappingPair(tmpPairs[i].m_pProxy0, tmpPairs[i].m_pProxy1, dispatcher);
	}

	for (i = 0; i < m_next.size(); i++)
	{
		m_next[i] = DRX3D_NULL_PAIR;
	}

	tmpPairs.quickSort(BroadphasePairSortPredicate());

	for (i = 0; i < tmpPairs.size(); i++)
	{
		addOverlappingPair(tmpPairs[i].m_pProxy0, tmpPairs[i].m_pProxy1);
	}
}

uk SortedOverlappingPairCache::removeOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1, Dispatcher* dispatcher)
{
	if (!hasDeferredRemoval())
	{
		BroadphasePair findPair(*proxy0, *proxy1);

		i32 findIndex = m_overlappingPairArray.findLinearSearch(findPair);
		if (findIndex < m_overlappingPairArray.size())
		{
			BroadphasePair& pair = m_overlappingPairArray[findIndex];
			uk userData = pair.m_internalInfo1;
			cleanOverlappingPair(pair, dispatcher);
			if (m_ghostPairCallback)
				m_ghostPairCallback->removeOverlappingPair(proxy0, proxy1, dispatcher);

			m_overlappingPairArray.swap(findIndex, m_overlappingPairArray.capacity() - 1);
			m_overlappingPairArray.pop_back();
			return userData;
		}
	}

	return 0;
}

BroadphasePair* SortedOverlappingPairCache::addOverlappingPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
{
	//don't add overlap with own
	Assert(proxy0 != proxy1);

	if (!needsBroadphaseCollision(proxy0, proxy1))
		return 0;

	uk mem = &m_overlappingPairArray.expandNonInitializing();
	BroadphasePair* pair = new (mem) BroadphasePair(*proxy0, *proxy1);

	if (m_ghostPairCallback)
		m_ghostPairCallback->addOverlappingPair(proxy0, proxy1);
	return pair;
}

///this findPair becomes really slow. Either sort the list to speedup the query, or
///use a different solution. It is mainly used for Removing overlapping pairs. Removal could be delayed.
///we could keep a linked list in each proxy, and store pair in one of the proxies (with lowest memory address)
///Also we can use a 2D bitmap, which can be useful for a future GPU implementation
BroadphasePair* SortedOverlappingPairCache::findPair(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
{
	if (!needsBroadphaseCollision(proxy0, proxy1))
		return 0;

	BroadphasePair tmpPair(*proxy0, *proxy1);
	i32 findIndex = m_overlappingPairArray.findLinearSearch(tmpPair);

	if (findIndex < m_overlappingPairArray.size())
	{
		//Assert(it != m_overlappingPairSet.end());
		BroadphasePair* pair = &m_overlappingPairArray[findIndex];
		return pair;
	}
	return 0;
}

//#include <stdio.h>

void SortedOverlappingPairCache::processAllOverlappingPairs(OverlapCallback* callback, Dispatcher* dispatcher)
{
	i32 i;

	for (i = 0; i < m_overlappingPairArray.size();)
	{
		BroadphasePair* pair = &m_overlappingPairArray[i];
		if (callback->processOverlap(*pair))
		{
			cleanOverlappingPair(*pair, dispatcher);
			pair->m_pProxy0 = 0;
			pair->m_pProxy1 = 0;
			m_overlappingPairArray.swap(i, m_overlappingPairArray.size() - 1);
			m_overlappingPairArray.pop_back();
		}
		else
		{
			i++;
		}
	}
}

SortedOverlappingPairCache::SortedOverlappingPairCache() : m_blockedForChanges(false),
															   m_hasDeferredRemoval(true),
															   m_overlapFilterCallback(0),
															   m_ghostPairCallback(0)
{
	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve(initialAllocatedSize);
}

SortedOverlappingPairCache::~SortedOverlappingPairCache()
{
}

void SortedOverlappingPairCache::cleanOverlappingPair(BroadphasePair& pair, Dispatcher* dispatcher)
{
	if (pair.m_algorithm)
	{
		{
			pair.m_algorithm->~CollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
			pair.m_algorithm = 0;
		}
	}
}

void SortedOverlappingPairCache::cleanProxyFromPairs(BroadphaseProxy* proxy, Dispatcher* dispatcher)
{
	class CleanPairCallback : public OverlapCallback
	{
		BroadphaseProxy* m_cleanProxy;
		OverlappingPairCache* m_pairCache;
		Dispatcher* m_dispatcher;

	public:
		CleanPairCallback(BroadphaseProxy* cleanProxy, OverlappingPairCache* pairCache, Dispatcher* dispatcher)
			: m_cleanProxy(cleanProxy),
			  m_pairCache(pairCache),
			  m_dispatcher(dispatcher)
		{
		}
		virtual bool processOverlap(BroadphasePair& pair)
		{
			if ((pair.m_pProxy0 == m_cleanProxy) ||
				(pair.m_pProxy1 == m_cleanProxy))
			{
				m_pairCache->cleanOverlappingPair(pair, m_dispatcher);
			}
			return false;
		}
	};

	CleanPairCallback cleanPairs(proxy, this, dispatcher);

	processAllOverlappingPairs(&cleanPairs, dispatcher);
}

void SortedOverlappingPairCache::removeOverlappingPairsContainingProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher)
{
	class RemovePairCallback : public OverlapCallback
	{
		BroadphaseProxy* m_obsoleteProxy;

	public:
		RemovePairCallback(BroadphaseProxy* obsoleteProxy)
			: m_obsoleteProxy(obsoleteProxy)
		{
		}
		virtual bool processOverlap(BroadphasePair& pair)
		{
			return ((pair.m_pProxy0 == m_obsoleteProxy) ||
					(pair.m_pProxy1 == m_obsoleteProxy));
		}
	};

	RemovePairCallback removeCallback(proxy);

	processAllOverlappingPairs(&removeCallback, dispatcher);
}

void SortedOverlappingPairCache::sortOverlappingPairs(Dispatcher* dispatcher)
{
	//should already be sorted
}
