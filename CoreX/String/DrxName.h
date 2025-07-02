// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/CoreX/Memory/STLGlobalAllocator.h>

class CNameTable;

struct INameTable
{
	virtual ~INameTable(){}

	//! Name entry header, immediately after this header in memory starts actual string data.
	struct SNameEntry
	{
		//! Reference count of this string.
		i32 nRefCount;

		//! Current length of string.
		i32 nLength;

		//! Size of memory allocated at the end of this class.
		i32 nAllocSize;
		// Here in memory starts character buffer of size nAllocSize.
		//char data[nAllocSize].

		tukk GetStr()         { return (tuk)(this + 1); }
		void        AddRef()         { nRefCount++; /*InterlockedIncrement(&_header()->nRefCount);*/ };
		i32         Release()        { return --nRefCount; };
		i32         GetMemoryUsage() { return sizeof(SNameEntry) + strlen(GetStr()); }
		i32         GetLength()      { return nLength; }
	};

	//! Finds an existing name table entry, or creates a new one if not found.
	virtual INameTable::SNameEntry* GetEntry(tukk str) = 0;

	//! Only finds an existing name table entry, return 0 if not found.
	virtual INameTable::SNameEntry* FindEntry(tukk str) = 0;

	//! Release existing name table entry.
	virtual void Release(SNameEntry* pEntry) = 0;
	virtual i32  GetMemoryUsage() = 0;
	virtual i32  GetNumberOfEntries() = 0;

	//! Output all names from the table to log.
	virtual void LogNames() = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

//////////////////////////////////////////////////////////////////////////
class CNameTable : public INameTable
{
private:
	typedef std::unordered_map<tukk , SNameEntry*, stl::hash_stricmp<tukk >, stl::hash_stricmp<tukk >, stl::STLGlobalAllocator<std::pair<tukk const, SNameEntry*>>> NameMap;
	NameMap m_nameMap;

public:
	CNameTable() {}

	~CNameTable()
	{
		for (NameMap::iterator it = m_nameMap.begin(); it != m_nameMap.end(); ++it)
		{
			free(it->second);
		}
	}

	//! Only finds an existing name table entry
	//! \return 0 if not found.
	virtual INameTable::SNameEntry* FindEntry(tukk str)
	{
		SNameEntry* pEntry = stl::find_in_map(m_nameMap, str, 0);
		return pEntry;
	}

	//! Finds an existing name table entry, or creates a new one if not found.
	virtual INameTable::SNameEntry* GetEntry(tukk str)
	{
		SNameEntry* pEntry = stl::find_in_map(m_nameMap, str, 0);
		if (!pEntry)
		{
			//! Create a new entry.
			u32 nLen = strlen(str);
			u32 allocLen = sizeof(SNameEntry) + (nLen + 1) * sizeof(char);
			pEntry = (SNameEntry*)malloc(allocLen);
			assert(pEntry != NULL);
			pEntry->nRefCount = 0;
			pEntry->nLength = nLen;
			pEntry->nAllocSize = allocLen;

			//! Copy string to the end of name entry.
			tuk pEntryStr = const_cast<tuk>(pEntry->GetStr());
			memcpy(pEntryStr, str, nLen + 1);
			// put in map.
			//m_nameMap.insert( NameMap::value_type(pEntry->GetStr(),pEntry) );
			m_nameMap[pEntry->GetStr()] = pEntry;
		}
		return pEntry;
	}

	//! Release existing name table entry.
	virtual void Release(SNameEntry* pEntry)
	{
		assert(pEntry);
		m_nameMap.erase(pEntry->GetStr());
		free(pEntry);
	}
	virtual i32 GetMemoryUsage()
	{
		i32 nSize = 0;
		NameMap::iterator it;
		i32 n = 0;
		for (it = m_nameMap.begin(); it != m_nameMap.end(); it++)
		{
			nSize += strlen(it->first);
			nSize += it->second->GetMemoryUsage();
			n++;
		}
		nSize += n * 8;

		return nSize;
	}
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_nameMap);
	}
	virtual i32 GetNumberOfEntries()
	{
		return m_nameMap.size();
	}

	//! Log all names inside DrxName table.
	virtual void LogNames()
	{
		NameMap::iterator it;
		for (it = m_nameMap.begin(); it != m_nameMap.end(); ++it)
		{
			SNameEntry* pNameEntry = it->second;
			DrxLog("[%4d] %s", pNameEntry->nLength, pNameEntry->GetStr());
		}
	}

};

