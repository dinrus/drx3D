// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include "DrxThread_win32.h"
#include <drx3D/CoreX/Platform/DrxWindows.h>

namespace DrxMT
{
namespace detail
{

static_assert(sizeof(DRX_CRITICAL_SECTION) == sizeof(CRITICAL_SECTION), "Win32 CRITICAL_SECTION size does not match DRX_CRITICAL_SECTION");
static_assert(sizeof(DRX_SRWLOCK) == sizeof(SRWLOCK), "Win32 SRWLOCK size does not match DRX_SRWLOCK");
static_assert(sizeof(DRX_CONDITION_VARIABLE) == sizeof(CONDITION_VARIABLE), "Win32 CONDITION_VARIABLE size does not match DRX_CONDITION_VARIABLE");

//////////////////////////////////////////////////////////////////////////
//DrxLock_SRWLOCK
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
DRX_SRWLOCK::DRX_SRWLOCK()
	: SRWLock_(0)
{
	static_assert(sizeof(SRWLock_) == sizeof(PSRWLOCK), "RWLock-pointer has invalid size");
	InitializeSRWLock(reinterpret_cast<PSRWLOCK>(&SRWLock_));
}

//////////////////////////////////////////////////////////////////////////
//DRX_CONDITION_VARIABLE
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
DRX_CONDITION_VARIABLE::DRX_CONDITION_VARIABLE()
	: condVar_(0)
{
	static_assert(sizeof(condVar_) == sizeof(PCONDITION_VARIABLE), "ConditionVariable-pointer has invalid size");
	InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&condVar_));
}

//////////////////////////////////////////////////////////////////////////
// DrxLock_SRWLOCK
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK::LockShared()
{
	AcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_));
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK::UnlockShared()
{
	ReleaseSRWLockShared(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_));
}

//////////////////////////////////////////////////////////////////////////
bool DrxLock_SRWLOCK::TryLockShared()
{
	return TryAcquireSRWLockShared(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_)) == TRUE;
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK::Lock()
{
	AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_));
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK::Unlock()
{
	ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_));
}

//////////////////////////////////////////////////////////////////////////
bool DrxLock_SRWLOCK::TryLock()
{
	return TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&m_win32_lock_type.SRWLock_)) == TRUE;
}

