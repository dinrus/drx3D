// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

#include <drx3D/Sys/StreamAsyncFileRequest.h>

//#ifndef STREAMENGINE_SUPPORT_DECRYPT
#define STREAMENGINE_SUPPORT_DECRYPT 1
	#include <drx3D/Sys/ZipEncrypt.h>
	#include <drx3D/Sys/StreamEngine.h>
//#endif  //STREAMENGINE_SUPPORT_DECRYPT

#include <drx3D/Sys/MTSafeAllocator.h>

extern void ZlibInflateElementPartial_Impl(
  i32* pReturnCode, z_stream* pZStream, ZipDir::UncompressLookahead* pLookahead,
  u8* pOutput, u64 nOutputLen, bool bOutputWriteOnly,
  u8k* pInput, u64 nInputLen, u64* pTotalOut);

//#pragma("control %push O=0")             // to disable optimization

#ifdef STREAMENGINE_ENABLE_LISTENER
	#include <drx3D/Sys/IStreamEngine.h>
class NotifyListener
{
public:
	NotifyListener(IStreamEngineListener* pL, CAsyncIOFileRequest* pReq) : m_pL(pL), m_pReq(pReq), m_bInProgress(false) {}
	virtual ~NotifyListener() {}
protected:
	IStreamEngineListener* m_pL;
	CAsyncIOFileRequest*   m_pReq;
	bool                   m_bInProgress;
};
class NotifyListenerInflate : NotifyListener
{
public:
	NotifyListenerInflate(IStreamEngineListener* pL, CAsyncIOFileRequest* pReq) : NotifyListener(pL, pReq)
	{
		if (m_pL)
		{
			m_pL->OnStreamBeginInflate(m_pReq);
			m_bInProgress = true;
		}
	}
	~NotifyListenerInflate()
	{
		End();
	}
	void End()
	{
		if (m_bInProgress)
		{
			m_pL->OnStreamEndInflate(m_pReq);
			m_bInProgress = false;
		}
	}
};
class NotifyListenerDecrypt : NotifyListener
{
public:
	NotifyListenerDecrypt(IStreamEngineListener* pL, CAsyncIOFileRequest* pReq) : NotifyListener(pL, pReq)
	{
		if (m_pL)
		{
			m_pL->OnStreamBeginDecrypt(m_pReq);
			m_bInProgress = true;
		}
	}
	~NotifyListenerDecrypt()
	{
		End();
	}
	void End()
	{
		if (m_bInProgress)
		{
			m_pL->OnStreamEndDecrypt(m_pReq);
			m_bInProgress = false;
		}
	}
};
#endif

#if defined(STREAMENGINE_SUPPORT_DECRYPT)
DECLARE_JOB("StreamDecryptBlock", TStreamDecryptBlockJob, CAsyncIOFileRequest::DecryptBlockEntry)
#endif  //STREAMENGINE_SUPPORT_DECRYPT

DECLARE_JOB("StreamInflateBlock", TStreamInflateBlockJob, CAsyncIOFileRequest::DecompressBlockEntry)

//#pragma optimize("",off)

#if defined(STREAMENGINE_ENABLE_STATS)
	#define STREAMENGINE_ENABLE_TIMING
#endif

//#define STREAM_DECOMPRESS_TRACE(...) printf(__VA_ARGS__)
#define STREAM_DECOMPRESS_TRACE(...)

void SStreamJobQueue::Flush(SStreamEngineTempMemStats& tms)
{
	extern CMTSafeHeap* g_pPakHeap;

	for (i32 c = m_nQueueLen, i = m_nPop % MaxJobs; c; --c, i = (i + 1) % MaxJobs)
	{
		Job& j = m_jobs[i];
		if (j.pSrcHdr && DrxInterlockedDecrement(&j.pSrcHdr->nRefs) == 0)
			tms.TempFree(g_pPakHeap, j.pSrc, j.pSrcHdr->nSize);
		j.pSrc = NULL;
	}
}

