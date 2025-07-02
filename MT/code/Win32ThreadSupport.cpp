#ifdef _WIN32

#include "../b3Win32ThreadSupport.h"

#include <windows.h>

///The number of threads should be equal to the number of available cores
///@todo: each worker should be linked to a single core, using SetThreadIdealProcessor.

///b3Win32ThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
///Setup and initialize SPU/CELL/Libspe2
b3Win32ThreadSupport::b3Win32ThreadSupport(const Win32ThreadConstructionInfo& threadConstructionInfo)
{
	m_maxNumTasks = threadConstructionInfo.m_numThreads;
	startThreads(threadConstructionInfo);
}

///cleanup/shutdown Libspe2
b3Win32ThreadSupport::~b3Win32ThreadSupport()
{
	stopThreads();
}

#include <stdio.h>

DWORD WINAPI Thread_no_1(LPVOID lpParam)
{
	b3Win32ThreadSupport::b3ThreadStatus* status = (b3Win32ThreadSupport::b3ThreadStatus*)lpParam;

	while (1)
	{
		WaitForSingleObject(status->m_eventStartHandle, INFINITE);

		uk userPtr = status->m_userPtr;

		if (userPtr)
		{
			drx3DAssert(status->m_status);
			status->m_userThreadFunc(userPtr, status->m_lsMemory);
			status->m_status = 2;
			SetEvent(status->m_eventCompletetHandle);
		}
		else
		{
			//exit Thread
			status->m_status = 3;
			printf("Thread with taskId %i with handle %p exiting\n", status->m_taskId, status->m_threadHandle);
			SetEvent(status->m_eventCompletetHandle);
			break;
		}
	}

	printf("Thread TERMINATED\n");
	return 0;
}

///send messages to SPUs
void b3Win32ThreadSupport::runTask(i32 uiCommand, uk uiArgument0, i32 taskId)
{
	///	gMidphaseSPU.sendRequest(CMD_GATHER_AND_PROCESS_PAIRLIST, (uk ) &taskDesc);

	///we should spawn an SPU task here, and in 'waitForResponse' it should wait for response of the (one of) the first tasks that finished

	switch (uiCommand)
	{
		case D3_THREAD_SCHEDULE_TASK:
		{
//#define SINGLE_THREADED 1
#ifdef SINGLE_THREADED

			b3ThreadStatus& threadStatus = m_activeThreadStatus[0];
			threadStatus.m_userPtr = (uk )uiArgument0;
			threadStatus.m_userThreadFunc(threadStatus.m_userPtr, threadStatus.m_lsMemory);
			HANDLE handle = 0;
#else

			b3ThreadStatus& threadStatus = m_activeThreadStatus[taskId];
			drx3DAssert(taskId >= 0);
			drx3DAssert(i32(taskId) < m_activeThreadStatus.size());

			threadStatus.m_commandId = uiCommand;
			threadStatus.m_status = 1;
			threadStatus.m_userPtr = (uk )uiArgument0;

			///fire event to start new task
			SetEvent(threadStatus.m_eventStartHandle);

#endif  //CollisionTask_LocalStoreMemory

			break;
		}
		default:
		{
			///not implemented
			drx3DAssert(0);
		}
	};
}

///check for messages from SPUs
void b3Win32ThreadSupport::waitForResponse(i32* puiArgument0, i32* puiArgument1)
{
	///We should wait for (one of) the first tasks to finish (or other SPU messages), and report its response

	///A possible response can be 'yes, SPU handled it', or 'no, please do a PPU fallback'

	drx3DAssert(m_activeThreadStatus.size());

	i32 last = -1;
#ifndef SINGLE_THREADED
	DWORD res = WaitForMultipleObjects(m_completeHandles.size(), &m_completeHandles[0], FALSE, INFINITE);
	drx3DAssert(res != WAIT_FAILED);
	last = res - WAIT_OBJECT_0;

	b3ThreadStatus& threadStatus = m_activeThreadStatus[last];
	drx3DAssert(threadStatus.m_threadHandle);
	drx3DAssert(threadStatus.m_eventCompletetHandle);

	//WaitForSingleObject(threadStatus.m_eventCompletetHandle, INFINITE);
	drx3DAssert(threadStatus.m_status > 1);
	threadStatus.m_status = 0;

	///need to find an active spu
	drx3DAssert(last >= 0);

#else
	last = 0;
	b3ThreadStatus& threadStatus = m_activeThreadStatus[last];
#endif  //SINGLE_THREADED

	*puiArgument0 = threadStatus.m_taskId;
	*puiArgument1 = threadStatus.m_status;
}

