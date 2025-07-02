// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ThreadBackEnd.h
//  Version:     v1.00
//  Created:     07/05/2011 by Christopher Bolte
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ThreadBackEnd.h>
#include <drx3D/Sys/JobUpr.h>
#include <drx3D/Sys/System.h>
#include <drx3D/Sys/CPUDetect.h>

#if DRX_PLATFORM_WINDOWS
	#include <intrin.h>
#endif

///////////////////////////////////////////////////////////////////////////////
JobUpr::ThreadBackEnd::CThreadBackEnd::CThreadBackEnd()
	: m_Semaphore(SJobQueue_ThreadBackEnd::eMaxWorkQueueJobsSize + JobUpr::detail::GetFallbackJobListSize())
	, m_nNumWorkerThreads(0)
{
	m_JobQueue.Init();

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	m_pBackEndWorkerProfiler = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
JobUpr::ThreadBackEnd::CThreadBackEnd::~CThreadBackEnd()
{
}

///////////////////////////////////////////////////////////////////////////////
bool JobUpr::ThreadBackEnd::CThreadBackEnd::Init(u32 nSysMaxWorker)
{

	// find out how many workers to create
#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS
	u32k nNumCores = 4;
#else
	CCpuFeatures* pCPU = ((CSystem*)gEnv->pSystem)->GetCPUFeatures();
	u32k nNumCores = std::max(pCPU->GetLogicalCPUCount() - 2u, 2u);
#endif

	u32 nNumWorkerToCreate = 0;

	if (nSysMaxWorker)
		nNumWorkerToCreate = std::min(nSysMaxWorker, nNumCores);
	else
		nNumWorkerToCreate = nNumCores;

	if (nNumWorkerToCreate == 0)
		return false;

	m_nNumWorkerThreads = nNumWorkerToCreate;

	m_arrWorkerThreads.resize(nNumWorkerToCreate);

	for (u32 i = 0; i < nNumWorkerToCreate; ++i)
	{
		m_arrWorkerThreads[i] = new CThreadBackEndWorkerThread(this, m_Semaphore, m_JobQueue, i);

		if (!gEnv->pThreadUpr->SpawnThread(m_arrWorkerThreads[i], "JobSystem_Worker_%u", i))
		{
			DrxFatalError("Error spawning \"JobSystem_Worker_%u\" thread.", i);
		}
	}
#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	m_pBackEndWorkerProfiler = new JobUpr::CWorkerBackEndProfiler;
	m_pBackEndWorkerProfiler->Init(nNumWorkerToCreate);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool JobUpr::ThreadBackEnd::CThreadBackEnd::ShutDown()
{
	// 1. Signal all threads to stop
	u32 numOfStoppedThreads = 0;
	for (u32 i = 0; i < m_arrWorkerThreads.size(); ++i)
	{
		if (m_arrWorkerThreads[i] == NULL)
			continue;
		m_arrWorkerThreads[i]->SignalStopWork();
		++numOfStoppedThreads;
	}

	// 2. Release semaphore count to wake up some/all threads waiting on the semaphore
	for (u32 i = 0; i < numOfStoppedThreads; ++i)
	{
		m_Semaphore.SignalNewJob();
	}

	// 3. Wait for threads to exit and delete worker thread
	for (u32 i = 0; i < m_arrWorkerThreads.size(); ++i)
	{
		if (m_arrWorkerThreads[i] == NULL)
			continue;

		if (gEnv->pThreadUpr->JoinThread(m_arrWorkerThreads[i], eJM_Join))
		{
			delete m_arrWorkerThreads[i];
			m_arrWorkerThreads[i] = NULL;
		}
	}

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	SAFE_DELETE(m_pBackEndWorkerProfiler);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::ThreadBackEnd::CThreadBackEnd::AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock)
{
	u32 nJobPriority = crJob.GetPriorityLevel();
	CJobUpr* __restrict pJobUpr = CJobUpr::Instance();

	/////////////////////////////////////////////////////////////////////////////
	// Acquire Infoblock to use
	u32 jobSlot;
	JobUpr::SInfoBlock* pFallbackInfoBlock = NULL;
	// only wait for a jobslot if we are submitting from a regular thread, or if we are submitting
	// a blocking job from a regular worker thread
	bool bWaitForFreeJobSlot = (JobUpr::IsWorkerThread() == false) && (JobUpr::IsBlockingWorkerThread() == false);
	JobUpr::detail::EAddJobRes cEnqRes = m_JobQueue.GetJobSlot(jobSlot, nJobPriority, bWaitForFreeJobSlot);

#if !defined(_RELEASE)
	pJobUpr->IncreaseRunJobs();
	if (cEnqRes == JobUpr::detail::eAJR_NeedFallbackJobInfoBlock)
		pJobUpr->IncreaseRunFallbackJobs();
#endif
	// allocate fallback infoblock if needed
	IF (cEnqRes == JobUpr::detail::eAJR_NeedFallbackJobInfoBlock, 0)
		pFallbackInfoBlock = new JobUpr::SInfoBlock();

	// copy info block into job queue
	PREFAST_ASSUME(pFallbackInfoBlock);
	JobUpr::SInfoBlock& RESTRICT_REFERENCE rJobInfoBlock = (cEnqRes == JobUpr::detail::eAJR_NeedFallbackJobInfoBlock ?
	                                                            *pFallbackInfoBlock : m_JobQueue.jobInfoBlocks[nJobPriority][jobSlot]);

	// since we will use the whole InfoBlock, and it is aligned to 128 bytes, clear the cacheline, this is faster than a cachemiss on write
#if DRX_PLATFORM_64BIT
	//STATIC_CHECK( sizeof(JobUpr::SInfoBlock) == 512, ERROR_SIZE_OF_SINFOBLOCK_NOT_EQUALS_512 );
#else
	//STATIC_CHECK( sizeof(JobUpr::SInfoBlock) == 384, ERROR_SIZE_OF_SINFOBLOCK_NOT_EQUALS_384 );
#endif

	// first cache line needs to be persistent
	ResetLine128(&rJobInfoBlock, 128);
	ResetLine128(&rJobInfoBlock, 256);
#if DRX_PLATFORM_64BIT
	ResetLine128(&rJobInfoBlock, 384);
#endif

	/////////////////////////////////////////////////////////////////////////////
	// Initialize the InfoBlock
	rInfoBlock.AssignMembersTo(&rJobInfoBlock);

	// copy job parameter if it is a non-queue job
	if (crJob.GetQueue() == NULL)
	{
		JobUpr::CJobUpr::CopyJobParameter(crJob.GetParamDataSize(), rJobInfoBlock.GetParamAddress(), crJob.GetJobParamData());
	}

	assert(rInfoBlock.jobInvoker);

	u32k cJobId = cJobHandle->jobId;
	rJobInfoBlock.jobId = (u8)cJobId;

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	assert(cJobId < JobUpr::detail::eJOB_FRAME_STATS_MAX_SUPP_JOBS);
	m_pBackEndWorkerProfiler->RegisterJob(cJobId, pJobUpr->GetJobName(rInfoBlock.jobInvoker));
	rJobInfoBlock.frameProfIndex = (u8)m_pBackEndWorkerProfiler->GetProfileIndex();
#endif

	/////////////////////////////////////////////////////////////////////////////
	// initialization finished, make all visible for worker threads
	// the producing thread won't need the info block anymore, flush it from the cache
	FlushLine128(&rJobInfoBlock, 0);
	FlushLine128(&rJobInfoBlock, 128);
#if DRX_PLATFORM_64BIT
	FlushLine128(&rJobInfoBlock, 256);
	FlushLine128(&rJobInfoBlock, 384);
#endif

	IF (cEnqRes == JobUpr::detail::eAJR_NeedFallbackJobInfoBlock, 0)
	{
		// catch submission from regular workers to the blocking backend
		if (crJob.IsBlocking())
		{
			pJobUpr->AddBlockingFallbackJob(pFallbackInfoBlock, JobUpr::GetWorkerThreadId());
		}
		else
		{
			JobUpr::detail::PushToFallbackJobList(&rJobInfoBlock);
		}
	}
	else
	{
		MemoryBarrier();
		m_JobQueue.jobInfoBlockStates[nJobPriority][jobSlot].SetReady();
	}
	// Release semaphore count to signal the workers that work is available
	m_Semaphore.SignalNewJob();
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::ThreadBackEnd::CThreadBackEndWorkerThread::SignalStopWork()
{
	m_bStop = true;
}

inline CFrameProfiler* GetFrameProfilerForName(tukk name)
{
	static std::vector<std::pair<tukk , CFrameProfiler*>> s_profilers;
	static DrxRWLock s_profilersLock;

	s_profilersLock.RLock();
	for (auto& p : s_profilers)
	{
		if (p.first == name) // compare pointer address. not content.
		{
			s_profilersLock.RUnlock();
			return p.second;
		}
	}
	s_profilersLock.RUnlock();


	s_profilersLock.WLock();
	for (auto& p : s_profilers)
	{
		if (p.first == name) // compare pointer address. not content.
		{
			s_profilersLock.WUnlock();
			return p.second;
		}
	}
	CFrameProfiler* pNewProfiler = new CFrameProfiler(PROFILE_SYSTEM, EProfileDescription::REGION, name, "", 0);
	s_profilers.reserve(256);
	s_profilers.push_back(std::make_pair(name, pNewProfiler));
	s_profilersLock.WUnlock();

	return pNewProfiler;
}

//////////////////////////////////////////////////////////////////////////
void JobUpr::ThreadBackEnd::CThreadBackEndWorkerThread::ThreadEntry()
{
	// set up thread id
	JobUpr::detail::SetWorkerThreadId(m_nId);

#if defined(JOB_SPIN_DURING_IDLE)
	HANDLE nThreadID = GetCurrentThread();
#endif

	LARGE_INTEGER freq;
	double frequency;
	QueryPerformanceFrequency(&freq);
	frequency = 1.f / static_cast<double>(freq.QuadPart);
	uint64 nTicksInJobExecution = 0;
	const float fMinTimeInJobExecution = 1.0f;

	CJobUpr* __restrict pJobUpr = CJobUpr::Instance();
	do
	{
		SInfoBlock infoBlock;
		CJobUpr* __restrict pJobUpr = CJobUpr::Instance();
		u32 nPriorityLevel = ~0;

		///////////////////////////////////////////////////////////////////////////
		// wait for new work
		// we will only do a real wait if jobs accumulated time was more
		// than fMinTimeInJobExecution ms, to prevent system calls when we
		// execute a massive number of small jobs
		{
			//DRX_PROFILE_REGION_WAITING(PROFILE_SYSTEM, "Wait - JobWorkerThread");

			float fMSInJobExecution = static_cast<float>(nTicksInJobExecution * 1000.0f * frequency);
			if (fMSInJobExecution > fMinTimeInJobExecution || !m_rSemaphore.TryGetJob())
			{
#if defined(JOB_SPIN_DURING_IDLE)
				SetThreadPriority(nThreadID, THREAD_PRIORITY_IDLE);
#endif
				m_rSemaphore.WaitForNewJob(m_nId);
#if defined(JOB_SPIN_DURING_IDLE)
				SetThreadPriority(nThreadID, THREAD_PRIORITY_TIME_CRITICAL);
#endif
				nTicksInJobExecution = 0;
			}
		}

		IF(m_bStop == true, 0)
			break;

		if (JobUpr::SInfoBlock* pFallbackInfoBlock = JobUpr::detail::PopFromFallbackJobList())
		{
			DRX_PROFILE_REGION(PROFILE_SYSTEM, "JobWorkerThread: Fallback");

			// in case of a fallback job, just get it from the global per thread list
			pFallbackInfoBlock->AssignMembersTo(&infoBlock);
			if (!infoBlock.HasQueue())  // copy parameters for non producer/consumer jobs
			{
				JobUpr::CJobUpr::CopyJobParameter(infoBlock.paramSize << 4, infoBlock.GetParamAddress(), pFallbackInfoBlock->GetParamAddress());
			}

			// free temp info block again
			delete pFallbackInfoBlock;
		}
		else
		{
			///////////////////////////////////////////////////////////////////////////
			// multiple steps to get a job of the queue
			// 1. get our job slot index
			uint64 currentPushIndex = ~0;
			uint64 currentPullIndex = ~0;
			uint64 newPullIndex = ~0;
			do
			{
				//  load
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID// emulate a 64bit atomic read on PC platfom
				currentPullIndex = DrxInterlockedCompareExchange64(alias_cast< int64*>(&m_rJobQueue.pull.index), 0, 0);
				currentPushIndex = DrxInterlockedCompareExchange64(alias_cast< int64*>(&m_rJobQueue.push.index), 0, 0);
#else
				currentPullIndex = *const_cast< uint64*>(&m_rJobQueue.pull.index);
				currentPushIndex = *const_cast< uint64*>(&m_rJobQueue.push.index);
#endif
				// spin if the updated push ptr didn't reach us yet
				if (currentPushIndex == currentPullIndex)
					continue;

				// compute priority level from difference between push/pull
				if (!JobUpr::SJobQueuePos::IncreasePullIndex(currentPullIndex, currentPushIndex, newPullIndex, nPriorityLevel,
				                                                 m_rJobQueue.GetMaxWorkerQueueJobs(eHighPriority), m_rJobQueue.GetMaxWorkerQueueJobs(eRegularPriority), m_rJobQueue.GetMaxWorkerQueueJobs(eLowPriority), m_rJobQueue.GetMaxWorkerQueueJobs(eStreamPriority)))
					continue;

				// stop spinning when we succesfull got the index
				if (DrxInterlockedCompareExchange64(alias_cast< int64*>(&m_rJobQueue.pull.index), newPullIndex, currentPullIndex) == currentPullIndex)
					break;

			}
			while (true);

			// compute our jobslot index from the only increasing publish index
			u32 nExtractedCurIndex = static_cast<u32>(JobUpr::SJobQueuePos::ExtractIndex(currentPullIndex, nPriorityLevel));
			u32 nNumWorkerQUeueJobs = m_rJobQueue.GetMaxWorkerQueueJobs(nPriorityLevel);
			u32 nJobSlot = nExtractedCurIndex & (nNumWorkerQUeueJobs - 1);

			// 2. Wait still the produces has finished writing all data to the SInfoBlock
			JobUpr::detail::SJobQueueSlotState* pJobInfoBlockState = &m_rJobQueue.jobInfoBlockStates[nPriorityLevel][nJobSlot];
			i32 iter = 0;
			while (!pJobInfoBlockState->IsReady())
			{
				DrxSleep(iter++ > 10 ? 1 : 0);
			}
			;

			// 3. Get a local copy of the info block as asson as it is ready to be used
			JobUpr::SInfoBlock* pCurrentJobSlot = &m_rJobQueue.jobInfoBlocks[nPriorityLevel][nJobSlot];
			pCurrentJobSlot->AssignMembersTo(&infoBlock);
			if (!infoBlock.HasQueue())  // copy parameters for non producer/consumer jobs
			{
				JobUpr::CJobUpr::CopyJobParameter(infoBlock.paramSize << 4, infoBlock.GetParamAddress(), pCurrentJobSlot->GetParamAddress());
			}

			// 4. Remark the job state as suspended
			MemoryBarrier();
			pJobInfoBlockState->SetNotReady();

			// 5. Mark the jobslot as free again
			MemoryBarrier();
			pCurrentJobSlot->Release((1 << JobUpr::SJobQueuePos::eBitsPerPriorityLevel) / m_rJobQueue.GetMaxWorkerQueueJobs(nPriorityLevel));
		}

		///////////////////////////////////////////////////////////////////////////
		// now we have a valid SInfoBlock to start work on it
		// check if it is a producer/consumer queue job
		IF (infoBlock.HasQueue(), 0)
		{
			DoWorkProducerConsumerQueue(infoBlock);
		}
		else
		{
			// Now we are safe to use the info block
			assert(infoBlock.jobInvoker);
			assert(infoBlock.GetParamAddress());

			// store job start time
#if defined(JOBMANAGER_SUPPORT_PROFILING)
			SJobProfilingData* pJobProfilingData = gEnv->GetJobUpr()->GetProfilingData(infoBlock.profilerIndex);
			pJobProfilingData->nStartTime = gEnv->pTimer->GetAsyncTime();
			pJobProfilingData->nWorkerThread = GetWorkerThreadId();
#endif

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
			const uint64 nStartTime = JobUpr::IWorkerBackEndProfiler::GetTimeSample();
#endif

			{
				// call delegator function to invoke job entry
#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
				tukk jobName = pJobUpr->GetJobName(infoBlock.jobInvoker);

				char job_info[128];
				CFrameProfiler* pProfiler = GetFrameProfilerForName(jobName);
				CFrameProfilerSection frameProfilerSection2(pProfiler, jobName, jobName, EProfileDescription::SECTION);
				BROFILER_SECTION(jobName)

				drx_sprintf(job_info, "%s (Prio %u)", jobName, nPriorityLevel);

				DRXPROFILE_SCOPE_PROFILE_MARKER(job_info);
				DRXPROFILE_SCOPE_PLATFORM_MARKER(job_info);
#endif

				uint64 nJobStartTicks = DrxGetTicks();

				if (infoBlock.jobLambdaInvoker)
				{
					infoBlock.jobLambdaInvoker();
				}
				else
				{
					(*infoBlock.jobInvoker)(infoBlock.GetParamAddress());
				}
				nTicksInJobExecution += DrxGetTicks() - nJobStartTicks;
			}

#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
			JobUpr::IWorkerBackEndProfiler* workerProfiler = m_pThreadBackend->GetBackEndWorkerProfiler();
			const uint64 nEndTime = JobUpr::IWorkerBackEndProfiler::GetTimeSample();
			workerProfiler->RecordJob(infoBlock.frameProfIndex, m_nId, static_cast<u32k>(infoBlock.jobId), static_cast<u32k>(nEndTime - nStartTime));
#endif

			IF (infoBlock.GetJobState(), 1)
			{
				SJobState* pJobState = infoBlock.GetJobState();
				pJobState->SetStopped();
			}
#if defined(JOBMANAGER_SUPPORT_PROFILING)
			pJobProfilingData->nEndTime = gEnv->pTimer->GetAsyncTime();
#endif
		}

	}
	while (m_bStop == false);

}

///////////////////////////////////////////////////////////////////////////////
ILINE void IncrQueuePullPointer(INT_PTR& rCurPullAddr, const INT_PTR cIncr, const INT_PTR cQueueStart, const INT_PTR cQueueEnd)
{
	const INT_PTR cNextPull = rCurPullAddr + cIncr;
	rCurPullAddr = (cNextPull >= cQueueEnd) ? cQueueStart : cNextPull;
}

///////////////////////////////////////////////////////////////////////////////
void JobUpr::ThreadBackEnd::CThreadBackEndWorkerThread::DoWorkProducerConsumerQueue(SInfoBlock& rInfoBlock)
{
	CJobUpr* __restrict pJobUpr = CJobUpr::Instance();

	JobUpr::SProdConsQueueBase* pQueue = (JobUpr::SProdConsQueueBase*)rInfoBlock.GetQueue();

	 INT_PTR* pQueuePull = alias_cast< INT_PTR*>(&pQueue->m_pPull);
	 INT_PTR* pQueuePush = alias_cast< INT_PTR*>(&pQueue->m_pPush);

	u32 queueIncr = pQueue->m_PullIncrement;
	INT_PTR queueStart = pQueue->m_RingBufferStart;
	INT_PTR queueEnd = pQueue->m_RingBufferEnd;
	INT_PTR curPullPtr = *pQueuePull;

	bool bNewJobFound = true;

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

	u32k cParamSize = (rInfoBlock.paramSize << 4);

	Invoker pInvoker = NULL;
	u8 nJobInvokerIdx = (u8) ~0;
	do
	{

		// == process job packet == //
		uk pParamMem = (uk )curPullPtr;
		SAddPacketData* const __restrict pAddPacketData = (SAddPacketData*)((u8*)curPullPtr + cParamSize);

#if defined(JOBMANAGER_SUPPORT_PROFILING)
		SJobProfilingData* pJobProfilingData = gEnv->GetJobUpr()->GetProfilingData(pAddPacketData->profilerIndex);
		pJobProfilingData->nStartTime = gEnv->pTimer->GetAsyncTime();
		pJobProfilingData->nWorkerThread = GetWorkerThreadId();
#endif

		// do we need another job invoker (multi-type job prod/con queue)
		IF (pAddPacketData->nInvokerIndex != nJobInvokerIdx, 0)
		{
			nJobInvokerIdx = pAddPacketData->nInvokerIndex;
			pInvoker = pJobUpr->GetJobInvoker(nJobInvokerIdx);
		}
		if (!pInvoker && pAddPacketData->nInvokerIndex == (u8) ~0)
		{
			pInvoker = rInfoBlock.jobInvoker;
		}

		// make sure we don't try to execute an already stopped job
		assert(!pAddPacketData->pJobState || pAddPacketData->pJobState->IsRunning() == true);

		PREFAST_ASSUME(pInvoker);

		{
			// call delegator function to invoke job entry
#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
			DRX_PROFILE_REGION(PROFILE_SYSTEM, "Job");
			DRXPROFILE_SCOPE_PROFILE_MARKER(pJobUpr->GetJobName(rInfoBlock.jobInvoker));
			DRXPROFILE_SCOPE_PLATFORM_MARKER(pJobUpr->GetJobName(rInfoBlock.jobInvoker));
#endif
			(*pInvoker)(pParamMem);
		}

		// mark job as finished
		IF (pAddPacketData->pJobState, 1)
		{
			pAddPacketData->pJobState->SetStopped();
		}

#if defined(JOBMANAGER_SUPPORT_PROFILING)
		pJobProfilingData->nEndTime = gEnv->pTimer->GetAsyncTime();
#endif

		// == update queue state == //
		IncrQueuePullPointer(curPullPtr, queueIncr, queueStart, queueEnd);

		// load cur push once
		INT_PTR curPushPtr = *pQueuePush;

		// update pull ptr (safe since only change by a single job worker)
		*pQueuePull = curPullPtr;

		// check if we need to wake up the producer from a queue full state
		IF ((curPushPtr & 1) == 1, 0)
		{
			(*pQueuePush) = curPushPtr & ~1;
			pQueue->m_pQueueFullSemaphore->Release();
		}

		// clear queue full marker
		curPushPtr = curPushPtr & ~1;

		// work on next packet if still there
		IF (curPushPtr != curPullPtr, 1)
		{
			continue;
		}

		// temp end of queue, try to update the state while checking the push ptr
		bNewJobFound = false;

		JobUpr::SJobSyncVariable runningSyncVar;
		runningSyncVar.SetRunning();
		JobUpr::SJobSyncVariable stoppedSyncVar;

#if DRX_PLATFORM_64BIT // for 64 bit, we need to atomically swap 128 bit
		bool bStopLoop = false;
		bool bUnlockQueueFullstate = false;
		SJobSyncVariable queueStoppedSemaphore;
		int64 resultValue[2] = { *alias_cast<int64*>(&runningSyncVar), curPushPtr };
		int64 compareValue[2] = { *alias_cast<int64*>(&runningSyncVar), curPushPtr };  // use for result comparsion, since the original compareValue is overwritten
		int64 exchangeValue[2] = { *alias_cast<int64*>(&stoppedSyncVar), curPushPtr };
		do
		{
			resultValue[0] = compareValue[0];
			resultValue[1] = compareValue[1];
			u8 ret = DrxInterlockedCompareExchange128(( int64*)pQueue, exchangeValue[1], exchangeValue[0], resultValue);

			bNewJobFound = ((resultValue[1] & ~1) != curPushPtr);
			bStopLoop = bNewJobFound || (resultValue[0] == compareValue[0] && resultValue[1] == compareValue[1]);

			if (bNewJobFound == false && resultValue[0] > 1)
			{
				// get a copy of the syncvar for unlock (since we will overwrite it)
				queueStoppedSemaphore = *alias_cast<SJobSyncVariable*>(&resultValue[0]);

				// update the exchange value to ensure the CAS succeeds
				compareValue[0] = resultValue[0];
			}

			if (bNewJobFound == false && ((resultValue[1] & 1) == 1))
			{
				// update the exchange value to ensure the CAS succeeds
				compareValue[1] = resultValue[1];
				exchangeValue[1] = (resultValue[1] & ~1);
				bUnlockQueueFullstate = true; // needs to be unset after the loop to ensure a correct state
			}

		}
		while (!bStopLoop);

		if (bUnlockQueueFullstate)
		{
			assert(bNewJobFound == false);
			pQueue->m_pQueueFullSemaphore->Release();
		}
		if (queueStoppedSemaphore.IsRunning())
		{
			assert(bNewJobFound == false);
			queueStoppedSemaphore.SetStopped();
		}
#else
		bool bStopLoop = false;
		bool bUnlockQueueFullstate = false;
		SJobSyncVariable queueStoppedSemaphore;
		T64BitValue resultValue;

		T64BitValue compareValue;
		compareValue.word0 = *alias_cast<u32*>(&runningSyncVar);
		compareValue.word1 = curPushPtr;

		T64BitValue exchangeValue;
		exchangeValue.word0 = *alias_cast<u32*>(&stoppedSyncVar);
		exchangeValue.word1 = curPushPtr;

		do
		{

			resultValue.doubleWord = DrxInterlockedCompareExchange64(( int64*)pQueue, exchangeValue.doubleWord, compareValue.doubleWord);

			bNewJobFound = ((resultValue.word1 & ~1) != curPushPtr);
			bStopLoop = bNewJobFound || resultValue.doubleWord == compareValue.doubleWord;

			if (bNewJobFound == false && resultValue.word0 > 1)
			{
				// get a copy of the syncvar for unlock (since we will overwrite it)
				queueStoppedSemaphore = *alias_cast<SJobSyncVariable*>(&resultValue.word0);

				// update the exchange value to ensure the CAS succeeds
				compareValue.word0 = resultValue.word0;
			}

			if (bNewJobFound == false && ((resultValue.word1 & 1) == 1))
			{
				// update the exchange value to ensure the CAS succeeds
				compareValue.word1 = resultValue.word1;
				exchangeValue.word1 = (resultValue.word1 & ~1);
				bUnlockQueueFullstate = true;
			}

		}
		while (!bStopLoop);

		if (bUnlockQueueFullstate)
		{
			assert(bNewJobFound == false);
			pQueue->m_pQueueFullSemaphore->Release();
		}
		if (queueStoppedSemaphore.IsRunning())
		{
			assert(bNewJobFound == false);
			queueStoppedSemaphore.SetStopped();
		}
#endif

	}
	while (bNewJobFound);

}

///////////////////////////////////////////////////////////////////////////////
JobUpr::ThreadBackEnd::CThreadBackEndWorkerThread::CThreadBackEndWorkerThread(CThreadBackEnd* pThreadBackend, detail::CWaitForJobObject& rSemaphore, JobUpr::SJobQueue_ThreadBackEnd& rJobQueue, u32 nId) :
	m_rSemaphore(rSemaphore),
	m_rJobQueue(rJobQueue),
	m_bStop(false),
	m_nId(nId),
	m_pThreadBackend(pThreadBackend)
{
}

///////////////////////////////////////////////////////////////////////////////
JobUpr::ThreadBackEnd::CThreadBackEndWorkerThread::~CThreadBackEndWorkerThread()
{

}
