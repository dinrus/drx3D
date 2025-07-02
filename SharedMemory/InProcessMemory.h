#ifndef IN_PROCESS_MEMORY_H
#define IN_PROCESS_MEMORY_H

#include <drx3D/SharedMemory/SharedMemoryInterface.h>

class InProcessMemory : public SharedMemoryInterface
{
	struct InProcessMemoryInternalData* m_data;

public:
	InProcessMemory();
	virtual ~InProcessMemory();

	virtual uk allocateSharedMemory(i32 key, i32 size, bool allowCreation);
	virtual void releaseSharedMemory(i32 key, i32 size);
};

#endif
