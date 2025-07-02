#include <drx3D/Physics/Collision/BroadPhase/b3OverlappingPairCache.h>
#include <drx3D/Geometry/b3AabbUtil.h>
#include <stdio.h>

i32 b3g_overlappingPairs = 0;
i32 b3g_removePairs = 0;
i32 b3g_addedPairs = 0;
i32 b3g_findPairs = 0;

b3HashedOverlappingPairCache::b3HashedOverlappingPairCache() : m_overlapFilterCallback(0)
//,	m_blockedForChanges(false)
{
	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve(initialAllocatedSize);
	growTables();
}

b3HashedOverlappingPairCache::~b3HashedOverlappingPairCache()
{
}

void b3HashedOverlappingPairCache::cleanOverlappingPair(b3BroadphasePair& pair, b3Dispatcher* dispatcher)
{
	/*	if (pair.m_algorithm)
	{
		{
			pair.m_algorithm->~b3CollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
			pair.m_algorithm=0;
		}
	}
	*/
}

void b3HashedOverlappingPairCache::cleanProxyFromPairs(i32 proxy, b3Dispatcher* dispatcher)
{
	class CleanPairCallback : public b3OverlapCallback
	{
		i32 m_cleanProxy;
		b3OverlappingPairCache* m_pairCache;
		b3Dispatcher* m_dispatcher;

	public:
		CleanPairCallback(i32 cleanProxy, b3OverlappingPairCache* pairCache, b3Dispatcher* dispatcher)
			: m_cleanProxy(cleanProxy),
			  m_pairCache(pairCache),
			  m_dispatcher(dispatcher)
		{
		}
		virtual bool processOverlap(b3BroadphasePair& pair)
		{
			if ((pair.x == m_cleanProxy) ||
				(pair.y == m_cleanProxy))
			{
				m_pairCache->cleanOverlappingPair(pair, m_dispatcher);
			}
			return false;
		}
	};

	CleanPairCallback cleanPairs(proxy, this, dispatcher);

	processAllOverlappingPairs(&cleanPairs, dispatcher);
}

void b3HashedOverlappingPairCache::removeOverlappingPairsContainingProxy(i32 proxy, b3Dispatcher* dispatcher)
{
	class RemovePairCallback : public b3OverlapCallback
	{
		i32 m_obsoleteProxy;

	public:
		RemovePairCallback(i32 obsoleteProxy)
			: m_obsoleteProxy(obsoleteProxy)
		{
		}
		virtual bool processOverlap(b3BroadphasePair& pair)
		{
			return ((pair.x == m_obsoleteProxy) ||
					(pair.y == m_obsoleteProxy));
		}
	};

	RemovePairCallback removeCallback(proxy);

	processAllOverlappingPairs(&removeCallback, dispatcher);
}

b3BroadphasePair* b3HashedOverlappingPairCache::findPair(i32 proxy0, i32 proxy1)
{
	b3g_findPairs++;
	if (proxy0 > proxy1)
		b3Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0;
	i32 proxyId2 = proxy1;

	/*if (proxyId1 > proxyId2) 
		b3Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	if (hash >= m_hashTable.size())
	{
		return NULL;
	}

	i32 index = m_hashTable[hash];
	while (index != D3_NULL_PAIR && equalsPair(m_overlappingPairArray[index], proxyId1, proxyId2) == false)
	{
		index = m_next[index];
	}

	if (index == D3_NULL_PAIR)
	{
		return NULL;
	}

	drx3DAssert(index < m_overlappingPairArray.size());

	return &m_overlappingPairArray[index];
}

//#include <stdio.h>

void b3HashedOverlappingPairCache::growTables()
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
			m_hashTable[i] = D3_NULL_PAIR;
		}
		for (i = 0; i < newCapacity; ++i)
		{
			m_next[i] = D3_NULL_PAIR;
		}

		for (i = 0; i < curHashtableSize; i++)
		{
			const b3BroadphasePair& pair = m_overlappingPairArray[i];
			i32 proxyId1 = pair.x;
			i32 proxyId2 = pair.y;
			/*if (proxyId1 > proxyId2) 
				b3Swap(proxyId1, proxyId2);*/
			i32 hashValue = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask
			m_next[i] = m_hashTable[hashValue];
			m_hashTable[hashValue] = i;
		}
	}
}

