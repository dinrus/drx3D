#ifndef SHARED_MEMORY_INTERFACE_H
#define SHARED_MEMORY_INTERFACE_H

#include <drxtypes.h>

class SharedMemoryInterface
{
public:
	virtual ~SharedMemoryInterface()
	{
	}

	virtual uk allocateSharedMemory(i32 key, i32 size, bool allowCreation) = 0;
	virtual void releaseSharedMemory(i32 key, i32 size) = 0;
};

#endif