///check for messages from SPUs
bool b3Win32ThreadSupport::isTaskCompleted(i32* puiArgument0, i32* puiArgument1, i32 timeOutInMilliseconds)
{
	///We should wait for (one of) the first tasks to finish (or other SPU messages), and report its response

	///A possible response can be 'yes, SPU handled it', or 'no, please do a PPU fallback'

	drx3DAssert(m_activeThreadStatus.size());

	i32 last = -1;
#ifndef SINGLE_THREADED
	DWORD res = WaitForMultipleObjects(m_completeHandles.size(), &m_completeHandles[0], FALSE, timeOutInMilliseconds);

	if ((res != STATUS_TIMEOUT) && (res != WAIT_FAILED))
	{
		drx3DAssert(res != WAIT_FAILED);
		last = res - WAIT_OBJECT_0;

		b3ThreadStatus& threadStatus = m_activeThreadStatus[last];
		drx3DAssert(threadStatus.m_threadHandle);
		drx3DAssert(threadStatus.m_eventCompletetHandle);

		//WaitForSingleObject(threadStatus.m_eventCompletetHandle, INFINITE);
		drx3DAssert(threadStatus.m_status > 1);
		threadStatus.m_status = 0;

		///need to find an active spu
		drx3DAssert(last >= 0);

#else
	last = 0;
	b3ThreadStatus& threadStatus = m_activeThreadStatus[last];
#endif  //SINGLE_THREADED

		*puiArgument0 = threadStatus.m_taskId;
		*puiArgument1 = threadStatus.m_status;

		return true;
	}

	return false;
}

void b3Win32ThreadSupport::startThreads(const Win32ThreadConstructionInfo& threadConstructionInfo)
{
	static i32 uniqueId = 0;
	uniqueId++;
	m_activeThreadStatus.resize(threadConstructionInfo.m_numThreads);
	m_completeHandles.resize(threadConstructionInfo.m_numThreads);

	m_maxNumTasks = threadConstructionInfo.m_numThreads;

	for (i32 i = 0; i < threadConstructionInfo.m_numThreads; i++)
	{
		printf("starting thread %d\n", i);

		b3ThreadStatus& threadStatus = m_activeThreadStatus[i];

		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
		SIZE_T dwStackSize = threadConstructionInfo.m_threadStackSize;
		LPTHREAD_START_ROUTINE lpStartAddress = &Thread_no_1;
		LPVOID lpParameter = &threadStatus;
		DWORD dwCreationFlags = 0;
		LPDWORD lpThreadId = 0;

		threadStatus.m_userPtr = 0;

		sprintf(threadStatus.m_eventStartHandleName, "es%.8s%d%d", threadConstructionInfo.m_uniqueName, uniqueId, i);
		threadStatus.m_eventStartHandle = CreateEventA(0, false, false, threadStatus.m_eventStartHandleName);

		sprintf(threadStatus.m_eventCompletetHandleName, "ec%.8s%d%d", threadConstructionInfo.m_uniqueName, uniqueId, i);
		threadStatus.m_eventCompletetHandle = CreateEventA(0, false, false, threadStatus.m_eventCompletetHandleName);

		m_completeHandles[i] = threadStatus.m_eventCompletetHandle;

		HANDLE handle = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
		switch (threadConstructionInfo.m_priority)
		{
			case 0:
			{
				SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
				break;
			}
			case 1:
			{
				SetThreadPriority(handle, THREAD_PRIORITY_TIME_CRITICAL);
				break;
			}
			case 2:
			{
				SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);
				break;
			}

			default:
			{
			}
		}

		//SetThreadAffinityMask(handle, 1 << 1); // this is what it was doing originally, a complete disaster for threading performance!
		//SetThreadAffinityMask(handle, 1 << i); // I'm guessing this was the intention, but is still bad for performance due to one of the threads
		//  sometimes unable to execute because it wants to be on the same processor as the main thread (my guess)

		threadStatus.m_taskId = i;
		threadStatus.m_commandId = 0;
		threadStatus.m_status = 0;
		threadStatus.m_threadHandle = handle;
		threadStatus.m_lsMemory = threadConstructionInfo.m_lsMemoryFunc();
		threadStatus.m_userThreadFunc = threadConstructionInfo.m_userThreadFunc;
		threadStatus.m_lsMemoryReleaseFunc = threadConstructionInfo.m_lsMemoryReleaseFunc;

		printf("started %s thread %d with threadHandle %p\n", threadConstructionInfo.m_uniqueName, i, handle);
	}
}

void b3Win32ThreadSupport::startThreads()
{
}

///tell the task scheduler we are done with the SPU tasks
void b3Win32ThreadSupport::stopThreads()
{
	i32 i;
	for (i = 0; i < m_activeThreadStatus.size(); i++)
	{
		b3ThreadStatus& threadStatus = m_activeThreadStatus[i];
		if (threadStatus.m_status > 0)
		{
			WaitForSingleObject(threadStatus.m_eventCompletetHandle, INFINITE);
		}

		if (threadStatus.m_lsMemoryReleaseFunc)
		{
			threadStatus.m_lsMemoryReleaseFunc(threadStatus.m_lsMemory);
		}

		threadStatus.m_userPtr = 0;
		SetEvent(threadStatus.m_eventStartHandle);
		WaitForSingleObject(threadStatus.m_eventCompletetHandle, INFINITE);

		CloseHandle(threadStatus.m_eventCompletetHandle);
		CloseHandle(threadStatus.m_eventStartHandle);
		CloseHandle(threadStatus.m_threadHandle);
	}

	m_activeThreadStatus.clear();
	m_completeHandles.clear();
}

