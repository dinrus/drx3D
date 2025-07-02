// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// Include basic multithread primitives.
#include "DrxAtomics.h"
#include <drx3D/CoreX/BitFiddling.h>

class DrxConditionVariable;
class DrxSemaphore;
class DrxFastSemaphore;
class DrxRWLock;

#define THREAD_NAME_LENGTH_MAX 64

enum DrxLockType
{
	DRXLOCK_FAST      = 1,  //!< Потенциально быстрый замок (нерекурсивный).
	DRXLOCK_RECURSIVE = 2,  //!< Рекурсивный замок.
};

//! Primitive locks and conditions.
//! Primitive locks are represented by instance of class DrxLockT<Type>.
template<DrxLockType Type> class DrxLockT
{
	/* Unsupported lock type. */
};

//////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////
typedef DrxLockT<DRXLOCK_RECURSIVE> DrxCriticalSection;
typedef DrxLockT<DRXLOCK_FAST>      DrxCriticalSectionNonRecursive;

//////////////////////////////////////////////////////////////////////////
//! DrxAutoCriticalSection implements a helper class to automatically.
//! lock critical section in constructor and release on destructor.
template<class LockClass> class DrxAutoLock
{
public:
	DrxAutoLock() = delete;
	DrxAutoLock(const DrxAutoLock<LockClass>&) = delete;
	DrxAutoLock<LockClass>& operator=(const DrxAutoLock<LockClass>&) = delete;

	DrxAutoLock(LockClass& Lock) : m_pLock(&Lock) { m_pLock->Lock(); }
	DrxAutoLock(const LockClass& Lock) : m_pLock(const_cast<LockClass*>(&Lock)) { m_pLock->Lock(); }
	~DrxAutoLock() { m_pLock->Unlock(); }
private:
	LockClass* m_pLock;
};

template<class LockClass> class DrxAutoReadLock
{
public:
	DrxAutoReadLock() = delete;
	DrxAutoReadLock(const DrxAutoReadLock<LockClass>&) = delete;
	DrxAutoReadLock<LockClass>& operator=(const DrxAutoReadLock<LockClass>&) = delete;

	DrxAutoReadLock(LockClass& Lock) : m_pLock(&Lock) { m_pLock->RLock(); }
	DrxAutoReadLock(const LockClass& Lock) : m_pLock(const_cast<LockClass*>(&Lock)) { m_pLock->RLock(); }
	~DrxAutoReadLock() { m_pLock->RUnlock(); }
private:
	LockClass* m_pLock;
};

template<class LockClass> class DrxAutoWriteLock
{
public:
	DrxAutoWriteLock() = delete;
	DrxAutoWriteLock(const DrxAutoWriteLock<LockClass>&) = delete;
	DrxAutoWriteLock<LockClass>& operator=(const DrxAutoWriteLock<LockClass>&) = delete;

	DrxAutoWriteLock(LockClass& Lock) : m_pLock(&Lock) { m_pLock->WLock(); }
	DrxAutoWriteLock(const LockClass& Lock) : m_pLock(const_cast<LockClass*>(&Lock)) { m_pLock->WLock(); }
	~DrxAutoWriteLock() { m_pLock->WUnlock(); }
private:
	LockClass* m_pLock;
};


//! DrxOptionalAutoLock implements a helper class to automatically.
//! Lock critical section (if needed) in constructor and release on destructor.
template<class LockClass> class DrxOptionalAutoLock
{
private:
	LockClass* m_Lock;
	bool       m_bLockAcquired;

	DrxOptionalAutoLock();
	DrxOptionalAutoLock(const DrxOptionalAutoLock<LockClass>&);
	DrxOptionalAutoLock<LockClass>& operator=(const DrxOptionalAutoLock<LockClass>&);

public:
	DrxOptionalAutoLock(LockClass& Lock, bool acquireLock) : m_Lock(&Lock), m_bLockAcquired(false)
	{
		if (acquireLock)
		{
			Acquire();
		}
	}
	~DrxOptionalAutoLock()
	{
		Release();
	}
	void Release()
	{
		if (m_bLockAcquired)
		{
			m_Lock->Unlock();
			m_bLockAcquired = false;
		}
	}
	void Acquire()
	{
		if (!m_bLockAcquired)
		{
			m_Lock->Lock();
			m_bLockAcquired = true;
		}
	}
};

