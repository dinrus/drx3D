// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   TimeOfDayScheduler.h
//  Version:     v1.00
//  Created:     27/02/2007 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: TimeOfDayScheduler
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TIMEOFDAYSCHEDULER_H__
#define __TIMEOFDAYSCHEDULER_H__

#pragma once

class CTimeOfDayScheduler
{
public:
	// timer id
	typedef i32 TimeOfDayTimerId;
	static i32k InvalidTimerId = -1;

	// timer id, user data, current time of day (not scheduled time of day!)
	typedef void (* TimeOfDayTimerCallback)(TimeOfDayTimerId, uk , float);

	CTimeOfDayScheduler();
	~CTimeOfDayScheduler();

	void Reset();  // clears all scheduled timers
	void Update(); // updates (should be called every frame, internally updates in intervalls)

	void GetMemoryStatistics(IDrxSizer* s)
	{
		s->Add(*this);
		s->AddContainer(m_entries);
	}

	TimeOfDayTimerId AddTimer(float time, TimeOfDayTimerCallback callback, uk pUserData);
	uk            RemoveTimer(TimeOfDayTimerId id); // returns user data

protected:
	struct SEntry
	{
		SEntry(TimeOfDayTimerId id, float time, TimeOfDayTimerCallback callback, uk pUserData)
		{
			this->id = id;
			this->time = time;
			this->callback = callback;
			this->pUserData = pUserData;
		}

		bool operator==(const TimeOfDayTimerId& otherId) const
		{
			return id == otherId;
		}

		bool operator<(const SEntry& other) const
		{
			return time < other.time;
		}

		bool operator<(const float& otherTime) const
		{
			return time < otherTime;
		}

		TimeOfDayTimerId       id;          // 4 bytes
		float                  time;        // 4 bytes
		TimeOfDayTimerCallback callback;    // 4/8 bytes
		uk                  pUserData;   // 4/8 bytes
		//                                   = 32/48 bytes
	};

	typedef std::vector<SEntry> TEntries;
	TEntries         m_entries;
	TimeOfDayTimerId m_nextId;
	float            m_lastTime;
	bool             m_bForceUpdate;
};

#endif // __TIMEOFDAYSCHEDULER_H__
