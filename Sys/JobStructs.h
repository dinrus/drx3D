// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*
   declaration of job structures
 */

#pragma once

#include <drx3D/CoreX/Thread/IJobUpr.h>

//forward declarations for friend usage
namespace JobUpr
{
namespace detail
{
// return results for AddJob
enum EAddJobRes
{
	eAJR_Success,                       // success of adding job
	eAJR_NeedFallbackJobInfoBlock,      // Job was added, but a fallback list was used
};

//triple buffer frame stats
enum
{
	eJOB_FRAME_STATS               = 3,
	eJOB_FRAME_STATS_MAX_SUPP_JOBS = 64
};

// configuration for job queue sizes:
// we have two types of backends, threads and blocking threads
// each jobqueue has three priority levels, the first value is high priority, then regular, followed by low priority
static u32k cMaxWorkQueueJobs_ThreadBackEnd_HighPriority = 2048;
static u32k cMaxWorkQueueJobs_ThreadBackEnd_RegularPriority = 2048;
static u32k cMaxWorkQueueJobs_ThreadBackEnd_LowPriority = 2048;
static u32k cMaxWorkQueueJobs_ThreadBackEnd_StreamPriority = 2048;

static u32k cMaxWorkQueueJobs_BlockingBackEnd_HighPriority = 512;
static u32k cMaxWorkQueueJobs_BlockingBackEnd_RegularPriority = 512;
static u32k cMaxWorkQueueJobs_BlockingBackEnd_LowPriority = 512;
static u32k cMaxWorkQueueJobs_BlockingBackEnd_StreamPriority = 512;

// struct to manage the state of a job slot
// used to indicate that a info block has been finished writing
struct SJobQueueSlotState
{
public:
	bool IsReady() const { return m_state == READY; }

	void SetReady()      { m_state = READY; }
	void SetNotReady()   { m_state = NOT_READY; }

private:
	enum StateT
	{
		NOT_READY = 0,
		READY     = 1
	};
	 StateT m_state;
};

}   // namespace detail

// queue node where jobs are pushed into and pulled from
// individual aligned because it is using MFC_LLAR atomics for mutual exclusive access(operates on a 128 byte address base)
// dont waste memory by putting the job queue between the 2 aligned buffers
template<i32 nMaxWorkQueueJobsHighPriority, i32 nMaxWorkQueueJobsRegularPriority, i32 nMaxWorkQueueJobsLowPriority, i32 nMaxWorkQueueJobsStreamPriority>
struct SJobQueue
{
	// store queue sizes to allow to query those
	enum
	{
		eMaxWorkQueueJobsHighPriority    = nMaxWorkQueueJobsHighPriority,
		eMaxWorkQueueJobsRegularPriority = nMaxWorkQueueJobsRegularPriority,
		eMaxWorkQueueJobsLowPriority     = nMaxWorkQueueJobsLowPriority,
		eMaxWorkQueueJobsStreamPriority  = nMaxWorkQueueJobsStreamPriority,
		eMaxWorkQueueJobsSize            = nMaxWorkQueueJobsHighPriority + nMaxWorkQueueJobsRegularPriority + nMaxWorkQueueJobsLowPriority + nMaxWorkQueueJobsStreamPriority
	};

	DRX_ALIGN(128) JobUpr::SJobQueuePos push;                     // position in which jobs are pushed by the PPU
	DRX_ALIGN(128) JobUpr::SJobQueuePos pull;                     // position from which jobs are pulled

	JobUpr::SInfoBlock*                 jobInfoBlocks[eNumPriorityLevel];      // aligned array of SInfoBlocks per priority level
	JobUpr::detail::SJobQueueSlotState* jobInfoBlockStates[eNumPriorityLevel]; // aligned array of SInfoBlocks states per priority level

	// initialize the jobqueue, should only be called once
	void Init();

	//gets job slot for next job (to get storage index for SJobdata), waits until a job slots becomes available again since data get overwritten
	JobUpr::detail::EAddJobRes GetJobSlot(u32& rJobSlot, u32 nPriorityLevel, bool bWaitForFreeJobSlot);