class b3Win32Barrier : public b3Barrier
{
private:
	CRITICAL_SECTION mExternalCriticalSection;
	CRITICAL_SECTION mLocalCriticalSection;
	HANDLE mRunEvent, mNotifyEvent;
	i32 mCounter, mEnableCounter;
	i32 mMaxCount;

public:
	b3Win32Barrier()
	{
		mCounter = 0;
		mMaxCount = 1;
		mEnableCounter = 0;
		InitializeCriticalSection(&mExternalCriticalSection);
		InitializeCriticalSection(&mLocalCriticalSection);
		mRunEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		mNotifyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	virtual ~b3Win32Barrier()
	{
		DeleteCriticalSection(&mExternalCriticalSection);
		DeleteCriticalSection(&mLocalCriticalSection);
		CloseHandle(mRunEvent);
		CloseHandle(mNotifyEvent);
	}

	void sync()
	{
		i32 eventId;

		EnterCriticalSection(&mExternalCriticalSection);

		//PFX_PRINTF("enter taskId %d count %d stage %d phase %d mEnableCounter %d\n",taskId,mCounter,debug&0xff,debug>>16,mEnableCounter);

		if (mEnableCounter > 0)
		{
			ResetEvent(mNotifyEvent);
			LeaveCriticalSection(&mExternalCriticalSection);
			WaitForSingleObject(mNotifyEvent, INFINITE);
			EnterCriticalSection(&mExternalCriticalSection);
		}

		eventId = mCounter;
		mCounter++;

		if (eventId == mMaxCount - 1)
		{
			SetEvent(mRunEvent);

			mEnableCounter = mCounter - 1;
			mCounter = 0;
		}
		else
		{
			ResetEvent(mRunEvent);
			LeaveCriticalSection(&mExternalCriticalSection);
			WaitForSingleObject(mRunEvent, INFINITE);
			EnterCriticalSection(&mExternalCriticalSection);
			mEnableCounter--;
		}

		if (mEnableCounter == 0)
		{
			SetEvent(mNotifyEvent);
		}

		//PFX_PRINTF("leave taskId %d count %d stage %d phase %d mEnableCounter %d\n",taskId,mCounter,debug&0xff,debug>>16,mEnableCounter);

		LeaveCriticalSection(&mExternalCriticalSection);
	}

	virtual void setMaxCount(i32 n) { mMaxCount = n; }
	virtual i32 getMaxCount() { return mMaxCount; }
};

class b3Win32CriticalSection : public b3CriticalSection
{
private:
	CRITICAL_SECTION mCriticalSection;

public:
	b3Win32CriticalSection()
	{
		InitializeCriticalSection(&mCriticalSection);
	}

	~b3Win32CriticalSection()
	{
		DeleteCriticalSection(&mCriticalSection);
	}

	u32 getSharedParam(i32 i)
	{
		drx3DAssert(i >= 0 && i < 31);
		return mCommonBuff[i + 1];
	}

	void setSharedParam(i32 i, u32 p)
	{
		drx3DAssert(i >= 0 && i < 31);
		mCommonBuff[i + 1] = p;
	}

	void lock()
	{
		EnterCriticalSection(&mCriticalSection);
		mCommonBuff[0] = 1;
	}

	void unlock()
	{
		mCommonBuff[0] = 0;
		LeaveCriticalSection(&mCriticalSection);
	}
};

b3Barrier* b3Win32ThreadSupport::createBarrier()
{
	u8* mem = (u8*)b3AlignedAlloc(sizeof(b3Win32Barrier), 16);
	b3Win32Barrier* barrier = new (mem) b3Win32Barrier();
	barrier->setMaxCount(getNumTasks());
	return barrier;
}

b3CriticalSection* b3Win32ThreadSupport::createCriticalSection()
{
	u8* mem = (u8*)b3AlignedAlloc(sizeof(b3Win32CriticalSection), 16);
	b3Win32CriticalSection* cs = new (mem) b3Win32CriticalSection();
	return cs;
}

void b3Win32ThreadSupport::deleteBarrier(b3Barrier* barrier)
{
	barrier->~b3Barrier();
	b3AlignedFree(barrier);
}

void b3Win32ThreadSupport::deleteCriticalSection(b3CriticalSection* criticalSection)
{
	criticalSection->~b3CriticalSection();
	b3AlignedFree(criticalSection);
}

#endif  //_WIN32
