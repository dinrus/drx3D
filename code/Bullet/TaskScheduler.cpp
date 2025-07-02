
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <stdio.h>
#include <algorithm>

#if DRX3D_THREADSAFE

#include <drx3D/Maths/Linear/TaskScheduler/ThreadSupportInterface.h>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif

typedef zu64 U64;
static i32k kCacheLineSize = 64;

void SpinPause()
{
#if defined(_WIN32)
	YieldProcessor();
#endif
}

struct WorkerThreadStatus
{
	enum Type
	{
		kInvalid,
		kWaitingForWork,
		kWorking,
		kSleeping,
	};
};

ATTRIBUTE_ALIGNED64(class)
WorkerThreadDirectives
{
	static i32k kMaxThreadCount = DRX3D_MAX_THREAD_COUNT;
	// directives for all worker threads packed into a single cacheline
	char m_threadDirs[kMaxThreadCount];

public:
	enum Type
	{
		kInvalid,
		kGoToSleep,         // go to sleep
		kStayAwakeButIdle,  // wait for not checking job queue
		kScanForJobs,       // actively scan job queue for jobs
	};
	WorkerThreadDirectives()
	{
		for (i32 i = 0; i < kMaxThreadCount; ++i)
		{
			m_threadDirs[i] = 0;
		}
	}

	Type getDirective(i32 threadId)
	{
		Assert(threadId < kMaxThreadCount);
		return static_cast<Type>(m_threadDirs[threadId]);
	}

	void setDirectiveByRange(i32 threadBegin, i32 threadEnd, Type dir)
	{
		Assert(threadBegin < threadEnd);
		Assert(threadEnd <= kMaxThreadCount);
		char dirChar = static_cast<char>(dir);
		for (i32 i = threadBegin; i < threadEnd; ++i)
		{
			m_threadDirs[i] = dirChar;
		}
	}
};

class JobQueue;

ATTRIBUTE_ALIGNED64(struct)
ThreadLocalStorage
{
	i32 m_threadId;
	WorkerThreadStatus::Type m_status;
	i32 m_numJobsFinished;
	SpinMutex m_mutex;
	Scalar m_sumResult;
	WorkerThreadDirectives* m_directive;
	JobQueue* m_queue;
	Clock* m_clock;
	u32 m_cooldownTime;
};

struct IJob
{
	virtual void executeJob(i32 threadId) = 0;
};

class ParallelForJob : public IJob
{
	const IParallelForBody* m_body;
	i32 m_begin;
	i32 m_end;

public:
	ParallelForJob(i32 iBegin, i32 iEnd, const IParallelForBody& body)
	{
		m_body = &body;
		m_begin = iBegin;
		m_end = iEnd;
	}
	virtual void executeJob(i32 threadId) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("executeJob");

		// call the functor body to do the work
		m_body->forLoop(m_begin, m_end);
	}
};

class ParallelSumJob : public IJob
{
	const IParallelSumBody* m_body;
	ThreadLocalStorage* m_threadLocalStoreArray;
	i32 m_begin;
	i32 m_end;

public:
	ParallelSumJob(i32 iBegin, i32 iEnd, const IParallelSumBody& body, ThreadLocalStorage* tls)
	{
		m_body = &body;
		m_threadLocalStoreArray = tls;
		m_begin = iBegin;
		m_end = iEnd;
	}
	virtual void executeJob(i32 threadId) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("executeJob");

		// call the functor body to do the work
		Scalar val = m_body->sumLoop(m_begin, m_end);
#if DRX3D_PARALLEL_SUM_DETERMINISTISM
		// by truncating bits of the result, we can make the parallelSum deterministic (at the expense of precision)
		const float TRUNC_SCALE = float(1 << 19);
		val = floor(val * TRUNC_SCALE + 0.5f) / TRUNC_SCALE;  // truncate some bits
#endif
		m_threadLocalStoreArray[threadId].m_sumResult += val;
	}
};

