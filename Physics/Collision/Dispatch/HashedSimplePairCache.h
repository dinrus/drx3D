#ifndef DRX3D_HASHED_SIMPLE_PAIR_CACHE_H
#define DRX3D_HASHED_SIMPLE_PAIR_CACHE_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

i32k DRX3D_SIMPLE_NULL_PAIR = 0xffffffff;

struct SimplePair
{
	SimplePair(i32 indexA, i32 indexB)
		: m_indexA(indexA),
		  m_indexB(indexB),
		  m_userPointer(0)
	{
	}

	i32 m_indexA;
	i32 m_indexB;
	union {
		uk m_userPointer;
		i32 m_userValue;
	};
};

typedef AlignedObjectArray<SimplePair> SimplePairArray;

#ifdef DRX3D_DEBUG_COLLISION_PAIRS
extern i32 gOverlappingSimplePairs;
extern i32 gRemoveSimplePairs;
extern i32 gAddedSimplePairs;
extern i32 gFindSimplePairs;
#endif  //DRX3D_DEBUG_COLLISION_PAIRS

class HashedSimplePairCache
{
	SimplePairArray m_overlappingPairArray;

protected:
	AlignedObjectArray<i32> m_hashTable;
	AlignedObjectArray<i32> m_next;

public:
	HashedSimplePairCache();
	virtual ~HashedSimplePairCache();

	void removeAllPairs();

	virtual uk removeOverlappingPair(i32 indexA, i32 indexB);

	// Add a pair and return the new pair. If the pair already exists,
	// no new pair is created and the old one is returned.
	virtual SimplePair* addOverlappingPair(i32 indexA, i32 indexB)
	{
#ifdef DRX3D_DEBUG_COLLISION_PAIRS
		gAddedSimplePairs++;
#endif

		return internalAddPair(indexA, indexB);
	}

	virtual SimplePair* getOverlappingPairArrayPtr()
	{
		return &m_overlappingPairArray[0];
	}

	const SimplePair* getOverlappingPairArrayPtr() const
	{
		return &m_overlappingPairArray[0];
	}

	SimplePairArray& getOverlappingPairArray()
	{
		return m_overlappingPairArray;
	}

	const SimplePairArray& getOverlappingPairArray() const
	{
		return m_overlappingPairArray;
	}

	SimplePair* findPair(i32 indexA, i32 indexB);

	i32 GetCount() const { return m_overlappingPairArray.size(); }

	i32 getNumOverlappingPairs() const
	{
		return m_overlappingPairArray.size();
	}

private:
	SimplePair* internalAddPair(i32 indexA, i32 indexB);

	void growTables();

	SIMD_FORCE_INLINE bool equalsPair(const SimplePair& pair, i32 indexA, i32 indexB)
	{
		return pair.m_indexA == indexA && pair.m_indexB == indexB;
	}

	SIMD_FORCE_INLINE u32 getHash(u32 indexA, u32 indexB)
	{
		u32 key = indexA | (indexB << 16);
		// Thomas Wang's hash

		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}

	SIMD_FORCE_INLINE SimplePair* internalFindPair(i32 proxyIdA, i32 proxyIdB, i32 hash)
	{
		i32 index = m_hashTable[hash];

		while (index != DRX3D_SIMPLE_NULL_PAIR && equalsPair(m_overlappingPairArray[index], proxyIdA, proxyIdB) == false)
		{
			index = m_next[index];
		}

		if (index == DRX3D_SIMPLE_NULL_PAIR)
		{
			return NULL;
		}

		Assert(index < m_overlappingPairArray.size());

		return &m_overlappingPairArray[index];
	}
};

#endif  //DRX3D_HASHED_SIMPLE_PAIR_CACHE_H
