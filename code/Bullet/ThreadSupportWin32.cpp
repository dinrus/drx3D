
#if defined(_WIN32) && DRX3D_THREADSAFE

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Maths/Linear/TaskScheduler/ThreadSupportInterface.h>
#include <windows.h>
#include <stdio.h>

struct ProcessorInfo
{
	i32 numLogicalProcessors;
	i32 numCores;
	i32 numNumaNodes;
	i32 numL1Cache;
	i32 numL2Cache;
	i32 numL3Cache;
	i32 numPhysicalPackages;
	static i32k maxNumTeamMasks = 32;
	i32 numTeamMasks;
	UINT64 processorTeamMasks[maxNumTeamMasks];
};

UINT64 getProcessorTeamMask(const ProcessorInfo& procInfo, i32 procId)
{
	UINT64 procMask = UINT64(1) << procId;
	for (i32 i = 0; i < procInfo.numTeamMasks; ++i)
	{
		if (procMask & procInfo.processorTeamMasks[i])
		{
			return procInfo.processorTeamMasks[i];
		}
	}
	return 0;
}

i32 getProcessorTeamIndex(const ProcessorInfo& procInfo, i32 procId)
{
	UINT64 procMask = UINT64(1) << procId;
	for (i32 i = 0; i < procInfo.numTeamMasks; ++i)
	{
		if (procMask & procInfo.processorTeamMasks[i])
		{
			return i;
		}
	}
	return -1;
}

i32 countSetBits(ULONG64 bits)
{
	i32 count = 0;
	while (bits)
	{
		if (bits & 1)
		{
			count++;
		}
		bits >>= 1;
	}
	return count;
}

typedef BOOL(WINAPI* Pfn_GetLogicalProcessorInformation)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

void getProcessorInformation(ProcessorInfo* procInfo)
{
	memset(procInfo, 0, sizeof(*procInfo));
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	// Can't dlopen libraries on UWP.
	return;
#else
	Pfn_GetLogicalProcessorInformation getLogicalProcInfo =
		(Pfn_GetLogicalProcessorInformation)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (getLogicalProcInfo == NULL)
	{
		// no info
		return;
	}
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buf = NULL;
	DWORD bufSize = 0;
	while (true)
	{
		if (getLogicalProcInfo(buf, &bufSize))
		{
			break;
		}
		else
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buf)
				{
					free(buf);
				}
				buf = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(bufSize);
			}
		}
	}

	i32 len = bufSize / sizeof(*buf);
	for (i32 i = 0; i < len; ++i)
	{
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION info = buf + i;
		switch (info->Relationship)
		{
			case RelationNumaNode:
				procInfo->numNumaNodes++;
				break;

			case RelationProcessorCore:
				procInfo->numCores++;
				procInfo->numLogicalProcessors += countSetBits(info->ProcessorMask);
				break;

			case RelationCache:
				if (info->Cache.Level == 1)
				{
					procInfo->numL1Cache++;
				}
				else if (info->Cache.Level == 2)
				{
					procInfo->numL2Cache++;
				}
				else if (info->Cache.Level == 3)
				{
					procInfo->numL3Cache++;
					// processors that share L3 cache are considered to be on the same team
					// because they can more easily work together on the same data.
					// Large performance penalties will occur if 2 or more threads from different
					// teams attempt to frequently read and modify the same cache lines.
					//
					// On the AMD Ryzen 7 CPU for example, the 8 cores on the CPU are split into
					// 2 CCX units of 4 cores each. Each CCX has a separate L3 cache, so if both
					// CCXs are operating on the same data, many cycles will be spent keeping the
					// two caches coherent.
					if (procInfo->numTeamMasks < ProcessorInfo::maxNumTeamMasks)
					{
						procInfo->processorTeamMasks[procInfo->numTeamMasks] = info->ProcessorMask;
						procInfo->numTeamMasks++;
					}
				}
				break;

			case RelationProcessorPackage:
				procInfo->numPhysicalPackages++;
				break;
		}
	}
	free(buf);
#endif
}