b3BroadphasePair* b3HashedOverlappingPairCache::internalAddPair(i32 proxy0, i32 proxy1)
{
	if (proxy0 > proxy1)
		b3Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0;
	i32 proxyId2 = proxy1;

	/*if (proxyId1 > proxyId2) 
		b3Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask

	b3BroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
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
	pair = &m_overlappingPairArray.expandNonInitializing();

	//this is where we add an actual pair, so also call the 'ghost'
	//	if (m_ghostPairCallback)
	//		m_ghostPairCallback->addOverlappingPair(proxy0,proxy1);

	i32 newCapacity = m_overlappingPairArray.capacity();

	if (oldCapacity < newCapacity)
	{
		growTables();
		//hash with new capacity
		hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));
	}

	*pair = b3MakeBroadphasePair(proxy0, proxy1);

	//	pair->m_pProxy0 = proxy0;
	//	pair->m_pProxy1 = proxy1;
	//pair->m_algorithm = 0;
	//pair->m_internalTmpValue = 0;

	m_next[count] = m_hashTable[hash];
	m_hashTable[hash] = count;

	return pair;
}

uk b3HashedOverlappingPairCache::removeOverlappingPair(i32 proxy0, i32 proxy1, b3Dispatcher* dispatcher)
{
	b3g_removePairs++;
	if (proxy0 > proxy1)
		b3Swap(proxy0, proxy1);
	i32 proxyId1 = proxy0;
	i32 proxyId2 = proxy1;

	/*if (proxyId1 > proxyId2) 
		b3Swap(proxyId1, proxyId2);*/

	i32 hash = static_cast<i32>(getHash(static_cast<u32>(proxyId1), static_cast<u32>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	b3BroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
	if (pair == NULL)
	{
		return 0;
	}

	cleanOverlappingPair(*pair, dispatcher);

	i32 pairIndex = i32(pair - &m_overlappingPairArray[0]);
	drx3DAssert(pairIndex < m_overlappingPairArray.size());

	// Remove the pair from the hash table.
	i32 index = m_hashTable[hash];
	drx3DAssert(index != D3_NULL_PAIR);

	i32 previous = D3_NULL_PAIR;
	while (index != pairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != D3_NULL_PAIR)
	{
		drx3DAssert(m_next[previous] == pairIndex);
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

	//if (m_ghostPairCallback)
	//	m_ghostPairCallback->removeOverlappingPair(proxy0, proxy1,dispatcher);

	// If the removed pair is the last pair, we are done.
	if (lastPairIndex == pairIndex)
	{
		m_overlappingPairArray.pop_back();
		return 0;
	}

	// Remove the last pair from the hash table.
	const b3BroadphasePair* last = &m_overlappingPairArray[lastPairIndex];
	/* missing swap here too, Nat. */
	i32 lastHash = static_cast<i32>(getHash(static_cast<u32>(last->x), static_cast<u32>(last->y)) & (m_overlappingPairArray.capacity() - 1));

	index = m_hashTable[lastHash];
	drx3DAssert(index != D3_NULL_PAIR);

	previous = D3_NULL_PAIR;
	while (index != lastPairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != D3_NULL_PAIR)
	{
		drx3DAssert(m_next[previous] == lastPairIndex);
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

	return 0;
}
//#include <stdio.h>

void b3HashedOverlappingPairCache::processAllOverlappingPairs(b3OverlapCallback* callback, b3Dispatcher* dispatcher)
{
	i32 i;

	//	printf("m_overlappingPairArray.size()=%d\n",m_overlappingPairArray.size());
	for (i = 0; i < m_overlappingPairArray.size();)
	{
		b3BroadphasePair* pair = &m_overlappingPairArray[i];
		if (callback->processOverlap(*pair))
		{
			removeOverlappingPair(pair->x, pair->y, dispatcher);

			b3g_overlappingPairs--;
		}
		else
		{
			i++;
		}
	}
}

void b3HashedOverlappingPairCache::sortOverlappingPairs(b3Dispatcher* dispatcher)
{
	///need to keep hashmap in sync with pair address, so rebuild all
	b3BroadphasePairArray tmpPairs;
	i32 i;
	for (i = 0; i < m_overlappingPairArray.size(); i++)
	{
		tmpPairs.push_back(m_overlappingPairArray[i]);
	}

	for (i = 0; i < tmpPairs.size(); i++)
	{
		removeOverlappingPair(tmpPairs[i].x, tmpPairs[i].y, dispatcher);
	}

	for (i = 0; i < m_next.size(); i++)
	{
		m_next[i] = D3_NULL_PAIR;
	}

	tmpPairs.quickSort(b3BroadphasePairSortPredicate());

	for (i = 0; i < tmpPairs.size(); i++)
	{
		addOverlappingPair(tmpPairs[i].x, tmpPairs[i].y);
	}
}

uk b3SortedOverlappingPairCache::removeOverlappingPair(i32 proxy0, i32 proxy1, b3Dispatcher* dispatcher)
{
	if (!hasDeferredRemoval())
	{
		b3BroadphasePair findPair = b3MakeBroadphasePair(proxy0, proxy1);

		i32 findIndex = m_overlappingPairArray.findLinearSearch(findPair);
		if (findIndex < m_overlappingPairArray.size())
		{
			b3g_overlappingPairs--;
			b3BroadphasePair& pair = m_overlappingPairArray[findIndex];

			cleanOverlappingPair(pair, dispatcher);
			//if (m_ghostPairCallback)
			//	m_ghostPairCallback->removeOverlappingPair(proxy0, proxy1,dispatcher);

			m_overlappingPairArray.swap(findIndex, m_overlappingPairArray.capacity() - 1);
			m_overlappingPairArray.pop_back();
			return 0;
		}
	}

	return 0;
}

b3BroadphasePair* b3SortedOverlappingPairCache::addOverlappingPair(i32 proxy0, i32 proxy1)
{
	//don't add overlap with own
	drx3DAssert(proxy0 != proxy1);

	if (!needsBroadphaseCollision(proxy0, proxy1))
		return 0;

	b3BroadphasePair* pair = &m_overlappingPairArray.expandNonInitializing();
	*pair = b3MakeBroadphasePair(proxy0, proxy1);

	b3g_overlappingPairs++;
	b3g_addedPairs++;

	//	if (m_ghostPairCallback)
	//		m_ghostPairCallback->addOverlappingPair(proxy0, proxy1);
	return pair;
}

///this findPair becomes really slow. Either sort the list to speedup the query, or
///use a different solution. It is mainly used for Removing overlapping pairs. Removal could be delayed.
///we could keep a linked list in each proxy, and store pair in one of the proxies (with lowest memory address)
///Also we can use a 2D bitmap, which can be useful for a future GPU implementation
b3BroadphasePair* b3SortedOverlappingPairCache::findPair(i32 proxy0, i32 proxy1)
{
	if (!needsBroadphaseCollision(proxy0, proxy1))
		return 0;

	b3BroadphasePair tmpPair = b3MakeBroadphasePair(proxy0, proxy1);
	i32 findIndex = m_overlappingPairArray.findLinearSearch(tmpPair);

	if (findIndex < m_overlappingPairArray.size())
	{
		//drx3DAssert(it != m_overlappingPairSet.end());
		b3BroadphasePair* pair = &m_overlappingPairArray[findIndex];
		return pair;
	}
	return 0;
}

//#include <stdio.h>

void b3SortedOverlappingPairCache::processAllOverlappingPairs(b3OverlapCallback* callback, b3Dispatcher* dispatcher)
{
	i32 i;

	for (i = 0; i < m_overlappingPairArray.size();)
	{
		b3BroadphasePair* pair = &m_overlappingPairArray[i];
		if (callback->processOverlap(*pair))
		{
			cleanOverlappingPair(*pair, dispatcher);
			pair->x = -1;
			pair->y = -1;
			m_overlappingPairArray.swap(i, m_overlappingPairArray.size() - 1);
			m_overlappingPairArray.pop_back();
			b3g_overlappingPairs--;
		}
		else
		{
			i++;
		}
	}
}

b3SortedOverlappingPairCache::b3SortedOverlappingPairCache() : m_blockedForChanges(false),
															   m_hasDeferredRemoval(true),
															   m_overlapFilterCallback(0)

{
	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve(initialAllocatedSize);
}

b3SortedOverlappingPairCache::~b3SortedOverlappingPairCache()
{
}

void b3SortedOverlappingPairCache::cleanOverlappingPair(b3BroadphasePair& pair, b3Dispatcher* dispatcher)
{
	/*	if (pair.m_algorithm)
	{
		{
			pair.m_algorithm->~b3CollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
			pair.m_algorithm=0;
			b3g_removePairs--;
		}
	}
	*/
}

void b3SortedOverlappingPairCache::cleanProxyFromPairs(i32 proxy, b3Dispatcher* dispatcher)
{
	class CleanPairCallback : public b3OverlapCallback
	{
		i32 m_cleanProxy;
		b3OverlappingPairCache* m_pairCache;
		b3Dispatcher* m_dispatcher;

	public:
		CleanPairCallback(i32 cleanProxy, b3OverlappingPairCache* pairCache, b3Dispatcher* dispatcher)
			: m_cleanProxy(cleanProxy),
			  m_pairCache(pairCache),
			  m_dispatcher(dispatcher)
		{
		}
		virtual bool processOverlap(b3BroadphasePair& pair)
		{
			if ((pair.x == m_cleanProxy) ||
				(pair.y == m_cleanProxy))
			{
				m_pairCache->cleanOverlappingPair(pair, m_dispatcher);
			}
			return false;
		}
	};

	CleanPairCallback cleanPairs(proxy, this, dispatcher);

	processAllOverlappingPairs(&cleanPairs, dispatcher);
}

void b3SortedOverlappingPairCache::removeOverlappingPairsContainingProxy(i32 proxy, b3Dispatcher* dispatcher)
{
	class RemovePairCallback : public b3OverlapCallback
	{
		i32 m_obsoleteProxy;

	public:
		RemovePairCallback(i32 obsoleteProxy)
			: m_obsoleteProxy(obsoleteProxy)
		{
		}
		virtual bool processOverlap(b3BroadphasePair& pair)
		{
			return ((pair.x == m_obsoleteProxy) ||
					(pair.y == m_obsoleteProxy));
		}
	};

	RemovePairCallback removeCallback(proxy);

	processAllOverlappingPairs(&removeCallback, dispatcher);
}

void b3SortedOverlappingPairCache::sortOverlappingPairs(b3Dispatcher* dispatcher)
{
	//should already be sorted
}
