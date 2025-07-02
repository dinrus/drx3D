// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Statoscope.cpp
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/Statoscope.h>
#include <drx3D/Sys/FrameProfileSystem.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Sys/SimpleStringPool.h>
#include <drx3D/Sys/System.h>
#include <drx3D/Sys/ThreadProfiler.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>
#include <drx3D/Sys/IScaleformHelper.h>
#include <drx3D/CoreX/ParticleSys/IParticlesPfx2.h>

#include <drx3D/Sys/StatoscopeStreamingIntervalGroup.h>
#include <drx3D/Sys/StatoscopeTextureStreamingIntervalGroup.h>

#if ENABLE_STATOSCOPE

namespace
{
class CCompareFrameProfilersSelfTime
{
public:
	bool operator()(const std::pair<CFrameProfiler*, int64>& p1, const std::pair<CFrameProfiler*, int64>& p2)
	{
		return p1.second > p2.second;
	}
};
}

static string GetFrameProfilerPath(CFrameProfiler* pProfiler)
{
	if (pProfiler)
	{
		tukk sThreadName = gEnv->pThreadUpr->GetThreadName(pProfiler->m_threadId);
		char sThreadNameBuf[11]; // 0x 12345678 \0 => 2+8+1=11
		if (!sThreadName || !sThreadName[0])
		{
			drx_sprintf(sThreadNameBuf, "%" PRI_THREADID, (pProfiler->m_threadId));
		}
		if (strstr(sThreadName, "JobSystem_Worker_") == sThreadName)
		{
			sThreadName = "JobSystem_Merged";
		}

		char sNameBuffer[256];
		drx_strcpy(sNameBuffer, pProfiler->m_name);

	#if DRX_FUNC_HAS_SIGNATURE // __FUNCTION__ only contains classname on MSVC, for other function we have __PRETTY_FUNCTION__, so we need to strip return / argument types
		{
			tuk pEnd = (tuk)strchr(sNameBuffer, '(');
			if (pEnd)
			{
				*pEnd = 0;
				while (*(pEnd) != ' ' && *(pEnd) != '*' && pEnd != (sNameBuffer - 1))
				{
					--pEnd;
				}
				memmove(sNameBuffer, pEnd + 1, &sNameBuffer[sizeof(sNameBuffer)] - (pEnd + 1));
			}
		}
	#endif

		string path = sThreadName ? sThreadName : sThreadNameBuf;
		path += "/";
		path += gEnv->pFrameProfileSystem->GetModuleName(pProfiler);
		path += "/";
		path += sNameBuffer;
		path += "/";

		return path;
	}
	else
	{
		return string("SmallFunctions/SmallFunction/");
	}
}

CStatoscopeDataClass::CStatoscopeDataClass(tukk format)
	: m_format(format)
	, m_numDataElements(0)
{
	u32 numOpening = 0;
	u32 numClosing = 0;
	u32 numDollars = 0;
	tukk pNameStart = NULL;

	string formatString(format);
	i32 pathStart = formatString.find_first_of('\'');
	i32 pathEnd = formatString.find_first_of('\'', pathStart + 1);
	m_path = formatString.substr(pathStart + 1, pathEnd - 2);

	for (tukk c = format; c && *c != '\0'; ++c)
	{
		if (*c == '(')
		{
			++numOpening;
			pNameStart = c + 1;
		}
		else if (*c == ')')
		{
			++numClosing;

			if (pNameStart)
			{
				ProcessNewBinDataElem(pNameStart, c);
				pNameStart = NULL;
			}
		}
		else if (*c == '$')
		{
			++numDollars;
		}
	}
	if (numClosing == numOpening)
	{
		m_numDataElements = numOpening + numDollars;
	}
	else
	{
		m_numDataElements = 0;
		DrxFatalError("Mismatched opening/closing braces in Statoscope format description.");
	}
}

void CStatoscopeDataClass::ProcessNewBinDataElem(tukk pStart, tukk pEnd)
{
	if (pStart)
	{
		BinDataElement newElem;

		//determine data type
		string s(pStart);
		i32 spaceIndex = s.find_first_of(' ');

		if (spaceIndex != -1)
		{
			string typeString = s.substr(0, spaceIndex);
			bool bBroken = false;

			if (typeString.compareNoCase("float") == 0)
			{
				newElem.type = StatoscopeDataWriter::Float;
			}
			else if (typeString.compareNoCase("i32") == 0)
			{
				newElem.type = StatoscopeDataWriter::Int;
			}
			else if (typeString.compareNoCase("int64") == 0)
			{
				newElem.type = StatoscopeDataWriter::Int64;
			}
			else if (typeString.compareNoCase("string") == 0)
			{
				newElem.type = StatoscopeDataWriter::String;
			}
			else
			{
				bBroken = true;
				DrxLogAlways("Broken!");
			}

			if (!bBroken)
			{
				i32 bracketIndex = s.find_first_of(')');
				newElem.name = s.substr(spaceIndex + 1, bracketIndex - spaceIndex - 1);

				m_binElements.push_back(newElem);
			}
		}
	}
}

void CStatoscopeDataGroup::WriteHeader(CDataWriter* pDataWriter)
{
	pDataWriter->WriteDataStr(m_dataClass.GetPath());
	i32 nDataElems = (i32)m_dataClass.GetNumBinElements();

	pDataWriter->WriteData(nDataElems);

	for (i32 i = 0; i < nDataElems; i++)
	{
		const CStatoscopeDataClass::BinDataElement& elem = m_dataClass.GetBinElement(i);
		pDataWriter->WriteData(elem.type);
		pDataWriter->WriteDataStr(elem.name.c_str());
	}
}

CStatoscopeIntervalGroup::CStatoscopeIntervalGroup(u32 id, tukk name, tukk format)
	: m_id(id)
	, m_name(name)
	, m_dataClass(format)
	, m_instLength(0)
	, m_pWriter(NULL)
{
}

void CStatoscopeIntervalGroup::Enable(CStatoscopeEventWriter* pWriter)
{
	m_pWriter = pWriter;
	Enable_Impl();
}

void CStatoscopeIntervalGroup::Disable()
{
	Disable_Impl();
	m_pWriter = NULL;
}

size_t CStatoscopeIntervalGroup::GetDescEventLength() const
{
	size_t numElements = m_dataClass.GetNumElements();
	size_t length = numElements * sizeof(u8);

	for (size_t i = 0; i < m_dataClass.GetNumBinElements(); ++i)
	{
		const CStatoscopeDataClass::BinDataElement& elem = m_dataClass.GetBinElement(i);
		length += elem.name.length() + 1;
	}

	return length;
}

void CStatoscopeIntervalGroup::WriteDescEvent(uk p) const
{
	size_t numElements = m_dataClass.GetNumElements();

	tuk pc = (tuk)p;
	for (size_t i = 0; i < m_dataClass.GetNumBinElements(); ++i)
	{
		const CStatoscopeDataClass::BinDataElement& elem = m_dataClass.GetBinElement(i);
		*pc++ = (char)elem.type;
		strcpy(pc, elem.name.c_str());
		pc += elem.name.length() + 1;
	}
}

void CStatoscopeFrameRecordWriter::AddValue(float f)
{
	m_pDataWriter->WriteData(f);
	++m_nWrittenElements;
}

void CStatoscopeFrameRecordWriter::AddValue(tukk s)
{
	m_pDataWriter->WriteDataStr(s);
	++m_nWrittenElements;
}

void CStatoscopeFrameRecordWriter::AddValue(i32 i)
{
	m_pDataWriter->WriteData(i);
	++m_nWrittenElements;
}

struct SFrameLengthDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('f', "frame lengths", "['/' (float frameLengthInMS) (float lostProfilerTimeInMS) ]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		IFrameProfileSystem* pFrameProfileSystem = gEnv->pSystem->GetIProfileSystem();

		fr.AddValue(gEnv->pTimer->GetRealFrameTime() * 1000.0f);
		fr.AddValue(pFrameProfileSystem ? pFrameProfileSystem->GetLostFrameTimeMS() : -1.f);
	}
};

struct SMemoryDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('m', "memory", "['/Memory/' (float mainMemUsageInMB) (i32 vidMemUsageInMB)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		IMemoryUpr::SProcessMemInfo processMemInfo;
		GetISystem()->GetIMemoryUpr()->GetProcessMemInfo(processMemInfo);
		fr.AddValue(processMemInfo.PagefileUsage / (1024.f * 1024.f));

		size_t vidMem, lastVidMem;
		gEnv->pRenderer->GetVideoMemoryUsageStats(vidMem, lastVidMem);
		fr.AddValue((i32)vidMem);
	}
};

struct SStreamingDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('s', "streaming", "['/Streaming/' (float cgfStreamingMemUsedInMB) (float cgfStreamingMemRequiredInMB) (i32 cgfStreamingPoolSize) (float tempMemInKB)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		I3DEngine::SObjectsStreamingStatus objectsStreamingStatus;
		gEnv->p3DEngine->GetObjectsStreamingStatus(objectsStreamingStatus);
		fr.AddValue(objectsStreamingStatus.nAllocatedBytes / (1024.f * 1024.f));
		fr.AddValue(objectsStreamingStatus.nMemRequired / (1024.f * 1024.f));
		fr.AddValue(objectsStreamingStatus.nMeshPoolSize);
		fr.AddValue(gEnv->pSystem->GetStreamEngine()->GetStreamingStatistics().nTempMemory / 1024.f);
	}
};

struct SStreamingAudioDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('a', "streaming audio", "['/StreamingAudio/' (float bandwidthActualKBsecond) (float bandwidthRequestedKBsecond)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		I3DEngine::SStremaingBandwidthData subsystemStreamingData;
		memset(&subsystemStreamingData, 0, sizeof(I3DEngine::SStremaingBandwidthData));
		gEnv->p3DEngine->GetStreamingSubsystemData(eStreamTaskTypeSound, subsystemStreamingData);
		fr.AddValue(subsystemStreamingData.fBandwidthActual);
		fr.AddValue(subsystemStreamingData.fBandwidthRequested);
	}
};

struct SStreamingObjectsDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('o', "streaming objects", "['/StreamingObjects/' (float bandwidthActualKBsecond) (float bandwidthRequestedKBsecond)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		I3DEngine::SStremaingBandwidthData subsystemStreamingData;
		memset(&subsystemStreamingData, 0, sizeof(I3DEngine::SStremaingBandwidthData));
		gEnv->p3DEngine->GetStreamingSubsystemData(eStreamTaskTypeGeometry, subsystemStreamingData);
		fr.AddValue(subsystemStreamingData.fBandwidthActual);
		fr.AddValue(subsystemStreamingData.fBandwidthRequested);
	}
};

struct SThreadsDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('t', "threading", "['/Threading/' (float MTLoadInMS) (float MTWaitingForRTInMS) "
		                                      "(float RTLoadInMS) (float RTWaitingForMTInMS) (float RTWaitingForGPUInMS) "
		                                      "(float RTFrameLengthInMS) (float RTSceneDrawningLengthInMS) (float NetThreadTimeInMS)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		IRenderer::SRenderTimes renderTimes;
		gEnv->pRenderer->GetRenderTimes(renderTimes);

		SNetworkPerformance netPerformance;
		if (gEnv->pNetwork)
			gEnv->pNetwork->GetPerformanceStatistics(&netPerformance);
		else
			netPerformance.m_threadTime = 0.0f;

		float RTWaitingForMTInMS = renderTimes.fWaitForMain * 1000.f;
		float MTWaitingForRTInMS = renderTimes.fWaitForRender * 1000.f;
		float RTWaitingForGPUInMS = renderTimes.fWaitForGPU * 1000.f;
		float RTLoadInMS = renderTimes.fTimeProcessedRT * 1000.f;

		float MTLoadInMS = (gEnv->pTimer->GetRealFrameTime() * 1000.0f) - MTWaitingForRTInMS;

		//Load represents pure RT work, so compensate for GPU sync
		RTLoadInMS = RTLoadInMS - RTWaitingForGPUInMS;

		fr.AddValue(MTLoadInMS);
		fr.AddValue(MTWaitingForRTInMS);
		fr.AddValue(RTLoadInMS);
		fr.AddValue(RTWaitingForMTInMS);
		fr.AddValue(RTWaitingForGPUInMS);
		fr.AddValue(renderTimes.fTimeProcessedRT * 1000);
		fr.AddValue(renderTimes.fTimeProcessedRTScene * 1000);
		fr.AddValue(netPerformance.m_threadTime * 1000.f);
	}
};

struct SSysThreadsDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('T', "system threading", "['/SystemThreading/' "
		                                             "(float MTLoadInMS) (float RTLoadInMS) (float otherLoadInMS) "
		                                             "(float sysIdle0InMS) (float sysIdle1InMS) (float sysIdleTotalInMS) "
		                                             "(float totalLoadInMS) (float timeFrameInMS)]");
	}

	virtual void Enable()
	{
		IStatoscopeDataGroup::Enable();

		SSysThreadsDG::StartThreadProf();
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();

		SSysThreadsDG::StopThreadProf();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
	#if defined(THREAD_SAMPLER)
		CSystem* pSystem = static_cast<CSystem*>(gEnv->pSystem);
		CThreadProfiler* pThreadProf = pSystem->GetThreadProfiler();
		IThreadSampler* pThreadSampler = pThreadProf ? pThreadProf->GetThreadSampler() : NULL;

		if (pThreadSampler)
		{
			pThreadSampler->Tick();

			fr.AddValue(pThreadSampler->GetExecutionTime(TT_MAIN));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_RENDER));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_OTHER));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_SYSTEM_IDLE_0));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_SYSTEM_IDLE_1));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_SYSTEM_IDLE_0) + pThreadSampler->GetExecutionTime(TT_SYSTEM_IDLE_1));
			fr.AddValue(pThreadSampler->GetExecutionTime(TT_TOTAL) / pThreadSampler->GetNumHWThreads());
			fr.AddValue(pThreadSampler->GetExecutionTimeFrame());
		}
		else
	#endif // defined(THREAD_SAMPLER)
		{
			for (u32 i = 0; i < 8; i++)
			{
				fr.AddValue(0.0f);
			}
		}
	}

	static void StartThreadProf()
	{
		CSystem* pSystem = static_cast<CSystem*>(gEnv->pSystem);
		CThreadProfiler* pThreadProf = pSystem->GetThreadProfiler();
		pThreadProf->Start();
	}

	static void StopThreadProf()
	{
		CSystem* pSystem = static_cast<CSystem*>(gEnv->pSystem);
		CThreadProfiler* pThreadProf = pSystem->GetThreadProfiler();
		pThreadProf->Stop();
	}
};

	#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)

struct SWorkerInfoSummarizedDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('X', "Worker Information Summarized", "['/WorkerInformation/Summary/$/' "
		                                                          "(float SamplePeriodInMS) (i32 ActiveWorkers) (float AvgUtilPerc) (float TotalExecutionPeriodeInMS) (i32 TotalNumberOfJobsExecuted)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		// Helpers
		struct SBackendPair
		{
			const JobUpr::IBackend* pBackend;
			tukk                 pPrefix;
		};

		const SBackendPair pBackends[] =
		{
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread),   "NoneBlocking" },
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking), "Blocking"     },
		};

		// Write worker summary for each backend
		for (i32 i = 0; i < DRX_ARRAY_COUNT(pBackends); i++)
		{
			const SBackendPair& rBackendPair = pBackends[i];
			bool addedValuesToStream = false;

			if (rBackendPair.pBackend)
			{
				JobUpr::SWorkerFrameStatsSummary frameStatsSummary;
				JobUpr::IWorkerBackEndProfiler* const __restrict pWorkerProfiler = rBackendPair.pBackend->GetBackEndWorkerProfiler();

				if (pWorkerProfiler)
				{
					pWorkerProfiler->GetFrameStatsSummary(frameStatsSummary);

					fr.AddValue(rBackendPair.pPrefix);                                    // Prefix
					fr.AddValue(frameStatsSummary.nSamplePeriod * 0.001f);                // SamplePeriode
					fr.AddValue((i32)frameStatsSummary.nNumActiveWorkers);                // ActiveWorkers
					fr.AddValue(frameStatsSummary.nAvgUtilPerc);                          // AvgUtilPerc
					fr.AddValue((float)frameStatsSummary.nTotalExecutionPeriod * 0.001f); // TotalExecutionPeriode
					fr.AddValue((i32)frameStatsSummary.nTotalNumJobsExecuted);            // TotalNumberOfJobsExecuted
					addedValuesToStream = true;
				}
			}

			if (!addedValuesToStream)
			{
				fr.AddValue(rBackendPair.pPrefix);
				fr.AddValue(0.f);
				fr.AddValue(0);
				fr.AddValue(0.f);
				fr.AddValue(0.f);
				fr.AddValue(0);
			}
		}
	}

	virtual u32 PrepareToWrite()
	{
		return 2; // None-Blocking & Blocking Worker Summary Data Set
	}

};

struct SJobsInfoSummarizedDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('Z', "Job Information Summarized", "['/JobInformation/Summary/$/' "
		                                                       "(float TotalExecutionTimeInMS) (i32 TotalNumberOfJobsExecuted) (i32 TotalNumberOfIndividualJobsExecuted)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		// Helpers
		struct SBackendPair
		{
			const JobUpr::IBackend* pBackend;
			tukk                 pPrefix;
		};

		const SBackendPair pBackendPairs[] =
		{
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread),   "NoneBlocking" },
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking), "Blocking"     },
		};

		// Write jobs summary for each backend
		for (i32 i = 0; i < DRX_ARRAY_COUNT(pBackendPairs); i++)
		{
			const SBackendPair& rBackendPair = pBackendPairs[i];
			bool addedValuesToStream = false;

			if (rBackendPair.pBackend)
			{
				JobUpr::SJobFrameStatsSummary jobFrameStatsSummary;
				JobUpr::IWorkerBackEndProfiler* const __restrict pWorkerProfiler = rBackendPair.pBackend->GetBackEndWorkerProfiler();

				if (pWorkerProfiler)
				{
					pWorkerProfiler->GetFrameStatsSummary(jobFrameStatsSummary);

					fr.AddValue(rBackendPair.pPrefix);                                      // Prefix
					fr.AddValue((float)jobFrameStatsSummary.nTotalExecutionTime * 0.001f);  // TotalExecutionTime
					fr.AddValue((i32)jobFrameStatsSummary.nNumJobsExecuted);                // TotalNumberOfJobsExecuted
					fr.AddValue((i32)jobFrameStatsSummary.nNumIndividualJobsExecuted);      // TotalNumberOfIndividualJobsExecuted
					addedValuesToStream = true;
				}
			}

			if (!addedValuesToStream)
			{
				fr.AddValue(rBackendPair.pPrefix);
				fr.AddValue(0.f);
				fr.AddValue(0);
				fr.AddValue(0);
			}
		}
	}

	virtual u32 PrepareToWrite()
	{
		return 2; // None-Blocking & Blocking Job Summary Data Set
	}
};

class CWorkerInfoIndividualDG : public IStatoscopeDataGroup
{

public:
	CWorkerInfoIndividualDG()
	{
		m_pThreadFrameStats = new JobUpr::CWorkerFrameStats(0);
		m_pBlockingFrameStats = new JobUpr::CWorkerFrameStats(0);
	}

	~CWorkerInfoIndividualDG()
	{
		SAFE_DELETE(m_pBlockingFrameStats);
		SAFE_DELETE(m_pThreadFrameStats);
	}

	virtual SDescription GetDescription() const
	{
		return SDescription('W', "Worker Information Individual", "['/WorkerInformation/Individual/$/$/' "
		                                                          "(float SamplePeriodInMS) (float ExecutionTimeInMS) (float IdleTimeInMS) (float UtilPerc) (i32 NumJobs)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		// Helpers
		struct SFrameStatsPair
		{
			const JobUpr::CWorkerFrameStats* pFrameStats;
			tukk                          pPrefix;
		};

		const SFrameStatsPair pFrameStatsPairs[] =
		{
			{ m_pThreadFrameStats,   "NoneBlocking" },
			{ m_pBlockingFrameStats, "Blocking"     },
		};

		// Write individual worker information for each backend
		for (i32 i = 0; i < DRX_ARRAY_COUNT(pFrameStatsPairs); i++)
		{
			const SFrameStatsPair& rFrameStatsPair = pFrameStatsPairs[i];
			const float cSamplePeriod = (float)rFrameStatsPair.pFrameStats->nSamplePeriod;

			for (i32 j = 0; j < rFrameStatsPair.pFrameStats->numWorkers; ++j)
			{
				const JobUpr::CWorkerFrameStats::SWorkerStats& pWorkerStats = rFrameStatsPair.pFrameStats->workerStats[j];
				const float nExecutionPeriod = (float)pWorkerStats.nExecutionPeriod * 0.001f;
				const float nIdleTime = (rFrameStatsPair.pFrameStats->nSamplePeriod > pWorkerStats.nExecutionPeriod) ? (rFrameStatsPair.pFrameStats->nSamplePeriod - pWorkerStats.nExecutionPeriod) * 0.001f : 0.f;

				char cWorkerName[16];
				drx_sprintf(cWorkerName, "Worker%i", j);
				fr.AddValue(rFrameStatsPair.pPrefix);             // Category
				fr.AddValue(cWorkerName);                         // Worker Name
				fr.AddValue(cSamplePeriod * 0.001f);              // SamplePeriodInMS
				fr.AddValue(nExecutionPeriod);                    // ExecutionTimeInMS
				fr.AddValue(nIdleTime);                           // IdleTimeInMS
				fr.AddValue(pWorkerStats.nUtilPerc);              // UtilPerc [0.f ... 100.f]
				fr.AddValue((i32)pWorkerStats.nNumJobsExecuted);  // NumJobs
			}
		}
	}

	virtual u32 PrepareToWrite()
	{
		// Helpers
		struct SBackendSTatePair
		{
			const JobUpr::IBackend*     pBackend;
			JobUpr::CWorkerFrameStats** pFrameStats;
		};

		SBackendSTatePair rBackendStatePairs[] =
		{
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread),   &m_pThreadFrameStats   },
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking), &m_pBlockingFrameStats },
		};

		// Get individual worker information for each backend
		u32 cNumTotalWorkers = 0;
		for (i32 i = 0; i < DRX_ARRAY_COUNT(rBackendStatePairs); i++)
		{
			SBackendSTatePair& rBackendStatePair = rBackendStatePairs[i];
			bool gotData = false;

			// Get job profile stats for backend
			if (rBackendStatePair.pBackend)
			{
				u32k cNumWorkers = rBackendStatePair.pBackend->GetBackEndWorkerProfiler()->GetNumWorkers();
				JobUpr::IWorkerBackEndProfiler* const __restrict pWorkerProfiler = rBackendStatePair.pBackend->GetBackEndWorkerProfiler();

				// Get stats
				if (pWorkerProfiler)
				{
					// Validate size
					if ((*rBackendStatePair.pFrameStats)->numWorkers != cNumWorkers)
					{
						SAFE_DELETE(*rBackendStatePair.pFrameStats);
						*rBackendStatePair.pFrameStats = new JobUpr::CWorkerFrameStats(cNumWorkers);
					}

					// Get frame stats
					pWorkerProfiler->GetFrameStats(**rBackendStatePair.pFrameStats);
					cNumTotalWorkers += cNumWorkers;
					gotData = true;
				}
			}

			// Set frame stats to 0 workers
			if (!gotData)
			{
				if ((*rBackendStatePair.pFrameStats)->numWorkers != 0)
				{
					SAFE_DELETE(*rBackendStatePair.pFrameStats);
					*rBackendStatePair.pFrameStats = new JobUpr::CWorkerFrameStats(0);
				}
			}
		}

		// Return number of data sets to be written
		return cNumTotalWorkers;
	}

private:
	JobUpr::CWorkerFrameStats* m_pThreadFrameStats;
	JobUpr::CWorkerFrameStats* m_pBlockingFrameStats;
};

class CJobsInfoIndividualDG : public IStatoscopeDataGroup
{
public:
	virtual SDescription GetDescription() const
	{
		return SDescription('Y', "Job Information Individual", "['/JobInformation/Individual/$/$/' "
		                                                       "(float ExecutionTimeInMS) (i32 NumberOfExecutions)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		// Helpers
		struct SFrameStatsPair
		{
			const JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer* pFrameStats;
			tukk pPrefix;
		};

		const SFrameStatsPair pFrameStatsPairs[] =
		{
			{ &m_ThreadJobFrameStats,   "NoneBlocking" },
			{ &m_BlockingJobFrameStats, "Blocking"     },
		};

		// Write individual job information for each backend
		for (i32 i = 0; i < DRX_ARRAY_COUNT(pFrameStatsPairs); i++)
		{
			const SFrameStatsPair& pFrameStatsPair = pFrameStatsPairs[i];

			// Write job information
			for (i32 j = 0, nSize = pFrameStatsPair.pFrameStats->size(); j < nSize; ++j)
			{
				char cJobName[128];
				const JobUpr::SJobFrameStats& rJobFrameStats = (*pFrameStatsPair.pFrameStats)[j];

				drx_sprintf(cJobName, "Job_%s", rJobFrameStats.cpName);
				fr.AddValue(pFrameStatsPair.pPrefix);             // Category
				fr.AddValue(cJobName);                            // Job Name
				fr.AddValue((float)rJobFrameStats.usec * 0.001f); // ExecutionTimeInMS
				fr.AddValue((i32)rJobFrameStats.count);           // NumberOfExecutions
			}
		}
	}

	virtual u32 PrepareToWrite()
	{
		// Helpers
		struct SBackendStatePair
		{
			const JobUpr::IBackend* pBackend;
			JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer* pFrameStats;
		};

		SBackendStatePair pBackendStatePair[] =
		{
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Thread),   &m_ThreadJobFrameStats   },
			{ gEnv->GetJobUpr()->GetBackEnd(JobUpr::eBET_Blocking), &m_BlockingJobFrameStats },
		};

		// Get individual job information for each backend
		u32 cNumTotalJobs = 0;
		for (i32 i = 0; i < DRX_ARRAY_COUNT(pBackendStatePair); i++)
		{
			SBackendStatePair& rBackendStatePair = pBackendStatePair[i];
			bool gotData = false;

			// Get job profile stats
			if (rBackendStatePair.pBackend)
			{
				JobUpr::IWorkerBackEndProfiler* const __restrict pWorkerProfiler = rBackendStatePair.pBackend->GetBackEndWorkerProfiler();

				if (pWorkerProfiler)
				{
					pWorkerProfiler->GetFrameStats(*rBackendStatePair.pFrameStats, JobUpr::IWorkerBackEndProfiler::eJobSortOrder_NoSort);
					cNumTotalJobs += rBackendStatePair.pFrameStats->size();
					gotData = true;
				}

				// Clear on no data
				if (!gotData)
					rBackendStatePair.pFrameStats->clear();
			}
		}

		// Return number of data sets to be written
		return cNumTotalJobs;
	}

private:
	JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer m_ThreadJobFrameStats;
	JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer m_BlockingJobFrameStats;
};
	#endif

struct SCPUTimesDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('j', "CPU Times", "['/CPUTimes/' (float physTime) "
		                                      "(float particleTime) (float particleSyncTime)"
		                                      "(float animTime) (i32 animNumCharacters) "
		                                      "(float aiTime) "
		                                      "(float flashTime)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		double frequency = 1.0 / static_cast<double>(freq.QuadPart);
	#define TICKS_TO_MS(t) ((float)((t) * 1000.0 * frequency))

		u32 gUpdateTimeIdx = 0, gUpdateTimesNum = 0;
		const sUpdateTimes* gUpdateTimes = gEnv->pSystem->GetUpdateTimeStats(gUpdateTimeIdx, gUpdateTimesNum);
		float curPhysTime = TICKS_TO_MS(gUpdateTimes[gUpdateTimeIdx].PhysStepTime);
		fr.AddValue(curPhysTime);

		IParticleUpr* pPartMan = gEnv->p3DEngine->GetParticleUpr();
		if (pPartMan != NULL)
		{
			float fTimeMS = TICKS_TO_MS(pPartMan->GetLightProfileCounts().NumFrameTicks());
			float fTimeSyncMS = TICKS_TO_MS(pPartMan->GetLightProfileCounts().NumFrameSyncTicks());

			fr.AddValue(fTimeMS);
			fr.AddValue(fTimeSyncMS);
		}
		else
		{
			fr.AddValue(0.0f);
			fr.AddValue(0.0f);
			fr.AddValue(0.0f);
		}

		ICharacterUpr* pCharUpr = gEnv->pCharacterUpr;
		if (pCharUpr != NULL)
		{
			i32 nNumCharacters = (i32)pCharUpr->NumCharacters();
			float fTimeMS = TICKS_TO_MS(pCharUpr->NumFrameTicks());

			fr.AddValue(fTimeMS);
			fr.AddValue(nNumCharacters);
		}
		else
		{
			fr.AddValue(0.0f);
			fr.AddValue(0);
		}

		IAISystem* pAISystem = gEnv->pAISystem;
		if (pAISystem != NULL)
		{
			float fTimeMS = TICKS_TO_MS(pAISystem->NumFrameTicks());
			fr.AddValue(fTimeMS);
		}
		else
		{
			fr.AddValue(0.0f);
		}

		const float flashCost = gEnv->pScaleformHelper ? gEnv->pScaleformHelper->GetFlashProfileResults() : -1.0f;
		if (flashCost >= 0.0f)
		{
			float flashCostInMS = flashCost * 1000.0f;
			fr.AddValue(flashCostInMS);
		}
		else
		{
			fr.AddValue(0.0f);
		}
	#undef TICKS_TO_MS
	}
};

