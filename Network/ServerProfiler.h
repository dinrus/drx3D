// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERVERPROFILER_H__
#define __SERVERPROFILER_H__

#pragma once

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT

	#include <drx3D/Network/Network.h>

class CServerProfiler
{
public:
	static void Init();
	static void Cleanup();

	static bool ShouldSaveAndCrash() { return m_saveAndCrash; }

private:
	CServerProfiler();
	~CServerProfiler();

	static CServerProfiler* m_pThis;
	FILE*                   m_fout;
	NetTimerId              m_timer;

	FILETIME                m_lastKernel, m_lastUser, m_lastTime;
	uint64                  m_lastIn, m_lastOut;

	static bool             m_saveAndCrash;

	static void             TimerCallback(NetTimerId, uk , CTimeValue);
	void Tick();
};

#else // DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT

class CServerProfiler
{
public:
	static void Init()               {}
	static void Cleanup()            {}
	static bool ShouldSaveAndCrash() { return false; }
};

#endif // DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT

#endif
