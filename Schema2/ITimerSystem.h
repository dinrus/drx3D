// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Use scoped connections to automatically disconnect timers?
// #SchematycTODO : Use combination of pools and seeds for more efficient look-up by id?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_ArrayView.h>
#include <drx3D/Schema2/TemplateUtils_Delegate.h>
#include <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>

#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/StringUtils.h>

namespace sxema2
{
	enum class ETimerUnits
	{
		Invalid,
		Frames,
		Seconds,
		Random
	};

	struct STimerDuration
	{
		inline STimerDuration()
			: units(ETimerUnits::Invalid)
		{
			memset(this, 0, sizeof(STimerDuration));
		}

		explicit inline STimerDuration(u32 _frames)
		{
			memset(this, 0, sizeof(STimerDuration));
			units  = ETimerUnits::Frames;
			frames = _frames;
		}

		explicit inline STimerDuration(float _seconds)
		{
			memset(this, 0, sizeof(STimerDuration));
			units   = ETimerUnits::Seconds;
			seconds = _seconds;
		}

		explicit inline STimerDuration(float _min, float _max)
		{
			memset(this, 0, sizeof(STimerDuration));
			units     = ETimerUnits::Random;
			range.min = _min;
			range.max = _max;
		}

		inline STimerDuration(const STimerDuration& rhs)
		{
			memcpy(this, &rhs, sizeof(STimerDuration));
		}

		ETimerUnits units;
		union
		{
			u32 frames;
			float  seconds;
			struct
			{
				float min;
				float max;
			} range;
		};
	};

	enum class ETimerFlags
	{
		None      = 0,
		AutoStart = BIT(0),
		Repeat    = BIT(1)
	};

	DECLARE_ENUM_CLASS_FLAGS(ETimerFlags)

	struct STimerParams
	{
		inline STimerParams(const STimerDuration& _duration = STimerDuration(), ETimerFlags _flags = ETimerFlags::None)
			: duration(_duration)
			, flags(_flags)
		{}

		STimerDuration duration;
		ETimerFlags    flags;
	};

	typedef TemplateUtils::CDelegate<void ()> TimerCallback;
	typedef u32                            TimerId;

	static const TimerId s_invalidTimerId = ~0;

	struct ITimerSystem
	{
		virtual ~ITimerSystem() {}

		virtual TimerId CreateTimer(const STimerParams& params, const TimerCallback& callback) = 0;
		virtual void DestroyTimer(TimerId timerId) = 0;
		virtual bool StartTimer(TimerId timerId) = 0;
		virtual bool StopTimer(TimerId timerId) = 0;
		virtual void ResetTimer(TimerId timerId) = 0;
		virtual bool IsTimerActive(TimerId timerId) const = 0;
		virtual STimerDuration GetTimeRemaining(TimerId timerId) const = 0;
		virtual void Update() = 0;
	};

	inline bool ToString(const STimerDuration& duration, const CharArrayView& output)
	{
		switch(duration.units)
		{
		case ETimerUnits::Frames:
			{
				return StringUtils::UInt32ToString(duration.frames, output) && StringUtils::Append("(frames)", output);
			}
		case ETimerUnits::Seconds:
			{
				return StringUtils::FloatToString(duration.seconds, output) && StringUtils::Append("(s)", output);
			}
		}
		return false;
	}
}
