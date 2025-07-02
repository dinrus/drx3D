#ifndef SHARED_MEMORY_USER_DATA_H
#define SHARED_MEMORY_USER_DATA_H

#include <string>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>

struct SharedMemoryUserData
{
	STxt m_key;
	i32 m_type;

	i32 m_bodyUniqueId;
	i32 m_linkIndex;
	i32 m_visualShapeIndex;

	AlignedObjectArray<char> m_bytes;

	SharedMemoryUserData()
		: m_type(-1), m_bodyUniqueId(-1), m_linkIndex(-1), m_visualShapeIndex(-1)
	{
	}

	SharedMemoryUserData(tukk key, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex)
		: m_key(key), m_type(-1), m_bodyUniqueId(bodyUniqueId), m_linkIndex(linkIndex), m_visualShapeIndex(visualShapeIndex)
	{
	}

	void replaceValue(tukk bytes, i32 len, i32 type)
	{
		m_type = type;
		m_bytes.resize(len);
		for (i32 i = 0; i < len; i++)
		{
			m_bytes[i] = bytes[i];
		}
	}

	virtual ~SharedMemoryUserData()
	{
	}

	void clear()
	{
		m_bytes.clear();
		m_type = -1;
	}
};

struct SharedMemoryUserDataHashKey
{
	u32 m_hash;

	HashString m_key;
	HashInt m_bodyUniqueId;
	HashInt m_linkIndex;
	HashInt m_visualShapeIndex;

	SIMD_FORCE_INLINE u32 getHash() const
	{
		return m_hash;
	}

	SharedMemoryUserDataHashKey() : m_hash(0) {}

	SharedMemoryUserDataHashKey(const struct SharedMemoryUserData* userData)
		: m_key(userData->m_key.c_str()),
		  m_bodyUniqueId(userData->m_bodyUniqueId),
		  m_linkIndex(userData->m_linkIndex),
		  m_visualShapeIndex(userData->m_visualShapeIndex)
	{
		calculateHash();
	}

	SharedMemoryUserDataHashKey(tukk key, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex)
		: m_key(key), m_bodyUniqueId(bodyUniqueId), m_linkIndex(linkIndex), m_visualShapeIndex(visualShapeIndex)
	{
		calculateHash();
	}

	void calculateHash()
	{
		m_hash = m_key.getHash() ^ m_bodyUniqueId.getHash() ^ m_linkIndex.getHash() ^ m_visualShapeIndex.getHash();
	}

	bool equals(const SharedMemoryUserDataHashKey& other) const
	{
		return m_bodyUniqueId.equals(other.m_bodyUniqueId) &&
			   m_linkIndex.equals(other.m_linkIndex) &&
			   m_visualShapeIndex.equals(other.m_visualShapeIndex) &&
			   m_key.equals(other.m_key);
	}
};

#endif  //SHARED_MEMORY_USER_DATA_H
