#ifndef _DRX3D_POOL_ALLOCATOR_H
#define _DRX3D_POOL_ALLOCATOR_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Maths/Linear/Threads.h>

///The PoolAllocator class allows to efficiently allocate a large pool of objects, instead of dynamically allocating them separately.
class PoolAllocator
{
	i32 m_elemSize;
	i32 m_maxElements;
	i32 m_freeCount;
	uk m_firstFree;
	u8* m_pool;
	SpinMutex m_mutex;  // only used if DRX3D_THREADSAFE

public:
	PoolAllocator(i32 elemSize, i32 maxElements)
		: m_elemSize(elemSize),
		  m_maxElements(maxElements)
	{
		m_pool = (u8*)AlignedAlloc(static_cast<u32>(m_elemSize * m_maxElements), 16);

		u8* p = m_pool;
		m_firstFree = p;
		m_freeCount = m_maxElements;
		i32 count = m_maxElements;
		while (--count)
		{
			*(uk *)p = (p + m_elemSize);
			p += m_elemSize;
		}
		*(uk *)p = 0;
	}

	~PoolAllocator()
	{
		AlignedFree(m_pool);
	}

	i32 getFreeCount() const
	{
		return m_freeCount;
	}

	i32 getUsedCount() const
	{
		return m_maxElements - m_freeCount;
	}

	i32 getMaxCount() const
	{
		return m_maxElements;
	}

	uk allocate(i32 size)
	{
		// release mode fix
		(void)size;
		MutexLock(&m_mutex);
		Assert(!size || size <= m_elemSize);
		//Assert(m_freeCount>0);  // should return null if all full
		uk result = m_firstFree;
		if (NULL != m_firstFree)
		{
			m_firstFree = *(uk *)m_firstFree;
			--m_freeCount;
		}
		MutexUnlock(&m_mutex);
		return result;
	}

	bool validPtr(uk ptr)
	{
		if (ptr)
		{
			if (((u8*)ptr >= m_pool && (u8*)ptr < m_pool + m_maxElements * m_elemSize))
			{
				return true;
			}
		}
		return false;
	}

	void freeMemory(uk ptr)
	{
		if (ptr)
		{
			Assert((u8*)ptr >= m_pool && (u8*)ptr < m_pool + m_maxElements * m_elemSize);

			MutexLock(&m_mutex);
			*(uk *)ptr = m_firstFree;
			m_firstFree = ptr;
			++m_freeCount;
			MutexUnlock(&m_mutex);
		}
	}

	i32 getElementSize() const
	{
		return m_elemSize;
	}

	u8* getPoolAddress()
	{
		return m_pool;
	}

	u8k* getPoolAddress() const
	{
		return m_pool;
	}
};

#endif  //_DRX3D_POOL_ALLOCATOR_H