	static u32                  GetMaxWorkerQueueJobs(u32 nPriorityLevel);
	static tukk             PrioToString(u32 nPriorityLevel);
};

///////////////////////////////////////////////////////////////////////////////
// convinient typedef for the different platform queues
typedef SJobQueue<JobUpr::detail::cMaxWorkQueueJobs_ThreadBackEnd_HighPriority, JobUpr::detail::cMaxWorkQueueJobs_ThreadBackEnd_RegularPriority, JobUpr::detail::cMaxWorkQueueJobs_ThreadBackEnd_LowPriority, JobUpr::detail::cMaxWorkQueueJobs_ThreadBackEnd_StreamPriority>         SJobQueue_ThreadBackEnd;
typedef SJobQueue<JobUpr::detail::cMaxWorkQueueJobs_BlockingBackEnd_HighPriority, JobUpr::detail::cMaxWorkQueueJobs_BlockingBackEnd_RegularPriority, JobUpr::detail::cMaxWorkQueueJobs_BlockingBackEnd_LowPriority, JobUpr::detail::cMaxWorkQueueJobs_BlockingBackEnd_StreamPriority> SJobQueue_BlockingBackEnd;
}

///////////////////////////////////////////////////////////////////////////////
template<i32 nMaxWorkQueueJobsHighPriority, i32 nMaxWorkQueueJobsRegularPriority, i32 nMaxWorkQueueJobsLowPriority, i32 nMaxWorkQueueJobsStreamPriority>
inline JobUpr::detail::EAddJobRes JobUpr::SJobQueue<nMaxWorkQueueJobsHighPriority, nMaxWorkQueueJobsRegularPriority, nMaxWorkQueueJobsLowPriority, nMaxWorkQueueJobsStreamPriority >::GetJobSlot(u32& rJobSlot, u32 nPriorityLevel, bool bWaitForFreeJobSlot)
{
	// verify assumation about queue size at compile time
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsHighPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__HIGH_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsRegularPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE_REGULAR_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsLowPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__LOW_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsStreamPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__LOW_PRIORITY_IS_NOT_POWER_OF_TWO);

	SJobQueuePos& RESTRICT_REFERENCE curPushEntry = push;

	uint64 currentIndex;
	uint64 nextIndex;
	u32 nRoundID;
	u32 nExtractedIndex;
	JobUpr::SInfoBlock* pPushInfoBlock = NULL;
	u32 nMaxWorkerQueueJobs = GetMaxWorkerQueueJobs(nPriorityLevel);
	do
	{
		// fetch next to update field
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX // emulate a 64bit atomic read on PC platfom
		currentIndex = DrxInterlockedCompareExchange64(alias_cast< int64*>(&curPushEntry.index), 0, 0);
#else
		currentIndex = *const_cast< uint64*>(&curPushEntry.index);
#endif

		nextIndex = JobUpr::SJobQueuePos::IncreasePushIndex(currentIndex, nPriorityLevel);
		nExtractedIndex = static_cast<u32>(JobUpr::SJobQueuePos::ExtractIndex(currentIndex, nPriorityLevel));
		nRoundID = nExtractedIndex / nMaxWorkerQueueJobs;

		// compute the job slot for this fetch index
		u32 jobSlot = nExtractedIndex & (nMaxWorkerQueueJobs - 1);
		pPushInfoBlock = &curPushEntry.jobQueue[nPriorityLevel][jobSlot];

		//do not overtake pull pointer
		bool bWait = false;
		bool bRetry = false;
		pPushInfoBlock->IsInUse(nRoundID, bWait, bRetry, (1 << JobUpr::SJobQueuePos::eBitsPerPriorityLevel) / nMaxWorkerQueueJobs);

		if (bRetry) // need to refetch due long suspending time
			continue;

		if (bWait)
		{
			DRX_ASSERT_MESSAGE(false, "JobUpr: Exceeded job queue size (%u) for priority level \"%s\"", nMaxWorkerQueueJobs, PrioToString(nPriorityLevel));
			if (bWaitForFreeJobSlot)
			{
				pPushInfoBlock->Wait(nRoundID, (1 << JobUpr::SJobQueuePos::eBitsPerPriorityLevel) / nMaxWorkerQueueJobs);
			}
			else
			{
				return JobUpr::detail::eAJR_NeedFallbackJobInfoBlock;
		}
		}

		rJobSlot = jobSlot;

		if (DrxInterlockedCompareExchange64(alias_cast< int64*>(&curPushEntry.index), nextIndex, currentIndex) == currentIndex)
			break;

	}
	while (true);

	return JobUpr::detail::eAJR_Success;
}

