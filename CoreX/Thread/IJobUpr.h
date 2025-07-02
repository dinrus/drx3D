// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IJobUpr.h
//  Version:     v1.00
//  Created:     1/8/2011 by Christopher Bolte
//  Описание: JobUpr interface
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Thread/DrxThread.h>

// Настройки управления задачами

//! Enable to obtain stats of spu usage each frame.
#define JOBMANAGER_SUPPORT_FRAMEPROFILER

#if !defined(DEDICATED_SERVER)
//! Собирать информацию позадачно о времени старпа, стопа, отправки и синхронизации
// (dispatch, start, stop and sync times).
	#define JOBMANAGER_SUPPORT_PROFILING
#endif

// Отключить фичи, замедляющие производительность.
#if !defined(USE_FRAME_PROFILER) || DRX_PLATFORM_MOBILE
	#undef JOBMANAGER_SUPPORT_FRAMEPROFILER
	#undef JOBMANAGER_SUPPORT_PROFILING
#endif

struct ILog;

//! Implementation of mutex/condition variable.
//! Used in the job manager for yield waiting in corner cases like waiting for a job to finish/jobqueue full and so on.
class SJobFinishedConditionVariable
{
public:
	SJobFinishedConditionVariable() :
		m_nRefCounter(0),
		m_pOwner(NULL)
	{
		m_nFinished = 1;
	}

	void Acquire()
	{
		//wait for completion of the condition
		m_Notify.Lock();
		while (m_nFinished == 0)
			m_CondNotify.Wait(m_Notify);
		m_Notify.Unlock();
	}

	void Release()
	{
		m_Notify.Lock();
		m_nFinished = 1;
		m_Notify.Unlock();
		m_CondNotify.Notify();
	}

	void SetRunning()
	{
		m_nFinished = 0;
	}

	bool IsRunning()
	{
		return m_nFinished == 0;
	}

	void SetStopped()
	{
		m_nFinished = 1;
	}

	void SetOwner( ukk pOwner)
	{
		if (m_pOwner != NULL)
			__debugbreak();
		m_pOwner = pOwner;
	}

	void ClearOwner( ukk pOwner)
	{
		if (m_pOwner != pOwner)
			__debugbreak();

		m_pOwner = NULL;
	}

	bool AddRef( ukk pOwner)
	{
		if (m_pOwner != pOwner)
			return false;

		++m_nRefCounter;
		return true;
	}
	u32 DecRef( ukk pOwner)
	{
		if (m_pOwner != pOwner)
			__debugbreak();

		return --m_nRefCounter;
	}

	bool HasOwner() const { return m_pOwner != NULL; }

private:
	DrxMutex             m_Notify;
	DrxConditionVariable m_CondNotify;
	 u32      m_nFinished;
	 u32      m_nRefCounter;
	 ukk m_pOwner;
};

namespace JobUpr {
class CJobBase;
}
namespace JobUpr {
class CJobDelegator;
}
namespace JobUpr {
struct SProdConsQueueBase;
}
namespace JobUpr {
namespace detail {
struct SJobQueueSlotState;
}
}

//! Wrapper for Vector4 uint.
struct DRX_ALIGN(16) SVEC4_UINT
{
#if !DRX_PLATFORM_SSE2
	u32 v[4];
#else
	__m128 v;
#endif
};

// Глобальные значения.

//! Front end values.
namespace JobUpr
{
//! Установки приоритета для задач.
enum TPriorityLevel
{
	eHighPriority     = 0,
	eRegularPriority  = 1,
	eLowPriority      = 2,
	eStreamPriority   = 3,
	eNumPriorityLevel = 4
};

//! Stores worker utilization. One instance per Worker.
//! One instance per cache line to avoid thread synchronization issues.
struct DRX_ALIGN(128) SWorkerStats
{
	u32 nExecutionPeriod;
	    //!< Accumulated job execution period in micro seconds.
	u32 nNumJobsExecuted;
	    //!< Number of jobs executed.
};

//! Type to reprensent a semaphore handle of the jobmanager.
typedef u16 TSemaphoreHandle;

//! Magic value to reprensent an invalid job handle.
enum  : u32 { INVALID_JOB_HANDLE = ((u32)-1) };

//! BackEnd Type.
enum EBackEndType
{
	eBET_Thread,
	eBET_Fallback,
	eBET_Blocking
};

namespace Fiber
{
//! The alignment of the fibertask stack (currently set to 128 kb).
enum {FIBERTASK_ALIGNMENT = 128U << 10 };

//! The number of switches the fibertask records (default: 32).
enum {FIBERTASK_RECORD_SWITCHES = 32U };

} //endns JobUpr::Fiber

} //endns JobUpr

//! Structures used by the JobUpr.
namespace JobUpr
{
struct SJobStringHandle
{
	tukk  cpString;       //!< Points into repository, must be first element.
	u32 strLen;         //!< String length.
	i32          jobId;          //!< Index (also acts as id) of job.
	u32       nJobInvokerIdx; //!< Index of the jobInvoker (used to job switching in the prod/con queue).

	inline bool operator==(const SJobStringHandle& crOther) const;
	inline bool operator<(const SJobStringHandle& crOther) const;

};

//! Handle retrieved by name for job invocation.
typedef SJobStringHandle* TJobHandle;

bool SJobStringHandle::operator==(const SJobStringHandle& crOther) const
{
	return strcmp(cpString, crOther.cpString) == 0;
}

bool SJobStringHandle::operator<(const SJobStringHandle& crOther) const
{
	return strcmp(cpString, crOther.cpString) < 0;
}

//! Struct to collect profiling informations about job invocations and sync times.
struct DRX_ALIGN(16) SJobProfilingData
{
	typedef CTimeValue TimeValueT;

	TimeValueT nStartTime;
	TimeValueT nEndTime;

	TimeValueT nWaitBegin;
	TimeValueT nWaitEnd;

	TJobHandle jobHandle;
	threadID nThreadId;
	u32 nWorkerThread;

};

struct SJobProfilingDataContainer
{
	enum {nCapturedFrames = 4};
	enum {nCapturedEntriesPerFrame = 4096 };
	u32       nFrameIdx;
	u32       nProfilingDataCounter[nCapturedFrames];

	ILINE u32 GetFillFrameIdx()       { return nFrameIdx % nCapturedFrames; }
	ILINE u32 GetPrevFrameIdx()       { return (nFrameIdx - 1) % nCapturedFrames; }
	ILINE u32 GetRenderFrameIdx()     { return (nFrameIdx - 2) % nCapturedFrames; }
	ILINE u32 GetPrevRenderFrameIdx() { return (nFrameIdx - 3) % nCapturedFrames; }

	DRX_ALIGN(128) SJobProfilingData m_DymmyProfilingData;
	SJobProfilingData arrJobProfilingData[nCapturedFrames][nCapturedEntriesPerFrame];
};

//! Special combination of a  spinning variable combined with a semaphore.
//! Used if the running state is not yet set to finish during waiting.
struct SJobSyncVariable
{
	SJobSyncVariable();

	bool IsRunning() const ;

	//! Interface, should only be used by the job manager or the job state classes.
	void Wait() ;
	void SetRunning(u16 count = 1) ;
	bool SetStopped(struct SJobStateBase* pPostCallback = nullptr, u16 count = 1) ;

private:
	friend class CJobUpr;

	//! Union used to combine the semaphore and the running state in a single word.
	union SyncVar
	{
		 u32 wordValue;
		struct
		{
			u16           nRunningCounter;
			TSemaphoreHandle semaphoreHandle;
		};
	};

	SyncVar syncVar;      //!< Sync-variable which contain the running state or the used semaphore.
#if DRX_PLATFORM_64BIT
	char    padding[4];
#endif
};

//! Condition variable like struct to be used for polling if a job has been finished.
struct SJobStateBase
{
public:
	ILINE bool IsRunning() const { return syncVar.IsRunning(); }
	ILINE void SetRunning(u16 count = 1)
	{
		syncVar.SetRunning(count);
	}
	virtual bool SetStopped(u16 count = 1)
	{
		return syncVar.SetStopped(this, count);
	}
	virtual void AddPostJob() {};

	virtual ~SJobStateBase() {}

private:
	friend class CJobUpr;

	SJobSyncVariable syncVar;
};

//! For speed, use 16 byte aligned job state.
struct DRX_ALIGN(16) SJobState: SJobStateBase
{
	//! When profiling, intercept the SetRunning() and SetStopped() functions for profiling informations.
	ILINE SJobState() :
	#if defined(JOBMANAGER_SUPPORT_PROFILING)
		   nProfilerIndex(~0),
#endif
     m_pFollowUpJob(NULL)
	{
	}

	virtual void AddPostJob() override;

	ILINE void RegisterPostJob(CJobBase* pFollowUpJob) { m_pFollowUpJob = pFollowUpJob; }

