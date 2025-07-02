// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>

#include "FrameProfiler_Shared.h"
#include "FrameProfiler_Internal.h"
#include "FrameProfiler_Platform.h"
#include "FrameProfiler_Brofiler.h"

//////////////////////////////////////////////////////////////////////////
/*! This callback will be called for Profiling Peaks.
 */
struct IFrameProfilePeakCallback
{
	virtual ~IFrameProfilePeakCallback(){}
	//! Called when peak is detected for this profiler.
	//! @param fPeakTime peak time in milliseconds.
	virtual void OnFrameProfilerPeak(CFrameProfiler* pProfiler, float fPeakTime) = 0;
};

struct SPeakRecord
{
	CFrameProfiler* pProfiler;
	float           peakValue;
	float           averageValue;
	float           variance;
	i32             pageFaults; //!< Number of page faults at this frame.
	i32             count;      //!< Number of times called for peak.
	float           when;       //!< When it added.
	BYTE            waiting;    //!< If it needs to go in separate waiting peak list
};

//////////////////////////////////////////////////////////////////////////
//! IFrameProfileSystem interface - system which does the gathering of stats.
struct IFrameProfileSystem
{
	virtual ~IFrameProfileSystem(){}
	enum EDisplayQuantity
	{
		SELF_TIME,
		TOTAL_TIME,
		SELF_TIME_EXTENDED,
		TOTAL_TIME_EXTENDED,
		PEAK_TIME,
		SUBSYSTEM_INFO,
		COUNT_INFO,
		STANDARD_DEVIATION,
		ALLOCATED_MEMORY,
		ALLOCATED_MEMORY_BYTES,
		STALL_TIME,
		PEAKS_ONLY,
	};

	//! Reset all profiling data.
	virtual void Reset() = 0;

	//! Add new frame profiler.
	virtual void AddFrameProfiler(CFrameProfiler* pProfiler) = 0;

	//! Remove existing frame profiler.
	virtual void RemoveFrameProfiler(CFrameProfiler* pProfiler) = 0;

	//! Must be called at the start of the frame.
	virtual void StartFrame() = 0;

	//! Must be called at the end of the frame.
	virtual void EndFrame() = 0;

	//! Must be called when something quacks like the end of the frame.
	virtual void OnSliceAndSleep() = 0;

	//! Get number of registered frame profilers.
	virtual i32 GetProfilerCount() const = 0;

	//! Get frame profiler at specified index.
	//! \param index must be 0 <= index < GetProfileCount().
	virtual CFrameProfiler* GetProfiler(i32 index) const = 0;

	//! Get number of registered peak records.
	virtual i32 GetPeaksCount() const = 0;

	//! Get peak record at specified index.
	//! \param index must be 0 <= index < GetPeaksCount().
	virtual const SPeakRecord* GetPeak(i32 index) const = 0;

	//! Gets the bottom active section.
	virtual CFrameProfilerSection const* GetCurrentProfilerSection() = 0;

	//! get internal time lost to profiling.
	virtual float GetLostFrameTimeMS() const = 0;

	virtual void  Enable(bool bCollect, bool bDisplay) = 0;
	virtual void  SetSubsystemFilter(bool bFilterSubsystem, EProfiledSubsystem subsystem) = 0;

	//! True if profiler is turned off (even if collection is paused).
	virtual bool IsEnabled() const = 0;

	//! True if profiler statistics is visible.
	virtual bool IsVisible() const = 0;

	//! True if profiler must collect profiling data.
	virtual bool IsProfiling() const = 0;
	virtual void SetDisplayQuantity(EDisplayQuantity quantity) = 0;

	//! For custom frame profilers.
	virtual void StartCustomSection(CCustomProfilerSection* pSection) = 0;
	virtual void EndCustomSection(CCustomProfilerSection* pSection) = 0;

	//! Register peak listener callback to be called when peak value is greater then this.
	virtual void AddPeaksListener(IFrameProfilePeakCallback* pPeakCallback) = 0;
	virtual void RemovePeaksListener(IFrameProfilePeakCallback* pPeakCallback) = 0;

