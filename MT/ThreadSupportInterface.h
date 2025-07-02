#ifndef D3_THREAD_SUPPORT_INTERFACE_H
#define D3_THREAD_SUPPORT_INTERFACE_H

enum
{
	D3_THREAD_SCHEDULE_TASK = 1,
};

#include <drx3D/Common/b3Scalar.h>  //for D3_ATTRIBUTE_ALIGNED16
//#include "PlatformDefinitions.h"
//#include "PpuAddressSpace.h"

class b3Barrier
{
public:
	b3Barrier() {}
	virtual ~b3Barrier() {}

	virtual void sync() = 0;
	virtual void setMaxCount(i32 n) = 0;
	virtual i32 getMaxCount() = 0;
};

class b3CriticalSection
{
public:
	b3CriticalSection() {}
	virtual ~b3CriticalSection() {}

	D3_ATTRIBUTE_ALIGNED16(u32 mCommonBuff[32]);

	virtual u32 getSharedParam(i32 i) = 0;
	virtual void setSharedParam(i32 i, u32 p) = 0;

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class ThreadSupportInterface
{
public:
	virtual ~ThreadSupportInterface();

	virtual void runTask(i32 uiCommand, uk uiArgument0, i32 uiArgument1) = 0;

	virtual void waitForResponse(i32* puiArgument0, i32* puiArgument1) = 0;

	///non-blocking test if a task is completed. First implement all versions, and then enable this API
	virtual bool isTaskCompleted(i32* puiArgument0, i32* puiArgument1, i32 timeOutInMilliseconds) = 0;

	virtual void stopThreads() = 0;

	///tell the task scheduler to use no more than numTasks tasks
	virtual void setNumTasks(i32 numTasks) = 0;

	virtual i32 getNumTasks() const = 0;

	virtual b3Barrier* createBarrier() = 0;

	virtual b3CriticalSection* createCriticalSection() = 0;

	virtual void deleteBarrier(b3Barrier* barrier) = 0;

	virtual void deleteCriticalSection(b3CriticalSection* criticalSection) = 0;

	virtual uk getThreadLocalMemory(i32 taskId) { return 0; }
};

#endif  //D3_THREAD_SUPPORT_INTERFACE_H
