#ifndef POSIX_SHARED_MEMORY_H
#define POSIX_SHARED_MEMORY_H

#include <drx3D/SharedMemory/SharedMemoryInterface.h>

class PosixSharedMemory : public SharedMemoryInterface
{
	struct PosixSharedMemoryInteralData* m_internalData;

public:
	PosixSharedMemory();
	virtual ~PosixSharedMemory();

	virtual uk allocateSharedMemory(i32 key, i32 size, bool allowCreation);
	virtual void releaseSharedMemory(i32 key, i32 size);
};

#endif  //