//! DrxAutoSet implements a helper class to automatically.
//! set and reset value in constructor and release on destructor.
template<class ValueClass> class DrxAutoSet
{
private:
	ValueClass* m_pValue;

	DrxAutoSet();
	DrxAutoSet(const DrxAutoSet<ValueClass>&);
	DrxAutoSet<ValueClass>& operator=(const DrxAutoSet<ValueClass>&);

public:
	DrxAutoSet(ValueClass& value) : m_pValue(&value) { *m_pValue = (ValueClass)1; }
	~DrxAutoSet() { *m_pValue = (ValueClass)0; }
};

//! Auto critical section is the most commonly used type of auto lock.
typedef DrxAutoLock<DrxCriticalSection>             DrxAutoCriticalSection;
typedef DrxAutoLock<DrxCriticalSectionNonRecursive> DrxAutoCriticalSectionNoRecursive;

#define AUTO_LOCK_T(Type, lock) PREFAST_SUPPRESS_WARNING(6246); DrxAutoLock<Type> __AutoLock(lock)
#define AUTO_LOCK(lock)         AUTO_LOCK_T(DrxCriticalSection, lock)
#define AUTO_LOCK_CS(csLock)    DrxAutoCriticalSection __AL__ ## csLock(csLock)

///////////////////////////////////////////////////////////////////////////////
//! Base class for lockless Producer/Consumer queue, due platforms specific they are implemented in DrxThead_platform.h.
namespace DrxMT {
namespace detail {

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SingleProducerSingleConsumerQueueBase
{
public:
	SingleProducerSingleConsumerQueueBase()
	{}

	void Push(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex, u32 nBufferSize, uk arrBuffer, u32 nObjectSize);
	void Pop(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex, u32 nBufferSize, uk arrBuffer, u32 nObjectSize);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class N_ProducerSingleConsumerQueueBase
{
public:
	N_ProducerSingleConsumerQueueBase()
	{
		DrxInitializeSListHead(fallbackList);
	}

	void Push(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex,  u32& rRunning, uk arrBuffer, u32 nBufferSize, u32 nObjectSize,  u32* arrStates);
	bool Pop(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex,  u32& rRunning, uk arrBuffer, u32 nBufferSize, u32 nObjectSize,  u32* arrStates);

private:
	SLockFreeSingleLinkedListHeader fallbackList;
	struct SFallbackList
	{
		SLockFreeSingleLinkedListEntry nextEntry;
		char                           alignment_padding[128 - sizeof(SLockFreeSingleLinkedListEntry)];
		char                           object[1];         //!< Struct will be overallocated with enough memory for the object
	};
};

} // namespace detail
} // namespace DrxMT

//////////////////////////////////////////////////////////////////////////
namespace DrxMT {
	void DrxMemoryBarrier();
	void DrxYieldThread();
} // namespace DrxMT

// Include architecture specific code.
#if DRX_PLATFORM_WINAPI
	#include <drx3D/CoreX/Thread/DrxThread_win32.h>
#elif DRX_PLATFORM_POSIX
	#include <drx3D/CoreX/Thread/DrxThread_posix.h>
#else
// Put other platform specific includes here!
	#include <drx3D/CoreX/Thread/DrxThread_dummy.h>
#endif

//! Sync primitive for multiple reads and exclusive locking change access.
//! Useful in case if you have rarely modified object that needs
//! to be read quite often from different threads but still
//! need to be exclusively modified sometimes.
//! Debug functionality:.
//! Can be used for debug-only lock calls, which verify that no
//! simultaneous access is attempted.
//! Use the bDebug argument of LockRead or LockModify,
//! or use the DEBUG_READLOCK or DEBUG_MODIFYLOCK macros.
//! There is no overhead in release builds, if you use the macros,
//! and the lock definition is inside #ifdef _DEBUG.
class DrxReadModifyLock
{
public:
	DrxReadModifyLock()
		: m_readCount(0), m_modifyCount(0)
	{
		SetDebugLocked(false);
	}

