// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

// Class with CRC sum. Used for optimizations  s

// #include "VectorMap.h"
#include "DrxName.h"

#define STORE_CRCNAME_STRINGS (1)

#if !defined(USE_STATIC_NAME_TABLE)
	#define USE_STATIC_NAME_TABLE
#endif

namespace NameCRCHelper
{

ILINE u32 GetCRC(tukk name)
{
#ifdef _USE_LOWERCASE
	return CCrc32::ComputeLowercase(name);
#else
	return CCrc32::Compute(name);
#endif
}

}

struct CNameCRCHelper
{
public:
	CNameCRCHelper() : m_CRC32Name(~0) {};

	u32 GetCRC32() const { return m_CRC32Name; };

	void   GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_Name);
	}
protected:
	string      m_Name;     //!< The name of the animation (not the name of the file) - unique per-model.
	// CDrxName m_Name;
	u32      m_CRC32Name; //!< Hash value for searching animations.

	tukk GetName() const { return m_Name.c_str(); };
	//	const string& GetNameString() const { return m_Name; };

	// Set name and compute CRC value for it.
	void SetName(const string& name)
	{
		m_Name = name;

		m_CRC32Name = NameCRCHelper::GetCRC(name.c_str());
	}

	void SetNameChar(tukk name)
	{
		m_Name = name;

		m_CRC32Name = NameCRCHelper::GetCRC(name);
	}

};

//! Custom hash map class.
struct CNameCRCMap
{

	// Fast method.
	typedef std::map<u32, size_t> NameHashMap;
	//typedef VectorMap<u32, size_t> NameHashMap;

	//! \return The index of the animation from crc value.
	size_t GetValueCRC(u32 crc) const
	{
		NameHashMap::const_iterator it = m_HashMap.find(crc);

		if (it == m_HashMap.end())
			return -1;

		return it->second;
	}

	//! \return The index of the animation from name. Name converted in lower case in this function.
	size_t GetValue(tukk name) const
	{
		return GetValueCRC(NameCRCHelper::GetCRC(name));
	}

	//! \return The index of the animation from name. Name should be in lower case!
	size_t GetValueLower(tukk name) const
	{
		return GetValueCRC(CCrc32::Compute(name));
	}

	// In
	bool InsertValue(CNameCRCHelper* header, size_t num)
	{
		bool res = m_HashMap.find(header->GetCRC32()) == m_HashMap.end();
		//if (m_HashMap.find(header->GetCRC32()) != m_HashMap.end())
		//{
		//	AnimWarning("[Animation] %s exist in the model!", header->GetName());
		//}
		m_HashMap[header->GetCRC32()] = num;

		return res;

	}

	bool InsertValue(u32 crc, size_t num)
	{
		bool res = m_HashMap.find(crc) == m_HashMap.end();
		m_HashMap[crc] = num;

		return res;
	}

	size_t GetAllocMemSize() const
	{
		return m_HashMap.size() * (sizeof(u32) + sizeof(size_t));
	}

	size_t GetMapSize() const
	{
		return m_HashMap.size();
	}

	void GetMemoryUsage(class IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_HashMap);
	}
protected:
	NameHashMap m_HashMap;
};

struct SCRCName
{
	u32 m_CRC32;
#if STORE_CRCNAME_STRINGS
	string m_name;
#endif //STORE_CRCNAME_STRINGS

	ILINE tukk GetName_DEBUG() const
	{
#if STORE_CRCNAME_STRINGS
		return m_name.c_str();
#else //STORE_CRCNAME_STRINGS
		return "NameStripped";
#endif //STORE_CRCNAME_STRINGS

	}
	ILINE void SetName(tukk name)
	{
		m_CRC32 = CCrc32::ComputeLowercase(name);
#if STORE_CRCNAME_STRINGS
		m_name = name;
#endif //STORE_CRCNAME_STRINGS
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
	}
};

//! \endcond