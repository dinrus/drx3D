// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StreamReadStream.cpp
//  Created:     27/07/2010 by Timur.
//  Описание: Streaming Engine
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IDiskProfiler.h>

#include <drx3D/Sys/StreamReadStream.h>
#include <drx3D/Sys/StreamEngine.h>
#include <drx3D/Sys/MTSafeAllocator.h>

extern CMTSafeHeap* g_pPakHeap;
SLockFreeSingleLinkedListHeader CReadStream::s_freeRequests;

CReadStream* CReadStream::Allocate(CStreamEngine* pEngine, const EStreamTaskType tSource, tukk szFilename, IStreamCallback* pCallback, const StreamReadParams* pParams)
{
	tuk pFree = reinterpret_cast<tuk>(DrxInterlockedPopEntrySList(s_freeRequests));

	CReadStream* pReq;
	IF_LIKELY (pFree)
	{
		ptrdiff_t offs = offsetof(CReadStream, m_nextFree);
		pReq = reinterpret_cast<CReadStream*>(pFree - offs);
	}
	else
	{
		pReq = new CReadStream;
	}

	pReq->m_pEngine = pEngine;
	pReq->m_Type = tSource;
	pReq->m_strFileName = szFilename;
	pReq->m_pCallback = pCallback;
	if (pParams)
		pReq->m_Params = *pParams;
	pReq->m_pBuffer = pReq->m_Params.pBuffer;

#ifdef STREAMENGINE_ENABLE_STATS
	pReq->m_requestTime = gEnv->pTimer->GetAsyncTime();
#endif

	return pReq;
}

void CReadStream::Flush()
{
	ptrdiff_t offs = offsetof(CReadStream, m_nextFree);

	for (tuk pFree = reinterpret_cast<tuk>(DrxInterlockedPopEntrySList(s_freeRequests));
	     pFree;
	     pFree = reinterpret_cast<tuk>(DrxInterlockedPopEntrySList(s_freeRequests)))
	{
		CReadStream* pReq = reinterpret_cast<CReadStream*>(pFree - offs);
		delete pReq;
	}
}

//////////////////////////////////////////////////////////////////////////
CReadStream::CReadStream()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
CReadStream::~CReadStream()
{
}

// returns true if the file read was completed (successfully or unsuccessfully)
// check IsError to check if the whole requested file (piece) was read
bool CReadStream::IsFinished()
{
	return m_bFinished;
}

// returns the number of bytes read so far (the whole buffer size if IsFinished())
u32 CReadStream::GetBytesRead(bool bWait)
{
	if (!m_bError)
		return m_Params.nSize;
	return 0;
}

// returns the buffer into which the data has been or will be read
// at least GetBytesRead() bytes in this buffer are guaranteed to be already read
ukk CReadStream::GetBuffer()
{
	return m_pBuffer;
}

void CReadStream::AbortShutdown()
{
	{
		DrxAutoCriticalSection lock(m_callbackLock);

		m_bError = true;
		m_nIOError = ERROR_ABORTED_ON_SHUTDOWN;
		m_bFileRequestComplete = true;

		if (!m_pFileRequest)
			DrxFatalError("File request still exists");
	}

	// lock this object to avoid preliminary destruction
	CReadStream_AutoPtr pLock(this);

	{
		DrxAutoCriticalSection lock(m_callbackLock);

		// all the callbacks have to handle error cases and needs to be called anyway, even if the stream I/O is aborted
		ExecuteAsyncCallback_CBLocked();
		ExecuteSyncCallback_CBLocked();

		m_pCallback = NULL;
	}
}

// tries to stop reading the stream; this is advisory and may have no effect
// all the callbacks	will be called after this. If you just destructing object,
// dereference this object and it will automatically abort and release all associated resources.
void CReadStream::Abort()
{
	{
		DrxAutoCriticalSection lock(m_callbackLock);

		m_bError = true;
		m_nIOError = ERROR_USER_ABORT;
		m_bFileRequestComplete = true;

		if (m_pFileRequest)
		{
			m_pFileRequest->Cancel();
			m_pFileRequest = nullptr;
		}
	}

	// lock this object to avoid preliminary destruction
	CReadStream_AutoPtr pLock(this);

	{
		DrxAutoCriticalSection lock(m_callbackLock);

		// all the callbacks have to handle error cases and needs to be called anyway, even if the stream I/O is aborted
		ExecuteAsyncCallback_CBLocked();
		ExecuteSyncCallback_CBLocked();

		m_pCallback = NULL;
	}

	m_pEngine->AbortJob(this);
}

