#include <drx3D/Common/b3Scalar.h"

#ifndef DRX3D_WIN32_THREAD_SUPPORT_H
#define DRX3D_WIN32_THREAD_SUPPORT_H

#include <drx3D/Common/b3AlignedObjectArray.h"

#include "ThreadSupportInterface.h"

typedef void (*b3Win32ThreadFunc)(uk userPtr, uk lsMemory);
typedef uk (*b3Win32lsMemorySetupFunc)();
typedef void (*b3Win32lsMemoryReleaseFunc)(uk );

///b3Win32ThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class b3Win32ThreadSupport : public ThreadSupportInterface
{
public:
	///placeholder, until libspe2 support is there
	struct b3ThreadStatus
	{
		i32 m_taskId;
		i32 m_commandId;
		i32 m_status;

		b3Win32ThreadFunc m_userThreadFunc;
		uk m_userPtr;   //for taskDesc etc
		uk m_lsMemory;  //initialized using Win32LocalStoreMemorySetupFunc

		b3Win32lsMemoryReleaseFunc m_lsMemoryReleaseFunc;

		uk m_threadHandle;  //this one is calling 'Win32ThreadFunc'

		uk m_eventStartHandle;
		char m_eventStartHandleName[32];

		uk m_eventCompletetHandle;
		char m_eventCompletetHandleName[32];
	};

private:
	b3AlignedObjectArray<b3ThreadStatus> m_activeThreadStatus;
	b3AlignedObjectArray<uk> m_completeHandles;

	i32 m_maxNumTasks;

public:
	///Setup and initialize SPU/CELL/Libspe2

	struct Win32ThreadConstructionInfo
	{
		Win32ThreadConstructionInfo(tukk uniqueName,
									b3Win32ThreadFunc userThreadFunc,
									b3Win32lsMemorySetupFunc lsMemoryFunc,
									b3Win32lsMemoryReleaseFunc lsMemoryReleaseFunc,
									i32 numThreads = 1,
									i32 threadStackSize = 65535)
			: m_uniqueName(uniqueName),
			  m_userThreadFunc(userThreadFunc),
			  m_lsMemoryFunc(lsMemoryFunc),
			  m_lsMemoryReleaseFunc(lsMemoryReleaseFunc),
			  m_numThreads(numThreads),
			  m_threadStackSize(threadStackSize),
			  m_priority(0)
		{
		}

		tukk m_uniqueName;
		b3Win32ThreadFunc m_userThreadFunc;
		b3Win32lsMemorySetupFunc m_lsMemoryFunc;
		b3Win32lsMemoryReleaseFunc m_lsMemoryReleaseFunc;
		i32 m_numThreads;
		i32 m_threadStackSize;
		i32 m_priority;
	};

	b3Win32ThreadSupport(const Win32ThreadConstructionInfo& threadConstructionInfo);

	///cleanup/shutdown Libspe2
	virtual ~b3Win32ThreadSupport();

	void startThreads(const Win32ThreadConstructionInfo& threadInfo);

	///send messages to SPUs
	virtual void runTask(i32 uiCommand, uk uiArgument0, i32 uiArgument1);

	///check for messages from SPUs
	virtual void waitForResponse(i32* puiArgument0, i32* puiArgument1);

	virtual bool isTaskCompleted(i32* puiArgument0, i32* puiArgument1, i32 timeOutInMilliseconds);

	///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual void startThreads();

	///tell the task scheduler we are done with the SPU tasks
	virtual void stopThreads();

	virtual void setNumTasks(i32 numTasks)
	{
		m_maxNumTasks = numTasks;
	}

	virtual i32 getNumTasks() const
	{
		return m_maxNumTasks;
	}

	virtual uk getThreadLocalMemory(i32 taskId)
	{
		return m_activeThreadStatus[taskId].m_lsMemory;
	}
	virtual b3Barrier* createBarrier();

	virtual b3CriticalSection* createCriticalSection();

	virtual void deleteBarrier(b3Barrier* barrier);

	virtual void deleteCriticalSection(b3CriticalSection* criticalSection);
};

#endif  //DRX3D_WIN32_THREAD_SUPPORT_H