//////////////////////////////////////////////////////////////////////////
// DrxLock_SRWLOCK_Recursive
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK_Recursive::Lock()
{
	const threadID threadId = DrxGetCurrentThreadId();

	if (threadId == m_exclusiveOwningThreadId)
	{
		++m_recurseCounter;
	}
	else
	{
		m_win32_lock_type.Lock();
		DRX_ASSERT(m_recurseCounter == 0);
		DRX_ASSERT(m_exclusiveOwningThreadId == THREADID_NULL);
		m_exclusiveOwningThreadId = threadId;
	}
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_SRWLOCK_Recursive::Unlock()
{
	const threadID threadId = DrxGetCurrentThreadId();
	DRX_ASSERT(m_exclusiveOwningThreadId == threadId);

	if (m_recurseCounter)
	{
		--m_recurseCounter;
	}
	else
	{
		m_exclusiveOwningThreadId = THREADID_NULL;
		m_win32_lock_type.Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////
bool DrxLock_SRWLOCK_Recursive::TryLock()
{
	const threadID threadId = DrxGetCurrentThreadId();
	if (m_exclusiveOwningThreadId == threadId)
	{
		++m_recurseCounter;
		return true;
	}
	else
	{
		const bool ret = (m_win32_lock_type.TryLock() == TRUE);
		if (ret)
		{
			m_exclusiveOwningThreadId = threadId;
		}
		return ret;
	}
}

//////////////////////////////////////////////////////////////////////////
// DrxLock_CritSection
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
DrxLock_CriticalSection::DrxLock_CriticalSection()
{
	InitializeCriticalSection((CRITICAL_SECTION*)&m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
DrxLock_CriticalSection::~DrxLock_CriticalSection()
{
	DeleteCriticalSection((CRITICAL_SECTION*)&m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_CriticalSection::Lock()
{
	EnterCriticalSection((CRITICAL_SECTION*)&m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_CriticalSection::Unlock()
{
	LeaveCriticalSection((CRITICAL_SECTION*)&m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
bool DrxLock_CriticalSection::TryLock()
{
	return TryEnterCriticalSection((CRITICAL_SECTION*)&m_win32_lock_type) != FALSE;
}

//////////////////////////////////////////////////////////////////////////
// DrxLock_WinMutex
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
DrxLock_WinMutex::DrxLock_WinMutex()
	: m_win32_lock_type(CreateMutex(NULL, FALSE, NULL))
{
	static_assert(sizeof(HANDLE) == sizeof(m_win32_lock_type), "WinMutex-pointer has invalid size");
}
DrxLock_WinMutex::~DrxLock_WinMutex()
{
	CloseHandle(m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_WinMutex::Lock()
{
	WaitForSingleObject(m_win32_lock_type, INFINITE);
}

//////////////////////////////////////////////////////////////////////////
void DrxLock_WinMutex::Unlock()
{
	ReleaseMutex(m_win32_lock_type);
}

//////////////////////////////////////////////////////////////////////////
bool DrxLock_WinMutex::TryLock()
{
	return WaitForSingleObject(m_win32_lock_type, 0) != WAIT_TIMEOUT;
}

}
}

//////////////////////////////////////////////////////////////////////////
DrxEvent::DrxEvent()
{
	m_handle = (uk )CreateEvent(NULL, FALSE, FALSE, NULL);
}

//////////////////////////////////////////////////////////////////////////
DrxEvent::~DrxEvent()
{
	CloseHandle(m_handle);
}

//////////////////////////////////////////////////////////////////////////
void DrxEvent::Reset()
{
	ResetEvent(m_handle);
}

//////////////////////////////////////////////////////////////////////////
void DrxEvent::Set()
{
	SetEvent(m_handle);
}

//////////////////////////////////////////////////////////////////////////
void DrxEvent::Wait() const
{
	WaitForSingleObject(m_handle, INFINITE);
}

//////////////////////////////////////////////////////////////////////////
bool DrxEvent::Wait(u32k timeoutMillis) const
{
	if (WaitForSingleObject(m_handle, timeoutMillis) == WAIT_TIMEOUT)
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void DrxConditionVariable::Wait(DrxMutex& lock)
{
	TimedWait(lock, INFINITE);
}

void DrxConditionVariable::Wait(DrxMutexFast& lock)
{
	TimedWait(lock, INFINITE);
}

//////////////////////////////////////////////////////////////////////////
bool DrxConditionVariable::TimedWait(DrxMutex& lock, u32 millis)
{
	if (lock.s_value == DrxMT::detail::eLockType_SRW)
	{
		DRX_ASSERT(lock.m_recurseCounter == 0);
		lock.m_exclusiveOwningThreadId = THREADID_NULL;
		bool ret = SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar), reinterpret_cast<PSRWLOCK>(&lock.m_win32_lock_type), millis, ULONG(0)) == TRUE;
		lock.m_exclusiveOwningThreadId = DrxGetCurrentThreadId();
		return ret;

	}
	else if (lock.s_value == DrxMT::detail::eLockType_CRITICAL_SECTION)
	{
		return SleepConditionVariableCS(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar), reinterpret_cast<PCRITICAL_SECTION>(&lock.m_win32_lock_type), millis) == TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
bool DrxConditionVariable::TimedWait(DrxMutexFast& lock, u32 millis)
{
	if (lock.s_value == DrxMT::detail::eLockType_SRW)
	{
		return SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar), reinterpret_cast<PSRWLOCK>(&lock.m_win32_lock_type), millis, ULONG(0)) == TRUE;
	}
	else if (lock.s_value == DrxMT::detail::eLockType_CRITICAL_SECTION)
	{
		return SleepConditionVariableCS(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar), reinterpret_cast<PCRITICAL_SECTION>(&lock.m_win32_lock_type), millis) == TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
void DrxConditionVariable::NotifySingle()
{
	WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar));
}

//////////////////////////////////////////////////////////////////////////
void DrxConditionVariable::Notify()
{
	WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&m_condVar));
}

//////////////////////////////////////////////////////////////////////////
DrxSemaphore::DrxSemaphore(i32 nMaximumCount, i32 nInitialCount)
{
	m_Semaphore = (uk )CreateSemaphore(NULL, nInitialCount, nMaximumCount, NULL);
}

//////////////////////////////////////////////////////////////////////////
DrxSemaphore::~DrxSemaphore()
{
	CloseHandle((HANDLE)m_Semaphore);
}

//////////////////////////////////////////////////////////////////////////
void DrxSemaphore::Acquire()
{
	WaitForSingleObject((HANDLE)m_Semaphore, INFINITE);
}

//////////////////////////////////////////////////////////////////////////
void DrxSemaphore::Release()
{
	ReleaseSemaphore((HANDLE)m_Semaphore, 1, NULL);
}

//////////////////////////////////////////////////////////////////////////
DrxFastSemaphore::DrxFastSemaphore(i32 nMaximumCount, i32 nInitialCount) :
	m_Semaphore(nMaximumCount),
	m_nCounter(nInitialCount)
{
}

//////////////////////////////////////////////////////////////////////////
DrxFastSemaphore::~DrxFastSemaphore()
{
}

//////////////////////////////////////////////////////////////////////////
void DrxFastSemaphore::Acquire()
{
	i32 nCount = ~0;
	do
	{
		nCount = *const_cast< i32*>(&m_nCounter);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nCounter), nCount - 1, nCount) != nCount);

	// if the count would have been 0 or below, go to kernel semaphore
	if ((nCount - 1) < 0)
		m_Semaphore.Acquire();
}

//////////////////////////////////////////////////////////////////////////
void DrxFastSemaphore::Release()
{
	i32 nCount = ~0;
	do
	{
		nCount = *const_cast< i32*>(&m_nCounter);
	}
	while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nCounter), nCount + 1, nCount) != nCount);

	// wake up kernel semaphore if we have waiter
	if (nCount < 0)
		m_Semaphore.Release();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::RLock()
{
	m_srw.LockShared();
}

//////////////////////////////////////////////////////////////////////////
bool DrxRWLock::TryRLock()
{
	return m_srw.TryLockShared();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::RUnlock()
{
	m_srw.UnlockShared();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::WLock()
{
	m_srw.Lock();
}

//////////////////////////////////////////////////////////////////////////
bool DrxRWLock::TryWLock()
{
	return m_srw.TryLock();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::WUnlock()
{
	m_srw.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::Lock()
{
	WLock();
}

//////////////////////////////////////////////////////////////////////////
bool DrxRWLock::TryLock()
{
	return TryWLock();
}

//////////////////////////////////////////////////////////////////////////
void DrxRWLock::Unlock()
{
	WUnlock();
}

///////////////////////////////////////////////////////////////////////////////
namespace DrxMT {

//////////////////////////////////////////////////////////////////////////
void DrxMemoryBarrier()
{
	MemoryBarrier();
}

//////////////////////////////////////////////////////////////////////////
void DrxYieldThread()
{
	SwitchToThread();
}
} // namespace DrxMT
