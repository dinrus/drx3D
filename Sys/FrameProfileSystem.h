// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   frameprofilesystem.h
//  Version:     v1.00
//  Created:     24/6/2003 by Timur,Sergey,Wouter.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Sys/FrameProfiler.h>
#include <drx3D/Input/IInput.h>

#ifdef USE_FRAME_PROFILER

extern ColorF profile_colors[];

//////////////////////////////////////////////////////////////////////////
//! the system which does the gathering of stats
// unfortunately the IInput system does not work on win32 in the editor mode
// so this has to be ugly. can be removed as soon as input is handled in
// a unified way [mm]
class CFrameProfileSystem : public IFrameProfileSystem, public IInputEventListener
{
public:
	i32   m_nCurSample;

	tuk m_pPrefix;
	bool  m_bEnabled;
	//! True when collection must be paused.
	bool  m_bCollectionPaused;

	//! If set profiling data will be collected.
	bool                        m_bCollect;
	//! If set profiling data will be displayed.
	bool                        m_bDisplay;
	//! True if network profiling is enabled.
	bool                        m_bMemoryProfiling;
	//! Put memory info also in the log.
	bool                        m_bLogMemoryInfo;

	IRenderer*                  m_pRenderer;

	static CFrameProfileSystem* s_pFrameProfileSystem;
	static threadID             s_nFilterThreadId;

	// Cvars.
	i32   profile_graph;
	float profile_graphScale;
	i32   profile_pagefaults;
	i32   profile_network;
	i32   profile_additionalsub;
	float profile_peak;
	float profile_peak_display;
	float profile_min_display_ms;
	float profile_row, profile_col;
	i32   profile_meminfo;
	i32   profile_sampler_max_samples;
	i32   profile_callstack;
	i32   profile_log;

	//struct SPeakRecord
	//{
	//	CFrameProfiler *pProfiler;
	//	float peakValue;
	//	float averageValue;
	//	float variance;
	//	i32 pageFaults; // Number of page faults at this frame.
	//	i32 count;  // Number of times called for peak.
	//	float when; // when it added.
	//};
	struct SProfilerDisplayInfo
	{
		float           x, y; // Position where this profiler rendered.
		i32             averageCount;
		i32             level; // child level.
		CFrameProfiler* pProfiler;
	};
	struct SSubSystemInfo
	{
		tukk name = "";
		float       selfTime = 0;
		float       waitTime = 0;
		float       budgetTime = 0;
		float       maxTime = 0;
		i32         totalAnalized = 0;
		i32         totalOverBudget = 0;
	};

	EDisplayQuantity m_displayQuantity;

	//! When profiling frame started.
	int64 m_frameStartTime;
	//! Total time of profiling.
	int64 m_totalProfileTime;
	//! Frame time from the last frame.
	int64 m_frameTime;
	//! Ticks per profiling call, for adjustment.
	int64 m_callOverheadTime;
	int64 m_nCallOverheadTotal;
	int64 m_nCallOverheadRemainder;
	i32 m_nCallOverheadCalls;

	//! Smoothed version of frame time, lost time, and overhead.
	float                     m_frameSecAvg;
	float                     m_frameLostSecAvg;
	float                     m_frameOverheadSecAvg;

	DrxCriticalSection        m_profilersLock;
	static DrxCriticalSection m_staticProfilersLock;

	#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)

	ILINE void ValThreadFrameStatsCapacity(int8 numWorkers)
	{
		if (m_ThreadFrameStats->numWorkers == numWorkers)
			return;

		SAFE_DELETE(m_ThreadFrameStats);
		m_ThreadFrameStats = new JobUpr::CWorkerFrameStats(numWorkers);
	}

	ILINE void ValBlockingFrameStatsCapacity(int8 numWorkers)
	{
		if (m_BlockingFrameStats->numWorkers == numWorkers)
			return;

		SAFE_DELETE(m_BlockingFrameStats);
		m_BlockingFrameStats = new JobUpr::CWorkerFrameStats(numWorkers);
	}

	//Job frame stats
	JobUpr::CWorkerFrameStats*                              m_ThreadFrameStats;
	JobUpr::CWorkerFrameStats*                              m_BlockingFrameStats;

	JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer m_ThreadJobFrameStats;
	JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer m_BlockingJobFrameStats;

