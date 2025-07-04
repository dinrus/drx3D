// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:DrxPakHandleCache.h
//
//	История:
//	-Aug 3,2009:Created by Michael Glueck
//
//  acts as cache for file handles to prevent system operations for
//    opening/closing pak-files for direct op-mode
//  assume calls into this class are multithread protected by DrxPak.cpp
//
//////////////////////////////////////////////////////////////////////

#ifndef DRXPAKHANDLECACHE_H
#define DRXPAKHANDLECACHE_H

#include <drx3D/Sys/IDrxPak.h>
#include "md5.h"

class CPakHandleCache
{
public:
	CPakHandleCache() : m_Size(0), m_LRU(0), m_pBuffer(NULL){}
	CPakHandleCache(u32 size) : m_Size(size), m_LRU(0)
	{
		m_pBuffer = new SCacheEntry[size];
	}
	void ReCreate(u32 size)
	{
		//re-creates by closing all cached handles
		if (m_Size)
		{
			for (u32 i = 0; i < m_Size; ++i)
			{
				SCacheEntry& rEntry = m_pBuffer[i];
				if (rEntry.pHandle)
				{
					fclose(rEntry.pHandle);
					rEntry.pHandle = NULL;
				}
			}
			if (size != m_Size)
				delete[] m_pBuffer;
		}
		m_Size = size;
		if (size)
			m_pBuffer = new SCacheEntry[size];
		m_LRU = 0;
	}
	void Close(FILE* pHandle, const string& name)
	{
		if (!m_Size)
		{
			fclose(pHandle);
			return;
		}
		//add to some open slot or replace the one with the lowest lru
		u32 curLowestSlot = 0;
		u32 curLowestSlotLRU = 0xFFFFFFFF;
		for (u32 i = 0; i < m_Size; ++i)
		{
			SCacheEntry& rEntry = m_pBuffer[i];
			if (rEntry.pHandle == NULL)
			{
				ApplySlot(rEntry, pHandle, name);
				return;
			}
			if (rEntry.lru < curLowestSlotLRU)
			{
				curLowestSlotLRU = rEntry.lru;
				curLowestSlot = i;
			}
		}
		//reuse a slot
		SCacheEntry& rReUseEntry = m_pBuffer[curLowestSlot];
		fclose(rReUseEntry.pHandle);
		ApplySlot(rReUseEntry, pHandle, name);
	}
	FILE* Open(const string& name)
	{
		if (m_Size)
		{
			//retrieve cache one if existing, otherwise open new one
			TPakNameKey key;
			MD5Context context;
			MD5Init(&context);
			MD5Update(&context, (u8*)name.c_str(), name.size());
			MD5Final(key.c16, &context);
			for (u32 i = 0; i < m_Size; ++i)
			{
				SCacheEntry& rEntry = m_pBuffer[i];
				if (rEntry.pHandle && rEntry.pakNameKey.u64[0] == key.u64[0] && rEntry.pakNameKey.u64[1] == key.u64[1])
				{
					FILE* ret = rEntry.pHandle;
					rEntry.pHandle = NULL;
					return ret;
				}
			}
		}
		return fopen(name.c_str(), "rb");
	}
	u32 Size() const { return m_Size; }
	~CPakHandleCache()
	{
		ReCreate(0);
	}

private:
	typedef union
	{
		u8 c16[16];
		uint64        u64[2];
	} TPakNameKey;

	struct SCacheEntry
	{
		FILE*       pHandle;        //cached pak file handle
		u32      lru;            //lru counter last set op
		TPakNameKey pakNameKey;     //md5 of pake file name (full path)
		SCacheEntry() : pHandle(NULL), lru(0){ pakNameKey.u64[0] = pakNameKey.u64[1] = 0; }
	};
	u32       m_Size;          //size of file handle buffer
	SCacheEntry* m_pBuffer;       //file handle buffer
	u32       m_LRU;           //current lru counter to replace slots properly

	inline void ApplySlot(SCacheEntry& rSlot, FILE* pHandle, const string& name)
	{
		rSlot.pHandle = pHandle;
		rSlot.lru = ++m_LRU;
		MD5Context context;
		MD5Init(&context);
		MD5Update(&context, (u8*)name.c_str(), name.size());
		MD5Final(rSlot.pakNameKey.c16, &context);
	}
};

#endif
