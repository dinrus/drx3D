#ifndef D3_POSIX_THREAD_SUPPORT_H
#define D3_POSIX_THREAD_SUPPORT_H

#include <drx3D/Common/b3Scalar.h>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600  //for definition of pthread_barrier_t, see http://pages.cs.wisc.edu/~travitch/pthreads_primer.html
#endif                     //_XOPEN_SOURCE
#include <pthread.h>
#include <semaphore.h>

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/MT/ThreadSupportInterface.h>

typedef void (*b3PosixThreadFunc)(uk userPtr, uk lsMemory);
typedef uk (*b3PosixlsMemorySetupFunc)();
typedef void (*b3PosixlsMemoryReleaseFunc)(uk ptr);

// PosixThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class PosixThreadSupport : public ThreadSupportInterface
{
public:
	typedef enum sStatus
	{
		STATUS_BUSY,
		STATUS_READY,
		STATUS_FINISHED
	} Status;

	// placeholder, until libspe2 support is there
	struct b3ThreadStatus
	{
		i32 m_taskId;
		i32 m_commandId;
		i32 m_status;

		b3PosixThreadFunc m_userThreadFunc;
		uk m_userPtr;  //for taskDesc etc
		b3PosixlsMemoryReleaseFunc m_lsMemoryReleaseFunc;

		uk m_lsMemory;  //initialized using PosixLocalStoreMemorySetupFunc

		pthread_t thread;
		//each tread will wait until this signal to start its work
		sem_t* startSemaphore;

		// this is a copy of m_mainSemaphore,
		//each tread will signal once it is finished with its work
		sem_t* m_mainSemaphore;
		u64 threadUsed;
	};

private:
	b3AlignedObjectArray<b3ThreadStatus> m_activeThreadStatus;

	// m_mainSemaphoresemaphore will signal, if and how many threads are finished with their work
	sem_t* m_mainSemaphore;

public:
	///Setup and initialize SPU/CELL/Libspe2

	struct ThreadConstructionInfo
	{
		ThreadConstructionInfo(tukk uniqueName,
							   b3PosixThreadFunc userThreadFunc,
							   b3PosixlsMemorySetupFunc lsMemoryFunc,
							   b3PosixlsMemoryReleaseFunc lsMemoryReleaseFunc,
							   i32 numThreads = 1,
							   i32 threadStackSize = 65535)
			: m_uniqueName(uniqueName),
			  m_userThreadFunc(userThreadFunc),
			  m_lsMemoryFunc(lsMemoryFunc),
			  m_lsMemoryReleaseFunc(lsMemoryReleaseFunc),
			  m_numThreads(numThreads),
			  m_threadStackSize(threadStackSize)
		{
		}

		tukk m_uniqueName;
		b3PosixThreadFunc m_userThreadFunc;
		b3PosixlsMemorySetupFunc m_lsMemoryFunc;
		b3PosixlsMemoryReleaseFunc m_lsMemoryReleaseFunc;

		i32 m_numThreads;
		i32 m_threadStackSize;
	};

	PosixThreadSupport(ThreadConstructionInfo& threadConstructionInfo);

	///cleanup/shutdown Libspe2
	virtual ~PosixThreadSupport();

	void startThreads(ThreadConstructionInfo& threadInfo);

	virtual void runTask(i32 uiCommand, uk uiArgument0, i32 uiArgument1);

	virtual void waitForResponse(i32* puiArgument0, i32* puiArgument1);

	///tell the task scheduler we are done with the SPU tasks
	virtual void stopThreads();

	virtual void setNumTasks(i32 numTasks) {}

	virtual i32 getNumTasks() const
	{
		return m_activeThreadStatus.size();
	}

	///non-blocking test if a task is completed. First implement all versions, and then enable this API
	virtual bool isTaskCompleted(i32* puiArgument0, i32* puiArgument1, i32 timeOutInMilliseconds);

	virtual b3Barrier* createBarrier();

	virtual b3CriticalSection* createCriticalSection();

	virtual void deleteBarrier(b3Barrier* barrier);

	virtual void deleteCriticalSection(b3CriticalSection* criticalSection);

	virtual uk getThreadLocalMemory(i32 taskId)
	{
		return m_activeThreadStatus[taskId].m_lsMemory;
	}
};

#endif  // D3_POSIX_THREAD_SUPPORT_H
