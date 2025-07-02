// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNCLOCK_H__
#define __SYNCLOCK_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#include <semaphore.h>
#elif DRX_PLATFORM_WINDOWS
	#include <drx3D/CoreX/Platform/DrxWindows.h>
#endif

struct SSyncLock
{
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	typedef sem_t* HandleType;
#else
	typedef HANDLE HandleType;
#endif

	SSyncLock(tukk name, i32 id, bool own);
	SSyncLock(tukk name, i32 minId, i32 maxId);
	~SSyncLock();

	void Own(tukk name);
	bool Open(tukk name);
	bool Create(tukk name);
	void Signal();
	bool Wait(i32 ms);
	void Close();
	bool IsValid() const;

	HandleType ev;
	i32        number;
	string     o_name;
};

#endif // __SYNCLOCK_H__
