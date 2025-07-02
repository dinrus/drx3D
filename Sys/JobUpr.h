// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*
   definitions for job manager
   singleton implementation
 */

#ifndef __JOB_MANAGER_H__
#define __JOB_MANAGER_H__
#pragma once

#define JOBSYSTEM_INVOKER_COUNT (128)

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Thread/DrxAtomics.h>

#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/Sys/JobStructs.h>

#include <map>

///////////////////////////////////////////////////////////////////////////////
namespace JobUpr
{
/////////////////////////////////////////////////////////////////////////////
// Util Functions to get access Thread local data (needs to be unique per worker thread)
namespace detail {
// function to manipulate the per thread fallback job freelist
void                    PushToFallbackJobList(JobUpr::SInfoBlock* pInfoBlock);
JobUpr::SInfoBlock* PopFromFallbackJobList();
constexpr i32    GetFallbackJobListSize() { return 4096; }

// functions to access the per thread worker thread id
void   SetWorkerThreadId(u32 nWorkerThreadId);
u32 GetWorkerThreadId();

} // namespace detail

// Tracks CPU/PPU worker thread(s) utilization and job execution time per frame
class CWorkerBackEndProfiler : public IWorkerBackEndProfiler
{
public:
	CWorkerBackEndProfiler();
	virtual ~CWorkerBackEndProfiler();

	virtual void Init(u16k numWorkers);

	// Update the profiler at the beginning of the sample period
	virtual void Update();
	virtual void Update(u32k curTimeSample);

	// Register a job with the profiler
	virtual void RegisterJob(u32k jobId, tukk jobName);

	// Record execution information for a registered job
	virtual void RecordJob(u16k profileIndex, u8k workerId, u32k jobId, u32k runTimeMicroSec);