class CDrxName
{
public:
	CDrxName();
	CDrxName(const CDrxName& n);
	explicit CDrxName(tukk s);
	CDrxName(tukk s, bool bOnlyFind);
	~CDrxName();

	CDrxName&   operator=(const CDrxName& n);
	CDrxName&   operator=(tukk s);

	bool        operator==(const CDrxName& n) const;
	bool        operator!=(const CDrxName& n) const;

	bool        operator==(tukk s) const;
	bool        operator!=(tukk s) const;

	bool        operator<(const CDrxName& n) const;
	bool        operator>(const CDrxName& n) const;

	bool        empty() const { return !m_str || !m_str[0]; }
	void        reset()       { _release(m_str);  m_str = 0; }
	void        addref()      { _addref(m_str); }

	tukk c_str() const
	{
		return (m_str) ? m_str : "";
	}
	i32         length() const        { return _length(); };

	static bool find(tukk str) { return GetNameTable()->FindEntry(str) != 0; }
	void        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//pSizer->AddObject(m_str);
		pSizer->AddObject(GetNameTable());   // cause for slowness?
	}
	static i32 GetMemoryUsage()
	{
#ifdef USE_STATIC_NAME_TABLE
		CNameTable* pTable = GetNameTable();
#else
		INameTable* pTable = GetNameTable();
#endif
		return pTable->GetMemoryUsage();
	}
	static i32 GetNumberOfEntries()
	{
#ifdef USE_STATIC_NAME_TABLE
		CNameTable* pTable = GetNameTable();
#else
		INameTable* pTable = GetNameTable();
#endif
		return pTable->GetNumberOfEntries();
	}

	//! Compare functor for sorting CDrxNames lexically.
	struct CmpLex
	{
		bool operator()(const CDrxName& n1, const CDrxName& n2) const
		{
			return strcmp(n1.c_str(), n2.c_str()) < 0;
		}
	};

private:
	typedef INameTable::SNameEntry SNameEntry;

#ifdef USE_STATIC_NAME_TABLE
	static CNameTable* GetNameTable()
	{
		//! \note We cannot use a 'static CNameTable sTable' here, because that
		//! implies a static destruction order depenency - the name table is
		//! accessed from static destructor calls.
		static CNameTable* table = NULL;

		if (table == NULL)
			table = new CNameTable();
		return table;
	}
#else
	//static INameTable* GetNameTable() { return GetISystem()->GetINameTable(); }
	static INameTable* GetNameTable()
	{
		assert(gEnv && gEnv->pNameTable);
		return gEnv->pNameTable;
	}
#endif

	SNameEntry* _entry(tukk pBuffer) const { assert(pBuffer); return ((SNameEntry*)pBuffer) - 1; }
	void        _release(tukk pBuffer)
	{
		if (pBuffer && _entry(pBuffer)->Release() <= 0 && gEnv)
			GetNameTable()->Release(_entry(pBuffer));
	}
	i32  _length() const              { return (m_str) ? _entry(m_str)->nLength : 0; };
	void _addref(tukk pBuffer) { if (pBuffer) _entry(pBuffer)->AddRef(); }

	tukk m_str;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// DrxName