	//! Access to call stack string.
	virtual tukk GetFullName(CFrameProfiler* pProfiler) = 0;
	virtual tukk GetModuleName(CFrameProfiler* pProfiler) = 0;
};

//! CFrameProfilerSamplesHistory provides information on history of sample values for profiler counters.
template<class T, i32 TCount>
class CFrameProfilerSamplesHistory
{
public:
	CFrameProfilerSamplesHistory() : m_nHistoryNext(0), m_nHistoryCount(0) {}

	//! Add a new sample to history.
	void Add(T sample)
	{
		m_history[m_nHistoryNext] = sample;
		m_nHistoryNext = (m_nHistoryNext + TCount - 1) % TCount;
		if (m_nHistoryCount < TCount)
			++m_nHistoryCount;
	}
	//! Cleans up the data history.
	void Clear()
	{
		m_nHistoryNext = 0;
		m_nHistoryCount = 0;
	}
	//! Get last sample value.
	T GetLast()
	{
		if (m_nHistoryCount)
			return m_history[(m_nHistoryNext + 1) % TCount];
		else
			return 0;
	}
	//! Calculates average sample value for at most the given number of frames (less if so many unavailable).
	T GetAverage(i32 nCount = TCount)
	{
		if (m_nHistoryCount)
		{
			T fSum = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (i32 i = 1; i <= nCount; ++i)
			{
				fSum += m_history[(m_nHistoryNext + i) % (sizeof(m_history) / sizeof((m_history)[0]))];
			}
			return fSum / nCount;
		}
		else
			return 0;
	}
	//! Calculates average sample value for at most the given number of frames (less if so many unavailable),
	//! multiplied by the Poisson function.
	T GetAveragePoisson(i32 nCount, float fMultiplier)
	{
		if (m_nHistoryCount)
		{
			float fSum = 0, fCurrMult = 1, fSumWeight = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (i32 i = 1; i <= nCount; ++i)
			{
				fSum += m_history[(m_nHistoryNext + i) % TCount] * fCurrMult;
				fSumWeight += fCurrMult;
				fCurrMult *= fMultiplier;
			}
			return (T)(fSum / fSumWeight);
		}
		else
			return 0;
	}
	//! calculates Standard deviation of values.
	//! stdev = Sqrt( Sum((X-Xave)^2)/(n-1) ).
	T GetStdDeviation(i32 nCount = TCount)
	{
		if (m_nHistoryCount > 1)  // standard deviation is undefined for only one element
		{
			T fAve = GetAverage(nCount);
			T fVal = 0;
			T fSumVariance = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (i32 i = 1; i <= nCount; ++i)
			{
				fVal = m_history[(m_nHistoryNext + i) % (sizeof(m_history) / sizeof((m_history)[0]))];
				fSumVariance = (fVal - fAve) * (fVal - fAve); // (X-Xave)^2
			}
			return sqrtf(fSumVariance / (nCount - 1));
		}
		else
			return 0;
	}
	//! Calculates max sample value for at most the given number of frames (less if so many unavailable).
	T GetMax(i32 nCount = TCount)
	{
		if (m_nHistoryCount)
		{
			T fMax = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (i32 i = 1; i <= nCount; ++i)
			{
				T fCur = m_history[(m_nHistoryNext + i) % (sizeof(m_history) / sizeof((m_history)[0]))];
				if (i == 1 || fCur > fMax)
					fMax = fCur;
			}
			return fMax;
		}
		else
			return 0;
	}
	//! Calculates min sample value for at most the given number of frames (less if so many unavailable).
	T GetMin(i32 nCount = TCount)
	{
		if (m_nHistoryCount)
		{
			T fMin = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (i32 i = 1; i <= nCount; ++i)
			{
				T fCur = m_history[(m_nHistoryNext + i) % (sizeof(m_history) / sizeof((m_history)[0]))];
				if (i == 1 || fCur < fMin)
					fMin = fCur;
			}
			return fMin;
		}
		else
			return 0;
	}
protected:
	//! The timer values for the last frames.
	T m_history[TCount];

	//! The current pointer in the timer history, decreases.
	i32 m_nHistoryNext;

	//! The number of currently collected samples in the timer history.
	i32 m_nHistoryCount;

	// Adds the entry to the timer history (current timer value).
};

