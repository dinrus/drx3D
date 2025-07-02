// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ILog.h>

// PTHREAD_MUTEX_FAST_NP is only defined by Pthreads-w32 & Pthreads-w64
#if !defined(PTHREAD_MUTEX_FAST_NP)
	#define PTHREAD_MUTEX_FAST_NP PTHREAD_MUTEX_NORMAL
#endif

namespace DrxMT {
namespace detail {

enum eLOCK_TYPE
{
	eLockType_NORMAL     = PTHREAD_MUTEX_FAST_NP,
	eLockType_RECURSIVE  = PTHREAD_MUTEX_RECURSIVE,
	eLockType_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK
};

//////////////////////////////////////////////////////////////////////////
// Forward declarations (in same namespace)
template<eLOCK_TYPE pthreadMutexType> class DrxLock_Mutex;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// PThreadLockAttr wrapper
struct DrxPThreadLockAttr
{
public:
	DrxPThreadLockAttr(eLOCK_TYPE lockType)
	{
		pthread_mutexattr_init(&m_Attr);
		pthread_mutexattr_settype(&m_Attr, lockType);
	}
	~DrxPThreadLockAttr()
	{
		pthread_mutexattr_destroy(&m_Attr);
	}

	pthread_mutexattr_t m_Attr;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// PThread mutex can be of type:
// eLockType_NORMAL: No recursive lock functionality
// eLockType_RECURSIVE: Recursive lock functionality
// eLockType_ERRORCHECK: No recursive lock functionality. Added functionality of returning error on invalid lock procedures.
//

template<eLOCK_TYPE pthreadMutexType>
class DrxLock_Mutex
{
public:
	static const eLOCK_TYPE s_value = pthreadMutexType;
	friend class::DrxConditionVariable;

public:
	DrxLock_Mutex()
	{
		const DrxPThreadLockAttr attr(pthreadMutexType);
		pthread_mutex_init(&m_posix_mutex, &attr.m_Attr);
	}
	~DrxLock_Mutex()
	{
		pthread_mutex_destroy(&m_posix_mutex);
	}

	void Lock()
	{
		i32k errorCode = pthread_mutex_lock(&m_posix_mutex);
		if (pthreadMutexType == eLockType_ERRORCHECK && errorCode != 0)
		{
			// An invalid lock operation was performed.
			// Please refer to PThread documentation about the error code returned.
			__debugbreak();
		}

#ifndef _RELEASE
		++m_LockCount;
#endif
	}

	void Unlock()
	{
#ifndef _RELEASE
		--m_LockCount;
#endif
		i32k errorCode = pthread_mutex_unlock(&m_posix_mutex);

		if (pthreadMutexType == eLockType_ERRORCHECK && errorCode != 0)
		{
			// An invalid lock operation was performed.
			// Please refer to PThread documentation about the error code returned.
			__debugbreak();
		}
	}

	bool TryLock()
	{
		i32k errorCode = pthread_mutex_trylock(&m_posix_mutex);
		if (errorCode == 0)
		{
#ifndef _RELEASE
			++m_LockCount;
#endif
			return true;
		}

		if (pthreadMutexType == eLockType_ERRORCHECK && errorCode != EBUSY)
		{
			// An invalid lock operation was performed.
			// Please refer to PThread documentation about the error code returned.
			__debugbreak();
		}
		return false;
	}

	// Deprecated. Value might already be wrong on return
#ifndef _RELEASE
	bool IsLocked() const
	{
		return m_LockCount > 0;
	}
#endif

protected:
	pthread_mutex_t m_posix_mutex;
#ifndef _RELEASE
	 uint   m_LockCount = 0;
#endif
};

} // detail
} // DrxMT

//////////////////////////////////////////////////////////////////////////
/////////////////////////    DEFINE LOCKS    /////////////////////////////
//////////////////////////////////////////////////////////////////////////

template<> class DrxLockT<DRXLOCK_RECURSIVE> : public DrxMT::detail::DrxLock_Mutex<DrxMT::detail::eLockType_RECURSIVE>
{
};
template<> class DrxLockT<DRXLOCK_FAST> : public DrxMT::detail::DrxLock_Mutex<DrxMT::detail::eLockType_NORMAL>
{
};

typedef DrxMT::detail::DrxLock_Mutex<DrxMT::detail::eLockType_RECURSIVE> DrxMutex;
typedef DrxMT::detail::DrxLock_Mutex<DrxMT::detail::eLockType_NORMAL>    DrxMutexFast;        // Not recursive

//////////////////////////////////////////////////////////////////////////
class DrxConditionVariable
{
public:
	DrxConditionVariable()
	{
		pthread_cond_init(&m_condVar, NULL);
	};

