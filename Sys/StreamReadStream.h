// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   StreamReadStream.h
//  Created:     27/07/2010 by Timur.
//  Описание: Streaming Engine
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _DRX_SYSTEM_READ_STREAM_PROXY_HDR_
#define _DRX_SYSTEM_READ_STREAM_PROXY_HDR_

#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Sys/StreamAsyncFileRequest.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

class CStreamEngine;

class CReadStream : public IReadStream
{
	friend class CStreamEngine;

public:
	static CReadStream* Allocate(CStreamEngine* pEngine, const EStreamTaskType tSource, tukk szFilename, IStreamCallback* pCallback, const StreamReadParams* pParams);
	static void         Flush();

public:
	CReadStream();
	virtual ~CReadStream();

	virtual i32       AddRef();
	virtual i32       Release();

	virtual DWORD_PTR GetUserData() { return m_Params.dwUserData; }

	// set user defined data into stream's params
	virtual void SetUserData(DWORD_PTR dwUserData);

	// returns true if the file read was not successful.
	virtual bool IsError() { return m_bError; };

	// returns true if the file read was completed (successfully or unsuccessfully)
	// check IsError to check if the whole requested file (piece) was read
	virtual bool IsFinished();

	// returns the number of bytes read so far (the whole buffer size if IsFinished())
	virtual u32 GetBytesRead(bool bWait);

	// returns the buffer into which the data has been or will be read
	// at least GetBytesRead() bytes in this buffer are guaranteed to be already read
	virtual ukk GetBuffer();

	void                AbortShutdown();

	// tries to stop reading the stream; this is advisory and may have no effect
	// but the callback	will not be called after this. If you just destructing object,
	// dereference this object and it will automatically abort and release all associated resources.
	virtual void Abort();
	virtual bool TryAbort();

	// tries to raise the priority of the read; this is advisory and may have no effect
	virtual void SetPriority(EStreamTaskPriority EPriority);

	// unconditionally waits until the callback is called
	// i.e. if the stream hasn't yet finish, it's guaranteed that the user-supplied callback
	// is called before return from this function (unless no callback was specified)
	virtual void                    Wait(i32 nMaxWaitMillis = -1);

	virtual uint64                  GetPriority() const;

	virtual const StreamReadParams& GetParams() const     { return m_Params; }

	virtual const EStreamTaskType   GetCallerType() const { return m_Type; }

	virtual EStreamSourceMediaType  GetMediaType() const  { return m_MediaType; }

	// return pointer to callback routine(can be NULL)
	virtual IStreamCallback* GetCallback() const;

	// return IO error #
	virtual unsigned GetError() const;

	//	 Returns IO error name
	virtual tukk GetErrorName() const;

	// return stream name
	virtual tukk GetName() const { return m_strFileName.c_str(); };

	virtual void        FreeTemporaryMemory();

	// this gets called upon the IO has been executed to call the callbacks
	void MainThread_Finalize();

	bool IsReqReading();

#ifdef STREAMENGINE_ENABLE_STATS
	void              SetRequestTime(CTimeValue& time) { m_requestTime = time; }
	const CTimeValue& GetRequestTime()                 { return m_requestTime; }
#endif

	// decompression of zip-compressed files with default behavior
	CAsyncIOFileRequest* CreateFileRequest();
	void                 ComputedMediaType(EStreamSourceMediaType eMT) { m_MediaType = eMT; }
	uk                OnNeedStorage(size_t size, bool& bAbortOnFailToAlloc);
	void                 OnAsyncFileRequestComplete();
	CAsyncIOFileRequest* GetFileRequest() { return m_pFileRequest; }

private:
	void Reset();

	// call the async callback
	void ExecuteAsyncCallback_CBLocked();

	// call the sync callback
	void ExecuteSyncCallback_CBLocked();

private:
	uk operator new(size_t sz);
	void  operator delete(uk p);

private:
	static SLockFreeSingleLinkedListHeader s_freeRequests;

private:
	SLockFreeSingleLinkedListEntry m_nextFree;

	DrxStringLocal                 m_strFileName;
	DrxCriticalSection             m_callbackLock;
	CAsyncIOFileRequest_AutoPtr    m_pFileRequest;

	StreamReadParams               m_Params;

	// Only POD types must exist below here. They will be memset!

	 i32   m_nRefCount;
	CStreamEngine* m_pEngine;

	// the type of the task
	EStreamTaskType        m_Type;
	EStreamSourceMediaType m_MediaType;
	// the initial data from the user
	// the callback; may be NULL
	IStreamCallback* m_pCallback;

	// Bytes actually read from media.
	u32        m_nBytesRead;

	 bool m_bIsAsyncCallbackExecuted;
	 bool m_bIsSyncCallbackExecuted;
	 bool m_bFileRequestComplete;

	// the actual buffer to read to
	uk         m_pBuffer;

	 bool m_bError;
	 bool m_bFinished;
	u32  m_nIOError;

#ifdef STREAMENGINE_ENABLE_STATS
	// time when request was made
	CTimeValue m_requestTime;
	// Time for actual reading
	CTimeValue m_ReadTime;
#endif
};

TYPEDEF_AUTOPTR(CReadStream);

#endif