i32 SStreamJobQueue::Push(uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast)
{
	m_sema.Acquire();

	i32 nSlot = (m_nPush++) % MaxJobs;

	Job& j = m_jobs[nSlot];
	j.pSrc = pSrc;
	j.pSrcHdr = pSrcHdr;
	j.nOffs = nOffs;
	j.nBytes = nBytes;
	j.bLast = (u32)bLast;

	bool bStartNext = DrxInterlockedIncrement(&m_nQueueLen) == 1;
	return bStartNext ? nSlot : -1;
}

i32 SStreamJobQueue::Pop()
{
	i32 nSlot = (++m_nPop) % MaxJobs;
	bool bStartNext = DrxInterlockedDecrement(&m_nQueueLen) > 0;

	m_sema.Release();

	return bStartNext ? nSlot : -1;
}

void CAsyncIOFileRequest::AddRef()
{
	i32 nRef = DrxInterlockedIncrement(&m_nRefCount);
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] AddRef(%x) %p %i %s\n", DrxGetCurrentThreadId(), this, nRef, m_strFileName.c_str());
}

i32 CAsyncIOFileRequest::Release()
{
	i32 nRef = DrxInterlockedDecrement(&m_nRefCount);

	STREAM_DECOMPRESS_TRACE("[StreamDecompress] Release(%x) %p %i %s\n", DrxGetCurrentThreadId(), this, nRef, m_strFileName.c_str());

#ifndef _RELEASE
	if (nRef < 0)
		__debugbreak();
#endif

	if (nRef == 0)
	{
		Finalize();

		DrxInterlockedPushEntrySList(s_freeRequests, m_nextFree);

	}

	return nRef;
}

