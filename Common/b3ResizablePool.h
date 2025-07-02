#ifndef D3_RESIZABLE_POOL_H
#define D3_RESIZABLE_POOL_H

#include <drx3D/Common/b3AlignedObjectArray.h>

enum
{
	D3_POOL_HANDLE_TERMINAL_FREE = -1,
	D3_POOL_HANDLE_TERMINAL_USED = -2
};

template <typename U>
struct b3PoolBodyHandle : public U
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_nextFreeHandle;
	void setNextFree(i32 next)
	{
		m_nextFreeHandle = next;
	}
	i32 getNextFree() const
	{
		return m_nextFreeHandle;
	}
};

template <typename T>
class b3ResizablePool
{
protected:
	b3AlignedObjectArray<T> m_bodyHandles;
	i32 m_numUsedHandles;   // number of active handles
	i32 m_firstFreeHandle;  // free handles list

	T* getHandleInternal(i32 handle)
	{
		return &m_bodyHandles[handle];
	}
	const T* getHandleInternal(i32 handle) const
	{
		return &m_bodyHandles[handle];
	}

public:
	b3ResizablePool()
	{
		initHandles();
	}

	virtual ~b3ResizablePool()
	{
		exitHandles();
	}
	///handle management

	i32 getNumHandles() const
	{
		return m_bodyHandles.size();
	}

	void getUsedHandles(b3AlignedObjectArray<i32>& usedHandles) const
	{
		for (i32 i = 0; i < m_bodyHandles.size(); i++)
		{
			if (m_bodyHandles[i].getNextFree() == D3_POOL_HANDLE_TERMINAL_USED)
			{
				usedHandles.push_back(i);
			}
		}
	}

	T* getHandle(i32 handle)
	{
		drx3DAssert(handle >= 0);
		drx3DAssert(handle < m_bodyHandles.size());
		if ((handle < 0) || (handle >= m_bodyHandles.size()))
		{
			return 0;
		}

		if (m_bodyHandles[handle].getNextFree() == D3_POOL_HANDLE_TERMINAL_USED)
		{
			return &m_bodyHandles[handle];
		}
		return 0;
	}
	const T* getHandle(i32 handle) const
	{
		drx3DAssert(handle >= 0);
		drx3DAssert(handle < m_bodyHandles.size());
		if ((handle < 0) || (handle >= m_bodyHandles.size()))
		{
			return 0;
		}

		if (m_bodyHandles[handle].getNextFree() == D3_POOL_HANDLE_TERMINAL_USED)
		{
			return &m_bodyHandles[handle];
		}
		return 0;
	}

	void increaseHandleCapacity(i32 extraCapacity)
	{
		i32 curCapacity = m_bodyHandles.size();
		//drx3DAssert(curCapacity == m_numUsedHandles);
		i32 newCapacity = curCapacity + extraCapacity;
		m_bodyHandles.resize(newCapacity);

		{
			for (i32 i = curCapacity; i < newCapacity; i++)
				m_bodyHandles[i].setNextFree(i + 1);

			m_bodyHandles[newCapacity - 1].setNextFree(-1);
		}
		m_firstFreeHandle = curCapacity;
	}
	void initHandles()
	{
		m_numUsedHandles = 0;
		m_firstFreeHandle = -1;

		increaseHandleCapacity(1);
	}

	void exitHandles()
	{
		m_bodyHandles.resize(0);
		m_firstFreeHandle = -1;
		m_numUsedHandles = 0;
	}

	i32 allocHandle()
	{
		drx3DAssert(m_firstFreeHandle >= 0);

		i32 handle = m_firstFreeHandle;
		m_firstFreeHandle = getHandleInternal(handle)->getNextFree();
		m_numUsedHandles++;

		if (m_firstFreeHandle < 0)
		{
			//i32 curCapacity = m_bodyHandles.size();
			i32 additionalCapacity = m_bodyHandles.size();
			increaseHandleCapacity(additionalCapacity);

			getHandleInternal(handle)->setNextFree(m_firstFreeHandle);
		}
		getHandleInternal(handle)->setNextFree(D3_POOL_HANDLE_TERMINAL_USED);
		getHandleInternal(handle)->clear();
		return handle;
	}

	void freeHandle(i32 handle)
	{
		drx3DAssert(handle >= 0);

		if (m_bodyHandles[handle].getNextFree() == D3_POOL_HANDLE_TERMINAL_USED)
		{
			getHandleInternal(handle)->clear();
			getHandleInternal(handle)->setNextFree(m_firstFreeHandle);
			m_firstFreeHandle = handle;
			m_numUsedHandles--;
		}
	}
};
///end handle management

#endif  //D3_RESIZABLE_POOL_H
