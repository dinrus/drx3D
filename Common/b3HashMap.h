#ifndef D3_HASH_MAP_H
#define D3_HASH_MAP_H

#include <drx3D/Common/b3AlignedObjectArray.h>

#include <string>

///very basic hashable string implementation, compatible with b3HashMap
struct b3HashString
{
	STxt m_string;
	u32 m_hash;

	D3_FORCE_INLINE u32 getHash() const
	{
		return m_hash;
	}

	b3HashString(tukk name)
		: m_string(name)
	{
		/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
		static u32k InitialFNV = 2166136261u;
		static u32k FNVMultiple = 16777619u;

		/* Fowler / Noll / Vo (FNV) Hash */
		u32 hash = InitialFNV;
		i32 len = m_string.length();
		for (i32 i = 0; i < len; i++)
		{
			hash = hash ^ (m_string[i]); /* xor  the low 8 bits */
			hash = hash * FNVMultiple;   /* multiply by the magic number */
		}
		m_hash = hash;
	}

	i32 portableStringCompare(tukk src, tukk dst) const
	{
		i32 ret = 0;

		while (!(ret = *(u8*)src - *(u8*)dst) && *dst)
			++src, ++dst;

		if (ret < 0)
			ret = -1;
		else if (ret > 0)
			ret = 1;

		return (ret);
	}

	bool equals(const b3HashString& other) const
	{
		return (m_string == other.m_string);
	}
};

i32k D3_HASH_NULL = 0xffffffff;

class b3HashInt
{
	i32 m_uid;

public:
	b3HashInt(i32 uid) : m_uid(uid)
	{
	}

	i32 getUid1() const
	{
		return m_uid;
	}

	void setUid1(i32 uid)
	{
		m_uid = uid;
	}

	bool equals(const b3HashInt& other) const
	{
		return getUid1() == other.getUid1();
	}
	//to our success
	D3_FORCE_INLINE u32 getHash() const
	{
		i32 key = m_uid;
		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}
};

class b3HashPtr
{
	union {
		ukk m_pointer;
		i32 m_hashValues[2];
	};

public:
	b3HashPtr(ukk ptr)
		: m_pointer(ptr)
	{
	}

	ukk getPointer() const
	{
		return m_pointer;
	}

	bool equals(const b3HashPtr& other) const
	{
		return getPointer() == other.getPointer();
	}

	//to our success
	D3_FORCE_INLINE u32 getHash() const
	{
		const bool VOID_IS_8 = ((sizeof(uk ) == 8));

		i32 key = VOID_IS_8 ? m_hashValues[0] + m_hashValues[1] : m_hashValues[0];

		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}
};

template <class Value>
class b3HashKeyPtr
{
	i32 m_uid;

public:
	b3HashKeyPtr(i32 uid) : m_uid(uid)
	{
	}

	i32 getUid1() const
	{
		return m_uid;
	}

	bool equals(const b3HashKeyPtr<Value>& other) const
	{
		return getUid1() == other.getUid1();
	}

	//to our success
	D3_FORCE_INLINE u32 getHash() const
	{
		i32 key = m_uid;
		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}
};

template <class Value>
class b3HashKey
{
	i32 m_uid;

public:
	b3HashKey(i32 uid) : m_uid(uid)
	{
	}

	i32 getUid1() const
	{
		return m_uid;
	}

	bool equals(const b3HashKey<Value>& other) const
	{
		return getUid1() == other.getUid1();
	}
	//to our success
	D3_FORCE_INLINE u32 getHash() const
	{
		i32 key = m_uid;
		// Thomas Wang's hash
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}
};

///The b3HashMap template class implements a generic and lightweight hashmap.
///A basic sample of how to use b3HashMap is located in Demos\BasicDemo\main.cpp
template <class Key, class Value>
class b3HashMap
{
protected:
	b3AlignedObjectArray<i32> m_hashTable;
	b3AlignedObjectArray<i32> m_next;

	b3AlignedObjectArray<Value> m_valueArray;
	b3AlignedObjectArray<Key> m_keyArray;

