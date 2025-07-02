// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef STATOSCOPERENDERSTATS_H
#define STATOSCOPERENDERSTATS_H

#if ENABLE_STATOSCOPE
class CD3D9Renderer;

class CGPUTimesDG : public IStatoscopeDataGroup
{
public:
	CGPUTimesDG(CD3D9Renderer* pRenderer);

	virtual SDescription GetDescription() const;
	virtual void         Enable();
	virtual void         Write(IStatoscopeFrameRecord& fr);

protected:
	CD3D9Renderer* m_pRenderer;
};

class CDetailedRenderTimesDG : public IStatoscopeDataGroup
{
public:
	CDetailedRenderTimesDG(CD3D9Renderer* pRenderer);

	virtual SDescription GetDescription() const;
	virtual void         Enable();
	virtual void         Write(IStatoscopeFrameRecord& fr);
	virtual u32       PrepareToWrite();

protected:
	CD3D9Renderer* m_pRenderer;
	const DynArray<RPProfilerDetailedStats>* m_stats;
};

class CGraphicsDG : public IStatoscopeDataGroup
{
public:
	CGraphicsDG(CD3D9Renderer* pRenderer);

	virtual SDescription GetDescription() const;
	virtual void         Write(IStatoscopeFrameRecord& fr);
	virtual void         Enable();

protected:
	void ResetGPUUsageHistory();
	void TrackGPUUsage(float gpuLoad, float frameTimeMs, i32 totalDPs);

	static i32k   GPU_HISTORY_LENGTH = 10;
	static i32k   SCREEN_SHOT_FREQ = 60;
	static i32k   GPU_TIME_THRESHOLD_MS = 40;
	static i32k   DP_THRESHOLD = 2200;

	CD3D9Renderer*     m_pRenderer;
	std::vector<float> m_gpuUsageHistory;
	std::vector<float> m_frameTimeHistory;
	i32                m_nFramesGPULmited;
	float              m_totFrameTime;
	i32                m_lastFrameScreenShotRequested;
	i32                m_cvarScreenCapWhenGPULimited;
};

class CPerformanceOverviewDG : public IStatoscopeDataGroup
{
public:
	CPerformanceOverviewDG(CD3D9Renderer* pRenderer);

	virtual SDescription GetDescription() const;
	virtual void         Write(IStatoscopeFrameRecord& fr);
	virtual void         Enable();

protected:
	CD3D9Renderer* m_pRenderer;
};

#endif // ENABLE_STATOSCOPE

#endif // STATOSCOPERENDERSTATS_H
