/// ThreadingDemo shows how to use the cross platform thread support interface.
/// You can start threads and perform a blocking wait for completion
/// Under Windows it uses Win32 Threads. On Mac and Linux it uses pthreads. On PlayStation 3 Cell SPU it uses SPURS.

/// June 2010
/// New: critical section/barriers and non-blocking polling for completion

void SampleThreadFunc(uk userPtr, uk lsMemory);
uk SamplelsMemoryFunc();
void SamplelsMemoryReleaseFunc(uk ptr);
#include <stdio.h>
//#include "BulletMultiThreaded/PlatformDefinitions.h"

#ifndef _WIN32
#include <drx3D/MT/PosixThreadSupport.h>

ThreadSupportInterface* createThreadSupport(i32 numThreads)
{
	PosixThreadSupport::ThreadConstructionInfo constructionInfo("testThreads",
																  SampleThreadFunc,
																  SamplelsMemoryFunc,
SamplelsMemoryReleaseFunc,
																  numThreads);
	ThreadSupportInterface* threadSupport = new PosixThreadSupport(constructionInfo);

	return threadSupport;
}

#elif defined(_WIN32)
#include <drx3D/MT/b3Win32ThreadSupport.h>

ThreadSupportInterface* createThreadSupport(i32 numThreads)
{
	b3Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("testThreads", SampleThreadFunc, SamplelsMemoryFunc, SamplelsMemoryReleaseFunc, numThreads);
	b3Win32ThreadSupport* threadSupport = new b3Win32ThreadSupport(threadConstructionInfo);
	return threadSupport;
}
#endif

struct SampleArgs
{
	SampleArgs()
		: m_fakeWork(1)
	{
	}
	b3CriticalSection* m_cs;
	float m_fakeWork;
};

struct SampleThreadLocalStorage
{
	i32 threadId;
};

void SampleThreadFunc(uk userPtr, uk lsMemory)
{
	printf("thread started\n");

	SampleThreadLocalStorage* localStorage = (SampleThreadLocalStorage*)lsMemory;

	SampleArgs* args = (SampleArgs*)userPtr;
	i32 workLeft = true;
	while (workLeft)
	{
		args->m_cs->lock();
		i32 count = args->m_cs->getSharedParam(0);
		args->m_cs->setSharedParam(0, count - 1);
		args->m_cs->unlock();
		if (count > 0)
		{
			printf("thread %d processed number %d\n", localStorage->threadId, count);
		}
		//do some fake work
		for (i32 i = 0; i < 1000000; i++)
			args->m_fakeWork = b3Scalar(1.21) * args->m_fakeWork;
		workLeft = count > 0;
	}
	printf("finished\n");
	//do nothing
}

uk SamplelsMemoryFunc()
{
	//don't create local store memory, just return 0
	return new SampleThreadLocalStorage;
}

void SamplelsMemoryReleaseFunc(uk ptr)
{
        SampleThreadLocalStorage* p = (SampleThreadLocalStorage*)ptr;
        delete p;
}


i32 main(i32 argc, tuk* argv)
{
	i32 numThreads = 8;

	ThreadSupportInterface* threadSupport = createThreadSupport(numThreads);

	for (i32 i = 0; i < threadSupport->getNumTasks(); i++)
	{
		SampleThreadLocalStorage* storage = (SampleThreadLocalStorage*)threadSupport->getThreadLocalMemory(i);
		drx3DAssert(storage);
		storage->threadId = i;
	}

	SampleArgs args;
	args.m_cs = threadSupport->createCriticalSection();
	args.m_cs->setSharedParam(0, 100);

	i32 arg0, arg1;
	i32 i;
	for (i = 0; i < numThreads; i++)
	{
		threadSupport->runTask(D3_THREAD_SCHEDULE_TASK, (uk )&args, i);
	}

	bool blockingWait = false;
	if (blockingWait)
	{
		for (i = 0; i < numThreads; i++)
		{
			threadSupport->waitForResponse(&arg0, &arg1);
			printf("finished waiting for response: %d %d\n", arg0, arg1);
		}
	}
	else
	{
		i32 numActiveThreads = numThreads;
		while (numActiveThreads)
		{
			if (threadSupport->isTaskCompleted(&arg0, &arg1, 0))
			{
				numActiveThreads--;
				printf("numActiveThreads = %d\n", numActiveThreads);
			}
			else
			{
				//				printf("polling..");
			}
		};
	}

	printf("stopping threads\n");

	delete threadSupport;
	printf("Press ENTER to quit\n");
	//getchar();
	return 0;
}