struct SVertexCostDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('v', "Vertex data", "['/VertexData/' (i32 StaticPolyCountZ) (i32 SkinnedPolyCountZ) (i32 VegetationPolyCountZ)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		IRenderer* pRenderer = gEnv->pRenderer;

		i32 nPolyCountZ = pRenderer->GetPolygonCountByType(EFSLIST_GENERAL, EVCT_STATIC, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_SHADOW_GEN, EVCT_STATIC, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_AW, EVCT_STATIC, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_BW, EVCT_STATIC, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_DECAL, EVCT_STATIC, 1);
		fr.AddValue(nPolyCountZ);

		nPolyCountZ = pRenderer->GetPolygonCountByType(EFSLIST_GENERAL, EVCT_SKINNED, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_SHADOW_GEN, EVCT_SKINNED, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_AW, EVCT_SKINNED, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_BW, EVCT_SKINNED, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_DECAL, EVCT_SKINNED, 1);
		fr.AddValue(nPolyCountZ);

		nPolyCountZ = pRenderer->GetPolygonCountByType(EFSLIST_GENERAL, EVCT_VEGETATION, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_SHADOW_GEN, EVCT_VEGETATION, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_AW, EVCT_VEGETATION, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_TRANSP_BW, EVCT_VEGETATION, 1);
		nPolyCountZ += pRenderer->GetPolygonCountByType(EFSLIST_DECAL, EVCT_VEGETATION, 1);
		fr.AddValue(nPolyCountZ);
	}
};

struct SParticlesDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('p', "particles", "['/Particles/' (float numParticlesRendered) (float numParticlesActive) (float numParticlesAllocated) "
		                                      "(float particleScreenFractionRendered) (float particleScreenFractionProcessed) "
		                                      "(float numEmittersRendered) (float numEmittersActive) (float numEmittersAllocated) "
		                                      "(float numParticlesReiterated) (float numParticlesRejected) "
		                                      "(float numParticlesCollideTest) (float numParticlesCollideHit) (float numParticlesClipped)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		SParticleCounts stats;
		gEnv->pParticleUpr->GetCounts(stats);

		float fScreenPix = (float)(gEnv->pRenderer->GetWidth() * gEnv->pRenderer->GetHeight());
		stats.pixels.updated  /= fScreenPix;
		stats.pixels.rendered /= fScreenPix;
		for (auto stat: stats)
			fr.AddValue(i32(stat));
	}
};

struct SWavicleDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription(
			'P', "Wavicle", "['/Wavicle/'"
			"(i32 emittersAlive)(i32 emittersUpdated)(i32 emittersRendererd)"
			"(i32 componentsAlive)(i32 componentsUpdated)(i32 componentsRendered)"
			"(i32 particlesAllocated)(i32 particlesAlive)(i32 particlesUpdated)(i32 particlesRendered)(i32 particlesClipped)"
			"]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		using namespace pfx2;

		SParticleStats stats;
		GetIParticleSystem()->GetStats(stats);
		
		for (auto stat: stats)
			fr.AddValue(i32(stat));
	}
};

struct SLocationDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('l', "location", "['/' (float posx) (float posy) (float posz) (float rotx) (float roty) (float rotz)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		Matrix33 m = Matrix33(GetISystem()->GetViewCamera().GetMatrix());
		Vec3 pos = GetISystem()->GetViewCamera().GetPosition();
		Ang3 rot = RAD2DEG(Ang3::GetAnglesXYZ(m));

		fr.AddValue(pos.x);
		fr.AddValue(pos.y);
		fr.AddValue(pos.z);

		fr.AddValue(rot.x);
		fr.AddValue(rot.y);
		fr.AddValue(rot.z);
	}
};

struct SPerCGFGPUProfilersDG : public IStatoscopeDataGroup
{
	CSimpleStringPool m_cattedCGFNames;

	SPerCGFGPUProfilersDG()
	// Frame IDs are currently problematic and will be ommitted
	//: SDataGroup('c', "per-cgf gpu profilers", "['/DrawCalls/$' (i32 frameID) (i32 totalDrawCallCount) (i32 numInstances)]", 4)
		: m_cattedCGFNames(true)
	{}

	virtual SDescription GetDescription() const
	{
		// Frame IDs are currently problematic and will be ommitted
		//: SDataGroup('c', "per-cgf gpu profilers", "['/DrawCalls/$' (i32 frameID) (i32 totalDrawCallCount) (i32 numInstances)]", 4)
		return SDescription('c', "per-cgf gpu profilers", "['/DrawCalls/$' (i32 totalDrawCallCount) (i32 numInstances)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
	#if defined(_RELEASE)
		DrxFatalError("Per-CGF GPU profilers not enabled in release");
	#else
		IRenderer* pRenderer = gEnv->pRenderer;

		pRenderer->CollectDrawCallsInfo(true);

		IRenderer::RNDrawcallsMapMesh& drawCallsInfo = gEnv->pRenderer->GetDrawCallsInfoPerMesh();

		auto pEnd = drawCallsInfo.end();
		auto pItor = drawCallsInfo.begin();

		string sPathName;
		sPathName.reserve(64);

		//Per RenderNode Stats
		for (; pItor != pEnd; ++pItor)
		{
			IRenderer::SDrawCallCountInfo& drawInfo = pItor->second;

			tukk pRenderMeshName = drawInfo.meshName;
			tukk pNameShort = strrchr(pRenderMeshName, '/');

			if (pNameShort)
			{
				pRenderMeshName = pNameShort + 1;
			}

			if (drawInfo.nShadows > 0)
			{
				sPathName = "Shadow/";
				sPathName += drawInfo.typeName;
				sPathName += "/";
				sPathName += pRenderMeshName;
				sPathName += "/";
				fr.AddValue(m_cattedCGFNames.Append(sPathName.c_str(), sPathName.length()));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue(drawInfo.nShadows);
				fr.AddValue(1);
			}
			if (drawInfo.nGeneral > 0)
			{
				sPathName = "Opaque/";
				sPathName += drawInfo.typeName;
				sPathName += "/";
				sPathName += pRenderMeshName;
				sPathName += "/";
				fr.AddValue(m_cattedCGFNames.Append(sPathName.c_str(), sPathName.length()));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue(drawInfo.nGeneral);
				fr.AddValue(1);
			}
			if (drawInfo.nTransparent > 0)
			{
				sPathName = "Transparent/";
				sPathName += drawInfo.typeName;
				sPathName += "/";
				sPathName += pRenderMeshName;
				sPathName += "/";
				fr.AddValue(m_cattedCGFNames.Append(sPathName.c_str(), sPathName.length()));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue(drawInfo.nTransparent);
				fr.AddValue(1);
			}
			if (drawInfo.nZpass > 0)
			{
				sPathName = "ZPass/";
				sPathName += drawInfo.typeName;
				sPathName += "/";
				sPathName += pRenderMeshName;
				sPathName += "/";
				fr.AddValue(m_cattedCGFNames.Append(sPathName.c_str(), sPathName.length()));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue(drawInfo.nZpass);
				fr.AddValue(1);
			}
			if (drawInfo.nMisc > 0)
			{
				sPathName = "Misc/";
				sPathName += drawInfo.typeName;
				sPathName += "/";
				sPathName += pRenderMeshName;
				sPathName += "/";
				fr.AddValue(m_cattedCGFNames.Append(sPathName.c_str(), sPathName.length()));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue(drawInfo.nMisc);
				fr.AddValue(1);
			}
		}

		//Flash
		{
			u32 numDPs = 0;
			u32 numTris = 0;
			if (gEnv->pScaleformHelper)
			{
				gEnv->pScaleformHelper->GetFlashRenderStats(numDPs, numTris);
			}

			if (numDPs)
			{
				fr.AddValue(m_cattedCGFNames.Append("Flash/Scaleform/All/", strlen("Flash/Scaleform/All/")));
				//fr.AddValue( batchStat->nFrameID );
				fr.AddValue((i32)numDPs);
				fr.AddValue(1);
			}
		}
	#endif
	}

	virtual u32 PrepareToWrite()
	{
		u32 drawProfilerCount = 0;
	#if !defined(_RELEASE)
		IRenderer* pRenderer = gEnv->pRenderer;
		pRenderer->CollectDrawCallsInfo(true);
		IRenderer::RNDrawcallsMapMesh& drawCallsInfo = gEnv->pRenderer->GetDrawCallsInfoPerMesh();
		auto pEnd = drawCallsInfo.end();
		auto pItor = drawCallsInfo.begin();

		//Per RenderNode Stats
		for (; pItor != pEnd; ++pItor)
		{
			IRenderer::SDrawCallCountInfo& pInfo = pItor->second;

			if (pInfo.nShadows > 0)
				drawProfilerCount++;
			if (pInfo.nGeneral > 0)
				drawProfilerCount++;
			if (pInfo.nTransparent > 0)
				drawProfilerCount++;
			if (pInfo.nZpass > 0)
				drawProfilerCount++;
			if (pInfo.nMisc > 0)
				drawProfilerCount++;
		}

		//Flash!
		{
			u32 numDPs = 0;
			u32 numTris = 0;
			if (gEnv->pScaleformHelper)
			{
				gEnv->pScaleformHelper->GetFlashRenderStats(numDPs, numTris);
			}

			if (numDPs > 0)
				drawProfilerCount++;
		}
	#endif
		return drawProfilerCount;
	}
};

struct SParticleProfilersDG : public IStatoscopeDataGroup
{
	void AddParticleInfo(const SParticleInfo& pi)
	{
		if (IsEnabled())
			m_particleInfos.push_back(pi);
	}

	virtual SDescription GetDescription() const
	{
		return SDescription('y', "ParticlesColliding", "['/ParticlesColliding/$/' (i32 count)]");
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();

		m_particleInfos.clear();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		for (u32 i = 0; i < m_particleInfos.size(); i++)
		{
			SParticleInfo& particleInfo = m_particleInfos[i];

			tukk pEffectName = particleInfo.name.c_str();

			fr.AddValue(pEffectName);
			fr.AddValue(particleInfo.numParticles);
		}

		m_particleInfos.clear();
	}

	virtual u32 PrepareToWrite()
	{
		return m_particleInfos.size();
	}

private:
	std::vector<SParticleInfo> m_particleInfos;
};

struct SPhysEntityProfilersDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('w', "PhysEntities", "['/PhysEntities/$/' (float time) (i32 nCalls) (float x) (float y) (float z)]");
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();

		m_physInfos.clear();
	}

	virtual u32 PrepareToWrite()
	{
		return m_physInfos.size();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		//poke CVar in physics
		gEnv->pPhysicalWorld->GetPhysVars()->bProfileEntities = 2;

		for (u32 i = 0; i < m_physInfos.size(); i++)
		{
			SPhysInfo& physInfo = m_physInfos[i];

			tukk pEntityName = physInfo.name.c_str();

			fr.AddValue(pEntityName);
			fr.AddValue(physInfo.time);
			fr.AddValue(physInfo.nCalls);
			fr.AddValue(physInfo.pos.x);
			fr.AddValue(physInfo.pos.y);
			fr.AddValue(physInfo.pos.z);
		}

		m_physInfos.clear();
	}

	std::vector<SPhysInfo> m_physInfos;
};

struct SFrameProfilersDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('r', "frame profilers", "['/Threads/$' (i32 count) (float selfTimeInMS) (float peak)]");
	}

	virtual void Enable()
	{
		IStatoscopeDataGroup::Enable();
		ICVar* pCV_profile = gEnv->pConsole->GetCVar("profile");
		if (pCV_profile)
			pCV_profile->Set(-1);
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();
		ICVar* pCV_profile = gEnv->pConsole->GetCVar("profile");
		if (pCV_profile)
			pCV_profile->Set(0);
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		for (u32 i = 0; i < m_frameProfilerRecords.size(); i++)
		{
			SPerfStatFrameProfilerRecord& fpr = m_frameProfilerRecords[i];
			string fpPath = GetFrameProfilerPath(fpr.m_pProfiler);
			fr.AddValue(fpPath.c_str());
			fr.AddValue(fpr.m_count);
			fr.AddValue(fpr.m_selfTime);
			fr.AddValue(fpr.m_peak);
		}

		m_frameProfilerRecords.clear();
	}

	virtual u32 PrepareToWrite()
	{
		return m_frameProfilerRecords.size();
	}

	std::vector<SPerfStatFrameProfilerRecord> m_frameProfilerRecords; // the most recent frame's profiler data
};

struct SPerfCountersDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('q', "performance counters", "['/PerfCounters/' (i32 lhsCount) (i32 iCacheMissCount)]");
	}

	virtual void Enable()
	{
		IStatoscopeDataGroup::Enable();
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		fr.AddValue(0);
		fr.AddValue(0);
	}
};

struct SUserMarkerDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('u', "user markers", "['/UserMarkers/$' (string name)]");
	}

	virtual void Disable()
	{
		IStatoscopeDataGroup::Disable();

		m_userMarkers.clear();
		m_tmpUserMarkers.clear();
	}

	virtual u32 PrepareToWrite()
	{
		m_tmpUserMarkers.clear();
		m_userMarkers.swap(m_tmpUserMarkers);
		return m_tmpUserMarkers.size();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		for (u32 i = 0; i < m_tmpUserMarkers.size(); i++)
		{
			fr.AddValue(m_tmpUserMarkers[i].m_path.c_str());
			fr.AddValue(m_tmpUserMarkers[i].m_name.c_str());
		}
	}

	std::vector<SUserMarker>   m_tmpUserMarkers;
	DrxMT::vector<SUserMarker> m_userMarkers;
};

struct SCallstacksDG : public IStatoscopeDataGroup
{
	CSimpleStringPool m_callstackAddressStrings;

	SCallstacksDG()
		: m_callstackAddressStrings(true)
	{}

	virtual SDescription GetDescription() const
	{
		return SDescription('k', "callstacks", "['/Callstacks/$' (string callstack)]");
	}

	virtual u32 PrepareToWrite()
	{
		m_tmpCallstacks.clear();
		m_callstacks.swap(m_tmpCallstacks);
		return m_tmpCallstacks.size();
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		for (u32 i = 0; i < m_tmpCallstacks.size(); i++)
		{
			const SCallstack& callstack = m_tmpCallstacks[i];
			u32 numAddresses = callstack.m_addresses.size();
			string callstackString;
			callstackString.reserve((numAddresses * 11) + 1); // see ptrStr

			for (u32 j = 0; j < numAddresses; j++)
			{
				char ptrStr[20]; // 0x + 0123456789ABCDEF + " " + '\0'
				drx_sprintf(ptrStr, "0x%p ", callstack.m_addresses[j]);
				callstackString += ptrStr;
			}

			fr.AddValue(callstack.m_tag.c_str());
			fr.AddValue(m_callstackAddressStrings.Append(callstackString.c_str(), callstackString.length()));
		}
	}

