// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Interlocked API
//////////////////////////////////////////////////////////////////////////

// Returns the resulting incremented value
LONG DrxInterlockedIncrement(i32 * pDst);

// Returns the resulting decremented value
LONG DrxInterlockedDecrement(i32 * pDst);

// Returns the resulting added value
LONG DrxInterlockedAdd( LONG* pVal, LONG add);

// Returns the resulting added value
int64 DrxInterlockedAdd( int64* pVal, int64 add);

// Returns the resulting added value
size_t DrxInterlockedAdd( size_t* pVal, size_t add);

//////////////////////////////////////////////////////////////////////////
// Returns initial value prior exchange
LONG DrxInterlockedExchange( LONG* pDst, LONG exchange);

// Returns initial value prior exchange
int64 DrxInterlockedExchange64( int64* addr, int64 exchange);

// Returns initial value prior exchange
LONG DrxInterlockedExchangeAdd( LONG* pDst, LONG value);

// Returns initial value prior exchange
size_t DrxInterlockedExchangeAdd( size_t* pDst, size_t value);

// Returns initial value prior exchange
LONG DrxInterlockedExchangeAnd( LONG* pDst, LONG value);

// Returns initial value prior exchange
LONG DrxInterlockedExchangeOr( LONG* pDst, LONG value);

// Returns initial value prior exchange
uk DrxInterlockedExchangePointer(uk * pDst, uk pExchange);

//////////////////////////////////////////////////////////////////////////
// Returns initial value prior exchange
LONG DrxInterlockedCompareExchange( LONG* pDst, LONG exchange, LONG comperand);

// Returns initial value prior exchange
int64 DrxInterlockedCompareExchange64( int64* pDst, int64 exchange, int64 comperand);

#if DRX_PLATFORM_64BIT
// Returns initial value prior exchange
u8 DrxInterlockedCompareExchange128( int64* pDst, int64 exchangeHigh, int64 exchangeLow, int64* comparandResult);
#endif

// Returns initial address prior exchange
uk DrxInterlockedCompareExchangePointer(uk * pDst, uk pExchange, uk pComperand);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DrxInterlocked*SList Function, these are specialized C-A-S
// functions for single-linked lists which prevent the A-B-A problem there
// there are implemented in the platform specific DrxThread_*.h files
//NOTE: The sizes are verified at compile-time in the implementation functions, but this is still ugly

#if DRX_PLATFORM_64BIT
	#define LOCK_FREE_LINKED_LIST_DOUBLE_SIZE_PTR_ALIGN 16
#elif DRX_PLATFORM_32BIT
	#define LOCK_FREE_LINKED_LIST_DOUBLE_SIZE_PTR_ALIGN 8
#else
	#error "Неподдерживаемая платформа"
#endif

struct SLockFreeSingleLinkedListEntry
{
	DRX_ALIGN(LOCK_FREE_LINKED_LIST_DOUBLE_SIZE_PTR_ALIGN) SLockFreeSingleLinkedListEntry *  pNext;
};
static_assert(std::alignment_of<SLockFreeSingleLinkedListEntry>::value == sizeof(uintptr_t) * 2, "Alignment failure for SLockFreeSingleLinkedListEntry");

struct SLockFreeSingleLinkedListHeader
{
	//! Initializes the single-linked list.
	friend void DrxInitializeSListHead(SLockFreeSingleLinkedListHeader& list);

	//! Push one element atomically onto the front of a single-linked list.
	friend void DrxInterlockedPushEntrySList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& element);

	//! Push a list of elements atomically onto the front of a single-linked list.
	//! \note The entries must already be linked (ie, last must be reachable by moving through pNext starting at first).
	friend void DrxInterlockedPushListSList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& first, SLockFreeSingleLinkedListEntry& last, u32 count);

	//! Retrieves a pointer to the first item on a single-linked list.
	//! \note This does not remove the item from the list, and it's unsafe to inspect anything but the returned address.
	friend uk DrxRtlFirstEntrySList(SLockFreeSingleLinkedListHeader& list);

	//! Pops one element atomically from the front of a single-linked list, and returns a pointer to the item.
	//! \note If the list was empty, nullptr is returned instead.
	friend uk DrxInterlockedPopEntrySList(SLockFreeSingleLinkedListHeader& list);

	//! Flushes the entire single-linked list, and returns a pointer to the first item that was on the list.
	//! \note If the list was empty, nullptr is returned instead.
	friend uk DrxInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list);

private:
	DRX_ALIGN(LOCK_FREE_LINKED_LIST_DOUBLE_SIZE_PTR_ALIGN) SLockFreeSingleLinkedListEntry *  pNext;

#if DRX_PLATFORM_ORBIS
	// Only need "salt" on platforms using CAS (ORBIS uses embedded salt)
#elif DRX_PLATFORM_POSIX
	// If pointers 32bit, salt should be as well. Otherwise we get 4 bytes of padding between pNext and salt and CAS operations fail
	#if DRX_PLATFORM_64BIT
	 uint64 salt;
	#else
	 u32 salt;
	#endif
#endif
};
static_assert(std::alignment_of<SLockFreeSingleLinkedListHeader>::value == sizeof(uintptr_t) * 2, "Alignment failure for SLockFreeSingleLinkedListHeader");
#undef LOCK_FREE_LINKED_LIST_DOUBLE_SIZE_PTR_ALIGN

#if DRX_PLATFORM_WINAPI
	#include "DrxAtomics_win32.h"
#elif DRX_PLATFORM_ORBIS
	#include "DrxAtomics_sce.h"
#elif DRX_PLATFORM_POSIX
	#include "DrxAtomics_posix.h"
#endif

#define WRITE_LOCK_VAL (1 << 16)