	bool LockRead(bool bTry = false, cstr strDebug = 0, bool bDebug = false) const
	{
		if (!WriteLock(bTry, bDebug, strDebug))     // wait until write unlocked
			return false;
		DrxInterlockedIncrement(&m_readCount);      // increment read counter
		m_writeLock.Unlock();
		return true;
	}
	void UnlockRead() const
	{
		SetDebugLocked(false);
		i32k counter = DrxInterlockedDecrement(&m_readCount);     // release read
		assert(counter >= 0);
		if (m_writeLock.TryLock())
			m_writeLock.Unlock();
		else if (counter == 0 && m_modifyCount)
			m_ReadReleased.Set();                     // signal the final read released
	}
	bool LockModify(bool bTry = false, cstr strDebug = 0, bool bDebug = false) const
	{
		if (!WriteLock(bTry, bDebug, strDebug))
			return false;
		DrxInterlockedIncrement(&m_modifyCount);    // increment write counter (counter is for nested cases)
		while (m_readCount)
			m_ReadReleased.Wait();                    // wait for all threads finish read operation
		return true;
	}
	void UnlockModify() const
	{
		SetDebugLocked(false);
		i32 counter = DrxInterlockedDecrement(&m_modifyCount);    // decrement write counter
		assert(counter >= 0);
		m_writeLock.Unlock();                       // release exclusive lock
	}

protected:
	mutable  i32       m_readCount;
	mutable  i32       m_modifyCount;
	mutable DrxEvent           m_ReadReleased;
	mutable DrxCriticalSection m_writeLock;
	mutable bool               m_debugLocked;
	mutable tukk        m_debugLockStr;

	void SetDebugLocked(bool b, tukk str = 0) const
	{
#ifdef _DEBUG
		m_debugLocked = b;
		m_debugLockStr = str;
#endif
	}

