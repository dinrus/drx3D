// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StreamEngine.h
//  Created:     27/07/2010 by Timur.
//  Описание: Streaming Engine
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _DRX_SYSTEM_STREAM_ENGINE_HDR_
#define _DRX_SYSTEM_STREAM_ENGINE_HDR_

#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/TimeValue.h>

#include <drx3D/Sys/StreamIOThread.h>
#include <drx3D/Sys/StreamReadStream.h>

enum EIOThread
{
	eIOThread_HDD      = 0,
	eIOThread_Optical  = 1,
	eIOThread_InMemory = 2,
	eIOThread_Last     = 3,
};

//////////////////////////////////////////////////////////////////////////
class CStreamEngine : public IStreamEngine, public ISystemEventListener, public IInputEventListener
{
public:
	CStreamEngine();
	~CStreamEngine();

	void Shutdown();
	// This is called to cancel all pending requests, without sending callbacks.
	void CancelAll();

	//////////////////////////////////////////////////////////////////////////
	// IStreamEngine interface
	//////////////////////////////////////////////////////////////////////////
	IReadStreamPtr StartRead(const EStreamTaskType tSource, tukk szFile, IStreamCallback* pCallback, const StreamReadParams* pParams = NULL);
	size_t         StartBatchRead(IReadStreamPtr* pStreamsOut, const StreamReadBatchParams* pReqs, size_t numReqs);
	void           BeginReadGroup();
	void           EndReadGroup();

	bool           IsStreamDataOnHDD() const      { return m_bStreamDataOnHDD; }
	void           SetStreamDataOnHDD(bool bFlag) { m_bStreamDataOnHDD = bFlag; }

	void           Update();
	void           UpdateAndWait(bool bAbortAll = false);
	void           Update(u32 nUpdateTypesBitmask);

	void           GetMemoryStatistics(IDrxSizer* pSizer);

#if defined(STREAMENGINE_ENABLE_STATS)
	SStreamEngineStatistics& GetStreamingStatistics();
	void                     ClearStatistics();

	void                     GetBandwidthStats(EStreamTaskType type, float* bandwidth);
#endif

	void                       GetStreamingOpenStatistics(SStreamEngineOpenStats& openStatsOut);

	tukk                GetStreamTaskTypeName(EStreamTaskType type);

	SStreamJobEngineState      GetJobEngineState();
	SStreamEngineTempMemStats& GetTempMemStats() { return m_tempMem; }

	// Will pause or unpause streaming of specified by mask data types
	void   PauseStreaming(bool bPause, u32 nPauseTypesBitmask);
	// Pause/resumes any IO active from the streaming engine
	void   PauseIO(bool bPause);

	u32 GetPauseMask() const { return m_nPausedDataTypesMask; }

#if defined(STREAMENGINE_ENABLE_LISTENER)
	void                   SetListener(IStreamEngineListener* pListener);
	IStreamEngineListener* GetListener();
#endif

	//////////////////////////////////////////////////////////////////////////

	// updates the job priority of an IO job into the IOQueue while maintaining order in the queue
	void UpdateJobPriority(IReadStreamPtr pJobStream);

	void ReportAsyncFileRequestComplete(CAsyncIOFileRequest_AutoPtr pFileRequest);
	void AbortJob(CReadStream* pStream);

	// Dispatches synchrnous callbacks, free temporary memory hold for callbacks.
	void   MainThread_FinalizeIOJobs();
	void   MainThread_FinalizeIOJobs(u32 type);

	uk  TempAlloc(size_t nSize, tukk szDbgSource, bool bFallBackToMalloc = true, bool bUrgent = false, u32 align = 0);
	void   TempFree(uk p, size_t nSize);