	// Non blocking trying to stop state, and run post job.
	ILINE bool TryStopping()
	{
		if (IsRunning())
		{
			return SetStopped();
		}
		return true;
	}

	ILINE const bool Wait();

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	u16 nProfilerIndex;
#endif

	CJobBase* m_pFollowUpJob;
};

struct SJobStateLambda : public SJobState
{
	void RegisterPostJobCallback(tukk postJobName, const std::function<void()>& lambda, TPriorityLevel priority = eRegularPriority, SJobState* pJobState = 0)
	{
		m_callbackJobName = postJobName;
		m_callbackJobPriority = priority;
		m_callbackJobState = pJobState;
		m_callback = lambda;
	}
private:
	virtual void AddPostJob() override;
private:
	tukk                    m_callbackJobName;
	SJobState*                     m_callbackJobState;
	std::function<void()>          m_callback;
	TPriorityLevel                 m_callbackJobPriority;
	DrxCriticalSectionNonRecursive m_stopLock;
};

//! Stores worker utilization stats for a frame.
class CWorkerFrameStats
{
public:
	struct SWorkerStats
	{
		float  nUtilPerc;             //!< Utilization percentage this frame [0.f..100.f].
		u32 nExecutionPeriod;      //!< Total execution time on this worker in usec.
		u32 nNumJobsExecuted;      //!< Number of jobs executed contributing to nExecutionPeriod.
	};

public:
	ILINE CWorkerFrameStats(u8 workerNum)
		: numWorkers(workerNum)
	{
		workerStats = new SWorkerStats[workerNum];
		Reset();
	}

	ILINE ~CWorkerFrameStats()
	{
		delete[] workerStats;
	}

	ILINE void Reset()
	{
		for (i32 i = 0; i < numWorkers; ++i)
		{
			workerStats[i].nUtilPerc = 0.f;
			workerStats[i].nExecutionPeriod = 0;
			workerStats[i].nNumJobsExecuted = 0;
		}

		nSamplePeriod = 0;
	}

public:
	u8         numWorkers;
	u32        nSamplePeriod;
	SWorkerStats* workerStats;
};

struct SWorkerFrameStatsSummary
{
	u32 nSamplePeriod;             //!< Worker sample period.
	u32 nTotalExecutionPeriod;     //!< Total execution time on this worker in usec.
	u32 nTotalNumJobsExecuted;     //!< Total number of jobs executed contributing to nExecutionPeriod.
	float  nAvgUtilPerc;              //!< Average worker utilization [0.f ... 100.f].
	u8  nNumActiveWorkers;         //!< Active number of workers for this frame.
};

//! Per frame and job specific.
struct DRX_ALIGN(16) SJobFrameStats
{
	u32 usec;                //!< Last frame's job time in microseconds.
	u32 count;               //!< Number of calls this frame.
	tukk cpName;               //!< Job name.

	u32 usecLast;            //!< Last but one frames Job time in microseconds (written and accumulated by Thread).

	inline SJobFrameStats();
	inline SJobFrameStats(tukk cpJobName);

	inline void Reset();
	inline void operator=(const SJobFrameStats& crFrom);
	inline bool operator<(const SJobFrameStats& crOther) const;

	static bool sort_lexical(const JobUpr::SJobFrameStats& i, const JobUpr::SJobFrameStats& j)
	{
		return strcmp(i.cpName, j.cpName) < 0;
	}

	static bool sort_time_high_to_low(const JobUpr::SJobFrameStats& i, const JobUpr::SJobFrameStats& j)
	{
		return i.usec > j.usec;
	}

	static bool sort_time_low_to_high(const JobUpr::SJobFrameStats& i, const JobUpr::SJobFrameStats& j)
	{
		return i.usec < j.usec;
	}
};

struct DRX_ALIGN(16) SJobFrameStatsSummary
{
	u32 nTotalExecutionTime;             //!< Total execution time of all jobs in microseconds.
	u32 nNumJobsExecuted;                //!< Total Number of job executions this frame.
	u32 nNumIndividualJobsExecuted;      //!< Total Number of individual jobs.
};

SJobFrameStats::SJobFrameStats(tukk cpJobName) : usec(0), count(0), cpName(cpJobName), usecLast(0)
{}

SJobFrameStats::SJobFrameStats() : usec(0), cpName("Uninitialized"), count(0), usecLast(0)
{}

void SJobFrameStats::operator=(const SJobFrameStats& crFrom)
{
	usec = crFrom.usec;
	count = crFrom.count;
	cpName = crFrom.cpName;
	usecLast = crFrom.usecLast;
}

void SJobFrameStats::Reset()
{
	usecLast = usec;
	usec = count = 0;
}

bool SJobFrameStats::operator<(const SJobFrameStats& crOther) const
{
	//sort from large to small
	return usec > crOther.usec;
}

//! Struct to represent a packet for the consumer producer queue.
struct DRX_ALIGN(16) SAddPacketData
{
	JobUpr::SJobState* pJobState;
	u16 profilerIndex;
	u8 nInvokerIndex;   //!< To keep the struct 16 bytes long, we use a index into the info block.
};

//! Delegator function.
//! Takes a pointer to a params structure and does the decomposition of the parameters, then calls the Job entry function.
typedef void (* Invoker)(uk );

//! Info block transferred first for each job.
//! We have to transfer the info block, parameter block, input memory needed for execution.
struct DRX_ALIGN(128) SInfoBlock
{
	//! Struct to represent the state of a SInfoBlock.
	struct SInfoBlockState
	{
		//! Union of requiered members and uin32 to allow easy atomic operations.
		union
		{
			struct
			{
				TSemaphoreHandle nSemaphoreHandle;    //!< Handle for the conditon variable to use in case some thread is waiting.
				u16           nRoundID;            //!< Number of rounds over the job queue, to prevent a race condition with SInfoBlock reuse (we have 15 bits, thus 32k full job queue iterations are safe).
			};
			 LONG nValue;                     //!< value for easy Compare and Swap.
		};

		void IsInUse(u32 nRoundIdToCheckOrg, bool& rWait, bool& rRetry, u32 nMaxValue)  const
		{
			u32 nRoundIdToCheck = nRoundIdToCheckOrg & 0x7FFF;   //!< Clamp nRoundID to 15 bit.
			u32 nLoadedRoundID = nRoundID;
			u32 nNextLoadedRoundID = ((nLoadedRoundID + 1) & 0x7FFF);
			nNextLoadedRoundID = nNextLoadedRoundID >= nMaxValue ? 0 : nNextLoadedRoundID;

			if (nLoadedRoundID == nRoundIdToCheck)  // job slot free
			{
				rWait = false;
				rRetry = false;
			}
			else if (nNextLoadedRoundID == nRoundIdToCheck)     // job slot need to be worked on by worker
			{
				rWait = true;
				rRetry = false;
			}
			else   // producer was suspended long enough that the worker overtook it
			{
				rWait = false;
				rRetry = true;
			}
		}
	};

	//! data needed to run the job and it's functionality.
	 SInfoBlockState jobState;             //!< State of the SInfoBlock in job queue, should only be modified by access functions.
	Invoker jobInvoker;                            //!< Callback function to job invoker (extracts parameters and calls entry function).
	std::function<void()> jobLambdaInvoker;        //!< Alternative way to invoke job with lambda
	union                                          //!< External job state address /shared with address of prod-cons queue.
	{
		JobUpr::SJobState*          pJobState;
		JobUpr::SProdConsQueueBase* pQueue;
	};
	JobUpr::SInfoBlock* pNext;                 //!< Single linked list for fallback jobs in the case the queue is full, and a worker wants to push new work.

	// Per-job settings like cache size and so on.
	u8 frameProfIndex;                  //!< Index of SJobFrameStats*.
	u8 nflags;
	u8 paramSize;                       //!< Size in total of parameter block in 16 byte units.
	u8 jobId;                           //!< Corresponding job ID, needs to track jobs.

	// We could also use a union, but this solution is (hopefully) clearer.
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	unsigned short profilerIndex;                  //!< index for the job system profiler.
#endif

	//! Bits used for nflags.
	static u32k scHasQueue = 0x4;

	//! Size of the SInfoBlock struct and how much memory we have to store parameters.
#if DRX_PLATFORM_64BIT
	static u32k scSizeOfSJobQueueEntry = 512;
	static u32k scSizeOfJobQueueEntryHeader = 64;   //!< Please adjust when adding/removing members, keep as a multiple of 16.
	static u32k scAvailParamSize = scSizeOfSJobQueueEntry - scSizeOfJobQueueEntryHeader;
#else
	static u32k scSizeOfSJobQueueEntry = 384;
	static u32k scSizeOfJobQueueEntryHeader = 32;   //!< Please adjust when adding/removing members, keep as a multiple of 16.
	static u32k scAvailParamSize = scSizeOfSJobQueueEntry - scSizeOfJobQueueEntryHeader;
#endif