struct SCountTraits
{
	typedef u32 TValue;
	typedef u32 TDisplay;

	ILINE static TDisplay ToDisplay(TValue val) { return val; }
	ILINE static TDisplay ToDisplay(float val)  { return pos_round(val); }
};

struct STickTraits
{
	typedef int64 TValue;
	typedef float TDisplay;

	static TDisplay ToDisplay(TValue val) { return gEnv->pTimer->TicksToSeconds(val) * 1000.0f; }
	static TDisplay ToDisplay(float val)  { return val / (float)gEnv->pTimer->GetTicksPerSecond() * 1000.0f; }
};

template<class Traits>
class CSamplerHistory
{
public:
	using_type(Traits, TValue);
	using_type(Traits, TDisplay);

	//! Add a new sample to history.
	void Update(float blendCur)
	{
		float last = float(m_current);
		Blend(m_average, last, blendCur);
		Blend(m_deviationSqr, sqr(last - m_average), blendCur);
		Blend(m_minDecay, last, blendCur);
		if (last <= m_minDecay)
			m_minDecay = float(m_min = m_current);
		Blend(m_maxDecay, last, blendCur);
		if (last >= m_maxDecay)
			m_maxDecay = float(m_max = m_current);
	}
	//! Cleans up the data history.
	void Clear()                   { *this = {}; }

	operator TValue() const        { return m_current; }
	TValue& operator=(TValue val)  { return m_current = val; }
	TValue& operator+=(TValue val) { return m_current += val; }
	void operator++(i32)           { m_current++; }

	TValue Current() const         { return m_current; }
	TDisplay Last() const          { return Traits::ToDisplay(m_current); }
	TDisplay Average() const       { return Traits::ToDisplay(m_average); }
	TDisplay Variance() const      { return Traits::ToDisplay(sqrt(m_deviationSqr)); }
	TDisplay Max() const           { return Traits::ToDisplay(m_max); }
	TDisplay Min() const           { return Traits::ToDisplay(m_min); }

protected:
	TValue m_current      = 0;
	float  m_average      = 0,
           m_deviationSqr = 0,
           m_minDecay     = 0,
           m_maxDecay     = 0;
	TValue m_min          = 0,
           m_max          = 0;

	static void Blend(float& stat, float cur, float blendCur)
	{
		stat += (cur - stat) * blendCur;
	}
};

//////////////////////////////////////////////////////////////////////////
class CFrameProfilerGraph
{
public:
	i32                m_x;
	i32                m_y;
	i32                m_width;
	i32                m_height;
	std::vector<u8> m_data;
};

//////////////////////////////////////////////////////////////////////////
class CFrameProfilerOfflineHistory
{
public:
	//! Self time in microseconds.
	std::vector<u32> m_selfTime;
	//! Number of calls.
	std::vector<u16> m_count;
};

//! CFrameProfiler is a single profiler counter with unique name and data.
//! Multiple Sections can be executed for this profiler, they all will be merged in this class.
//! CFrameProfileSection must reference pointer to instance of this counter, to collect the sampling data.
class CFrameProfiler
{
public:
	ISystem*      m_pISystem;
	tukk   m_name;
	tukk   m_fileName;
	u64 m_fileLine;

	//! How many times this profiler counter was executed.
	CSamplerHistory<SCountTraits> m_count;

	//! Total time spent in this counter including time of child profilers in current frame.
	CSamplerHistory<STickTraits>  m_totalTime;

	//! Self frame time spent only in this counter (But includes recursive calls to same counter) in current frame.
	CSamplerHistory<STickTraits>  m_selfTime;
	
	//! Latest frame ID.
	uint64 m_latestFrame;
	
	//! Displayed quantity (interpolated or average).
	float  m_displayedValue;

	//! How variant this value.
	float m_variance;

	//! peak from this frame (useful if count is >1).
	int64 m_peak;

	//! Current parent profiler in last frame.
	CFrameProfiler* m_pParent;

	//! Expended or collapsed displaying state.
	bool m_bExpended;
	bool m_bHaveChildren;
	u8 m_subsystem;

	//! Identifier to be used for color lookup
	u8      m_colorIdentifier;

