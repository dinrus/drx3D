// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <process.h>

namespace DrxMT {
namespace detail {
enum eLOCK_TYPE
{
	eLockType_CRITICAL_SECTION,
	eLockType_SRW,
	eLockType_MUTEX
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! From winnt.h.
// Since we are not allowed to include windows.h while being included from platform.h and there seems to be no good way to include the
// required windows headers directly; without including a lot of other header, define a 1:1 copy of the required primitives defined in winnt.h.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
struct DRX_CRITICAL_SECTION // From winnt.h
{
	uk          DebugInfo;
	long           LockCount;
	long           RecursionCount;
	uk          OwningThread;
	uk          LockSemaphore;
	u64* SpinCount;  //!< Force size on 64-bit systems when packed.
};

//////////////////////////////////////////////////////////////////////////
struct DRX_SRWLOCK // From winnt.h
{
	DRX_SRWLOCK();
	uk SRWLock_;
};

//////////////////////////////////////////////////////////////////////////
struct DRX_CONDITION_VARIABLE // From winnt.h
{
	DRX_CONDITION_VARIABLE();
	uk condVar_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class DrxLock_SRWLOCK
{
public:
	static const eLOCK_TYPE s_value = eLockType_SRW;
	friend class DrxConditionVariable;
public:
	DrxLock_SRWLOCK() = default;

	void Lock();
	void Unlock();
	bool TryLock();

	void LockShared();
	void UnlockShared();
	bool TryLockShared();

//private:
	DrxLock_SRWLOCK(const DrxLock_SRWLOCK&) = delete;
	DrxLock_SRWLOCK& operator=(const DrxLock_SRWLOCK&) = delete;

//private:
	DRX_SRWLOCK m_win32_lock_type;
};

//////////////////////////////////////////////////////////////////////////
// SRW Lock (Slim Reader/Writer Lock)
// Faster + lighter than CriticalSection. Also only enters into kernel mode if contended.
// Cannot be shared between processes.
class DrxLock_SRWLOCK_Recursive
{
public:
	static const eLOCK_TYPE s_value = eLockType_SRW;
	friend class DrxConditionVariable;

public:
	DrxLock_SRWLOCK_Recursive() : m_recurseCounter(0), m_exclusiveOwningThreadId(THREADID_NULL) {}

	void Lock();
	void Unlock();
	bool TryLock();

	// Deprecated
#ifndef _RELEASE
	bool IsLocked()
	{
		return m_exclusiveOwningThreadId == DrxGetCurrentThreadId();
	}
#endif

//private:
	DrxLock_SRWLOCK_Recursive(const DrxLock_SRWLOCK_Recursive&) = delete;
	DrxLock_SRWLOCK_Recursive& operator=(const DrxLock_SRWLOCK_Recursive&) = delete;

//private:
	DrxLock_SRWLOCK m_win32_lock_type;
	u32          m_recurseCounter;
	
	// Due to its semantics, this member can be accessed in an unprotected manner,
	// but only for comparison with the current tid.
	threadID        m_exclusiveOwningThreadId;
};

//////////////////////////////////////////////////////////////////////////
// Critical section
// Faster then WinMutex as it only enters into kernel mode if contended.
// Cannot be shared between processes.
class DrxLock_CriticalSection
{
public:
	static const eLOCK_TYPE s_value = eLockType_CRITICAL_SECTION;
	friend class DrxConditionVariable;

public:
	DrxLock_CriticalSection();
	~DrxLock_CriticalSection();

	void Lock();
	void Unlock();
	bool TryLock();

	//! Deprecated: do not use this function - its return value might already be wrong the moment it is returned.
#ifndef _RELEASE
	bool IsLocked()
	{
		return m_win32_lock_type.RecursionCount > 0 && (UINT_PTR)m_win32_lock_type.OwningThread == DrxGetCurrentThreadId();
	}
#endif

private:
	DrxLock_CriticalSection(const DrxLock_CriticalSection&) = delete;
	DrxLock_CriticalSection& operator=(const DrxLock_CriticalSection&) = delete;

private:
	DRX_CRITICAL_SECTION m_win32_lock_type;
};

//////////////////////////////////////////////////////////////////////////
// WinMutex: (slow)
// Calls into kernel even when not contended.
// A named mutex can be shared between different processes.
class DrxLock_WinMutex
{
public:
	static const eLOCK_TYPE s_value = eLockType_MUTEX;

