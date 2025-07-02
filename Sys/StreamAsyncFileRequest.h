// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StreamAsyncFileRequest.h
//  Created:     22/07/2010 by Timur.
//  Описание: Streaming Thread for IO
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __StreamAsyncFileRequest_h__
#define __StreamAsyncFileRequest_h__
#pragma once

#include <drx3D/Sys/IStreamEngineDefs.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

class CStreamEngine;
class CAsyncIOFileRequest;
struct z_stream_s;
class CStreamingIOThread;
struct CCachedFileData;
class CDrxFile;
struct SStreamJobEngineState;
class CMTSafeHeap;
class CAsyncIOFileRequest_TransferPtr;
struct SStreamEngineTempMemStats;

#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION  //Could check for INCLUDE_LIBTOMCRYPT here, but only decryption is implemented here, not signing
	#include <drx3D/Plugins/tomcrypt/tomcrypt.h>
	#include <drx3D/CoreX/Assert/DrxAssert.h> // tomcrypt will re-define assert
	#undef byte                           // tomcrypt defines a byte macro which conflicts with out byte data type
#endif

#if !defined(USE_EDGE_ZLIB)
	#include <drx/Core/lib/z/zlib.h>
	#include <drx/Core/lib/z/zutil.h>
	#include <drx/Core/lib/z/inftrees.h>
	#include <drx/Core/lib/z/inflate.h>
//	Undefine macros defined in zutil.h to prevent compilation errors in 'steamclientpublic.h', 'OVR_Math.h' etc.
	#undef Assert
	#undef Trace
	#undef Tracev
	#undef Tracevv
	#undef Tracec
	#undef Tracecv
#endif

namespace ZipDir {
struct UncompressLookahead;
}

struct IAsyncIOFileCallback
{
	virtual ~IAsyncIOFileCallback(){}
	// Asynchronous finished event.
	// Must be thread safe, can be called from a different thread.
	virtual void OnAsyncFinished(CAsyncIOFileRequest* pFileRequest) = 0;
};

struct SStreamPageHdr
{
	explicit SStreamPageHdr(i32 nSize)
		: nRefs()
		, nSize(nSize)
	{}

	 i32 nRefs;
	i32          nSize;
};

struct SStreamJobQueue
{
	enum
	{
		MaxJobs = 256,
	};

	struct Job
	{
		uk           pSrc;
		SStreamPageHdr* pSrcHdr;
		u32          nOffs;
		u32          nBytes : 31;
		u32          bLast  : 1;
	};

	SStreamJobQueue()
		: m_sema(MaxJobs, MaxJobs)
	{
		m_nQueueLen = 0;
		m_nPush = 0;
		m_nPop = 0;
		memset(m_jobs, 0, sizeof(m_jobs));
	}

	void Flush(SStreamEngineTempMemStats& tms);

	i32  Push(uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast);
	i32  Pop();

	DrxFastSemaphore m_sema;
	Job              m_jobs[MaxJobs];
	 i32     m_nQueueLen;
	 i32     m_nPush;
	 i32     m_nPop;
};

// This class represent a request to read some file from disk asynchronously via one of the IO threads.
class CAsyncIOFileRequest
{
public:
	enum EStatus
	{
		eStatusNotReady,
		eStatusInFileQueue,
		eStatusFailed,
		eStatusUnzipComplete,
		eStatusDone,
	};

	enum
	{
		BUFFER_ALIGNMENT = 128,
		WINDOW_SIZE      = 1 << 15,

#if DRX_PLATFORM_ANDROID
		STREAMING_PAGE_SIZE = (128 * 1024),
#else
		STREAMING_PAGE_SIZE = (1 * 1024 * 1024),
#endif

#if DRX_PLATFORM_ANDROID
		STREAMING_BLOCK_SIZE = (64 * 1024),
#else
		STREAMING_BLOCK_SIZE = (32 * 1024),
#endif
	};

public:
	static CAsyncIOFileRequest* Allocate(EStreamTaskType eType);
	static void                 Flush();

public:
	void AddRef();
	i32  Release();

public:
	void       Init(EStreamTaskType eType);
	void       Finalize();

	void       Reset();

	ILINE bool IsCancelled() const { return m_nError == ERROR_USER_ABORT; }
	ILINE bool HasFailed() const   { return m_nError != 0; }
	void       Failed(u32 nError)
	{
		DrxInterlockedCompareExchange(reinterpret_cast< LONG*>(&m_nError), nError, 0);
	}

