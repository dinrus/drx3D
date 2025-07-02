#include <drx3D/Physics/Collision/Dispatch/HashedSimplePairCache.h>

#include <stdio.h>

#ifdef DRX3D_DEBUG_COLLISION_PAIRS
i32 gOverlappingSimplePairs = 0;
i32 gRemoveSimplePairs = 0;
i32 gAddedSimplePairs = 0;
i32 gFindSimplePairs = 0;
#endif  //DRX3D_DEBUG_COLLISION_PAIRS

HashedSimplePairCache::HashedSimplePairCache()
{
	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve ( initialAllocatedSize );
	growTables();
}

HashedSimplePairCache::~HashedSimplePairCache()
{
}

void HashedSimplePairCache::removeAllPairs()
{
	m_overlappingPairArray.clear();
	m_hashTable.clear();
	m_next.clear();

	i32 initialAllocatedSize = 2;
	m_overlappingPairArray.reserve ( initialAllocatedSize );
	growTables();
}

SimplePair* HashedSimplePairCache::findPair ( i32 indexA, i32 indexB )
{
#ifdef DRX3D_DEBUG_COLLISION_PAIRS
	gFindSimplePairs++;
#endif

	/*if (indexA > indexB)
		Swap(indexA, indexB);*/

	i32 hash = static_cast<i32> ( getHash ( static_cast<u32> ( indexA ), static_cast<u32> ( indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );

	if ( hash >= m_hashTable.size() )
	{
		return NULL;
	}

	i32 index = m_hashTable[hash];

	while ( index != DRX3D_SIMPLE_NULL_PAIR && equalsPair ( m_overlappingPairArray[index], indexA, indexB ) == false )
	{
		index = m_next[index];
	}

	if ( index == DRX3D_SIMPLE_NULL_PAIR )
	{
		return NULL;
	}

	Assert ( index < m_overlappingPairArray.size() );

	return &m_overlappingPairArray[index];
}

//#include <stdio.h>

void HashedSimplePairCache::growTables()
{
	i32 newCapacity = m_overlappingPairArray.capacity();

	if ( m_hashTable.size() < newCapacity )
	{
		//grow hashtable and next table
		i32 curHashtableSize = m_hashTable.size();

		m_hashTable.resize ( newCapacity );
		m_next.resize ( newCapacity );

		i32 i;

		for ( i = 0; i < newCapacity; ++i )
		{
			m_hashTable[i] = DRX3D_SIMPLE_NULL_PAIR;
		}

		for ( i = 0; i < newCapacity; ++i )
		{
			m_next[i] = DRX3D_SIMPLE_NULL_PAIR;
		}

		for ( i = 0; i < curHashtableSize; i++ )
		{
			const SimplePair& pair = m_overlappingPairArray[i];
			i32 indexA = pair.m_indexA;
			i32 indexB = pair.m_indexB;

			i32 hashValue = static_cast<i32> ( getHash ( static_cast<u32> ( indexA ), static_cast<u32> ( indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );  // New hash value with new mask
			m_next[i] = m_hashTable[hashValue];
			m_hashTable[hashValue] = i;
		}
	}
}

SimplePair* HashedSimplePairCache::internalAddPair ( i32 indexA, i32 indexB )
{
	i32 hash = static_cast<i32> ( getHash ( static_cast<u32> ( indexA ), static_cast<u32> ( indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );  // New hash value with new mask

	SimplePair* pair = internalFindPair ( indexA, indexB, hash );

	if ( pair != NULL )
	{
		return pair;
	}

	i32 count = m_overlappingPairArray.size();

	i32 oldCapacity = m_overlappingPairArray.capacity();
	uk mem = &m_overlappingPairArray.expandNonInitializing();

	i32 newCapacity = m_overlappingPairArray.capacity();

	if ( oldCapacity < newCapacity )
	{
		growTables();
		//hash with new capacity
		hash = static_cast<i32> ( getHash ( static_cast<u32> ( indexA ), static_cast<u32> ( indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );
	}

	pair = new ( mem ) SimplePair ( indexA, indexB );

	pair->m_userPointer = 0;

	m_next[count] = m_hashTable[hash];
	m_hashTable[hash] = count;

	return pair;
}

uk HashedSimplePairCache::removeOverlappingPair ( i32 indexA, i32 indexB )
{
#ifdef DRX3D_DEBUG_COLLISION_PAIRS
	gRemoveSimplePairs++;
#endif

	/*if (indexA > indexB)
		Swap(indexA, indexB);*/

	i32 hash = static_cast<i32> ( getHash ( static_cast<u32> ( indexA ), static_cast<u32> ( indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );

	SimplePair* pair = internalFindPair ( indexA, indexB, hash );

	if ( pair == NULL )
	{
		return 0;
	}

	uk userData = pair->m_userPointer;

	i32 pairIndex = i32 ( pair - &m_overlappingPairArray[0] );
	Assert ( pairIndex < m_overlappingPairArray.size() );

	// Remove the pair from the hash table.
	i32 index = m_hashTable[hash];
	Assert ( index != DRX3D_SIMPLE_NULL_PAIR );

	i32 previous = DRX3D_SIMPLE_NULL_PAIR;

	while ( index != pairIndex )
	{
		previous = index;
		index = m_next[index];
	}

	if ( previous != DRX3D_SIMPLE_NULL_PAIR )
	{
		Assert ( m_next[previous] == pairIndex );
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

	// If the removed pair is the last pair, we are done.
	if ( lastPairIndex == pairIndex )
	{
		m_overlappingPairArray.pop_back();
		return userData;
	}

	// Remove the last pair from the hash table.
	const SimplePair* last = &m_overlappingPairArray[lastPairIndex];

	/* missing swap here too, Nat. */
	i32 lastHash = static_cast<i32> ( getHash ( static_cast<u32> ( last->m_indexA ), static_cast<u32> ( last->m_indexB ) ) & ( m_overlappingPairArray.capacity() - 1 ) );

	index = m_hashTable[lastHash];

	Assert ( index != DRX3D_SIMPLE_NULL_PAIR );

	previous = DRX3D_SIMPLE_NULL_PAIR;

	while ( index != lastPairIndex )
	{
		previous = index;
		index = m_next[index];
	}

	if ( previous != DRX3D_SIMPLE_NULL_PAIR )
	{
		Assert ( m_next[previous] == lastPairIndex );
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