	JobUpr::IWorkerBackEndProfiler::TJobFrameStatsContainer* GetActiveFrameStats(JobUpr::EBackEndType backeEndType)
	{
		switch (backeEndType)
		{
		case JobUpr::eBET_Thread:
			return &m_ThreadJobFrameStats;
		case JobUpr::eBET_Blocking:
			return &m_BlockingJobFrameStats;
		default:
			DRX_ASSERT_MESSAGE(0, "Unsupported BackEndType encountered.");
		}
		;

		return 0;
	}

	u32k GetActiveFrameStatCount(JobUpr::EBackEndType backeEndType)
	{
		switch (backeEndType)
		{
		case JobUpr::eBET_Thread:
			return m_ThreadJobFrameStats.size();
		case JobUpr::eBET_Blocking:
			return m_BlockingJobFrameStats.size();
		default:
			DRX_ASSERT_MESSAGE(0, "Unsupported BackEndType encountered.");
		}
		;

		return 0;
	}

	#endif

	//! Maintain separate profiler stacks for each thread.
	//! Disallow any post-init allocation, to avoid profiler/allocator conflicts
	typedef threadID TThreadId;
	struct SProfilerThreads
	{
		SProfilerThreads(TThreadId nMainThreadId)
		{
			// First thread stack is for main thread.
			m_aThreadStacks[0].threadId = nMainThreadId;
			m_nThreadStacks = 1;
			m_pReservedProfilers = 0;
			m_lock = 0;
		}

		void            Reset();

		ILINE TThreadId GetMainThreadId() const
		{
			return m_aThreadStacks[0].threadId;
		}
		ILINE CFrameProfilerSection const* GetMainSection() const
		{
			return m_aThreadStacks[0].pProfilerSection;
		}

		static ILINE void Push(CFrameProfilerSection*& parent, CFrameProfilerSection* child)
		{
			assert(child);
			child->m_pParent = parent;
			parent = child;
		}

		ILINE void PushSection(CFrameProfilerSection* pSection, TThreadId threadId)
		{
			assert(threadId != TThreadId(0) && pSection);

			for (i32 i = 0, end = m_nThreadStacks; i < end; ++i)
			{
				if (m_aThreadStacks[i].threadId == threadId)
				{
					Push(m_aThreadStacks[i].pProfilerSection, pSection);
					return;
				}
			}

			// add new thread stack entry
			i32 newIndex = DrxInterlockedIncrement(( i32*)&m_nThreadStacks) - 1;

			if (newIndex == nMAX_THREADS)
				DrxFatalError("Profiled thread count of %d exceeded!", nMAX_THREADS);

			m_aThreadStacks[newIndex].threadId = threadId;
			Push(m_aThreadStacks[newIndex].pProfilerSection, pSection);
			return;
		}

		ILINE void PopSection(CFrameProfilerSection* pSection, TThreadId threadId)
		{
			// Thread-safe without locking.
			for (i32 i = 0; i < m_nThreadStacks; ++i)
				if (m_aThreadStacks[i].threadId == threadId)
				{
					assert(m_aThreadStacks[i].pProfilerSection == pSection || m_aThreadStacks[i].pProfilerSection == 0);
					m_aThreadStacks[i].pProfilerSection = pSection->m_pParent;
					return;
				}
			assert(0);
		}

		ILINE CFrameProfiler* GetThreadProfiler(CFrameProfiler* pMainProfiler, TThreadId nThreadId)
		{
			// Check main threads, or existing linked threads.
			if (nThreadId == GetMainThreadId())
				return pMainProfiler;
			for (CFrameProfiler* pProfiler = pMainProfiler->m_pNextThread; pProfiler; pProfiler = pProfiler->m_pNextThread)
				if (pProfiler->m_threadId == nThreadId)
					return pProfiler;
			return NewThreadProfiler(pMainProfiler, nThreadId);
		}

		ILINE void OnEnterSliceAndSleep(TThreadId nThreadId)
		{
			for (CFrameProfilerSection* pSection = m_aThreadStacks[m_sliceAndSleepThreadIdx].pProfilerSection; pSection; pSection = pSection->m_pParent)
			{
				if (pSection->m_pFrameProfiler)
				{
					CFrameProfileSystem::AccumulateProfilerSection(pSection);
				}
			}
		}

		ILINE void OnLeaveSliceAndSleep(TThreadId nThreadId)
		{
			int64 now = DrxGetTicks();

			for (CFrameProfilerSection* pSection = m_aThreadStacks[m_sliceAndSleepThreadIdx].pProfilerSection; pSection; pSection = pSection->m_pParent)
			{
				if (pSection->m_pFrameProfiler)
				{
					pSection->m_startTime = now;
					pSection->m_excludeTime = 0;
				}
			}
		}