bool CReadStream::TryAbort()
{
	if (!m_callbackLock.TryLock())
		return false;

	if (m_pFileRequest && !m_pFileRequest->TryCancel())
	{
		m_callbackLock.Unlock();
		return false;
	}

	m_bError = true;
	m_nIOError = ERROR_USER_ABORT;
	m_bFileRequestComplete = true;
	m_pFileRequest = nullptr;

	// lock this object to avoid preliminary destruction
	CReadStream_AutoPtr pLock(this);

	// all the callbacks have to handle error cases and needs to be called anyway, even if the stream I/O is aborted
	ExecuteAsyncCallback_CBLocked();
	ExecuteSyncCallback_CBLocked();

	m_pCallback = NULL;

	m_callbackLock.Unlock();

	m_pEngine->AbortJob(this);

	return true;
}

// tries to raise the priority of the read; this is advisory and may have no effect
void CReadStream::SetPriority(EStreamTaskPriority ePriority)
{
	if (m_Params.ePriority != ePriority)
	{
		m_Params.ePriority = ePriority;
		if (m_pFileRequest && m_pFileRequest->m_status == CAsyncIOFileRequest::eStatusInFileQueue)
		{
			m_pEngine->UpdateJobPriority(this);
		}
	}
}

// unconditionally waits until the callback is called
// i.e. if the stream hasn't yet finish, it's guaranteed that the user-supplied callback
// is called before return from this function (unless no callback was specified)
void CReadStream::Wait(i32 nMaxWaitMillis)
{
	// lock this object to avoid preliminary destruction
	CReadStream_AutoPtr pLock(this);

	bool bNeedFinalize = (m_Params.nFlags & IStreamEngine::FLAGS_NO_SYNC_CALLBACK) == 0;

	if (!m_bFinished && !m_bError && !m_pFileRequest)
	{
		// This will almost certainly cause Dead-Lock
		DrxFatalError("Waiting for stream when StreamingEngine is paused");
	}

	CTimeValue t0;

	if (nMaxWaitMillis > 0)
	{
		t0 = gEnv->pTimer->GetAsyncTime();
	}

	while (!m_bFinished && !m_bError)
	{
		if (bNeedFinalize)
		{
			m_pEngine->MainThread_FinalizeIOJobs();
		}
		if (!m_bFileRequestComplete)
		{
			DrxSleep(5);
		}

		if (nMaxWaitMillis > 0)
		{
			CTimeValue t1 = gEnv->pTimer->GetAsyncTime();
			if (CTimeValue(t1 - t0).GetMilliSeconds() > nMaxWaitMillis)
			{
				// Break if we are waiting for too long.
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
uint64 CReadStream::GetPriority() const
{
	return 0;
}

// this gets called upon the IO has been executed to call the callbacks
void CReadStream::MainThread_Finalize()
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	// call asynchronous callback function if needed synchronously
	{
		DrxAutoCriticalSection lock(m_callbackLock);
		ExecuteSyncCallback_CBLocked();
	}
	m_pFileRequest = nullptr;
}

IStreamCallback* CReadStream::GetCallback() const
{
	return m_pCallback;
}

unsigned CReadStream::GetError() const
{
	return m_nIOError;
}

tukk CReadStream::GetErrorName() const
{
	switch (m_nIOError)
	{
	case ERROR_UNKNOWN_ERROR:
		return "Unknown error";
	case ERROR_UNEXPECTED_DESTRUCTION:
		return "Unexpected destruction";
	case ERROR_INVALID_CALL:
		return "Invalid call";
	case ERROR_CANT_OPEN_FILE:
		return "Cannot open the file";
	case ERROR_REFSTREAM_ERROR:
		return "Refstream error";
	case ERROR_OFFSET_OUT_OF_RANGE:
		return "Offset out of range";
	case ERROR_REGION_OUT_OF_RANGE:
		return "Region out of range";
	case ERROR_SIZE_OUT_OF_RANGE:
		return "Size out of range";
	case ERROR_CANT_START_READING:
		return "Cannot start reading";
	case ERROR_OUT_OF_MEMORY:
		return "Out of memory";
	case ERROR_ABORTED_ON_SHUTDOWN:
		return "Aborted on shutdown";
	case ERROR_OUT_OF_MEMORY_QUOTA:
		return "Out of memory quota";
	case ERROR_ZIP_CACHE_FAILURE:
		return "ZIP cache failure";
	case ERROR_USER_ABORT:
		return "User aborted";
	}
	return "Unrecognized error";
}

i32 CReadStream::AddRef()
{
	return DrxInterlockedIncrement(&m_nRefCount);
}

i32 CReadStream::Release()
{
	i32 nRef = DrxInterlockedDecrement(&m_nRefCount);

#ifndef _RELEASE
	if (nRef < 0)
		__debugbreak();
#endif

	if (nRef == 0)
	{
		Reset();
		DrxInterlockedPushEntrySList(s_freeRequests, m_nextFree);
	}

	return nRef;
}

void CReadStream::Reset()
{
	m_strFileName.clear();
	m_pFileRequest = nullptr;
	m_Params = StreamReadParams();
	// WTHF
	memset((uk )&m_nRefCount, 0, (tuk)(this + 1) - (tuk)(&m_nRefCount));
}

void CReadStream::SetUserData(DWORD_PTR dwUserData)
{
	m_Params.dwUserData = dwUserData;
}

//////////////////////////////////////////////////////////////////////////
void CReadStream::ExecuteAsyncCallback_CBLocked()
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	if (!m_bIsAsyncCallbackExecuted && m_pCallback)
	{
		m_bIsAsyncCallbackExecuted = true;
		MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "Streaming Callback %s", gEnv->pSystem->GetStreamEngine()->GetStreamTaskTypeName(m_Type));

		m_pCallback->StreamAsyncOnComplete(this, m_nIOError);
	}
}

void CReadStream::ExecuteSyncCallback_CBLocked()
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);

	CReadStream_AutoPtr protectMe;
	if (!m_bIsSyncCallbackExecuted && m_pCallback && (0 == (m_Params.nFlags & IStreamEngine::FLAGS_NO_SYNC_CALLBACK)))
	{
		m_bIsSyncCallbackExecuted = true;
		protectMe = this; // Stream can be freed inside the callback!

		m_pCallback->StreamOnComplete(this, m_nIOError);
	}

	m_pFileRequest = nullptr;
	m_pBuffer = nullptr;
	m_bFinished = true;

