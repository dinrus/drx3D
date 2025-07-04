// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   objmancullqueue.cpp
//  Version:     v1.00
//  Created:     2/12/2009 by Michael Glueck
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Реализация асинхронных очередей obj-culling.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/PolygonClipContext.h>
#include <drx3D/Eng3D/ObjManCullQueue.h>
#include <drx3D/Eng3D/CCullThread.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

#if defined(USE_CULL_QUEUE)
	#define CULL_QUEUE_USE_JOB 1
#else
	#define CULL_QUEUE_USE_JOB 0
#endif

#if CULL_QUEUE_USE_JOB
DECLARE_JOB("IsBoxOccluded", TBoxOccludedJob, NCullQueue::SCullQueue::ProcessInternal);
JobUpr::SJobState g_OcclJobState;
#endif

NCullQueue::SCullQueue::SCullQueue()
	: cullBufWriteHeadIndex(0)
	, cullBufReadHeadIndex(0)
	, m_TestItemQueueReady(false)
	, m_BufferAndCameraReady(false)
	, m_MainFrameID(0)
	, m_pCam(NULL)
{
	memset(cullItemBuf, 0, sizeof(cullItemBuf));
}

void NCullQueue::SCullQueue::FinishedFillingTestItemQueue()
{
#ifdef USE_CULL_QUEUE
	assert(gEnv->IsEditor() || m_TestItemQueueReady == false);
	m_TestItemQueueReady = true;
	if (m_BufferAndCameraReady == true)
	{
		//If both sets of data are ready, begin the test.
		Process();
	}
#endif
}

void NCullQueue::SCullQueue::SetTestParams(u32 mainFrameID, const CCamera* pCam)
{
	m_MainFrameID = mainFrameID;
	m_pCam = pCam;
	assert(gEnv->IsEditor() || m_BufferAndCameraReady == false);
	m_BufferAndCameraReady = true;
	if (m_TestItemQueueReady == true)
	{
		//If both sets of data are ready, begin the test.
		Process();
	}
}

void NCullQueue::SCullQueue::Process()
{
#ifdef USE_CULL_QUEUE
	FUNCTION_PROFILER_3DENGINE;

	TBoxOccludedJob job(m_MainFrameID, (CZBufferCuller*)m_pCullBuffer, m_pCam);
	job.SetClassInstance(this);
	job.RegisterJobState(&g_OcclJobState);
	job.Run();
#endif //USE_CULL_QUEUE
}

NCullQueue::SCullQueue::~SCullQueue()
{
}

void NCullQueue::SCullQueue::Wait()
{
	FUNCTION_PROFILER_3DENGINE;

#ifdef USE_CULL_QUEUE
	gEnv->GetJobUpr()->WaitForJob(g_OcclJobState, 300);

	cullBufWriteHeadIndex = 0;
	cullBufReadHeadIndex = 0;
#endif
}

void NCullQueue::SCullQueue::ProcessInternal(u32 mainFrameID, CZBufferCuller* const pCullBuffer, const CCamera* const pCam)
{
#ifdef USE_CULL_QUEUE
	CZBufferCuller& cullBuffer = *(CZBufferCuller*)pCullBuffer;
	#define localQueue (*this)
	#define localCam   (*pCam)

	cullBuffer.BeginFrame(localCam);

	//FeedbackLoop:

	{
		const SCullItem* const cEnd = &localQueue.cullItemBuf[localQueue.cullBufWriteHeadIndex];
		for (u16 a = 1, b = 0; b < 4; a <<= 1, b++)//traverse through all 4 occlusion buffers
		{
			cullBuffer.ReloadBuffer(b);
			for (SCullItem* it = &localQueue.cullItemBuf[localQueue.cullBufReadHeadIndex]; it != cEnd; ++it)
			{
				SCullItem& rItem = *it;
				IF (!(rItem.BufferID & a), 1)
					continue;

				IF ((rItem.BufferID & 1), 0)  //zbuffer
				{
					if (!cullBuffer.IsObjectVisible(rItem.objBox, eoot_OBJECT, 0.f, &rItem.pOcclTestVars->nLastOccludedMainFrameID))
						rItem.pOcclTestVars->nLastOccludedMainFrameID = mainFrameID;
					else
						rItem.pOcclTestVars->nLastVisibleMainFrameID = mainFrameID;
				}
				else  //shadow buffer
				if (rItem.pOcclTestVars->nLastNoShadowCastMainFrameID != mainFrameID)
				{
					if (cullBuffer.IsObjectVisible(rItem.objBox, eoot_OBJECT, 0.f, &rItem.pOcclTestVars->nLastOccludedMainFrameID))
						rItem.pOcclTestVars->nLastShadowCastMainFrameID = mainFrameID;
				}
			}
		}

		localQueue.cullBufReadHeadIndex = (cEnd - localQueue.cullItemBuf);

		//if not visible, set occluded
		for (SCullItem* it = localQueue.cullItemBuf; it != cEnd; ++it)
		{
			SCullItem& rItem = *it;
			IF ((rItem.BufferID & 6) & (rItem.pOcclTestVars->nLastNoShadowCastMainFrameID != mainFrameID), 1)
				rItem.pOcclTestVars->nLastShadowCastMainFrameID = mainFrameID;
		}

	}

#endif
#undef localQueue
#undef localCam
}
