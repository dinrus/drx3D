// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/CoreX/Project/ProjectDefines.h>
#if defined(MAP_LOADING_SLICING)

	#include "SyncLock.h"

SSyncLock::SSyncLock(tukk name, i32 id, bool own)
{
	stack_string ss;
	ss.Format("%s_%d", name, id);

	Open(ss);
	if (own)
	{
		if (!IsValid())
		{
			Create(ss);
			number = id;
		}
		else
		{
			Close();
		}
	}
	else
		number = id;
}

SSyncLock::SSyncLock(tukk name, i32 minId, i32 maxId)
{
	ev = 0;
	stack_string ss;
	for (i32 i = minId; i < maxId; ++i)
	{
		ss.Format("%s_%d", name, i);
		if (Open(ss))
		{
			Close();
			continue;
		}
		if (Create(ss))
			number = i;
		break;
	}
}

SSyncLock::~SSyncLock()
{
	Close();
}

void SSyncLock::Own(tukk name)
{
	o_name.Format("%s_%d", name, number);
}

	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE

bool SSyncLock::Open(tukk name)
{
	ev = sem_open(name, 0);
	if (ev != SEM_FAILED)
		DrxLogAlways("Opened semaphore %p %s", ev, name);
	return IsValid();
}

bool SSyncLock::Create(tukk name)
{
	ev = sem_open(name, O_CREAT | O_EXCL, 0777, 0);
	if (ev != SEM_FAILED)
		DrxLogAlways("Created semaphore %p %s", ev, name);
	else
		DrxLogAlways("Failed to create semaphore %s %d", name, errno);
	return IsValid();
}

void SSyncLock::Signal()
{
	if (ev)
		sem_post(ev);
}

bool SSyncLock::Wait(i32 ms)
{
	if (!ev)
		return false;

	timespec t = { 0 };
		#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	clock_gettime(CLOCK_REALTIME, &t);
		#elif DRX_PLATFORM_APPLE
	// On OSX/iOS there is no sem_timedwait()
	// We use repeated sem_trywait() instead
	if (sem_trywait(ev) == 0) return true;
		#endif

	static const long NANOSECS_IN_MSEC = 1000000L;
	static const long NANOSECS_IN_SEC = 1000000000L;

	t.tv_sec += ms / 1000;
	t.tv_nsec += (ms % 1000) * NANOSECS_IN_MSEC;
	if (t.tv_nsec > NANOSECS_IN_SEC)
	{
		t.tv_nsec -= NANOSECS_IN_SEC;
		++t.tv_sec;
	}
		#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	return sem_timedwait(ev, &t) == 0; //ETIMEDOUT for timeout
		#elif DRX_PLATFORM_APPLE
	// t = time left, interval = max time between tries, elapsed = actual time elapsed during a try
	i32k num_ms_interval = 50; // poll time, in ms
	const timespec interval = { 0, NANOSECS_IN_MSEC * num_ms_interval };
	while (t.tv_sec >= 0 || t.tv_nsec > interval.tv_nsec)
	{
		timespec remaining;
		timespec elapsed = interval;
		if (nanosleep(&interval, &remaining) == -1)
		{
			elapsed.tv_nsec -= remaining.tv_nsec;
		}
		t.tv_nsec -= elapsed.tv_nsec;
		if (t.tv_nsec < 0L)
		{
			t.tv_nsec += NANOSECS_IN_SEC;
			t.tv_sec -= 1;
		}
		if (sem_trywait(ev) == 0) return true;
	}
	nanosleep(&t, NULL);
	return sem_trywait(ev) == 0;
		#else
			#error Not implemented
		#endif
}

void SSyncLock::Close()
{
	if (ev)
	{
		DrxLogAlways("Closed event %p", ev);
		if (!o_name.empty())
			sem_unlink(o_name);
		sem_close(ev);
		ev = 0;
	}
}

	#else

bool SSyncLock::Open(tukk name)
{
	ev = OpenEvent(SYNCHRONIZE, FALSE, name);
	if (ev)
		DrxLogAlways("Opened event %p %s", ev, name);
	return IsValid();
}

bool SSyncLock::Create(tukk name)
{
	ev = CreateEvent(NULL, FALSE, FALSE, name);
	if (ev)
		DrxLogAlways("Created event %p %s", ev, name);
	else
		DrxLogAlways("Failed to create event %s", name);
	return IsValid();
}

bool SSyncLock::Wait(i32 ms)
{
	//	DrxLogAlways("Waiting %p", ev);
	DWORD res = WaitForSingleObject(ev, ms);
	if (res != WAIT_OBJECT_0)
	{
		DrxLogAlways("WFS result %d", res);
	}
	return res == WAIT_OBJECT_0;
}

void SSyncLock::Signal()
{
	//DrxLogAlways("Signaled %p", ev);
	if (!SetEvent(ev))
	{
		DrxLogAlways("Error signalling!");
	}
}

void SSyncLock::Close()
{
	if (ev)
	{
		DrxLogAlways("Closed event %p", ev);
		CloseHandle(ev);
		ev = 0;
	}
}

	#endif

bool SSyncLock::IsValid() const
{
	return ev != 0;
}

#endif // defined(MAP_LOADING_SLICING)
