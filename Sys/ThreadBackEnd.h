// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ThreadBackEnd.h
//  Version:     v1.00
//  Created:     07/05/2011 by Christopher Bolte
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////

#ifndef THREAD_BACKEND_H_
#define THREAD_BACKEND_H_

#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/Sys/JobStructs.h>

#include <drx3D/CoreX/Thread/IThreadUpr.h>

#if DRX_PLATFORM_DURANGO
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#define JOB_SPIN_DURING_IDLE
#endif

namespace JobUpr
{
class CJobUpr;
class CWorkerBackEndProfiler;
}

namespace JobUpr {
class CJobUpr;

namespace ThreadBackEnd {
namespace detail {
// stack size for backend worker threads
enum { eStackSize = 256 * 1024 };

class CWaitForJobObject
{
public:
	CWaitForJobObject(u32 nMaxCount) :
#if defined(JOB_SPIN_DURING_IDLE)
		m_nCounter(0)
#else
		m_Semaphore(nMaxCount)
#endif
	{}

	void SignalNewJob()
	{
#if defined(JOB_SPIN_DURING_IDLE)
		i32 nCount = ~0;
		do
		{
			nCount = *const_cast< i32*>(&m_nCounter);
		}
		while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nCounter), nCount + 1, nCount) != nCount);
#else
		m_Semaphore.Release();
#endif
	}

	bool TryGetJob()
	{
#if DRX_PLATFORM_DURANGO
		i32 nCount = *const_cast< i32*>(&m_nCounter);
		if (nCount > 0)
		{
			if (DrxInterlockedCompareExchange(alias_cast< long*>(&m_nCounter), nCount - 1, nCount) == nCount)
				return true;
		}
		return false;
#else
		return false;
#endif
	}
	void WaitForNewJob(u32 nWorkerID)
	{
#if defined(JOB_SPIN_DURING_IDLE)
		i32 nCount = ~0;

	#if DRX_PLATFORM_DURANGO
		UAsyncDipState nCurrentState;
		UAsyncDipState nNewState;

		do      // mark as idle
		{
			nCurrentState.nValue = *const_cast< u32*>(&gEnv->mAsyncDipState.nValue);
			nNewState = nCurrentState;
			nNewState.nWorker_Idle |= 1 << nWorkerID;
			if (DrxInterlockedCompareExchange(( long*)&gEnv->mAsyncDipState.nValue, nNewState.nValue, nCurrentState.nValue) == nCurrentState.nValue)
				break;
		}
		while (true);
retry:
	#endif  // DRX_PLATFORM_DURANGO

		do
		{

	#if DRX_PLATFORM_DURANGO
			nCurrentState.nValue = *const_cast< u32*>(&gEnv->mAsyncDipState.nValue);
			if (nCurrentState.nQueueGuard == 0 && nCurrentState.nNumJobs > 0)
			{
				SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

				nNewState = nCurrentState;
				nNewState.nQueueGuard = 1;
				if (DrxInterlockedCompareExchange(( long*)&gEnv->mAsyncDipState.nValue, nNewState.nValue, nCurrentState.nValue) == nCurrentState.nValue)
				{
ExecuteAsyncDip:
					gEnv->pRenderer->ExecuteAsyncDIP();

					// clear guard variable
					do
					{
						nCurrentState.nValue = *const_cast< u32*>(&gEnv->mAsyncDipState.nValue);
						nNewState = nCurrentState;
						nNewState.nQueueGuard = 0;

						// if jobs were added in the meantime, continue executing those
						if (nCurrentState.nNumJobs > 0)
							goto ExecuteAsyncDip;

						if (DrxInterlockedCompareExchange(( long*)&gEnv->mAsyncDipState.nValue, nNewState.nValue, nCurrentState.nValue) == nCurrentState.nValue)
							break;
					}
					while (true);
				}     // else another thread must have gotten the guard var, go back to IDLE priority spinning
				SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
			}
	#endif    // DRX_PLATFORM_DURANGO

			nCount = *const_cast< i32*>(&m_nCounter);
			if (nCount > 0)
			{
				if (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nCounter), nCount - 1, nCount) == nCount)
					break;
			}

