// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/Watchdog.h>
#include <drx3D/CoreX/Thread/IThreadUpr.h>

void CWatchdogThread::SignalStopWork()
{
	m_bQuit = true;
}

CWatchdogThread::~CWatchdogThread()
{
	SignalStopWork();
}

void CWatchdogThread::SetTimeout(i32 timeOutSeconds)
{
	DRX_ASSERT(timeOutSeconds > 0);
	m_timeOutSeconds = timeOutSeconds;
}

CWatchdogThread::CWatchdogThread(i32 timeOutSeconds)
	: m_timeOutSeconds(timeOutSeconds)
{
	DRX_ASSERT(timeOutSeconds > 0);
	if (!gEnv->pThreadUpr->SpawnThread(this, "Watch Dog"))
	{
		DRX_ASSERT_MESSAGE(false, "Error spawning \"Watch Dog\" thread.");
	}
}

uint64 CWatchdogThread::GetSystemUpdateCounter()
{
	DRX_ASSERT(GetISystem());
	return GetISystem()->GetUpdateCounter();
}

void CWatchdogThread::ThreadEntry()
{
	uint64 prevUpdateCounter = GetSystemUpdateCounter();
	Sleep();
	while (!m_bQuit)
	{
		auto curUpdateCounter = GetSystemUpdateCounter();
		if (prevUpdateCounter != curUpdateCounter)
		{
			prevUpdateCounter = curUpdateCounter;
		}
		else
		{
			DrxFatalError("Freeze detected. Watchdog timeout.");
		}
		Sleep();
	}
}

void CWatchdogThread::Sleep() const
{
	DRX_ASSERT(m_timeOutSeconds > 0);
	DrxSleep(1000 * m_timeOutSeconds);
}