///////////////////////////////////////////////////////////////////////////////
template<i32 nMaxWorkQueueJobsHighPriority, i32 nMaxWorkQueueJobsRegularPriority, i32 nMaxWorkQueueJobsLowPriority, i32 nMaxWorkQueueJobsStreamPriority>
inline void JobUpr::SJobQueue<nMaxWorkQueueJobsHighPriority, nMaxWorkQueueJobsRegularPriority, nMaxWorkQueueJobsLowPriority, nMaxWorkQueueJobsStreamPriority >::Init()
{
	// verify assumation about queue size at compile time
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsHighPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__HIGH_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsRegularPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE_REGULAR_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsLowPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__LOW_PRIORITY_IS_NOT_POWER_OF_TWO);
	STATIC_CHECK(IsPowerOfTwoCompileTime<eMaxWorkQueueJobsStreamPriority>::IsPowerOfTwo, ERROR_MAX_JOB_QUEUE_SIZE__LOW_PRIORITY_IS_NOT_POWER_OF_TWO);

	// init job queues
	jobInfoBlocks[eHighPriority] = static_cast<JobUpr::SInfoBlock*>(DrxModuleMemalign(eMaxWorkQueueJobsHighPriority * sizeof(JobUpr::SInfoBlock), 128));
	jobInfoBlockStates[eHighPriority] = static_cast<JobUpr::detail::SJobQueueSlotState*>(DrxModuleMemalign(eMaxWorkQueueJobsHighPriority * sizeof(JobUpr::detail::SJobQueueSlotState), 128));
	memset(jobInfoBlocks[eHighPriority], 0, eMaxWorkQueueJobsHighPriority * sizeof(JobUpr::SInfoBlock));
	memset(jobInfoBlockStates[eHighPriority], 0, eMaxWorkQueueJobsHighPriority * sizeof(JobUpr::detail::SJobQueueSlotState));

	jobInfoBlocks[eRegularPriority] = static_cast<JobUpr::SInfoBlock*>(DrxModuleMemalign(eMaxWorkQueueJobsRegularPriority * sizeof(JobUpr::SInfoBlock), 128));
	jobInfoBlockStates[eRegularPriority] = static_cast<JobUpr::detail::SJobQueueSlotState*>(DrxModuleMemalign(eMaxWorkQueueJobsRegularPriority * sizeof(JobUpr::detail::SJobQueueSlotState), 128));
	memset(jobInfoBlocks[eRegularPriority], 0, eMaxWorkQueueJobsRegularPriority * sizeof(JobUpr::SInfoBlock));
	memset(jobInfoBlockStates[eRegularPriority], 0, eMaxWorkQueueJobsRegularPriority * sizeof(JobUpr::detail::SJobQueueSlotState));

	jobInfoBlocks[eLowPriority] = static_cast<JobUpr::SInfoBlock*>(DrxModuleMemalign(eMaxWorkQueueJobsLowPriority * sizeof(JobUpr::SInfoBlock), 128));
	jobInfoBlockStates[eLowPriority] = static_cast<JobUpr::detail::SJobQueueSlotState*>(DrxModuleMemalign(eMaxWorkQueueJobsLowPriority * sizeof(JobUpr::detail::SJobQueueSlotState), 128));
	memset(jobInfoBlocks[eLowPriority], 0, eMaxWorkQueueJobsLowPriority * sizeof(JobUpr::SInfoBlock));
	memset(jobInfoBlockStates[eLowPriority], 0, eMaxWorkQueueJobsLowPriority * sizeof(JobUpr::detail::SJobQueueSlotState));

	jobInfoBlocks[eStreamPriority] = static_cast<JobUpr::SInfoBlock*>(DrxModuleMemalign(eMaxWorkQueueJobsStreamPriority * sizeof(JobUpr::SInfoBlock), 128));
	jobInfoBlockStates[eStreamPriority] = static_cast<JobUpr::detail::SJobQueueSlotState*>(DrxModuleMemalign(eMaxWorkQueueJobsStreamPriority * sizeof(JobUpr::detail::SJobQueueSlotState), 128));
	memset(jobInfoBlocks[eStreamPriority], 0, eMaxWorkQueueJobsStreamPriority * sizeof(JobUpr::SInfoBlock));
	memset(jobInfoBlockStates[eStreamPriority], 0, eMaxWorkQueueJobsStreamPriority * sizeof(JobUpr::detail::SJobQueueSlotState));

	// init queue pos objects
	push.jobQueue[eHighPriority] = jobInfoBlocks[eHighPriority];
	push.jobQueue[eRegularPriority] = jobInfoBlocks[eRegularPriority];
	push.jobQueue[eLowPriority] = jobInfoBlocks[eLowPriority];
	push.jobQueue[eStreamPriority] = jobInfoBlocks[eStreamPriority];
	push.jobQueueStates[eHighPriority] = jobInfoBlockStates[eHighPriority];
	push.jobQueueStates[eRegularPriority] = jobInfoBlockStates[eRegularPriority];
	push.jobQueueStates[eLowPriority] = jobInfoBlockStates[eLowPriority];
	push.jobQueueStates[eStreamPriority] = jobInfoBlockStates[eStreamPriority];
	push.index = 0;

	pull.jobQueue[eHighPriority] = jobInfoBlocks[eHighPriority];
	pull.jobQueue[eRegularPriority] = jobInfoBlocks[eRegularPriority];
	pull.jobQueue[eLowPriority] = jobInfoBlocks[eLowPriority];
	pull.jobQueue[eStreamPriority] = jobInfoBlocks[eStreamPriority];
	pull.jobQueueStates[eHighPriority] = jobInfoBlockStates[eHighPriority];
	pull.jobQueueStates[eRegularPriority] = jobInfoBlockStates[eRegularPriority];
	pull.jobQueueStates[eLowPriority] = jobInfoBlockStates[eLowPriority];
	pull.jobQueueStates[eStreamPriority] = jobInfoBlockStates[eStreamPriority];
	pull.index = 0;
}

