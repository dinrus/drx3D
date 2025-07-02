// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/DeviceUpr/DeviceObjects.h>

class CRenderPipelineProfiler
{
public:
	enum EProfileSectionFlags
	{
		eProfileSectionFlags_RootElement = BIT(0),
		eProfileSectionFlags_MultithreadedSection = BIT(1)
	};
	
	CRenderPipelineProfiler();

	void                   Init();
	void                   BeginFrame(i32k frameID);
	void                   EndFrame();
	void                   BeginSection(tukk name);
	void                   EndSection(tukk name);

	u32                 InsertMultithreadedSection(tukk name);
	void                   UpdateMultithreadedSection(u32 index, bool bSectionStart, i32 numDIPs, i32 numPolys, bool bIssueTimestamp, CTimeValue deltaTimestamp, CDeviceCommandList* pCommandList);
	
	bool                   IsEnabled();
	void                   SetEnabled(bool enabled)                                        { m_enabled = enabled; }

	const RPProfilerStats&                   GetBasicStats(ERenderPipelineProfilerStats stat, i32 nThreadID) { assert((u32)stat < RPPSTATS_NUM); return m_basicStats[nThreadID][stat]; }
	const RPProfilerStats*                   GetBasicStatsArray(i32 nThreadID)                               { return m_basicStats[nThreadID]; }
	const DynArray<RPProfilerDetailedStats>* GetDetailedStatsArray(i32 nThreadID)                            { return &m_detailedStats[nThreadID]; }

protected:
	struct SProfilerSection
	{
		char            name[31]; 
		float           gpuTime;
		float           gpuTimeSmoothed;
		float           cpuTime;
		float           cpuTimeSmoothed;
		CTimeValue      startTimeCPU, endTimeCPU;
		u32          startTimestamp, endTimestamp;
		CDrxNameTSCRC   path;
		i32             numDIPs, numPolys;		
		int8            recLevel;   // Negative value means error in stack
		u8           flags;
	};

	struct SFrameData
	{
		enum { kMaxNumSections = CDeviceTimestampGroup::kMaxTimestamps / 2 };

		u32                  m_numSections;
		i32                     m_frameID;
		SProfilerSection        m_sections[kMaxNumSections];
		CDeviceTimestampGroup   m_timestampGroup;
		bool                    m_updated;

		float fTimeRealFrameTime;
		float fTimeWaitForMain;
		float fTimeWaitForRender;
		float fTimeProcessedRT;
		float fTimeProcessedGPU;
		float fTimeWaitForGPU;
		float fTimeGPUIdlePercent;

		// cppcheck-suppress uninitMemberVar
		SFrameData()
			: m_numSections(0)
			, m_frameID(0)
			, m_updated(false)
		{
			memset(m_sections, 0, sizeof(m_sections));
		}
	};

	struct SThreadTimings
	{
		float waitForMain;
		float waitForRender;
		float waitForGPU;
		float gpuIdlePerc;
		float gpuFrameTime;
		float frameTime;
		float renderTime;

		SThreadTimings()
			: waitForMain(0)
			, waitForRender(0)
			, waitForGPU(0)
			, gpuIdlePerc(0)
			, gpuFrameTime(33.0f / 1000.0f)
			, frameTime(33.0f / 1000.0f)
			, renderTime(0)
		{
		}
	};

	struct SStaticElementInfo
	{
		uint nPos;
		bool bUsed;

		SStaticElementInfo(uint _nPos)
			: nPos(_nPos)
			, bUsed(false)
		{
		}
	};

protected:
	u32 InsertSection(tukk name, u32 profileSectionFlags = 0);
	
	bool FilterLabel(tukk name);
	void UpdateSectionTimesAndStats(u32 frameDataIndex);
	void UpdateThreadTimings(u32 frameDataIndex);
	
	void ResetBasicStats(RPProfilerStats* pBasicStats, bool bResetAveragedStats);
	void ResetDetailedStats(DynArray<RPProfilerDetailedStats>& pDetailedStats, bool bResetAveragedStats);
	void ComputeAverageStats(SFrameData& currData, SFrameData& prevData);
	void AddToStats(RPProfilerStats& outStats, SProfilerSection& section);
	void SubtractFromStats(RPProfilerStats& outStats, SProfilerSection& section);
	void UpdateStats(u32 frameDataIndex);

	void DisplayOverviewStats(u32 frameDataIndex);
	void DisplayDetailedPassStats(u32 frameDataIndex);

private:
	SProfilerSection& FindSection(SFrameData& frameData, SProfilerSection& section);

protected:
	enum { kNumPendingFrames = MAX_FRAMES_IN_FLIGHT };

	std::vector<u32>               m_stack;
	SFrameData                        m_frameData[kNumPendingFrames];
	u32                            m_frameDataIndex;
	SFrameData*                       m_frameDataRT;
	SFrameData*                       m_frameDataLRU;
	float                             m_avgFrameTime;
	bool                              m_enabled;
	bool                              m_recordData;

	RPProfilerStats                   m_basicStats[RT_COMMAND_BUF_COUNT][RPPSTATS_NUM];
	DynArray<RPProfilerDetailedStats> m_detailedStats[RT_COMMAND_BUF_COUNT];
	SThreadTimings                    m_frameTimings[RT_COMMAND_BUF_COUNT];

	// we take a snapshot every now and then and store it in here to prevent the text from jumping too much
	std::multimap<CDrxNameTSCRC, SStaticElementInfo> m_staticNameList;
};