	//! Parameter data are enclosed within to save a cache miss.
	DRX_ALIGN(16) u8 paramData[scAvailParamSize];    //!< is 16 byte aligned, make sure it is kept aligned.

	ILINE void AssignMembersTo(SInfoBlock* pDest) const
	{
		pDest->jobInvoker = jobInvoker;
		pDest->jobLambdaInvoker = jobLambdaInvoker;
		pDest->pJobState = pJobState;
		pDest->pQueue = pQueue;
		pDest->pNext = pNext;

		pDest->frameProfIndex = frameProfIndex;
		pDest->nflags = nflags;
		pDest->paramSize = paramSize;
		pDest->jobId = jobId;

#if defined(JOBMANAGER_SUPPORT_PROFILING)
		pDest->profilerIndex = profilerIndex;
#endif

		memcpy(pDest->paramData, paramData, sizeof(paramData));
	}

	ILINE bool HasQueue() const
	{
		return (nflags & (u8)scHasQueue) != 0;
	}

	ILINE void SetJobState(JobUpr::SJobState* _pJobState)
	{
		assert(!HasQueue());
		pJobState = _pJobState;
	}

	ILINE JobUpr::SJobState* GetJobState() const
	{
		assert(!HasQueue());
		return pJobState;
	}

	ILINE JobUpr::SProdConsQueueBase* GetQueue() const
	{
		assert(HasQueue());
		return pQueue;
	}

	ILINE u8* const __restrict GetParamAddress()
	{
		assert(!HasQueue());
		return &paramData[0];
	}

	//! Functions to access jobstate of SInfoBlock.
	ILINE void IsInUse(u32 nRoundId, bool& rWait, bool& rRetry, u32 nMaxValue) const { return jobState.IsInUse(nRoundId, rWait, rRetry, nMaxValue); }

	void Release(u32 nMaxValue);
	void Wait(u32 nRoundID, u32 nMaxValue);

};

//! State information for the fill and empty pointers of the job queue.
struct DRX_ALIGN(16) SJobQueuePos
{
	//! Base of job queue per priority level.
	JobUpr::SInfoBlock* jobQueue[JobUpr::eNumPriorityLevel];

	//! Base of job queue per priority level.
	JobUpr::detail::SJobQueueSlotState* jobQueueStates[JobUpr::eNumPriorityLevel];

	//! Index use for job slot publishing each priority level is encoded in this single value.
	 uint64 index;