	// Get worker frame stats for the JobUpr::detail::eJOB_FRAME_STATS - 1 frame
	virtual void GetFrameStats(JobUpr::CWorkerFrameStats& rStats) const;
	virtual void GetFrameStats(TJobFrameStatsContainer& rJobStats, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const;
	virtual void GetFrameStats(JobUpr::CWorkerFrameStats& rStats, TJobFrameStatsContainer& rJobStats, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const;

	// Get worker frame stats summary
	virtual void GetFrameStatsSummary(SWorkerFrameStatsSummary& rStats) const;
	virtual void GetFrameStatsSummary(SJobFrameStatsSummary& rStats) const;

	// Returns the index of the active multi-buffered profile data
	virtual u16 GetProfileIndex() const;

	// Get the number of workers tracked
	virtual u32 GetNumWorkers() const;

protected:
	void GetWorkerStats(u8k nBufferIndex, JobUpr::CWorkerFrameStats& rWorkerStats) const;
	void GetJobStats(u8k nBufferIndex, TJobFrameStatsContainer& rJobStatsContainer, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const;
	void ResetWorkerStats(u8k nBufferIndex, u32k curTimeSample);
	void ResetJobStats(u8k nBufferIndex);

protected:
	struct SJobStatsInfo
	{
		JobUpr::SJobFrameStats m_pJobStats[JobUpr::detail::eJOB_FRAME_STATS * JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS];    // Array of job stats (multi buffered)
	};

	struct SWorkerStatsInfo
	{
		u32                    m_nStartTime[JobUpr::detail::eJOB_FRAME_STATS]; // Start Time of sample period (multi buffered)
		u32                    m_nEndTime[JobUpr::detail::eJOB_FRAME_STATS];   // End Time of sample period (multi buffered)
		u16                    m_nNumWorkers;                                      // Number of workers tracked
		JobUpr::SWorkerStats* m_pWorkerStats;                                     // Array of worker stats for each worker (multi buffered)
	};

protected:
	u8            m_nCurBufIndex;      // Current buffer index [0,(JobUpr::detail::eJOB_FRAME_STATS-1)]
	SJobStatsInfo    m_JobStatsInfo;      // Information about all job activities
	SWorkerStatsInfo m_WorkerStatsInfo;   // Information about each worker's utilization
};

class CJobLambda : public CJobBase
{
public:
	CJobLambda(tukk jobName, const std::function<void()>& lambda)
	{
		m_jobHandle = gEnv->GetJobUpr()->GetJobHandle(jobName, &Invoke);
		m_JobDelegator.SetJobParamData(0);
		m_JobDelegator.SetParamDataSize(0);
		m_JobDelegator.SetDelegator(Invoke);
		m_JobDelegator.SetLambda(lambda);
		SetJobProgramData(m_jobHandle);
	}
	void SetPriorityLevel(u32 nPriorityLevel)
	{
		m_JobDelegator.SetPriorityLevel(nPriorityLevel);
	}
	void SetBlocking()
	{
		m_JobDelegator.SetBlocking();
	}

private:
	static void Invoke(uk p)
	{
	}

public:
	TJobHandle m_jobHandle;
};

// singleton managing the job queues
class DRX_ALIGN(128) CJobUpr: public IJobUpr, public IInputEventListener
{
public:
	// singleton stuff
	static CJobUpr* Instance();

	//destructor
	virtual ~CJobUpr()
	{
		delete m_pThreadBackEnd;
		delete m_pFallBackBackEnd;
		DrxAlignedDelete(m_pBlockingBackEnd);
	}

	virtual void Init(u32 nSysMaxWorker) override;

	// wait for a job, preempt the calling thread if the job is not done yet
	virtual const bool WaitForJob(JobUpr::SJobState & rJobState) const override;

	//adds a job
	virtual void AddJob(JobUpr::CJobDelegator & crJob, const JobUpr::TJobHandle cJobHandle) override;

	virtual void AddLambdaJob(tukk jobName, const std::function<void()> &lambdaCallback, TPriorityLevel priority = JobUpr::eRegularPriority, SJobState * pJobState = nullptr) override;

	//obtain job handle from name
	virtual const JobUpr::TJobHandle GetJobHandle(tukk cpJobName, u32k cStrLen, JobUpr::Invoker pInvoker) override;
	virtual const JobUpr::TJobHandle GetJobHandle(tukk cpJobName, JobUpr::Invoker pInvoker) override
	{
		return GetJobHandle(cpJobName, strlen(cpJobName), pInvoker);
	}

	virtual JobUpr::IBackend* GetBackEnd(JobUpr::EBackEndType backEndType) override
	{
		switch (backEndType)
		{
		case eBET_Thread:
			return m_pThreadBackEnd;
		case eBET_Blocking:
			return m_pBlockingBackEnd;
		case eBET_Fallback:
			return m_pFallBackBackEnd;
		default:
			DRX_ASSERT_MESSAGE(0, "Unsupported EBackEndType encountered.");
			__debugbreak();
			return 0;
		}
		;

		return 0;
	}

	//shuts down job manager
	virtual void ShutDown() override;

	virtual bool InvokeAsJob(tukk cpJobName) const override;
	virtual bool InvokeAsJob(const JobUpr::TJobHandle cJobHandle) const override;

	virtual void SetJobFilter(tukk pFilter) override
	{
		m_pJobFilter = pFilter;
	}

	virtual void SetJobSystemEnabled(i32 nEnable) override
	{
		m_nJobSystemEnabled = nEnable;
	}

	virtual void PushProfilingMarker(tukk pName) override;
	virtual void PopProfilingMarker() override;

	// move to right place
	enum { nMarkerEntries = 1024 };
	struct SMarker
	{
		typedef DrxFixedStringT<64> TMarkerString;
		enum MarkerType { PUSH_MARKER, POP_MARKER };

		SMarker() {}
		SMarker(MarkerType _type, CTimeValue _time, bool _bIsMainThread) : type(_type), time(_time), bIsMainThread(_bIsMainThread) {}
		SMarker(MarkerType _type, tukk pName, CTimeValue _time, bool _bIsMainThread) : type(_type), marker(pName), time(_time), bIsMainThread(_bIsMainThread) {}

		TMarkerString marker;
		MarkerType    type;
		CTimeValue    time;
		bool          bIsMainThread;
	};
	u32 m_nMainThreadMarkerIndex[SJobProfilingDataContainer::nCapturedFrames];
	u32 m_nRenderThreadMarkerIndex[SJobProfilingDataContainer::nCapturedFrames];
	SMarker m_arrMainThreadMarker[SJobProfilingDataContainer::nCapturedFrames][nMarkerEntries];
	SMarker m_arrRenderThreadMarker[SJobProfilingDataContainer::nCapturedFrames][nMarkerEntries];
	// move to right place

	//copy the job parameter into the jobinfo  structure
	static void CopyJobParameter(u32k cJobParamSize, uk pDest, ukk pSrc);

	u32 GetWorkerThreadId() const override;

	virtual JobUpr::SJobProfilingData* GetProfilingData(u16 nProfilerIndex) override;
	virtual u16 ReserveProfilingData() override;

	void Update(i32 nJobSystemProfiler) override;

	virtual void SetFrameStartTime(const CTimeValue &rFrameStartTime) override;

	ColorB GetRegionColor(SMarker::TMarkerString marker);
	JobUpr::Invoker GetJobInvoker(u32 nIdx)
	{
		assert(nIdx < m_nJobInvokerIdx);
		assert(nIdx < JOBSYSTEM_INVOKER_COUNT);
		return m_arrJobInvokers[nIdx];
	}
	virtual u32 GetNumWorkerThreads() const override
	{
		return m_pThreadBackEnd ? m_pThreadBackEnd->GetNumWorkerThreads() : 0;
	}

	// get a free semaphore from the jobmanager pool
	virtual JobUpr::TSemaphoreHandle AllocateSemaphore( ukk pOwner) override;

	// return a semaphore to the jobmanager pool
	virtual void DeallocateSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) override;

	// increase the refcounter of a semaphore, but only if it is > 0, else returns false
	virtual bool AddRefSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) override;

	// 'allocate' a semaphore in the jobmanager, and return the index of it
	virtual SJobFinishedConditionVariable* GetSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) override;

