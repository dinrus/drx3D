// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__

#pragma once

#include <drx3D/CoreX/Containers/MiniQueue.h>
#include <drx3D/CoreX/Typelist.h>
#include <drx3D/Network/NetProfile.h>

// abstracts away offset selection for byte buffer manipulation; used for both reading and writing
// tries to pack data into a byte buffer smartly:
//   4 byte or bigger things are aligned on 4 byte boundaries
//   2 byte things are aligned on 2 byte boundaries
//   1 byte things are aligned on 1 byte boundaries
// selection of where to place something is done in a branch free manner for fixed sized things
// TODO: determine if our 64-bit targets require 64-bit alignment for 64-bit values, and expand this class as needed
class CByteStreamPacker
{
public:
	CByteStreamPacker()
	{
		for (i32 i = 0; i < 3; i++)
			m_data[i] = 0;
	}

	u32 GetNextOfs(u32 sz);

	template<i32 N>
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<N> )
	{
		return GetNextOfs(N);
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<0> )
	{
		return m_data[eD_Size];
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<1> )
	{
		// SEL is a lookup table (packed into bits)
		static u32k SEL = 0x0e; // binary 1110
		// selector will be 0 or 1
		//   if there was a valid size 1 location free, it's 1
		//   otherwise it's 0
		// a valid size 1 location exists if Ofs1 is not a multiple of four
		u32 selector = (SEL >> (m_data[eD_Ofs1] & 3)) & 1;

		// use this value to lookup the offset we need to use
		// (produces a useless copy of selector == 1, but eliminates a branch)
		u32 out = m_data[eD_Ofs1] = m_data[selector];
		m_data[eD_Ofs1]++;
		m_data[eD_Size] += 4 - 4 * selector;

		return out;
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<2> )
	{
		// SEL is a lookup table (packed into bits)
		static u32k SEL = 0x1c; // binary 1110<<1
		// selector will be 0 or 2
		//   if there was a valid size 2 location free, it's 2
		//   otherwise it's 0
		// a valid size 2 location exists if Ofs2 is not a multiple of four
		u32 selector = (SEL >> (m_data[eD_Ofs2] & 3)) & 2;

		// use this value to lookup the offset we need to use
		// (produces a useless copy of selector == 2, but eliminates a branch)
		u32 out = m_data[eD_Ofs2] = m_data[selector];
		m_data[eD_Ofs2] += 2;
		m_data[eD_Size] += 4 - 2 * selector;

		return out;
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<4> )
	{
		u32 ofs = m_data[eD_Size];
		m_data[eD_Size] += 4;
		return ofs;
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<8> )
	{
		u32 ofs = m_data[eD_Size];
		m_data[eD_Size] += 8;
		return ofs;
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<12> )
	{
		u32 ofs = m_data[eD_Size];
		m_data[eD_Size] += 12;
		return ofs;
	}
	ILINE u32 GetNextOfs_Fixed(NTypelist::Int2Type<16> )
	{
		u32 ofs = m_data[eD_Size];
		m_data[eD_Size] += 16;
		return ofs;
	}

protected:
	enum EData
	{
		eD_Size = 0, // size of our output
		eD_Ofs1,     // position of the next free size 1 element
		eD_Ofs2,     // position of the next free size 2 element
	};
	u32 m_data[3];
};

class CByteOutputStream : private CByteStreamPacker
{
public:
	CByteOutputStream(IStreamAllocator* pSA, size_t initSize = 16, uk caller = 0) : m_pSA(pSA), m_capacity(initSize), m_buffer((u8*)pSA->Alloc(initSize, caller ? caller : UP_STACK_PTR))
	{
		memset(m_buffer, 0, initSize);
	}

	// the horrible slow way to write variable sized things
	void Put(ukk pWhat, size_t sz)
	{
		u32 where = GetNextOfs(sz);
		if (m_data[eD_Size] > m_capacity)
			Grow(m_data[eD_Size]);
		memcpy(m_buffer + where, pWhat, sz);
#if NET_PROFILE_ENABLE
		m_bytes += sz;
#endif
	}

	// the nice fast way to write known size things
	template<class T>
	ILINE T& PutTyped()
	{
		u32 where = GetNextOfs_Fixed(NTypelist::Int2Type<sizeof(T)>());
		if (m_data[eD_Size] > m_capacity)
			Grow(m_data[eD_Size]);
#if NET_PROFILE_ENABLE
		m_bytes += sizeof(T);
#endif
		return *reinterpret_cast<T*>(m_buffer + where);
	}

	ILINE void PutByte(u8 c)
	{
		PutTyped<u8>() = c;
	}

	ILINE size_t GetSize()
	{
		return m_data[eD_Size];
	}

	u8k* GetBuffer() const
	{
		return m_buffer;
	}

#if NET_PROFILE_ENABLE
	void ConditionalPrelude()  { m_bytes = 0; }
	void ConditionalPostlude() { NET_PROFILE_ADD_WRITE_BITS(m_bytes * 8); }
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CByteOutputStream");

		pSizer->Add(*this);
		if (m_pSA)
			pSizer->Add(m_buffer, m_capacity);
	}

private:
	void Grow(size_t sz);

	IStreamAllocator* m_pSA;
	u32            m_capacity;
	u8*            m_buffer;
#if NET_PROFILE_ENABLE
	u32            m_bytes;
#endif
};

class CByteInputStream : private CByteStreamPacker
{
public:
	CByteInputStream(u8k* pData, size_t sz) : m_pData(pData), m_capacity(sz)
	{
	}

	~CByteInputStream()
	{
	}

	ILINE void Get(uk pWhere, size_t sz)
	{
		NET_ASSERT(sz <= m_data[eD_Size]);
		memcpy(pWhere, Get(sz), sz);
#if NET_PROFILE_ENABLE
		m_bytes += sz;
#endif
	}

	ILINE ukk Get(size_t sz)
	{
		u32 where = GetNextOfs(sz);
		NET_ASSERT((where + sz) <= m_capacity);
#if NET_PROFILE_ENABLE
		m_bytes += sz;
#endif
		return m_pData + where;
	}

	template<class T>
	ILINE const T& GetTyped()
	{
		u32 where = GetNextOfs_Fixed(NTypelist::Int2Type<sizeof(T)>());
		NET_ASSERT((where + sizeof(T)) <= m_capacity);
#if NET_PROFILE_ENABLE
		m_bytes += sizeof(T);
#endif
		return *static_cast<const T*>(static_cast<ukk>(m_pData + where));
	}

	ILINE u8 GetByte()
	{
		return GetTyped<u8>();
	}

#if NET_PROFILE_ENABLE
	void ConditionalPrelude()  { m_bytes = 0; }
	void ConditionalPostlude() { NET_PROFILE_ADD_READ_BITS(m_bytes * 8); }
#endif

	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CByteInputStream");

		pSizer->Add(*this);
		pSizer->Add(m_pData, m_capacity);
	}

	size_t GetSize() const
	{
		return m_data[eD_Size];
	}

private:
	u8k* m_pData;
	size_t       m_capacity;
#if NET_PROFILE_ENABLE
	u32       m_bytes;
#endif
};

#endif