	u32 GetCurrentTempMemorySize() const { return m_tempMem.m_nTempAllocatedMemory; }
	void   FlagTempMemOutOfBudget()
	{
#ifdef STREAMENGINE_ENABLE_STATS
		m_bTempMemOutOfBudget = true;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// IInputEventListener
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnInputEvent(const SInputEvent& event);
	//////////////////////////////////////////////////////////////////////////

	bool StartFileRequest(CAsyncIOFileRequest* pFileRequest);
	void SignalToStartWork(EIOThread e, bool bForce);

private:
	void StartThreads();
	void StopThreads();

	void ResumePausedStreams_PauseLocked();

#if defined(STREAMENGINE_ENABLE_STATS)
	// add job to current statistics
	void UpdateStatistics(CReadStream* pReadStream);
	void DrawStatistics();
#endif

	void PushRequestToAsyncCallbackThread(CAsyncIOFileRequest* pFileRequest);

	//////////////////////////////////////////////////////////////////////////
	// ISystemEventListener
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	//////////////////////////////////////////////////////////////////////////

private:

	//////////////////////////////////////////////////////////////////////////
	DrxMT::set<CReadStream_AutoPtr>    m_streams;
	DrxMT::vector<CReadStream_AutoPtr> m_finishedStreams;
	std::vector<CReadStream_AutoPtr>   m_tempFinishedStreams;

	// 2 IO threads.
	_smart_ptr<CStreamingIOThread>                  m_pThreadIO[eIOThread_Last];
	std::vector<_smart_ptr<CStreamingWorkerThread>> m_asyncCallbackThreads;
	std::vector<SStreamRequestQueue*>               m_asyncCallbackQueues;

	DrxCriticalSection                              m_pausedLock;
	std::vector<CReadStream_AutoPtr>                m_pausedStreams;
	 u32 m_nPausedDataTypesMask;

	bool            m_bStreamDataOnHDD;
	bool            m_bUseOpticalDriveThread;

	//////////////////////////////////////////////////////////////////////////
	// Streaming statistics.
	//////////////////////////////////////////////////////////////////////////

#ifdef STREAMENGINE_ENABLE_LISTENER
	IStreamEngineListener* m_pListener;
#endif

#ifdef STREAMENGINE_ENABLE_STATS
	SStreamEngineStatistics                  m_Statistics;
	SStreamEngineDecompressStats             m_decompressStats;
	CTimeValue                               m_TimeOfLastReset;
	CTimeValue                               m_TimeOfLastUpdate;

	DrxCriticalSection                       m_csStats;
	std::vector<CAsyncIOFileRequest_AutoPtr> m_statsRequestList;

	struct SExtensionInfo
	{
		SExtensionInfo() : m_fTotalReadTime(0.0f), m_nTotalRequests(0), m_nTotalReadSize(0),
			m_nTotalRequestSize(0)
		{
		}
		float  m_fTotalReadTime;
		size_t m_nTotalRequests;
		uint64 m_nTotalReadSize;
		uint64 m_nTotalRequestSize;
	};
	typedef std::map<string, SExtensionInfo> TExtensionInfoMap;
	TExtensionInfoMap m_PerExtensionInfo;

	//////////////////////////////////////////////////////////////////////////
	// Used to calculate unzip/decrypt/verify bandwidth for statistics.
	u32     m_nUnzipBandwidth;
	u32     m_nUnzipBandwidthAverage;
	u32     m_nDecryptBandwidth;
	u32     m_nDecryptBandwidthAverage;
	u32     m_nVerifyBandwidth;
	u32     m_nVerifyBandwidthAverage;
	CTimeValue m_nLastBandwidthUpdateTime;

	bool       m_bStreamingStatsPaused;
	bool       m_bInputCallback;
	bool       m_bTempMemOutOfBudget;
	//////////////////////////////////////////////////////////////////////////
#endif

	SStreamEngineOpenStats m_OpenStatistics;

	bool                   m_bShutDown;

	 i32           m_nBatchMode;

	// Memory currently allocated by streaming engine for temporary storage.
	SStreamEngineTempMemStats m_tempMem;
};

#endif
