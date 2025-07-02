// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <algorithm>
#include <utility>
#include <vector>

template<typename T>
class CXMLAttrReader
{
	typedef std::vector<std::pair<tukk , T>> TRecordList;
	TRecordList m_dictionary;

public:
	void Reserve(const size_t elementCount)
	{
		m_dictionary.reserve(elementCount);
	}

	void Add(tukk szKey, const T value)
	{
		m_dictionary.push_back(std::make_pair(szKey, value));
	}

	bool Get(const XmlNodeRef& node, tukk szAttrName, T& value, bool bMandatory = false) const
	{
		tukk szKey = node->getAttr(szAttrName);
		const auto it = std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [szKey](const typename TRecordList::value_type& it) { return stricmp(it.first, szKey) == 0; });
		if (it != m_dictionary.cend())
		{
			value = it->second;
			return true;
		}

		if (bMandatory)
		{
			DrxWarning(VALIDATOR_MODULE_UNKNOWN, VALIDATOR_WARNING, "Unable to get mandatory attribute '%s' of node '%s' at line %d.",
			           szAttrName, node->getTag(), node->getLine());
		}

		return false;
	}

	tukk GetFirstRecordName(const T value) const
	{
		const auto it = std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [&value](const typename TRecordList::value_type& it) { return it.second == value; });
		return it != m_dictionary.cend() ? it->first : nullptr;
	}

	const T* GetFirstValue(tukk szName) const
	{
		const auto it = std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [szName](const typename TRecordList::value_type& it) { return stricmp(it.first, szName) == 0; });
		return it != m_dictionary.cend() ? &it->second : nullptr;
	}
};