	bool WriteLock(bool bTry, bool bDebug, tukk strDebug) const
	{
		if (!m_writeLock.TryLock())
		{
#ifdef _DEBUG
			assert(!m_debugLocked);
			assert(!bDebug);
#endif
			if (bTry)
				return false;
			m_writeLock.Lock();
		}
#ifdef _DEBUG
		if (!m_readCount && !m_modifyCount)         // not yet locked
			SetDebugLocked(bDebug, strDebug);
#endif
		return true;
	}
};

//! Auto-locking classes.
template<class T, bool bDEBUG = false>
class AutoLockRead
{
protected:
	const T& m_lock;
public:
	AutoLockRead(const T& lock, cstr strDebug = 0)
		: m_lock(lock) { m_lock.LockRead(bDEBUG, strDebug, bDEBUG); }
	~AutoLockRead()
	{ m_lock.UnlockRead(); }
};

template<class T, bool bDEBUG = false>
class AutoLockModify
{
protected:
	const T& m_lock;
public:
	AutoLockModify(const T& lock, cstr strDebug = 0)
		: m_lock(lock) { m_lock.LockModify(bDEBUG, strDebug, bDEBUG); }
	~AutoLockModify()
	{ m_lock.UnlockModify(); }
};

#define AUTO_READLOCK(p)      PREFAST_SUPPRESS_WARNING(6246) AutoLockRead<DrxReadModifyLock> __readlock ## __LINE__(p, __FUNC__)
#define AUTO_READLOCK_PROT(p) PREFAST_SUPPRESS_WARNING(6246) AutoLockRead<DrxReadModifyLock> __readlock_prot ## __LINE__(p, __FUNC__)
#define AUTO_MODIFYLOCK(p)    PREFAST_SUPPRESS_WARNING(6246) AutoLockModify<DrxReadModifyLock> __modifylock ## __LINE__(p, __FUNC__)

#if defined(_DEBUG)
	#define DEBUG_READLOCK(p)   AutoLockRead<DrxReadModifyLock> __readlock ## __LINE__(p, __FUNC__)
	#define DEBUG_MODIFYLOCK(p) AutoLockModify<DrxReadModifyLock> __modifylock ## __LINE__(p, __FUNC__)
#else
	#define DEBUG_READLOCK(p)
	#define DEBUG_MODIFYLOCK(p)
#endif

///////////////////////////////////////////////////////////////////////////////
//! Base class for lockless Producer/Consumer queue, due platforms specific they are implemented in DrxThead_platform.h.
namespace DrxMT {
namespace detail {

///////////////////////////////////////////////////////////////////////////////
inline void SingleProducerSingleConsumerQueueBase::Push(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex, u32 nBufferSize, uk arrBuffer, u32 nObjectSize)
{
	// spin if queue is full
	CSimpleThreadBackOff backoff;
	while (rProducerIndex - rConsumerIndex == nBufferSize)
	{
		backoff.backoff();
	}

	DrxMT::DrxMemoryBarrier();
	tuk pBuffer = alias_cast<tuk>(arrBuffer);
	u32 nIndex = rProducerIndex % nBufferSize;

	memcpy(pBuffer + (nIndex * nObjectSize), pObj, nObjectSize);
	DrxMT::DrxMemoryBarrier();
	rProducerIndex += 1;
	DrxMT::DrxMemoryBarrier();
}

///////////////////////////////////////////////////////////////////////////////
inline  void SingleProducerSingleConsumerQueueBase::Pop(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex, u32 nBufferSize, uk arrBuffer, u32 nObjectSize)
{
	DrxMT::DrxMemoryBarrier();
	// busy-loop if queue is empty
	CSimpleThreadBackOff backoff;
	while (rProducerIndex - rConsumerIndex == 0)
	{
		backoff.backoff();
	}

	tuk pBuffer = alias_cast<tuk>(arrBuffer);
	u32 nIndex = rConsumerIndex % nBufferSize;

	memcpy(pObj, pBuffer + (nIndex * nObjectSize), nObjectSize);
	DrxMT::DrxMemoryBarrier();
	rConsumerIndex += 1;
	DrxMT::DrxMemoryBarrier();
}

///////////////////////////////////////////////////////////////////////////////
inline  void N_ProducerSingleConsumerQueueBase::Push(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex,  u32& rRunning, uk arrBuffer, u32 nBufferSize, u32 nObjectSize,  u32* arrStates)
{
	DrxMT::DrxMemoryBarrier();
	u32 nProducerIndex;
	u32 nConsumerIndex;

	i32 iter = 0;
	CSimpleThreadBackOff backoff;
	do
	{
		nProducerIndex = rProducerIndex;
		nConsumerIndex = rConsumerIndex;

		if (nProducerIndex - nConsumerIndex == nBufferSize)
		{
			if (iter++ > CSimpleThreadBackOff::kHardYieldInterval)
			{
				u32 nSizeToAlloc = sizeof(SFallbackList) + nObjectSize - 1;
				SFallbackList* pFallbackEntry = (SFallbackList*)DrxModuleMemalign(nSizeToAlloc, 128);
				memcpy(pFallbackEntry->object, pObj, nObjectSize);
				DrxMT::DrxMemoryBarrier();
				DrxInterlockedPushEntrySList(fallbackList, pFallbackEntry->nextEntry);
				return;
			}
			backoff.backoff();
			continue;
		}

		if (DrxInterlockedCompareExchange(alias_cast< LONG*>(&rProducerIndex), nProducerIndex + 1, nProducerIndex) == nProducerIndex)
			break;
	}
	while (true);

	DrxMT::DrxMemoryBarrier();
	tuk pBuffer = alias_cast<tuk>(arrBuffer);
	u32 nIndex = nProducerIndex % nBufferSize;

	memcpy(pBuffer + (nIndex * nObjectSize), pObj, nObjectSize);
	DrxMT::DrxMemoryBarrier();
	arrStates[nIndex] = 1;
	DrxMT::DrxMemoryBarrier();
}

///////////////////////////////////////////////////////////////////////////////
inline  bool N_ProducerSingleConsumerQueueBase::Pop(uk pObj,  u32& rProducerIndex,  u32& rConsumerIndex,  u32& rRunning, uk arrBuffer, u32 nBufferSize, u32 nObjectSize,  u32* arrStates)
{
	DrxMT::DrxMemoryBarrier();

	// busy-loop if queue is empty
	CSimpleThreadBackOff backoff;
	if (rRunning && rProducerIndex - rConsumerIndex == 0)
	{
		while (rRunning && rProducerIndex - rConsumerIndex == 0)
		{
			backoff.backoff();
		}
	}

	if (rRunning == 0 && rProducerIndex - rConsumerIndex == 0)
	{
		SFallbackList* pFallback = (SFallbackList*)DrxInterlockedPopEntrySList(fallbackList);
		IF (pFallback, 0)
		{
			memcpy(pObj, pFallback->object, nObjectSize);
			DrxModuleMemalignFree(pFallback);
			return true;
		}
		// if the queue was empty, make sure we really are empty
		return false;
	}

	backoff.reset();
	while (arrStates[rConsumerIndex % nBufferSize] == 0)
	{
		backoff.backoff();
	}

	tuk pBuffer = alias_cast<tuk>(arrBuffer);
	u32 nIndex = rConsumerIndex % nBufferSize;

	memcpy(pObj, pBuffer + (nIndex * nObjectSize), nObjectSize);
	DrxMT::DrxMemoryBarrier();
	arrStates[nIndex] = 0;
	DrxMT::DrxMemoryBarrier();
	rConsumerIndex += 1;
	DrxMT::DrxMemoryBarrier();

	return true;
}

} // namespace detail
} // namespace DrxMT

// Include all multithreading containers.
#include "MultiThread_Containers.h"