	DrxMT::vector<SCallstack> m_callstacks;
	std::vector<SCallstack>   m_tmpCallstacks;
};

struct SNetworkDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('n', "network", "['/Network/' (i32 TotalBandwidthSent) (i32 TotalBandwidthRecvd) (i32 TotalPacketsSent) (i32 LobbyBandwidthSent) (i32 LobbyPacketsSent) (i32 SeqBandwidthSent) (i32 SeqPacketsSent) (i32 FragmentedBandwidthSent) (i32 FragmentedPacketsSent) (i32 OtherBandwidthSent) (i32 OtherPacketsSent)]");
	}

	//this PREFAST_SUPPRESS_WARNING needs better investigation
	virtual void Write(IStatoscopeFrameRecord& fr) PREFAST_SUPPRESS_WARNING(6262)
	{
		if (gEnv->pNetwork)
		{
			SBandwidthStats bandwidthStats;
			gEnv->pNetwork->GetBandwidthStatistics(&bandwidthStats);

			memcpy(&bandwidthStats.m_prev, &m_prev, sizeof(SBandwidthStatsSubset));
			SBandwidthStatsSubset delta = bandwidthStats.TickDelta();
			memcpy(&m_prev, &bandwidthStats.m_total, sizeof(SBandwidthStatsSubset));

			fr.AddValue((i32)delta.m_totalBandwidthSent);
			fr.AddValue((i32)delta.m_totalBandwidthRecvd);
			fr.AddValue(delta.m_totalPacketsSent);
			fr.AddValue((i32)delta.m_lobbyBandwidthSent);
			fr.AddValue(delta.m_lobbyPacketsSent);
			fr.AddValue((i32)delta.m_seqBandwidthSent);
			fr.AddValue(delta.m_seqPacketsSent);
			fr.AddValue((i32)delta.m_fragmentBandwidthSent);
			fr.AddValue(delta.m_fragmentPacketsSent);
			fr.AddValue((i32)(delta.m_totalBandwidthSent - delta.m_lobbyBandwidthSent - delta.m_seqBandwidthSent - delta.m_fragmentBandwidthSent));
			fr.AddValue(delta.m_totalPacketsSent - delta.m_lobbyPacketsSent - delta.m_seqPacketsSent - delta.m_fragmentPacketsSent);
		}
		else
		{
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
			fr.AddValue(0);
		}
	}

	SBandwidthStatsSubset m_prev;
};

struct SChannelDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
	#if ENABLE_URGENT_RMIS
		return SDescription('z', "channel", "['/Channel/$/' (float BandwidthSent) (float BandwidthRecvd) (float PacketRate) (i32 MaxPacketSize) (i32 IdealPacketSize) (i32 SparePacketSize) (i32 UsedPacketSize) (i32 SentMessages) (i32 UnsentMessages) (i32 UrgentRMIs) (i32 Ping)]");
	#else
		return SDescription('z', "channel", "['/Channel/$/' (float BandwidthSent) (float BandwidthRecvd) (float PacketRate) (i32 MaxPacketSize) (i32 IdealPacketSize) (i32 SparePacketSize) (i32 UsedPacketSize) (i32 SentMessages) (i32 UnsentMessages) (i32 Ping)]");
	#endif // ENABLE_URGENT_RMIS
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		if (gEnv->pNetwork)
		{
			for (u32 index = 0; index < m_count; ++index)
			{
				fr.AddValue(m_bandwidthStats.m_channel[index].m_name);
				fr.AddValue(m_bandwidthStats.m_channel[index].m_bandwidthOutbound);
				fr.AddValue(m_bandwidthStats.m_channel[index].m_bandwidthInbound);
				fr.AddValue(m_bandwidthStats.m_channel[index].m_currentPacketRate);
				fr.AddValue((i32)m_bandwidthStats.m_channel[index].m_maxPacketSize);
				fr.AddValue((i32)m_bandwidthStats.m_channel[index].m_idealPacketSize);
				fr.AddValue((i32)m_bandwidthStats.m_channel[index].m_sparePacketSize);
				fr.AddValue((i32)m_bandwidthStats.m_channel[index].m_messageQueue.m_usedPacketSize);
				fr.AddValue(m_bandwidthStats.m_channel[index].m_messageQueue.m_sentMessages);
				fr.AddValue(m_bandwidthStats.m_channel[index].m_messageQueue.m_unsentMessages);
	#if ENABLE_URGENT_RMIS
				fr.AddValue(m_bandwidthStats.m_channel[index].m_messageQueue.m_urgentRMIs);
	#endif // ENABLE_URGENT_RMIS
				fr.AddValue((i32)m_bandwidthStats.m_channel[index].m_ping);
			}
		}
	}

	virtual u32 PrepareToWrite()
	{
		m_count = 0;
		if (gEnv->pNetwork)
		{
			gEnv->pNetwork->GetBandwidthStatistics(&m_bandwidthStats);

			for (u32 index = 0; index < STATS_MAX_NUMBER_OF_CHANNELS; ++index)
			{
				if (m_bandwidthStats.m_channel[index].m_inUse)
				{
					++m_count;
				}
			}
		}

		return m_count;
	}

protected:
	SBandwidthStats m_bandwidthStats;
	u32          m_count;
};

struct SNetworkProfileDG : public IStatoscopeDataGroup
{
	virtual SDescription GetDescription() const
	{
		return SDescription('d', "network profile", "['/NetworkProfile/$' (i32 totalBits) (i32 seqBits) (i32 rmiBits) (i32 calls)]");
	}

	virtual void Write(IStatoscopeFrameRecord& fr)
	{
		for (u32 i = 0; i < m_statsCache.size(); i++)
		{
			SProfileInfoStat& nps = m_statsCache[i];
			fr.AddValue(nps.m_name.c_str());
			fr.AddValue((i32)nps.m_totalBits); //-- as bits, not Kbits, so we can graph against the bandwidth stats on a comparable scale.
			fr.AddValue((i32)(nps.m_rmi ? 0 : nps.m_totalBits));
			fr.AddValue((i32)(nps.m_rmi ? nps.m_totalBits : 0));
			fr.AddValue((i32)nps.m_calls);
		}
	}

	virtual u32 PrepareToWrite()
	{
		m_statsCache.clear();

		if (gEnv->pNetwork)
		{
			SNetworkProfilingStats profileStats;
			gEnv->pNetwork->GetProfilingStatistics(&profileStats);

			for (i32 i = 0; i < profileStats.m_ProfileInfoStats.size(); i++)
			{
				SProfileInfoStat& nps = profileStats.m_ProfileInfoStats[i];
				if (nps.m_totalBits)
				{
					m_statsCache.push_back(nps);
				}
			}
		}

		return m_statsCache.size();
	}

	std::vector<SProfileInfoStat> m_statsCache;
};

class CStreamingObjectIntervalGroup : public CStatoscopeIntervalGroup, public IStreamedObjectListener
{
public:
	enum
	{
		Stage_Unloaded,
		Stage_Requested,
		Stage_LoadedUnused,
		Stage_LoadedUsed,
	};

public:
	CStreamingObjectIntervalGroup()
		: CStatoscopeIntervalGroup('o', "streaming objects",
		                           "['/Objects/' "
		                           "(string filename) "
		                           "(i32 stage) "
		                           "]")
	{
	}

	void Enable_Impl()
	{
		gEnv->p3DEngine->SetStreamableListener(this);
	}

	void Disable_Impl()
	{
		gEnv->p3DEngine->SetStreamableListener(NULL);
	}

	void WriteChangeStageBlockEvent(CStatoscopeEventWriter* pWriter, ukk pHandle, i32 stage)
	{
		size_t payloadLen = GetValueLength(stage);
		StatoscopeDataWriter::EventModifyInterval* pEv = pWriter->BeginBlockEvent<StatoscopeDataWriter::EventModifyInterval>(payloadLen);
		pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
		pEv->classId = GetId();
		pEv->field = StatoscopeDataWriter::EventModifyInterval::FieldSplitIntervalMask | 1;

		tuk pPayload = (tuk)(pEv + 1);
		WriteValue(pPayload, stage);

		pWriter->EndBlockEvent();
	}

	void WriteChangeStageEvent(ukk pHandle, i32 stage)
	{
		CStatoscopeEventWriter* pWriter = GetWriter();

		if (pWriter)
		{
			pWriter->BeginBlock();
			WriteChangeStageBlockEvent(pWriter, pHandle, stage);
			pWriter->EndBlock();
		}
	}

public: // IStreamedObjectListener Members
	virtual void OnCreatedStreamedObject(tukk filename, uk pHandle)
	{
		CStatoscopeEventWriter* pWriter = GetWriter();

		if (pWriter)
		{
			size_t payloadLen =
			  GetValueLength(filename) +
			  GetValueLength(0)
			;

			StatoscopeDataWriter::EventBeginInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventBeginInterval>(payloadLen);
			pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
			pEv->classId = GetId();

			tuk pPayload = (tuk)(pEv + 1);
			WriteValue(pPayload, filename);
			WriteValue(pPayload, Stage_Unloaded);

			pWriter->EndEvent();
		}
	}

	virtual void OnRequestedStreamedObject(uk pHandle)
	{
		WriteChangeStageEvent(pHandle, Stage_Requested);
	}

	virtual void OnReceivedStreamedObject(uk pHandle)
	{
		WriteChangeStageEvent(pHandle, Stage_LoadedUnused);
	}

	virtual void OnUnloadedStreamedObject(uk pHandle)
	{
		WriteChangeStageEvent(pHandle, Stage_Unloaded);
	}

	void OnBegunUsingStreamedObjects(uk * pHandles, size_t numHandles)
	{
		CStatoscopeEventWriter* pWriter = GetWriter();

		if (pWriter)
		{
			pWriter->BeginBlock();

			for (size_t i = 0; i < numHandles; ++i)
				WriteChangeStageBlockEvent(pWriter, pHandles[i], Stage_LoadedUsed);

			pWriter->EndBlock();
		}
	}

	void OnEndedUsingStreamedObjects(uk * pHandles, size_t numHandles)
	{
		CStatoscopeEventWriter* pWriter = GetWriter();

		if (pWriter)
		{
			pWriter->BeginBlock();

			for (size_t i = 0; i < numHandles; ++i)
				WriteChangeStageBlockEvent(pWriter, pHandles[i], Stage_LoadedUnused);

			pWriter->EndBlock();
		}
	}

	virtual void OnDestroyedStreamedObject(uk pHandle)
	{
		CStatoscopeEventWriter* pWriter = GetWriter();

		if (pWriter)
		{
			StatoscopeDataWriter::EventEndInterval* pEv = pWriter->BeginEvent<StatoscopeDataWriter::EventEndInterval>();
			pEv->id = reinterpret_cast<UINT_PTR>(pHandle);
			pWriter->EndEvent();
		}
	}
};

CStatoscopeEventWriter::CStatoscopeEventWriter()
	: m_eventNextSequence(0)
	, m_lastTimestampUs(0)
{
}

void CStatoscopeEventWriter::Flush(CDataWriter* pWriter)
{
	{
		using std::swap;
		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_eventStreamLock);
		swap(m_eventStream, m_eventStreamTmp);
	}

	u32 size = (u32)m_eventStreamTmp.size();
	pWriter->WriteData(size);
	if (size)
		pWriter->WriteData(&m_eventStreamTmp[0], size);

	#ifndef _RELEASE
	if (!m_eventStreamTmp.empty())
	{
		tuk pBuff = (tuk)&m_eventStreamTmp[0];
		tuk pBuffEnd = pBuff + size;
		StatoscopeDataWriter::EventHeader* pHdr = (StatoscopeDataWriter::EventHeader*)pBuff;
		uint64 firstTimestampUs = pHdr->timeStampUs;

		pBuff += pHdr->eventLengthInWords * 4;
		while (pBuff < pBuffEnd)
		{
			pHdr = (StatoscopeDataWriter::EventHeader*)pBuff;

			if (pHdr->timeStampUs < firstTimestampUs)
				__debugbreak();
			firstTimestampUs = pHdr->timeStampUs;

			pBuff += pHdr->eventLengthInWords * 4;
		}
	}
	#endif

	m_eventStreamTmp.clear();
}

void CStatoscopeEventWriter::Reset()
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_eventStreamLock);
	stl::free_container(m_eventStream);
	stl::free_container(m_eventStreamTmp);
	m_eventNextSequence = 0;
}

CStatoscope::CStatoscope()
{
	m_lastDumpTime = 0.f;
	m_screenshotLastCaptureTime = 0.f;
	m_activeDataGroupMask = 0;
	m_activeIvDataGroupMask = 0;
	m_groupMaskInitialized = false;
	m_lastScreenWidth = 0;
	m_lastScreenHeight = 0;

	m_pScreenShotBuffer = NULL;
	m_ScreenShotState = eSSCS_Idle;

	m_bLevelStarted = false;

	RegisterBuiltInDataGroups();
	RegisterBuiltInIvDataGroups();

	m_pStatoscopeEnabledCVar = REGISTER_INT("e_StatoscopeEnabled", 0, VF_NULL, "Controls whether all statoscope is enabled.");
	m_pStatoscopeDumpAllCVar = REGISTER_INT("e_StatoscopeDumpAll", 0, VF_NULL, "Controls whether all functions are dumped in a profile log.");
	m_pStatoscopeDataGroupsCVar = REGISTER_INT64("e_StatoscopeDataGroups", AlphaBits64("fgmrtuO"), VF_BITFIELD, GetDataGroupsCVarHelpString(m_allDataGroups));
	m_pStatoscopeIvDataGroupsCVar = REGISTER_INT64("e_StatoscopeIvDataGroups", m_activeIvDataGroupMask, VF_BITFIELD, GetDataGroupsCVarHelpString(m_intervalGroups));
	m_pStatoscopeLogDestinationCVar = REGISTER_INT_CB("e_StatoscopeLogDestination", eLD_Socket, VF_NULL, "Where the Statoscope log gets written to:\n  0 - file\n  1 - socket\n  2 - telemetry server (default)", OnLogDestinationCVarChange);  // see ELogDestination
	m_pStatoscopeScreenshotCapturePeriodCVar = REGISTER_FLOAT("e_StatoscopeScreenshotCapturePeriod", -1.0f, VF_NULL, "How many seconds between Statoscope screenshot captures (-1 to disable).");
	m_pStatoscopeFilenameUseBuildInfoCVar = REGISTER_INT("e_StatoscopeFilenameUseBuildInfo", 1, VF_NULL, "Set to include the platform and build number in the log filename.");
	m_pStatoscopeFilenameUseMapCVar = REGISTER_INT("e_StatoscopeFilenameUseMap", 0, VF_NULL, "Set to include the map name in the log filename.");
	m_pStatoscopeFilenameUseTagCvar = REGISTER_STRING_CB("e_StatoscopeFilenameUseTag", "", VF_NULL, "Set to include tag in the log file name.", OnTagCVarChange);
	m_pStatoscopeFilenameUseTimeCVar = REGISTER_INT("e_StatoscopeFilenameUseTime", 0, VF_NULL, "Set to include the time and date in the log filename.");
	m_pStatoscopeFilenameUseDatagroupsCVar = REGISTER_INT("e_StatoscopeFilenameUseDatagroups", 0, VF_NULL, "Set to include the datagroup and date in the log filename.");
	m_pStatoscopeMinFuncLengthMsCVar = REGISTER_FLOAT("e_StatoscopeMinFuncLengthMs", 0.01f, VF_NULL, "Min func duration (ms) to be logged by statoscope.");
	m_pStatoscopeMaxNumFuncsPerFrameCVar = REGISTER_INT("e_StatoscopeMaxNumFuncsPerFrame", 150, VF_NULL, "Max number of funcs to log per frame.");
	m_pStatoscopeCreateLogFilePerLevelCVar = REGISTER_INT("e_StatoscopeCreateLogFilePerLevel", 0, VF_NULL, "Create a new perflog file per level.");
	m_pStatoscopeWriteTimeout = REGISTER_FLOAT("e_StatoscopeWriteTimeout", 1.0f, VF_NULL, "The number of seconds the data writer will stall before it gives up trying to write data (currently only applies to the telemetry data writer).");
	m_pStatoscopeConnectTimeout = REGISTER_FLOAT("e_StatoscopeConnectTimeout", 5.0f, VF_NULL, "The number of seconds the data writer will stall while trying connect to the telemetry server.");
	m_pStatoscopeAllowFPSOverrideCVar = REGISTER_INT("e_StatoscopeAllowFpsOverride", 1, VF_NULL, "Allow overriding of cvars in release for fps captures (MP only).");
	m_pGameRulesCVar = NULL;

	m_logNum = 1;

	REGISTER_COMMAND("e_StatoscopeAddUserMarker", &ConsoleAddUserMarker, 0, "Add a user marker to the perf stat logging for this frame");

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CStatoscope");

	DrxCreateDirectory("%USER%/statoscope");

	m_pServer = new CStatoscopeServer(this);

	m_pDataWriter = NULL;
}