	virtual void DumpJobList() override;

	virtual bool OnInputEvent(const SInputEvent &event) override;

	void IncreaseRunJobs();
	void IncreaseRunFallbackJobs();

	void AddBlockingFallbackJob(JobUpr::SInfoBlock * pInfoBlock, u32 nWorkerThreadID);

	tukk GetJobName(JobUpr::Invoker invoker);

private:
	static ColorB GenerateColorBasedOnName(tukk name);

	DrxCriticalSection m_JobUprLock;                             // lock to protect non-performance critical parts of the jobmanager
	JobUpr::Invoker m_arrJobInvokers[JOBSYSTEM_INVOKER_COUNT];   // support 128 jobs for now
	u32 m_nJobInvokerIdx;

	tukk m_pJobFilter;
	i32 m_nJobSystemEnabled;                                // should the job system be used
	i32 m_bJobSystemProfilerEnabled;                        // should the job system profiler be enabled
	bool m_bJobSystemProfilerPaused;                        // should the job system profiler be paused

	bool m_Initialized;                                     //true if JobUpr have been initialized

	IBackend* m_pFallBackBackEnd;               // Backend for development, jobs are executed in their calling thread
	IBackend* m_pThreadBackEnd;                 // Backend for regular jobs, available on PC/XBOX. on Xbox threads are polling with a low priority
	IBackend* m_pBlockingBackEnd;               // Backend for tasks which can block to prevent stalling regular jobs in this case

	u16 m_nJobIdCounter;                     // JobId counter for jobs dynamically allocated at runtime

	std::set<JobUpr::SJobStringHandle> m_registeredJobs;

	enum { nSemaphorePoolSize = 16 };
	SJobFinishedConditionVariable m_JobSemaphorePool[nSemaphorePoolSize];
	u32 m_nCurrentSemaphoreIndex;

	// per frame counter for jobs run/fallback jobs
	u32 m_nJobsRunCounter;
	u32 m_nFallbackJobsRunCounter;

	JobUpr::SInfoBlock** m_pRegularWorkerFallbacks;
	u32 m_nRegularWorkerThreads;

	bool m_bSuspendWorkerForMP;
	DrxMutex m_SuspendWorkerForMPLock;
	DrxConditionVariable m_SuspendWorkerForMPCondion;
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	SJobProfilingDataContainer m_profilingData;
	std::map<JobUpr::SJobStringHandle, ColorB> m_JobColors;
	std::map<SMarker::TMarkerString, ColorB> m_RegionColors;
	CTimeValue m_FrameStartTime[SJobProfilingDataContainer::nCapturedFrames];

#endif

	// singleton stuff
	CJobUpr();
	// disable copy and assignment
	CJobUpr(const CJobUpr &);
	CJobUpr& operator=(const CJobUpr&);

};
}//JobUpr

///////////////////////////////////////////////////////////////////////////////
inline void JobUpr::CJobUpr::CopyJobParameter(u32k cJobParamSize, uk pDestParam, ukk pSrcParam)
{
	assert(IsAligned(cJobParamSize, 16) && "JobParameter Size needs to be a multiple of 16");
	assert(cJobParamSize <= JobUpr::SInfoBlock::scAvailParamSize && "JobParameter Size larger than available storage");
	memcpy(pDestParam, pSrcParam, cJobParamSize);
}

///////////////////////////////////////////////////////////////////////////////
inline void JobUpr::CJobUpr::IncreaseRunJobs()
{
	DrxInterlockedIncrement((i32 *)&m_nJobsRunCounter);
}

///////////////////////////////////////////////////////////////////////////////
inline void JobUpr::CJobUpr::IncreaseRunFallbackJobs()
{
	DrxInterlockedIncrement((i32 *)&m_nFallbackJobsRunCounter);
}

#endif //__JOB_MANAGER_H__