//ThreadSupportWin32 helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class ThreadSupportWin32 : public ThreadSupportInterface
{
public:
	struct ThreadStatus
	{
		i32 m_taskId;
		i32 m_commandId;
		i32 m_status;

		ThreadFunc m_userThreadFunc;
		uk m_userPtr;  //for taskDesc etc

		uk m_threadHandle;  //this one is calling 'Win32ThreadFunc'

		uk m_eventStartHandle;
		char m_eventStartHandleName[32];

		uk m_eventCompleteHandle;
		char m_eventCompleteHandleName[32];
	};

private:
	AlignedObjectArray<ThreadStatus> m_activeThreadStatus;
	AlignedObjectArray<uk> m_completeHandles;
	i32 m_numThreads;
	DWORD_PTR m_startedThreadMask;
	ProcessorInfo m_processorInfo;

	void startThreads(const ConstructionInfo& threadInfo);
	void stopThreads();
	i32 waitForResponse();

public:
	ThreadSupportWin32(const ConstructionInfo& threadConstructionInfo);
	virtual ~ThreadSupportWin32();

	virtual i32 getNumWorkerThreads() const DRX3D_OVERRIDE { return m_numThreads; }
	virtual i32 getCacheFriendlyNumThreads() const DRX3D_OVERRIDE { return countSetBits(m_processorInfo.processorTeamMasks[0]); }
	virtual i32 getLogicalToPhysicalCoreRatio() const DRX3D_OVERRIDE { return m_processorInfo.numLogicalProcessors / m_processorInfo.numCores; }

	virtual void runTask(i32 threadIndex, uk userData) DRX3D_OVERRIDE;
	virtual void waitForAllTasks() DRX3D_OVERRIDE;

	virtual CriticalSection* createCriticalSection() DRX3D_OVERRIDE;
	virtual void deleteCriticalSection(CriticalSection* criticalSection) DRX3D_OVERRIDE;
};

ThreadSupportWin32::ThreadSupportWin32(const ConstructionInfo& threadConstructionInfo)
{
	startThreads(threadConstructionInfo);
}

ThreadSupportWin32::~ThreadSupportWin32()
{
	stopThreads();
}

DWORD WINAPI win32threadStartFunc(LPVOID lpParam)
{
	ThreadSupportWin32::ThreadStatus* status = (ThreadSupportWin32::ThreadStatus*)lpParam;

	while (1)
	{
		WaitForSingleObject(status->m_eventStartHandle, INFINITE);
		uk userPtr = status->m_userPtr;

		if (userPtr)
		{
			Assert(status->m_status);
			status->m_userThreadFunc(userPtr);
			status->m_status = 2;
			SetEvent(status->m_eventCompleteHandle);
		}
		else
		{
			//exit Thread
			status->m_status = 3;
			printf("Thread with taskId %i with handle %p exiting\n", status->m_taskId, status->m_threadHandle);
			SetEvent(status->m_eventCompleteHandle);
			break;
		}
	}
	printf("Thread TERMINATED\n");
	return 0;
}

void ThreadSupportWin32::runTask(i32 threadIndex, uk userData)
{
	ThreadStatus& threadStatus = m_activeThreadStatus[threadIndex];
	Assert(threadIndex >= 0);
	Assert(i32(threadIndex) < m_activeThreadStatus.size());

	threadStatus.m_commandId = 1;
	threadStatus.m_status = 1;
	threadStatus.m_userPtr = userData;
	m_startedThreadMask |= DWORD_PTR(1) << threadIndex;

	///fire event to start new task
	SetEvent(threadStatus.m_eventStartHandle);
}

i32 ThreadSupportWin32::waitForResponse()
{
	Assert(m_activeThreadStatus.size());

	i32 last = -1;
	DWORD res = WaitForMultipleObjects(m_completeHandles.size(), &m_completeHandles[0], FALSE, INFINITE);
	Assert(res != WAIT_FAILED);
	last = res - WAIT_OBJECT_0;

	ThreadStatus& threadStatus = m_activeThreadStatus[last];
	Assert(threadStatus.m_threadHandle);
	Assert(threadStatus.m_eventCompleteHandle);

	//WaitForSingleObject(threadStatus.m_eventCompleteHandle, INFINITE);
	Assert(threadStatus.m_status > 1);
	threadStatus.m_status = 0;

	///need to find an active spu
	Assert(last >= 0);
	m_startedThreadMask &= ~(DWORD_PTR(1) << last);

	return last;
}

void ThreadSupportWin32::waitForAllTasks()
{
	while (m_startedThreadMask)
	{
		waitForResponse();
	}
}

