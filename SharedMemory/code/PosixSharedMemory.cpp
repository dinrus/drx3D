#include "../PosixSharedMemory.h"
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Maths/Linear/Scalar.h>  //for Assert
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

//Windows implementation is in Win32SharedMemory.cpp
#ifndef _WIN32
#define TEST_SHARED_MEMORY
#endif  //_WIN32

//Shmem not available on target api < 26
#if defined(__ANDROID_API__) && (__ANDROID_API__ < 26)
#undef TEST_SHARED_MEMORY
#endif  //__ANDROID__

#include <stddef.h>

#ifdef TEST_SHARED_MEMORY

#include <sys/shm.h>
#include <sys/ipc.h>

#endif

struct SharedMemorySegment
{
	i32 m_key;
	i32 m_sharedMemoryId;
	uk m_sharedMemoryPtr;
	bool m_createdSharedMemory;

	SharedMemorySegment()
		: m_sharedMemoryId(-1),
		  m_sharedMemoryPtr(0),
		  m_createdSharedMemory(true)
	{
	}
};

struct PosixSharedMemoryInteralData
{
	AlignedObjectArray<SharedMemorySegment> m_segments;

	PosixSharedMemoryInteralData()
	{
	}
};

PosixSharedMemory::PosixSharedMemory()
{
	m_internalData = new PosixSharedMemoryInteralData;
}

PosixSharedMemory::~PosixSharedMemory()
{
	delete m_internalData;
}

struct PointerCaster
{
	union {
		uk ptr;
		ptrdiff_t integer;
	};
};

uk PosixSharedMemory::allocateSharedMemory(i32 key, i32 size, bool allowCreation)
{
#ifdef TEST_SHARED_MEMORY

	{
		SharedMemorySegment* seg = 0;
		i32 i = 0;

		for (i = 0; i < m_internalData->m_segments.size(); i++)
		{
			if (m_internalData->m_segments[i].m_key == key)
			{
				seg = &m_internalData->m_segments[i];
				break;
			}
		}
		if (seg)
		{
			drx3DError("already created shared memory segment using same key");
			return seg->m_sharedMemoryPtr;
		}
	}

	i32 flags = (allowCreation ? IPC_CREAT : 0) | 0666;
	i32 id = shmget((key_t)key, (size_t)size, flags);
	if (id < 0)
	{
		//drx3DWarning("shmget error1");
	}
	else
	{
		PointerCaster result;
		result.ptr = shmat(id, 0, 0);
		if (result.integer == -1)
		{
			drx3DError("shmat returned -1");
		}
		else
		{
			SharedMemorySegment seg;
			seg.m_key = key;
			seg.m_createdSharedMemory = allowCreation;
			seg.m_sharedMemoryId = id;
			seg.m_sharedMemoryPtr = result.ptr;
			m_internalData->m_segments.push_back(seg);
			return result.ptr;
		}
	}
#else
	//not implemented yet
	Assert(0);
#endif
	return 0;
}
void PosixSharedMemory::releaseSharedMemory(i32 key, i32 size)
{
#ifdef TEST_SHARED_MEMORY

	SharedMemorySegment* seg = 0;
	i32 i = 0;

	for (i = 0; i < m_internalData->m_segments.size(); i++)
	{
		if (m_internalData->m_segments[i].m_key == key)
		{
			seg = &m_internalData->m_segments[i];
			break;
		}
	}

	if (0 == seg)
	{
		drx3DError("PosixSharedMemory::releaseSharedMemory: shared memory key not found");
		return;
	}

	if (seg->m_sharedMemoryId < 0)
	{
		drx3DError("PosixSharedMemory::releaseSharedMemory: shared memory id is not set");
	}
	else
	{
		if (seg->m_createdSharedMemory)
		{
			i32 result = shmctl(seg->m_sharedMemoryId, IPC_RMID, 0);
			if (result == -1)
			{
				drx3DError("PosixSharedMemory::releaseSharedMemory: shmat returned -1");
			}
			else
			{
				drx3DPrintf("PosixSharedMemory::releaseSharedMemory removed shared memory");
			}
			seg->m_createdSharedMemory = false;
			seg->m_sharedMemoryId = -1;
		}
		if (seg->m_sharedMemoryPtr)
		{
			shmdt(seg->m_sharedMemoryPtr);
			seg->m_sharedMemoryPtr = 0;
			drx3DPrintf("PosixSharedMemory::releaseSharedMemory detached shared memory\n");
		}
	}

	m_internalData->m_segments.removeAtIndex(i);

#endif
}