	~DrxConditionVariable()
	{
		pthread_cond_destroy(&m_condVar);
	}

	void Wait(DrxMutex& lock)
	{
		pthread_cond_wait(&m_condVar, &lock.m_posix_mutex);
	}

	void Wait(DrxMutexFast& lock)
	{
		pthread_cond_wait(&m_condVar, &lock.m_posix_mutex);
	}

	bool TimedWait(DrxMutex& lock, u32 millis)
	{
		return TimedWait_Internal(lock, millis);
	}

	bool TimedWait(DrxMutexFast& lock, u32 millis)
	{
		return TimedWait_Internal(lock, millis);
	}

	void NotifySingle() { pthread_cond_signal(&m_condVar); }
	void Notify()       { pthread_cond_broadcast(&m_condVar); }

private:
	template<typename T>
	bool TimedWait_Internal(T& lock, u32 millis)
	{
		struct timeval now;
		struct timespec timeout;
		i32 err;

		gettimeofday(&now, NULL);
		while (true)
		{
			timeout.tv_sec = now.tv_sec + millis / 1000;
			uint64 nsec = (uint64)now.tv_usec * 1000 + (uint64)millis * 1000000;
			if (nsec >= 1000000000)
			{
				timeout.tv_sec += (long)(nsec / 1000000000);
				nsec %= 1000000000;
			}
			timeout.tv_nsec = (long)nsec;
			err = pthread_cond_timedwait(&m_condVar, &lock.m_posix_mutex, &timeout);
			if (err == EINTR)
			{
				// Interrupted by a signal.
				continue;
			}
			else if (err == ETIMEDOUT)
			{
				return false;
			}
			else
				assert(err == 0);
			break;
		}
		return true;
	}

private:
	DrxConditionVariable(const DrxConditionVariable&);
	DrxConditionVariable& operator=(const DrxConditionVariable&);

private:
	pthread_cond_t m_condVar;
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
	sem_t m_Semaphore;
};

//////////////////////////////////////////////////////////////////////////
inline DrxSemaphore::DrxSemaphore(i32 nMaximumCount, i32 nInitialCount)
{
	sem_init(&m_Semaphore, 0, nInitialCount);
}

//////////////////////////////////////////////////////////////////////////
inline DrxSemaphore::~DrxSemaphore()
{
	sem_destroy(&m_Semaphore);
}

//////////////////////////////////////////////////////////////////////////
inline void DrxSemaphore::Acquire()
{
	sem_wait(&m_Semaphore);
}

//////////////////////////////////////////////////////////////////////////
inline void DrxSemaphore::Release()
{
	sem_post(&m_Semaphore);
}

//////////////////////////////////////////////////////////////////////////
//! Platform independet wrapper for a counting semaphore,
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
inline DrxFastSemaphore::DrxFastSemaphore(i32 nMaximumCount, i32 nInitialCount) :
	m_Semaphore(nMaximumCount),
	m_nCounter(nInitialCount)
{
}

//////////////////////////////////////////////////////////////////////////
inline DrxFastSemaphore::~DrxFastSemaphore()
{
}

/////////////////////////////////////////////////////////////////////////
inline void DrxFastSemaphore::Acquire()
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
inline void DrxFastSemaphore::Release()
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
class DrxRWLock
{
	pthread_rwlock_t m_Lock;

