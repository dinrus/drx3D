// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: CTPEndpoint.cpp
//  Описание: нить, посылающая поддерживающие активность уведомления зарегистрированным сокетам,
// в случае если глобальный замок удерживается длительное время.
//
//	История:
//	-August 24,2007:Created by Craig Tiller
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <drx3D/Network/NetAddress.h>
#include <drx3D/Network/DrxSocks.h>

#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
#include <memory>

extern  u32 g_watchdogTimerGlobalLockCount;
extern  u32 g_watchdogTimerLockedCounter;

class CAutoUpdateWatchdogCounters
{
public:
	ILINE CAutoUpdateWatchdogCounters()
	{
		++g_watchdogTimerGlobalLockCount;
		++g_watchdogTimerLockedCounter;
	}
	ILINE ~CAutoUpdateWatchdogCounters()
	{
		--g_watchdogTimerLockedCounter;
	}
};

namespace detail
{
class CTimerThread;
}

class CWatchdogTimer
{
public:
	CWatchdogTimer();
	~CWatchdogTimer();

	void RegisterTarget(DRXSOCKET sock, const TNetAddress& addr);
	void UnregisterTarget(DRXSOCKET sock, const TNetAddress& addr);

	bool HasStalled() const;
	void ClearStalls();

private:
	std::unique_ptr<detail::CTimerThread> m_pThread;
};
