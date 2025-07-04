// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ------------------------------------------------------------------------
//  Имя файла:   GeomCacheUpr.h
//  Created:     20/7/2012 by Axel Gneiting
//  Описание: Manages geometry cache instances and streaming
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _GEOMCACHE_MANAGER_
#define _GEOMCACHE_MANAGER_

#pragma once

#if defined(USE_GEOM_CACHES)

	#include <drx3D/Eng3D/GeomCacheDecoder.h>
	#include <drx3D/Eng3D/GeomCacheMeshUpr.h>

class CGeomCache;
class CGeomCacheRenderNode;

struct SGeomCacheStreamInfo;

struct SGeomCacheBufferHandle
{
	SGeomCacheBufferHandle()
	{
		m_pNext = NULL;
		m_startFrame = 0;
		m_endFrame = 0;
		m_bufferSize = 0;
		m_pBuffer = NULL;
		m_numJobReferences = 0;
	}

	 i32          m_numJobReferences;
	u32                m_bufferSize;
	u32                m_startFrame;
	u32                m_endFrame;
	tuk                 m_pBuffer;

	SGeomCacheStreamInfo* m_pStream;
	CTimeValue            m_frameTime;

	// Next buffer handle for this stream or in the free list
	SGeomCacheBufferHandle* m_pNext;

	DrxConditionVariable    m_jobReferencesCV;
};

// This handle represents a block in the read buffer
struct SGeomCacheReadRequestHandle : public SGeomCacheBufferHandle, public IStreamCallback
{
	// IStreamCallback
	virtual void StreamOnComplete(IReadStream* pStream, unsigned nError) {}
	virtual void StreamAsyncOnComplete(IReadStream* pStream, unsigned nError)
	{
		if (nError != 0 && nError != ERROR_USER_ABORT)
		{
			string error = "Geom cache read request failed with error: " + string(pStream->GetErrorName());
			gEnv->pLog->LogError("%s", error.c_str());
		}

		m_state = eRRHS_FinishedRead;
		m_error = nError;

		if (DrxInterlockedDecrement(&m_numJobReferences) == 0)
		{
			m_jobReferencesCV.Notify();
		}
	}

	enum EReadRequestHandleState
	{
		eRRHS_Reading       = 0,
		eRRHS_FinishedRead  = 1,
		eRRHS_Decompressing = 2,
		eRRHS_Done          = 3
	};

	 EReadRequestHandleState m_state;
	 long                    m_error;
	IReadStreamPtr                   m_pReadStream;
};

struct SGeomCacheStreamInfo
{
	SGeomCacheStreamInfo(CGeomCacheRenderNode* pRenderNode, CGeomCache* pGeomCache, const uint numFrames)
		: m_pRenderNode(pRenderNode)
		, m_pGeomCache(pGeomCache)
		, m_numFrames(numFrames)
		, m_displayedFrameTime(-1.0f)
		, m_wantedPlaybackTime(0.0f)
		, m_wantedFloorFrame(0)
		, m_wantedCeilFrame(0)
		, m_sameFrameFillCount(0)
		, m_numFramesMissed(0)
		, m_pOldestReadRequestHandle(NULL)
		, m_pNewestReadRequestHandle(NULL)
		, m_pReadAbortListHead(NULL)
		, m_pOldestDecompressHandle(NULL)
		, m_pNewestDecompressHandle(NULL)
		, m_pDecompressAbortListHead(NULL)
		, m_bAbort(0L)
		, m_bLooping(false)
	{}

	CGeomCacheRenderNode*        m_pRenderNode;
	CGeomCache*                  m_pGeomCache;

	uint                         m_numFrames;

	 float               m_displayedFrameTime;
	 float               m_wantedPlaybackTime;
	 uint                m_wantedFloorFrame;
	 uint                m_wantedCeilFrame;
	 i32                 m_sameFrameFillCount;

	uint                         m_numFramesMissed;

	SGeomCacheReadRequestHandle* m_pOldestReadRequestHandle;
	SGeomCacheReadRequestHandle* m_pNewestReadRequestHandle;
	SGeomCacheReadRequestHandle* m_pReadAbortListHead;

	SGeomCacheBufferHandle*      m_pOldestDecompressHandle;
	SGeomCacheBufferHandle*      m_pNewestDecompressHandle;
	SGeomCacheBufferHandle*      m_pDecompressAbortListHead;

	 bool                m_bAbort;
	DrxCriticalSection           m_abortCS;

	bool                         m_bLooping;

	JobUpr::SJobState        m_fillRenderNodeJobState;

	struct SFrameData
	{
		bool m_bDecompressJobLaunched;

		// For each frame we initialize a counter to the number of jobs
		// that need to complete before the frame can be decoded.
		//
		// Each index frame has exactly one dependent job (inflate)
		// The first B frame after an index frame has three dependencies (inflate + previous and last index frame)
		// All other B frames have only two dependencies (inflate + previous B frame)
		i32 m_decodeDependencyCounter;

		// Pointer to decompress handle for this frame.
		SGeomCacheBufferHandle* m_pDecompressHandle;
	};

	// Array with data for each frame
	std::vector<SFrameData> m_frameData;
};

struct SDecodeFrameJobData
{
	uint                  m_frameIndex;
	const CGeomCache*     m_pGeomCache;
	SGeomCacheStreamInfo* m_pStreamInfo;
};

