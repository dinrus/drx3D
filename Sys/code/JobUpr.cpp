// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*
   implementation of job manager
   DMA memory mappings can be issued in any order
 */
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/JobUpr.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Thread/IJobUpr.h>

#include <drx3D/Sys/FallBackBackend.h>
#include <drx3D/Sys/ThreadBackEnd.h>
#include <drx3D/Sys/BlockingBackEnd.h>

#include <drx3D/Sys/System.h>
#include <drx3D/Sys/CPUDetect.h>

#include <drx3D/Plugins/concqueue/concqueue.hpp>

namespace JobUpr {
namespace Detail {

/////////////////////////////////////////////////////////////////////////////
// functions to convert between an index and a semaphore handle by salting
// the index with a special bit (bigger than the max index), this is requiered
// to have a index of 0, but have all semaphore handles != 0
// a TSemaphoreHandle needs to be 16 bytes, it shared one word (4 byte) with the number of jobs running in a syncvar
enum { nSemaphoreSaltBit = 0x8000 };    // 2^15 (highest bit)

static TSemaphoreHandle IndexToSemaphoreHandle(u32 nIndex)           { return nIndex | nSemaphoreSaltBit; }
static u32           SemaphoreHandleToIndex(TSemaphoreHandle handle) { return handle & ~nSemaphoreSaltBit; }

} // namespace Detail
} // namespace JobUpr

///////////////////////////////////////////////////////////////////////////////
JobUpr::CWorkerBackEndProfiler::CWorkerBackEndProfiler()
	: m_nCurBufIndex(0)
{
	m_WorkerStatsInfo.m_pWorkerStats = 0;
}