void CAsyncIOFileRequest::DecompressBlockEntry(SStreamJobEngineState engineState, i32 nJob)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] DecompressBlockEntry(%x) %p %s %i\n", DrxGetCurrentThreadId(), this, m_strFileName.c_str(), nJob);

	CAsyncIOFileRequest_TransferPtr pSelf(this);

	SStreamJobQueue::Job& job = m_pDecompQueue->m_jobs[nJob];

	uk pSrc = job.pSrc;
	SStreamPageHdr* const pSrcHdr = job.pSrcHdr;
	u32k nOffs = job.nOffs;
	u32k nBytes = job.nBytes;
	const bool bLast = job.bLast;
	const bool bFailed = HasFailed();

	if (!bFailed)
	{
#if defined(STREAMENGINE_ENABLE_TIMING)

		LARGE_INTEGER liStart;
		QueryPerformanceCounter(&liStart);

		tukk pFileNameShort = PathUtil::GetFile(m_strFileName.c_str());
		char eventName[128] = { 0 };
		drx_sprintf(eventName, "DcmpBlck %u : %s", m_pZlibStream ? m_pZlibStream->avail_in : 0, pFileNameShort);
		DRX_PROFILE_REGION(PROFILE_SYSTEM, "DcmpBlck");
		DRXPROFILE_SCOPE_PROFILE_MARKER(eventName);
		DRXPROFILE_SCOPE_PLATFORM_MARKER(eventName);
#endif

		//printf("Inflate: %s Avail in: %d, Avail Out: %d, Next In: 0x%p, Next Out: 0x%p\n", m_strFileName.c_str(), m_pZlibStream->avail_in, m_pZlibStream->avail_out, m_pZlibStream->next_in, m_pZlibStream->next_out);

#ifdef STREAMENGINE_ENABLE_LISTENER
		NotifyListenerInflate inflateListener(gEnv->pSystem->GetStreamEngine()->GetListener(), this);
#endif

		u64 nBytesDecomped = m_nBytesDecompressed;

		STREAM_DECOMPRESS_TRACE("[StreamDecompress] ZlibInflateElementPartial_Impl %p %i %p %i %i\n",
		                        (u8*)m_pReadMemoryBuffer + nBytesDecomped,
		                        m_nFileSize - nBytesDecomped,
		                        (u8*)pSrc + nOffs,
		                        nBytes,
		                        nBytesDecomped);

		i32 readStatus = Z_OK;

		{
			DrxOptionalAutoLock<DrxCriticalSection> decompLock(m_externalBufferLockDecompress, m_pExternalMemoryBuffer != NULL);

			ZlibInflateElementPartial_Impl(
			  &readStatus,
			  m_pZlibStream,
			  m_pLookahead,
			  (u8*)m_pReadMemoryBuffer + nBytesDecomped,
			  m_nFileSize - nBytesDecomped,
			  m_bWriteOnlyExternal,
			  (u8*)pSrc + nOffs,
			  nBytes,
			  &nBytesDecomped
			  );
		}

		m_nBytesDecompressed = nBytesDecomped;

		//inform listen, so aysnc callback does not overlap
#ifdef STREAMENGINE_ENABLE_LISTENER
		inflateListener.End();
#endif

		if (readStatus == Z_OK || readStatus == Z_STREAM_END)
		{
#if defined(STREAMENGINE_ENABLE_TIMING)
			LARGE_INTEGER liEnd, liFreq;
			QueryPerformanceCounter(&liEnd);
			QueryPerformanceFrequency(&liFreq);

			m_unzipTime += CTimeValue((int64)((liEnd.QuadPart - liStart.QuadPart) * CTimeValue::TIMEVALUE_PRECISION / liFreq.QuadPart));
#endif
		}
		else
		{
#ifndef _RELEASE
			DrxFatalError("Decomp Error: %s : %s\n", m_strFileName.c_str(), m_pZlibStream ? m_pZlibStream->msg : "m_pZlibStream == NULL, no message available");
#endif
			Failed(ERROR_DECOMPRESSION_FAIL);
		}
	}

	if (pSrcHdr)
	{
		if (DrxInterlockedDecrement(&pSrcHdr->nRefs) == 0)
			engineState.pTempMem->TempFree(engineState.pHeap, pSrc, pSrcHdr->nSize);
	}

	job.pSrc = NULL;

	i32 nPopSlot = m_pDecompQueue->Pop();

	// job is no longer valid

	if (HasFailed() || bLast)
	{
		JobFinalize_Decompress(pSelf, engineState);
	}
	else if (nPopSlot >= 0)
	{
		// Chain start the next job, we're responsible for it.
		STREAM_DECOMPRESS_TRACE("[StreamDecompress] Chaining %p %i\n", this, nPopSlot);
		JobStart_Decompress(pSelf, engineState, nPopSlot);
	}

#if defined(STREAMENGINE_ENABLE_STATS)
	DrxInterlockedDecrement(&engineState.pStats->nCurrentDecompressCount);
#endif
}

