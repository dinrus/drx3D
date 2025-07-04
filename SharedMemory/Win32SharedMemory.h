#ifndef WIN32_SHARED_MEMORY_H
#define WIN32_SHARED_MEMORY_H

#include <drx3D/SharedMemory/SharedMemoryInterface.h>

class Win32SharedMemory : public SharedMemoryInterface
{
	struct Win32SharedMemoryInteralData* m_internalData;

public:
	Win32SharedMemory();
	virtual ~Win32SharedMemory();

	virtual uk allocateSharedMemory(i32 key, i32 size, bool allowCreation);
	virtual void releaseSharedMemory(i32 key, i32 size);
};

class Win32SharedMemoryServer : public Win32SharedMemory
{
public:
	Win32SharedMemoryServer();
	virtual ~Win32SharedMemoryServer();
};

class Win32SharedMemoryClient : public Win32SharedMemory
{
public:
	Win32SharedMemoryClient();
	virtual ~Win32SharedMemoryClient();
};

#endif  //WIN32_SHARED_MEMORY_H
