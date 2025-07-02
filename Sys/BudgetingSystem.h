// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IBudgetingSystem.h>

struct IRenderer;
struct IRenderAuxGeom;
struct ITimer;
namespace DrxAudio
{
struct IAudioSystem;
}
struct IStreamEngine;

class CBudgetingSystem : public IBudgetingSystem
{
public:
	CBudgetingSystem();

	// interface
	virtual void SetSysMemLimit(i32 sysMemLimitInMB);
	virtual void SetVideoMemLimit(i32 videoMemLimitInMB);
	virtual void SetFrameTimeLimit(float frameLimitInMS);
	virtual void SetSoundChannelsPlayingLimit(i32 soundChannelsPlayingLimit);
	virtual void SetSoundMemLimit(i32 SoundMemLimit);
	virtual void SetSoundCPULimit(i32 SoundCPULimit);
	virtual void SetNumDrawCallsLimit(i32 numDrawCallsLimit);
	virtual void SetStreamingThroughputLimit(float streamingThroughputLimit);
	virtual void SetBudget(i32 sysMemLimitInMB, i32 videoMemLimitInMB,
	                       float frameTimeLimitInMS, i32 soundChannelsPlayingLimit, i32 SoundMemLimitInMB, i32 SoundCPULimit, i32 numDrawCallsLimit);

	virtual i32   GetSysMemLimit() const;
	virtual i32   GetVideoMemLimit() const;
	virtual float GetFrameTimeLimit() const;
	virtual i32   GetSoundChannelsPlayingLimit() const;
	virtual i32   GetSoundMemLimit() const;
	virtual i32   GetSoundCPULimit() const;
	virtual i32   GetNumDrawCallsLimit() const;
	virtual float GetStreamingThroughputLimit() const;
	virtual void  GetBudget(i32& sysMemLimitInMB, i32& videoMemLimitInMB,
	                        float& frameTimeLimitInMS, i32& soundChannelsPlayingLimit, i32& SoundMemLimitInMB, i32& SoundCPULimitInPercent, i32& numDrawCallsLimit) const;

	virtual void MonitorBudget();
	virtual void Render(float x, float y);

	virtual void Release();

protected:
	virtual ~CBudgetingSystem();

	void DrawText(float& x, float& y, float* pColor, tukk format, ...) PRINTF_PARAMS(5, 6);

	void MonitorSystemMemory(float& x, float& y);
	void MonitorVideoMemory(float& x, float& y);
	void MonitorFrameTime(float& x, float& y);
	void MonitorSoundChannels(float& x, float& y);
	void MonitorSoundMemory(float& x, float& y);
	void MonitorSoundCPU(float& x, float& y);
	void MonitorDrawCalls(float& x, float& y);
	void MonitorPolyCount(float& x, float& y);
	void MonitorStreaming(float& x, float& y);

	void GetColor(float scale, float* pColor);
	void DrawMeter(float& x, float& y, float scale);

	void RegisterWithPerfHUD();

	//static perfhud render callback
	static void PerfHudRender(float x, float y);

protected:
	IRenderer*              m_pRenderer;
	IRenderAuxGeom*         m_pAuxRenderer;
	ITimer*                 m_pTimer;
	DrxAudio::IAudioSystem* m_pIAudioSystem;
	IStreamEngine*          m_pStreamEngine;

	i32                     m_sysMemLimitInMB;
	i32                     m_videoMemLimitInMB;
	float                   m_frameTimeLimitInMS;
	i32                     m_soundChannelsPlayingLimit;
	i32                     m_soundMemLimitInMB;
	i32                     m_soundCPULimitInPercent;
	i32                     m_numDrawCallsLimit;
	i32                     m_numPolysLimit;
	float                   m_streamingThroughputLimit;
	i32                     m_width;
	i32                     m_height;
};