CStatoscope::~CStatoscope()
{
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
	gEnv->pRenderer->UnRegisterCaptureFrame(this);
	delete[] m_pScreenShotBuffer;
	m_pScreenShotBuffer = NULL;

	SAFE_DELETE(m_pDataWriter);
	SAFE_DELETE(m_pServer);
}

static tuk Base64Encode(u8k* buffer, i32 len)
{
	DRX_PROFILE_REGION(PROFILE_SYSTEM, "CStatoscope::Base64Encode");

	static const char base64Dict[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
	};
	i32 b64Len = ((len + 2) / 3) * 4;
	tuk b64Buf = new char[b64Len + 1];
	i32 byteCount = 0;
	i32 cycle = 0;

	for (i32 i = 0; i < b64Len; i++)
	{
		u8 val = 0;

		switch (cycle)
		{
		case 0:
			val = buffer[byteCount] >> 2;
			break;
		case 1:
			val = (buffer[byteCount++] & 0x3) << 4;
			val |= buffer[byteCount] >> 4;
			break;
		case 2:
			val = (buffer[byteCount++] & 0xf) << 2;
			val |= buffer[byteCount] >> 6;
			break;
		case 3:
			val = buffer[byteCount++] & 0x3f;
			break;
		}

		cycle = (cycle + 1) & 3;

		b64Buf[i] = base64Dict[val];
	}

	b64Buf[b64Len] = '\0';

	return b64Buf;
}

bool CStatoscope::RegisterDataGroup(IStatoscopeDataGroup* pDG)
{
	IStatoscopeDataGroup::SDescription desc = pDG->GetDescription();

	for (std::vector<CStatoscopeDataGroup*>::iterator it = m_allDataGroups.begin(), itEnd = m_allDataGroups.end(); it != itEnd; ++it)
	{
		CStatoscopeDataGroup* pGroup = *it;

		if (pGroup->GetId() == desc.key)
			return false;
	}

	CStatoscopeDataGroup* pNewGroup = new CStatoscopeDataGroup(desc, pDG);
	m_allDataGroups.push_back(pNewGroup);

	return true;
}

void CStatoscope::UnregisterDataGroup(IStatoscopeDataGroup* pDG)
{
	for (std::vector<CStatoscopeDataGroup*>::iterator it = m_activeDataGroups.begin(), itEnd = m_activeDataGroups.end(); it != itEnd; ++it)
	{
		CStatoscopeDataGroup* pGroup = *it;
		IStatoscopeDataGroup* pCallback = pGroup->GetCallback();

		if (pCallback == pDG)
		{
			pDG->Disable();
			m_activeDataGroups.erase(it);
			break;
		}
	}

	for (std::vector<CStatoscopeDataGroup*>::iterator it = m_allDataGroups.begin(), itEnd = m_allDataGroups.end(); it != itEnd; ++it)
	{
		CStatoscopeDataGroup* pGroup = *it;
		IStatoscopeDataGroup* pCallback = pGroup->GetCallback();

		if (pCallback == pDG)
		{
			delete *it;
			m_allDataGroups.erase(it);
			break;
		}
	}
}

void CStatoscope::Tick()
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	if (m_pStatoscopeEnabledCVar->GetIVal() != 0)
	{
		CreateDataWriter();

		if (m_pDataWriter && m_pDataWriter->Open())
		{
			SetIsRunning(true);

			uint64 currentActiveDataGroupMask = (uint64)m_pStatoscopeDataGroupsCVar->GetI64Val();
			uint64 differentDGs = currentActiveDataGroupMask ^ m_activeDataGroupMask;

			if (differentDGs)
			{
				uint64 enabledDGs = differentDGs & currentActiveDataGroupMask;
				uint64 disabledDGs = differentDGs & ~currentActiveDataGroupMask;
				SetDataGroups(enabledDGs, disabledDGs);

				m_activeDataGroupMask = currentActiveDataGroupMask;
				m_groupMaskInitialized = true;
			}

			AddFrameRecord(differentDGs != 0);
		}
	}
	else if (IsRunning())
	{
		DrxLogAlways("Flushing Statoscope log\n");
		if (m_pDataWriter)
		{
			m_pDataWriter->Close();
		}

		for (IntervalGroupVec::const_iterator it = m_intervalGroups.begin(), itEnd = m_intervalGroups.end(); it != itEnd; ++it)
			(*it)->Disable();

		for (std::vector<CStatoscopeDataGroup*>::iterator it = m_activeDataGroups.begin(), itEnd = m_activeDataGroups.end(); it != itEnd; ++it)
			(*it)->GetCallback()->Disable();

		m_activeDataGroups.clear();
		m_activeDataGroupMask = 0;

		m_eventWriter.Reset();

		SetIsRunning(false);
	}
}

void CStatoscope::SetCurrentProfilerRecords(const std::vector<CFrameProfiler*>* profilers)
{
	if (m_pFrameProfilers)
	{
		// we want to avoid reallocation of m_perfStatDumpProfilers
		// even if numProfilers is quite large (in the thousands), it'll only be tens of KB
		u32 numProfilers = profilers->size();
		m_perfStatDumpProfilers.clear();
		m_perfStatDumpProfilers.reserve(std::max((size_t)numProfilers, m_perfStatDumpProfilers.size()));		

		float minFuncTime = m_pStatoscopeMinFuncLengthMsCVar->GetFVal();

		int64 smallFuncs = 0;
		u32 smallFuncsCount = 0;

		for (u32 i = 0; i < numProfilers; i++)
		{
			CFrameProfiler* pProfiler = (*profilers)[i];

			// ignore really quick functions or ones what weren't called
			if (1000.f * gEnv->pTimer->TicksToSeconds(pProfiler->m_selfTime) > minFuncTime)
			{
				m_perfStatDumpProfilers.push_back(std::make_pair(pProfiler, pProfiler->m_selfTime));
			}
			else
			{
				smallFuncs += pProfiler->m_selfTime;
				smallFuncsCount++;
			}
		}

		std::sort(m_perfStatDumpProfilers.begin(), m_perfStatDumpProfilers.end(), CCompareFrameProfilersSelfTime());

		bool bDumpAll = false;

		if (m_pStatoscopeDumpAllCVar->GetIVal())
		{
			bDumpAll = true;
		}

		if (!bDumpAll)
		{
			u32 maxNumFuncs = (u32)m_pStatoscopeMaxNumFuncsPerFrameCVar->GetIVal();
			// limit the number being recorded
			m_perfStatDumpProfilers.resize(std::min(m_perfStatDumpProfilers.size(), (size_t)maxNumFuncs));
		}

		u32 numDumpProfilers = m_perfStatDumpProfilers.size();
		std::vector<SPerfStatFrameProfilerRecord>& records = m_pFrameProfilers->m_frameProfilerRecords;

		records.reserve(numDumpProfilers);
		records.resize(0);

		for (u32 i = 0; i < numDumpProfilers; i++)
		{
			CFrameProfiler* pProfiler = m_perfStatDumpProfilers[i].first;
			int64 selfTime = m_perfStatDumpProfilers[i].second;
			SPerfStatFrameProfilerRecord profilerRecord;

			profilerRecord.m_pProfiler = pProfiler;
			profilerRecord.m_count = pProfiler->m_count;
			profilerRecord.m_selfTime = 1000.f * gEnv->pTimer->TicksToSeconds(selfTime);
			profilerRecord.m_variance = pProfiler->m_variance;
			profilerRecord.m_peak = 1000.f * gEnv->pTimer->TicksToSeconds(pProfiler->m_peak);

			records.push_back(profilerRecord);
		}

		if (bDumpAll)
		{
			SPerfStatFrameProfilerRecord profilerRecord;

			profilerRecord.m_pProfiler = NULL;
			profilerRecord.m_count = smallFuncsCount;
			profilerRecord.m_selfTime = 1000.f * gEnv->pTimer->TicksToSeconds(smallFuncs);
			profilerRecord.m_variance = 0;

			records.push_back(profilerRecord);
		}
	}
}

void CStatoscope::AddParticleEffect(tukk pEffectName, i32 count)
{
	if (m_pParticleProfilers)
	{
		SParticleInfo p = { pEffectName, count };
		m_pParticleProfilers->AddParticleInfo(p);
	}
}

void CStatoscope::AddPhysEntity(const phys_profile_info* pInfo)
{
	if (m_pPhysEntityProfilers && m_pPhysEntityProfilers->IsEnabled())
	{
		float dt = 1000.f * gEnv->pTimer->TicksToSeconds(pInfo->nTicksLast * 1024);

		if (dt > 0.f)
		{
			static tukk peTypes[] =
			{
				"None",         //PE_NONE=0,
				"Static",       //PE_STATIC=1,
				"Rigid",        //PE_RIGID=2,
				"Wheeled",      //PE_WHEELEDVEHICLE=3,
				"Living",       //PE_LIVING=4,
				"Particle",     //PE_PARTICLE=5,
				"Articulated",  //PE_ARTICULATED=6,
				"Rope",         //PE_ROPE=7,
				"Soft",         //PE_SOFT=8,
				"Area"          //PE_AREA=9
			};

			pe_type type = pInfo->pEntity->GetType();

			i32 numElems = sizeof(peTypes) / sizeof(tuk);

			if (type >= numElems)
			{
				DrxFatalError("peTypes enum has changed, please update statoscope CStatoscope::AddPhysEntity");
			}

			tukk pEntName = pInfo->pName;

			IRenderNode* pRenderNode = (IRenderNode*)pInfo->pEntity->GetForeignData(PHYS_FOREIGN_ID_STATIC);

			if (pRenderNode)
			{
				pEntName = pRenderNode->GetName();
			}

			//extra '/'s will break the log output
			tukk pLastSlash = strrchr(pEntName, '/');
			pEntName = pLastSlash ? pLastSlash + 1 : pEntName;

			string name;
			assert(type < 10);
			PREFAST_ASSUME(type < 10);
			name.Format("%s/%s", peTypes[type], pEntName);

			pe_status_pos status_pos;
			status_pos.pos = Vec3(0.f, 0.f, 0.f);

			pInfo->pEntity->GetStatus(&status_pos);

			SPhysInfo p = { name.c_str(), dt, pInfo->nCalls, status_pos.pos };
			m_pPhysEntityProfilers->m_physInfos.push_back(p);
		}
	}
}

void CStatoscope::CreateTelemetryStream(tukk postHeader, tukk hostname, i32 port)
{
	SAFE_DELETE(m_pDataWriter);
	if (postHeader)
	{
		m_pDataWriter = new CTelemetryDataWriter(postHeader, hostname, port, m_pStatoscopeWriteTimeout->GetFVal(), m_pStatoscopeConnectTimeout->GetFVal());
	}
}

void CStatoscope::CloseTelemetryStream()
{
	SAFE_DELETE(m_pDataWriter);
}

void CStatoscope::PrepareScreenShot()
{
	i32k widthDelta  = m_lastScreenWidth  - gEnv->pRenderer->GetWidth();
	i32k heightDelta = m_lastScreenHeight - gEnv->pRenderer->GetHeight();

	m_lastScreenWidth  = gEnv->pRenderer->GetWidth();
	m_lastScreenHeight = gEnv->pRenderer->GetHeight();

	DRX_ASSERT(gEnv->pRenderer->GetWidth () == gEnv->pRenderer->GetOverlayWidth ());
	DRX_ASSERT(gEnv->pRenderer->GetHeight() == gEnv->pRenderer->GetOverlayHeight());

	i32k shrunkenWidthNotAligned = OnGetFrameWidth();
	i32k shrunkenWidth = shrunkenWidthNotAligned - (shrunkenWidthNotAligned % 4);
	i32k shrunkenHeight = OnGetFrameHeight();

	if (!m_pScreenShotBuffer || widthDelta != 0 || heightDelta != 0)
	{
		SAFE_DELETE_ARRAY(m_pScreenShotBuffer);

		const size_t SCREENSHOT_BIT_DEPTH = 4;
		const size_t bufferSize = shrunkenWidth * shrunkenHeight * SCREENSHOT_BIT_DEPTH;
		m_pScreenShotBuffer = new u8[bufferSize];
		memset(m_pScreenShotBuffer, 0, bufferSize * sizeof(u8));
		gEnv->pRenderer->RegisterCaptureFrame(this);
	}

}

