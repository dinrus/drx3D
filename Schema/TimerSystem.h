// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// Restarting random timers will not affect duration. Is this an issue?

#pragma once

#include <drx3D/Schema/ITimerSystem.h>
#include <drx3D/Schema/EnumFlags.h>

namespace sxema
{
enum class EPrivateTimerFlags
{
	None    = 0,
	Active  = BIT(0),
	Repeat  = BIT(1),
	Destroy = BIT(2)
};

typedef CEnumFlags<EPrivateTimerFlags> PrivateTimerFlags;

class CTimerSystem : public ITimerSystem
{
private:

	struct STimer
	{
		STimer();
		STimer(int64 _time, const TimerId& _timerId, const STimerDuration& _duration, const TimerCallback& _callback, const PrivateTimerFlags& _privateFlags);
		STimer(const STimer& rhs);

		int64             time;
		TimerId           timerId;
		STimerDuration    duration;
		TimerCallback     callback;
		PrivateTimerFlags privateFlags;
	};

	struct SEqualTimerId
	{
		inline SEqualTimerId(const TimerId& _timerId)
			: timerId(_timerId)
		{}

		inline bool operator()(const STimer& timer) const
		{
			return timer.timerId == timerId;
		}

		TimerId timerId;
	};

	typedef std::vector<STimer> Timers;

public:

	CTimerSystem();

	// ITimerSystem
	virtual TimerId        CreateTimer(const STimerParams& params, const TimerCallback& callback) override;
	virtual void           DestroyTimer(const TimerId& timerId) override;
	virtual bool           StartTimer(const TimerId& timerId) override;
	virtual bool           StopTimer(const TimerId& timerId) override;
	virtual void           ResetTimer(const TimerId& timerId) override;
	virtual bool           IsTimerActive(const TimerId& timerId) const override;
	virtual STimerDuration GetTimeRemaining(const TimerId& timerId) const override;
	virtual void           Update() override;
	// ~ITimerSystem

private:

	static u32 ms_nextTimerId;

private:

	int64  m_frameCounter;
	Timers m_timers;
};
} // sxema