ATTRIBUTE_ALIGNED64(class)
JobQueue
{
	ThreadSupportInterface* m_threadSupport;
	CriticalSection* m_queueLock;
	SpinMutex m_mutex;

	AlignedObjectArray<IJob*> m_jobQueue;
	tuk m_jobMem;
	i32 m_jobMemSize;
	bool m_queueIsEmpty;
	i32 m_tailIndex;
	i32 m_headIndex;
	i32 m_allocSize;
	bool m_useSpinMutex;
	AlignedObjectArray<JobQueue*> m_neighborContexts;
	char m_cachePadding[kCacheLineSize];  // prevent false sharing

	void freeJobMem()
	{
		if (m_jobMem)
		{
			// free old
			AlignedFree(m_jobMem);
			m_jobMem = NULL;
		}
	}
	void resizeJobMem(i32 newSize)
	{
		if (newSize > m_jobMemSize)
		{
			freeJobMem();
			m_jobMem = static_cast<tuk>(AlignedAlloc(newSize, kCacheLineSize));
			m_jobMemSize = newSize;
		}
	}

public:
	JobQueue()
	{
		m_jobMem = NULL;
		m_jobMemSize = 0;
		m_threadSupport = NULL;
		m_queueLock = NULL;
		m_headIndex = 0;
		m_tailIndex = 0;
		m_useSpinMutex = false;
	}
	~JobQueue()
	{
		exit();
	}
	void exit()
	{
		freeJobMem();
		if (m_queueLock && m_threadSupport)
		{
			m_threadSupport->deleteCriticalSection(m_queueLock);
			m_queueLock = NULL;
			m_threadSupport = 0;
		}
	}

	void init(ThreadSupportInterface * threadSup, AlignedObjectArray<JobQueue> * contextArray)
	{
		m_threadSupport = threadSup;
		if (threadSup)
		{
			m_queueLock = m_threadSupport->createCriticalSection();
		}
		setupJobStealing(contextArray, contextArray->size());
	}
	void setupJobStealing(AlignedObjectArray<JobQueue> * contextArray, i32 numActiveContexts)
	{
		AlignedObjectArray<JobQueue>& contexts = *contextArray;
		i32 selfIndex = 0;
		for (i32 i = 0; i < contexts.size(); ++i)
		{
			if (this == &contexts[i])
			{
				selfIndex = i;
				break;
			}
		}
		i32 numNeighbors = d3Min(2, contexts.size() - 1);
		i32 neighborOffsets[] = {-1, 1, -2, 2, -3, 3};
		i32 numOffsets = sizeof(neighborOffsets) / sizeof(neighborOffsets[0]);
		m_neighborContexts.reserve(numNeighbors);
		m_neighborContexts.resizeNoInitialize(0);
		for (i32 i = 0; i < numOffsets && m_neighborContexts.size() < numNeighbors; i++)
		{
			i32 neighborIndex = selfIndex + neighborOffsets[i];
			if (neighborIndex >= 0 && neighborIndex < numActiveContexts)
			{
				m_neighborContexts.push_back(&contexts[neighborIndex]);
			}
		}
	}

	bool isQueueEmpty() const { return m_queueIsEmpty; }
	void lockQueue()
	{
		if (m_useSpinMutex)
		{
			m_mutex.lock();
		}
		else
		{
			m_queueLock->lock();
		}
	}
	void unlockQueue()
	{
		if (m_useSpinMutex)
		{
			m_mutex.unlock();
		}
		else
		{
			m_queueLock->unlock();
		}
	}
	void clearQueue(i32 jobCount, i32 jobSize)
	{
		lockQueue();
		m_headIndex = 0;
		m_tailIndex = 0;
		m_allocSize = 0;
		m_queueIsEmpty = true;
		i32 jobBufSize = jobSize * jobCount;
		// make sure we have enough memory allocated to store jobs
		if (jobBufSize > m_jobMemSize)
		{
			resizeJobMem(jobBufSize);
		}
		// make sure job queue is big enough
		if (jobCount > m_jobQueue.capacity())
		{
			m_jobQueue.reserve(jobCount);
		}
		unlockQueue();
		m_jobQueue.resizeNoInitialize(0);
	}
	uk allocJobMem(i32 jobSize)
	{
		Assert(m_jobMemSize >= (m_allocSize + jobSize));
		uk jobMem = &m_jobMem[m_allocSize];
		m_allocSize += jobSize;
		return jobMem;
	}
	void submitJob(IJob * job)
	{
		Assert(reinterpret_cast<tuk>(job) >= &m_jobMem[0] && reinterpret_cast<tuk>(job) < &m_jobMem[0] + m_allocSize);
		m_jobQueue.push_back(job);
		lockQueue();
		m_tailIndex++;
		m_queueIsEmpty = false;
		unlockQueue();
	}
	IJob* consumeJobFromOwnQueue()
	{
		if (m_queueIsEmpty)
		{
			// lock free path. even if this is taken erroneously it isn't harmful
			return NULL;
		}
		IJob* job = NULL;
		lockQueue();
		if (!m_queueIsEmpty)
		{
			job = m_jobQueue[m_headIndex++];
			Assert(reinterpret_cast<tuk>(job) >= &m_jobMem[0] && reinterpret_cast<tuk>(job) < &m_jobMem[0] + m_allocSize);
			if (m_headIndex == m_tailIndex)
			{
				m_queueIsEmpty = true;
			}
		}
		unlockQueue();
		return job;
	}
	IJob* consumeJob()
	{
		if (IJob* job = consumeJobFromOwnQueue())
		{
			return job;
		}
		// own queue is empty, try to steal from neighbor
		for (i32 i = 0; i < m_neighborContexts.size(); ++i)
		{
			JobQueue* otherContext = m_neighborContexts[i];
			if (IJob* job = otherContext->consumeJobFromOwnQueue())
			{
				return job;
			}
		}
		return NULL;
	}
};