///////////////////////////////////////////////////////////////////////////////
JobUpr::CWorkerBackEndProfiler::~CWorkerBackEndProfiler()
{
	if (m_WorkerStatsInfo.m_pWorkerStats)
		DrxModuleMemalignFree(m_WorkerStatsInfo.m_pWorkerStats);
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::Init(u16 numWorkers)
{
	// Init Job Stats
	ZeroMemory(m_JobStatsInfo.m_pJobStats, sizeof(m_JobStatsInfo.m_pJobStats));

	// Init Worker Stats
	for (u32 i = 0; i < JobUpr::detail::eJOB_FRAME_STATS; ++i)
	{
		m_WorkerStatsInfo.m_nStartTime[i] = 0;
		m_WorkerStatsInfo.m_nEndTime[i] = 0;
	}
	m_WorkerStatsInfo.m_nNumWorkers = numWorkers;

	if (m_WorkerStatsInfo.m_pWorkerStats)
		DrxModuleMemalignFree(m_WorkerStatsInfo.m_pWorkerStats);

	i32 nWorkerStatsBufSize = sizeof(JobUpr::SWorkerStats) * numWorkers * JobUpr::detail::eJOB_FRAME_STATS;
	m_WorkerStatsInfo.m_pWorkerStats = (JobUpr::SWorkerStats*)DrxModuleMemalign(nWorkerStatsBufSize, 128);
	ZeroMemory(m_WorkerStatsInfo.m_pWorkerStats, nWorkerStatsBufSize);
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::Update()
{
	Update(IWorkerBackEndProfiler::GetTimeSample());
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::Update(u32k curTimeSample)
{
	// End current state's time period
	m_WorkerStatsInfo.m_nEndTime[m_nCurBufIndex] = curTimeSample;

	// Get next buffer index
	u8 nNextIndex = (m_nCurBufIndex + 1);
	nNextIndex = (nNextIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nNextIndex;

	// Reset next buffer slot and start its time period
	ResetWorkerStats(nNextIndex, curTimeSample);
	ResetJobStats(nNextIndex);

	// Advance buffer index
	m_nCurBufIndex = nNextIndex;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetFrameStats(JobUpr::CWorkerFrameStats& rStats) const
{
	u8 nTailIndex = (m_nCurBufIndex + 1);
	nTailIndex = (nTailIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nTailIndex;

	// Get worker stats from tail
	GetWorkerStats(nTailIndex, rStats);
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetFrameStats(TJobFrameStatsContainer& rJobStats, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const
{
	u8 nTailIndex = (m_nCurBufIndex + 1);
	nTailIndex = (nTailIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nTailIndex;

	// Get job info from tail
	GetJobStats(nTailIndex, rJobStats, jobSortOrder);
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetFrameStats(JobUpr::CWorkerFrameStats& rStats, TJobFrameStatsContainer& rJobStats, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const
{
	u8 nTailIndex = (m_nCurBufIndex + 1);
	nTailIndex = (nTailIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nTailIndex;

	// Get worker stats from tail
	GetWorkerStats(nTailIndex, rStats);

	// Get job info from tail
	GetJobStats(nTailIndex, rJobStats, jobSortOrder);
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetFrameStatsSummary(SWorkerFrameStatsSummary& rStats) const
{
	u8 nTailIndex = (m_nCurBufIndex + 1);
	nTailIndex = (nTailIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nTailIndex;

	// Calculate percentage range multiplier for this frame
	// Take absolute time delta to handle microsecond time sample counter overfilling
	i32 nSamplePeriod = abs((i32)m_WorkerStatsInfo.m_nEndTime[nTailIndex] - (i32)m_WorkerStatsInfo.m_nStartTime[nTailIndex]);
	const float nMultiplier = (1.0f / (float)nSamplePeriod) * 100.0f;

	// Accumulate stats
	u32 totalExecutionTime = 0;
	u32 totalNumJobsExecuted = 0;
	const JobUpr::SWorkerStats* pWorkerStatsOffset = &m_WorkerStatsInfo.m_pWorkerStats[nTailIndex * m_WorkerStatsInfo.m_nNumWorkers];

	for (u8 i = 0; i < m_WorkerStatsInfo.m_nNumWorkers; ++i)
	{
		const JobUpr::SWorkerStats& workerStats = pWorkerStatsOffset[i];
		totalExecutionTime += workerStats.nExecutionPeriod;
		totalNumJobsExecuted += workerStats.nNumJobsExecuted;
	}

	rStats.nSamplePeriod = nSamplePeriod;
	rStats.nNumActiveWorkers = (u8)m_WorkerStatsInfo.m_nNumWorkers;
	rStats.nAvgUtilPerc = ((float)totalExecutionTime / (float)m_WorkerStatsInfo.m_nNumWorkers) * nMultiplier;
	rStats.nTotalExecutionPeriod = totalExecutionTime;
	rStats.nTotalNumJobsExecuted = totalNumJobsExecuted;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetFrameStatsSummary(SJobFrameStatsSummary& rStats) const
{
	u8 nTailIndex = (m_nCurBufIndex + 1);
	nTailIndex = (nTailIndex > (JobUpr::detail::eJOB_FRAME_STATS - 1)) ? 0 : nTailIndex;

	const JobUpr::SJobFrameStats* pJobStatsToCopyFrom = &m_JobStatsInfo.m_pJobStats[nTailIndex * JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS];

	// Accumulate job stats
	u16 totalIndividualJobCount = 0;
	u32 totalJobsExecutionTime = 0;
	u32 totalJobCount = 0;
	for (u16 i = 0; i < JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS; ++i)
	{
		const JobUpr::SJobFrameStats& rJobStats = pJobStatsToCopyFrom[i];
		if (rJobStats.count > 0)
		{
			totalJobsExecutionTime += rJobStats.usec;
			totalJobCount += rJobStats.count;
			totalIndividualJobCount++;
		}
	}

	rStats.nTotalExecutionTime = totalJobsExecutionTime;
	rStats.nNumJobsExecuted = totalJobCount;
	rStats.nNumIndividualJobsExecuted = totalIndividualJobCount;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::RegisterJob(u32k jobId, tukk jobName)
{
	DRX_ASSERT_MESSAGE(jobId < JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS,
	                   string().Format("JobUpr::CWorkerBackEndProfiler::RegisterJob: Limit for current max supported jobs reached. Current limit: %u. Increase JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS limit."
	                                   , JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS));

	for (u8 i = 0; i < JobUpr::detail::eJOB_FRAME_STATS; ++i)
		m_JobStatsInfo.m_pJobStats[(i* JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS) +jobId].cpName = jobName;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::RecordJob(u16k profileIndex, u8k workerId, u32k jobId, u32k runTimeMicroSec)
{
	DRX_ASSERT_MESSAGE(workerId < m_WorkerStatsInfo.m_nNumWorkers, string().Format("JobUpr::CWorkerBackEndProfiler::RecordJob: workerId is out of scope. workerId:%u , scope:%u"
	                                                                               , workerId, m_WorkerStatsInfo.m_nNumWorkers));

	JobUpr::SJobFrameStats& jobStats = m_JobStatsInfo.m_pJobStats[(profileIndex* JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS) +jobId];
	JobUpr::SWorkerStats& workerStats = m_WorkerStatsInfo.m_pWorkerStats[profileIndex * m_WorkerStatsInfo.m_nNumWorkers + workerId];

	// Update job stats
	u32 nCount = ~0;
	do
	{
		nCount = *const_cast< u32*>(&jobStats.count);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&jobStats.count), nCount + 1, nCount) != nCount);

	u32 nUsec = ~0;
	do
	{
		nUsec = *const_cast< u32*>(&jobStats.usec);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&jobStats.usec), nUsec + runTimeMicroSec, nUsec) != nUsec);

	// Update worker stats
	u32 threadExcutionTime = ~0;
	do
	{
		threadExcutionTime = *const_cast< u32*>(&workerStats.nExecutionPeriod);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&workerStats.nExecutionPeriod), threadExcutionTime + runTimeMicroSec, threadExcutionTime) != threadExcutionTime);

	u32 numJobsExecuted = ~0;
	do
	{
		numJobsExecuted = *const_cast< u32*>(&workerStats.nNumJobsExecuted);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&workerStats.nNumJobsExecuted), numJobsExecuted + 1, numJobsExecuted) != numJobsExecuted);
}

///////////////////////////////////////////////////////////////////////////////
u16 JobUpr::CWorkerBackEndProfiler::GetProfileIndex() const
{
	return m_nCurBufIndex;
}

///////////////////////////////////////////////////////////////////////////////
u32 JobUpr::CWorkerBackEndProfiler::GetNumWorkers() const
{
	return m_WorkerStatsInfo.m_nNumWorkers;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetWorkerStats(u8k nBufferIndex, JobUpr::CWorkerFrameStats& rWorkerStats) const
{
	assert(rWorkerStats.numWorkers <= m_WorkerStatsInfo.m_nNumWorkers);

	// Calculate percentage range multiplier for this frame
	// Take absolute time delta to handle microsecond time sample counter overfilling
	u32 nSamplePeriode = max(m_WorkerStatsInfo.m_nEndTime[nBufferIndex] - m_WorkerStatsInfo.m_nStartTime[nBufferIndex], (u32)1);
	const float nMultiplier = nSamplePeriode ? (1.0f / fabsf(static_cast<float>(nSamplePeriode))) * 100.0f : 0.f;
	JobUpr::SWorkerStats* pWorkerStatsOffset = &m_WorkerStatsInfo.m_pWorkerStats[nBufferIndex * m_WorkerStatsInfo.m_nNumWorkers];

	rWorkerStats.numWorkers = (u8)m_WorkerStatsInfo.m_nNumWorkers;
	rWorkerStats.nSamplePeriod = nSamplePeriode;

	for (u8 i = 0; i < m_WorkerStatsInfo.m_nNumWorkers; ++i)
	{
		// Get previous frame's stats
		JobUpr::SWorkerStats& workerStats = pWorkerStatsOffset[i];
		if (workerStats.nExecutionPeriod > 0)
		{
			rWorkerStats.workerStats[i].nUtilPerc = (float)workerStats.nExecutionPeriod * nMultiplier;
			rWorkerStats.workerStats[i].nExecutionPeriod = workerStats.nExecutionPeriod;
			rWorkerStats.workerStats[i].nNumJobsExecuted = workerStats.nNumJobsExecuted;
		}
		else
		{
			ZeroMemory(&rWorkerStats.workerStats[i], sizeof(CWorkerFrameStats::SWorkerStats));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::GetJobStats(u8k nBufferIndex, TJobFrameStatsContainer& rJobStatsContainer, IWorkerBackEndProfiler::EJobSortOrder jobSortOrder) const
{
	const JobUpr::SJobFrameStats* pJobStatsToCopyFrom = &m_JobStatsInfo.m_pJobStats[nBufferIndex * JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS];

	// Clear and ensure size
	rJobStatsContainer.clear();
	rJobStatsContainer.reserve(JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS);

	// Copy job stats
	u16 curCount = 0;
	for (u16 i = 0; i < JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS; ++i)
	{
		const JobUpr::SJobFrameStats& rJobStats = pJobStatsToCopyFrom[i];
		if (rJobStats.count > 0)
		{
			rJobStatsContainer.push_back(rJobStats);
		}
	}

	// Sort job stats
	switch (jobSortOrder)
	{
	case JobUpr::IWorkerBackEndProfiler::eJobSortOrder_TimeHighToLow:
		std::sort(rJobStatsContainer.begin(), rJobStatsContainer.end(), SJobFrameStats::sort_time_high_to_low);
		break;
	case JobUpr::IWorkerBackEndProfiler::eJobSortOrder_TimeLowToHigh:
		std::sort(rJobStatsContainer.begin(), rJobStatsContainer.end(), SJobFrameStats::sort_time_low_to_high);
		break;
	case JobUpr::IWorkerBackEndProfiler::eJobSortOrder_Lexical:
		std::sort(rJobStatsContainer.begin(), rJobStatsContainer.end(), SJobFrameStats::sort_lexical);
		break;
	case JobUpr::IWorkerBackEndProfiler::eJobSortOrder_NoSort:
		break;
	default:
		DRX_ASSERT_MESSAGE(false, "Unsupported type");
	}
	;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::ResetWorkerStats(u8k nBufferIndex, u32k curTimeSample)
{
	ZeroMemory(&m_WorkerStatsInfo.m_pWorkerStats[nBufferIndex * m_WorkerStatsInfo.m_nNumWorkers], sizeof(JobUpr::SWorkerStats) * m_WorkerStatsInfo.m_nNumWorkers);
	m_WorkerStatsInfo.m_nStartTime[nBufferIndex] = curTimeSample;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CWorkerBackEndProfiler::ResetJobStats(u8k nBufferIndex)
{
	JobUpr::SJobFrameStats* pJobStatsToReset = &m_JobStatsInfo.m_pJobStats[nBufferIndex * JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS];

	// Reset job stats
	u16 curCount = 0;
	for (u16 i = 0; i < JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS; ++i)
	{
		JobUpr::SJobFrameStats& rJobStats = pJobStatsToReset[i];
		rJobStats.Reset();
	}
}

///////////////////////////////////////////////////////////////////////////////
JobUpr::CJobUpr* JobUpr::CJobUpr::Instance()
{
	static JobUpr::CJobUpr _singleton;
	return &_singleton;
}

extern "C"
{
	JobUpr::IJobUpr* GetJobUprInterface()
	{
		return JobUpr::CJobUpr::Instance();
	}
}

JobUpr::CJobUpr::CJobUpr()
	: m_Initialized(false),
	m_pFallBackBackEnd(NULL),
	m_pThreadBackEnd(NULL),
	m_pBlockingBackEnd(NULL),
	m_nJobIdCounter(0),
	m_nJobSystemEnabled(1),
	m_bJobSystemProfilerPaused(0),
	m_bJobSystemProfilerEnabled(false),
	m_nJobsRunCounter(0),
	m_nFallbackJobsRunCounter(0),
	m_bSuspendWorkerForMP(false)
{
	// create backends
	m_pThreadBackEnd = new ThreadBackEnd::CThreadBackEnd();

#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS
	m_nRegularWorkerThreads = 6;
#else
	CCpuFeatures* pCPU = new CCpuFeatures;
	pCPU->Detect();
	i32 numWorkers = (i32)pCPU->GetLogicalCPUCount() - 1;
	if (numWorkers < 0)
		numWorkers = 0;
	//m_nRegularWorkerThreads = pCPU->GetLogicalCPUCount();
	m_nRegularWorkerThreads = numWorkers;
	delete pCPU;
#endif

	m_pRegularWorkerFallbacks = new JobUpr::SInfoBlock*[m_nRegularWorkerThreads];
	memset(m_pRegularWorkerFallbacks, 0, sizeof(JobUpr::SInfoBlock*) * m_nRegularWorkerThreads);

	m_pBlockingBackEnd = DrxAlignedNew<BlockingBackEnd::CBlockingBackEnd>(m_pRegularWorkerFallbacks, m_nRegularWorkerThreads);
	m_pFallBackBackEnd = new FallBackBackEnd::CFallBackBackEnd();

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	m_profilingData.nFrameIdx = 0;
#endif

	memset(m_arrJobInvokers, 0, sizeof(m_arrJobInvokers));
	m_nJobInvokerIdx = 0;

	// init fallback backend early to be able to handle jobs before jobmanager is initialized
	if (m_pFallBackBackEnd)  m_pFallBackBackEnd->Init(-1 /*not used for fallback*/);
}

const bool JobUpr::CJobUpr::WaitForJob(JobUpr::SJobState& rJobState) const
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	SJobProfilingData* pJobProfilingData = gEnv->GetJobUpr()->GetProfilingData(rJobState.nProfilerIndex);
	pJobProfilingData->nWaitBegin = gEnv->pTimer->GetAsyncTime();
	pJobProfilingData->nThreadId = DrxGetCurrentThreadId();
#endif

	rJobState.syncVar.Wait();

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	pJobProfilingData->nWaitEnd = gEnv->pTimer->GetAsyncTime();
	pJobProfilingData = NULL;
#endif

	return true;
}

ColorB JobUpr::CJobUpr::GenerateColorBasedOnName(tukk name)
{
	ColorB color;

	u32 hash = CCrc32::Compute(name);
	color.r = static_cast<u8>(hash);
	color.g = static_cast<u8>(hash >> 8);
	color.b = static_cast<u8>(hash >> 16);

	return color;
}

const JobUpr::TJobHandle JobUpr::CJobUpr::GetJobHandle(tukk cpJobName, u32k cStrLen, JobUpr::Invoker pInvoker)
{
	static JobUpr::SJobStringHandle cFailedLookup = { "", 0 };
	JobUpr::SJobStringHandle cLookup = { cpJobName, cStrLen };
	bool bJobIdSet = false;

	// mis-use the JobQueue lock, this shouldn't create contention,
	// since this functions is only called once per job
	AUTO_LOCK(m_JobUprLock);

	// don't insert in list when we only look up the job for debugging settings
	if (pInvoker == NULL)
	{
		std::set<JobUpr::SJobStringHandle>::iterator it = m_registeredJobs.find(cLookup);
		return it == m_registeredJobs.end() ? &cFailedLookup : (JobUpr::TJobHandle)&(*(it));
	}

	std::pair<std::set<JobUpr::SJobStringHandle>::iterator, bool> it = m_registeredJobs.insert(cLookup);
	JobUpr::TJobHandle ret = (JobUpr::TJobHandle)&(*(it.first));

	bJobIdSet = !it.second;

	// generate color for each entry
	if (it.second)
	{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
		m_JobColors[cLookup] = GenerateColorBasedOnName(cpJobName);
#endif
		m_arrJobInvokers[m_nJobInvokerIdx] = pInvoker;
		ret->nJobInvokerIdx = m_nJobInvokerIdx;
		m_nJobInvokerIdx += 1;
		assert(m_nJobInvokerIdx < DRX_ARRAY_COUNT(m_arrJobInvokers));
	}

	if (!bJobIdSet)
	{
		ret->jobId = m_nJobIdCounter;
		m_nJobIdCounter++;
	}

	return ret;
}

tukk JobUpr::CJobUpr::GetJobName(JobUpr::Invoker invoker)
{

	u32 idx = ~0;
	// find the index for this invoker function
	for (size_t i = 0; i < m_nJobInvokerIdx; ++i)
	{
		if (m_arrJobInvokers[i] == invoker)
		{
			idx = i;
			break;
		}
	}
	if (idx == ~0)
		return "JobNotFound";

	// now search for thix idx in all registered jobs
	for (std::set<JobUpr::SJobStringHandle>::iterator it = m_registeredJobs.begin(), end = m_registeredJobs.end(); it != end; ++it)
	{
		if (it->nJobInvokerIdx == idx)
			return it->cpString;
	}

	return "JobNotFound";
}

void JobUpr::CJobUpr::AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle)
{
	char job_info[128];
	drx_sprintf(job_info, "AddJob_%s", cJobHandle->cpString);
	DrxProfile::detail::SetProfilingEvent(0, job_info);

	JobUpr::SInfoBlock infoBlock;

	// Test if the job should be invoked
	bool bUseJobSystem = m_nJobSystemEnabled ? CJobUpr::InvokeAsJob(cJobHandle) : false;

	// get producer/consumer queue settings
	JobUpr::SProdConsQueueBase* cpQueue = crJob.GetQueue();
	const bool cNoQueue = (cpQueue == NULL);

	u32k cOrigParamSize = crJob.GetParamDataSize();
	u8k cParamSize = cOrigParamSize >> 4;

	//reset info block
	u32 flagSet = cNoQueue ? 0 : (u32)JobUpr::SInfoBlock::scHasQueue;

	infoBlock.pQueue = cpQueue;
	infoBlock.nflags = (u8)(flagSet);
	infoBlock.paramSize = cParamSize;
	infoBlock.jobInvoker = crJob.GetGenericDelegator();
	infoBlock.jobLambdaInvoker = crJob.GetLambda();
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	infoBlock.profilerIndex = crJob.GetProfilingDataIndex();
#endif

	if (cNoQueue && crJob.GetJobState())
	{
		infoBlock.SetJobState(crJob.GetJobState());
		crJob.SetRunning();
	}

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	SJobProfilingData* pJobProfilingData = gEnv->GetJobUpr()->GetProfilingData(infoBlock.profilerIndex);
	pJobProfilingData->jobHandle = cJobHandle;
#endif

	// == dispatch to the right BackEnd == //
	IF (crJob.IsBlocking() == false && (bUseJobSystem == false || m_Initialized == false), 0)
		return static_cast<FallBackBackEnd::CFallBackBackEnd*>(m_pFallBackBackEnd)->FallBackBackEnd::CFallBackBackEnd::AddJob(crJob, cJobHandle, infoBlock);

	IF (m_pBlockingBackEnd && crJob.IsBlocking(), 0)
		return static_cast<BlockingBackEnd::CBlockingBackEnd*>(m_pBlockingBackEnd)->BlockingBackEnd::CBlockingBackEnd::AddJob(crJob, cJobHandle, infoBlock);

	// default case is the threadbackend
	if (m_pThreadBackEnd)
		return static_cast<ThreadBackEnd::CThreadBackEnd*>(m_pThreadBackEnd)->ThreadBackEnd::CThreadBackEnd::AddJob(crJob, cJobHandle, infoBlock);

	// last resort - fallback backend
	return static_cast<FallBackBackEnd::CFallBackBackEnd*>(m_pFallBackBackEnd)->FallBackBackEnd::CFallBackBackEnd::AddJob(crJob, cJobHandle, infoBlock);
}

void JobUpr::CJobUpr::AddLambdaJob(tukk jobName, const std::function<void()>& callback, TPriorityLevel priority, SJobState* pJobState)
{
	CJobLambda job(jobName, callback);
	job.SetPriorityLevel(priority);
	if (pJobState)
		job.RegisterJobState(pJobState);
	job.Run();
}

void JobUpr::CJobUpr::ShutDown()
{
	if (m_pFallBackBackEnd) m_pFallBackBackEnd->ShutDown();
	if (m_pThreadBackEnd) m_pThreadBackEnd->ShutDown();
	if (m_pBlockingBackEnd) m_pBlockingBackEnd->ShutDown();
}

void JobUpr::CJobUpr::Init(u32 nSysMaxWorker)
{
	// only init once
	if (m_Initialized)
		return;

	m_Initialized = true;

	// initialize the backends for this platform
	if (m_pThreadBackEnd)
	{
		if (!m_pThreadBackEnd->Init(nSysMaxWorker))
		{
			delete m_pThreadBackEnd;
			m_pThreadBackEnd = NULL;
		}
	}
	if (m_pBlockingBackEnd)    m_pBlockingBackEnd->Init(1);
}

bool JobUpr::CJobUpr::InvokeAsJob(const JobUpr::TJobHandle cJobHandle) const
{
	tukk cpJobName = cJobHandle->cpString;
	return this->CJobUpr::InvokeAsJob(cpJobName);
}

bool JobUpr::CJobUpr::InvokeAsJob(tukk cpJobName) const
{
#if defined(_RELEASE)
	return true; // no support for fallback interface in release
#endif

	// try to find the jobname in the job filter list
	IF (m_pJobFilter, 0)
	{
		if (tukk p = strstr(m_pJobFilter, cpJobName))
			if (p == m_pJobFilter || p[-1] == ',')
			{
				p += strlen(cpJobName);
				if (*p == 0 || *p == ',')
					return false;
			}
	}

	return m_nJobSystemEnabled != 0;
}

u32 JobUpr::CJobUpr::GetWorkerThreadId() const
{
	return JobUpr::detail::GetWorkerThreadId();
}

JobUpr::SJobProfilingData* JobUpr::CJobUpr::GetProfilingData(u16 nProfilerIndex)
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	u32 nFrameIdx = (nProfilerIndex & 0xC000) >> 14; // frame index is encoded in the top two bits
	u32 nProfilingDataEntryIndex = (nProfilerIndex & ~0xC000);

	IF (nProfilingDataEntryIndex >= SJobProfilingDataContainer::nCapturedEntriesPerFrame, 0)
		return &m_profilingData.m_DymmyProfilingData;

	JobUpr::SJobProfilingData* pProfilingData = &m_profilingData.arrJobProfilingData[nFrameIdx][nProfilingDataEntryIndex];
	return pProfilingData;
#else
	return NULL;
#endif
}

u16 JobUpr::CJobUpr::ReserveProfilingData()
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	u32 nFrameIdx = m_profilingData.GetFillFrameIdx();
	u32 nProfilingDataEntryIndex = DrxInterlockedIncrement(( i32*)&m_profilingData.nProfilingDataCounter[nFrameIdx]);

	// encore nFrameIdx in top two bits
	if (nProfilingDataEntryIndex >= SJobProfilingDataContainer::nCapturedEntriesPerFrame)
	{
		//printf("Out of Profiling entries\n");
		nProfilingDataEntryIndex = 16383;
	}
	assert(nFrameIdx <= 4);
	assert(nProfilingDataEntryIndex <= 16383);

	u16 nProfilerIndex = (nFrameIdx << 14) | nProfilingDataEntryIndex;

	return nProfilerIndex;
#else
	return ~0;
#endif
}

// struct to contain profling entries
struct SOrderedProfilingData
{
	SOrderedProfilingData(const ColorB& rColor, i32 beg, i32 end) :
		color(rColor), nBegin(beg), nEnd(end){}

	ColorB color;   // color of the entry
	i32    nBegin;  // begin offset from graph
	i32    nEnd;    // end offset from graph
};

void MyDraw2dLabel(float x, float y, float font_size, const float* pfColor, bool bCenter, tukk label_text, ...) PRINTF_PARAMS(6, 7);
void MyDraw2dLabel(float x, float y, float font_size, const float* pfColor, bool bCenter, tukk label_text, ...)
{
	va_list args;
	va_start(args, label_text);
	IRenderAuxText::DrawText(Vec3(x, y, 1.0f), font_size, pfColor, eDrawText_2D | eDrawText_FixedSize | eDrawText_IgnoreOverscan | eDrawText_Monospace, label_text, args);
	va_end(args);
}

// free function to make the profiling rendering code more compact/readable
namespace DrawUtils
{
void AddToGraph(ColorB* pGraph, const SOrderedProfilingData& rProfilingData)
{
	for (i32 i = rProfilingData.nBegin; i < rProfilingData.nEnd; ++i)
	{
		pGraph[i] = rProfilingData.color;
	}
}

void Draw2DBox(float fX, float fY, float fHeigth, float fWidth, const ColorB& rColor, float fScreenHeigth, float fScreenWidth, IRenderAuxGeom* pAuxRenderer)
{

	float fPosition[4][2] = {
		{ fX,          fY           },
		{ fX,          fY + fHeigth },
		{ fX + fWidth, fY + fHeigth },
		{ fX + fWidth, fY           }
	};

	// compute normalized position from absolute points
	Vec3 vPosition[4] = {
		Vec3(fPosition[0][0], fPosition[0][1], 0.0f),
		Vec3(fPosition[1][0], fPosition[1][1], 0.0f),
		Vec3(fPosition[2][0], fPosition[2][1], 0.0f),
		Vec3(fPosition[3][0], fPosition[3][1], 0.0f)
	};

	vtx_idx const anTriangleIndices[6] = {
		0, 1, 2,
		0, 2, 3
	};

	pAuxRenderer->DrawTriangles(vPosition, 4, anTriangleIndices, 6, rColor);
}

void Draw2DBoxOutLine(float fX, float fY, float fHeigth, float fWidth, const ColorB& rColor, float fScreenHeigth, float fScreenWidth, IRenderAuxGeom* pAuxRenderer)
{
	float fPosition[4][2] = {
		{ fX - 1.0f,          fY - 1.0f           },
		{ fX - 1.0f,          fY + fHeigth + 1.0f },
		{ fX + fWidth + 1.0f, fY + fHeigth + 1.0f },
		{ fX + fWidth + 1.0f, fY - 1.0f           }
	};

	// compute normalized position from absolute points
	Vec3 vPosition[4] = {
		Vec3(fPosition[0][0], fPosition[0][1], 0.0f),
		Vec3(fPosition[1][0], fPosition[1][1], 0.0f),
		Vec3(fPosition[2][0], fPosition[2][1], 0.0f),
		Vec3(fPosition[3][0], fPosition[3][1], 0.0f)
	};

	pAuxRenderer->DrawLine(vPosition[0], rColor, vPosition[1], rColor);
	pAuxRenderer->DrawLine(vPosition[1], rColor, vPosition[2], rColor);
	pAuxRenderer->DrawLine(vPosition[2], rColor, vPosition[3], rColor);
	pAuxRenderer->DrawLine(vPosition[3], rColor, vPosition[0], rColor);
}

void DrawGraph(ColorB* pGraph, i32 nGraphSize, float fBaseX, float fbaseY, float fHeight, float fScreenHeigth, float fScreenWidth, IRenderAuxGeom* pAuxRenderer)
{
	for (i32 i = 0; i < nGraphSize; )
	{
		// skip empty fields
		if (pGraph[i].r == 0 && pGraph[i].g == 0 && pGraph[i].b == 0)
		{
			++i;
			continue;
		}

		float fBegin = (float)i;
		ColorB color = pGraph[i];

		while (true)
		{
			if ((i + 1 >= nGraphSize) ||      // reached end of graph
			    (pGraph[i + 1] != pGraph[i])) // start of different region
			{
				// draw box for this graph part
				float fX = fBaseX + fBegin;
				float fY = fbaseY;
				float fEnd = (float)i;
				float fWidth = fEnd - fBegin;   // compute width
				DrawUtils::Draw2DBox(fX, fY, fHeight, fWidth, pGraph[i], fScreenHeigth, fScreenWidth, pAuxRenderer);
				++i;
				break;
			}

			++i;   // still the same graph, go to next entry
		}
	}
}

void WriteShortLabel(float fTextSideOffset, float fTopOffset, float fTextSize, float* fTextColor, tukk tmpBuffer, i32 nCapChars)
{
	char textBuffer[512] = { 0 };
	tuk pDst = textBuffer;
	tuk pEnd = textBuffer + nCapChars;   // keep space for tailing '\0'
	tukk pSrc = tmpBuffer;
	while (*pSrc != '\0' && pDst < pEnd)
	{
		*pDst = *pSrc;
		++pDst;
		++pSrc;
	}
	MyDraw2dLabel(fTextSideOffset, fTopOffset, fTextSize, fTextColor, false, "%s", textBuffer);
}

} // namespace DrawUtils

struct SJobProflingRenderData
{
	tukk pName;      // name of the job
	ColorB      color;      // color to use to represent this job

	CTimeValue  runTime;
	CTimeValue  waitTime;

	CTimeValue  cacheTime;
	CTimeValue  codePagingTime;
	CTimeValue  fnResolveTime;
	CTimeValue  memtransferSyncTime;

	u32      invocations;

	bool operator<(const SJobProflingRenderData& rOther) const
	{
		return (INT_PTR)pName < (INT_PTR)rOther.pName;
	}

	bool operator<(tukk pOther) const
	{
		return (INT_PTR)pName < (INT_PTR)pOther;
	}
};

struct SJobProflingRenderDataCmp
{
	bool operator()(const SJobProflingRenderData& rA, tukk pB) const                   { return (INT_PTR)rA.pName < (INT_PTR)pB; }
	bool operator()(const SJobProflingRenderData& rA, const SJobProflingRenderData& rB) const { return (INT_PTR)rA.pName < (INT_PTR)rB.pName; }
	bool operator()(tukk pA, const SJobProflingRenderData& rB) const                   { return (INT_PTR)pA < (INT_PTR)rB.pName; }
};

struct SJobProfilingLexicalSort
{
	bool operator()(const SJobProflingRenderData& rA, const SJobProflingRenderData& rB) const
	{
		return strcmp(rA.pName, rB.pName) < 0;
	}

};

struct SWorkerProfilingRenderData
{
	CTimeValue runTime;
};

struct SRegionTime
{
	JobUpr::CJobUpr::SMarker::TMarkerString pName;
	ColorB     color;
	CTimeValue executionTime;
	bool       bIsMainThread;

	bool operator==(const SRegionTime& rOther) const
	{
		return strcmp(pName.c_str(), rOther.pName.c_str()) == 0;
	}
	bool operator<(const SRegionTime& rOther) const
	{
		return strcmp(pName.c_str(), rOther.pName.c_str()) < 0;
	}
};

struct SRegionLexicalSorter
{
	bool operator()(const SRegionTime& rA, const SRegionTime& rB) const
	{
		// sort highest times first
		return strcmp(rA.pName.c_str(), rB.pName.c_str()) < 0;
	}
};

struct SThreadProflingRenderData
{
	CTimeValue executionTime;
	CTimeValue waitTime;
};

void JobUpr::CJobUpr::Update(i32 nJobSystemProfiler)
{
#if 0 // integrate into profiler after fixing it's memory issues
	float fColorGreen[4] = { 0, 1, 0, 1 };
	float fColorRed[4] = { 1, 0, 0, 1 };
	u32 nJobsRunCounter = m_nJobsRunCounter;
	u32 nFallbackJobsRunCounter = m_nFallbackJobsRunCounter;
	IRenderAuxText::Draw2dLabel(1, 5.0f, 1.3f, nFallbackJobsRunCounter ? fColorRed : fColorGreen, false, "Jobs Submitted %d, FallbackJobs %d", nJobsRunCounter, nFallbackJobsRunCounter);
#endif
	m_nJobsRunCounter = 0;
	m_nFallbackJobsRunCounter = 0;

	// Listen for keyboard input if enabled
	if (m_bJobSystemProfilerEnabled != nJobSystemProfiler && gEnv->pInput)
	{
		if (nJobSystemProfiler)
			gEnv->pInput->AddEventListener(this);
		else
			gEnv->pInput->RemoveEventListener(this);

		m_bJobSystemProfilerEnabled = nJobSystemProfiler;
	}

	// profiler disabled
	if (nJobSystemProfiler == 0)
		return;

	AUTO_LOCK(m_JobUprLock);

#if defined(JOBMANAGER_SUPPORT_PROFILING)
	i32 nFrameId = m_profilingData.GetRenderFrameIdx();

	CTimeValue frameStartTime = m_FrameStartTime[nFrameId];
	CTimeValue frameEndTime = m_FrameStartTime[m_profilingData.GetPrevFrameIdx()];

	// skip first frames still we have enough data
	if (frameStartTime.GetValue() == 0)
		return;

	// compute how long the displayed frame took
	CTimeValue diffTime = frameEndTime - frameStartTime;

	// get used thread ids
	threadID nMainThreadId, nRenderThreadId;
	gEnv->pRenderer->GetThreadIDs(nMainThreadId, nRenderThreadId);

	// now compute the relative screen size, and how many pixels are represented by a time value
	i32 nScreenHeight = gEnv->pRenderer->GetOverlayHeight();
	i32 nScreenWidth  = gEnv->pRenderer->GetOverlayWidth();

	float fScreenHeight = (float)nScreenHeight;
	float fScreenWidth = (float)nScreenWidth;

	float fTextSize = 1.1f;
	float fTextSizePixel = 8.0f * fTextSize;
	float fTextCharWidth = 6.0f * fTextSize;

	float fTopOffset = fScreenHeight * 0.01f;                       // keep 0.1% screenspace at top
	float fTextSideOffset = fScreenWidth * 0.01f;                   // start text rendering after 0.1% of screen width
	float fGraphSideOffset = fTextSideOffset + 15 * fTextCharWidth; // leave enough space for 15 characters before drawing the graphs

	float fInfoBoxSize = fTextCharWidth * 35;
	float fGraphHeight = fTextSizePixel;
	float fGraphWidth = (fScreenWidth - fInfoBoxSize) * 0.70f; // 70%

	float pixelPerTime = (float)fGraphWidth / diffTime.GetValue();

	i32k nNumWorker = m_pThreadBackEnd->GetNumWorkerThreads();
	i32k nNumJobs = m_registeredJobs.size();
	i32k nGraphSize = (i32)fGraphWidth;
	i32 nNumRegions = m_nMainThreadMarkerIndex[nFrameId] + m_nRenderThreadMarkerIndex[nFrameId];

	ColorB boxBorderColor(128, 128, 128, 0);
	float fTextColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// allocate structure on the stack to prevent too many costly memory allocations
	//first structures to represent the graph data, we just use (0,0,0) as not set
	// and set each field with the needed color, then we render the resulting boxes
	// in one pass
	PREFAST_SUPPRESS_WARNING(6255)
	ColorB * arrMainThreadRegion = (ColorB*)alloca(nGraphSize * sizeof(ColorB));
	memset(arrMainThreadRegion, 0, nGraphSize * sizeof(ColorB));
	PREFAST_SUPPRESS_WARNING(6255)
	ColorB * arrRenderThreadRegion = (ColorB*)alloca(nGraphSize * sizeof(ColorB));
	memset(arrRenderThreadRegion, 0, nGraphSize * sizeof(ColorB));
	PREFAST_SUPPRESS_WARNING(6255)
	ColorB * arrMainThreadWaitRegion = (ColorB*)alloca(nGraphSize * sizeof(ColorB));
	memset(arrMainThreadWaitRegion, 0, nGraphSize * sizeof(ColorB));
	PREFAST_SUPPRESS_WARNING(6255)
	ColorB * arrRenderThreadWaitRegion = (ColorB*)alloca(nGraphSize * sizeof(ColorB));
	memset(arrRenderThreadWaitRegion, 0, nGraphSize * sizeof(ColorB));

	PREFAST_SUPPRESS_WARNING(6255)
	SRegionTime * arrRegionProfilingData = (SRegionTime*)alloca(nNumRegions * sizeof(SRegionTime));
	memset(arrRegionProfilingData, 0, nNumRegions * sizeof(SRegionTime));

	// accumulate region time for overview
	i32 nRegionCounter = 0;
	for (u32 i = 0; i < m_nMainThreadMarkerIndex[nFrameId]; ++i)
	{
		if (m_arrMainThreadMarker[nFrameId][i].type == SMarker::POP_MARKER)
			continue;

		arrRegionProfilingData[nRegionCounter].pName = m_arrMainThreadMarker[nFrameId][i].marker;
		arrRegionProfilingData[nRegionCounter].color = ColorB();
		arrRegionProfilingData[nRegionCounter].bIsMainThread = m_arrMainThreadMarker[nFrameId][i].bIsMainThread;
		++nRegionCounter;
	}
	for (u32 i = 0; i < m_nRenderThreadMarkerIndex[nFrameId]; ++i)
	{
		if (m_arrRenderThreadMarker[nFrameId][i].type == SMarker::POP_MARKER)
			continue;

		arrRegionProfilingData[nRegionCounter].pName = m_arrRenderThreadMarker[nFrameId][i].marker;
		arrRegionProfilingData[nRegionCounter].color = ColorB();
		arrRegionProfilingData[nRegionCounter].bIsMainThread = m_arrRenderThreadMarker[nFrameId][i].bIsMainThread;
		++nRegionCounter;
	}

	// remove duplicates of region entries
	std::sort(arrRegionProfilingData, arrRegionProfilingData + nRegionCounter, SRegionLexicalSorter());
	SRegionTime* pEnd = std::unique(arrRegionProfilingData, arrRegionProfilingData + nRegionCounter);
	nNumRegions = (i32)(pEnd - arrRegionProfilingData);

	// get region colors
	for (i32 i = 0; i < nNumRegions; ++i)
	{
		SRegionTime& rRegionData = arrRegionProfilingData[i];
		rRegionData.color = GetRegionColor(rRegionData.pName);
	}

	PREFAST_SUPPRESS_WARNING(6255)
	ColorB * *arrWorkerThreadsRegions = (ColorB**)alloca(nNumWorker * sizeof(ColorB * *));

	for (i32 i = 0; i < nNumWorker; ++i)
	{
		PREFAST_SUPPRESS_WARNING(6263) PREFAST_SUPPRESS_WARNING(6255)
		arrWorkerThreadsRegions[i] = (ColorB*)alloca(nGraphSize * sizeof(ColorB));
		memset(arrWorkerThreadsRegions[i], 0, nGraphSize * sizeof(ColorB));
	}

	// allocate per worker data
	PREFAST_SUPPRESS_WARNING(6255)
	SWorkerProfilingRenderData * arrWorkerProfilingRenderData = (SWorkerProfilingRenderData*)alloca(nNumWorker * sizeof(SWorkerProfilingRenderData));
	memset(arrWorkerProfilingRenderData, 0, nNumWorker * sizeof(SWorkerProfilingRenderData));

	// allocate per job informations
	PREFAST_SUPPRESS_WARNING(6255)
	SJobProflingRenderData * arrJobProfilingRenderData = (SJobProflingRenderData*)alloca(nNumJobs * sizeof(SJobProflingRenderData));
	memset(arrJobProfilingRenderData, 0, nNumJobs * sizeof(SJobProflingRenderData));

	// init job data
	i32 nJobIndex = 0;
	for (std::set<JobUpr::SJobStringHandle>::const_iterator it = m_registeredJobs.begin(); it != m_registeredJobs.end(); ++it)
	{
		arrJobProfilingRenderData[nJobIndex].pName = it->cpString;
		arrJobProfilingRenderData[nJobIndex].color = m_JobColors[*it];
		++nJobIndex;
	}

	std::sort(arrJobProfilingRenderData, arrJobProfilingRenderData + nNumJobs);

	CTimeValue waitTimeMainThread;
	CTimeValue waitTimeRenderThread;

	// ==== collect per job/thread times == //
	for (i32 j = 0; j < 2; ++j)
	{
		// select the right frame for the pass
		i32 nIdx = (j == 0 ? m_profilingData.GetPrevRenderFrameIdx() : m_profilingData.GetRenderFrameIdx());
		for (u32 i = 0; i < m_profilingData.nProfilingDataCounter[nIdx] && i < SJobProfilingDataContainer::nCapturedEntriesPerFrame; ++i)
		{
			SJobProfilingData profilingData = m_profilingData.arrJobProfilingData[nIdx][i];

			// skip invalid entries
			IF (profilingData.jobHandle == NULL, 0)
				continue;

			// skip jobs which did never run
			IF (profilingData.nEndTime.GetValue() == 0 || profilingData.nStartTime.GetValue() == 0, 0)
				continue;

			// get the job profiling rendering data structure
			SJobProflingRenderData* pJobProfilingRenderingData =
			  std::lower_bound(arrJobProfilingRenderData, arrJobProfilingRenderData + nNumJobs, profilingData.jobHandle->cpString, SJobProflingRenderDataCmp());

			// did part of the job run during this frame
			if ((profilingData.nEndTime <= frameEndTime && profilingData.nStartTime >= frameStartTime) ||                                            // in this frame
			    (profilingData.nEndTime >= frameStartTime && profilingData.nEndTime <= frameEndTime && profilingData.nStartTime < frameStartTime) || // from the last frame into this
			    (profilingData.nStartTime >= frameStartTime && profilingData.nStartTime <= frameEndTime && profilingData.nEndTime > frameEndTime))   // goes into the next frame
			{
				// clamp frame start/end into this frame
				profilingData.nEndTime = (profilingData.nEndTime > frameEndTime ? frameEndTime : profilingData.nEndTime);
				profilingData.nStartTime = (profilingData.nStartTime < frameStartTime ? frameStartTime : profilingData.nStartTime);

				// compute integer offset in the graph for start and stop position
				CTimeValue startOffset = profilingData.nStartTime - frameStartTime;
				CTimeValue endOffset = profilingData.nEndTime - frameStartTime;

				i32 nGraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
				i32 nGraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);

				// get the correct worker id
				u32 nWorkerIdx = profilingData.nWorkerThread;

				// accumulate the time spend in dispatch(time the job waited to run)
				pJobProfilingRenderingData->runTime += (profilingData.nEndTime - profilingData.nStartTime);

				// count job invocations
				pJobProfilingRenderingData->invocations += 1;

				// accumulate time per worker thread
				arrWorkerProfilingRenderData[nWorkerIdx].runTime += (profilingData.nEndTime - profilingData.nStartTime);
				if (nGraphOffsetEnd < nGraphSize)
					DrawUtils::AddToGraph(arrWorkerThreadsRegions[nWorkerIdx], SOrderedProfilingData(pJobProfilingRenderingData->color, nGraphOffsetStart, nGraphOffsetEnd));
			}

			// did this job have wait time in this frame
			if (((profilingData.nWaitEnd.GetValue() - profilingData.nWaitBegin.GetValue()) > 1) &&
			    ((profilingData.nWaitEnd <= frameEndTime && profilingData.nWaitBegin >= frameStartTime) ||                                            // in this frame
			     (profilingData.nWaitEnd >= frameStartTime && profilingData.nWaitEnd <= frameEndTime && profilingData.nWaitBegin < frameStartTime) || // from the last frame into this
			     (profilingData.nWaitBegin >= frameStartTime && profilingData.nWaitBegin <= frameEndTime && profilingData.nWaitEnd > frameEndTime)))  // goes into the next frame
			{
				// clamp frame start/end into this frame
				profilingData.nWaitEnd = (profilingData.nWaitEnd > frameEndTime ? frameEndTime : profilingData.nWaitEnd);
				profilingData.nWaitBegin = (profilingData.nWaitBegin < frameStartTime ? frameStartTime : profilingData.nWaitBegin);

				// accumulate wait time
				pJobProfilingRenderingData->waitTime += (profilingData.nWaitEnd - profilingData.nWaitBegin);

				// compute graph offsets
				CTimeValue startOffset = profilingData.nWaitBegin - frameStartTime;
				CTimeValue endOffset = profilingData.nWaitEnd - frameStartTime;

				i32 nGraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
				i32 nGraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);

				// add to the right thread(we care only for main and renderthread)
				if (profilingData.nThreadId == nMainThreadId)
				{
					if (nGraphOffsetEnd < nGraphSize)
						DrawUtils::AddToGraph(arrMainThreadWaitRegion, SOrderedProfilingData(pJobProfilingRenderingData->color, nGraphOffsetStart, nGraphOffsetEnd));
					waitTimeMainThread += (profilingData.nWaitEnd - profilingData.nWaitBegin);
				}
				else if (profilingData.nThreadId == nRenderThreadId)
				{
					if (nGraphOffsetEnd < nGraphSize)
						DrawUtils::AddToGraph(arrRenderThreadWaitRegion, SOrderedProfilingData(pJobProfilingRenderingData->color, nGraphOffsetStart, nGraphOffsetEnd));
					waitTimeRenderThread += (profilingData.nWaitEnd - profilingData.nWaitBegin);
				}
			}
		}
	}

	// ==== collect mainthread regions ==== //
	if (m_nMainThreadMarkerIndex[nFrameId])
	{
		u32 nStackPos = 0;
		PREFAST_SUPPRESS_WARNING(6255)
		SMarker * pStack = (SMarker*)alloca(m_nMainThreadMarkerIndex[nFrameId] * sizeof(SMarker));
		for (u32 i = 0; i < m_nMainThreadMarkerIndex[nFrameId]; ++i)
			new(&pStack[i])SMarker();

		for (u32 nInputPos = 0; nInputPos < m_nMainThreadMarkerIndex[nFrameId]; ++nInputPos)
		{
			SMarker& rCurrentMarker = m_arrMainThreadMarker[nFrameId][nInputPos];
			if (rCurrentMarker.type == SMarker::POP_MARKER && nStackPos > 0) // end of marker, pop it from the stack
			{
				CTimeValue startOffset = pStack[nStackPos - 1].time - frameStartTime;
				CTimeValue endOffset = rCurrentMarker.time - frameStartTime;

				i32 GraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
				i32 GraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);
				if (GraphOffsetEnd < nGraphSize)
					DrawUtils::AddToGraph(arrMainThreadRegion, SOrderedProfilingData(GetRegionColor(pStack[nStackPos - 1].marker), GraphOffsetStart, GraphOffsetEnd));

				// accumulate global time
				SRegionTime cmp = { pStack[nStackPos - 1].marker, ColorB(), CTimeValue(), false };
				SRegionTime* pRegionProfilingData = std::lower_bound(arrRegionProfilingData, arrRegionProfilingData + nNumRegions, cmp);
				pRegionProfilingData->executionTime += (rCurrentMarker.time - pStack[nStackPos - 1].time);

				// pop last elemnt from stack, and update parent time
				nStackPos -= 1;
				if (nStackPos > 0)
					pStack[nStackPos - 1].time = rCurrentMarker.time;
			}
			if (rCurrentMarker.type == SMarker::PUSH_MARKER)
			{
				if (nStackPos > 0) // only draw last segment if there was one
				{
					CTimeValue startOffset = pStack[nStackPos - 1].time - frameStartTime;
					CTimeValue endOffset = rCurrentMarker.time - frameStartTime;

					i32 GraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
					i32 GraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);
					if (GraphOffsetEnd < nGraphSize)
						DrawUtils::AddToGraph(arrMainThreadRegion, SOrderedProfilingData(GetRegionColor(pStack[nStackPos - 1].marker), GraphOffsetStart, GraphOffsetEnd));

					// accumulate global time
					SRegionTime cmp = { pStack[nStackPos - 1].marker, ColorB(), CTimeValue(), false };
					SRegionTime* pRegionProfilingData = std::lower_bound(arrRegionProfilingData, arrRegionProfilingData + nNumRegions, cmp);
					pRegionProfilingData->executionTime += (rCurrentMarker.time - pStack[nStackPos - 1].time);
				}

				// push marker to stack
				pStack[nStackPos++] = rCurrentMarker;
			}
		}
	}
	// ==== collect renderthread regions ==== //
	if (m_nRenderThreadMarkerIndex[nFrameId])
	{
		u32 nStackPos = 0;
		PREFAST_SUPPRESS_WARNING(6255)
		SMarker * pStack = (SMarker*)alloca(m_nRenderThreadMarkerIndex[nFrameId] * sizeof(SMarker));
		for (u32 i = 0; i < m_nRenderThreadMarkerIndex[nFrameId]; ++i)
			new(&pStack[i])SMarker();

		for (u32 nInputPos = 0; nInputPos < m_nRenderThreadMarkerIndex[nFrameId]; ++nInputPos)
		{
			SMarker& rCurrentMarker = m_arrRenderThreadMarker[nFrameId][nInputPos];
			if (rCurrentMarker.type == SMarker::POP_MARKER && nStackPos > 0) // end of marker, pop it from the stack
			{
				CTimeValue startOffset = pStack[nStackPos - 1].time - frameStartTime;
				CTimeValue endOffset = rCurrentMarker.time - frameStartTime;

				i32 GraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
				i32 GraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);
				if (GraphOffsetEnd < nGraphSize)
					DrawUtils::AddToGraph(arrRenderThreadRegion, SOrderedProfilingData(GetRegionColor(pStack[nStackPos - 1].marker), GraphOffsetStart, GraphOffsetEnd));

				// accumulate global time
				SRegionTime cmp = { pStack[nStackPos - 1].marker, ColorB(), CTimeValue(), false };
				SRegionTime* pRegionProfilingData = std::lower_bound(arrRegionProfilingData, arrRegionProfilingData + nNumRegions, cmp);
				pRegionProfilingData->executionTime += (rCurrentMarker.time - pStack[nStackPos - 1].time);

				// pop last elemnt from stack, and update parent time
				nStackPos -= 1;
				if (nStackPos > 0)
					pStack[nStackPos - 1].time = rCurrentMarker.time;
			}
			if (rCurrentMarker.type == SMarker::PUSH_MARKER)
			{
				if (nStackPos > 0) // only draw last segment if there was one
				{
					CTimeValue startOffset = pStack[nStackPos - 1].time - frameStartTime;
					CTimeValue endOffset = rCurrentMarker.time - frameStartTime;

					i32 GraphOffsetStart = (i32)(startOffset.GetValue() * pixelPerTime);
					i32 GraphOffsetEnd = (i32)(endOffset.GetValue() * pixelPerTime);
					if (GraphOffsetEnd < nGraphSize)
						DrawUtils::AddToGraph(arrRenderThreadRegion, SOrderedProfilingData(GetRegionColor(pStack[nStackPos - 1].marker), GraphOffsetStart, GraphOffsetEnd));

					// accumulate global time
					SRegionTime cmp = { pStack[nStackPos - 1].marker, ColorB(), CTimeValue(), false };
					SRegionTime* pRegionProfilingData = std::lower_bound(arrRegionProfilingData, arrRegionProfilingData + nNumRegions, cmp);
					pRegionProfilingData->executionTime += (rCurrentMarker.time - pStack[nStackPos - 1].time);
				}

				// push marker to stack
				pStack[nStackPos++] = rCurrentMarker;
			}
		}
	}

	// ==== begin rendering of profiling data ==== //
	// render profiling data
	IRenderAuxGeom* pAuxGeomRenderer = gEnv->pRenderer->GetIRenderAuxGeom();
	SAuxGeomRenderFlags const oOldFlags = pAuxGeomRenderer->GetRenderFlags();

	SAuxGeomRenderFlags oFlags(e_Def2DPublicRenderflags);
	oFlags.SetDepthTestFlag(e_DepthTestOff);
	oFlags.SetDepthWriteFlag(e_DepthWriteOff);
	oFlags.SetCullMode(e_CullModeNone);
	oFlags.SetAlphaBlendMode(e_AlphaNone);
	pAuxGeomRenderer->SetRenderFlags(oFlags);

	float fGraphTopOffset = fTopOffset;
	// == main thread == //
	// draw main thread box and label
	MyDraw2dLabel(fInfoBoxSize + fTextSideOffset, fGraphTopOffset, fTextSize, fTextColor, false, "MainThread");
	DrawUtils::Draw2DBoxOutLine(fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight, fGraphWidth, boxBorderColor, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
	DrawUtils::DrawGraph(arrMainThreadRegion, nGraphSize, fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight / 2.0f, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
	DrawUtils::DrawGraph(arrMainThreadWaitRegion, nGraphSize, fInfoBoxSize + fGraphSideOffset, fGraphTopOffset + fGraphHeight / 2.0f, fGraphHeight / 2.0f, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
	fGraphTopOffset += fGraphHeight + 2;

	// == render thread == //
	MyDraw2dLabel(fInfoBoxSize + fTextSideOffset, fGraphTopOffset, fTextSize, fTextColor, false, "RenderThread");
	DrawUtils::Draw2DBoxOutLine(fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight, fGraphWidth, boxBorderColor, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
	DrawUtils::DrawGraph(arrRenderThreadRegion, nGraphSize, fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight / 2.0f, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
	DrawUtils::DrawGraph(arrRenderThreadWaitRegion, nGraphSize, fInfoBoxSize + fGraphSideOffset, fGraphTopOffset + fGraphHeight / 2.0f, fGraphHeight / 2.0f, fScreenHeight, fScreenWidth, pAuxGeomRenderer);

	// == worker threads == //
	fGraphTopOffset += 2.0f * fGraphHeight; // add a little bit more spacing between mainthreads and worker
	for (i32 i = 0; i < nNumWorker; ++i)
	{
		// draw worker box and label
		char workerThreadName[128];
		drx_sprintf(workerThreadName, "Worker %d", i);
		MyDraw2dLabel(fInfoBoxSize + fTextSideOffset, fGraphTopOffset, fTextSize, fTextColor, false, "%s", workerThreadName);
		DrawUtils::Draw2DBoxOutLine(fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight, fGraphWidth, boxBorderColor, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
		DrawUtils::DrawGraph(arrWorkerThreadsRegions[i], nGraphSize, fInfoBoxSize + fGraphSideOffset, fGraphTopOffset, fGraphHeight, fScreenHeight, fScreenWidth, pAuxGeomRenderer);

		fGraphTopOffset += fGraphHeight + 2;
	}
	fGraphTopOffset += 2.0f * fGraphHeight; // add a little bit after the worker threads

	// are we only interrested in the graph and not in the values?
	if (nJobSystemProfiler == 2)
	{
		// Restore Aux Render setup
		pAuxGeomRenderer->SetRenderFlags(oOldFlags);
		return;
	}

	// == draw info boxes == //
	char tmpBuffer[512] = { 0 };
	float fInfoBoxTextOffset = fTopOffset;

	// draw worker data
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, "Workers", 27);

	fInfoBoxTextOffset += fTextSizePixel;
	CTimeValue accumulatedWorkerTime;
	for (i32 i = 0; i < nNumWorker; ++i)
	{
		float runTimePercent = 100.0f / (frameEndTime - frameStartTime).GetValue() * arrWorkerProfilingRenderData[i].runTime.GetValue();
		drx_sprintf(tmpBuffer, "  Worker %d: %05.2f ms %04.1f p", i, arrWorkerProfilingRenderData[i].runTime.GetMilliSeconds(), runTimePercent);
		DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
		fInfoBoxTextOffset += fTextSizePixel;

		// accumulate times for all worker
		accumulatedWorkerTime += arrWorkerProfilingRenderData[i].runTime;
	}

	// draw accumulated worker time and percentage
	drx_sprintf(tmpBuffer, "-------------------------------");
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
	fInfoBoxTextOffset += fTextSizePixel;
	float accRunTimePercentage = 100.0f / ((frameEndTime - frameStartTime).GetValue() * nNumWorker) * accumulatedWorkerTime.GetValue();
	drx_sprintf(tmpBuffer, "Sum: %05.2f ms %04.1f p", accumulatedWorkerTime.GetMilliSeconds(), accRunTimePercentage);
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
	fInfoBoxTextOffset += fTextSizePixel;

	// draw accumulated wait times of main and renderthread
	fInfoBoxTextOffset += 2.0f * fTextSizePixel;
	drx_sprintf(tmpBuffer, "MainThread Wait %05.2f ms", waitTimeMainThread.GetMilliSeconds());
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
	fInfoBoxTextOffset += fTextSizePixel;
	drx_sprintf(tmpBuffer, "RenderThread Wait %05.2f ms", waitTimeRenderThread.GetMilliSeconds());
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
	fInfoBoxTextOffset += fTextSizePixel;
	drx_sprintf(tmpBuffer, "MainThread %05.2f ms", (frameEndTime - frameStartTime).GetMilliSeconds());
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 27);
	fInfoBoxTextOffset += fTextSizePixel;
	fInfoBoxTextOffset += fTextSizePixel;

	// sort regions by time
	std::sort(arrRegionProfilingData, arrRegionProfilingData + nNumRegions, SRegionLexicalSorter());

	fTopOffset = fInfoBoxTextOffset += fTextSizePixel;
	drx_sprintf(tmpBuffer, " Name                Time(MS)");
	DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 80);
	fInfoBoxTextOffset += 1.4f * fTextSizePixel;
	for (i32 i = 0; i < nNumRegions; ++i)
	{
		if (!arrRegionProfilingData[i].bIsMainThread) // for now don't write names for RT regions
			continue;

		// do we need to restart a new colum
		if (fInfoBoxTextOffset + (fTextSize * fTextSizePixel) > (fScreenHeight * 0.99f))
		{
			fInfoBoxTextOffset = fTopOffset;
			fTextSideOffset += fTextSizePixel * 25; // keep a little space between the bars
			drx_sprintf(tmpBuffer, " Name                Time(MS)");
			DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 80);
			fInfoBoxTextOffset += 1.4f * fTextSizePixel;
		}

		drx_sprintf(tmpBuffer, " %-21.21s %05.2f ", arrRegionProfilingData[i].pName.c_str(), arrRegionProfilingData[i].executionTime.GetMilliSeconds());
		DrawUtils::WriteShortLabel(fTextSideOffset, fInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 28);
		DrawUtils::Draw2DBox(fTextSideOffset, fInfoBoxTextOffset + 2.0f, fTextSizePixel * 1.25f, 30 * fTextCharWidth, arrRegionProfilingData[i].color, fScreenHeight, fScreenWidth, pAuxGeomRenderer);

		fInfoBoxTextOffset += fTextSizePixel * 1.5f;
	}

	// == render per job informations == //
	float fJobInfoBoxTextOffset = fTopOffset;
	float fJobInfoBoxSideOffset = max(fTextSideOffset += fTextSizePixel * 25, fScreenWidth * 0.5f);
	float fJobInfoBoxTextWidth = fTextCharWidth * 80;

	// sort jobs by their name
	std::sort(arrJobProfilingRenderData, arrJobProfilingRenderData + nNumJobs, SJobProfilingLexicalSort());

	drx_sprintf(tmpBuffer, " JobName                  (Num Invocations) TimeExecuted(MS) TimeWait(MS) AvG(MS)");
	DrawUtils::WriteShortLabel(fJobInfoBoxSideOffset, fJobInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 80);
	fJobInfoBoxTextOffset += 1.4f * fTextSizePixel;

	for (i32 i = 0; i < nNumJobs; ++i)
	{
		// do we need to restart a new colum
		if (fJobInfoBoxTextOffset + (fTextSize * fTextSizePixel) > (fScreenHeight * 0.99f))
		{
			fJobInfoBoxTextOffset = fTopOffset;
			fJobInfoBoxSideOffset += fTextCharWidth * 85; // keep a little space between the bars
			drx_sprintf(tmpBuffer, " JobName                  (Num Invocations) TimeExecuted(MS) TimeWait(MS) AvG(MS)");
			DrawUtils::WriteShortLabel(fJobInfoBoxSideOffset, fJobInfoBoxTextOffset, fTextSize, fTextColor, tmpBuffer, 80);
			fJobInfoBoxTextOffset += 1.4f * fTextSizePixel;
		}

		SJobProflingRenderData& rJobProfilingData = arrJobProfilingRenderData[i];
		drx_sprintf(tmpBuffer, " %-35.35s (%3d):      %5.2f      %5.2f         %5.2f", rJobProfilingData.pName, rJobProfilingData.invocations,
		            rJobProfilingData.runTime.GetMilliSeconds(), rJobProfilingData.waitTime.GetMilliSeconds(),
		            rJobProfilingData.invocations ? rJobProfilingData.runTime.GetMilliSeconds() / rJobProfilingData.invocations : 0.0f);

		DrawUtils::WriteShortLabel(fJobInfoBoxSideOffset, fJobInfoBoxTextOffset - 1, fTextSize, fTextColor, tmpBuffer, 80);
		DrawUtils::Draw2DBox(fJobInfoBoxSideOffset, fJobInfoBoxTextOffset + 2.0f, fTextSizePixel * 1.25f, fJobInfoBoxTextWidth, rJobProfilingData.color, fScreenHeight, fScreenWidth, pAuxGeomRenderer);
		fJobInfoBoxTextOffset += fTextSizePixel * 1.5f;

	}

	// Restore Aux Render setup
	pAuxGeomRenderer->SetRenderFlags(oOldFlags);

#endif
}

void JobUpr::CJobUpr::SetFrameStartTime(const CTimeValue& rFrameStartTime)
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	if (!m_bJobSystemProfilerPaused)
		++m_profilingData.nFrameIdx;

	i32 idx = m_profilingData.GetFillFrameIdx();
	m_FrameStartTime[idx] = rFrameStartTime;
	// reset profiling counter
	m_profilingData.nProfilingDataCounter[idx] = 0;

	// clear marker
	m_nMainThreadMarkerIndex[idx] = 0;
	m_nRenderThreadMarkerIndex[idx] = 0;
#endif

}

void JobUpr::CJobUpr::PushProfilingMarker(tukk pName)
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	// get used thread ids
	static threadID nMainThreadId = ~0;
	static threadID nRenderThreadId = ~0;
	static bool bInitialized = false;
	IF (!bInitialized, 0)
	{
		if (!gEnv->pRenderer)
			return;
		gEnv->pRenderer->GetThreadIDs(nMainThreadId, nRenderThreadId);
		bInitialized = true;
	}

	threadID nThreadID = DrxGetCurrentThreadId();
	u32 nFrameIdx = m_profilingData.GetFillFrameIdx();
	if (nThreadID == nMainThreadId && m_nMainThreadMarkerIndex[nFrameIdx] < nMarkerEntries)
		m_arrMainThreadMarker[nFrameIdx][m_nMainThreadMarkerIndex[nFrameIdx]++] = SMarker(SMarker::PUSH_MARKER, pName, gEnv->pTimer->GetAsyncTime(), true);
	if (nThreadID == nRenderThreadId && m_nRenderThreadMarkerIndex[nFrameIdx] < nMarkerEntries)
		m_arrRenderThreadMarker[nFrameIdx][m_nRenderThreadMarkerIndex[nFrameIdx]++] = SMarker(SMarker::PUSH_MARKER, pName, gEnv->pTimer->GetAsyncTime(), false);
#endif
}

void JobUpr::CJobUpr::PopProfilingMarker()
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	// get used thread ids
	static threadID nMainThreadId = ~0;
	static threadID nRenderThreadId = ~0;
	static bool bInitialized = false;
	IF (!bInitialized, 0)
	{
		if (!gEnv->pRenderer)
			return;
		gEnv->pRenderer->GetThreadIDs(nMainThreadId, nRenderThreadId);
		bInitialized = true;
	}

	threadID nThreadID = DrxGetCurrentThreadId();
	u32 nFrameIdx = m_profilingData.GetFillFrameIdx();
	if (nThreadID == nMainThreadId && m_nMainThreadMarkerIndex[nFrameIdx] < nMarkerEntries)
		m_arrMainThreadMarker[nFrameIdx][m_nMainThreadMarkerIndex[nFrameIdx]++] = SMarker(SMarker::POP_MARKER, gEnv->pTimer->GetAsyncTime(), true);
	if (nThreadID == nRenderThreadId && m_nRenderThreadMarkerIndex[nFrameIdx] < nMarkerEntries)
		m_arrRenderThreadMarker[nFrameIdx][m_nRenderThreadMarkerIndex[nFrameIdx]++] = SMarker(SMarker::POP_MARKER, gEnv->pTimer->GetAsyncTime(), false);
#endif
}

ColorB JobUpr::CJobUpr::GetRegionColor(SMarker::TMarkerString marker)
{
#if defined(JOBMANAGER_SUPPORT_PROFILING)
	if (m_RegionColors.find(marker) == m_RegionColors.end())
	{
		m_RegionColors[marker] = GenerateColorBasedOnName(marker.c_str());
	}
	return m_RegionColors[marker];
#else
	return ColorB();
#endif
}

///////////////////////////////////////////////////////////////////////////////
JobUpr::TSemaphoreHandle JobUpr::CJobUpr::AllocateSemaphore( ukk pOwner)
{
	// static checks
	STATIC_CHECK(sizeof(JobUpr::TSemaphoreHandle) == 2, ERROR_SIZE_OF_SEMAPHORE_HANDLE_IS_NOT_2);
	STATIC_CHECK(static_cast<i32>(nSemaphorePoolSize) < JobUpr::Detail::nSemaphoreSaltBit, ERROR_SEMAPHORE_POOL_IS_BIGGER_THAN_SALT_HANDLE);
	STATIC_CHECK(IsPowerOfTwoCompileTime<nSemaphorePoolSize>::IsPowerOfTwo, ERROR_SEMAPHORE_SIZE_IS_NOT_POWER_OF_TWO);

	AUTO_LOCK(m_JobUprLock);
	i32 nSpinCount = 0;
	for (;; ) // normally we should never spin here, if we do, increase the semaphore pool size
	{
		if (nSpinCount > 10)
			__debugbreak(); // breaking here means that there is a logic flaw which causes not finished syncvars to be returned to the pool

		u32 nIndex = (++m_nCurrentSemaphoreIndex) % nSemaphorePoolSize;
		SJobFinishedConditionVariable* pSemaphore = &m_JobSemaphorePool[nIndex];
		if (pSemaphore->HasOwner())
		{
			nSpinCount++;
			continue;
		}

		// set owner and increase ref counter
		pSemaphore->SetRunning();
		pSemaphore->SetOwner(pOwner);
		pSemaphore->AddRef(pOwner);
		return JobUpr::Detail::IndexToSemaphoreHandle(nIndex);
	}
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CJobUpr::DeallocateSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner)
{
	AUTO_LOCK(m_JobUprLock);
	u32 nIndex = JobUpr::Detail::SemaphoreHandleToIndex(nSemaphoreHandle);
	assert(nIndex < nSemaphorePoolSize);

	SJobFinishedConditionVariable* pSemaphore = &m_JobSemaphorePool[nIndex];

	if (pSemaphore->DecRef(pOwner) == 0)
	{
		if (pSemaphore->IsRunning())
			__debugbreak();
		pSemaphore->ClearOwner(pOwner);
	}
}

///////////////////////////////////////////////////////////////////////////////
bool JobUpr::CJobUpr::AddRefSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner)
{
	AUTO_LOCK(m_JobUprLock);
	u32 nIndex = JobUpr::Detail::SemaphoreHandleToIndex(nSemaphoreHandle);
	assert(nIndex < nSemaphorePoolSize);

	SJobFinishedConditionVariable* pSemaphore = &m_JobSemaphorePool[nIndex];

	return pSemaphore->AddRef(pOwner);
}

///////////////////////////////////////////////////////////////////////////////
SJobFinishedConditionVariable* JobUpr::CJobUpr::GetSemaphore(JobUpr::TSemaphoreHandle nSemaphoreHandle,  ukk pOwner)
{
	u32 nIndex = JobUpr::Detail::SemaphoreHandleToIndex(nSemaphoreHandle);
	assert(nIndex < nSemaphorePoolSize);

	return &m_JobSemaphorePool[nIndex];
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CJobUpr::DumpJobList()
{
	i32 i = 1;
	DrxLogAlways("== JobUpr registered Job List ==");
	for (std::set<JobUpr::SJobStringHandle>::iterator it = m_registeredJobs.begin(); it != m_registeredJobs.end(); ++it)
	{
		DrxLogAlways("%3d. %s", i++, it->cpString);
	}
}

//////////////////////////////////////////////////////////////////////////
bool JobUpr::CJobUpr::OnInputEvent(const SInputEvent& event)
{
	bool ret = false;

	// Only process keyboard input
	if (eIDT_Keyboard == event.deviceType)
	{
		// Only if key was pressed
		if (eIS_Pressed == event.state)
		{
			switch (event.keyId)
			{
			case eKI_ScrollLock: // Pause/Continue profiler data collection with Scroll Lock key
				m_bJobSystemProfilerPaused = !m_bJobSystemProfilerPaused;
				break;
			}
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::CJobUpr::AddBlockingFallbackJob(JobUpr::SInfoBlock* pInfoBlock, u32 nWorkerThreadID)
{
	assert(m_pFallBackBackEnd);
	static_cast<BlockingBackEnd::CBlockingBackEnd*>(m_pBlockingBackEnd)->AddBlockingFallbackJob(pInfoBlock, nWorkerThreadID);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TLS_DEFINE(u32, gWorkerThreadId);

///////////////////////////////////////////////////////////////////////////////
namespace JobUpr {
namespace detail {

enum { eWorkerThreadMarker = 0x80000000 };
u32 mark_worker_thread_id(u32 nWorkerThreadID)      { return nWorkerThreadID | eWorkerThreadMarker; }
u32 unmark_worker_thread_id(u32 nWorkerThreadID)    { return nWorkerThreadID & ~eWorkerThreadMarker; }
bool   is_marked_worker_thread_id(u32 nWorkerThreadID) { return (nWorkerThreadID & eWorkerThreadMarker) != 0; }
} // namespace detail
} // namespace JobUpr

///////////////////////////////////////////////////////////////////////////////
void JobUpr::detail::SetWorkerThreadId(u32 nWorkerThreadId)
{
	TLS_SET(gWorkerThreadId, (size_t)mark_worker_thread_id(nWorkerThreadId));
}

///////////////////////////////////////////////////////////////////////////////
u32 JobUpr::detail::GetWorkerThreadId()
{
	u32 nID = (u32)TLS_GET(uintptr_t, gWorkerThreadId);
	return is_marked_worker_thread_id(nID) ? unmark_worker_thread_id(nID) : ~0;
}

static BoundMPMC<JobUpr::SInfoBlock*> gFallbackInfoBlocks(JobUpr::detail::GetFallbackJobListSize());

///////////////////////////////////////////////////////////////////////////////
void JobUpr::detail::PushToFallbackJobList(JobUpr::SInfoBlock* pInfoBlock)
{
	bool ret = gFallbackInfoBlocks.enqueue(pInfoBlock);
	DRX_ASSERT_MESSAGE(ret, "JobSystem: Fallback info block limit reached");
}

JobUpr::SInfoBlock* JobUpr::detail::PopFromFallbackJobList()
{
	JobUpr::SInfoBlock* pInfoBlock = nullptr;
	return gFallbackInfoBlocks.dequeue(pInfoBlock) ? pInfoBlock : nullptr;
}