	//! Util function to increase counter for their priority level only.
	static const uint64 eBitsInIndex = sizeof(uint64) * CHAR_BIT;
	static const uint64 eBitsPerPriorityLevel = eBitsInIndex / JobUpr::eNumPriorityLevel;
	static const uint64 eMaskStreamPriority = (1ULL << eBitsPerPriorityLevel) - 1ULL;
	static const uint64 eMaskLowPriority = (((1ULL << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) - 1ULL) & ~eMaskStreamPriority;
	static const uint64 eMaskRegularPriority = ((((1ULL << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) - 1ULL) & ~(eMaskStreamPriority | eMaskLowPriority);
	static const uint64 eMaskHighPriority = (((((1ULL << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) << eBitsPerPriorityLevel) - 1ULL) & ~(eMaskRegularPriority | eMaskStreamPriority | eMaskLowPriority);

	ILINE static uint64 ExtractIndex(uint64 currentIndex, u32 nPriorityLevel)
	{
		switch (nPriorityLevel)
		{
		case eHighPriority:
			return ((((currentIndex & eMaskHighPriority) >> eBitsPerPriorityLevel) >> eBitsPerPriorityLevel) >> eBitsPerPriorityLevel);
		case eRegularPriority:
			return (((currentIndex & eMaskRegularPriority) >> eBitsPerPriorityLevel) >> eBitsPerPriorityLevel);
		case eLowPriority:
			return ((currentIndex & eMaskLowPriority) >> eBitsPerPriorityLevel);
		case eStreamPriority:
			return (currentIndex & eMaskStreamPriority);
		default:
			return ~0;
		}
	}

	ILINE static bool IncreasePullIndex(uint64 currentPullIndex, uint64 currentPushIndex, uint64& rNewPullIndex, u32& rPriorityLevel,
	                                    u32 nJobQueueSizeHighPriority, u32 nJobQueueSizeRegularPriority, u32 nJobQueueSizeLowPriority, u32 nJobQueueSizeStreamPriority)
	{
		u32 nPushExtractedHighPriority = static_cast<u32>(ExtractIndex(currentPushIndex, eHighPriority));
		u32 nPushExtractedRegularPriority = static_cast<u32>(ExtractIndex(currentPushIndex, eRegularPriority));
		u32 nPushExtractedLowPriority = static_cast<u32>(ExtractIndex(currentPushIndex, eLowPriority));
		u32 nPushExtractedStreamPriority = static_cast<u32>(ExtractIndex(currentPushIndex, eStreamPriority));

		u32 nPullExtractedHighPriority = static_cast<u32>(ExtractIndex(currentPullIndex, eHighPriority));
		u32 nPullExtractedRegularPriority = static_cast<u32>(ExtractIndex(currentPullIndex, eRegularPriority));
		u32 nPullExtractedLowPriority = static_cast<u32>(ExtractIndex(currentPullIndex, eLowPriority));
		u32 nPullExtractedStreamPriority = static_cast<u32>(ExtractIndex(currentPullIndex, eStreamPriority));

		u32 nRoundPushHighPriority = nPushExtractedHighPriority / nJobQueueSizeHighPriority;
		u32 nRoundPushRegularPriority = nPushExtractedRegularPriority / nJobQueueSizeRegularPriority;
		u32 nRoundPushLowPriority = nPushExtractedLowPriority / nJobQueueSizeLowPriority;
		u32 nRoundPushStreamPriority = nPushExtractedStreamPriority / nJobQueueSizeStreamPriority;

		u32 nRoundPullHighPriority = nPullExtractedHighPriority / nJobQueueSizeHighPriority;
		u32 nRoundPullRegularPriority = nPullExtractedRegularPriority / nJobQueueSizeRegularPriority;
		u32 nRoundPullLowPriority = nPullExtractedLowPriority / nJobQueueSizeLowPriority;
		u32 nRoundPullStreamPriority = nPullExtractedStreamPriority / nJobQueueSizeStreamPriority;

		u32 nPushJobSlotHighPriority = nPushExtractedHighPriority & (nJobQueueSizeHighPriority - 1);
		u32 nPushJobSlotRegularPriority = nPushExtractedRegularPriority & (nJobQueueSizeRegularPriority - 1);
		u32 nPushJobSlotLowPriority = nPushExtractedLowPriority & (nJobQueueSizeLowPriority - 1);
		u32 nPushJobSlotStreamPriority = nPushExtractedStreamPriority & (nJobQueueSizeStreamPriority - 1);

		u32 nPullJobSlotHighPriority = nPullExtractedHighPriority & (nJobQueueSizeHighPriority - 1);
		u32 nPullJobSlotRegularPriority = nPullExtractedRegularPriority & (nJobQueueSizeRegularPriority - 1);
		u32 nPullJobSlotLowPriority = nPullExtractedLowPriority & (nJobQueueSizeLowPriority - 1);
		u32 nPullJobSlotStreamPriority = nPullExtractedStreamPriority & (nJobQueueSizeStreamPriority - 1);

		//NOTE: if this shows in the profiler, find a lockless variant of it
		if (((nRoundPushHighPriority == nRoundPullHighPriority) && nPullJobSlotHighPriority < nPushJobSlotHighPriority) ||
		    ((nRoundPushHighPriority != nRoundPullHighPriority) && nPullJobSlotHighPriority >= nPushJobSlotHighPriority))
		{
			rPriorityLevel = eHighPriority;
			rNewPullIndex = IncreaseIndex(currentPullIndex, eHighPriority);
		}
		else if (((nRoundPushRegularPriority == nRoundPullRegularPriority) && nPullJobSlotRegularPriority < nPushJobSlotRegularPriority) ||
		         ((nRoundPushRegularPriority != nRoundPullRegularPriority) && nPullJobSlotRegularPriority >= nPushJobSlotRegularPriority))
		{
			rPriorityLevel = eRegularPriority;
			rNewPullIndex = IncreaseIndex(currentPullIndex, eRegularPriority);
		}
		else if (((nRoundPushLowPriority == nRoundPullLowPriority) && nPullJobSlotLowPriority < nPushJobSlotLowPriority) ||
		         ((nRoundPushLowPriority != nRoundPullLowPriority) && nPullJobSlotLowPriority >= nPushJobSlotLowPriority))
		{
			rPriorityLevel = eLowPriority;
			rNewPullIndex = IncreaseIndex(currentPullIndex, eLowPriority);
		}
		else if (((nRoundPushStreamPriority == nRoundPullStreamPriority) && nPullJobSlotStreamPriority < nPushJobSlotStreamPriority) ||
		         ((nRoundPushStreamPriority != nRoundPullStreamPriority) && nPullJobSlotStreamPriority >= nPushJobSlotStreamPriority))
		{
			rPriorityLevel = eStreamPriority;
			rNewPullIndex = IncreaseIndex(currentPullIndex, eStreamPriority);
		}
		else
		{
			rNewPullIndex = ~0;
			return false;
		}

		return true;
	}

	ILINE static uint64 IncreasePushIndex(uint64 currentPushIndex, u32 nPriorityLevel)
	{
		return IncreaseIndex(currentPushIndex, nPriorityLevel);
	}

	ILINE static uint64 IncreaseIndex(uint64 currentIndex, u32 nPriorityLevel)
	{
		uint64 nIncrease = 1ULL;
		uint64 nMask = 0;
		switch (nPriorityLevel)
		{
		case eHighPriority:
			nIncrease <<= eBitsPerPriorityLevel;
			nIncrease <<= eBitsPerPriorityLevel;
			nIncrease <<= eBitsPerPriorityLevel;
			nMask = eMaskHighPriority;
			break;
		case eRegularPriority:
			nIncrease <<= eBitsPerPriorityLevel;
			nIncrease <<= eBitsPerPriorityLevel;
			nMask = eMaskRegularPriority;
			break;
		case eLowPriority:
			nIncrease <<= eBitsPerPriorityLevel;
			nMask = eMaskLowPriority;
			break;
		case eStreamPriority:
			nMask = eMaskStreamPriority;
			break;
		}

		// increase counter while preventing overflow
		uint64 nCurrentValue = currentIndex & nMask;                        // extract all bits for this priority only
		uint64 nCurrentValueCleared = currentIndex & ~nMask;                // extract all bits of other priorities (they shouldn't change)
		nCurrentValue += nIncrease;                                         // increase value by one (already at the right bit position)
		nCurrentValue &= nMask;                                             // mask out again to handle overflow
		uint64 nNewCurrentValue = nCurrentValueCleared | nCurrentValue;     // add new value to other priorities
		return nNewCurrentValue;
	}

};

//! Struct covers DMA data setup common to all jobs and packets.
class CCommonDMABase
{
public:
	CCommonDMABase();
	void        SetJobParamData(uk pParamData);
	ukk GetJobParamData() const;
	u16      GetProfilingDataIndex() const;

	//! In case of persistent job objects, we have to reset the profiling data
	void ForceUpdateOfProfilingDataIndex();

protected:
	uk  m_pParamData;      //!< parameter struct, not initialized to 0 for performance reason.
	u16 nProfilerIndex;    //!< index handle for Jobmanager Profiling Data.
};

//! Delegation class for each job.
class CJobDelegator : public CCommonDMABase
{
public:

	typedef  CJobBase* TDepJob;

	CJobDelegator();
	void                            RunJob(const JobUpr::TJobHandle cJobHandle);
	void                            RegisterQueue(const JobUpr::SProdConsQueueBase* const cpQueue);
	JobUpr::SProdConsQueueBase* GetQueue() const;

	void                            RegisterJobState(JobUpr::SJobState* __restrict pJobState);

	//return a copy since it will get overwritten
	void                   SetParamDataSize(u32k cParamDataSize);
	u32k     GetParamDataSize() const;
	void                   SetCurrentThreadId(const threadID cID);
	const threadID         GetCurrentThreadId() const;

	JobUpr::SJobState* GetJobState() const;
	void                   SetRunning();

	ILINE void             SetDelegator(Invoker pGenericDelecator)
	{
		m_pGenericDelecator = pGenericDelecator;
	}

	Invoker GetGenericDelegator()
	{
		return m_pGenericDelecator;
	}

	u32 GetPriorityLevel() const                      { return m_nPrioritylevel; }
	bool         IsBlocking() const                            { return m_bIsBlocking; };

	void         SetPriorityLevel(u32 nPrioritylevel) { m_nPrioritylevel = nPrioritylevel; }
	void         SetBlocking()                                 { m_bIsBlocking = true; }

	void         SetLambda(const std::function<void()>& lambda)
	{
		m_lambda = lambda;
	}
	const std::function<void()>& GetLambda() const
	{
		return m_lambda;
	}

protected:
	JobUpr::SJobState*                m_pJobState;      //!< Extern job state.
	const JobUpr::SProdConsQueueBase* m_pQueue;         //!< Consumer/producer queue.
	u32                          m_nPrioritylevel; //!< Enum represent the priority level to use.
	bool m_bIsBlocking;                                     //!< If true, then the job could block on a mutex/sleep.
	u32                          m_ParamDataSize;  //!< Sizeof parameter struct.
	threadID                              m_CurThreadID;    //!< Current thread id.
	Invoker                               m_pGenericDelecator;
	std::function<void()>                 m_lambda;
};

//! Base class for jobs.
class CJobBase
{
public:
	ILINE CJobBase() : m_pJobProgramData(NULL)
	{
	}

	ILINE void RegisterJobState(JobUpr::SJobState* __restrict pJobState)
	{
		((CJobBase*)this)->m_JobDelegator.RegisterJobState(pJobState);
	}

	ILINE void RegisterQueue(const JobUpr::SProdConsQueueBase* const cpQueue)
	{
		((CJobBase*)this)->m_JobDelegator.RegisterQueue(cpQueue);
	}
	ILINE void Run()
	{
		m_JobDelegator.RunJob(m_pJobProgramData);
	}

	ILINE const JobUpr::TJobHandle GetJobProgramData()
	{
		return m_pJobProgramData;
	}

	ILINE void SetCurrentThreadId(const threadID cID)
	{
		((CJobBase*)this)->m_JobDelegator.SetCurrentThreadId(cID);
	}

	ILINE const threadID GetCurrentThreadId() const
	{
		return ((CJobBase*)this)->m_JobDelegator.GetCurrentThreadId();
	}

	ILINE CJobDelegator* GetJobDelegator()
	{
		return &m_JobDelegator;
	}

protected:
	CJobDelegator          m_JobDelegator;        //!< Delegation implementation, all calls to job manager are going through it.
	JobUpr::TJobHandle m_pJobProgramData;     //!< Handle to program data to run.

	//! Set the job program data.
	ILINE void SetJobProgramData(const JobUpr::TJobHandle pJobProgramData)
	{
		assert(pJobProgramData);
		m_pJobProgramData = pJobProgramData;
	}

};
} //endns JobUpr

// Interface of the JobUpr.

//! Create a Producer consumer queue for a job type.
#define PROD_CONS_QUEUE_TYPE(name, size) JobUpr::CProdConsQueue < name, (size) >

//! Declaration for the InvokeOnLinkedStacked util function.
extern "C"
{
	void InvokeOnLinkedStack(void (* proc)(uk ), uk arg, uk stack, size_t stackSize);
} // extern "C"

namespace JobUpr
{
class IWorkerBackEndProfiler;

//! Base class for the various backends the jobmanager supports.
struct IBackend
{
	// <interfuscator:shuffle>
	virtual ~IBackend(){}

	virtual bool   Init(u32 nSysMaxWorker) = 0;
	virtual bool   ShutDown() = 0;
	virtual void   Update() = 0;

	virtual void   AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock) = 0;
	virtual u32 GetNumWorkerThreads() const = 0;
	// </interfuscator:shuffle>

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	virtual IWorkerBackEndProfiler* GetBackEndWorkerProfiler() const = 0;
#endif
};

//! Синглтон, управляющий очередями задач.
struct IJobUpr
{
	// <interfuscator:shuffle>
	// === front end interface ===
	virtual ~IJobUpr(){}

	virtual void Init(u32 nSysMaxWorker) = 0;

	//! Добавить задачу.
	virtual void AddJob(JobUpr::CJobDelegator& RESTRICT_REFERENCE crJob, const JobUpr::TJobHandle cJobHandle) = 0;

	//! Добавить задачу в виде лямбда-обрвызова.
	virtual void AddLambdaJob(tukk jobName, const std::function<void()>& lambdaCallback, TPriorityLevel priority = JobUpr::eRegularPriority, SJobState* pJobState = nullptr) = 0;

	//! Wait for a job, preempt the calling thread if the job is not done yet.
	virtual const bool WaitForJob(JobUpr::SJobState& rJobState) const = 0;

	//! Получить дескриптор задачи из имени.
	virtual const JobUpr::TJobHandle GetJobHandle(tukk cpJobName, u32k cStrLen, JobUpr::Invoker pInvoker) = 0;

	//! Получить дескриптор задачи из имени.
	virtual const JobUpr::TJobHandle GetJobHandle(tukk cpJobName, JobUpr::Invoker pInvoker) = 0;

	//! Закрыть (свернуть) управление задачами.
	virtual void                           ShutDown() = 0;

	virtual JobUpr::IBackend*          GetBackEnd(JobUpr::EBackEndType backEndType) = 0;

	virtual bool                           InvokeAsJob(tukk cJobHandle) const = 0;
	virtual bool                           InvokeAsJob(const JobUpr::TJobHandle cJobHandle) const = 0;

	virtual void                           SetJobFilter(tukk pFilter) = 0;
	virtual void                           SetJobSystemEnabled(i32 nEnable) = 0;

	virtual u32                         GetWorkerThreadId() const = 0;

	virtual JobUpr::SJobProfilingData* GetProfilingData(u16 nProfilerIndex) = 0;
	virtual u16                         ReserveProfilingData() = 0;

	virtual void                           Update(i32 nJobSystemProfiler) = 0;
	virtual void                           PushProfilingMarker(tukk pName) = 0;
	virtual void                           PopProfilingMarker() = 0;

	virtual u32                         GetNumWorkerThreads() const = 0;

	//! Get a free semaphore handle from the Job Upr pool.
	virtual JobUpr::TSemaphoreHandle AllocateSemaphore( ukk pOwner) = 0;

	//! Return a semaphore handle to the Job Upr pool.
	virtual void DeallocateSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) = 0;

	//! Increase the refcounter of a semaphore, but only if it is > 0, else returns false.
	virtual bool AddRefSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) = 0;

	//! Get the actual semaphore object for aquire/release calls.
	virtual SJobFinishedConditionVariable* GetSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner) = 0;

	virtual void                           DumpJobList() = 0;

	virtual void                           SetFrameStartTime(const CTimeValue& rFrameStartTime) = 0;
};

//! Utility function to get the worker thread id in a job, returns 0xFFFFFFFF otherwise.
ILINE u32 GetWorkerThreadId()
{
	u32 nWorkerThreadID = gEnv->GetJobUpr()->GetWorkerThreadId();
	return nWorkerThreadID == ~0 ? ~0 : (nWorkerThreadID & ~0x40000000);
}

//! Utility function to find out if a call comes from the mainthread or from a worker thread.
ILINE bool IsWorkerThread()
{
	return gEnv->GetJobUpr()->GetWorkerThreadId() != ~0;
}

//! Utility function to find out if a call comes from the mainthread or from a worker thread.
ILINE bool IsBlockingWorkerThread()
{
	return (gEnv->GetJobUpr()->GetWorkerThreadId() != ~0) && ((gEnv->GetJobUpr()->GetWorkerThreadId() & 0x40000000) != 0);
}

//! Utility function to check if a specific job should really run as job.
ILINE bool InvokeAsJob(tukk pJobName)
{
#if defined(_RELEASE)
	return true;      // In release mode, always assume that the job should be executed.
#else
	return gEnv->pJobUpr->InvokeAsJob(pJobName);
#endif
}

/////////////////////////////////////////////////////////////////////////////
inline void SJobState::AddPostJob()
{
	// Start post job if set.
	if (m_pFollowUpJob)
		m_pFollowUpJob->Run();
}

//////////////////////////////////////////////////////////////////////////
inline void SJobStateLambda::AddPostJob()
{
	if (m_callback)
	{
		// Add post job callback before trying to stop and releasing the semaphore
		gEnv->GetJobUpr()->AddLambdaJob(m_callbackJobName, m_callback, m_callbackJobPriority, m_callbackJobState);   // Use same job state for post job.
	}
}

/////////////////////////////////////////////////////////////////////////////
inline const bool SJobState::Wait()
{
	return gEnv->pJobUpr->WaitForJob(*this);
}

//! Interface of the Producer/Consumer Queue for JobUpr.
//! Producer - consumer queue.
//! - All implemented ILINE using a template:.
//!     - Ring buffer size (num of elements).
//!     - Instance type of job.
//!     - Param type of job.
//! - Factory with macro instantiating the queue (knowing the exact names for a job).
//! - Queue consists of:.
//!     - Ring buffer consists of 1 instance of param type of job.
//!     - For atomic test on finished and push pointer update, the queue is 128 byte aligned and push, pull ptr and DMA job state are lying within that first 128 byte.
//!     -  push (only modified by producer) /pull (only modified by consumer) pointer, point to ring buffer, both equal in the beginning.
//!     - Job instance (create with def. ctor).
//!     - DMA job state (running, finished).
//!     - AddPacket - method to add a packet.
//!         - Wait on current push/pull if any space is available.
//!         - Need atomically test if a job is running and update the push pointer, if job is finished, start new job.
//!     - Finished method, returns push==pull.
//! Job producer side:
//!     - provide RegisterProdConsumerQueue - method, set flags accordingly.
//! Job side:
//!     - Check if it has  a prod/consumer queue.
//!     - If it is part of queue, it must obtain DMA address for job state and of push/pull (static offset to pointer).
//!     - A flag tells if job state is job state or queue.
//!     - Lock is obtained when current push/pull ptr is obtained, snooping therefore enabled.
//!     - Before FlushCacheComplete, get next parameter packet if multiple packets needs to be processed.
//!     - If all packets were processed, try to write updated pull pointer and set finished state. If it fails, push pointer was updated during processing time, in that case get lock again and with it the new push pointer.
//!     - loop till the lock and finished state was updated successfully.
//!     - no HandleCallback method, only 1 JOb is permitted to run with queue.
//!     - no write back to external job state, always just one inner packet loop.
struct SProdConsQueueBase
{
	// Don't move the state and push ptr to another place in this struct - they are updated together with an atomic operation.
	SJobSyncVariable               m_QueueRunningState;     //!< Waiting state of the queue.
	uk                          m_pPush;                 //!< Push pointer, current ptr to push packets into (written by Producer.

