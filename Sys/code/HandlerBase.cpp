// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/CoreX/Project/ProjectDefines.h>
#if defined(MAP_LOADING_SLICING)

	#include "HandlerBase.h"
	#include <drx3D/CoreX/Platform/DrxWindows.h>

tukk SERVER_LOCK_NAME = "SynchronizeGameServer";
tukk CLIENT_LOCK_NAME = "SynchronizeGameClient";

HandlerBase::HandlerBase(tukk bucket, i32 affinity)
{
	m_serverLockName.Format("%s_%s", SERVER_LOCK_NAME, bucket);
	m_clientLockName.Format("%s_%s", CLIENT_LOCK_NAME, bucket);
	if (affinity != 0)
	{
		m_affinity = u32(1) << (affinity - 1);
	}
	else
	{
		m_affinity = -1;
	}
	m_prevAffinity = 0;
}

HandlerBase::~HandlerBase()
{
	if (m_prevAffinity)
	{
		if (SyncSetAffinity(m_prevAffinity))
			DrxLogAlways("Restored affinity to %d", m_prevAffinity);
		else
			DrxLogAlways("Failed to restore affinity to %d", m_prevAffinity);
	}
}

void HandlerBase::SetAffinity()
{
	if (m_prevAffinity) //already set
		return;
	if (u32 p = SyncSetAffinity(m_affinity))
	{
		DrxLogAlways("Changed affinity to %d", m_affinity);
		m_prevAffinity = p;
	}
	else
		DrxLogAlways("Failed to change affinity to %d", m_affinity);
}

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID

u32 HandlerBase::SyncSetAffinity(u32 cpuMask)//put -1
{
#if  DRX_PLATFORM_ANDROID
	return 0;
#else
	if (cpuMask != 0)
	{
		cpu_set_t cpuSet;
		u32 affinity = 0;
		if (!sched_getaffinity(getpid(), sizeof cpuSet, &cpuSet))
		{
			for (i32 cpu = 0; cpu < sizeof(cpuMask) * 8; ++cpu)
			{
				if (CPU_ISSET(cpu, &cpuSet))
					affinity |= 1 << cpu;
			}
		}
		if (affinity)
		{
			CPU_ZERO(&cpuSet);
			for (i32 cpu = 0; cpu < sizeof(cpuMask) * 8; ++cpu)
			{
				if (cpuMask & (1 << cpu))
				{
					CPU_SET(cpu, &cpuSet);
				}
			}

			if (!sched_setaffinity(getpid(), sizeof(cpuSet), &cpuSet))
				return affinity;
		}
	}
	return 0;
#endif
}

	#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO

u32 HandlerBase::SyncSetAffinity(u32 cpuMask)//put -1
{
	u32 p = (u32)SetThreadAffinityMask(GetCurrentThread(), cpuMask);
	if (p == 0)
		DrxLogAlways("Error updating affinity mask to %d", cpuMask);
	return p;
}

	#else

u32 HandlerBase::SyncSetAffinity(u32 cpuMask)//put -1
{
	DrxLogAlways("Updating thread affinity not supported on this platform");
	return 0;
}

	#endif

#endif // defined(MAP_LOADING_SLICING)