#if defined(STREAMENGINE_SUPPORT_DECRYPT)
//////////////////////////////////////////////////////////////////////////
void CAsyncIOFileRequest::DecryptBlockEntry(SStreamJobEngineState engineState, i32 nJob)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecrypt] DecryptBlockEntry(%x) %p %s %i\n", DrxGetCurrentThreadId(), this, m_strFileName.c_str(), nJob);

	SStreamJobQueue::Job& job = m_pDecryptQueue->m_jobs[nJob];

	uk const pSrc = job.pSrc;
	SStreamPageHdr* const pSrcHdr = job.pSrcHdr;
	u32k nOffs = job.nOffs;
	u32k nBytes = job.nBytes;
	const bool bLast = job.bLast;
	const bool bFailed = HasFailed();
	const bool bCompressed = m_bCompressedBuffer;

	CAsyncIOFileRequest_TransferPtr pSelf(this);

	bool decryptOK = false;

	if (!bFailed)
	{
	#if defined(STREAMENGINE_ENABLE_TIMING)

		LARGE_INTEGER liStart;
		QueryPerformanceCounter(&liStart);

		tukk pFileNameShort = PathUtil::GetFile(m_strFileName.c_str());
		char eventName[128] = { 0 };
		drx_sprintf(eventName, "DcptBlck %s", pFileNameShort);
		DRX_PROFILE_REGION(PROFILE_SYSTEM, "DcptBlck");
		DRXPROFILE_SCOPE_PROFILE_MARKER(eventName);
		DRXPROFILE_SCOPE_PLATFORM_MARKER(eventName);
	#endif

		//printf("Inflate: %s Avail in: %d, Avail Out: %d, Next In: 0x%p, Next Out: 0x%p\n", m_strFileName.c_str(), m_pZlibStream->avail_in, m_pZlibStream->avail_out, m_pZlibStream->next_in, m_pZlibStream->next_out);

	#ifdef STREAMENGINE_ENABLE_LISTENER
		NotifyListenerDecrypt decryptListener(gEnv->pSystem->GetStreamEngine()->GetListener(), this);
	#endif

		u64 nBytesDecrypted = m_nBytesDecrypted;
		u8* pData = (u8*)pSrc + nOffs;

		//if (reinterpret_cast<UINT_PTR>(m_pExternalMemoryBuffer) < 0xc0000000 || reinterpret_cast<UINT_PTR>(m_pExternalMemoryBuffer) >= 0xd0000000)
		{
			DrxOptionalAutoLock<DrxCriticalSection> decryptLock(m_externalBufferLockDecrypt, m_pExternalMemoryBuffer != NULL);

			if (0)
			{
				//Intentionally empty
			}
	#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
			else if (m_pDecryptionCTR)
			{
				STREAM_DECOMPRESS_TRACE("[StreamDecrypt] ZipEncrypt::DecryptBufferWithStreamCipher %p %i %p %i %i\n",
				                        pData,
				                        m_nFileSize - nBytesDecrypted,
				                        (u8*)pSrc + nOffs,
				                        nBytes,
				                        nBytesDecrypted);

				decryptOK = ZipEncrypt::DecryptBufferWithStreamCipher(
				  pData,  //In
				  pData,  //Out - same = decrypt in place
				  nBytes,
				  m_pDecryptionCTR);
				nBytesDecrypted += decryptOK ? nBytes : 0;
			}
	#endif
			else
			{
				//Should never get here, this should have been checked in the prep functions
				DrxFatalError("Invalid encryption technique in streaming engine");
			}
		}

		m_nBytesDecrypted = nBytesDecrypted;

		//inform listen, so aysnc callback does not overlap
	#ifdef STREAMENGINE_ENABLE_LISTENER
		decryptListener.End();
	#endif

		if (decryptOK)
		{
	#if defined(STREAMENGINE_ENABLE_TIMING)
			LARGE_INTEGER liEnd, liFreq;
			QueryPerformanceCounter(&liEnd);
			QueryPerformanceFrequency(&liFreq);

			m_decryptTime += CTimeValue((int64)((liEnd.QuadPart - liStart.QuadPart) * CTimeValue::TIMEVALUE_PRECISION / liFreq.QuadPart));
	#endif
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "Decrypt Error: %s\n", m_strFileName.c_str());
			Failed(ERROR_DECRYPTION_FAIL);
		}
	}

	// FIXME later - if we end up here with a uncompressed request, that is not in-place, this won't copy
	// to the output. Not currently an issue given how ConfigureRead sets up m_bStreamInPlace, but may be in
	// future.

	if (!decryptOK || !bCompressed) // Inverse of push condition below
	{
		if (pSrcHdr)
		{
			if (DrxInterlockedDecrement(&pSrcHdr->nRefs) == 0)
				engineState.pTempMem->TempFree(engineState.pHeap, pSrc, pSrcHdr->nSize);
		}
	}

	i32 nPopSlot = m_pDecryptQueue->Pop();

	// job is no longer valid

	if (decryptOK && bCompressed)
	{
		PushDecompressBlock(engineState, pSrc, pSrcHdr, nOffs, nBytes, bLast);

		if (pSrcHdr)
		{
			if (DrxInterlockedDecrement(&pSrcHdr->nRefs) == 0)
				engineState.pTempMem->TempFree(engineState.pHeap, pSrc, pSrcHdr->nSize);
		}
	}

	if (HasFailed() || bLast)
	{
		JobFinalize_Decrypt(pSelf, engineState);
	}
	else if (nPopSlot >= 0)
	{
		// Chain start the next job, we're responsible for it.
		STREAM_DECOMPRESS_TRACE("[StreamDecrypt] Chaining %p %i\n", this, nPopSlot);

		JobStart_Decrypt(pSelf, engineState, nPopSlot);
	}

	#if defined(STREAMENGINE_ENABLE_STATS)
	DrxInterlockedDecrement(&engineState.pStats->nCurrentDecryptCount);
	#endif
}
#endif  //STREAMENGINE_SUPPORT_DECRYPT