#ifdef STREAMENGINE_ENABLE_LISTENER
	IStreamEngineListener* pListener = m_pEngine->GetListener();
	if (pListener)
		pListener->OnStreamDone(this);
#endif
}

uk CReadStream::operator new(size_t sz)
{
	return DrxModuleMemalign(sz, alignof(CReadStream));
}

void CReadStream::operator delete(uk p)
{
	DrxModuleMemalignFree(p);
}

//////////////////////////////////////////////////////////////////////////
void CReadStream::FreeTemporaryMemory()
{
	// Free temporary block.
	if (m_pFileRequest)
	{
		m_pFileRequest->SyncWithDecompress();
		m_pFileRequest->SyncWithDecrypt();
		m_pFileRequest->FreeBuffer();
	}
	m_pBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
bool CReadStream::IsReqReading()
{
	return !m_strFileName.empty();
}

//////////////////////////////////////////////////////////////////////////
CAsyncIOFileRequest* CReadStream::CreateFileRequest()
{
	m_pFileRequest = CAsyncIOFileRequest::Allocate(m_Type);
	m_pFileRequest->m_nRequestedSize = m_Params.nSize;
	m_pFileRequest->m_nRequestedOffset = m_Params.nOffset;
	m_pFileRequest->m_pExternalMemoryBuffer = m_pBuffer;
	m_pFileRequest->m_bWriteOnlyExternal = (m_Params.nFlags & IStreamEngine::FLAGS_WRITE_ONLY_EXTERNAL_BUFFER) != 0;
	m_pFileRequest->m_pReadStream = this;
	m_pFileRequest->m_strFileName = m_strFileName;
	m_pFileRequest->m_ePriority = m_Params.ePriority;
	m_pFileRequest->m_eMediaType = m_Params.eMediaType;

	m_bFileRequestComplete = false;
	return m_pFileRequest;
}

uk CReadStream::OnNeedStorage(size_t size, bool& bAbortOnFailToAlloc)
{
	DrxAutoCriticalSection lock(m_callbackLock);

	if (m_pCallback)
		return m_pCallback->StreamOnNeedStorage(this, size, bAbortOnFailToAlloc);
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CReadStream::OnAsyncFileRequestComplete()
{
	DrxAutoCriticalSection lock(m_callbackLock);

	if (!m_bFileRequestComplete)
	{
		if (m_pFileRequest)
		{
			m_Params.nSize = m_pFileRequest->m_nRequestedSize;
			m_pBuffer = m_pFileRequest->m_pOutputMemoryBuffer;
			m_nBytesRead = m_pFileRequest->m_nSizeOnMedia;
			m_nIOError = m_pFileRequest->m_nError;
			m_bError = m_nIOError != 0;
			if (m_bError)
				m_nBytesRead = 0;

#ifdef STREAMENGINE_ENABLE_STATS
			m_ReadTime = m_pFileRequest->m_readTime;
#endif
		}

		ExecuteAsyncCallback_CBLocked();

		if (m_Params.nFlags & IStreamEngine::FLAGS_NO_SYNC_CALLBACK)
		{
			// We do not need FileRequest here anymore, and not its temporary memory.
			m_pFileRequest = nullptr;
			m_bFinished = true;
		}

		m_bFileRequestComplete = true;
	}
}