	tukk m_stallCause;     //!< Additional information of stall cause.

	//! Thread Id of this instance.
	threadID            m_threadId;
	uint                m_activeThreads;
	//! Linked list to other thread instances.
	CFrameProfiler*     m_pNextThread;

	EProfileDescription m_description;

#if ALLOW_BROFILER
	std::shared_ptr<::Profiler::EventDescription> m_brofilerEventDescription;
#endif

	//! Tells if this FrameProfiler has yet been added to the FrameProfileSystem.
	bool m_bInitialized;

	//! Graph data for this frame profiler.

	//! Graph associated with this profiler.
	CFrameProfilerGraph*          m_pGraph;
	CFrameProfilerOfflineHistory* m_pOfflineHistory;

	CFrameProfiler(const EProfiledSubsystem subsystem, const EProfileDescription desc,
	               tukk sCollectorName, tukk fileName, const u64 fileLine)
		: m_name(sCollectorName)
		, m_fileName(fileName)
		, m_fileLine(fileLine)
		, m_latestFrame(0)
		, m_displayedValue(0.0f)
		, m_variance(0.0f)
		, m_peak(0)
		, m_pParent(nullptr)
		, m_bExpended(false)
		, m_bHaveChildren(false)
		, m_subsystem((u8)subsystem)
		, m_colorIdentifier(0)
		, m_stallCause(nullptr)
		, m_threadId(0)
		, m_activeThreads(0)
		, m_pNextThread(nullptr)
		, m_description(desc)
		, m_bInitialized(false)
#if ALLOW_BROFILER
		, m_brofilerEventDescription(nullptr)
#endif
		, m_pGraph(nullptr)
		, m_pOfflineHistory(nullptr)
	{
		if (ISystem* const pSystem = GetISystem())
		{
			if (IFrameProfileSystem* const pFrameProfileSystem = pSystem->GetIProfileSystem())
			{
				pFrameProfileSystem->AddFrameProfiler(this);
			}
		}
	}

	~CFrameProfiler()
	{
		// This is needed for when modules get unloaded at runtime.
		if (ISystem* const pSystem = GetISystem())
		{
			if (IFrameProfileSystem* const pFrameProfileSystem = pSystem->GetIProfileSystem())
			{
				pFrameProfileSystem->RemoveFrameProfiler(this);
			}
		}
	}

