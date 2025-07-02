

#if DRX3D_THREADSAFE && !defined(_WIN32)

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/TaskScheduler/ThreadSupportInterface.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600  //for definition of pthread_barrier_t, see http://pages.cs.wisc.edu/~travitch/pthreads_primer.html
#endif                     //_XOPEN_SOURCE
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>  //for sysconf

///
/// getNumHardwareThreads()
///
///
/// https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
///
#if __cplusplus >= 201103L

#include <thread>

i32 GetNumHardwareThreads()
{
	return d3Max(1u, d3Min(DRX3D_MAX_THREAD_COUNT, std::thread::hardware_concurrency()));
}

#else

i32 GetNumHardwareThreads()
{
	return d3Max(1, d3Min<i32>(DRX3D_MAX_THREAD_COUNT, sysconf(_SC_NPROCESSORS_ONLN)));
}

#endif

// ThreadSupportPosix helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class ThreadSupportPosix : public ThreadSupportInterface
{
public:
	struct ThreadStatus
	{
		i32 m_taskId;
		i32 m_commandId;
		i32 m_status;

		ThreadFunc m_userThreadFunc;
		uk m_userPtr;  //for taskDesc etc

		pthread_t thread;
		//each tread will wait until this signal to start its work
		sem_t* startSemaphore;
		CriticalSection* m_cs;
		// this is a copy of m_mainSemaphore,
		//each tread will signal once it is finished with its work
		sem_t* m_mainSemaphore;
		u64 threadUsed;
	};

private:
	typedef zu64 UINT64;

	AlignedObjectArray<ThreadStatus> m_activeThreadStatus;
	// m_mainSemaphoresemaphore will signal, if and how many threads are finished with their work
	sem_t* m_mainSemaphore;
	i32 m_numThreads;
	UINT64 m_startedThreadsMask;
	void startThreads(const ConstructionInfo& threadInfo);
	void stopThreads();
	i32 waitForResponse();
	CriticalSection* m_cs;
public:
	ThreadSupportPosix(const ConstructionInfo& threadConstructionInfo);
	virtual ~ThreadSupportPosix();

	virtual i32 getNumWorkerThreads() const DRX3D_OVERRIDE { return m_numThreads; }
	// TODO: return the number of logical processors sharing the first L3 cache
	virtual i32 getCacheFriendlyNumThreads() const DRX3D_OVERRIDE { return m_numThreads + 1; }
	// TODO: detect if CPU has hyperthreading enabled
	virtual i32 getLogicalToPhysicalCoreRatio() const DRX3D_OVERRIDE { return 1; }

	virtual void runTask(i32 threadIndex, uk userData) DRX3D_OVERRIDE;
	virtual void waitForAllTasks() DRX3D_OVERRIDE;

	virtual CriticalSection* createCriticalSection() DRX3D_OVERRIDE;
	virtual void deleteCriticalSection(CriticalSection* criticalSection) DRX3D_OVERRIDE;
};

#define checkPThreadFunction(returnValue)                                                                 \
	if (0 != returnValue)                                                                                 \
	{                                                                                                     \
		printf("Проблема PThread на строке %i в файле %s: %i %d\n", __LINE__, __FILE__, returnValue, errno); \
	}

// The number of threads should be equal to the number of available cores
// Todo: each worker should be linked to a single core, using SetThreadIdealProcessor.

ThreadSupportPosix::ThreadSupportPosix(const ConstructionInfo& threadConstructionInfo)
{
	m_cs = createCriticalSection();
	startThreads(threadConstructionInfo);
}

// cleanup/shutdown Libspe2
ThreadSupportPosix::~ThreadSupportPosix()
{
	stopThreads();
	deleteCriticalSection(m_cs);
	m_cs=0;
}

#if (defined(__APPLE__))
#define NAMED_SEMAPHORES
#endif

static sem_t* createSem(tukk baseName)
{
	static i32 semCount = 0;
#ifdef NAMED_SEMAPHORES
	/// Named semaphore begin
	char name[32];
	snprintf(name, 32, "/%8.s-%4.d-%4.4d", baseName, getpid(), semCount++);
	sem_t* tempSem = sem_open(name, O_CREAT, 0600, 0);

	if (tempSem != reinterpret_cast<sem_t*>(SEM_FAILED))
	{
		//        printf("Created \"%s\" Semaphore %p\n", name, tempSem);
	}
	else
	{
		//printf("Error creating Semaphore %d\n", errno);
		exit(-1);
	}
	/// Named semaphore end
#else
	sem_t* tempSem = new sem_t;
	checkPThreadFunction(sem_init(tempSem, 0, 0));
#endif
	return tempSem;
}

static void destroySem(sem_t* semaphore)
{
#ifdef NAMED_SEMAPHORES
	checkPThreadFunction(sem_close(semaphore));
#else
	checkPThreadFunction(sem_destroy(semaphore));
	delete semaphore;
#endif
}