		CFrameProfiler* NewThreadProfiler(CFrameProfiler* pMainProfiler, TThreadId nThreadId);

	protected:

		struct SThreadStack
		{
			TThreadId              threadId;
			CFrameProfilerSection* pProfilerSection;

			SThreadStack(TThreadId id = 0)
				: threadId(id), pProfilerSection(0)
			{}
		};
		static i32k nMAX_THREADS = 128;
		static i32k m_sliceAndSleepThreadIdx = 0;          // SLICE_AND_SLEEP and Statoscope tick are on main thread
		i32              m_nThreadStacks;
		SThreadStack     m_aThreadStacks[nMAX_THREADS];
		CFrameProfiler*  m_pReservedProfilers;
		 i32     m_lock;
	};
	SProfilerThreads        m_ProfilerThreads;
	CCustomProfilerSection* m_pCurrentCustomSection;

	typedef std::vector<CFrameProfiler*> Profilers;
	//! Array of all registered profilers.
	Profilers  m_profilers;
	//! Network profilers, they are not in regular list.
	Profilers  m_netTrafficProfilers;
	//! Currently active profilers array.
	Profilers* m_pProfilers;

	//! List of several latest peaks.
	std::vector<SPeakRecord>          m_peaks;
	std::vector<SPeakRecord>          m_absolutepeaks;
	std::vector<SProfilerDisplayInfo> m_displayedProfilers;
	bool                              m_bDisplayedProfilersValid;
	EProfiledSubsystem                m_subsystemFilter;
	bool                              m_bSubsystemFilterEnabled;
	i32                               m_maxProfileCount;

	//////////////////////////////////////////////////////////////////////////
	//! Smooth frame time in milliseconds.
	CFrameProfilerSamplesHistory<float, 32> m_frameTimeHistory;
	CFrameProfilerSamplesHistory<float, 32> m_frameTimeLostHistory;

	//////////////////////////////////////////////////////////////////////////
	// Graphs.
	//////////////////////////////////////////////////////////////////////////
	std::vector<u8> m_timeGraphPageFault;
	std::vector<u8> m_timeGraphFrameTime;
	#if defined(JOBMANAGER_SUPPORT_FRAMEPROFILER)

	std::vector<std::vector<u8>> m_timeGraphWorkers;
	i32 m_nWorkerGraphCurPos;

	#endif
	i32             m_timeGraphCurrentPos;
	CFrameProfiler* m_pGraphProfiler;

	//////////////////////////////////////////////////////////////////////////
	// Histograms.
	//////////////////////////////////////////////////////////////////////////
	bool  m_bEnableHistograms;
	i32   m_histogramsCurrPos;
	i32   m_histogramsMaxPos;
	i32   m_histogramsHeight;

	//////////////////////////////////////////////////////////////////////////
	// Selection/Render.
	//////////////////////////////////////////////////////////////////////////
	i32    m_selectedRow;
	float  ROW_SIZE, COL_SIZE;
	float  m_baseY;
	float  m_offset;
	i32    m_textModeBaseExtra;

	i32    m_nPagesFaultsLastFrame;
	i32    m_nPagesFaultsPerSec;
	int64  m_nLastPageFaultCount;

	string m_filterThreadName;

	//////////////////////////////////////////////////////////////////////////
	// Subsystems.
	//////////////////////////////////////////////////////////////////////////
	SSubSystemInfo               m_subsystems[PROFILE_LAST_SUBSYSTEM];

	CFrameProfilerOfflineHistory m_frameTimeOfflineHistory;

	//////////////////////////////////////////////////////////////////////////
	// Peak callbacks.
	//////////////////////////////////////////////////////////////////////////
	std::vector<IFrameProfilePeakCallback*> m_peakCallbacks;

	class CSampler*                         m_pSampler;

public:
	//////////////////////////////////////////////////////////////////////////
	// Methods.
	//////////////////////////////////////////////////////////////////////////
	CFrameProfileSystem();
	~CFrameProfileSystem();
	void Init();
	void Done();

	void SetProfiling(bool on, bool display, tuk prefix, ISystem* pSystem);