	void Reset()
	{
		m_count = {};
		m_totalTime = {};
		m_selfTime = {};
		m_peak = 0;
		m_displayedValue = 0;
		m_variance = 0;
		m_activeThreads = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
//! CFrameProfilerSection is an auto class placed where code block need to be profiled.
//! Every time this object is constructed and destructed the time between constructor
//! and destructor is merged into the referenced CFrameProfiler instance.
//! For consoles, a faster non LHS triggering way is used.
//! (but potentially problematic with many threads when profiling is enabled at a very 'bad' time).
class CFrameProfilerSection
{
public:
	int64                  m_startTime;
	int64                  m_excludeTime;
	CFrameProfiler*        m_pFrameProfiler;
	CFrameProfilerSection* m_pParent;
	CBootProfilerRecord*   m_pRecord;
#if ALLOW_BROFILER
	::Profiler::EventData * m_brofilerEventData;
#endif

	ILINE CFrameProfilerSection(CFrameProfiler* profiler, tukk sectionName, tukk instanceArguments, EProfileDescription profileDescription)
		: m_startTime(0)
		, m_excludeTime(0)
		, m_pFrameProfiler(nullptr)
		, m_pParent(nullptr)
		, m_pRecord(nullptr)
#if ALLOW_BROFILER
		, m_brofilerEventData(nullptr)
#endif
	{
		m_pRecord = (gEnv->bBootProfilerEnabledFrames) ? gEnv->pSystem->StartBootSectionProfiler(sectionName, instanceArguments,profileDescription) : nullptr;
		if (gEnv->bDeepProfiling || profileDescription == EProfileDescription::REGION)
		{
			m_pFrameProfiler = profiler;
			gEnv->callbackStartSection(this);
		}
	}

	ILINE ~CFrameProfilerSection()
	{
		if (m_pFrameProfiler)
		{
			gEnv->callbackEndSection(this);
		}
		if (m_pRecord)
		{
			gEnv->pSystem->StopBootSectionProfiler(m_pRecord);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//! CCustomProfilerSection is an auto class placed where any custom data need to be profiled.
//! Works similarly to CFrameProfilerSection, but records any custom data, instead of elapsed time.
class DRX_ALIGN(32) CCustomProfilerSection
{
public:
	i32* m_pValue;
	i32 m_excludeValue;
	CFrameProfiler* m_pFrameProfiler;
	CCustomProfilerSection* m_pParent;

	//! pValue pointer must remain valid until after calling destructor of this custom profiler section.
	ILINE CCustomProfilerSection(CFrameProfiler* profiler, i32* pValue)
		: m_pValue(pValue)
		  , m_excludeValue(0)
		  , m_pFrameProfiler(profiler)
		  , m_pParent(nullptr)
	{
		if (ISystem* const pSystem = GetISystem())
		{
			pSystem->GetIProfileSystem()->StartCustomSection(this);
		}
	}
	ILINE ~CCustomProfilerSection()
	{
		if (ISystem* const pSystem = GetISystem())
		{
			pSystem->GetIProfileSystem()->EndCustomSection(this);
		}
	}
};

class CBootProfileFrameScope
{
public:
	CBootProfileFrameScope()
	{
		if (gEnv && gEnv->pSystem)
		{
			gEnv->pSystem->OnFrameStart("frames");
		}
	}
	~CBootProfileFrameScope()
	{
		if (gEnv && gEnv->pSystem)
		{
			gEnv->pSystem->OnFrameEnd();
		}
	}
};

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
// Profiling is enabled in every configuration except Release

	#define DRX_PROFILE_THREADNAME(szName) \
	  INTERNAL_PROFILER_THREADNAME(szName) \
	  BROFILER_THREADNAME(szName)          \
	  PLATFORM_PROFILER_THREADNAME(szName)

	#define DRX_PROFILE_FRAMESTART(szName) \
	  INTERNAL_PROFILER_FRAMESTART(szName) \
	  BROFILER_FRAMESTART(szName)          \
	  PLATFORM_PROFILER_FRAMESTART(szName)

	#define DRX_PROFILE_REGION(subsystem, szName) \
	  INTERNAL_PROFILER_REGION(subsystem, szName) \
	  BROFILER_REGION(szName)                     \
	  PLATFORM_PROFILER_REGION(szName)

	#define DRX_PROFILE_REGION_ARG(subsystem, szName, argument) \
	  INTERNAL_PROFILER_REGION_ARG(subsystem, szName, argument) \
	  BROFILER_REGION(szName)                                   \
	  PLATFORM_PROFILER_REGION(szName)

	#define DRX_PROFILE_REGION_WAITING(subsystem, szName) \
	  INTERNAL_PROFILER_REGION_WAITING(subsystem, szName) \
	  BROFILER_REGION_WAITING(szName)                     \
	  PLATFORM_PROFILER_REGION_WAITING(szName)

	#if ALLOW_DEEP_PROFILING
		#define DRX_PROFILE_FUNCTION(subsystem)           \
		  INTERNAL_PROFILER_FUNCTION(subsystem, __FUNC__) \
		  BROFILER_FUNCTION(__FUNC__)                     \
		  PLATFORM_PROFILER_FUNCTION(__FUNC__)

		#define DRX_PROFILE_FUNCTION_ARG(subsystem, argument)           \
		  INTERNAL_PROFILER_FUNCTION_ARG(subsystem, __FUNC__, argument) \
		  BROFILER_FUNCTION(__FUNC__)                                   \
		  PLATFORM_PROFILER_FUNCTION(__FUNC__)

		#define DRX_PROFILE_FUNCTION_WAITING(subsystem)           \
		  INTERNAL_PROFILER_FUNCTION_WAITING(subsystem, __FUNC__) \
		  BROFILER_FUNCTION_WAITING(__FUNC__)                     \
		  PLATFORM_PROFILER_FUNCTION_WAITING(__FUNC__)

		#define DRX_PROFILE_SECTION(subsystem, szName) \
		  INTERNAL_PROFILER_SECTION(subsystem, szName) \
		  BROFILER_SECTION(szName)                     \
		  PLATFORM_PROFILER_SECTION(szName)

		#define DRX_PROFILE_SECTION_WAITING(subsystem, szName) \
		  INTERNAL_PROFILER_SECTION_WAITING(subsystem, szName) \
		  BROFILER_SECTION_WAITING(szName)                     \
		  PLATFORM_PROFILER_SECTION_WAITING(szName)

		#define DRX_PROFILE_MARKER(szLabel) \
		  INTERNAL_PROFILER_MARKER(szLabel) \
		  BROFILER_MARKER(szLabel)          \
		  PLATFORM_PROFILER_MARKER(szLabel)


		#define DRX_PROFILE_PUSH_MARKER(szLabel) \
		  INTERNAL_PROFILER_PUSH(szLabel) \
		  BROFILER_PUSH(szLabel) \
		  PLATFORM_PROFILER_PUSH(szLabel)

		#define DRX_PROFILE_POP_MARKER(szLabel) \
		  INTERNAL_PROFILER_POP(szLabel) \
		  BROFILER_POP() \
		  PLATFORM_PROFILER_POP(szLabel)

	#else
		#define DRX_PROFILE_FUNCTION(subsystem)                /*do nothing*/
		#define DRX_PROFILE_FUNCTION_WAITING(subsystem)        /*do nothing*/
		#define DRX_PROFILE_SECTION(subsystem, szName)         /*do nothing*/
		#define DRX_PROFILE_SECTION_WAITING(subsystem, szName) /*do nothing*/
		#define DRX_PROFILE_MARKER(szName)                     /*do nothing*/
		#define DRX_PROFILE_PUSH_MARKER(szLabel)               /*do nothing*/
		#define DRX_PROFILE_POP_MARKER(szLabel)                /*do nothing*/
	#endif

#else
// Release builds do not have any kind of profiling information
	#define DRX_PROFILE_THREADNAME(szName)                      /*do nothing*/
	#define DRX_PROFILE_FRAMESTART(szName)                      /*do nothing*/
	#define DRX_PROFILE_REGION(subsystem, szName)               /*do nothing*/
	#define DRX_PROFILE_REGION_ARG(subsystem, szName, argument) /*do nothing*/
	#define DRX_PROFILE_REGION_WAITING(subsystem, szName)       /*do nothing*/
	#define DRX_PROFILE_FUNCTION(subsystem)                     /*do nothing*/
	#define DRX_PROFILE_FUNCTION_ARG(subsystem, argument)       /*do nothing*/
	#define DRX_PROFILE_FUNCTION_WAITING(subsystem)             /*do nothing*/
	#define DRX_PROFILE_SECTION(subsystem, szName)              /*do nothing*/
	#define DRX_PROFILE_SECTION_WAITING(subsystem, szName)      /*do nothing*/
	#define DRX_PROFILE_MARKER(szLabel)                         /*do nothing*/
	#define DRX_PROFILE_PUSH_MARKER(szLabel)                    /*do nothing*/
	#define DRX_PROFILE_POP_MARKER(szLabel)                     /*do nothing*/
#endif

#if defined(ENABLE_LOADING_PROFILER)

struct DiskOperationInfo
{
	DiskOperationInfo() : m_nSeeksCount(0), m_nFileOpenCount(0), m_nFileReadCount(0), m_dOperationTime(0.), m_dOperationSize(0.) {}
	i32    m_nSeeksCount;
	i32    m_nFileOpenCount;
	i32    m_nFileReadCount;
	double m_dOperationTime;
	double m_dOperationSize;

	DiskOperationInfo& operator-=(const DiskOperationInfo& rv)
	{
		m_nSeeksCount -= rv.m_nSeeksCount;
		m_nFileOpenCount -= rv.m_nFileOpenCount;
		m_nFileReadCount -= rv.m_nFileReadCount;
		m_dOperationSize -= rv.m_dOperationSize;
		m_dOperationTime -= rv.m_dOperationTime;
		return *this;
	}

	DiskOperationInfo& operator+=(const DiskOperationInfo& rv)
	{
		m_nSeeksCount += rv.m_nSeeksCount;
		m_nFileOpenCount += rv.m_nFileOpenCount;
		m_nFileReadCount += rv.m_nFileReadCount;
		m_dOperationSize += rv.m_dOperationSize;
		m_dOperationTime += rv.m_dOperationTime;
		return *this;
	}

	DiskOperationInfo operator-(const DiskOperationInfo& rv)
	{
		DiskOperationInfo res(*this);
		return res -= rv;
	}

	DiskOperationInfo operator+(const DiskOperationInfo& rv)
	{
		DiskOperationInfo res(*this);
		return res += rv;
	}

};

struct CLoadingTimeProfiler
{
	CLoadingTimeProfiler(ISystem* pSystem, tukk szFuncName) : m_pSystem(pSystem)
	{
		m_pSystem = pSystem;
		m_pTimeContainer = m_pSystem->StartLoadingSectionProfiling(this, szFuncName);
	}

	~CLoadingTimeProfiler()
	{
		m_pSystem->EndLoadingSectionProfiling(this);
	}

	struct SLoadingTimeContainer* m_pTimeContainer;
	double                        m_fConstructorTime;
	double                        m_fConstructorMemUsage;

	DiskOperationInfo             m_constructorInfo;

	ISystem*                      m_pSystem;
};

class CSYSBootProfileBlock
{
	ISystem*             m_pSystem;
	CBootProfilerRecord* m_pRecord;
public:
	CSYSBootProfileBlock(ISystem* pSystem, tukk name, tukk args = NULL) : m_pSystem(pSystem)
	{
		m_pRecord = m_pSystem ? m_pSystem->StartBootSectionProfiler(name, args,EProfileDescription::FUNCTIONENTRY) : nullptr;
	}

	~CSYSBootProfileBlock()
	{
		if (m_pRecord)
		{
			m_pSystem->StopBootSectionProfiler(m_pRecord);
		}
	}
};

class CSYSBootProfileAutoSession
{
	ISystem*    m_pSystem;
	tukk m_szSessionName;
public:
	CSYSBootProfileAutoSession(ISystem* pSystem, tukk szSessionName)
		: m_pSystem(pSystem), m_szSessionName(szSessionName)
	{
		m_pSystem->StartBootProfilerSession(m_szSessionName);
	}

	~CSYSBootProfileAutoSession()
	{
		m_pSystem->StopBootProfilerSession(m_szSessionName);
	}
};

// DRX_PROFILE_FUNCTION(PROFILE_LOADING_ONLY);
#define LOADING_TIME_PROFILE_SECTION \
	static CFrameProfiler staticFrameProfilerLoading(PROFILE_LOADING_ONLY, EProfileDescription::FUNCTIONENTRY, __FUNC__, __FILE__, __LINE__); \
	CFrameProfilerSection frameProfilerSectionLoading(&staticFrameProfilerLoading, __FILE__, nullptr, EProfileDescription::FUNCTIONENTRY);

#define LOADING_TIME_PROFILE_SECTION_ARGS(args)                    CSYSBootProfileBlock _profileBlockLine_args(gEnv->pSystem, __FUNC__, args);
#define LOADING_TIME_PROFILE_SECTION_NAMED(sectionName)            CSYSBootProfileBlock _profileBlockLine_named(gEnv->pSystem, sectionName);
#define LOADING_TIME_PROFILE_SECTION_NAMED_ARGS(sectionName, args) CSYSBootProfileBlock _profileBlockLine_named_args(gEnv->pSystem, sectionName, args);
#define LOADING_TIME_PROFILE_AUTO_SESSION(sessionName)             CSYSBootProfileAutoSession _profileAutoSession(gEnv->pSystem, (sessionName));


#else //ENABLE_LOADING_PROFILER

#define LOADING_TIME_PROFILE_SECTION
#define LOADING_TIME_PROFILE_SECTION_ARGS(args)
#define LOADING_TIME_PROFILE_SECTION_NAMED(sectionName)
#define LOADING_TIME_PROFILE_SECTION_NAMED_ARGS(sectionName, args)
#define LOADING_TIME_PROFILE_AUTO_SESSION(sessionName)

#endif ////ENABLE_LOADING_PROFILER

//! \endcond