///////////////////////////////////////////////////////////////////////////////
template<i32 nMaxWorkQueueJobsHighPriority, i32 nMaxWorkQueueJobsRegularPriority, i32 nMaxWorkQueueJobsLowPriority, i32 nMaxWorkQueueJobsStreamPriority>
ILINE u32 JobUpr::SJobQueue<nMaxWorkQueueJobsHighPriority, nMaxWorkQueueJobsRegularPriority, nMaxWorkQueueJobsLowPriority, nMaxWorkQueueJobsStreamPriority >::GetMaxWorkerQueueJobs(u32 nPriorityLevel)
{
	switch (nPriorityLevel)
	{
	case eHighPriority:
		return eMaxWorkQueueJobsHighPriority;
	case eRegularPriority:
		return eMaxWorkQueueJobsRegularPriority;
	case eLowPriority:
		return eMaxWorkQueueJobsLowPriority;
	case eStreamPriority:
		return eMaxWorkQueueJobsStreamPriority;
	default:
		return ~0;
	}
}


template<i32 nMaxWorkQueueJobsHighPriority, i32 nMaxWorkQueueJobsRegularPriority, i32 nMaxWorkQueueJobsLowPriority, i32 nMaxWorkQueueJobsStreamPriority>
ILINE tukk JobUpr::SJobQueue<nMaxWorkQueueJobsHighPriority, nMaxWorkQueueJobsRegularPriority, nMaxWorkQueueJobsLowPriority, nMaxWorkQueueJobsStreamPriority>::PrioToString(u32 nPriorityLevel)
{
	switch (nPriorityLevel)
	{
	case eHighPriority:
		return "High";
	case eRegularPriority:
		return "Regular";
	case eLowPriority:
		return "Low";
	case eStreamPriority:
		return "Stream";
	default:
		return "Unknown";
	}
}