	//////////////////////////////////////////////////////////////////////////
	// IFrameProfileSystem interface implementation.
	//////////////////////////////////////////////////////////////////////////
	//! Reset all profiling data.
	void Reset();
	//! Add new frame profiler.
	//! Profile System will not delete those pointers, client must take care of memory management issues.
	void         AddFrameProfiler(CFrameProfiler* pProfiler);
	//! Remove existing frame profiler.
	virtual void RemoveFrameProfiler(CFrameProfiler* pProfiler);
	//! Must be called at the start of the frame.
	void         StartFrame();
	//! Must be called at the end of the frame.
	void         EndFrame();
	//! Must be called when something quacks like the end of the frame.
	void         OnSliceAndSleep();
	//! Get number of registered frame profilers.
	i32          GetProfilerCount() const { return (i32)m_profilers.size(); };
	//! Return the fraction used to blend current with average values.
	float        GetSmoothFactor() const;

	virtual i32  GetPeaksCount() const
	{
		return (i32)m_absolutepeaks.size();
	}

	virtual const SPeakRecord* GetPeak(i32 index) const
	{
		if (index >= 0 && index < (i32)m_absolutepeaks.size())
			return &m_absolutepeaks[index];
		return 0;
	}

	virtual float GetLostFrameTimeMS() const { return 0.0f; }

	//! Get frame profiler at specified index.
	//! @param index must be 0 <= index < GetProfileCount()
	CFrameProfiler*  GetProfiler(i32 index) const;
	inline TThreadId GetMainThreadId() const
	{
		return m_ProfilerThreads.GetMainThreadId();
	}

	//////////////////////////////////////////////////////////////////////////
	// Sampling related.
	//////////////////////////////////////////////////////////////////////////
	void StartSampling();

	//////////////////////////////////////////////////////////////////////////
	// Adds a value to profiler.
	virtual void StartCustomSection(CCustomProfilerSection* pSection);
	virtual void EndCustomSection(CCustomProfilerSection* pSection);

	//////////////////////////////////////////////////////////////////////////
	// Peak callbacks.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddPeaksListener(IFrameProfilePeakCallback* pPeakCallback);
	virtual void RemovePeaksListener(IFrameProfilePeakCallback* pPeakCallback);

	//////////////////////////////////////////////////////////////////////////
	//! Starts profiling a new section.
	static void StartProfilerSection(CFrameProfilerSection* pSection);
	//! Ends profiling a section.
	static void EndProfilerSection(CFrameProfilerSection* pSection);
	static void AccumulateProfilerSection(CFrameProfilerSection* pSection);

	static void StartMemoryProfilerSection(CFrameProfilerSection* pSection);
	static void EndMemoryProfilerSection(CFrameProfilerSection* pSection);

	//! Gets the bottom active section.
	virtual CFrameProfilerSection const* GetCurrentProfilerSection();

	//! Enable/Disable profile samples gathering.
	void         Enable(bool bCollect, bool bDisplay);
	void         SetSubsystemFilter(bool bFilterSubsystem, EProfiledSubsystem subsystem);
	bool         IsSubSystemFiltered(EProfiledSubsystem subsystem) const { return m_bSubsystemFilterEnabled && m_subsystemFilter != subsystem; }
	bool         IsSubSystemFiltered(CFrameProfiler* pProfiler) const    { return IsSubSystemFiltered((EProfiledSubsystem)pProfiler->m_subsystem); }
	void         EnableHistograms(bool bEnableHistograms);
	bool         IsEnabled() const   { return m_bEnabled; }
	virtual bool IsVisible() const   { return m_bDisplay; }
	bool         IsProfiling() const { return m_bCollect; }
	void         SetDisplayQuantity(EDisplayQuantity quantity);
	void         AddPeak(SPeakRecord& peak);

	void         SetSubsystemFilter(tukk sFilterName);
	void         SetSubsystemFilterThread(tukk sFilterThreadName);

	void         UpdateOfflineHistory(CFrameProfiler* pProfiler);

	//////////////////////////////////////////////////////////////////////////
	// Rendering.
	//////////////////////////////////////////////////////////////////////////
	void            Render();
	void            RenderMemoryInfo();
	void            RenderProfiler(CFrameProfiler* pProfiler, i32 level, float col, float row, bool bExtended, bool bSelected);
	void            RenderProfilerHeader(float col, float row, bool bExtended);
	void            RenderProfilers(float col, float row, bool bExtended);
	float           RenderPeaks();
	void            RenderSubSystems(float col, float row);
	void            RenderHistograms();
	void            CalcDisplayedProfilers();
	void            DrawGraph();
	void            DrawLabel(float raw, float column, float* fColor, float glow, tukk szText, float fScale = 1.0f);
	void            DrawRect(float x1, float y1, float x2, float y2, float* fColor);
	CFrameProfiler* GetSelectedProfiler();
	// Recursively add frame profiler and childs to displayed list.
	void            AddDisplayedProfiler(CFrameProfiler* pProfiler, i32 level);