	void growTables(const Key& /*key*/)
	{
		i32 newCapacity = m_valueArray.capacity();

		if (m_hashTable.size() < newCapacity)
		{
			//grow hashtable and next table
			i32 curHashtableSize = m_hashTable.size();

			m_hashTable.resize(newCapacity);
			m_next.resize(newCapacity);

			i32 i;

			for (i = 0; i < newCapacity; ++i)
			{
				m_hashTable[i] = D3_HASH_NULL;
			}
			for (i = 0; i < newCapacity; ++i)
			{
				m_next[i] = D3_HASH_NULL;
			}

			for (i = 0; i < curHashtableSize; i++)
			{
				//const Value& value = m_valueArray[i];
				//const Key& key = m_keyArray[i];

				i32 hashValue = m_keyArray[i].getHash() & (m_valueArray.capacity() - 1);  // New hash value with new mask
				m_next[i] = m_hashTable[hashValue];
				m_hashTable[hashValue] = i;
			}
		}
	}

public:
	void insert(const Key& key, const Value& value)
	{
		i32 hash = key.getHash() & (m_valueArray.capacity() - 1);

		//replace value if the key is already there
		i32 index = findIndex(key);
		if (index != D3_HASH_NULL)
		{
			m_valueArray[index] = value;
			return;
		}

		i32 count = m_valueArray.size();
		i32 oldCapacity = m_valueArray.capacity();
		m_valueArray.push_back(value);
		m_keyArray.push_back(key);

		i32 newCapacity = m_valueArray.capacity();
		if (oldCapacity < newCapacity)
		{
			growTables(key);
			//hash with new capacity
			hash = key.getHash() & (m_valueArray.capacity() - 1);
		}
		m_next[count] = m_hashTable[hash];
		m_hashTable[hash] = count;
	}

	void remove(const Key& key)
	{
		i32 hash = key.getHash() & (m_valueArray.capacity() - 1);

		i32 pairIndex = findIndex(key);

		if (pairIndex == D3_HASH_NULL)
		{
			return;
		}

		// Remove the pair from the hash table.
		i32 index = m_hashTable[hash];
		drx3DAssert(index != D3_HASH_NULL);

		i32 previous = D3_HASH_NULL;
		while (index != pairIndex)
		{
			previous = index;
			index = m_next[index];
		}

		if (previous != D3_HASH_NULL)
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

		i32 lastPairIndex = m_valueArray.size() - 1;

		// If the removed pair is the last pair, we are done.
		if (lastPairIndex == pairIndex)
		{
			m_valueArray.pop_back();
			m_keyArray.pop_back();
			return;
		}

		// Remove the last pair from the hash table.
		i32 lastHash = m_keyArray[lastPairIndex].getHash() & (m_valueArray.capacity() - 1);

		index = m_hashTable[lastHash];
		drx3DAssert(index != D3_HASH_NULL);

		previous = D3_HASH_NULL;
		while (index != lastPairIndex)
		{
			previous = index;
			index = m_next[index];
		}

		if (previous != D3_HASH_NULL)
		{
			drx3DAssert(m_next[previous] == lastPairIndex);
			m_next[previous] = m_next[lastPairIndex];
		}
		else
		{
			m_hashTable[lastHash] = m_next[lastPairIndex];
		}

		// Copy the last pair into the remove pair's spot.
		m_valueArray[pairIndex] = m_valueArray[lastPairIndex];
		m_keyArray[pairIndex] = m_keyArray[lastPairIndex];

		// Insert the last pair into the hash table
		m_next[pairIndex] = m_hashTable[lastHash];
		m_hashTable[lastHash] = pairIndex;

		m_valueArray.pop_back();
		m_keyArray.pop_back();
	}

	i32 size() const
	{
		return m_valueArray.size();
	}

	const Value* getAtIndex(i32 index) const
	{
		drx3DAssert(index < m_valueArray.size());

		return &m_valueArray[index];
	}

	Value* getAtIndex(i32 index)
	{
		drx3DAssert(index < m_valueArray.size());

		return &m_valueArray[index];
	}

	Key getKeyAtIndex(i32 index)
	{
		drx3DAssert(index < m_keyArray.size());
		return m_keyArray[index];
	}

	const Key getKeyAtIndex(i32 index) const
	{
		drx3DAssert(index < m_keyArray.size());
		return m_keyArray[index];
	}

	Value* operator[](const Key& key)
	{
		return find(key);
	}

	const Value* find(const Key& key) const
	{
		i32 index = findIndex(key);
		if (index == D3_HASH_NULL)
		{
			return NULL;
		}
		return &m_valueArray[index];
	}

	Value* find(const Key& key)
	{
		i32 index = findIndex(key);
		if (index == D3_HASH_NULL)
		{
			return nullptr;
		}
		return &m_valueArray[index];
	}

	i32 findIndex(const Key& key) const
	{
		u32 hash = key.getHash() & (m_valueArray.capacity() - 1);

		if (hash >= (u32)m_hashTable.size())
		{
			return D3_HASH_NULL;
		}

		i32 index = m_hashTable[hash];
		while ((index != D3_HASH_NULL) && key.equals(m_keyArray[index]) == false)
		{
			index = m_next[index];
		}
		return index;
	}

	void clear()
	{
		m_hashTable.clear();
		m_next.clear();
		m_valueArray.clear();
		m_keyArray.clear();
	}
};

#endif  //D3_HASH_MAP_H