static uk threadFunction(uk argument)
{
	ThreadSupportPosix::ThreadStatus* status = (ThreadSupportPosix::ThreadStatus*)argument;

	while (1)
	{
		checkPThreadFunction(sem_wait(status->startSemaphore));
		uk userPtr = status->m_userPtr;

		if (userPtr)
		{
			Assert(status->m_status);
			status->m_userThreadFunc(userPtr);
			status->m_cs->lock();
			status->m_status = 2;
			status->m_cs->unlock();
			checkPThreadFunction(sem_post(status->m_mainSemaphore));
			status->threadUsed++;
		}
		else
		{
			//exit Thread
			status->m_cs->lock();
			status->m_status = 3;
			status->m_cs->unlock();
			checkPThreadFunction(sem_post(status->m_mainSemaphore));
			break;
		}
	}

	return 0;
}

///send messages to SPUs
void ThreadSupportPosix::runTask(i32 threadIndex, uk userData)
{
	///we should spawn an SPU task here, and in 'waitForResponse' it should wait for response of the (one of) the first tasks that finished
	ThreadStatus& threadStatus = m_activeThreadStatus[threadIndex];
	Assert(threadIndex >= 0);
	Assert(threadIndex < m_activeThreadStatus.size());
	threadStatus.m_cs = m_cs;
	threadStatus.m_commandId = 1;
	threadStatus.m_status = 1;
	threadStatus.m_userPtr = userData;
	m_startedThreadsMask |= UINT64(1) << threadIndex;

	// fire event to start new task
	checkPThreadFunction(sem_post(threadStatus.startSemaphore));
}

///check for messages from SPUs
i32 ThreadSupportPosix::waitForResponse()
{
	///We should wait for (one of) the first tasks to finish (or other SPU messages), and report its response
	///A possible response can be 'yes, SPU handled it', or 'no, please do a PPU fallback'

	Assert(m_activeThreadStatus.size());

	// wait for any of the threads to finish
	checkPThreadFunction(sem_wait(m_mainSemaphore));
	// get at least one thread which has finished
	size_t last = -1;

	for (size_t t = 0; t < size_t(m_activeThreadStatus.size()); ++t)
	{
		m_cs->lock();
		bool hasFinished = (2 == m_activeThreadStatus[t].m_status);
		m_cs->unlock();
		if (hasFinished)
		{
			last = t;
			break;
		}
	}

	ThreadStatus& threadStatus = m_activeThreadStatus[last];

	Assert(threadStatus.m_status > 1);
	threadStatus.m_status = 0;

	// need to find an active spu
	Assert(last >= 0);
	m_startedThreadsMask &= ~(UINT64(1) << last);

	return last;
}

void ThreadSupportPosix::waitForAllTasks()
{
	while (m_startedThreadsMask)
	{
		waitForResponse();
	}
}

void ThreadSupportPosix::startThreads(const ConstructionInfo& threadConstructionInfo)
{
	m_numThreads = GetNumHardwareThreads() - 1;  // main thread exists already
	m_activeThreadStatus.resize(m_numThreads);
	m_startedThreadsMask = 0;

	m_mainSemaphore = createSem("main");
	//checkPThreadFunction(sem_wait(mainSemaphore));

	for (i32 i = 0; i < m_numThreads; i++)
	{
		ThreadStatus& threadStatus = m_activeThreadStatus[i];
		threadStatus.startSemaphore = createSem("threadLocal");
		threadStatus.m_userPtr = 0;
		threadStatus.m_cs = m_cs;
		threadStatus.m_taskId = i;
		threadStatus.m_commandId = 0;
		threadStatus.m_status = 0;
		threadStatus.m_mainSemaphore = m_mainSemaphore;
		threadStatus.m_userThreadFunc = threadConstructionInfo.m_userThreadFunc;
		threadStatus.threadUsed = 0;
		checkPThreadFunction(pthread_create(&threadStatus.thread, NULL, &threadFunction, (uk )&threadStatus));

	}
}

///tell the task scheduler we are done with the SPU tasks
void ThreadSupportPosix::stopThreads()
{
	for (size_t t = 0; t < size_t(m_activeThreadStatus.size()); ++t)
	{
		ThreadStatus& threadStatus = m_activeThreadStatus[t];

		threadStatus.m_userPtr = 0;
		checkPThreadFunction(sem_post(threadStatus.startSemaphore));
		checkPThreadFunction(sem_wait(m_mainSemaphore));

		checkPThreadFunction(pthread_join(threadStatus.thread, 0));
		destroySem(threadStatus.startSemaphore);
	}
	destroySem(m_mainSemaphore);
	m_activeThreadStatus.clear();
}

class CriticalSectionPosix : public CriticalSection
{
	pthread_mutex_t m_mutex;

public:
	CriticalSectionPosix()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}
	virtual ~CriticalSectionPosix()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	virtual void lock()
	{
		pthread_mutex_lock(&m_mutex);
	}
	virtual void unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}
};

CriticalSection* ThreadSupportPosix::createCriticalSection()
{
	return new CriticalSectionPosix();
}

void ThreadSupportPosix::deleteCriticalSection(CriticalSection* cs)
{
	delete cs;
}

ThreadSupportInterface* ThreadSupportInterface::create(const ConstructionInfo& info)
{
	return new ThreadSupportPosix(info);
}

#endif  // DRX3D_THREADSAFE && !defined( _WIN32 )
