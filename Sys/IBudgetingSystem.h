// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _I_BUDGETING_SYSTEM_
#define _I_BUDGETING_SYSTEM_

struct IBudgetingSystem
{
	// <interfuscator:shuffle>
	// set budget
	virtual void SetSysMemLimit(i32 sysMemLimitInMB) = 0;
	virtual void SetVideoMemLimit(i32 videoMemLimitInMB) = 0;
	virtual void SetFrameTimeLimit(float frameLimitInMS) = 0;
	virtual void SetSoundChannelsPlayingLimit(i32 soundChannelsPlayingLimit) = 0;
	virtual void SetSoundMemLimit(i32 SoundMemLimit) = 0;
	virtual void SetSoundCPULimit(i32 SoundCPULimit) = 0;
	virtual void SetNumDrawCallsLimit(i32 numDrawCallsLimit) = 0;
	virtual void SetStreamingThroughputLimit(float streamingThroughputLimit) = 0;
	virtual void SetBudget(i32 sysMemLimitInMB, i32 videoMemLimitInMB, float frameTimeLimitInMS, i32 soundChannelsPlayingLimit, i32 SoundMemLimitInMB, i32 SoundCPULimit, i32 numDrawCallsLimit) = 0;

	// get budget
	virtual i32   GetSysMemLimit() const = 0;
	virtual i32   GetVideoMemLimit() const = 0;
	virtual float GetFrameTimeLimit() const = 0;
	virtual i32   GetSoundChannelsPlayingLimit() const = 0;
	virtual i32   GetSoundMemLimit() const = 0;
	virtual i32   GetSoundCPULimit() const = 0;
	virtual i32   GetNumDrawCallsLimit() const = 0;
	virtual float GetStreamingThroughputLimit() const = 0;
	virtual void  GetBudget(i32& sysMemLimitInMB, i32& videoMemLimitInMB, float& frameTimeLimitInMS, i32& soundChannelsPlayingLimit, i32& SoundMemLimitInMB, i32& SoundCPULimitInPercent, i32& numDrawCallsLimit) const = 0;

	// monitoring
	virtual void MonitorBudget() = 0;
	virtual void Render(float x, float y) = 0;

	// destruction
	virtual void Release() = 0;
	// </interfuscator:shuffle>

protected:
	virtual ~IBudgetingSystem() {}
};

#endif // #ifndef _I_BUDGETING_SYSTEM_
