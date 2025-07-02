// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Stroboscope.h
//  Version:     v1.00
//  Created:     17/6/2012 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __Stroboscope_H__
#define __Stroboscope_H__

#if defined(ENABLE_PROFILING_CODE)

	#include <drx3D/Sys/ThreadInfo.h>
	#include <drx3D/CoreX/Thread/IThreadUpr.h>

	#define MAX_CALLSTACK_DEPTH 256

struct SStrobosopeSamplingData
{
	struct SCallstackSampling
	{
		// Note: Stack not initialized on purpose, accessed items guarded by Depth
		// cppcheck-suppress uninitMemberVar
		SCallstackSampling() : Depth(0) {};
		bool operator<(const SCallstackSampling& other) const;

		uk Stack[MAX_CALLSTACK_DEPTH];
		i32   Depth;
	};
	typedef std::map<SCallstackSampling, float> TCallstacks;
	typedef std::map<i32, TCallstacks>          TFrameCallstacks;
	typedef std::map<i32, float>                TFrameTime;

	struct SThreadResult
	{

		TFrameCallstacks Callstacks;
		i32              Samples;
		string           Name;
	};
	typedef std::map<u32, SThreadResult> TThreadResult;

	bool          Valid;
	float         Duration;
	TThreadResult Threads;
	TFrameTime    FrameTime;
	i32           StartFrame;
	i32           EndFrame;

	void          Clear()
	{
		Valid = false;
		Duration = 0;
		StartFrame = 0;
		EndFrame = 0;
		Threads.clear();
		FrameTime.clear();
	}
};

struct SStrobosopeResult
{
	struct SSymbolInfo
	{
		string Module;
		string Procname;
		string File;
		i32    Line;
		uk  BaseAddr;
	};
	typedef std::map<i32, SSymbolInfo> TSymbolInfo;

	struct SCallstackInfo
	{
		float    Spend;
		typedef std::vector<i32> TSymbols;
		TSymbols Symbols;
		u32   ThreadId;
		i32      FrameId;
	};
	typedef std::vector<SCallstackInfo> TCallstackInfo;

	struct SThreadInfo
	{
		u32 Id;
		string Name;
		i32    Samples;
		float  Counts;
	};
	typedef std::vector<SThreadInfo> TThreadInfo;
	typedef std::map<i32, float>     TFrameTime;

	SStrobosopeResult()
		: Valid(false)
		, Duration(0.0f)
		, Samples(0)
		, TotalCounts(0.0f)
		, StartFrame(0)
		, EndFrame(0)
	{}

	bool           Valid;
	float          Duration;
	string         File;
	string         Date;
	i32            Samples;
	float          TotalCounts;
	i32            StartFrame;
	i32            EndFrame;
	TSymbolInfo    SymbolInfo;
	TCallstackInfo CallstackInfo;
	TThreadInfo    ThreadInfo;
	TFrameTime     FrameTime;

	void           Clear()
	{
		Valid = false;
		Duration = 0;
		File = "";
		Date = "";
		Samples = 0;
		TotalCounts = 0;
		StartFrame = 0;
		EndFrame = 0;
		FrameTime.clear();
		SymbolInfo.clear();
		CallstackInfo.clear();
		ThreadInfo.clear();
	}
};

class CStroboscope : public IThread
{
public:
	static CStroboscope*     GetInst() { static CStroboscope inst; return &inst; }

	void                     RegisterCommands();

	void                     StartProfiling(float deltaStart = 0, float duration = -1, i32 throttle = 100, const SThreadInfo::TThreadIds threadIds = SThreadInfo::TThreadIds());
	void                     StopProfiling();
	const SStrobosopeResult& GetResult() { UpdateResult(); return m_result; }

	// Start accepting work on thread
	virtual void ThreadEntry();

	static void  StartProfilerCmd(IConsoleCmdArgs* pArgs);
	static void  StopProfilerCmd(IConsoleCmdArgs* pArgs);
	static void  DumpResultsCmd(IConsoleCmdArgs* pArgs);
private:
	CStroboscope();
	~CStroboscope();

	void ProfileThreads();
	bool SampleThreads(const SThreadInfo::TThreads& threads, float delta, i32 frameId);
	bool SampleThread(const SThreadInfo::SThreadHandle& thread, float delta, i32 frameId);

	void UpdateResult();
	void DumpOutputResult();

private:
	float                   m_startTime;
	float                   m_endTime;
	i32                     m_throttle;
	SThreadInfo::TThreadIds m_threadIds;
	 LONG           m_started;

	SStrobosopeSamplingData m_sampling;
	SStrobosopeResult       m_result;

	bool                    m_run;
};

#endif //#if defined(ENABLE_PROFILING_CODE)

#endif //#ifndef __Stroboscope_H__