class CGeomCacheUpr : public DinrusX3dEngBase
{
public:
	CGeomCacheUpr();
	~CGeomCacheUpr();

	// Called during level unload to free all resource references
	void        Reset();

	CGeomCache* LoadGeomCache(tukk szFileName);
	void        DeleteGeomCache(CGeomCache* pGeomCache);

	void        StreamingUpdate();

	void        RegisterForStreaming(CGeomCacheRenderNode* pRenderNode);
	void        UnRegisterForStreaming(CGeomCacheRenderNode* pRenderNode, bool bWaitForJobs);

	float       GetPrecachedTime(const IGeomCacheRenderNode* pRenderNode);

	#ifndef _RELEASE
	void DrawDebugInfo();
	void ResetDebugInfo() { m_numMissedFrames = 0; m_numStreamAborts = 0; m_numFailedAllocs = 0; }
	#endif

	void DecompressFrame_JobEntry(SGeomCacheStreamInfo* pStreamInfo, const uint blockIndex,
	                              SGeomCacheBufferHandle* pDecompressHandle, SGeomCacheReadRequestHandle* pReadRequestHandle);

	void                   FillRenderNodeAsync_JobEntry(SGeomCacheStreamInfo* pStreamInfo);

	void                   DecodeIFrame_JobEntry(SDecodeFrameJobData jobState);
	void                   DecodeBFrame_JobEntry(SDecodeFrameJobData jobState);

	CGeomCacheMeshUpr& GetMeshUpr() { return m_meshUpr; }

	void                   StopCacheStreamsAndWait(CGeomCache* pGeomCache);

	CGeomCache*            FindGeomCacheByFilename(tukk filename);

	// For changing the buffer size on runtime. This will do a blocking wait on all active streams,
	// so it should only be called when we are sure that no caches are playing (e.g. on level load)
	void ChangeBufferSize(const uint newSizeInMiB);

private:
	static void                                          OnChangeBufferSize(ICVar* pCVar);

	void                                                 ReinitializeStreamFrameData(SGeomCacheStreamInfo& streamInfo, uint startFrame, uint endFrame);

	void                                                 UnloadGeomCaches();

	bool                                                 IssueDiskReadRequest(SGeomCacheStreamInfo& pStreamInfo);

	void                                                 LaunchStreamingJobs(const uint numStreams, const CTimeValue currentFrameTime);
	void                                                 LaunchDecompressJobs(SGeomCacheStreamInfo* pStreamInfo, const CTimeValue currentFrameTime);
	void                                                 LaunchDecodeJob(SDecodeFrameJobData jobState);

	template<class TBufferHandleType> TBufferHandleType* NewBufferHandle(u32k size, SGeomCacheStreamInfo& streamInfo);
	SGeomCacheReadRequestHandle*                         NewReadRequestHandle(u32k size, SGeomCacheStreamInfo& streamInfo);

	void                                                 RetireHandles(SGeomCacheStreamInfo& streamInfo);
	void                                                 RetireOldestReadRequestHandle(SGeomCacheStreamInfo& streamInfo);
	void                                                 RetireOldestDecompressHandle(SGeomCacheStreamInfo& streamInfo);
	void                                                 RetireDecompressHandle(SGeomCacheStreamInfo& streamInfo, SGeomCacheBufferHandle* pHandle);
	template<class TBufferHandleType> void               RetireBufferHandle(TBufferHandleType* pHandle);

	void                                                 RetireRemovedStreams();
	void                                                 ValidateStream(SGeomCacheStreamInfo& streamInfo);
	void                                                 AbortStream(SGeomCacheStreamInfo& streamInfo);
	void                                                 AbortStreamAndWait(SGeomCacheStreamInfo& streamInfo);
	void                                                 RetireAbortedHandles(SGeomCacheStreamInfo& streamInfo);

	SGeomCacheBufferHandle*                              GetFrameDecompressHandle(SGeomCacheStreamInfo* pStreamInfo, const uint frameIndex);
	SGeomCacheFrameHeader*                               GetFrameDecompressHeader(SGeomCacheStreamInfo* pStreamInfo, const uint frameIndex);
	tuk                                                GetFrameDecompressData(SGeomCacheStreamInfo* pStreamInfo, const uint frameIndex);
	i32*                                                 GetDependencyCounter(SGeomCacheStreamInfo* pStreamInfo, const uint frameIndex);

	uk               m_pPoolBaseAddress;
	IGeneralMemoryHeap* m_pPool;
	size_t              m_poolSize;

	uint                m_lastRequestStream;

	uint                m_numMissedFrames;
	uint                m_numStreamAborts;
	uint                m_numErrorAborts;
	uint                m_numDecompressStreamAborts;
	uint                m_numReadStreamAborts;
	uint                m_numFailedAllocs;

	typedef std::vector<SGeomCacheStreamInfo*>::iterator TStreamInfosIter;
	std::vector<SGeomCacheStreamInfo*> m_streamInfos;
	std::vector<SGeomCacheStreamInfo*> m_streamInfosAbortList;

	typedef std::map<string, CGeomCache*, stl::less_stricmp<string>> TGeomCacheMap;
	TGeomCacheMap         m_nameToGeomCacheMap;

	CGeomCacheMeshUpr m_meshUpr;
};

#endif
#endif