//////////////////////////////////////////////////////////////////////////

u32 CAsyncIOFileRequest::PushDecompressPage(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nBytes, bool bLast)
{
	u32 nError = 0;

	for (u32 nBlockPos = 0; !nError && (nBlockPos < nBytes); nBlockPos += STREAMING_BLOCK_SIZE)
	{
		bool bLastBlock = (nBlockPos + STREAMING_BLOCK_SIZE) >= nBytes;
		u32 nBlockSize = min(nBytes - nBlockPos, (u32)STREAMING_BLOCK_SIZE);

		nError = PushDecompressBlock(engineState, pSrc, pSrcHdr, nBlockPos, nBlockSize, bLast && bLastBlock);
	}

	return nError;
}

u32 CAsyncIOFileRequest::PushDecompressBlock(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast)
{
	u32 nError = m_nError;

	if (!nError)
	{
		if (pSrcHdr)
			DrxInterlockedIncrement(&pSrcHdr->nRefs);

		i32 nPushJob = m_pDecompQueue->Push(pSrc, pSrcHdr, nOffs, nBytes, bLast);
		if (nPushJob >= 0)
		{
			STREAM_DECOMPRESS_TRACE("[StreamDecrypt] Spawning decompress job %p %i\n", this, nPushJob);

			AddRef();
			CAsyncIOFileRequest_TransferPtr pSelf(this);
			JobStart_Decompress(pSelf, engineState, nPushJob);
		}
	}

	return nError;
}

void CAsyncIOFileRequest::JobStart_Decompress(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState, i32 nJob)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] QueueDecompressBlockAppend(%x) %p %s %i\n", DrxGetCurrentThreadId(), this, m_strFileName.c_str(), nJob);

#if defined(STREAMENGINE_ENABLE_STATS)
	DrxInterlockedIncrement(&engineState.pStats->nCurrentDecompressCount);
#endif

	TStreamInflateBlockJob job(engineState, nJob);
	job.RegisterJobState(&pSelf->m_DecompJob);
	job.SetClassInstance(pSelf.Relinquish());
	job.SetPriorityLevel(JobUpr::eStreamPriority);
	job.Run();
}

#if defined(STREAMENGINE_SUPPORT_DECRYPT)
u32 CAsyncIOFileRequest::PushDecryptPage(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nBytes, bool bLast)
{
	u32 nError = 0;

	for (u32 nBlockPos = 0; !nError && (nBlockPos < nBytes); nBlockPos += STREAMING_BLOCK_SIZE)
	{
		bool bLastBlock = (nBlockPos + STREAMING_BLOCK_SIZE) >= nBytes;
		u32 nBlockSize = min(nBytes - nBlockPos, (u32)STREAMING_BLOCK_SIZE);

		nError = PushDecryptBlock(engineState, pSrc, pSrcHdr, nBlockPos, nBlockSize, bLast && bLastBlock);
	}

	return nError;
}