	 uk                 m_pPull;                 //!< Pull pointer, current ptr to pull packets from (written by Consumer).
	DRX_ALIGN(16) u32 m_PullIncrement;                   //!< Increment of pull.
	u32                         m_AddPacketDataOffset;   //!< Offset of additional data relative to push ptr.
	INT_PTR                        m_RingBufferStart;       //!< Start of ring buffer.
	INT_PTR                        m_RingBufferEnd;         //!< End of ring buffer.
	SJobFinishedConditionVariable* m_pQueueFullSemaphore;   //!< Semaphore used for yield-waiting when the queue is full.

	SProdConsQueueBase();
};

//! Utility functions namespace for the producer consumer queue.
namespace ProdConsQueueBase
{
inline INT_PTR MarkPullPtrThatWeAreWaitingOnASemaphore(INT_PTR nOrgValue) { assert((nOrgValue & 1) == 0); return nOrgValue | 1; }
inline bool    IsPullPtrMarked(INT_PTR nOrgValue)                         { return (nOrgValue & 1) == 1; }
}

template<class TJobType, u32 Size>
class DRX_ALIGN(128) CProdConsQueue: public SProdConsQueueBase
{
public:
	CProdConsQueue();
	~CProdConsQueue();

	//! Add a new parameter packet with different job type (job invocation).
	template<class TAnotherJobType>
	void AddPacket
	(
	  const typename TJobType::packet & crPacket,
	  JobUpr::TPriorityLevel nPriorityLevel,
	  TAnotherJobType * pJobParam,
	  bool differentJob = true
	);

	//! Adds a new parameter packet (job invocation).
	void AddPacket
	(
	  const typename TJobType::packet & crPacket,
	  JobUpr::TPriorityLevel nPriorityLevel = JobUpr::eRegularPriority
	);

	//! Wait til all current jobs have been finished and been processed.
	void WaitFinished();

	//! Returns true if queue is empty.
	bool IsEmpty();

private:
	//! Initializes queue.
	void Init(u32k cPacketSize);

	//! Get incremented pointer, takes care of wrapping.
	uk const GetIncrementedPointer();

	//! The ring buffer.
	DRX_ALIGN(128) uk m_pRingBuffer;

	//! Job instance.
	TJobType m_JobInstance;
	i32 m_Initialized;

};

//! Интерфейс для отслеживания использования BackEnd worker
//! и сроков выполнения задач.
class IWorkerBackEndProfiler
{
public:
	enum EJobSortOrder
	{
		eJobSortOrder_NoSort,
		eJobSortOrder_TimeHighToLow,
		eJobSortOrder_TimeLowToHigh,
		eJobSortOrder_Lexical,
	};

public:
	typedef DynArray<JobUpr::SJobFrameStats> TJobFrameStatsContainer;

public:
	virtual ~IWorkerBackEndProfiler(){; }

	virtual void Init(u16k numWorkers) = 0;

	//! Обновить профайлер в начале сэмплового периода.
	virtual void Update() = 0;
	virtual void Update(u32k curTimeSample) = 0;

	//! Зарегестрировать задачу профайлером.
	virtual void RegisterJob(u32k jobId, tukk jobName) = 0;

	//! Записать информацию о выполнении зарегистрированной задачи.
	virtual void RecordJob(u16k profileIndex, u8k workerId, u32k jobId, u32k runTimeMicroSec) = 0;

