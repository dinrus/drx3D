// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __NETTIMER_H__
#define __NETTIMER_H__

#pragma once

///////////////////////////////////////////////////////////////////////////////
//
// Old net timers
//

#include <drx3D/Sys/TimeValue.h>
#include <queue>
//#include <drx3D/Network/VectorMap.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
#include <drx3D/Network/Config.h>

typedef u32  NetTimerId;
typedef void (* NetTimerCallback)(NetTimerId, uk , CTimeValue);

static i32k TIMER_HERTZ = 250;

class CNetTimer
{
public:
	CNetTimer();
#if TIMER_DEBUG
	NetTimerId AddTimer(CTimeValue when, NetTimerCallback callback, uk pUserData, tuk pFile, size_t line, tuk pName);
#else
	NetTimerId AddTimer(CTimeValue when, NetTimerCallback callback, uk pUserData);
#endif // TIMER_DEBUG
	uk      CancelTimer(NetTimerId& id);
	CTimeValue Update();

	void       GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CNetTimer");
		if (countingThis)
			pSizer->Add(*this);
		pSizer->AddContainer(m_callbacks);
	}

private:
	static i32k TIMER_SLOTS = 32; // 0.128 seconds
	i32              m_timerCallbacks[TIMER_SLOTS];
	i32              m_curSlot;
#if USE_SYSTEM_ALLOCATOR
	std::multimap<CTimeValue, i32, std::less<CTimeValue>> m_slowCallbacks;
#else
	std::multimap<CTimeValue, i32, std::less<CTimeValue>, stl::STLPoolAllocator<std::pair<const CTimeValue, i32>, stl::PoolAllocatorSynchronizationSinglethreaded>> m_slowCallbacks;
#endif

	struct SCallbackInfo
	{
		SCallbackInfo(NetTimerCallback callback, uk pUserData)
		{
			this->callback = callback;
			this->pUserData = pUserData;
			next = -1;
			cancelled = true;
		}
		SCallbackInfo()
		{
			this->callback = 0;
			this->pUserData = 0;
			next = -1;
			cancelled = true;
		}
		NetTimerCallback callback;
		uk            pUserData;
		i32              next;
		bool             cancelled;
#if TIMER_DEBUG
		CTimeValue       schedTime;
		i32              slotsInFuture;
		i32              curSlotWhenScheduled;
#endif
	};

	typedef std::vector<i32, stl::STLGlobalAllocator<i32>> TGlobalIntVector;

	std::vector<SCallbackInfo, stl::STLGlobalAllocator<SCallbackInfo>> m_callbacks;
	TGlobalIntVector m_freeCallbacks;
	TGlobalIntVector m_execCallbacks;
	CTimeValue       m_wakeup;
	CTimeValue       m_lastExec;
#if TIMER_DEBUG
	CTimeValue       m_epoch; // just for debugging
#endif
};

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Accurate net timers
//

class CAccurateNetTimer
{
public:
	CAccurateNetTimer();
#if TIMER_DEBUG
	NetTimerId AddTimer(CTimeValue when, NetTimerCallback callback, uk pUserData, tuk pFile, size_t line, tuk pName);
#else
	NetTimerId AddTimer(CTimeValue when, NetTimerCallback callback, uk pUserData);
#endif // TIMER_DEBUG
	uk      CancelTimer(NetTimerId& id);
	CTimeValue Update();

	void       GetMemoryStatistics(IDrxSizer* pSizer, bool countingThis = false)
	{
		SIZER_COMPONENT_NAME(pSizer, "CAccurateNetTimer");
		if (countingThis)
			pSizer->Add(*this);
		pSizer->AddContainer(m_callbacks);
	}

private:
	struct SCallbackInfo
	{
		SCallbackInfo(NetTimerCallback callback, uk pUserData)
		{
			this->callback = callback;
			this->pUserData = pUserData;
			inUse = true;
		}
		SCallbackInfo()
		{
			this->callback = 0;
			this->pUserData = 0;
			inUse = false;
		}
		NetTimerCallback     callback;
		uk                pUserData;
		bool                 inUse;
		CTimeValue           schedTime;
#if TIMER_DEBUG
		CTimeValue           timeAdded;
		DrxFixedStringT<256> location;
		DrxFixedStringT<128> name;
#endif
	};

	typedef std::vector<i32, stl::STLGlobalAllocator<i32>> TGlobalIntVector;

	std::vector<SCallbackInfo, stl::STLGlobalAllocator<SCallbackInfo>> m_callbacks;
	TGlobalIntVector m_freeCallbacks;
	CTimeValue       m_nextUpdate;
};

//
///////////////////////////////////////////////////////////////////////////////

#endif