	DrxRWLock(const DrxRWLock&);
	DrxRWLock& operator=(const DrxRWLock&);

public:
	DrxRWLock() { pthread_rwlock_init(&m_Lock, NULL); }
	~DrxRWLock() { pthread_rwlock_destroy(&m_Lock); }
	void RLock() { pthread_rwlock_rdlock(&m_Lock); }
	bool TryRLock()
	{
		return pthread_rwlock_tryrdlock(&m_Lock) != EBUSY;
	}
	void RUnlock() { Unlock(); }
	void WLock()   { pthread_rwlock_wrlock(&m_Lock); }
	bool TryWLock()
	{
		return pthread_rwlock_trywrlock(&m_Lock) != EBUSY;
	}
	void WUnlock() { Unlock(); }
	void Lock()    { WLock(); }
	bool TryLock() { return TryWLock(); }
	void Unlock()  { pthread_rwlock_unlock(&m_Lock); }
};

//! Provide TLS implementation using pthreads for those platforms without thread.
struct SDrxPthreadTLSBase
{
	SDrxPthreadTLSBase(void(*pDestructor)(uk ))
	{
		pthread_key_create(&m_kKey, pDestructor);
	}

	~SDrxPthreadTLSBase()
	{
		pthread_key_delete(m_kKey);
	}

	uk GetSpecific()
	{
		return pthread_getspecific(m_kKey);
	}

	void SetSpecific(ukk pValue)
	{
		pthread_setspecific(m_kKey, pValue);
	}

	pthread_key_t m_kKey;
};

template<typename T, bool bDirect> struct SDrxPthreadTLSImpl {};

template<typename T> struct SDrxPthreadTLSImpl<T, true> : private SDrxPthreadTLSBase
{
	SDrxPthreadTLSImpl()
		: SDrxPthreadTLSBase(NULL)
	{
	}

	T Get()
	{
		uk pSpecific(GetSpecific());
		return *reinterpret_cast<const T*>(&pSpecific);
	}

	void Set(const T& kValue)
	{
		SetSpecific(*reinterpret_cast<ukk const*>(&kValue));
	}
};

template<typename T> struct SDrxPthreadTLSImpl<T, false> : private SDrxPthreadTLSBase
{
	SDrxPthreadTLSImpl()
		: SDrxPthreadTLSBase(&Destroy)
	{
	}

	T* GetPtr()
	{
		T* pPtr(static_cast<T*>(GetSpecific()));
		if (pPtr == NULL)
		{
			pPtr = new T();
			SetSpecific(pPtr);
		}
		return pPtr;
	}

	static void Destroy(uk pPointer)
	{
		delete static_cast<T*>(pPointer);
	}

	const T& Get()
	{
		return *GetPtr();
	}

	void Set(const T& kValue)
	{
		*GetPtr() = kValue;
	}
};

template<typename T>
struct SDrxPthreadTLS : SDrxPthreadTLSImpl<T, sizeof(T) <= sizeof(uk )>
{
};

//! DrxEvent(Timed) represent a synchronization event.
class DrxEventTimed
{
public:
	DrxEventTimed() { m_flag = false; }
	~DrxEventTimed() {}

	//! Reset the event to the unsignaled state.
	void Reset();

	//! Set the event to the signaled state.
	void Set();

	//! Access a HANDLE to wait on.
	uk GetHandle() const { return NULL; };

	//! Wait indefinitely for the object to become signalled.
	void Wait();

	//! Wait, with a time limit, for the object to become signalled.
	bool Wait(u32k timeoutMillis);

private:
	//! Lock for synchronization of notifications.
	DrxCriticalSection   m_lockNotify;
	DrxConditionVariable m_cond;
	 bool        m_flag;
};

typedef DrxEventTimed DrxEvent;

///////////////////////////////////////////////////////////////////////////////
namespace DrxMT {

//////////////////////////////////////////////////////////////////////////
inline void DrxMemoryBarrier()
{
	MemoryBarrier();
}

//////////////////////////////////////////////////////////////////////////
inline void DrxYieldThread()
{
	sched_yield();
}

} //DrxMT