	DrxLock_WinMutex();
	~DrxLock_WinMutex();

	void Lock();
	void Unlock();
	bool TryLock();
#ifndef _RELEASE
	//! Deprecated: do not use this function - its return value might already be wrong the moment it is returned.
	bool IsLocked()
	{
		if (TryLock())
		{
			Unlock();
			return true;
		}
		else
		{
			return false;
		}
	}
#endif
private:
	DrxLock_WinMutex(const DrxLock_WinMutex&) = delete;
	DrxLock_WinMutex& operator=(const DrxLock_WinMutex&) = delete;

private:
	uk m_win32_lock_type;
};
} // detail
} // DrxMT

  //////////////////////////////////////////////////////////////////////////
  /////////////////////////    DEFINE LOCKS    /////////////////////////////
  //////////////////////////////////////////////////////////////////////////

template<> class DrxLockT<DRXLOCK_RECURSIVE> : public DrxMT::detail::DrxLock_SRWLOCK_Recursive
{
};
template<> class DrxLockT<DRXLOCK_FAST> : public DrxMT::detail::DrxLock_SRWLOCK
{
};

typedef DrxMT::detail::DrxLock_SRWLOCK_Recursive DrxMutex;
typedef DrxMT::detail::DrxLock_SRWLOCK           DrxMutexFast; // Not recursive

//////////////////////////////////////////////////////////////////////////
//! DrxEvent represent a synchronization event.
class DrxEvent
{
public:
	DrxEvent();
	~DrxEvent();

	//! Reset the event to the unsignalled state.
	void Reset();

	//! Set the event to the signalled state.
	void Set();

	//! Access a HANDLE to wait on.
	uk GetHandle() const { return m_handle; };

	//! Wait indefinitely for the object to become signalled.
	void Wait() const;

	//! Wait, with a time limit, for the object to become signalled.
	bool Wait(u32k timeoutMillis) const;

private:
	DrxEvent(const DrxEvent&);
	DrxEvent& operator=(const DrxEvent&);

private:
	uk m_handle;
};
typedef DrxEvent DrxEventTimed;

//////////////////////////////////////////////////////////////////////////
class DrxConditionVariable
{
public:
	DrxConditionVariable() = default;
	void Wait(DrxMutex& lock);
	void Wait(DrxMutexFast& lock);
	bool TimedWait(DrxMutex& lock, u32 millis);
	bool TimedWait(DrxMutexFast& lock, u32 millis);
	void NotifySingle();
	void Notify();

private:
	DrxConditionVariable(const DrxConditionVariable&);
	DrxConditionVariable& operator=(const DrxConditionVariable&);

private:
	DrxMT::detail::DRX_CONDITION_VARIABLE m_condVar;
};

//////////////////////////////////////////////////////////////////////////
//! Platform independent wrapper for a counting semaphore.
class DrxSemaphore
{
public:
	DrxSemaphore(i32 nMaximumCount, i32 nInitialCount = 0);
	~DrxSemaphore();
	void Acquire();
	void Release();

private:
	uk m_Semaphore;
};

//////////////////////////////////////////////////////////////////////////
//! Platform independent wrapper for a counting semaphore
//! except that this version uses C-A-S only until a blocking call is needed.
//! -> No kernel call if there are object in the semaphore.
class DrxFastSemaphore
{
public:
	DrxFastSemaphore(i32 nMaximumCount, i32 nInitialCount = 0);
	~DrxFastSemaphore();
	void Acquire();
	void Release();

private:
	DrxSemaphore   m_Semaphore;
	 i32 m_nCounter;
};

//////////////////////////////////////////////////////////////////////////
class DrxRWLock
{
public:
	DrxRWLock() = default;

	void RLock();
	void RUnlock();

	void WLock();
	void WUnlock();

	void Lock();
	void Unlock();

	bool TryRLock();
	bool TryWLock();
	bool TryLock();
private:
	DrxMT::detail::DrxLock_SRWLOCK m_srw;

	DrxRWLock(const DrxRWLock&) = delete;
	DrxRWLock& operator=(const DrxRWLock&) = delete;
};
