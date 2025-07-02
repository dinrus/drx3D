// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FallbackBackend.h
//  Version:     v1.00
//  Created:     07/05/2011 by Christopher Bolte
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/FallBackBackend.h>
#include <drx3D/Sys/JobUpr.h>

JobUpr::FallBackBackEnd::CFallBackBackEnd::CFallBackBackEnd()
{

}

JobUpr::FallBackBackEnd::CFallBackBackEnd::~CFallBackBackEnd()
{

}

void JobUpr::FallBackBackEnd::CFallBackBackEnd::AddJob(JobUpr::CJobDelegator& crJob, const JobUpr::TJobHandle cJobHandle, JobUpr::SInfoBlock& rInfoBlock)
{
	CJobUpr* __restrict pJobUpr = CJobUpr::Instance();

	// just execute the job in the calling context
	IF (rInfoBlock.HasQueue(), 0)
	{
		JobUpr::SProdConsQueueBase* pQueue = (JobUpr::SProdConsQueueBase*)rInfoBlock.GetQueue();
		JobUpr::SJobSyncVariable* pQueueState = &pQueue->m_QueueRunningState;

		 INT_PTR* pQueuePull = alias_cast< INT_PTR*>(&pQueue->m_pPull);
		 INT_PTR* pQueuePush = alias_cast< INT_PTR*>(&pQueue->m_pPush);

		u32 queueIncr = pQueue->m_PullIncrement;
		INT_PTR queueStart = pQueue->m_RingBufferStart;
		INT_PTR queueEnd = pQueue->m_RingBufferEnd;
		INT_PTR curPullPtr = *pQueuePull;

		// == process job packet == //
		uk pParamMem = (uk )curPullPtr;
		u32k cParamSize = (rInfoBlock.paramSize << 4);
		SAddPacketData* const __restrict pAddPacketData = (SAddPacketData*)((u8*)curPullPtr + cParamSize);

		Invoker pInvoker = pJobUpr->GetJobInvoker(pAddPacketData->nInvokerIndex);

		// call delegator function to invoke job entry
		(*pInvoker)(pParamMem);

		// mark job as finished
		IF (pAddPacketData->pJobState, 1)
		{
			pAddPacketData->pJobState->SetStopped();
		}

		// == update queue state == //
		const INT_PTR cNextPull = curPullPtr + queueIncr;
		curPullPtr = (cNextPull >= queueEnd) ? queueStart : cNextPull;

		// update pull ptr (safe since only change by a single job worker)
		*pQueuePull = curPullPtr;

		// mark queue as finished(safe since the fallback is not threaded)
		pQueueState->SetStopped();
	}
	else
	{
		Invoker delegator = crJob.GetGenericDelegator();
		ukk pParamMem = crJob.GetJobParamData();

		// execute job function
		if (crJob.GetLambda())
		{
			crJob.GetLambda()();
		}
		else
		{
			(*delegator)((uk )pParamMem);
		}

		IF (rInfoBlock.GetJobState(), 1)
		{
			SJobState* pJobState = rInfoBlock.GetJobState();
			pJobState->SetStopped();
		}
	}
}