u32 CAsyncIOFileRequest::PushDecryptBlock(const SStreamJobEngineState& engineState, uk pSrc, SStreamPageHdr* pSrcHdr, u32 nOffs, u32 nBytes, bool bLast)
{
	u32 nError = m_nError;

	if (!nError)
	{
		if (pSrcHdr)
			DrxInterlockedIncrement(&pSrcHdr->nRefs);

		i32 nPushJob = m_pDecryptQueue->Push(pSrc, pSrcHdr, nOffs, nBytes, bLast);
		if (nPushJob >= 0)
		{
			STREAM_DECOMPRESS_TRACE("[StreamDecrypt] Spawning decrypt job %p %i\n", this, nPushJob);

			AddRef();
			CAsyncIOFileRequest_TransferPtr pSelf(this);
			JobStart_Decrypt(pSelf, engineState, nPushJob);
		}
	}

	return nError;
}

void CAsyncIOFileRequest::JobStart_Decrypt(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState, i32 nJob)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecrypt] QueueDecryptBlockAppend(%x) %p %s %i\n", DrxGetCurrentThreadId(), this, m_strFileName.c_str(), nJob);

	#if defined(STREAMENGINE_ENABLE_STATS)
	DrxInterlockedIncrement(&engineState.pStats->nCurrentDecryptCount);
	#endif

	TStreamDecryptBlockJob job(engineState, nJob);
	job.RegisterJobState(&pSelf->m_DecryptJob);
	job.SetClassInstance(pSelf.Relinquish());
	job.SetPriorityLevel(JobUpr::eStreamPriority);
	job.Run();
}
#endif  //STREAMENGINE_SUPPORT_DECRYPT

//////////////////////////////////////////////////////////////////////////
void CAsyncIOFileRequest::JobFinalize_Read(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState)
{
	if ((!pSelf->m_bCompressedBuffer && !pSelf->m_bEncryptedBuffer) || pSelf->HasFailed())
		JobFinalize_Transfer(pSelf, engineState);
}

void CAsyncIOFileRequest::JobFinalize_Decompress(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] FinalizeDecompress(%x) %p %s %p %p %p\n", DrxGetCurrentThreadId(), this, pSelf->m_strFileName.c_str(), &engineState, engineState.pStats, engineState.pDecompressStats);

	CAsyncIOFileRequest* pReq = &*pSelf;

	if (!pReq->HasFailed())
	{
		// Handle reads of subsections of a compressed file, by copying the section to the output
		u8* pDst = (u8*)pReq->m_pOutputMemoryBuffer;
		u8* pSrc = (u8*)pReq->m_pReadMemoryBuffer + pReq->m_nRequestedOffset;

		if (pDst != pSrc)
			memmove(pReq->m_pOutputMemoryBuffer, pSrc, pReq->m_nRequestedSize);

		pReq->JobFinalize_Validate(engineState);
	}

	pReq->JobFinalize_Buffer(engineState);

#if defined(STREAMENGINE_ENABLE_STATS) && defined(STREAMENGINE_ENABLE_TIMING)
	if (pReq->m_unzipTime.GetValue() != 0)
	{
		engineState.pDecompressStats->m_nTotalBytesUnziped += pReq->m_nFileSize;
		engineState.pDecompressStats->m_totalUnzipTime += pReq->m_unzipTime;

		engineState.pDecompressStats->m_nTempBytesUnziped += pReq->m_nFileSize;
		engineState.pDecompressStats->m_tempUnzipTime += pReq->m_unzipTime;
	}
#endif

	JobFinalize_Transfer(pSelf, engineState);
}

void CAsyncIOFileRequest::JobFinalize_Decrypt(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] FinalizeDecompress(%x) %p %s %p %p %p\n", DrxGetCurrentThreadId(), this, pSelf->m_strFileName.c_str(), &engineState, engineState.pStats, engineState.pDecompressStats);

	CAsyncIOFileRequest* pReq = &*pSelf;

	const bool bCompressed = pReq->m_bCompressedBuffer;
	const bool bFailed = pReq->HasFailed();

	if (!bCompressed && !bFailed)
	{
		pReq->JobFinalize_Validate(engineState);
	}

	pReq->JobFinalize_Buffer(engineState);

