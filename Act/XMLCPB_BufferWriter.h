// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
*************************************************************************/

#pragma once
#ifndef XMLCPB_BUFFERWRITER_H
	#define XMLCPB_BUFFERWRITER_H

	#include <drx3D/Act/XMLCPB_Common.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>
	#include <drx3D/CoreX/StlUtils.h>

namespace XMLCPB {

class CWriter;

class CBufferWriter
{
public:
	struct SAddr
	{
		u16 bufferOffset;
		u16 bufferIndex;

		SAddr() { Reset(); }

		void Reset()
		{
			bufferOffset = 0xffff;
			bufferIndex = 0xffff;
		}

		bool IsValid() const
		{
			return bufferOffset != 0xffff || bufferIndex != 0xffff;
		}
	};

public:

	CBufferWriter(i32 bufferSize);
	void         Init(CWriter* pWriter, bool useStreaming);

	void         AddData(ukk pSource, i32 sizeToAdd);
	template<class T>
	void         AddDataEndianAware(T val);
	void         AddDataNoSplit(SAddr& addr, ukk pSource, i32 sizeToAdd);
	void         GetCurrentAddr(SAddr& addr) const;
	u32       GetUsedMemory() const;
	FlatAddr     ConvertSegmentedAddrToFlatAddr(const SAddr& addr);
	void         WriteToFile();
	void         WriteToMemory(u8*& rpData, u32& outWriteLoc);
	u32       GetDataSize() const;

	ILINE u8* GetPointerFromAddr(const SAddr& addr) const
	{
		assert(!m_usingStreaming);
		//if (m_usingStreaming)
		//	return NULL;

		assert(addr.bufferIndex < m_buffers.size());
		return m_buffers[addr.bufferIndex]->GetPointer(addr.bufferOffset);
	}

private:

	void AddBuffer();

	struct SSimpleBuffer : public _reference_target_t
	{
	public:
		SSimpleBuffer(i32 size)
			: m_pBuffer(NULL)
			, m_size(size)
			, m_used(0)
		{
			m_pBuffer = new u8[size];
		}

		~SSimpleBuffer()
		{
			FreeBuffer();
		}

		bool   CanAddData(i32 sizeToAdd) const { return m_used + sizeToAdd <= m_size; }
		bool   IsFull() const                  { return m_used == m_size; }
		void   AddData(u8k* pSource, i32 sizeToAdd);
		i32    GetFreeSpaceSize() const        { return m_size - m_used; }
		u32 GetUsed() const                 { return m_used; }
		u8* GetPointer(u32 offset) const { assert(offset < m_used); return GetBasePointer() + offset; }
		void   WriteToFile(CWriter& Writer);
		void   WriteToMemory(CWriter& Writer, u8*& rpData, u32& outWriteLoc);
		u32 GetDataSize() const    { return m_used; }
		u8* GetBasePointer() const { return m_pBuffer; }
		void   FreeBuffer()           { SAFE_DELETE_ARRAY(m_pBuffer); }

	private:
		u8* m_pBuffer;
		u32 m_used;
		u32 m_size;
	};

private:
	typedef _smart_ptr<SSimpleBuffer> SSimpleBufferT;
	std::vector<SSimpleBufferT> m_buffers;
	CWriter*                    m_pWriter;
	i32                         m_bufferSize;
	bool                        m_usingStreaming;
};

template<class T>
inline void CBufferWriter::AddDataEndianAware(T val)
{
	SwapIntegerValue(val);
	AddData(&val, sizeof(val));
}

inline void CBufferWriter::SSimpleBuffer::AddData(u8k* pSource, i32 sizeToAdd)
{
	assert(CanAddData(sizeToAdd));

	memcpy(GetBasePointer() + m_used, pSource, sizeToAdd);

	m_used += sizeToAdd;
}

class AttrStringAllocatorImpl
{
	static i32k k_numPerBucket = 4096;
	static i32k k_maxItems = 1024 * 1024;
	static uint64*   s_buckets[k_maxItems / k_numPerBucket];
	static uint64*   s_ptr;
	static uint64*   s_end;
	static i32       s_currentBucket;
	static i32       s_poolInUse;
public:
	static void LockPool()
	{
	#ifndef _RELEASE
		if (gEnv->mMainThreadId != DrxGetCurrentThreadId())
			__debugbreak();
	#endif
		s_poolInUse++;
	}
	static uk AttrStringAlloc()
	{
	#ifndef _RELEASE
		if (gEnv->mMainThreadId != DrxGetCurrentThreadId())
			__debugbreak();
	#endif
		uint64* ptr = s_ptr;
		if (ptr)
		{
			s_ptr++;
			if (s_ptr == s_end)
			{
				s_ptr = NULL;
			}
		}
		else
		{
	#ifndef _RELEASE
			if (s_currentBucket == DRX_ARRAY_COUNT(s_buckets))
				__debugbreak();
	#endif
			ptr = (uint64*)malloc(k_numPerBucket * sizeof(uint64));
			assert(s_currentBucket >= 0 && s_currentBucket < DRX_ARRAY_COUNT(s_buckets));
			s_buckets[s_currentBucket++] = ptr;
			s_ptr = ptr + 1;
			s_end = ptr + k_numPerBucket;
		}
		return ptr;
	}
	static void CleanPool()
	{
	#ifndef _RELEASE
		if (gEnv->mMainThreadId != DrxGetCurrentThreadId())
			__debugbreak();
	#endif
		s_poolInUse--;
		if (!s_poolInUse)
		{
			for (i32 i = 0; i < s_currentBucket; i++)
			{
				free(s_buckets[i]);
			}
			s_currentBucket = 0;
			s_ptr = NULL;
		}
	}
};

template<class T>
class AttrStringAllocator : public stl::SAllocatorConstruct
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;

	template<class U> struct rebind
	{
		typedef AttrStringAllocator<U> other;
	};

	AttrStringAllocator() throw()
	{
	}

	AttrStringAllocator(const AttrStringAllocator&) throw()
	{
	}

	template<class U> AttrStringAllocator(const AttrStringAllocator<U> ) throw()
	{
	}

	~AttrStringAllocator() throw()
	{
	}

	pointer address(reference x) const
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type n = 1, ukk hint = 0)
	{
		if (sizeof(T) == 8 && n == 1)
		{
			return (T*)AttrStringAllocatorImpl::AttrStringAlloc();
		}
		else
		{
			return (T*)malloc(n * sizeof(T));
		}
	}

	void deallocate(pointer p, size_type n = 1)
	{
		if (sizeof(T) != 8 || n != 1)
		{
			free(p);
		}
	}

	size_type max_size() const throw()
	{
		return INT_MAX;
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	pointer new_pointer()
	{
		return new(allocate())T();
	}

	pointer new_pointer(const T& val)
	{
		return new(allocate())T(val);
	}

	void delete_pointer(pointer p)
	{
		p->~T();
		deallocate(p);
	}

	bool operator==(const AttrStringAllocator&) { return true; }
	bool operator!=(const AttrStringAllocator&) { return false; }
};

}  // end namespace

#endif