static void WorkerThreadFunc(uk userPtr)
{
	DRX3D_PROFILE("WorkerThreadFunc");
	ThreadLocalStorage* localStorage = (ThreadLocalStorage*)userPtr;
	JobQueue* jobQueue = localStorage->m_queue;

	bool shouldSleep = false;
	i32 threadId = localStorage->m_threadId;
	while (!shouldSleep)
	{
		// do work
		localStorage->m_mutex.lock();
		while (IJob* job = jobQueue->consumeJob())
		{
			localStorage->m_status = WorkerThreadStatus::kWorking;
			job->executeJob(threadId);
			localStorage->m_numJobsFinished++;
		}
		localStorage->m_status = WorkerThreadStatus::kWaitingForWork;
		localStorage->m_mutex.unlock();
		U64 clockStart = localStorage->m_clock->getTimeMicroseconds();
		// while queue is empty,
		while (jobQueue->isQueueEmpty())
		{
			// todo: spin wait a bit to avoid hammering the empty queue
			SpinPause();
			if (localStorage->m_directive->getDirective(threadId) == WorkerThreadDirectives::kGoToSleep)
			{
				shouldSleep = true;
				break;
			}
			// if jobs are incoming,
			if (localStorage->m_directive->getDirective(threadId) == WorkerThreadDirectives::kScanForJobs)
			{
				clockStart = localStorage->m_clock->getTimeMicroseconds();  // reset clock
			}
			else
			{
				for (i32 i = 0; i < 50; ++i)
				{
					SpinPause();
					SpinPause();
					SpinPause();
					SpinPause();
					if (localStorage->m_directive->getDirective(threadId) == WorkerThreadDirectives::kScanForJobs || !jobQueue->isQueueEmpty())
					{
						break;
					}
				}
				// if no jobs incoming and queue has been empty for the cooldown time, sleep
				U64 timeElapsed = localStorage->m_clock->getTimeMicroseconds() - clockStart;
				if (timeElapsed > localStorage->m_cooldownTime)
				{
					shouldSleep = true;
					break;
				}
			}
		}
	}
	{
		DRX3D_PROFILE("sleep");
		// go sleep
		localStorage->m_mutex.lock();
		localStorage->m_status = WorkerThreadStatus::kSleeping;
		localStorage->m_mutex.unlock();
	}
}

class TaskSchedulerDefault : public ITaskScheduler
{
	ThreadSupportInterface* m_threadSupport;
	WorkerThreadDirectives* m_workerDirective;
	AlignedObjectArray<JobQueue> m_jobQueues;
	AlignedObjectArray<JobQueue*> m_perThreadJobQueues;
	AlignedObjectArray<ThreadLocalStorage> m_threadLocalStorage;
	SpinMutex m_antiNestingLock;  // prevent nested parallel-for
	Clock m_clock;
	i32 m_numThreads;
	i32 m_numWorkerThreads;
	i32 m_numActiveJobQueues;
	i32 m_maxNumThreads;
	i32 m_numJobs;
	static i32k kFirstWorkerThreadId = 1;

public:
	TaskSchedulerDefault() : ITaskScheduler("ThreadSupport")
	{
		m_threadSupport = NULL;
		m_workerDirective = NULL;
	}

