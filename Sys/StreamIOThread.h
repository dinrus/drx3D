// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StreamIOThread.h
//  Created:     22/07/2010 by Timur.
//  Описание: Streaming Thread for IO
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __StreamingIOThread_h__
#define __StreamingIOThread_h__
#pragma once

#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Sys/StreamAsyncFileRequest.h>

#include <drx3D/CoreX/Thread/IThreadUpr.h>

class CStreamEngine;

//////////////////////////////////////////////////////////////////////////
// Thread that performs IO operations.
//////////////////////////////////////////////////////////////////////////
class CStreamingIOThread : public IThread, public CMultiThreadRefCount
{
public:
	CStreamingIOThread(CStreamEngine* pStreamEngine, EStreamSourceMediaType mediaType, tukk name);
	~CStreamingIOThread();

	void                    CancelAll();
	void                    AbortAll(bool bAbort);

	void                    BeginReset();
	void                    EndReset();

	void                    AddRequest(CAsyncIOFileRequest* pRequest, bool bStartImmidietly);
	i32                     GetRequestCount() const { return m_fileRequestQueue.size(); };
	void                    SortRequests();
	void                    NeedSorting();
	void                    SignalStartWork(bool bForce);
	bool                    HasUrgentRequests();
	EStreamSourceMediaType  GetMediaType() const { return m_eMediaType; }
	bool                    IsMisscheduled(EStreamSourceMediaType mt) const;

	void                    Pause(bool bPause);

	void                    RegisterFallbackIOThread(EStreamSourceMediaType mediaType, CStreamingIOThread* pIOThread);

	CStreamEngineWakeEvent& GetWakeEvent() { return m_awakeEvent; }

	//////////////////////////////////////////////////////////////////////////
	// IThread
	//////////////////////////////////////////////////////////////////////////
	// Start accepting work on thread
	virtual void ThreadEntry();
	//////////////////////////////////////////////////////////////////////////

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

protected:

	void ProcessNewRequests();
	void ProcessReset();

public:

#ifdef STREAMENGINE_ENABLE_STATS
	struct SStats
	{
		SStats() : m_nTotalReadBytes(0), m_nCurrentReadBandwith(0),
			m_nReadBytesInLastSecond(0), m_fReadingDuringLastSecond(.0f),
			m_nTempBytesRead(0), m_nActualReadBandwith(0), m_nTempReadOffset(0),
			m_nTotalReadOffset(0), m_nReadOffsetInLastSecond(0), m_nTempRequestCount(0),
			m_nTotalRequestCount(0), m_nRequestCountInLastSecond(0)
		{}

		void Update(const CTimeValue& deltaT);

		void Reset()
		{
			m_nTotalReadBytes = 0;
			m_nTotalReadOffset = 0;
			m_nTotalRequestCount = 0;
			m_TotalReadTime.SetValue(0);
		}

		float      m_fReadingDuringLastSecond;
		CTimeValue m_TotalReadTime;
		uint64     m_nTotalReadBytes;
		uint64     m_nTotalReadOffset;
		u32     m_nTotalRequestCount;
		u32     m_nCurrentReadBandwith;  // Read bandwidth over one second
		u32     m_nActualReadBandwith;   // Actual read bandwidth extrapolated over one second
		u32     m_nReadBytesInLastSecond;
		u32     m_nRequestCountInLastSecond;
		uint64     m_nReadOffsetInLastSecond;

		u32     m_nTempRequestCount;
		uint64     m_nTempBytesRead;
		uint64     m_nTempReadOffset;
		CTimeValue m_TempReadTime;
	};

	SStats m_InMemoryStats;
	SStats m_NotInMemoryStats;
#endif

	int64 m_nLastReadDiskOffset;

private:
	CStreamEngine*                      m_pStreamEngine;
	std::vector<CAsyncIOFileRequest*>   m_fileRequestQueue;
	std::vector<CAsyncIOFileRequest*>   m_temporaryArray;
	DrxMT::vector<CAsyncIOFileRequest*> m_newFileRequests;

	EStreamSourceMediaType              m_eMediaType;
	u32                              m_nFallbackMTs;

	typedef std::pair<CStreamingIOThread*, EStreamSourceMediaType> TFallbackIOPair;
	typedef std::vector<TFallbackIOPair>                           TFallbackIOVec;
	typedef TFallbackIOVec::iterator                               TFallbackIOVecConstIt;
	TFallbackIOVec         m_FallbackIOThreads;

	 bool          m_bCancelThreadRequest;
	 bool          m_bNeedSorting;
	 bool          m_bNewRequests;
	 bool          m_bPaused;
	 bool          m_bNeedReset;
	 bool          m_bAbortReads;

	 i32           m_iUrgentRequests;

	CStreamEngineWakeEvent m_awakeEvent;
	DrxEvent               m_resetDoneEvent;
	string                 m_name;
	u32                 m_nReadCounter;
};

//////////////////////////////////////////////////////////////////////////
// Thread that performs IO operations.
//////////////////////////////////////////////////////////////////////////
class CStreamingWorkerThread : public IThread, public CMultiThreadRefCount
{
public:
	enum EWorkerType
	{
		eWorkerAsyncCallback,
	};
	CStreamingWorkerThread(CStreamEngine* pStreamEngine, tukk name, EWorkerType type, SStreamRequestQueue* pQueue);
	~CStreamingWorkerThread();

	void BeginReset();
	void EndReset();

	void CancelAll();

	//////////////////////////////////////////////////////////////////////////
	// IThead
	//////////////////////////////////////////////////////////////////////////
	// Start accepting work on thread
	virtual void ThreadEntry();
	//////////////////////////////////////////////////////////////////////////

	// Signals the thread that it should not accept anymore work and exit
	void SignalStopWork();

private:
	EWorkerType          m_type;
	CStreamEngine*       m_pStreamEngine;
	SStreamRequestQueue* m_pQueue;

	 bool        m_bCancelThreadRequest;
	 bool        m_bNeedsReset;

	DrxEvent             m_resetDoneEvent;
	string               m_name;
};

#endif //__StreamingIOThread_h__