#if defined(STREAMENGINE_ENABLE_STATS) && defined(STREAMENGINE_ENABLE_TIMING)
	if (pReq->m_decryptTime.GetValue() != 0)
	{
		engineState.pDecompressStats->m_nTotalBytesDecrypted += pReq->m_nFileSize;
		engineState.pDecompressStats->m_totalDecryptTime += pReq->m_decryptTime;

		engineState.pDecompressStats->m_nTempBytesDecrypted += pReq->m_nFileSize;
		engineState.pDecompressStats->m_tempDecryptTime += pReq->m_decryptTime;
	}
#endif

	if (!bCompressed || pReq->HasFailed())
		JobFinalize_Transfer(pSelf, engineState);
}

void CAsyncIOFileRequest::JobFinalize_Buffer(const SStreamJobEngineState& engineState)
{
	if (DrxInterlockedDecrement(&m_nMemoryBufferUsers) == 0)
	{
		z_stream_s* pZlib = m_pZlibStream;

		if (pZlib)
		{
			//if the stream was cancelled in flight, inform zlib to free internal allocs
			if (pZlib->state)
				inflateEnd(pZlib);

			m_pZlibStream = NULL;
		}

		if (m_pMemoryBuffer)
		{
			engineState.pTempMem->TempFree(engineState.pHeap, m_pMemoryBuffer, m_nMemoryBufferSize);

			m_pMemoryBuffer = NULL;
			m_nMemoryBufferSize = 0;
		}
	}
}

void CAsyncIOFileRequest::JobFinalize_Validate(const SStreamJobEngineState& engineState)
{
#if defined(SKIP_CHECKSUM_FROM_OPTICAL_MEDIA)
	if (m_eMediaType != eStreamSourceTypeDisc)
#endif  //SKIP_CHECKSUM_FROM_OPTICAL_MEDIA
	{
		DrxOptionalAutoLock<DrxCriticalSection> readLock(m_externalBufferLockRead, m_pExternalMemoryBuffer != NULL);
		if (!HasFailed())
		{
			if (m_crc32FromHeader != 0 && m_nPageReadStart == 0 && m_nRequestedSize == m_nFileSize)  //Compute the CRC32 if appropriate.
			{
#if defined(STREAMENGINE_ENABLE_TIMING)
				LARGE_INTEGER liStart, liEnd, liFreq;
				QueryPerformanceCounter(&liStart);
#endif  //STREAMENGINE_ENABLE_TIMING

				u32 nCRC32 = crc32(0, (u8*)m_pReadMemoryBuffer + m_nPageReadStart, m_nRequestedSize);

#if defined(STREAMENGINE_ENABLE_TIMING)
				QueryPerformanceCounter(&liEnd);
				QueryPerformanceFrequency(&liFreq);

				m_verifyTime += CTimeValue((int64)((liEnd.QuadPart - liStart.QuadPart) * CTimeValue::TIMEVALUE_PRECISION / liFreq.QuadPart));

	#if defined(STREAMENGINE_ENABLE_STATS)
				engineState.pDecompressStats->m_nTotalBytesVerified += m_nFileSize;
				engineState.pDecompressStats->m_totalVerifyTime += m_verifyTime;
				engineState.pDecompressStats->m_nTempBytesVerified += m_nFileSize;
				engineState.pDecompressStats->m_tempVerifyTime += m_verifyTime;
	#endif //defined(STREAMENGINE_ENABLE_STATS)
#endif   //STREAMENGINE_ENABLE_TIMING

				if (m_crc32FromHeader != nCRC32)
				{
					//The contents of this file don't match what the header expects
#if !defined(_RELEASE)
					DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "Streaming Engine Failed to verify a file (%s). Computed CRC32 %u does not match stored CRC32 %u", m_strFileName.c_str(), nCRC32, m_crc32FromHeader);
#endif    //!_RELEASE
					Failed(ERROR_VERIFICATION_FAIL);
				}
			}
		}
	}
}