	virtual ~TaskSchedulerDefault()
	{
		waitForWorkersToSleep();

		for (i32 i = 0; i < m_jobQueues.size(); ++i)
		{
			m_jobQueues[i].exit();
		}

		if (m_threadSupport)
		{
			delete m_threadSupport;
			m_threadSupport = NULL;
		}
		if (m_workerDirective)
		{
			AlignedFree(m_workerDirective);
			m_workerDirective = NULL;
		}
	}

	void init()
	{
		ThreadSupportInterface::ConstructionInfo constructionInfo("TaskScheduler", WorkerThreadFunc);
		m_threadSupport = ThreadSupportInterface::create(constructionInfo);
		m_workerDirective = static_cast<WorkerThreadDirectives*>(AlignedAlloc(sizeof(*m_workerDirective), 64));

		m_numWorkerThreads = m_threadSupport->getNumWorkerThreads();
		m_maxNumThreads = m_threadSupport->getNumWorkerThreads() + 1;
		m_numThreads = m_maxNumThreads;
		// ideal to have one job queue for each physical processor (except for the main thread which needs no queue)
		i32 numThreadsPerQueue = m_threadSupport->getLogicalToPhysicalCoreRatio();
		i32 numJobQueues = (numThreadsPerQueue == 1) ? (m_maxNumThreads - 1) : (m_maxNumThreads / numThreadsPerQueue);
		m_jobQueues.resize(numJobQueues);
		m_numActiveJobQueues = numJobQueues;
		for (i32 i = 0; i < m_jobQueues.size(); ++i)
		{
			m_jobQueues[i].init(m_threadSupport, &m_jobQueues);
		}
		m_perThreadJobQueues.resize(m_numThreads);
		for (i32 i = 0; i < m_numThreads; i++)
		{
			JobQueue* jq = NULL;
			// only worker threads get a job queue
			if (i > 0)
			{
				if (numThreadsPerQueue == 1)
				{
					// one queue per worker thread
					jq = &m_jobQueues[i - kFirstWorkerThreadId];
				}
				else
				{
					// 2 threads share each queue
					jq = &m_jobQueues[i / numThreadsPerQueue];
				}
			}
			m_perThreadJobQueues[i] = jq;
		}
		m_threadLocalStorage.resize(m_numThreads);
		for (i32 i = 0; i < m_numThreads; i++)
		{
			ThreadLocalStorage& storage = m_threadLocalStorage[i];
			storage.m_threadId = i;
			storage.m_directive = m_workerDirective;
			storage.m_status = WorkerThreadStatus::kSleeping;
			storage.m_cooldownTime = 100;  // 100 microseconds, threads go to sleep after this long if they have nothing to do
			storage.m_clock = &m_clock;
			storage.m_queue = m_perThreadJobQueues[i];
		}
		setWorkerDirectives(WorkerThreadDirectives::kGoToSleep);  // no work for them yet
		setNumThreads(m_threadSupport->getCacheFriendlyNumThreads());
	}

	void setWorkerDirectives(WorkerThreadDirectives::Type dir)
	{
		m_workerDirective->setDirectiveByRange(kFirstWorkerThreadId, m_numThreads, dir);
	}

	virtual i32 getMaxNumThreads() const DRX3D_OVERRIDE
	{
		return m_maxNumThreads;
	}

	virtual i32 getNumThreads() const DRX3D_OVERRIDE
	{
		return m_numThreads;
	}

	virtual void setNumThreads(i32 numThreads) DRX3D_OVERRIDE
	{
		m_numThreads = d3Max(d3Min(numThreads, i32(m_maxNumThreads)), 1);
		m_numWorkerThreads = m_numThreads - 1;
		m_numActiveJobQueues = 0;
		// if there is at least 1 worker,
		if (m_numWorkerThreads > 0)
		{
			// re-setup job stealing between queues to avoid attempting to steal from an inactive job queue
			JobQueue* lastActiveContext = m_perThreadJobQueues[m_numThreads - 1];
			i32 iLastActiveContext = lastActiveContext - &m_jobQueues[0];
			m_numActiveJobQueues = iLastActiveContext + 1;
			for (i32 i = 0; i < m_jobQueues.size(); ++i)
			{
				m_jobQueues[i].setupJobStealing(&m_jobQueues, m_numActiveJobQueues);
			}
		}
		m_workerDirective->setDirectiveByRange(m_numThreads, DRX3D_MAX_THREAD_COUNT, WorkerThreadDirectives::kGoToSleep);
	}

