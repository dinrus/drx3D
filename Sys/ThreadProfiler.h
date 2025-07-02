// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ThreadProfiler.h
//  Version:     v1.00
//  Created:     24/6/2003 by Timur
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ThreadProfiler_h__
#define __ThreadProfiler_h__
#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#if defined(USE_FRAME_PROFILER) && DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
	#define THREAD_SAMPLER
#endif

#ifdef THREAD_SAMPLER

enum ETraceThreads
{
	TT_SYSTEM_IDLE_0,
	TT_SYSTEM_IDLE_1,
	TT_MAIN,
	TT_RENDER,
	TT_OTHER,
	TT_TOTAL, // total = other + render + ... + sys idle 0

	TT_NUM  // must be at the end
};

// Pixels span used to display data.
struct SThreadDisplaySpan
{
	u16 start;
	u16 end;
};

typedef std::vector<SThreadDisplaySpan> TTDSpanList;
typedef u32                          TProcessID;

enum { INVALID_THREADID = -1 };

struct SSnapshotInfo
{
	SSnapshotInfo(i32 numProcessors) : m_procNumCtxtSwitches(numProcessors) {}

	void Reset()
	{
		for (u32 i = 0, c = m_procNumCtxtSwitches.size(); i < c; i++)
			m_procNumCtxtSwitches[i] = 0;
	}

	std::vector<i32> m_procNumCtxtSwitches; // number of context switches per processor
};

class IThreadSampler
{
public:
	virtual ~IThreadSampler() {}

	virtual void                 EnumerateThreads(TProcessID processId) = 0;
	virtual i32                  GetNumHWThreads() = 0;    // hardware threads
	virtual i32                  GetNumThreads() = 0;      // software threads
	virtual threadID             GetThreadId(i32 idx) = 0; // 0 <= idx < GetNumThreads()
	virtual tukk          GetThreadName(threadID threadId) = 0;
	virtual float                GetExecutionTimeFrame() = 0;            // the time duration of the last tick
	virtual float                GetExecutionTime(ETraceThreads tt) = 0; // the on CPU time for tt

	virtual void                 Tick() = 0;
	virtual const SSnapshotInfo* GetSnapshot() = 0;

	// Create span list for drawing graph
	// processId, threadId - build list for specified thread.
	// threadId = OTHER_THREADS - build list for threads not belonging to specified process
	// width - width of timeline, in pixels
	// scale - scale of timeline, in 0.5 seconds units, f.e 2 for 1-second timeline. 4 for 2 seconds timeline etc.
	// processorId - index of the hardware processor thread (or core)
	// totalTime - receives total time in milliseconds, used by this thread during one second (does not depend on 'scale' parameter).
	virtual void CreateSpanListForThread(TProcessID processId, threadID threadId,
	                                     TTDSpanList& spanList,
	                                     u32 width, u32 scale,
	                                     u32* totalTime, i32* processorId, u32* color) = 0;
};

//////////////////////////////////////////////////////////////////////////
//! the system which does the gathering of stats
class CThreadProfiler
{
public:
	CThreadProfiler();
	~CThreadProfiler();

	void            Start();
	void            Stop();
	void            Render();
	IThreadSampler* GetThreadSampler() { return m_pSampler; }

private:
	bool               m_bRenderEnabled;
	i32                m_nUsers;
	std::vector<float> m_lastActive;
	i32                m_nDisplayedThreads;

	IThreadSampler*    m_pSampler;
};

#else // THREAD_SAMPLER

class IThreadSampler;

class CThreadProfiler
{
public:
	void            Start()            {};
	void            Stop()             {};
	void            Render()           {};
	IThreadSampler* GetThreadSampler() { return NULL; }
};

#endif

#endif // __ThreadProfiler_h__