uk      DrxCreateCriticalSection();
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
void       DrxCreateCriticalSectionInplace(uk );
#endif
void       DrxDeleteCriticalSection(uk cs);
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
void       DrxDeleteCriticalSectionInplace(uk cs);
#endif
void       DrxEnterCriticalSection(uk cs);
bool       DrxTryCriticalSection(uk cs);
void       DrxLeaveCriticalSection(uk cs);

ILINE void DrxSpinLock( i32* pLock, i32 checkVal, i32 setVal)
{
	static_assert(sizeof(i32) == sizeof(LONG), "Unsecured cast. Int is not same size as LONG.");
	CSimpleThreadBackOff threadBackoff;
	while (DrxInterlockedCompareExchange(( LONG*)pLock, setVal, checkVal) != checkVal)
	{
		threadBackoff.backoff();
	}
}

ILINE void DrxReleaseSpinLock( i32* pLock, i32 setVal)
{
	*pLock = setVal;
}

ILINE void DrxReadLock( i32* rw)
{
	DrxInterlockedAdd(rw, 1);
#ifdef NEED_ENDIAN_SWAP
	 tuk pw = ( tuk)rw + 1;
#else
	 tuk pw = ( tuk)rw + 2;
#endif

	CSimpleThreadBackOff backoff;
	for (; *pw; )
	{
		backoff.backoff();
	}
}

ILINE void DrxReleaseReadLock( i32* rw)
{
	DrxInterlockedAdd(rw, -1);
}

ILINE void DrxWriteLock( i32* rw)
{
	DrxSpinLock(rw, 0, WRITE_LOCK_VAL);
}

ILINE void DrxReleaseWriteLock( i32* rw)
{
	DrxInterlockedAdd(rw, -WRITE_LOCK_VAL);
}

//////////////////////////////////////////////////////////////////////////
struct ReadLock
{
	ReadLock( i32& rw) : prw(&rw)
	{
		DrxReadLock(prw);
	}

	~ReadLock()
	{
		DrxReleaseReadLock(prw);
	}

private:
	 i32* const prw;
};

struct ReadLockCond
{
	ReadLockCond( i32& rw, i32 bActive) : iActive(0),prw(&rw)
	{
		if (bActive)
		{
			iActive = 1;
			DrxReadLock(prw);
		}
	}

	void SetActive(i32 bActive = 1)
	{
		iActive = bActive;
	}

	void Release()
	{
		DrxInterlockedAdd(prw, -iActive);
	}

	~ReadLockCond()
	{
		DrxInterlockedAdd(prw, -iActive);
	}

private:
	i32                 iActive;
	 i32* const prw;
};

//////////////////////////////////////////////////////////////////////////
struct WriteLock
{
	WriteLock( i32& rw) : prw(&rw)
	{
		DrxWriteLock(&rw);
	}

	~WriteLock()
	{
		DrxReleaseWriteLock(prw);
	}

private:
	 i32* const prw;
};

//////////////////////////////////////////////////////////////////////////
struct WriteAfterReadLock
{
	WriteAfterReadLock( i32& rw) : prw(&rw)
	{
		DrxSpinLock(&rw, 1, WRITE_LOCK_VAL + 1);
	}

	~WriteAfterReadLock()
	{
		DrxInterlockedAdd(prw, -WRITE_LOCK_VAL);
	}

private:
	 i32* const prw;
};

//////////////////////////////////////////////////////////////////////////
struct WriteLockCond
{
	WriteLockCond( i32& rw, i32 bActive = 1) : iActive(0), prw(&rw)
	{
		if (bActive)
		{
			iActive = WRITE_LOCK_VAL;
			DrxSpinLock(prw, 0, iActive);
		}
	}

	WriteLockCond() : iActive(0), prw(&iActive) {}

	~WriteLockCond()
	{
		DrxInterlockedAdd(prw, -iActive);
	}

	void SetActive(i32 bActive = 1)
	{
		iActive = -bActive & WRITE_LOCK_VAL;
	}

	void Release()
	{
		DrxInterlockedAdd(prw, -iActive);
	}

	i32           iActive; //!< Not private because used directly in Physics RWI.
	 i32* prw;     //!< Not private because used directly in Physics RWI.
};

//////////////////////////////////////////////////////////////////////////
#if defined(EXCLUDE_PHYSICS_THREAD)
ILINE void SpinLock( i32* pLock, i32 checkVal, i32 setVal)
{
	*(i32*)pLock = setVal;
}
ILINE void AtomicAdd( i32* pVal, i32 iAdd)                    { *(i32*)pVal += iAdd; }
ILINE void AtomicAdd( u32* pVal, i32 iAdd)           { *(u32*)pVal += iAdd; }

ILINE void JobSpinLock( i32* pLock, i32 checkVal, i32 setVal) { DrxSpinLock(pLock, checkVal, setVal); }
#else
ILINE void SpinLock( i32* pLock, i32 checkVal, i32 setVal)
{
	DrxSpinLock(pLock, checkVal, setVal);
}
ILINE void AtomicAdd( i32* pVal, i32 iAdd)                    { DrxInterlockedAdd(pVal, iAdd); }
ILINE void AtomicAdd( u32* pVal, i32 iAdd)           { DrxInterlockedAdd(( i32*)pVal, iAdd); }

ILINE void JobSpinLock( i32* pLock, i32 checkVal, i32 setVal) { SpinLock(pLock, checkVal, setVal); }
#endif

ILINE void JobAtomicAdd( i32* pVal, i32 iAdd)
{
	DrxInterlockedAdd(pVal, iAdd);
}
ILINE void JobAtomicAdd( u32* pVal, i32 iAdd) { DrxInterlockedAdd(( i32*)pVal, iAdd); }