u8* CStatoscope::ProcessScreenShot()
{
	//Reserved bytes in buffer indicate size and scale
	enum { SCREENSHOT_SCALED_WIDTH, SCREENSHOT_SCALED_HEIGHT, SCREENSHOT_MULTIPLIER };
	u8* pScreenshotBuf = NULL;

	if (m_ScreenShotState == eSSCS_DataReceived)
	{
		i32k SCREENSHOT_BIT_DEPTH = 4;
		i32k SCREENSHOT_TARGET_BIT_DEPTH = 3;
		i32k shrunkenWidthNotAligned = OnGetFrameWidth();
		i32k shrunkenWidth = shrunkenWidthNotAligned - shrunkenWidthNotAligned % 4;
		i32k shrunkenHeight = OnGetFrameHeight();

		const size_t bufferSize = 3 + (shrunkenWidth * shrunkenHeight * SCREENSHOT_TARGET_BIT_DEPTH);
		pScreenshotBuf = new u8[bufferSize];

		if (pScreenshotBuf)
		{
			pScreenshotBuf[SCREENSHOT_MULTIPLIER] = (u8)((max(shrunkenWidth, shrunkenHeight) + UCHAR_MAX) / UCHAR_MAX);            //Scaling factor
			pScreenshotBuf[SCREENSHOT_SCALED_WIDTH] = (u8)(shrunkenWidth / pScreenshotBuf[SCREENSHOT_MULTIPLIER]);
			pScreenshotBuf[SCREENSHOT_SCALED_HEIGHT] = (u8)(shrunkenHeight / pScreenshotBuf[SCREENSHOT_MULTIPLIER]);
			i32 iSrcPixel = 0;
			i32 iDstPixel = 3;

			while (iSrcPixel < shrunkenWidth * shrunkenHeight * SCREENSHOT_BIT_DEPTH)
			{
				pScreenshotBuf[iDstPixel + 0] = m_pScreenShotBuffer[iSrcPixel++];
				pScreenshotBuf[iDstPixel + 1] = m_pScreenShotBuffer[iSrcPixel++];
				pScreenshotBuf[iDstPixel + 2] = m_pScreenShotBuffer[iSrcPixel++];

				iSrcPixel++;
				iDstPixel += SCREENSHOT_TARGET_BIT_DEPTH;
			}

			m_ScreenShotState = eSSCS_Idle;
		}
	}

	return pScreenshotBuf;
}

void CStatoscope::AddFrameRecord(bool bOutputHeader)
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	float currentTime = gEnv->pTimer->GetAsyncTime().GetSeconds();

	CStatoscopeFrameRecordWriter fr(m_pDataWriter);

	u8* pScreenshot = NULL;

	//Screen shot in progress, attempt to process
	if (m_ScreenShotState == eSSCS_DataReceived)
	{
		pScreenshot = ProcessScreenShot();
	}

	//auto screen shot logic
	float screenshotCapturePeriod = m_pStatoscopeScreenshotCapturePeriodCVar->GetFVal();

	if ((m_ScreenShotState == eSSCS_Idle) && (screenshotCapturePeriod >= 0.0f))
	{
		if (currentTime >= m_screenshotLastCaptureTime + screenshotCapturePeriod)
		{
			//Tell the Render thread to dump the mini screenshot to main memory
			//Then wait for the callback to tell us the data is ready
			RequestScreenShot();
			m_screenshotLastCaptureTime = currentTime;
		}
	}

	if (m_pDataWriter->m_bShouldOutputLogTopHeader)
	{
	#if defined(NEED_ENDIAN_SWAP)
		m_pDataWriter->WriteData((char)StatoscopeDataWriter::EE_BigEndian);
	#else
		m_pDataWriter->WriteData((char)StatoscopeDataWriter::EE_LittleEndian);
	#endif

		m_pDataWriter->WriteData(STATOSCOPE_BINARY_VERSION);

		//Using string pool?
		m_pDataWriter->WriteData(m_pDataWriter->IsUsingStringPool());

		for (IntervalGroupVec::const_iterator it = m_intervalGroups.begin(), itEnd = m_intervalGroups.end(); it != itEnd; ++it)
			(*it)->Disable();

		m_eventWriter.Reset();
		WriteIntervalClassEvents();

		uint64 ivGroups = m_pStatoscopeIvDataGroupsCVar->GetI64Val();

		for (IntervalGroupVec::const_iterator it = m_intervalGroups.begin(), itEnd = m_intervalGroups.end(); it != itEnd; ++it)
		{
			if (AlphaBit64((*it)->GetId()) & ivGroups)
				(*it)->Enable(&m_eventWriter);
		}

		bOutputHeader = true;
		m_pDataWriter->m_bShouldOutputLogTopHeader = false;
	}

	if (bOutputHeader)
	{
		m_pDataWriter->WriteData(true);

		//Module info
		if (m_activeDataGroupMask & AlphaBit64('k') && !m_pDataWriter->m_bHaveOutputModuleInformation)
		{
			m_pDataWriter->WriteData(true);
			OutputLoadedModuleInformation(m_pDataWriter);
		}
		else
		{
			m_pDataWriter->WriteData(false);
		}

		m_pDataWriter->WriteData((i32)m_activeDataGroups.size());

		for (u32 i = 0; i < m_activeDataGroups.size(); i++)
		{
			CStatoscopeDataGroup* dataGroup = m_activeDataGroups[i];
			dataGroup->WriteHeader(m_pDataWriter);
		}
	}
	else
	{
		m_pDataWriter->WriteData(false);
	}

	//
	// 1. Frame time
	//
	m_pDataWriter->WriteData(currentTime);

	//
	// 2. Screen shot
	//
	if (pScreenshot)
	{
		m_pDataWriter->WriteData(StatoscopeDataWriter::B64Texture);
		i32 screenShotSize = 3 + ((pScreenshot[0] * pScreenshot[2]) * (pScreenshot[1] * pScreenshot[2]) * 3); // width,height,scale + (width*scale * height*scale * 3bpp)
		m_pDataWriter->WriteData(screenShotSize);
		m_pDataWriter->WriteData(pScreenshot, screenShotSize);
		SAFE_DELETE_ARRAY(pScreenshot);
	}
	else
	{
		m_pDataWriter->WriteData(StatoscopeDataWriter::None);
	}

	//
	// 3. Data groups
	//
	for (u32 i = 0; i < m_activeDataGroups.size(); i++)
	{
		CStatoscopeDataGroup& dataGroup = *m_activeDataGroups[i];
		IStatoscopeDataGroup* pCallback = dataGroup.GetCallback();

		//output how many data sets to expect
		i32 nDataSets = pCallback->PrepareToWrite();
		m_pDataWriter->WriteData(nDataSets);

		fr.ResetWrittenElementCount();
		pCallback->Write(fr);

		i32 nElementsWritten = fr.GetWrittenElementCount();
		i32 nExpectedElems = dataGroup.GetNumElements() * nDataSets;

		if (nExpectedElems != nElementsWritten)
			DrxFatalError("Statoscope data group: %s is broken. Check data group declaration", dataGroup.GetName());
	}

	//
	// 4. Events
	//
	m_eventWriter.Flush(m_pDataWriter);

	// 5. Magic Number, indicate end of frame record
	m_pDataWriter->WriteData(0xdeadbeef);
}

void CStatoscope::Flush()
{
	if (m_pDataWriter)
		m_pDataWriter->Flush();
}

bool CStatoscope::RequiresParticleStats(bool& bEffectStats)
{
	bool bGlobalStats = (m_activeDataGroupMask & AlphaBit64('p')) != 0;
	bool bCollisionStats = (m_activeDataGroupMask & AlphaBit64('y')) != 0;

	bool bRequiresParticleStats = false;
	bEffectStats = false;

	if (bCollisionStats)
	{
		bEffectStats = true;
		bRequiresParticleStats = true;
	}
	else if (bGlobalStats)
	{
		bRequiresParticleStats = true;
	}

	return bRequiresParticleStats;
}

void CStatoscope::SetLogFilename()
{
	m_logFilename = "%USER%/statoscope/perf";

	if (m_pStatoscopeFilenameUseBuildInfoCVar->GetIVal() > 0)
	{
	#if DRX_PLATFORM_DURANGO
		m_logFilename += "_XBOXONE";
	#elif DRX_PLATFORM_ORBIS
		m_logFilename += "_PS4";
	#elif DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
		m_logFilename += "_WIN64";
	#elif DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
		m_logFilename += "_WIN32";
	#elif DRX_PLATFORM_MAC
		m_logFilename += "_MAC";
	#elif DRX_PLATFORM_ANDROID
		m_logFilename += "_ANDROID";
	#elif DRX_PLATFORM_LINUX && DRX_PLATFORM_64BIT
		m_logFilename += "_LINUX64";
	#elif DRX_PLATFORM_LINUX && DRX_PLATFORM_32BIT
		m_logFilename += "_LINUX32";
	#elif DRX_PLATFORM_IOS && DRX_PLATFORM_64BIT
		m_logFilename += "_IOS64";
	#elif DRX_PLATFORM_IOS && DRX_PLATFORM_32BIT
		m_logFilename += "_IOS32";
	#else
		m_logFilename += "_UNKNOWNPLAT";
	#endif

		const SFileVersion& ver = gEnv->pSystem->GetFileVersion();
		char versionString[64];
		drx_sprintf(versionString, "_%d_%d_%d_%d", ver.v[3], ver.v[2], ver.v[1], ver.v[0]);

		m_logFilename += versionString;
	}

	if (strcmp(m_pStatoscopeFilenameUseTagCvar->GetString(), "") != 0)
	{
		m_logFilename += "_";
		m_logFilename += m_pStatoscopeFilenameUseTagCvar->GetString();
	}

	if (m_pStatoscopeFilenameUseMapCVar->GetIVal() > 0)
	{
		tukk mapName = NULL;

		//If we don't have a last map loaded, try to look up the current one now
		if (m_currentMap.empty())
		{
			if (gEnv->pGameFramework)
			{
				mapName = gEnv->pGameFramework->GetLevelName();
			}
		}
		//If we tracked the last map loaded then use it here
		else
		{
			mapName = m_currentMap.c_str();
		}

		if (mapName)
		{
			tukk nameNoDir = strrchr(mapName, '/');

			// strip directory for now
			if (nameNoDir)
			{
				mapName = nameNoDir + 1;
			}

			string mapNameTrucated(mapName);

			m_logFilename += "_";
			i32 mapNameLenLimit = 32;
			if (m_pStatoscopeFilenameUseBuildInfoCVar->GetIVal() > 0)
			{
				mapNameLenLimit = 12;
			}
			m_logFilename += mapNameTrucated.Left(mapNameLenLimit);
		}
	}

	if (m_pStatoscopeFilenameUseDatagroupsCVar->GetIVal() > 0)
	{
		string datagroups = "_";
		uint64 currentActiveDataGroupMask = (uint64)m_pStatoscopeDataGroupsCVar->GetI64Val();
		for (u32 i = 0, ic = m_allDataGroups.size(); i < ic; i++)
		{
			CStatoscopeDataGroup& dataGroup = *m_allDataGroups[i];

			if (AlphaBit64(dataGroup.GetId()) & currentActiveDataGroupMask)
			{
				datagroups += dataGroup.GetId();
			}
		}

		m_logFilename += datagroups;
	}

	if (m_pStatoscopeFilenameUseTimeCVar->GetIVal() > 0)
	{
		time_t curTime;
		time(&curTime);
		const struct tm* lt = localtime(&curTime);

		char name[MAX_PATH];
		strftime(name, DRX_ARRAY_COUNT(name), "_%Y%m%d_%H%M%S", lt);

		m_logFilename += name;
	}

	//ensure unique log name
	if (m_pStatoscopeCreateLogFilePerLevelCVar->GetIVal())
	{
		char logNumBuf[10];
		drx_sprintf(logNumBuf, "_%u", m_logNum++);
		m_logFilename += logNumBuf;
	}

	if (m_pStatoscopeLogDestinationCVar->GetIVal() == eLD_File)
		SAFE_DELETE(m_pDataWriter);

	m_logFilename += ".bin";
}

void CStatoscope::SetDataGroups(uint64 enabledDGs, uint64 disabledDGs)
{
	for (u32 i = 0, ic = m_allDataGroups.size(); i < ic; i++)
	{
		CStatoscopeDataGroup& dataGroup = *m_allDataGroups[i];

		if (AlphaBit64(dataGroup.GetId()) & disabledDGs)
		{
			dataGroup.GetCallback()->Disable();
			m_activeDataGroups.erase(std::find(m_activeDataGroups.begin(), m_activeDataGroups.end(), &dataGroup));
		}

		if (AlphaBit64(dataGroup.GetId()) & enabledDGs)
		{
			m_activeDataGroups.push_back(&dataGroup);
			dataGroup.GetCallback()->Enable();
		}
	}
}

void CStatoscope::OutputLoadedModuleInformation(CDataWriter* pDataWriter)
{
	pDataWriter->WriteData(0);
	pDataWriter->m_bHaveOutputModuleInformation = true;
}

void CStatoscope::StoreCallstack(tukk tag, uk * callstackAddresses, u32 callstackLength)
{
	if (m_pCallstacks && m_pCallstacks->IsEnabled())
	{
		DrxMT::vector<SCallstack>::AutoLock lock(m_pCallstacks->m_callstacks.get_lock());
		SCallstack callstack(callstackAddresses, callstackLength, tag);
		m_pCallstacks->m_callstacks.push_back(SCallstack());
		m_pCallstacks->m_callstacks.back().swap(callstack);
	}
}

void CStatoscope::AddUserMarker(tukk path, tukk name)
{
	if (!IsRunning())
		return;

	if (m_pUserMarkers && m_pUserMarkers->IsEnabled())
	{
		m_pUserMarkers->m_userMarkers.push_back(SUserMarker(path, name));
	}
}

void CStatoscope::AddUserMarkerFmt(tukk path, tukk fmt, ...)
{
	if (!IsRunning())
		return;

	if (m_pUserMarkers && m_pUserMarkers->IsEnabled())
	{
		char msg[1024];
		va_list args;
		va_start(args, fmt);
		drx_vsprintf(msg, fmt, args);
		va_end(args);

		m_pUserMarkers->m_userMarkers.push_back(SUserMarker(path, msg));
	}
}

//LogCallstack("USER MARKER");
void CStatoscope::LogCallstack(tukk tag)
{
	if (!IsRunning())
	{
		return;
	}

	u32 callstackLength = 128;
	uk callstackAddresses[128];

	CSystem::debug_GetCallStackRaw(callstackAddresses, callstackLength);
	StoreCallstack(tag, callstackAddresses, callstackLength);
}

void CStatoscope::LogCallstackFormat(tukk tagFormat, ...)
{
	if (!IsRunning())
	{
		return;
	}

	va_list args;
	va_start(args, tagFormat);
	char tag[256];
	drx_vsprintf(tag, tagFormat, args);
	va_end(args);

	u32 callstackLength = 128;
	uk callstackAddresses[128];

	CSystem::debug_GetCallStackRaw(callstackAddresses, callstackLength);
	StoreCallstack(tag, callstackAddresses, callstackLength);
}