void CAsyncIOFileRequest::JobFinalize_Transfer(CAsyncIOFileRequest_TransferPtr& pSelf, const SStreamJobEngineState& engineState)
{
	STREAM_DECOMPRESS_TRACE("[StreamDecompress] FinalizeTransform(%x) %p %s %p %p %p\n", DrxGetCurrentThreadId(), this, pSelf->m_strFileName.c_str(), &engineState, engineState.pStats, engineState.pDecompressStats);

	if (DrxInterlockedCompareExchange(&pSelf->m_nFinalised, 1, 0) == 0)
	{
#if defined(STREAMENGINE_ENABLE_STATS)
		DrxInterlockedIncrement(&engineState.pStats->nCurrentAsyncCount);
#endif

#if defined(STREAMENGINE_ENABLE_TIMING)
		pSelf->m_completionTime = gEnv->pTimer->GetAsyncTime();
#endif

		i32 nCallbackThreads = engineState.pReportQueues->size();

		EStreamTaskType eType = pSelf->m_eType;
		if (nCallbackThreads > 1 && eType == eStreamTaskTypeGeometry)
		{
			// If we have more then 1 call back threads, use this one for geometry only.
			(*engineState.pReportQueues)[1]->TransferRequest(pSelf);
		}
		else if (nCallbackThreads > 2 && eType == eStreamTaskTypeTexture)
		{
			// If we have more then 1 call back threads, use this one for textures only.
			(*engineState.pReportQueues)[2]->TransferRequest(pSelf);
		}
		else if (nCallbackThreads > 3 && eType == eStreamTaskTypeMergedMesh)
		{
			// If we have more then 3 call back threads, use this one for merged meshes only.
			(*engineState.pReportQueues)[3]->TransferRequest(pSelf);
		}
		else if (nCallbackThreads > 0)
		{
			(*engineState.pReportQueues)[0]->TransferRequest(pSelf);
		}
		else
		{
			__debugbreak();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void SStreamRequestQueue::TransferRequest(CAsyncIOFileRequest_TransferPtr& pRequest)
{
	{
		DrxAutoLock<DrxCriticalSection> l(m_lock);

		m_requests.push_back(pRequest.Relinquish());
	}

	m_awakeEvent.Set();
}

//////////////////////////////////////////////////////////////////////////
void SStreamEngineTempMemStats::TempFree(CMTSafeHeap* pHeap, ukk p, size_t nSize)
{
#if MTSAFE_USE_GENERAL_HEAP
	bool bInGenHeap = pHeap->IsInGeneralHeap(p);
#else
	bool bInGenHeap = false;
#endif
	pHeap->FreeTemporary(const_cast<uk>(p));
	ReportTempMemAlloc(0, bInGenHeap ? 0 : nSize, true);
}

void SStreamEngineTempMemStats::ReportTempMemAlloc(u32 nSizeAlloc, u32 nSizeFree, bool bTriggerWake)
{
	i32 nAdd = (i32)nSizeAlloc - (i32)nSizeFree;
	i32 const nOldSize = DrxInterlockedExchangeAdd(&m_nTempAllocatedMemory, nAdd);
	i32 const nNewSize = nOldSize + nAdd;

	LONG nNewMax = 0;
	LONG nOldMax = 0;
	do
	{
		nOldMax = m_nTempAllocatedMemoryFrameMax;
		nNewMax = (LONG)max((i32)nNewSize, (i32)nOldMax);
	}
	while (DrxInterlockedCompareExchange(&m_nTempAllocatedMemoryFrameMax, nNewMax, nOldMax) != nOldMax);

	if (bTriggerWake)
	{
		for (i32 i = 0, c = m_nWakeEvents; i != c; ++i)
			m_wakeEvents[i]->Set();
	}
}