void ThreadSupportWin32::startThreads(const ConstructionInfo& threadConstructionInfo)
{
	static i32 uniqueId = 0;
	uniqueId++;
	ProcessorInfo& procInfo = m_processorInfo;
	getProcessorInformation(&procInfo);
	DWORD_PTR dwProcessAffinityMask = 0;
	DWORD_PTR dwSystemAffinityMask = 0;
	if (!GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinityMask, &dwSystemAffinityMask))
	{
		dwProcessAffinityMask = 0;
	}
	///The number of threads should be equal to the number of available cores - 1
	m_numThreads = d3Min(procInfo.numLogicalProcessors, i32(DRX3D_MAX_THREAD_COUNT)) - 1;  // cap to max thread count (-1 because main thread already exists)

	m_activeThreadStatus.resize(m_numThreads);
	m_completeHandles.resize(m_numThreads);
	m_startedThreadMask = 0;

	// set main thread affinity
	if (DWORD_PTR mask = dwProcessAffinityMask & getProcessorTeamMask(procInfo, 0))
	{
		SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadIdealProcessor(GetCurrentThread(), 0);
	}

	for (i32 i = 0; i < m_numThreads; i++)
	{
		printf("starting thread %d\n", i);

		ThreadStatus& threadStatus = m_activeThreadStatus[i];

		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
		SIZE_T dwStackSize = threadConstructionInfo.m_threadStackSize;
		LPTHREAD_START_ROUTINE lpStartAddress = &win32threadStartFunc;
		LPVOID lpParameter = &threadStatus;
		DWORD dwCreationFlags = 0;
		LPDWORD lpThreadId = 0;

		threadStatus.m_userPtr = 0;

		sprintf(threadStatus.m_eventStartHandleName, "es%.8s%d%d", threadConstructionInfo.m_uniqueName, uniqueId, i);
		threadStatus.m_eventStartHandle = CreateEventA(0, false, false, threadStatus.m_eventStartHandleName);

		sprintf(threadStatus.m_eventCompleteHandleName, "ec%.8s%d%d", threadConstructionInfo.m_uniqueName, uniqueId, i);
		threadStatus.m_eventCompleteHandle = CreateEventA(0, false, false, threadStatus.m_eventCompleteHandleName);

		m_completeHandles[i] = threadStatus.m_eventCompleteHandle;

		HANDLE handle = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
		//SetThreadPriority( handle, THREAD_PRIORITY_HIGHEST );
		// highest priority -- can cause erratic performance when numThreads > numCores
		//                     we don't want worker threads to be higher priority than the main thread or the main thread could get
		//                     totally shut out and unable to tell the workers to stop
		//SetThreadPriority( handle, THREAD_PRIORITY_BELOW_NORMAL );

		{
			i32 processorId = i + 1;  // leave processor 0 for main thread
			DWORD_PTR teamMask = getProcessorTeamMask(procInfo, processorId);
			if (teamMask)
			{
				// bind each thread to only execute on processors of it's assigned team
				//  - for single-socket Intel x86 CPUs this has no effect (only a single, shared L3 cache so there is only 1 team)
				//  - for multi-socket Intel this will keep threads from migrating from one socket to another
				//  - for AMD Ryzen this will keep threads from migrating from one CCX to another
				DWORD_PTR mask = teamMask & dwProcessAffinityMask;
				if (mask)
				{
					SetThreadAffinityMask(handle, mask);
				}
			}
			SetThreadIdealProcessor(handle, processorId);
		}

		threadStatus.m_taskId = i;
		threadStatus.m_commandId = 0;
		threadStatus.m_status = 0;
		threadStatus.m_threadHandle = handle;
		threadStatus.m_userThreadFunc = threadConstructionInfo.m_userThreadFunc;

		printf("started %s thread %d with threadHandle %p\n", threadConstructionInfo.m_uniqueName, i, handle);
	}
}

///tell the task scheduler we are done with the SPU tasks
void ThreadSupportWin32::stopThreads()
{
	for (i32 i = 0; i < m_activeThreadStatus.size(); i++)
	{
		ThreadStatus& threadStatus = m_activeThreadStatus[i];
		if (threadStatus.m_status > 0)
		{
			WaitForSingleObject(threadStatus.m_eventCompleteHandle, INFINITE);
		}

		threadStatus.m_userPtr = NULL;
		SetEvent(threadStatus.m_eventStartHandle);
		WaitForSingleObject(threadStatus.m_eventCompleteHandle, INFINITE);

		CloseHandle(threadStatus.m_eventCompleteHandle);
		CloseHandle(threadStatus.m_eventStartHandle);
		CloseHandle(threadStatus.m_threadHandle);
	}

	m_activeThreadStatus.clear();
	m_completeHandles.clear();
}

class Win32CriticalSection : public CriticalSection
{
private:
	CRITICAL_SECTION mCriticalSection;

public:
	btWin32CriticalSection()
	{
		InitializeCriticalSection(&mCriticalSection);
	}

	~btWin32CriticalSection()
	{
		DeleteCriticalSection(&mCriticalSection);
	}

	void lock()
	{
		EnterCriticalSection(&mCriticalSection);
	}

	void unlock()
	{
		LeaveCriticalSection(&mCriticalSection);
	}
};

CriticalSection* ThreadSupportWin32::createCriticalSection()
{
	u8* mem = (u8*)AlignedAlloc(sizeof(Win32CriticalSection), 16);
	btWin32CriticalSection* cs = new (mem) btWin32CriticalSection();
	return cs;
}

void ThreadSupportWin32::deleteCriticalSection(CriticalSection* criticalSection)
{
	criticalSection->~CriticalSection();
	AlignedFree(criticalSection);
}

ThreadSupportInterface* ThreadSupportInterface::create(const ConstructionInfo& info)
{
	return new ThreadSupportWin32(info);
}

#endif  //defined(_WIN32) && DRX3D_THREADSAFE