void CStatoscope::ConsoleAddUserMarker(IConsoleCmdArgs* pParams)
{
	if (pParams->GetArgCount() == 3)
	{
		gEnv->pStatoscope->AddUserMarker(pParams->GetArg(1), pParams->GetArg(2));
	}
	else
	{
		DrxLogAlways("Invalid use of e_StatoscopeAddUserMarker. Expecting 2 arguments, not %d.\n", pParams->GetArgCount() - 1);
	}
}

void CStatoscope::OnLogDestinationCVarChange(ICVar* pVar)
{
	CStatoscope* pStatoscope = (CStatoscope*)gEnv->pStatoscope;
	SAFE_DELETE(pStatoscope->m_pDataWriter);
	pStatoscope->m_pServer->CloseConnection();
}

void CStatoscope::OnTagCVarChange(ICVar* pVar)
{
	CStatoscope* pStatoscope = (CStatoscope*)gEnv->pStatoscope;
	if (pStatoscope->m_pDataWriter)
	{
		pStatoscope->m_pDataWriter->Close();
	}
	SAFE_DELETE(pStatoscope->m_pDataWriter);
	pStatoscope->m_pServer->CloseConnection();

	pStatoscope->SetLogFilename();
}

bool CStatoscope::IsLoggingForTelemetry()
{
	return m_pStatoscopeLogDestinationCVar->GetIVal() == eLD_Telemetry;
}

void CStatoscope::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_PRECACHE_END:
		{
			if (!m_bLevelStarted)
			{
				if (!m_pGameRulesCVar)
				{
					m_pGameRulesCVar = gEnv->pConsole->GetCVar("sv_gamerules");
				}

				tukk mapName = NULL;

				if (gEnv->pGameFramework)
				{
					mapName = gEnv->pGameFramework->GetLevelName();
				}

				if (!mapName)
				{
					mapName = "unknown_map";
				}
				else
				{
					m_currentMap = mapName;
				}

				string userMarker = "Start ";
				userMarker += mapName;

				if (m_pGameRulesCVar)
				{
					userMarker += " ";
					userMarker += m_pGameRulesCVar->GetString();
				}

				AddUserMarker("Level", userMarker.c_str());

				if (m_pStatoscopeCreateLogFilePerLevelCVar->GetIVal())
				{
					//force new log file
					SetLogFilename();
				}

				m_bLevelStarted = true;
			}
		}
		break;
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			AddUserMarker("Level", "End");
			m_bLevelStarted = false;

			if (m_pDataWriter)
				AddFrameRecord(false);
		}
		break;
	}
}

//Screenshot capturing
bool CStatoscope::OnNeedFrameData(u8*& pConvertedTextureBuf)
{
	//The renderer will only perform the screen grab if we supply a pointer from this callback. Since we only want one intermittently return null most of the time
	if (m_ScreenShotState == eSSCS_AwaitingBufferRequest || m_ScreenShotState == eSSCS_AwaitingCapture)
	{
		m_ScreenShotState = eSSCS_AwaitingCapture;
		pConvertedTextureBuf = m_pScreenShotBuffer;
		return true;
	}
	return false;
}

void CStatoscope::OnFrameCaptured()
{
	//The renderer has finished copying the screenshot into the buffer. Change state so we write the shot out.
	m_ScreenShotState = eSSCS_DataReceived;
}

i32 CStatoscope::OnGetFrameWidth(void)
{
	return m_lastScreenWidth / SCREENSHOT_SCALING_FACTOR;
}

i32 CStatoscope::OnGetFrameHeight(void)
{
	return m_lastScreenHeight / SCREENSHOT_SCALING_FACTOR;
}

i32 CStatoscope::OnCaptureFrameBegin(i32* pTexHandle)
{
	//Called at the beginning of the rendering pass, the flags returned determine if the screenshot render target gets written to.
	//For performance reasons we do this infrequently.
	i32 flags = 0;
	if (m_ScreenShotState == eSSCS_RequestCapture)
	{
		flags |= eCFF_CaptureThisFrame;
		m_ScreenShotState = eSSCS_AwaitingBufferRequest;  //Frame initialised. Wait for the buffer to be requested
	}

	return flags;
}

void CStatoscope::SetupFPSCaptureCVars()
{
	if (m_pStatoscopeAllowFPSOverrideCVar && m_pStatoscopeAllowFPSOverrideCVar->GetIVal() != 0)
	{
		if (m_pStatoscopeDataGroupsCVar)
		{
			m_pStatoscopeDataGroupsCVar->Set("fgmut");
		}
		if (m_pStatoscopeFilenameUseBuildInfoCVar)
		{
			m_pStatoscopeFilenameUseBuildInfoCVar->Set("0");
		}
		if (m_pStatoscopeFilenameUseMapCVar)
		{
			m_pStatoscopeFilenameUseMapCVar->Set("1");
		}
		if (m_pStatoscopeCreateLogFilePerLevelCVar)
		{
			m_pStatoscopeCreateLogFilePerLevelCVar->Set("1");
		}

		ICVar* pCVar = gEnv->pConsole->GetCVar("e_StatoscopeScreenCapWhenGPULimited");
		if (pCVar)
		{
			pCVar->Set(1);
		}
	}
}

bool CStatoscope::RequestScreenShot()
{
	if (m_ScreenShotState == eSSCS_Idle)
	{
		m_ScreenShotState = eSSCS_RequestCapture;
		PrepareScreenShot();
		return true;
	}

	return false;
}

void CStatoscope::RegisterBuiltInDataGroups()
{
	m_pParticleProfilers = new SParticleProfilersDG();
	m_pPhysEntityProfilers = new SPhysEntityProfilersDG();
	m_pFrameProfilers = new SFrameProfilersDG();
	m_pUserMarkers = new SUserMarkerDG();
	m_pCallstacks = new SCallstacksDG();

	RegisterDataGroup(new SFrameLengthDG());
	RegisterDataGroup(new SMemoryDG());
	RegisterDataGroup(new SStreamingDG());
	RegisterDataGroup(new SStreamingAudioDG());
	RegisterDataGroup(new SStreamingObjectsDG());
	RegisterDataGroup(new SThreadsDG());
	RegisterDataGroup(new SSysThreadsDG());
	#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)
	RegisterDataGroup(new CWorkerInfoIndividualDG());
	RegisterDataGroup(new SWorkerInfoSummarizedDG());
	RegisterDataGroup(new CJobsInfoIndividualDG());
	RegisterDataGroup(new SJobsInfoSummarizedDG());
	#endif
	RegisterDataGroup(new SCPUTimesDG());
	RegisterDataGroup(new SVertexCostDG());
	RegisterDataGroup(new SParticlesDG);
	RegisterDataGroup(new SWavicleDG);
	RegisterDataGroup(new SLocationDG());
	RegisterDataGroup(new SPerCGFGPUProfilersDG());
	RegisterDataGroup(m_pParticleProfilers);
	RegisterDataGroup(m_pPhysEntityProfilers);
	RegisterDataGroup(m_pFrameProfilers);
	RegisterDataGroup(new SPerfCountersDG());
	RegisterDataGroup(m_pUserMarkers);
	RegisterDataGroup(m_pCallstacks);
	RegisterDataGroup(new SNetworkDG());
	RegisterDataGroup(new SChannelDG());
	RegisterDataGroup(new SNetworkProfileDG());
}

void CStatoscope::RegisterBuiltInIvDataGroups()
{
	m_intervalGroups.push_back(new CStatoscopeStreamingIntervalGroup());
	m_intervalGroups.push_back(new CStreamingObjectIntervalGroup());
	m_intervalGroups.push_back(new CStatoscopeTextureStreamingIntervalGroup());
}

void CStatoscope::CreateDataWriter()
{
	if (m_pDataWriter == NULL)
	{
		if (m_pStatoscopeLogDestinationCVar->GetIVal() == eLD_File)
		{
			if (m_logFilename.empty())
				SetLogFilename();

			m_pDataWriter = new CFileDataWriter(m_logFilename, m_pStatoscopeWriteTimeout->GetFVal() * 10);
		}
		else if (m_pStatoscopeLogDestinationCVar->GetIVal() == eLD_Socket)
		{
			m_pDataWriter = new CSocketDataWriter(m_pServer, m_pStatoscopeWriteTimeout->GetFVal() * 10);
		}
		else if (m_pStatoscopeLogDestinationCVar->GetIVal() == eLD_Telemetry)
		{
			return;
		}

		assert(m_pDataWriter);
	}
}

void CStatoscope::WriteIntervalClassEvents()
{
	for (IntervalGroupVec::iterator it = m_intervalGroups.begin(), itEnd = m_intervalGroups.end(); it != itEnd; ++it)
	{
		size_t descLength = (*it)->GetDescEventLength();

		StatoscopeDataWriter::EventDefineClass* pEv = m_eventWriter.BeginEvent<StatoscopeDataWriter::EventDefineClass>(descLength);

		pEv->classId = (*it)->GetId();
		pEv->numElements = (*it)->GetNumElements();
		(*it)->WriteDescEvent(pEv + 1);

		m_eventWriter.EndEvent();
	}
}

CStatoscopeServer::CStatoscopeServer(CStatoscope* pStatoscope)
	: m_socket(DRX_INVALID_SOCKET),
	m_isConnected(false),
	m_pStatoscope(pStatoscope)
{
	//m_DataThread.SetServer(this);
}

void CStatoscopeServer::StartListening()
{
	CloseConnection();

	assert(m_socket == DRX_INVALID_SOCKET);

	m_socket = DrxSock::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == DRX_INVALID_SOCKET)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "CStatoscopeServer() - failed to create a valid socket");
		return;
	}

	i32 err = 0;
	i32 statoscopeBasePort = 29527;
	do
	{
		DRXSOCKADDR_IN sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = INADDR_ANY;
		sa.sin_port = htons(statoscopeBasePort++);
		err = DrxSock::bind(m_socket, (const DRXSOCKADDR*)&sa, sizeof(sa));
	}
	while (DrxSock::TranslateSocketError(err) == DrxSock::eCSE_EADDRINUSE);
	if (CheckError(err, "CStatoscopeServer() - failed to bind the socket"))
	{
		return;
	}

	if (CheckError(DrxSock::listen(m_socket, 1), "CStatoscopeServer() - failed to set the socket to listen"))
	{
		return;
	}

	DRXSOCKADDR_IN saIn;
	DRXSOCKLEN_T slen = sizeof(saIn);
	i32 result = DrxSock::getsockname(m_socket, (DRXSOCKADDR*)&saIn, &slen);
	if (DrxSock::TranslateSocketError(result) == DrxSock::eCSE_NO_ERROR)
	{
		DrxLog("Statoscope server listening on port: %u\n", ntohs(saIn.sin_port));
	}

	SetBlockingState(false);
	m_pStatoscope->m_pDataWriter->ResetForNewLog();
}

void CStatoscopeServer::CheckForConnection()
{
	if (m_isConnected)
		return;

	if (m_socket == DRX_INVALID_SOCKET)
		StartListening();

	if (m_socket == DRX_INVALID_SOCKET)
		return;

	DRXSOCKADDR sa;
	DRXSOCKLEN_T addrLen = sizeof(sa);
	DRXSOCKET newSocket = DrxSock::accept(m_socket, &sa, &addrLen);
	const DrxSock::eDrxSockError errCode = DrxSock::TranslateInvalidSocket(newSocket);

	if (errCode != DrxSock::eCSE_NO_ERROR)
	{
		// this error reflects the absence of a pending connection request
		if (errCode != DrxSock::eCSE_EWOULDBLOCK)
		{
			CheckError(newSocket ? static_cast<i32>(newSocket) : 0, "CStatoscopeServer::CheckForConnection() - invalid socket from accept()");
		}
		return;
	}

	if (CheckError(DrxSock::closesocket(m_socket), "CStatoscopeServer::CheckForConnection() - failed to close the listening socket"))
	{
		return;
	}

	m_socket = newSocket;
	SetBlockingState(true);
	m_isConnected = true;

	DRXSOCKADDR_IN saIn;
	memcpy(&saIn, &sa, sizeof(saIn));
	u32 addr = ntohl(saIn.sin_addr.s_addr);
	DrxLog("CStatoscopeServer connected to: %u.%u.%u.%u:%u\n", (u8)(addr >> 24), (u8)(addr >> 16), (u8)(addr >> 8), (u8)(addr >> 0), ntohs(saIn.sin_port));
}

void CStatoscopeServer::CloseConnection()
{
	if (m_socket == DRX_INVALID_SOCKET)
		return;

	SetBlockingState(false);

	if (DrxSock::shutdown(m_socket, SD_SEND) < 0)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "CStatoscopeServer::CloseConnection() - shutdown failed");

	i32 ret;

	do
	{
		char buffer[256];
		ret = DrxSock::recv(m_socket, buffer, sizeof(buffer), 0);
	}
	while (ret > 0);

	if (DrxSock::closesocket(m_socket) < 0)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "CStatoscopeServer::CloseConnection() - closesocket failed");

	m_socket = DRX_INVALID_SOCKET;
	m_isConnected = false;
}

void CStatoscopeServer::SetBlockingState(bool block)
{
	bool succeeded = block ? DrxSock::MakeSocketBlocking(m_socket) : DrxSock::MakeSocketNonBlocking(m_socket);
	if (!succeeded)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "CStatoscopeServer::SetBlockingState() failed");
}

i32 CStatoscopeServer::ReceiveData(uk buffer, i32 bufferSize)
{
	if (m_socket == DRX_INVALID_SOCKET || !m_isConnected)
	{
		return 0;
	}

	i32 ret = DrxSock::recv(m_socket, static_cast<tuk>(buffer), bufferSize, 0);

	if (CheckError(ret, "CStatoscopeServer::ReceiveData()"))
	{
		return 0;
	}

	return ret;
}

void CStatoscopeServer::SendData(tukk buffer, i32 bufferSize)
{
	threadID threadID = DrxGetCurrentThreadId();

	if (m_socket == DRX_INVALID_SOCKET || !m_isConnected)
	{
		return;
	}

	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	//float startTime = gEnv->pTimer->GetAsyncCurTime();
	//i32 origBufferSize = bufferSize;
	//tukk origBuffer = buffer;

	while (bufferSize > 0)
	{
		i32 ret = DrxSock::send(m_socket, buffer, bufferSize, 0);

		if (CheckError(ret, "CStatoscopeServer::SendData()"))
		{
			return;
		}

		buffer += ret;
		bufferSize -= ret;
	}

	//float endTime = gEnv->pTimer->GetAsyncCurTime();
	//printf("Statoscope Send Data 0x%p size: %d time: %f time taken %f\n", origBuffer, origBufferSize, endTime, endTime - startTime);
}

bool CStatoscopeServer::CheckError(i32 err, tukk tag)
{
	if (err < 0)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "%s error: %d", tag, err);
		CloseConnection();
		return true;
	}

	return false;
}