	u32         OpenFile(CDrxFile& file);

	u32         ReadFile(CStreamingIOThread* pIOThread);
	u32         ReadFileResume(CStreamingIOThread* pIOThread);
	u32         ReadFileInPages(CStreamingIOThread* pIOThread, CDrxFile& file);
	u32         ReadFileCheckPreempt(CStreamingIOThread* pIOThread);

	u32         ConfigureRead(CCachedFileData* pFileData);
	bool           CanReadInPages();
	u32         AllocateOutput(CCachedFileData* pZipEntry);
	u8* AllocatePage(size_t sz, bool bOnlyPakMem, SStreamPageHdr*& pHdrOut);

	static void    JobFinalize_Read(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState);

	u32         PushDecompressPage(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nBytes, bool bLast);
	u32         PushDecompressBlock(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast);
	static void    JobStart_Decompress(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState, i32 nSlot);
	void           DecompressBlockEntry(SStreamJobEngineState engineState, i32 nJob);

#if defined(STREAMENGINE_SUPPORT_DECRYPT)
	u32      PushDecryptPage(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nBytes, bool bLast);
	u32      PushDecryptBlock(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast);
	static void JobStart_Decrypt(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState, i32 nSlot);
	void        DecryptBlockEntry(SStreamJobEngineState engineState, i32 nJob);
#endif //STREAMENGINE_SUPPORT_DECRYPT

	void                   Cancel();
	bool                   TryCancel();
	void                   SyncWithDecrypt();
	void                   SyncWithDecompress();
	void                   ComputeSortKey(uint64 nCurrentKeyInProgress);
	void                   SetPriority(EStreamTaskPriority estp);
	void                   BumpSweep();
	void                   FreeBuffer();

	bool                   IgnoreOutofTmpMem() const;

	CStreamEngine*         GetStreamEngine();

	EStreamSourceMediaType GetMediaType();

private:
	uk operator new(size_t sz);
	void  operator delete(uk p);

private:
	static void JobFinalize_Decompress(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState);
	static void JobFinalize_Decrypt(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState);
	static void JobFinalize_Transfer(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState);

private:
	void JobFinalize_Buffer(const SStreamJobEngineState& engineState);
	void JobFinalize_Validate(const SStreamJobEngineState& engineState);

private:
	CAsyncIOFileRequest();
	~CAsyncIOFileRequest();

public:
	static  i32                    s_nLiveRequests;
	static SLockFreeSingleLinkedListHeader s_freeRequests;

public:
	// Must be first
	SLockFreeSingleLinkedListEntry m_nextFree;

	 i32                   m_nRefCount;

	// Locks to be held whilst the file is being read, and an external memory buffer is in use
	// (to ensure that if cancelled, the stream engine doesn't write to the external buffer)
	// Separate locks for read and decomp as they can overlap (block decompress)
	// Cancel() must acquire both
	DrxCriticalSection m_externalBufferLockRead;
	DrxCriticalSection m_externalBufferLockDecompress;
#if defined(STREAMENGINE_SUPPORT_DECRYPT)
	DrxCriticalSection m_externalBufferLockDecrypt;
#endif  //STREAMENGINE_SUPPORT_DECRYPT

	DrxStringLocal m_strFileName;
	string         m_pakFile;

#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
	string m_decryptionCTRInitialisedAgainst;
#endif  //SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION

	// If request come from stream, it will be not 0.
	IReadStreamPtr m_pReadStream;

#if defined(STREAMENGINE_SUPPORT_DECRYPT)
	JobUpr::SJobState m_DecryptJob;
#endif  //STREAMENGINE_SUPPORT_DECRYPT
	JobUpr::SJobState m_DecompJob;

	// Only POD data should exist beyond this point - will be memsetted to 0 on Reset !

	uint64                 m_nSortKey;

	EStreamTaskPriority    m_ePriority;
	EStreamSourceMediaType m_eMediaType;
	EStreamTaskType        m_eType;

	 EStatus       m_status;
	 u32        m_nError;

	u32                 m_nRequestedOffset;
	u32                 m_nRequestedSize;

	// the file size, or 0 if the file couldn't be opened
	u32       m_nFileSize;
	u32       m_nFileSizeCompressed;