			YieldProcessor();
			YieldProcessor();
			YieldProcessor();
			YieldProcessor();
			YieldProcessor();
			YieldProcessor();
			YieldProcessor();
			YieldProcessor();

			SwitchToThread();

		}
		while (true);

	#if DRX_PLATFORM_DURANGO
		do      // mark as busy
		{
			nCurrentState.nValue = *const_cast< u32*>(&gEnv->mAsyncDipState.nValue);
			nNewState = nCurrentState;
			nNewState.nWorker_Idle &= ~(1 << nWorkerID);

			// new job was submitted while we were leaving the loop
			// try to get the guard again
			if (nCurrentState.nQueueGuard == 0 && nCurrentState.nNumJobs > 0)
			{
				do      // increment job counter again, to allow another thread to take this job
				{
					nCount = *const_cast< i32*>(&m_nCounter);
					if (DrxInterlockedCompareExchange(alias_cast< long*>(&m_nCounter), nCount + 1, nCount) == nCount)
						break;
				}
				while (true);

				goto retry;
			}

			if (DrxInterlockedCompareExchange(( long*)&gEnv->mAsyncDipState.nValue, nNewState.nValue, nCurrentState.nValue) == nCurrentState.nValue)
				break;

		}
		while (true);
	#endif  // DRX_PLATFORM_DURANGO
#else
		m_Semaphore.Acquire();
#endif
	}
private:
#if defined(JOB_SPIN_DURING_IDLE)
	 i32 m_nCounter;
#else
	DrxFastSemaphore m_Semaphore;
#endif
};

} // namespace detail
  // forward declarations
class CThreadBackEnd;

// class to represent a worker thread for the PC backend
class CThreadBackEndWorkerThread : public IThread
{
public:
	CThreadBackEndWorkerThread(CThreadBackEnd* pThreadBackend, detail::CWaitForJobObject& rSemaphore, JobUpr::SJobQueue_ThreadBackEnd& rJobQueue, u32 nId);
	~CThreadBackEndWorkerThread();

	// Start accepting work on thread
	virtual void ThreadEntry();

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();
private:
	void DoWorkProducerConsumerQueue(SInfoBlock& rInfoBlock);

	u32                               m_nId;                   // id of the worker thread
	 bool                        m_bStop;
	detail::CWaitForJobObject&           m_rSemaphore;
	JobUpr::SJobQueue_ThreadBackEnd& m_rJobQueue;
	CThreadBackEnd*                      m_pThreadBackend;
};

// the implementation of the PC backend
// has n-worker threads which use atomic operations to pull from the job queue
// and uses a semaphore to signal the workers if there is work required
class CThreadBackEnd : public IBackend
{
public:
	CThreadBackEnd();
	virtual ~CThreadBackEnd();

	bool           Init(u32 nSysMaxWorker);
	bool           ShutDown();
	void           Update() {}

	virtual void   AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock);

	virtual u32 GetNumWorkerThreads() const { return m_nNumWorkerThreads; }

	// returns the index to use for the frame profiler
	u32 GetCurrentFrameBufferIndex() const;

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	JobUpr::IWorkerBackEndProfiler* GetBackEndWorkerProfiler() const { return m_pBackEndWorkerProfiler; }
#endif

private:
	friend class JobUpr::CJobUpr;

	JobUpr::SJobQueue_ThreadBackEnd      m_JobQueue;              // job queue node where jobs are pushed into and from
	detail::CWaitForJobObject                m_Semaphore;             // semaphore to count available jobs, to allow the workers to go sleeping instead of spinning when no work is required
	std::vector<CThreadBackEndWorkerThread*> m_arrWorkerThreads;      // array of worker threads
	u8 m_nNumWorkerThreads;                                        // number of worker threads

	// members required for profiling jobs in the frame profiler
#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	JobUpr::IWorkerBackEndProfiler* m_pBackEndWorkerProfiler;
#endif
};

} // namespace ThreadBackEnd
} // namespace JobUpr

#endif // THREAD_BACKEND_H_