CDataWriter::CDataWriter(bool bUseStringPool, float writeTimeout)
{
	m_bUseStringPool = bUseStringPool;
	m_bTimedOut = false;
	m_writeTimeout = writeTimeout;

	m_buffer.resize(128 * 1024);
	m_formatBuffer.resize(64 * 1024);

	m_pWritePtr = &m_buffer[0];
	m_pFlushStartPtr = &m_buffer[0];

	ResetForNewLog();
}

void CDataWriter::Close()
{
	Flush();
}

void CDataWriter::Flush()
{
	m_DataThread.QueueSendData(m_pFlushStartPtr, (i32)(m_pWritePtr - m_pFlushStartPtr));
	m_pFlushStartPtr = m_pWritePtr;

	m_DataThread.Flush();
}

void CDataWriter::ResetForNewLog()
{
	m_pFlushStartPtr = m_pWritePtr;
	m_DataThread.Clear();
	m_DataThread.Flush();
	m_bShouldOutputLogTopHeader = true;
	m_bHaveOutputModuleInformation = false;
	m_GlobalStringPoolHashes.clear();
	m_bTimedOut = false;
}

void CDataWriter::FlushIOThread()
{
	printf("[Statoscope]Flush Data Thread\n");
	m_DataThread.Flush();
	m_pWritePtr = &m_buffer[0];
	m_pFlushStartPtr = &m_buffer[0];
}

//align write pointer to 4byte
void CDataWriter::Pad4()
{
	i32 pad = ((INT_PTR)m_pWritePtr) & 3;

	if (pad)
	{
		char pBuf[4] = { 0 };
		WriteData(pBuf, pad);
	}
}

void CDataWriter::WriteData(ukk vpData, i32 vsize)
{
	if (m_bTimedOut)
	{
		return;
	}
	tukk pData = (tukk)vpData;
	i32k bufferSize = (i32)m_buffer.size() - 1;
	while (vsize > 0)
	{
		i32k size = std::min(bufferSize, vsize);

		i32 capacity = (i32)(&m_buffer[bufferSize] - m_pWritePtr);

		bool bWrapBuffer = size > capacity;

		//if we are wrapping the buffer, send the remaining jobs
		if (bWrapBuffer)
		{
			m_DataThread.QueueSendData(m_pFlushStartPtr, (i32)(m_pWritePtr - m_pFlushStartPtr));

			//reset write pointer
			m_pWritePtr = &m_buffer[0];
			m_pFlushStartPtr = &m_buffer[0];
		}

		tuk pWriteStart = m_pWritePtr;
		tuk pWriteEnd = pWriteStart + size;

		// Stall until the read thread clears the buffer we need to use
		CTimeValue startTime = gEnv->pTimer->GetAsyncTime();
		do
		{
			tukk pReadBoundsStart, * pReadBoundsEnd;
			u32 numBytesInQueue = m_DataThread.GetReadBounds(pReadBoundsStart, pReadBoundsEnd);

			if (numBytesInQueue == 0)
				break;

			// if these are the same, there's no room in the buffer
			if (pReadBoundsStart != pReadBoundsEnd)
			{
				if (pReadBoundsStart <= pReadBoundsEnd)
				{
					// Simple case, just the one in use section
					if (pWriteEnd <= pReadBoundsStart || pWriteStart >= pReadBoundsEnd)
						break;
				}
				else
				{
					// Two in use sections, wrapping at the boundaries
					if ((pWriteEnd <= pReadBoundsStart) && (pWriteStart >= pReadBoundsEnd))
						break;
				}
			}
			CTimeValue currentTime = gEnv->pTimer->GetAsyncTime();
			if (m_writeTimeout != 0.0f && currentTime.GetDifferenceInSeconds(startTime) > m_writeTimeout)
			{
				DrxLog("CDataWriter write timeout exceeded: %f", currentTime.GetDifferenceInSeconds(startTime));
				m_bTimedOut = true;
				return;
			}

			DrxSleep(1);
		}
		while (true);

		memcpy((uk )pWriteStart, pData, size);
		m_pWritePtr = pWriteEnd;

		if ((m_pWritePtr - m_pFlushStartPtr) >= FlushLength)
		{
			m_DataThread.QueueSendData(m_pFlushStartPtr, (i32)(m_pWritePtr - m_pFlushStartPtr));
			m_pFlushStartPtr = m_pWritePtr;
		}

		pData += size;
		vsize -= size;
	}
}
CFileDataWriter::CFileDataWriter(const string& fileName, float writeTimeout)
	: CDataWriter(true, writeTimeout)
{
	m_fileName = fileName;
	m_pFile = NULL;
	m_bAppend = false;
	m_DataThread.SetDataWriter(this);
}

CFileDataWriter::~CFileDataWriter()
{
	Close();
}

bool CFileDataWriter::Open()
{
	if (!m_pFile)
	{
		SCOPED_ALLOW_FILE_ACCESS_FROM_THIS_THREAD();

		tukk modeStr = m_bAppend ? "ab" : "wb";
		m_bAppend = true;

		m_pFile = fxopen(m_fileName.c_str(), modeStr);
	}

	return m_pFile != NULL;
}

void CFileDataWriter::Close()
{
	if (m_pFile != NULL)
	{
		CDataWriter::Close();
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

void CFileDataWriter::Flush()
{
	if (m_pFile != NULL)
	{
		CDataWriter::Flush();
		fflush(m_pFile);
	}
}

void CFileDataWriter::SendData(tukk pBuffer, i32 nBytes)
{
	if (m_pFile)
	{
		fwrite(pBuffer, nBytes, 1, m_pFile);

		//float endTime = gEnv->pTimer->GetAsyncCurTime();
		//printf("Statoscope Write Data 0x%p size: %d time: %f time taken %f\n", pBuffer, nBytes, endTime, endTime - startTime);
	}
	else
	{
		DrxFatalError("Statoscope file not open");
	}
}

CSocketDataWriter::CSocketDataWriter(CStatoscopeServer* pStatoscopeServer, float writeTimeout)
	: CDataWriter(true, writeTimeout)
{
	m_pStatoscopeServer = pStatoscopeServer;
	m_DataThread.SetDataWriter(this);
}

bool CSocketDataWriter::Open()
{
	m_pStatoscopeServer->CheckForConnection();
	return m_pStatoscopeServer->IsConnected();
}

void CSocketDataWriter::SendData(tukk pBuffer, i32 nBytes)
{
	m_pStatoscopeServer->SendData(pBuffer, nBytes);
}

CTelemetryDataWriter::CTelemetryDataWriter(tukk postHeader, tukk hostname, i32 port, float writeTimeout, float connectTimeout)
	: CDataWriter(true, writeTimeout)
	, m_socket(-1)
	, m_hasSentHeader(false)
	, m_socketErrorTriggered(false)
	, m_connectTimeout(connectTimeout)
{
	m_postHeader = postHeader;
	m_hostname = hostname;
	m_port = port;
	m_DataThread.SetDataWriter(this);
}

bool CTelemetryDataWriter::Open()
{
	if (m_socket == DRX_INVALID_SOCKET && !m_socketErrorTriggered)
	{
		DRXSOCKET s = DrxSock::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == DRX_INVALID_SOCKET)
		{
			DrxLog("CTelemetryDataWriter failed to create a valid socket");
			m_socketErrorTriggered = true;
			return false;
		}
		if (!DrxSock::MakeSocketNonBlocking(s))
		{
			DrxLog("CTelemetryDataWriter failed to make socket non-blocking");
			m_socketErrorTriggered = true;
			return false;
		}
		DRXSOCKADDR_IN sa;
		sa.sin_family = AF_INET;
		sa.sin_port = htons(m_port);
		sa.sin_addr.s_addr = DrxSock::DNSLookup(m_hostname.c_str());
		if (sa.sin_addr.s_addr)
		{
			m_socket = s;
			i32 ret = DrxSock::connect(m_socket, (const DRXSOCKADDR*)&sa, sizeof(sa));
			DrxSock::eDrxSockError err = DrxSock::TranslateSocketError(ret);
			if (err != DrxSock::eCSE_EWOULDBLOCK_CONN && err != DrxSock::eCSE_EINPROGRESS)
			{
				CheckSocketError(err, "Connect to telemetry server");
			}
		}
		else
		{
			DrxLog("CTelemetryDataWriter failed to resolve host name '%s'", m_hostname.c_str());
			m_socketErrorTriggered = true;
		}
	}
	return (m_socket >= 0);
}

void CTelemetryDataWriter::Close()
{
	CDataWriter::Close();
	if (m_socket >= 0)
	{
		DRXSOCKET sock = m_socket;
		SendToSocket("0\r\n\r\n", 5, "End HTTP chunked data stream");
		CheckSocketError(DrxSock::TranslateSocketError(DrxSock::shutdown(sock, SD_SEND)), "Shutdown socket for sending");
		while (!m_bTimedOut)
		{
			i32k recvBufferSize = 1024;
			char recvBuffer[recvBufferSize];
			i32 result = DrxSock::recv(sock, recvBuffer, recvBufferSize, 0);
			CheckSocketError(DrxSock::TranslateSocketError(result), "Wait for response");
			if (result <= 0)
			{
				break;
			}
			// TODO: Check recvBuffer contains OK message
		}
		CheckSocketError(DrxSock::TranslateSocketError(DrxSock::closesocket(sock)), "Close socket");
		m_socket = -1;
	}
}

void CTelemetryDataWriter::SendData(tukk pBuffer, i32 nBytes)
{
	if (m_socket >= 0)
	{
		if (!m_hasSentHeader)
		{
			// Wait until the connection is fully established
			DRXTIMEVAL timeout;
			timeout.tv_sec = (long)(m_connectTimeout);
			timeout.tv_usec = (long)((m_connectTimeout - timeout.tv_sec) * 1e6);

			i32 result = DrxSock::WaitForWritableSocket(m_socket, &timeout);

			if (result != 1)
			{
				if (result == 0)
				{
					CheckSocketError(DrxSock::eCSE_ETIMEDOUT, "Waiting for connection to be fully established");
				}
				else
				{
					CheckSocketError(DrxSock::TranslateSocketError(result), "Waiting for connection to be fully established");
				}
			}

			SendToSocket(m_postHeader.c_str(), m_postHeader.size(), "Begin HTTP chunked data stream");
			m_hasSentHeader = true;
		}
		char chunkHeaderBuffer[16];
		drx_sprintf(chunkHeaderBuffer, "%x\r\n", nBytes);
		SendToSocket(chunkHeaderBuffer, strlen(chunkHeaderBuffer), "Write HTTP chunk size");
		SendToSocket(pBuffer, nBytes, "Write HTTP chunk data");
		SendToSocket("\r\n", 2, "Terminate HTTP chunk data");
	}
}

void CTelemetryDataWriter::CheckSocketError(DrxSock::eDrxSockError sockErr, tukk description)
{
	if (sockErr != DrxSock::eCSE_NO_ERROR)
	{
		DrxLog("CTelemetryDataWriter socket error '%s' - '%d'", description, sockErr);
		if (m_socket >= 0)
		{
			DrxSock::closesocket(m_socket);
			m_socket = -1;
		}
		m_socketErrorTriggered = true;
	}
}

void CTelemetryDataWriter::SendToSocket(tukk pData, size_t nSize, tukk sDescription)
{
	if (m_socket >= 0)
	{
		size_t nSent = 0;
		while (!m_bTimedOut)
		{
			i32 ret = DrxSock::send(m_socket, pData + nSent, nSize - nSent, 0);
			DrxSock::eDrxSockError sockErr = DrxSock::TranslateSocketError(ret);
			if (sockErr != DrxSock::eCSE_NO_ERROR)
			{
				if (sockErr != DrxSock::eCSE_EWOULDBLOCK)
				{
					CheckSocketError(sockErr, sDescription);
					return;
				}
			}
			else
			{
				nSent += ret;
				if (nSent >= nSize)
				{
					return;
				}
			}
			DrxSleep(1);
		}
	}
}

CStatoscopeIOThread::CStatoscopeIOThread()
{
	m_pDataWriter = NULL;
	m_threadID = THREADID_NULL;
	m_numBytesInQueue = 0;
	m_bRun = true;

	if (!gEnv->pThreadUpr->SpawnThread(this, "StatoscopeDataWriter"))
	{
		DrxFatalError("Error spawning \"StatoscopeDataWriter\" thread.");
	}
}

CStatoscopeIOThread::~CStatoscopeIOThread()
{
	//ensure all data is sent
	Flush();

	// Stop thread task.
	m_bRun = false;
	gEnv->pThreadUpr->JoinThread(this, eJM_Join);
}

void CStatoscopeIOThread::Flush()
{
	CTimeValue startTime = gEnv->pTimer->GetAsyncTime();
	while (m_sendJobs.size())
	{
		DrxSleep(1);
		CTimeValue currentTime = gEnv->pTimer->GetAsyncTime();
		float timeout = m_pDataWriter->GetWriteTimeout();
		if (timeout != 0.0f && currentTime.GetDifferenceInSeconds(startTime) > timeout)
		{
			// This should cause the data writer to abort attempting to write data and clear the send jobs queue
			DrxLog("CDataWriter write timeout exceeded during flush: %f", currentTime.GetDifferenceInSeconds(startTime));
			m_pDataWriter->TimeOut();
		}
	}
}

void CStatoscopeIOThread::ThreadEntry()
{
	while (m_bRun)
	{
		if (m_sendJobs.size() > 0)
		{
			SendJob job;
			{
				DrxMT::queue<SendJob>::AutoLock lock(m_sendJobs.get_lock());
				job = m_sendJobs.front();
			}

			m_pDataWriter->SendData(job.pBuffer, job.nBytes);

			{
				DrxMT::queue<SendJob>::AutoLock lock(m_sendJobs.get_lock());
				SendJob j;
				m_sendJobs.try_pop(j);
				m_numBytesInQueue -= j.nBytes;
			}

			//PIXEndNamedEvent();
		}
		else
		{
			DrxSleep(1);
		}
	}
}

void CStatoscopeIOThread::QueueSendData(tukk pBuffer, i32 nBytes)
{
	if (nBytes > 0)
	{
		bool bWait = false;

		//PIXSetMarker(0, "[STATOSCOPE]Queue Data\n");

		assert(pBuffer);
		assert(nBytes > 0);

		SendJob newJob = { pBuffer, nBytes };

		{
			DrxMT::queue<SendJob>::AutoLock lock(m_sendJobs.get_lock());
			m_sendJobs.push(newJob);
			m_numBytesInQueue += newJob.nBytes;
		}

		//printf("Statoscope Queue Data 0x%p size %d at time: %f\n", pBuffer, nBytes, gEnv->pTimer->GetAsyncCurTime());
	}
	else if (nBytes < 0)
	{
		DrxFatalError("Borked!");
	}
}

	#undef EWOULDBLOCK
	#undef GetLastError

#endif // ENABLE_STATOSCOPE
