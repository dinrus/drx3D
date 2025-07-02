// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef SXEMA_USE_STD_TYPE_ID
	#define SXEMA_USE_STD_TYPE_ID 0
#endif

#if SXEMA_USE_STD_TYPE_ID
	#include <typeinfo>
	#include <typeindex>
#else
	#include <drx3D/Schema/StringHashWrapper.h>
	#include <drx3D/Schema/TypeUtils.h>
#endif

#include <drx3D/Schema/PreprocessorUtils.h>
namespace sxema
{
class CTypeName
{
	template<typename TYPE> friend const CTypeName& GetTypeName();

private:

	struct EmptyType {};

#if SXEMA_USE_STD_TYPE_ID
	typedef std::type_index                                                                   Value;
#else
	typedef CStringHashWrapper<CFastStringHash, CEmptyStringConversion, CRawPtrStringStorage> Value;
#endif

public:

	inline CTypeName()
#if SXEMA_USE_STD_TYPE_ID
		: m_value(typeid(EmptyType))
#endif
	{}

	inline CTypeName(const CTypeName& rhs)
		: m_value(rhs.m_value)
	{}

	inline size_t GetHash() const
	{
#if SXEMA_USE_STD_TYPE_ID
		return m_value.hash_code();
#else
		return static_cast<size_t>(m_value.GetHash().GetValue());
#endif
	}

	inline tukk c_str() const
	{
#if SXEMA_USE_STD_TYPE_ID
		return m_value != typeid(EmptyType) ? m_value.name() : "";
#else
		return m_value.c_str();
#endif
	}

	inline bool IsEmpty() const
	{
#if SXEMA_USE_STD_TYPE_ID
		return m_value == typeid(EmptyType);
#else
		return m_value.IsEmpty();
#endif
	}

	inline CTypeName& operator=(const CTypeName& rhs)
	{
		m_value = rhs.m_value;
		return *this;
	}

	inline bool operator==(const CTypeName& rhs) const
	{
		return m_value == rhs.m_value;
	}

	inline bool operator!=(const CTypeName& rhs) const
	{
		return m_value != rhs.m_value;
	}

	inline bool operator<(const CTypeName& rhs) const
	{
		return m_value < rhs.m_value;
	}

	inline bool operator<=(const CTypeName& rhs) const
	{
		return m_value <= rhs.m_value;
	}

	inline bool operator>(const CTypeName& rhs) const
	{
		return m_value > rhs.m_value;
	}

	inline bool operator>=(const CTypeName& rhs) const
	{
		return m_value >= rhs.m_value;
	}

	explicit inline CTypeName(const Value& value)
		: m_value(value)
	{}

private:

	Value m_value;
};

namespace Private
{
template<typename TYPE> struct STypeNameExtractor
{
	inline tukk operator()(string& storage) const
	{
		static tukk szResult = "";
		if (!szResult[0])
		{
			tukk szFunctionName = SXEMA_FUNCTION_NAME;
			tukk szPrefix = "sxema::Private::STypeNameExtractor<";
			tukk szStartOfTypeName = strstr(szFunctionName, szPrefix);
			if (szStartOfTypeName)
			{
				szStartOfTypeName += strlen(szPrefix);
				tukk keyWords[] = { "struct ", "class ", "enum " };
				for (u32 iKeyWord = 0; iKeyWord < DRX_ARRAY_COUNT(keyWords); ++iKeyWord)
				{
					tukk keyWord = keyWords[iKeyWord];
					u32k keyWordLength = strlen(keyWord);
					if (strncmp(szStartOfTypeName, keyWord, keyWordLength) == 0)
					{
						szStartOfTypeName += keyWordLength;
						break;
					}
				}
				static tukk szSffix = ">::operator";
				tukk szEndOfTypeName = strstr(szStartOfTypeName, szSffix);
				if (szEndOfTypeName)
				{
					while (*(szEndOfTypeName - 1) == ' ')
					{
						--szEndOfTypeName;
					}
					storage.assign(szStartOfTypeName, 0, static_cast<string::size_type>(szEndOfTypeName - szStartOfTypeName));
				}
			}
			DRX_ASSERT_MESSAGE(!storage.empty(), "Failed to extract type name!");
		}
		return storage.c_str();
	}
};
} // Private

template<typename TYPE> inline const CTypeName& GetTypeName()
{
	typedef typename std::remove_const<TYPE>::type NonConstType;

#if SXEMA_USE_STD_TYPE_ID
	static const CTypeName typeName(typeid(NonConstType));
#else
	static const string storage = Private::SExtractTypeName<NonConstType>()();
	static const CTypeName typeName(storage.c_str());
#endif
	return typeName;
}
} // sxema

// Facilitate use of type id class as key in std::unordered containers.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
template<> struct hash<sxema::CTypeName>
{
	inline u32 operator()(const sxema::CTypeName& rhs) const
	{
		return static_cast<u32>(rhs.GetHash());
	}
};
} // std