	void waitJobs()
	{
		DRX3D_PROFILE("waitJobs");
		// have the main thread work until the job queues are empty
		i32 numMainThreadJobsFinished = 0;
		for (i32 i = 0; i < m_numActiveJobQueues; ++i)
		{
			while (IJob* job = m_jobQueues[i].consumeJob())
			{
				job->executeJob(0);
				numMainThreadJobsFinished++;
			}
		}

		// done with jobs for now, tell workers to rest (but not sleep)
		setWorkerDirectives(WorkerThreadDirectives::kStayAwakeButIdle);

		U64 clockStart = m_clock.getTimeMicroseconds();
		// wait for workers to finish any jobs in progress
		while (true)
		{
			i32 numWorkerJobsFinished = 0;
			for (i32 iThread = kFirstWorkerThreadId; iThread < m_numThreads; ++iThread)
			{
				ThreadLocalStorage* storage = &m_threadLocalStorage[iThread];
				storage->m_mutex.lock();
				numWorkerJobsFinished += storage->m_numJobsFinished;
				storage->m_mutex.unlock();
			}
			if (numWorkerJobsFinished + numMainThreadJobsFinished == m_numJobs)
			{
				break;
			}
			U64 timeElapsed = m_clock.getTimeMicroseconds() - clockStart;
			Assert(timeElapsed < 1000);
			if (timeElapsed > 100000)
			{
				break;
			}
			SpinPause();
		}
	}

	void wakeWorkers(i32 numWorkersToWake)
	{
		DRX3D_PROFILE("wakeWorkers");
		Assert(m_workerDirective->getDirective(1) == WorkerThreadDirectives::kScanForJobs);
		i32 numDesiredWorkers = d3Min(numWorkersToWake, m_numWorkerThreads);
		i32 numActiveWorkers = 0;
		for (i32 iWorker = 0; iWorker < m_numWorkerThreads; ++iWorker)
		{
			// note this count of active workers is not necessarily totally reliable, because a worker thread could be
			// just about to put itself to sleep. So we may on occasion fail to wake up all the workers. It should be rare.
			ThreadLocalStorage& storage = m_threadLocalStorage[kFirstWorkerThreadId + iWorker];
			if (storage.m_status != WorkerThreadStatus::kSleeping)
			{
				numActiveWorkers++;
			}
		}
		for (i32 iWorker = 0; iWorker < m_numWorkerThreads && numActiveWorkers < numDesiredWorkers; ++iWorker)
		{
			ThreadLocalStorage& storage = m_threadLocalStorage[kFirstWorkerThreadId + iWorker];
			if (storage.m_status == WorkerThreadStatus::kSleeping)
			{
				m_threadSupport->runTask(iWorker, &storage);
				numActiveWorkers++;
			}
		}
	}

	void waitForWorkersToSleep()
	{
		DRX3D_PROFILE("waitForWorkersToSleep");
		setWorkerDirectives(WorkerThreadDirectives::kGoToSleep);
		m_threadSupport->waitForAllTasks();
		for (i32 i = kFirstWorkerThreadId; i < m_numThreads; i++)
		{
			ThreadLocalStorage& storage = m_threadLocalStorage[i];
			Assert(storage.m_status == WorkerThreadStatus::kSleeping);
		}
	}