	//! Получить статистику рабочего фрейма.
	virtual void GetFrameStats(JobUpr::CWorkerFrameStats& rStats) const = 0;
	virtual void GetFrameStats(TJobFrameStatsContainer& rJobStats, EJobSortOrder jobSortOrder) const = 0;
	virtual void GetFrameStats(JobUpr::CWorkerFrameStats& rStats, TJobFrameStatsContainer& rJobStats, EJobSortOrder jobSortOrder) const = 0;

	//! Получить сводку по статистике рабочего фрейма.
	virtual void GetFrameStatsSummary(SWorkerFrameStatsSummary& rStats) const = 0;
	virtual void GetFrameStatsSummary(SJobFrameStatsSummary& rStats) const = 0;

	//! Returns the index of the active multi-buffered profile data.
	virtual u16 GetProfileIndex() const = 0;

	//! Получить число отслеживаемых трудяг.
	virtual u32 GetNumWorkers() const = 0;

public:
	//! Returns a microsecond sample.
	static u32 GetTimeSample()
	{
		return static_cast<u32>(gEnv->pTimer->GetAsyncTime().GetMicroSecondsAsInt64());
	}
};

}// namespace JobUpr

extern "C"
{
	//! Interface to create the JobUpr.
	//! Needed here for calls from Producer Consumer queue.
	DLL_EXPORT JobUpr::IJobUpr* GetJobUprInterface();
}

//! Implementation of Producer Consumer functions.
//! Currently adding jobs to a producer/consumer queue is not supported.
template<class TJobType, u32 Size>
ILINE JobUpr::CProdConsQueue<TJobType, Size>::~CProdConsQueue()
{
	if (m_pRingBuffer)
		DrxModuleMemalignFree(m_pRingBuffer);
}

///////////////////////////////////////////////////////////////////////////////
ILINE JobUpr::SProdConsQueueBase::SProdConsQueueBase() :
	m_pPush(NULL), m_pPull(NULL), m_PullIncrement(0), m_AddPacketDataOffset(0),
	m_RingBufferStart(0), m_RingBufferEnd(0), m_pQueueFullSemaphore(NULL)
{

}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
ILINE JobUpr::CProdConsQueue<TJobType, Size>::CProdConsQueue() : m_Initialized(0), m_pRingBuffer(NULL)
{
	assert(Size > 2);
	m_JobInstance.RegisterQueue(this);
}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
ILINE void JobUpr::CProdConsQueue<TJobType, Size >::Init(u32k cPacketSize)
{
	assert((cPacketSize & 15) == 0);
	m_AddPacketDataOffset = cPacketSize;
	m_PullIncrement = m_AddPacketDataOffset + sizeof(SAddPacketData);
	m_pRingBuffer = DrxModuleMemalign(Size * m_PullIncrement, 128);

	assert(m_pRingBuffer);
	m_pPush = m_pRingBuffer;
	m_pPull = m_pRingBuffer;
	m_RingBufferStart = (INT_PTR)m_pRingBuffer;
	m_RingBufferEnd = m_RingBufferStart + Size * m_PullIncrement;
	m_Initialized = 1;
	((TJobType*)&m_JobInstance)->SetParamDataSize(cPacketSize);
	m_pQueueFullSemaphore = NULL;
}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
ILINE void JobUpr::CProdConsQueue<TJobType, Size >::WaitFinished()
{
	m_QueueRunningState.Wait();
	// ensure that the pull ptr is set right
	assert(m_pPull == m_pPush);

}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
ILINE bool JobUpr::CProdConsQueue<TJobType, Size >::IsEmpty()
{
	return (INT_PTR)m_pPush == (INT_PTR)m_pPull;
}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
ILINE uk const JobUpr::CProdConsQueue<TJobType, Size >::GetIncrementedPointer()
{
	//returns branch free the incremented wrapped aware param pointer
	INT_PTR cNextPtr = (INT_PTR)m_pPush + m_PullIncrement;
#if DRX_PLATFORM_64BIT
	if ((INT_PTR)cNextPtr >= (INT_PTR)m_RingBufferEnd) cNextPtr = (INT_PTR)m_RingBufferStart;
	return (uk )cNextPtr;
#else
	u32k cNextPtrMask = (u32)(((i32)(cNextPtr - m_RingBufferEnd)) >> 31);
	return (uk )(cNextPtr & cNextPtrMask | m_RingBufferStart & ~cNextPtrMask);
#endif
}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
inline void JobUpr::CProdConsQueue<TJobType, Size >::AddPacket
(
  const typename TJobType::packet& crPacket,
  JobUpr::TPriorityLevel nPriorityLevel
)
{
	AddPacket<TJobType>(crPacket, nPriorityLevel, (TJobType*)&m_JobInstance, false);
}

///////////////////////////////////////////////////////////////////////////////
template<class TJobType, u32 Size>
template<class TAnotherJobType>
inline void JobUpr::CProdConsQueue<TJobType, Size >::AddPacket
(
  const typename TJobType::packet& crPacket,
  JobUpr::TPriorityLevel nPriorityLevel,
  TAnotherJobType* pJobParam,
  bool differentJob
)
{
	u32k cPacketSize = crPacket.GetPacketSize();
	IF (m_Initialized == 0, 0)
		Init(cPacketSize);

	assert(m_RingBufferEnd == m_RingBufferStart + Size * (cPacketSize + sizeof(SAddPacketData)));

	ukk const cpCurPush = m_pPush;

	// don't overtake the pull ptr

	SJobSyncVariable curQueueRunningState = m_QueueRunningState;
	IF ((cpCurPush == m_pPull) && (curQueueRunningState.IsRunning()), 0)
	{

		INT_PTR nPushPtr = (INT_PTR)cpCurPush;
		INT_PTR markedPushPtr = JobUpr::ProdConsQueueBase::MarkPullPtrThatWeAreWaitingOnASemaphore(nPushPtr);
		TSemaphoreHandle nSemaphoreHandle = gEnv->pJobUpr->AllocateSemaphore(this);
		m_pQueueFullSemaphore = gEnv->pJobUpr->GetSemaphore(nSemaphoreHandle, this);

		bool bNeedSemaphoreWait = false;
#if DRX_PLATFORM_64BIT // for 64 bit, we need to atomicly swap 128 bit
		int64 compareValue[2] = { *alias_cast<int64*>(&curQueueRunningState), (int64)nPushPtr };
		DrxInterlockedCompareExchange128(( int64*)this, (int64)markedPushPtr, *alias_cast<int64*>(&curQueueRunningState), compareValue);
		// make sure nobody set the state to stopped in the meantime
		bNeedSemaphoreWait = (compareValue[0] == *alias_cast<int64*>(&curQueueRunningState) && compareValue[1] == (int64)nPushPtr);
#else
		// union used to construct 64 bit value for atomic updates
		union T64BitValue
		{
			int64 doubleWord;
			struct
			{
				u32 word0;
				u32 word1;
			};
		};

		// job system is process the queue right now, we need an atomic update
		T64BitValue compareValue;
		compareValue.word0 = *alias_cast<u32*>(&curQueueRunningState);
		compareValue.word1 = (u32)nPushPtr;

		T64BitValue exchangeValue;
		exchangeValue.word0 = *alias_cast<u32*>(&curQueueRunningState);
		exchangeValue.word1 = (u32)markedPushPtr;

		T64BitValue resultValue;
		resultValue.doubleWord = DrxInterlockedCompareExchange64(( int64*)this, exchangeValue.doubleWord, compareValue.doubleWord);

		bNeedSemaphoreWait = (resultValue.word0 == *alias_cast<u32*>(&curQueueRunningState) && resultValue.word1 == nPushPtr);
#endif

		// C-A-S successful, now we need to do a yield-wait
		IF (bNeedSemaphoreWait, 0)
			m_pQueueFullSemaphore->Acquire();
		else
			m_pQueueFullSemaphore->SetStopped();

		gEnv->pJobUpr->DeallocateSemaphore(nSemaphoreHandle, this);
		m_pQueueFullSemaphore = NULL;
	}

	if (crPacket.GetJobStateAddress())
	{
		JobUpr::SJobState* pJobState = reinterpret_cast<JobUpr::SJobState*>(crPacket.GetJobStateAddress());
		pJobState->SetRunning();
	}

	//get incremented push pointer and check if there is a slot to push it into
	uk const cpNextPushPtr = GetIncrementedPointer();

	const SVEC4_UINT* __restrict pPacketCont = crPacket.GetPacketCont();
	SVEC4_UINT* __restrict pPushCont = (SVEC4_UINT*)cpCurPush;

	//copy packet data
	u32k cIters = cPacketSize >> 4;
	for (u32 i = 0; i < cIters; ++i)
		pPushCont[i] = pPacketCont[i];

	// setup addpacket data for Jobs
	SAddPacketData* const __restrict pAddPacketData = (SAddPacketData*)((u8*)cpCurPush + m_AddPacketDataOffset);
	pAddPacketData->pJobState = crPacket.GetJobStateAddress();

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	SJobProfilingData* pJobProfilingData = gEnv->GetJobUpr()->GetProfilingData(crPacket.GetProfilerIndex());
	pJobProfilingData->jobHandle = pJobParam->GetJobProgramData();
	pAddPacketData->profilerIndex = crPacket.GetProfilerIndex();
	if (pAddPacketData->pJobState) // also store profilerindex in syncvar, so be able to record wait times
	{
		pAddPacketData->pJobState->nProfilerIndex = pAddPacketData->profilerIndex;
	}
#endif

	// set invoker, for the case the the job changes within the queue
	pAddPacketData->nInvokerIndex = pJobParam->GetProgramHandle()->nJobInvokerIdx;

	// new job queue, or empty job queue
	if (!m_QueueRunningState.IsRunning())
	{
		m_pPush = cpNextPushPtr;//make visible
		m_QueueRunningState.SetRunning();

		pJobParam->RegisterQueue(this);
		pJobParam->SetParamDataSize(((TJobType*)&m_JobInstance)->GetParamDataSize());
		pJobParam->SetPriorityLevel(nPriorityLevel);
		pJobParam->Run();

		return;
	}

	bool bAtomicSwapSuccessfull = false;
	JobUpr::SJobSyncVariable newSyncVar;
	newSyncVar.SetRunning();
#if DRX_PLATFORM_64BIT // for 64 bit, we need to atomicly swap 128 bit
	int64 compareValue[2] = { *alias_cast<int64*>(&newSyncVar), (int64)cpCurPush };
	DrxInterlockedCompareExchange128(( int64*)this, (int64)cpNextPushPtr, *alias_cast<int64*>(&newSyncVar), compareValue);
	// make sure nobody set the state to stopped in the meantime
	bAtomicSwapSuccessfull = (compareValue[0] > 0);
#else
	// union used to construct 64 bit value for atomic updates
	union T64BitValue
	{
		int64 doubleWord;
		struct
		{
			u32 word0;
			u32 word1;
		};
	};

	// job system is process the queue right now, we need an atomic update
	T64BitValue compareValue;
	compareValue.word0 = *alias_cast<u32*>(&newSyncVar);
	compareValue.word1 = (u32)(TRUNCATE_PTR)cpCurPush;

	T64BitValue exchangeValue;
	exchangeValue.word0 = *alias_cast<u32*>(&newSyncVar);
	exchangeValue.word1 = (u32)(TRUNCATE_PTR)cpNextPushPtr;

	T64BitValue resultValue;
	resultValue.doubleWord = DrxInterlockedCompareExchange64(( int64*)this, exchangeValue.doubleWord, compareValue.doubleWord);

	bAtomicSwapSuccessfull = (resultValue.word0 > 0);
#endif

	// job system finished in the meantime, next to issue a new job
	if (!bAtomicSwapSuccessfull)
	{
		m_pPush = cpNextPushPtr;//make visible
		m_QueueRunningState.SetRunning();

		pJobParam->RegisterQueue(this);
		pJobParam->SetParamDataSize(((TJobType*)&m_JobInstance)->GetParamDataSize());
		pJobParam->SetPriorityLevel(nPriorityLevel);
		pJobParam->Run();
	}
}

//! CCommonDMABase Function implementations.
inline JobUpr::CCommonDMABase::CCommonDMABase()
{
	ForceUpdateOfProfilingDataIndex();
}

///////////////////////////////////////////////////////////////////////////////
inline void JobUpr::CCommonDMABase::ForceUpdateOfProfilingDataIndex()
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	nProfilerIndex = gEnv->GetJobUpr()->ReserveProfilingData();
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline void JobUpr::CCommonDMABase::SetJobParamData(uk pParamData)
{
	m_pParamData = pParamData;
}

///////////////////////////////////////////////////////////////////////////////
inline ukk JobUpr::CCommonDMABase::GetJobParamData() const
{
	return m_pParamData;
}

///////////////////////////////////////////////////////////////////////////////
inline u16 JobUpr::CCommonDMABase::GetProfilingDataIndex() const
{
	return nProfilerIndex;
}

//! Job Delegator Function implementations.
ILINE JobUpr::CJobDelegator::CJobDelegator() : m_ParamDataSize(0), m_CurThreadID(THREADID_NULL)
{
	m_pQueue = NULL;
	m_pJobState = NULL;
	m_nPrioritylevel = JobUpr::eRegularPriority;
	m_bIsBlocking = false;
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::RunJob(const JobUpr::TJobHandle cJobHandle)
{
	gEnv->GetJobUpr()->AddJob(*static_cast<CJobDelegator*>(this), cJobHandle);
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::RegisterQueue(const JobUpr::SProdConsQueueBase* const cpQueue)
{
	m_pQueue = cpQueue;
}

///////////////////////////////////////////////////////////////////////////////
ILINE JobUpr::SProdConsQueueBase* JobUpr::CJobDelegator::GetQueue() const
{
	return const_cast<JobUpr::SProdConsQueueBase*>(m_pQueue);
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::RegisterJobState(JobUpr::SJobState* __restrict pJobState)
{
	assert(pJobState);
	m_pJobState = pJobState;
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	pJobState->nProfilerIndex = this->GetProfilingDataIndex();
#endif
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::SetParamDataSize(u32k cParamDataSize)
{
	m_ParamDataSize = cParamDataSize;
}

///////////////////////////////////////////////////////////////////////////////
ILINE u32k JobUpr::CJobDelegator::GetParamDataSize() const
{
	return m_ParamDataSize;
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::SetCurrentThreadId(const threadID cID)
{
	m_CurThreadID = cID;
}

///////////////////////////////////////////////////////////////////////////////
ILINE const threadID JobUpr::CJobDelegator::GetCurrentThreadId() const
{
	return m_CurThreadID;
}

///////////////////////////////////////////////////////////////////////////////
ILINE JobUpr::SJobState* JobUpr::CJobDelegator::GetJobState() const
{
	return m_pJobState;
}

///////////////////////////////////////////////////////////////////////////////
ILINE void JobUpr::CJobDelegator::SetRunning()
{
	m_pJobState->SetRunning();
}

//! Implementation of SJObSyncVariable functions.
inline JobUpr::SJobSyncVariable::SJobSyncVariable()
{
	syncVar.wordValue = 0;
#if DRX_PLATFORM_64BIT
	padding[0] = padding[1] = padding[2] = padding[3] = 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////////
inline bool JobUpr::SJobSyncVariable::IsRunning() const
{
	return syncVar.nRunningCounter != 0;
}

/////////////////////////////////////////////////////////////////////////////////
inline void JobUpr::SJobSyncVariable::Wait()
{
	if (syncVar.wordValue == 0)
		return;

	IJobUpr* pJobUpr = gEnv->GetJobUpr();

	SyncVar currentValue;
	SyncVar newValue;
	SyncVar resValue;
	TSemaphoreHandle semaphoreHandle = gEnv->GetJobUpr()->AllocateSemaphore(this);

retry:
	//  read
	currentValue.wordValue = syncVar.wordValue;

	if (currentValue.wordValue != 0)
	{
		// there is already a semaphore, wait for this one
		if (currentValue.semaphoreHandle)
		{
			// prevent the following race condition: Thread A Gets the semaphore and waits, then Thread B fails the earlier C-A-S
			// thus will wait for the same semaphore, but Thread A already release the semaphore and now another Thread (could even be A again)
			// is using the same semaphore to wait for the job (executed by thread B) which waits for the samephore -> Deadlock
			// this works since the jobmanager semphore function internally are using locks
			// so if the following line succeeds, we got the lock before the release and have increased the use counter
			// if not, it means that thread A release the semaphore, which also means thread B doesn't have to wait anymore

			if (pJobUpr->AddRefSemaphore(currentValue.semaphoreHandle, this))
			{
				pJobUpr->GetSemaphore(currentValue.semaphoreHandle, this)->Acquire();
				pJobUpr->DeallocateSemaphore(currentValue.semaphoreHandle, this);
			}
		}
		else // no semaphore found
		{
			newValue = currentValue;
			newValue.semaphoreHandle = semaphoreHandle;
			resValue.wordValue = DrxInterlockedCompareExchange(( LONG*)&syncVar.wordValue, newValue.wordValue, currentValue.wordValue);

			// four case are now possible:
			//a) job has finished -> we only need to free our semaphore
			//b) we succeeded setting our semaphore, thus wait for it
			//c) another waiter has set it's sempahore wait for the other one
			//d) another thread has increased/decreased the num runner variable, we need to do the run again

			if (resValue.wordValue != 0) // case a), do nothing
			{
				if (resValue.wordValue == currentValue.wordValue) // case b)
				{
					pJobUpr->GetSemaphore(semaphoreHandle, this)->Acquire();
				}
				else // C-A-S not succeeded, check how to proceed
				{
					if (resValue.semaphoreHandle) // case c)
					{
						// prevent the following race condition: Thread A Gets the semaphore and waits, then Thread B fails the earlier C-A-S
						// thus will wait for the same semaphore, but Thread A already release the semaphore and now another Thread (could even be A again)
						// is using the same semaphore to wait for the job (executed by thread B) which waits for the samephore -> Deadlock
						// this works since the jobmanager semphore function internally are using locks
						// so if the following line succeeds, we got the lock before the release and have increased the use counter
						// if not, it means that thread A release the semaphore, which also means thread B doesn't have to wait anymore

						if (pJobUpr->AddRefSemaphore(resValue.semaphoreHandle, this))
						{
							pJobUpr->GetSemaphore(resValue.semaphoreHandle, this)->Acquire();
							pJobUpr->DeallocateSemaphore(resValue.semaphoreHandle, this);
						}
					}
					else // case d
					{
						goto retry;
					}
				}
			}
		}
	}

	// mark an old semaphore as stopped, in case we didn't use use	(if we did use it, this is a nop)
	pJobUpr->GetSemaphore(semaphoreHandle, this)->SetStopped();
	pJobUpr->DeallocateSemaphore(semaphoreHandle, this);
}

/////////////////////////////////////////////////////////////////////////////////
inline void JobUpr::SJobSyncVariable::SetRunning(u16 count)
{
	SyncVar currentValue;
	SyncVar newValue;

	do
	{
		//  read
		currentValue.wordValue = syncVar.wordValue;

		newValue = currentValue;
		newValue.nRunningCounter += count;

		if (newValue.nRunningCounter == 0)
		{
			DRX_ASSERT_MESSAGE(0, "JobUpr: Atomic counter overflow");
		}
	}
	while (DrxInterlockedCompareExchange(( LONG*)&syncVar.wordValue, newValue.wordValue, currentValue.wordValue) != currentValue.wordValue);
}

/////////////////////////////////////////////////////////////////////////////////
inline bool JobUpr::SJobSyncVariable::SetStopped(SJobStateBase* pPostCallback, u16 count)
{
	SyncVar currentValue;
	SyncVar newValue;
	SyncVar resValue;

	// first use atomic operations to decrease the running counter
	do
	{
		//  read
		currentValue.wordValue = syncVar.wordValue;

		if (currentValue.nRunningCounter == 0)
		{
			DRX_ASSERT_MESSAGE(0, "JobUpr: Atomic counter underflow");
			newValue.nRunningCounter = 1; // Force for potential stability problem, should not happen.
		}

		newValue = currentValue;
		newValue.nRunningCounter -= count;

		resValue.wordValue = DrxInterlockedCompareExchange(( LONG*)&syncVar.wordValue, newValue.wordValue, currentValue.wordValue);
	}
	while (resValue.wordValue != currentValue.wordValue);

	// we now successfully decreased our counter, check if we need to signal a semaphore
	if (newValue.nRunningCounter > 0) // return if we still have other jobs on this syncvar
		return false;

	// Post job needs to be added before releasing semaphore of the current job, to allow chain of post jobs to be waited on.
	if (pPostCallback)
		pPostCallback->AddPostJob();

	//! Do we need to release a semaphore?
	if (currentValue.semaphoreHandle)
	{
		// try to set the running counter to 0 atomically
		bool bNeedSemaphoreRelease = true;
		do
		{
			//  read
			currentValue.wordValue = syncVar.wordValue;
			newValue = currentValue;
			newValue.semaphoreHandle = 0;

			// another thread increased the running counter again, don't call semaphore release in this case
			if (currentValue.nRunningCounter)
				return false;

		}
		while (DrxInterlockedCompareExchange(( LONG*)&syncVar.wordValue, newValue.wordValue, currentValue.wordValue) != currentValue.wordValue);
		// set the running successfull to 0, now we can release the semaphore
		gEnv->GetJobUpr()->GetSemaphore(resValue.semaphoreHandle, this)->Release();
	}

	return true;
}

// SInfoBlock State functions.

inline void JobUpr::SInfoBlock::Release(u32 nMaxValue)
{
	// Free lambda bound resources prior marking the info block as free
	jobLambdaInvoker = nullptr;

	JobUpr::IJobUpr* pJobUpr = gEnv->GetJobUpr();

	SInfoBlockState currentInfoBlockState;
	SInfoBlockState newInfoBlockState;
	SInfoBlockState resultInfoBlockState;

	// use atomic operations to set the running flag
	do
	{
		//  read
		currentInfoBlockState.nValue = *(const_cast< LONG*>(&jobState.nValue));

		newInfoBlockState.nSemaphoreHandle = 0;
		newInfoBlockState.nRoundID = (currentInfoBlockState.nRoundID + 1) & 0x7FFF;
		newInfoBlockState.nRoundID = newInfoBlockState.nRoundID >= nMaxValue ? 0 : newInfoBlockState.nRoundID;

		resultInfoBlockState.nValue = DrxInterlockedCompareExchange(( LONG*)&jobState.nValue, newInfoBlockState.nValue, currentInfoBlockState.nValue);
	}
	while (resultInfoBlockState.nValue != currentInfoBlockState.nValue);

	// Release semaphore
	// Since this is a copy of the state when we succeeded the CAS it is ok to it after original jobState was returned to the free list.
	if (currentInfoBlockState.nSemaphoreHandle)
		gEnv->GetJobUpr()->GetSemaphore(currentInfoBlockState.nSemaphoreHandle, this)->Release();
}

/////////////////////////////////////////////////////////////////////////////////
inline void JobUpr::SInfoBlock::Wait(u32 nRoundID, u32 nMaxValue)
{
	bool bWait = false;
	bool bRetry = false;
	jobState.IsInUse(nRoundID, bWait, bRetry, nMaxValue);
	if (!bWait)
		return;

	JobUpr::IJobUpr* pJobUpr = gEnv->GetJobUpr();

	// get a semaphore to wait on
	TSemaphoreHandle semaphoreHandle = pJobUpr->AllocateSemaphore(this);

	// try to set semaphore (info block could have finished in the meantime, or another thread has set the semaphore
	SInfoBlockState currentInfoBlockState;
	SInfoBlockState newInfoBlockState;
	SInfoBlockState resultInfoBlockState;

	currentInfoBlockState.nValue = *(const_cast< LONG*>(&jobState.nValue));

	currentInfoBlockState.IsInUse(nRoundID, bWait, bRetry, nMaxValue);
	if (bWait)
	{
		// is a semaphore already set
		if (currentInfoBlockState.nSemaphoreHandle != 0)
		{
			if (pJobUpr->AddRefSemaphore(currentInfoBlockState.nSemaphoreHandle, this))
			{
				pJobUpr->GetSemaphore(currentInfoBlockState.nSemaphoreHandle, this)->Acquire();
				pJobUpr->DeallocateSemaphore(currentInfoBlockState.nSemaphoreHandle, this);
			}
		}
		else
		{
			// try to set the semaphore
			newInfoBlockState.nRoundID = currentInfoBlockState.nRoundID;
			newInfoBlockState.nSemaphoreHandle = semaphoreHandle;

			resultInfoBlockState.nValue = DrxInterlockedCompareExchange(( LONG*)&jobState.nValue, newInfoBlockState.nValue, currentInfoBlockState.nValue);

			// three case are now possible:
			//a) job has finished -> we only need to free our semaphore
			//b) we succeeded setting our semaphore, thus wait for it
			//c) another waiter has set it's sempahore wait for the other one
			resultInfoBlockState.IsInUse(nRoundID, bWait, bRetry, nMaxValue);
			if (bWait == true)  // case a) do nothing
			{
				if (resultInfoBlockState.nValue == currentInfoBlockState.nValue) // case b)
				{
					pJobUpr->GetSemaphore(semaphoreHandle, this)->Acquire();
				}
				else // case c)
				{
					if (pJobUpr->AddRefSemaphore(resultInfoBlockState.nSemaphoreHandle, this))
					{
						pJobUpr->GetSemaphore(resultInfoBlockState.nSemaphoreHandle, this)->Acquire();
						pJobUpr->DeallocateSemaphore(resultInfoBlockState.nSemaphoreHandle, this);
					}
				}
			}
		}
	}

	pJobUpr->GetSemaphore(semaphoreHandle, this)->SetStopped();
	pJobUpr->DeallocateSemaphore(semaphoreHandle, this);
}

//! Global helper function to wait for a job.
//! Wait for a job, preempt the calling thread if the job is not done yet.
inline const bool DrxWaitForJob(JobUpr::SJobState& rJobState)
{
	return gEnv->pJobUpr->WaitForJob(rJobState);
}

// Shorter helper type for job states
typedef JobUpr::SJobState       DrxJobState;
typedef JobUpr::SJobStateLambda DrxJobStateLambda;