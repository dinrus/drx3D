// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// Unique identifier class used to represent parameters (inputs and outputs) passed between functions and nodes.
// Can be created from multiple sources:
//   + FromIdx    - Index, for example the index of a node's input/output
//   + FromUInt32 - This is convenient when assigning identifiers in C++ e.g. CUniqueId::FromUInt32('eid') // EntityId
//   + FromString - The hash of a string identifier
//   + FromGUID   - A globally unique identifier
// It is guaranteed that identifiers created from different sources will be different
// e.g. you could declare CUniqueId::FromUInt32(0) and CUniqueId::FromUInt32(0) within the same scope without a collision.

#pragma once

#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/GUID.h>

// #SchematycTODO:
// 1) Set SXEMA_HASH_UNIQUE_IDS 1 in order to reduce size of CUniqueId from 24bytes to 8bytes.
//    This should be safe since unique identifiers don't need to be globally unique, only unique within a set of parameters.
//    Once made this change cannot be reverted because files will only contain a uint64 hash of the source data.
// 2) Set SXEMA_PATCH_UNIQUE_IDS 0 once all files have been updated to new CUniqueId format.
// 3) Strip out redundant code.
#define SXEMA_HASH_UNIQUE_IDS  0
#define SXEMA_PATCH_UNIQUE_IDS 1

namespace sxema
{

#if SXEMA_HASH_UNIQUE_IDS
class CUniqueId
{
public:

	enum class EType
	{
		Empty,
		Idx,
		UInt32,
		StringHash,
		GUID
	};

private:

#if SXEMA_PATCH_UNIQUE_IDS
	class CPatchSerializer
	{
	public:

		inline CPatchSerializer(CUniqueId& value)
			: m_value(value)
		{}

		inline void Serialize(Serialization::IArchive& archive)
		{
			EType type = EType::Empty;
			if (archive(type, "type"))
			{
				switch (type)
				{
				case EType::Empty:
					{
						m_value.m_value = 0;
						break;
					}
				case EType::Idx:
				case EType::UInt32:
				case EType::StringHash:
					{
						u32 temp;
						archive(temp, "value");
						m_value.m_value = Pack(type, temp);
						break;
					}
				case EType::GUID:
					{
						DrxGUID temp;
						archive(temp, "value");
						m_value.m_value = Pack(type, GUID::Hash(temp));
						break;
					}
				}
			}
		}

	private:

		CUniqueId& m_value;
	};
#endif

public:

	inline CUniqueId()
		: m_value(0)
	{}

	inline CUniqueId(const CUniqueId& rhs)
		: m_value(rhs.m_value)
	{}

	inline bool IsEmpty() const
	{
		return m_value != 0;
	}

#ifdef SXEMA_PATCH_UNIQUE_IDS
	inline bool IsIdx() const
	{
		return static_cast<EType>(m_value >> 60) == EType::Idx;
	}

	inline u32 GetIdx() const
	{
		return static_cast<u32>(m_value);
	}
#endif

	inline bool operator==(const CUniqueId& rhs) const
	{
		return m_value == rhs.m_value;
	}

	inline bool operator!=(const CUniqueId& rhs) const
	{
		return m_value != rhs.m_value;
	}

	inline bool operator<(const CUniqueId& rhs) const
	{
		return m_value < rhs.m_value;
	}

	inline bool operator>(const CUniqueId& rhs) const
	{
		return m_value > rhs.m_value;
	}

	inline bool operator<=(const CUniqueId& rhs) const
	{
		return m_value <= rhs.m_value;
	}

	inline bool operator>=(const CUniqueId& rhs) const
	{
		return m_value >= rhs.m_value;
	}

	static inline CUniqueId FromIdx(u32 value)
	{
		return CUniqueId(Pack(EType::Idx, value));
	}

	static inline CUniqueId FromUInt32(u32 value)
	{
		return CUniqueId(Pack(EType::UInt32, value));
	}

	static inline CUniqueId FromString(tukk szValue)
	{
		return CUniqueId(Pack(EType::StringHash, DrxStringUtils::CalculateHash(szValue)));
	}