	//////////////////////////////////////////////////////////////////////////
	float               TranslateToDisplayValue(int64 val);
	tukk         GetFullName(CFrameProfiler* pProfiler);
	virtual tukk GetModuleName(CFrameProfiler* pProfiler);
	tukk         GetModuleName(i32 num) const;
	i32k           GetModuleCount(void) const;
	const float         GetOverBudgetRatio(i32 modulenumber) const;

	//////////////////////////////////////////////////////////////////////////
	// Performance stats logging
	//////////////////////////////////////////////////////////////////////////

protected:
	//inherited from IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event);
	void         UpdateInputSystemStatus();
	tukk  GetProfilerThreadName(CFrameProfiler* pProfiler)  const;
};

#else

// Dummy Frame profile system interface.
struct CFrameProfileSystem : public IFrameProfileSystem
{
	//! Reset all profiling data.
	virtual void Reset() {};
	//! Add new frame profiler.
	//! Profile System will not delete those pointers, client must take care of memory managment issues.
	virtual void AddFrameProfiler(CFrameProfiler* pProfiler);
	//! Remove existing frame profiler.
	virtual void RemoveFrameProfiler(CFrameProfiler* pProfiler);
	//! Must be called at the start of the frame.
	virtual void StartFrame();
	//! Must be called at the end of the frame.
	virtual void EndFrame();
	//! Must be called when something quacks like the end of the frame.
	virtual void OnSliceAndSleep();

	//! Here the new methods needed to enable profiling to go off.

	virtual i32                          GetPeaksCount(void) const  { return 0; }
	virtual const SPeakRecord*           GetPeak(i32 index) const   { return 0; }
	virtual i32                          GetProfilerCount() const   { return 0; }

	virtual float                        GetLostFrameTimeMS() const { return 0.f; }

	virtual CFrameProfiler*              GetProfiler(i32 index) const;

	virtual void                         Enable(bool bCollect, bool bDisplay);

	virtual void                         SetSubsystemFilter(bool bFilterSubsystem, EProfiledSubsystem subsystem) {}
	virtual void                         SetSubsystemFilter(tukk sFilterName)                             {}
	virtual void                         SetSubsystemFilterThread(tukk sFilterThreadName)                 {}

	virtual bool                         IsEnabled() const                                                       { return 0; }

	virtual bool                         IsProfiling() const                                                     { return 0; }

	virtual void                         SetDisplayQuantity(EDisplayQuantity quantity)                           {}

	virtual void                         StartCustomSection(CCustomProfilerSection* pSection)                    {}
	virtual void                         EndCustomSection(CCustomProfilerSection* pSection)                      {}

	virtual void                         StartSampling(i32)                                                      {}

	virtual void                         AddPeaksListener(IFrameProfilePeakCallback* pPeakCallback);
	virtual void                         RemovePeaksListener(IFrameProfilePeakCallback* pPeakCallback);

	virtual tukk                  GetFullName(CFrameProfiler* pProfiler)     { return 0; }
	virtual i32k                    GetModuleCount(void) const                 { return 0; }
	virtual void                         SetPeakDisplayDuration(float duration)     {}
	virtual void                         SetAdditionalSubsystems(bool bEnabled)     {}
	virtual const float                  GetOverBudgetRatio(i32 modulenumber) const { return 0.0f; }

	void                                 Init()                   {}
	void                                 Done()                                     {}
	void                                 Render()                                   {}

	void                                 SetHistogramScale(float fScale)            {}
	void                                 SetDrawGraph(bool bDrawGraph)              {}
	void                                 SetNetworkProfiler(bool bNet)              {}
	void                                 SetPeakTolerance(float fPeakTimeMillis)    {}
	void                                 SetSmoothingTime(float fSmoothTime)        {}
	void                                 SetPageFaultsGraph(bool bEnabled)          {}
	void                                 SetThreadSupport(i32)                      {}

	virtual bool                         IsVisible() const                          { return true; }
	virtual const CFrameProfilerSection* GetCurrentProfilerSection()                { return NULL; }

	virtual tukk                  GetModuleName(CFrameProfiler* pProfiler)   { return 0; }
	tukk                          GetModuleName(i32 num) const               { return 0; }

protected:
	//inherited from IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event) { return false; }
	void         UpdateInputSystemStatus()              {}
};

#endif // USE_FRAME_PROFILER