//////////////////////////////////////////////////////////////////////////
inline CDrxName::CDrxName()
{
	m_str = 0;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxName::CDrxName(const CDrxName& n)
{
	_addref(n.m_str);
	m_str = n.m_str;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxName::CDrxName(tukk s)
{
	m_str = 0;
	*this = s;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxName::CDrxName(tukk s, bool bOnlyFind)
{
	assert(s);
	m_str = 0;
	if (*s) // if not empty
	{
		SNameEntry* pNameEntry = GetNameTable()->FindEntry(s);
		if (pNameEntry)
		{
			m_str = pNameEntry->GetStr();
			_addref(m_str);
		}
	}
}

inline CDrxName::~CDrxName()
{
	_release(m_str);
}

//////////////////////////////////////////////////////////////////////////
inline CDrxName& CDrxName::operator=(const CDrxName& n)
{
	if (m_str != n.m_str)
	{
		_release(m_str);
		m_str = n.m_str;
		_addref(m_str);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxName& CDrxName::operator=(tukk s)
{
	assert(s);
	tukk pBuf = 0;
	if (s && *s) // if not empty
	{
		pBuf = GetNameTable()->GetEntry(s)->GetStr();
	}
	if (m_str != pBuf)
	{
		_release(m_str);
		m_str = pBuf;
		_addref(m_str);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
inline bool CDrxName::operator==(const CDrxName& n) const
{
	return m_str == n.m_str;
}

inline bool CDrxName::operator!=(const CDrxName& n) const
{
	return !(*this == n);
}

inline bool CDrxName::operator==(tukk str) const
{
	return m_str && stricmp(m_str, str) == 0;
}

inline bool CDrxName::operator!=(tukk str) const
{
	if (!m_str)
		return true;
	return stricmp(m_str, str) != 0;
}

inline bool CDrxName::operator<(const CDrxName& n) const
{
	return m_str < n.m_str;
}

inline bool CDrxName::operator>(const CDrxName& n) const
{
	return m_str > n.m_str;
}

inline bool operator==(const string& s, const CDrxName& n)
{
	return n == s;
}
inline bool operator!=(const string& s, const CDrxName& n)
{
	return n != s;
}

inline bool operator==(tukk s, const CDrxName& n)
{
	return n == s;
}
inline bool operator!=(tukk s, const CDrxName& n)
{
	return n != s;
}

class CDrxNameCRC
{
public:
	CDrxNameCRC();
	CDrxNameCRC(const CDrxNameCRC& n);
	explicit CDrxNameCRC(tukk s);
	CDrxNameCRC(tukk s, bool bOnlyFind);
	explicit CDrxNameCRC(u32 n) { m_nID = n; }
	~CDrxNameCRC();

	CDrxNameCRC& operator=(const CDrxNameCRC& n);
	CDrxNameCRC& operator=(tukk s);

	bool         operator==(const CDrxNameCRC& n) const;
	bool         operator!=(const CDrxNameCRC& n) const;

	bool         operator==(tukk s) const;
	bool         operator!=(tukk s) const;

	bool         operator<(const CDrxNameCRC& n) const;
	bool         operator>(const CDrxNameCRC& n) const;

	bool         empty() const { return m_nID == 0; }
	void         reset()       { m_nID = 0; }
	u32       get() const   { return m_nID; }
	void         add(i32 nAdd) { m_nID += nAdd; }

	AUTO_STRUCT_INFO;

	void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
private:

	u32 m_nID;

};

inline CDrxNameCRC::CDrxNameCRC()
{
	m_nID = 0;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxNameCRC::CDrxNameCRC(const CDrxNameCRC& n)
{
	m_nID = n.m_nID;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxNameCRC::CDrxNameCRC(tukk s)
{
	m_nID = 0;
	*this = s;
}

inline CDrxNameCRC::~CDrxNameCRC()
{
	m_nID = 0;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxNameCRC& CDrxNameCRC::operator=(const CDrxNameCRC& n)
{
	m_nID = n.m_nID;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
inline CDrxNameCRC& CDrxNameCRC::operator=(tukk s)
{
	assert(s);
	if (*s) // if not empty
	{
		m_nID = CCrc32::ComputeLowercase(s);
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////////
inline bool CDrxNameCRC::operator==(const CDrxNameCRC& n) const
{
	return m_nID == n.m_nID;
}

inline bool CDrxNameCRC::operator!=(const CDrxNameCRC& n) const
{
	return !(*this == n);
}

inline bool CDrxNameCRC::operator==(tukk str) const
{
	assert(str);
	if (*str) // if not empty
	{
		u32 nID = CCrc32::ComputeLowercase(str);
		return m_nID == nID;
	}
	return m_nID == 0;
}

inline bool CDrxNameCRC::operator!=(tukk str) const
{
	if (!m_nID)
		return true;
	if (*str) // if not empty
	{
		u32 nID = CCrc32::ComputeLowercase(str);
		return m_nID != nID;
	}
	return false;
}

inline bool CDrxNameCRC::operator<(const CDrxNameCRC& n) const
{
	return m_nID < n.m_nID;
}

inline bool CDrxNameCRC::operator>(const CDrxNameCRC& n) const
{
	return m_nID > n.m_nID;
}

inline bool operator==(const string& s, const CDrxNameCRC& n)
{
	return n == s;
}
inline bool operator!=(const string& s, const CDrxNameCRC& n)
{
	return n != s;
}

inline bool operator==(tukk s, const CDrxNameCRC& n)
{
	return n == s;
}
inline bool operator!=(tukk s, const CDrxNameCRC& n)
{
	return n != s;
}

//! \endcond