	uk        m_pMemoryBuffer;
	u32       m_nMemoryBufferSize;
	 i32 m_nMemoryBufferUsers;

	uk        m_pExternalMemoryBuffer;
	uk        m_pOutputMemoryBuffer;
	uk        m_pReadMemoryBuffer;
	u32       m_nReadMemoryBufferSize;

	u32       m_bCompressedBuffer  : 1;
	u32       m_bEncryptedBuffer   : 1;
	u32       m_bStatsUpdated      : 1;
	u32       m_bStreamInPlace     : 1;
	u32       m_bWriteOnlyExternal : 1;
	u32       m_bSortKeyComputed   : 1;
	u32       m_bOutputAllocated   : 1;
	u32       m_bReadBegun         : 1;

	// Actual size of the data on the media.
	u32                m_nSizeOnMedia;

	int64                 m_nDiskOffset;
	i32                 m_nReadHeadOffsetKB; // Offset of the Read Head when reading from media.
	i32                 m_nTimeGroup;
	i32                 m_nSweep;

	IAsyncIOFileCallback* m_pCallback;

	//
	//	Block based streaming
	//

	u32                       m_nPageReadStart;
	u32                       m_nPageReadCurrent;
	u32                       m_nPageReadEnd;

	 u32              m_nBytesDecompressed;
	 u32              m_nBytesDecrypted;

	u32                       m_crc32FromHeader;

	 LONG                m_nFinalised;

	z_stream_s*                  m_pZlibStream;
	ZipDir::UncompressLookahead* m_pLookahead;
	SStreamJobQueue*             m_pDecompQueue;
#if defined(STREAMENGINE_SUPPORT_DECRYPT)
	SStreamJobQueue*             m_pDecryptQueue;
#endif  //STREAMENGINE_SUPPORT_DECRYPT
#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
	symmetric_CTR* m_pDecryptionCTR;
#endif  //SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION

#ifdef STREAMENGINE_ENABLE_STATS
	// Time that read operation took.
	CTimeValue m_readTime;
	CTimeValue m_unzipTime;
	CTimeValue m_verifyTime;
	CTimeValue m_decryptTime;
	CTimeValue m_startTime;
	CTimeValue m_completionTime;

	u32     m_nReadCounter;
#endif
};
TYPEDEF_AUTOPTR(CAsyncIOFileRequest);

struct SStreamRequestQueue
{
	DrxCriticalSection                m_lock;
	std::vector<CAsyncIOFileRequest*> m_requests;

	DrxEvent                          m_awakeEvent;

	SStreamRequestQueue();
	~SStreamRequestQueue();

	void Reset();
	bool IsEmpty() const;

	// Transfers ownership (rather than shares ownership) to the queue
	void TransferRequest(CAsyncIOFileRequest_TransferPtr& pReq);
	bool TryPopRequest(CAsyncIOFileRequest_AutoPtr& pOut);

private:
	SStreamRequestQueue(const SStreamRequestQueue&);
	SStreamRequestQueue& operator=(const SStreamRequestQueue&);
};

#if defined(STREAMENGINE_ENABLE_STATS)
struct SStreamEngineDecompressStats
{
	uint64     m_nTotalBytesUnziped;
	uint64     m_nTempBytesUnziped;
	uint64     m_nTotalBytesDecrypted;
	uint64     m_nTempBytesDecrypted;
	uint64     m_nTotalBytesVerified;
	uint64     m_nTempBytesVerified;

	CTimeValue m_totalUnzipTime;
	CTimeValue m_tempUnzipTime;
	CTimeValue m_totalDecryptTime;
	CTimeValue m_tempDecryptTime;
	CTimeValue m_totalVerifyTime;
	CTimeValue m_tempVerifyTime;
};
#endif

class CAsyncIOFileRequest_TransferPtr
{
public:
	explicit CAsyncIOFileRequest_TransferPtr(CAsyncIOFileRequest* p)
		: m_p(p)
	{
	}

	~CAsyncIOFileRequest_TransferPtr()
	{
		if (m_p)
			m_p->Release();
	}

	CAsyncIOFileRequest*       operator->()       { return m_p; }
	CAsyncIOFileRequest&       operator*()        { return *m_p; }

	const CAsyncIOFileRequest* operator->() const { return m_p; }
	const CAsyncIOFileRequest& operator*() const  { return *m_p; }

