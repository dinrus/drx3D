// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CallbackTimer_h__
#define __CallbackTimer_h__

#pragma once

#include <drx3D/CoreX/Containers/IDMap.h>

class CTimeValue;
class CallbackTimer
{
public:
	CallbackTimer();

	typedef u32                   TimerID;
	typedef Functor2<uk , TimerID> Callback;

	void    Clear();
	void    Update();

	TimerID AddTimer(CTimeValue interval, bool repeating, const Callback& callback, uk userdata = 0);
	uk   RemoveTimer(TimerID timerID);

	void    GetMemoryStatistics(IDrxSizer* s);
private:
	struct TimerInfo
	{
		TimerInfo(CTimeValue _interval, bool _repeating, const Callback& _callback, uk _userdata)
			: interval(_interval)
			, repeating(_repeating)
			, callback(_callback)
			, userdata(_userdata)
		{
		}

		CTimeValue interval;
		Callback   callback;
		uk      userdata;
		bool       repeating;
	};

	typedef id_map<size_t, TimerInfo> Timers;

	struct TimeoutInfo
	{
		TimeoutInfo(CTimeValue _timeout, TimerID _timerID)
			: timeout(_timeout)
			, timerID(_timerID)
		{
		}

		CTimeValue timeout;
		TimerID    timerID;

		ILINE bool operator<(const TimeoutInfo& other) const
		{
			return timeout < other.timeout;
		}
	};
	typedef std::deque<TimeoutInfo> Timeouts;

	Timers   m_timers;
	Timeouts m_timeouts;
	bool     m_resort;
};

#endif //__CallbackTimer_h__