	static inline CUniqueId FromGUID(const DrxGUID& value)
	{
		return CUniqueId(Pack(EType::GUID, GUID::Hash(value)));
	}

	friend inline bool Serialize(Serialization::IArchive& archive, CUniqueId& value, tukk szName, tukk szLabel)
	{
		if (!archive.isEdit())
		{
#if SXEMA_PATCH_UNIQUE_IDS
			if (archive.isInput())
			{
				CPatchSerializer patchSerializer(value);
				if (archive(patchSerializer, szName))
				{
					return true;
				}
			}
#endif
			return archive(value.m_value, szName);
		}
		return true;
	}

private:

	inline CUniqueId(uint64 value)
		: m_value(value)
	{}

	static inline uint64 Pack(EType type, u32 value)
	{
		return (static_cast<uint64>(type) << 60) | value;
	}

	static inline uint64 Pack(EType type, uint64 value)
	{
		return (static_cast<uint64>(type) << 60) | (value >> 4);
	}

private:

	uint64 m_value;
};
#else
class CUniqueId
{
public:

	enum class EType
	{
		Empty,
		Idx,
		UInt32,
		StringHash,
		GUID
	};

public:

	inline CUniqueId()
		: m_type(EType::Empty)
	{}

	inline CUniqueId(const CUniqueId& rhs)
		: m_type(rhs.m_type)
		, m_value(rhs.m_value)
	{}

	inline bool IsEmpty() const
	{
		return m_type == EType::Empty;
	}

#ifdef SXEMA_PATCH_UNIQUE_IDS
	inline bool IsIdx() const
	{
		return m_type == EType::Idx;
	}

	inline u32 GetIdx() const
	{
		return static_cast<u32>(m_value.hipart);
	}
#endif

	inline void Serialize(Serialization::IArchive& archive)
	{
		archive(m_type, "type");
		switch (m_type)
		{
		case EType::Empty:
			break;
		case EType::Idx:
		case EType::UInt32:
		case EType::StringHash:
			{
				archive(AsUInt32(), "value");
				break;
			}
		case EType::GUID:
			{
				archive(m_value, "value");
				break;
			}
		}
	}

	inline bool operator==(const CUniqueId& rhs) const
	{
		return (m_type == rhs.m_type) && (m_value == rhs.m_value);
	}

	inline bool operator!=(const CUniqueId& rhs) const
	{
		return (m_type != rhs.m_type) || (m_value != rhs.m_value);
	}

	inline bool operator<(const CUniqueId& rhs) const
	{
		return m_type == rhs.m_type ? m_value < rhs.m_value : m_type < rhs.m_type;
	}

	inline bool operator>(const CUniqueId& rhs) const
	{
		return m_type == rhs.m_type ? m_value > rhs.m_value : m_type > rhs.m_type;
	}

	inline bool operator<=(const CUniqueId& rhs) const
	{
		return m_type == rhs.m_type ? m_value <= rhs.m_value : m_type <= rhs.m_type;
	}

	inline bool operator>=(const CUniqueId& rhs) const
	{
		return m_type == rhs.m_type ? m_value >= rhs.m_value : m_type >= rhs.m_type;
	}

	static inline CUniqueId FromIdx(u32 value)
	{
		return CUniqueId(EType::Idx, value);
	}

	static inline CUniqueId FromUInt32(u32 value)
	{
		return CUniqueId(EType::UInt32, value);
	}

	static inline CUniqueId FromString(tukk szValue)
	{
		return CUniqueId(EType::StringHash, DrxStringUtils::CalculateHash(szValue));
	}

	static inline CUniqueId FromGUID(const DrxGUID& value)
	{
		return CUniqueId(EType::GUID, value);
	}

private:

	inline CUniqueId(EType type, u32 value)
		: m_type(type)
	{
		AsUInt32() = value;
	}

	inline CUniqueId(EType type, const DrxGUID& value)
		: m_type(type)
		, m_value(value)
	{}

	inline u32& AsUInt32()
	{
		return reinterpret_cast<u32&>(m_value.hipart);
	}

private:

	EType m_type;
	DrxGUID m_value;
};
#endif

} // sxema