	operator bool() const { return m_p != NULL; }

	CAsyncIOFileRequest* Relinquish()
	{
		CAsyncIOFileRequest* p = m_p;
		m_p = NULL;
		return p;
	}

	CAsyncIOFileRequest_TransferPtr& operator=(CAsyncIOFileRequest* p)
	{
#ifndef _RELEASE
		if (m_p)
			__debugbreak();
#endif
		m_p = p;
		return *this;
	}

private:
	CAsyncIOFileRequest_TransferPtr(const CAsyncIOFileRequest_TransferPtr&);
	CAsyncIOFileRequest_TransferPtr& operator=(const CAsyncIOFileRequest_TransferPtr&);

private:
	CAsyncIOFileRequest* m_p;
};

class CStreamEngineWakeEvent
{
public:
	CStreamEngineWakeEvent()
		: m_state(0)
	{
	}

	void Set()
	{
		 LONG oldState, newState;
		bool bSignalInner;

		do
		{
			bSignalInner = false;
			oldState = m_state;

			newState = oldState | 0x80000000;
			if (oldState & 0x7fffffff)
			{
				bSignalInner = true;
			}
		}
		while (DrxInterlockedCompareExchange(&m_state, newState, oldState) != oldState);

		if (bSignalInner)
		{
			m_innerEvent.Set();
		}
	}

	bool Wait(u32 timeout = 0)
	{
		bool bTimedOut = false;
		bool bAcquiredSignal = false;

		while (!bTimedOut && !bAcquiredSignal)
		{
			 long oldState, newState;
			do
			{
				bAcquiredSignal = false;

				oldState = m_state;
				if (oldState & 0x80000000)
				{
					// Signalled
					newState = oldState & 0x7fffffff;
					bAcquiredSignal = true;
				}
				else
				{
					newState = oldState + 1;
				}
			}
			while (DrxInterlockedCompareExchange(&m_state, newState, oldState) != oldState);

			if (!bAcquiredSignal)
			{
				if (!timeout)
				{
					m_innerEvent.Wait();
				}
				else
				{
					bTimedOut = !m_innerEvent.Wait(timeout);
				}

				if (!bTimedOut)
				{
					m_innerEvent.Reset();
				}

				do
				{
					bAcquiredSignal = false;

					oldState = m_state;
					if (!bTimedOut && (oldState & 0x80000000))
					{
						newState = (oldState & 0x7fffffff) - 1;
						bAcquiredSignal = true;
					}
					else
					{
						newState = oldState - 1;
					}
				}
				while (DrxInterlockedCompareExchange(&m_state, newState, oldState) != oldState);
			}
		}

		return bAcquiredSignal;
	}

private:
	CStreamEngineWakeEvent(const CStreamEngineWakeEvent&);
	CStreamEngineWakeEvent& operator=(const CStreamEngineWakeEvent&);

private:
	 LONG m_state;
	DrxEvent      m_innerEvent;
};

struct SStreamEngineTempMemStats
{
	enum
	{
		MaxWakeEvents = 8,
	};

	SStreamEngineTempMemStats()
	{
		memset(this, 0, sizeof(*this));
	}

	uk TempAlloc(CMTSafeHeap* pHeap, size_t nSize, tukk szDbgSource, bool bFallBackToMalloc = true, bool bUrgent = false, u32 align = 0);
	void  TempFree(CMTSafeHeap* pHeap, ukk p, size_t nSize);
	void  ReportTempMemAlloc(u32 nSizeAlloc, u32 nSizeFree, bool bTriggerWake);

	 LONG           m_nTempAllocatedMemory;
	 LONG           m_nTempAllocatedMemoryFrameMax;
	i32                     m_nTempMemoryBudget;
	CStreamEngineWakeEvent* m_wakeEvents[MaxWakeEvents];
	i32                     m_nWakeEvents;
};

struct SStreamJobEngineState
{
	std::vector<SStreamRequestQueue*>* pReportQueues;

#if defined(STREAMENGINE_ENABLE_STATS)
	SStreamEngineStatistics*      pStats;
	SStreamEngineDecompressStats* pDecompressStats;
#endif

	SStreamEngineTempMemStats* pTempMem;

	CMTSafeHeap*               pHeap;
};

#endif //StreamAsyncFileRequest