	virtual void sleepWorkerThreadsHint() DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("sleepWorkerThreadsHint");
		// hint the task scheduler that we may not be using these threads for a little while
		setWorkerDirectives(WorkerThreadDirectives::kGoToSleep);
	}

	void prepareWorkerThreads()
	{
		for (i32 i = kFirstWorkerThreadId; i < m_numThreads; ++i)
		{
			ThreadLocalStorage& storage = m_threadLocalStorage[i];
			storage.m_mutex.lock();
			storage.m_numJobsFinished = 0;
			storage.m_mutex.unlock();
		}
		setWorkerDirectives(WorkerThreadDirectives::kScanForJobs);
	}

	virtual void parallelFor(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelForBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelFor_ThreadSupport");
		Assert(iEnd >= iBegin);
		Assert(grainSize >= 1);
		i32 iterationCount = iEnd - iBegin;
		if (iterationCount > grainSize && m_numWorkerThreads > 0 && m_antiNestingLock.tryLock())
		{
			typedef ParallelForJob JobType;
			i32 jobCount = (iterationCount + grainSize - 1) / grainSize;
			m_numJobs = jobCount;
			Assert(jobCount >= 2);  // need more than one job for multithreading
			i32 jobSize = sizeof(JobType);

			for (i32 i = 0; i < m_numActiveJobQueues; ++i)
			{
				m_jobQueues[i].clearQueue(jobCount, jobSize);
			}
			// prepare worker threads for incoming work
			prepareWorkerThreads();
			// submit all of the jobs
			i32 iJob = 0;
			i32 iThread = kFirstWorkerThreadId;  // first worker thread
			for (i32 i = iBegin; i < iEnd; i += grainSize)
			{
				Assert(iJob < jobCount);
				i32 iE = d3Min(i + grainSize, iEnd);
				JobQueue* jq = m_perThreadJobQueues[iThread];
				Assert(jq);
				Assert((jq - &m_jobQueues[0]) < m_numActiveJobQueues);
				uk jobMem = jq->allocJobMem(jobSize);
				JobType* job = new (jobMem) ParallelForJob(i, iE, body);  // placement new
				jq->submitJob(job);
				iJob++;
				iThread++;
				if (iThread >= m_numThreads)
				{
					iThread = kFirstWorkerThreadId;  // first worker thread
				}
			}
			wakeWorkers(jobCount - 1);

			// put the main thread to work on emptying the job queue and then wait for all workers to finish
			waitJobs();
			m_antiNestingLock.unlock();
		}
		else
		{
			DRX3D_PROFILE("parallelFor_mainThread");
			// just run on main thread
			body.forLoop(iBegin, iEnd);
		}
	}
	virtual Scalar parallelSum(i32 iBegin, i32 iEnd, i32 grainSize, const IParallelSumBody& body) DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("parallelSum_ThreadSupport");
		Assert(iEnd >= iBegin);
		Assert(grainSize >= 1);
		i32 iterationCount = iEnd - iBegin;
		if (iterationCount > grainSize && m_numWorkerThreads > 0 && m_antiNestingLock.tryLock())
		{
			typedef ParallelSumJob JobType;
			i32 jobCount = (iterationCount + grainSize - 1) / grainSize;
			m_numJobs = jobCount;
			Assert(jobCount >= 2);  // need more than one job for multithreading
			i32 jobSize = sizeof(JobType);
			for (i32 i = 0; i < m_numActiveJobQueues; ++i)
			{
				m_jobQueues[i].clearQueue(jobCount, jobSize);
			}

			// initialize summation
			for (i32 iThread = 0; iThread < m_numThreads; ++iThread)
			{
				m_threadLocalStorage[iThread].m_sumResult = Scalar(0);
			}

			// prepare worker threads for incoming work
			prepareWorkerThreads();
			// submit all of the jobs
			i32 iJob = 0;
			i32 iThread = kFirstWorkerThreadId;  // first worker thread
			for (i32 i = iBegin; i < iEnd; i += grainSize)
			{
				Assert(iJob < jobCount);
				i32 iE = d3Min(i + grainSize, iEnd);
				JobQueue* jq = m_perThreadJobQueues[iThread];
				Assert(jq);
				Assert((jq - &m_jobQueues[0]) < m_numActiveJobQueues);
				uk jobMem = jq->allocJobMem(jobSize);
				JobType* job = new (jobMem) ParallelSumJob(i, iE, body, &m_threadLocalStorage[0]);  // placement new
				jq->submitJob(job);
				iJob++;
				iThread++;
				if (iThread >= m_numThreads)
				{
					iThread = kFirstWorkerThreadId;  // first worker thread
				}
			}
			wakeWorkers(jobCount - 1);

			// put the main thread to work on emptying the job queue and then wait for all workers to finish
			waitJobs();

			// add up all the thread sums
			Scalar sum = Scalar(0);
			for (i32 iThread = 0; iThread < m_numThreads; ++iThread)
			{
				sum += m_threadLocalStorage[iThread].m_sumResult;
			}
			m_antiNestingLock.unlock();
			return sum;
		}
		else
		{
			DRX3D_PROFILE("parallelSum_mainThread");
			// just run on main thread
			return body.sumLoop(iBegin, iEnd);
		}
	}
};

ITaskScheduler* CreateDefaultTaskScheduler()
{
	TaskSchedulerDefault* ts = new TaskSchedulerDefault();
	ts->init();
	return ts;
}

#else  // #if DRX3D_THREADSAFE

ITaskScheduler* CreateDefaultTaskScheduler()
{
	return NULL;
}

#endif  // #else // #if DRX3D_THREADSAFE
