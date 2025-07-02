// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <sched.h>
#include <drx3D/CoreX/Assert/DrxAssert.h>

//////////////////////////////////////////////////////////////////////////
// Interlocked API
//////////////////////////////////////////////////////////////////////////

// Возвращает итоговое инкрементированное значение
ILINE LONG DrxInterlockedIncrement( i32* pDst)
{
	static_assert(sizeof(i32) == sizeof(LONG), "Небезопасное приведение. i32 отличается размером от LONG.");
	return __sync_add_and_fetch(pDst, 1);
}

// Возвращает итоговое декрементированное значение
ILINE LONG DrxInterlockedDecrement( i32* pDst)
{
	static_assert(sizeof(i32) == sizeof(LONG), "Небезопасное приведение. i32 отличается размером от LONG.");
	return __sync_sub_and_fetch(pDst, 1);
}

// Возвращает итоговое прибавленное значение
ILINE LONG DrxInterlockedAdd( LONG* pDst, LONG add)
{
	return __sync_add_and_fetch(pDst, add);
}

// Возвращает итоговое прибавленное значение
ILINE int64 DrxInterlockedAdd( int64* pDst, int64 add)
{
	return __sync_add_and_fetch(pDst, add);
}

// Возвращает итоговое прибавленное значение
ILINE size_t DrxInterlockedAdd( size_t* pDst, size_t add)
{
	return __sync_add_and_fetch(pDst, add);
}

// Возвращает начальное значение до обмена
ILINE LONG DrxInterlockedExchange( LONG* pDst, LONG exchange)
{
#if !(DRX_PLATFORM_X64 || DRX_PLATFORM_X86)
	__sync_synchronize(); // следует стандарту X64 и гарантирует полный барьер памяти (также добавляет mfence)
#endif
	return __sync_lock_test_and_set(pDst, exchange); // only creates acquire memory barrier

}

// Возвращает начальное значение до обмена
ILINE int64 DrxInterlockedExchange64( int64* addr, int64 exchange)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(addr, exchange);
}

// Возвращает начальное значение до обмена
ILINE LONG DrxInterlockedExchangeAdd( LONG* pDst, LONG value)
{
	return __sync_fetch_and_add(pDst, value);
}

// Возвращает начальное значение до обмена
ILINE size_t DrxInterlockedExchangeAdd( size_t* pDst, size_t add)
{
	return __sync_fetch_and_add(pDst, add);
}

// Возвращает начальное значение до обмена
ILINE LONG DrxInterlockedExchangeAnd( LONG* pDst, LONG value)
{
	return __sync_fetch_and_and(pDst, value);
}

// Возвращает начальное значение до обмена
ILINE LONG DrxInterlockedExchangeOr( LONG* pDst, LONG value)
{
	return __sync_fetch_and_or(pDst, value);
}

// Возвращает начальный адрес до обмена
ILINE uk DrxInterlockedExchangePointer(uk * pDst, uk pExchange)
{
	__sync_synchronize();                             // follow X86/X64 standard and ensure full memory barrier
	return __sync_lock_test_and_set(pDst, pExchange); // only creates acquire memory barrier
}

// Возвращает начальный адрес до обмена
ILINE LONG DrxInterlockedCompareExchange( LONG* pDst, LONG exchange, LONG comperand)
{
	return __sync_val_compare_and_swap(pDst, comperand, exchange);
}

// Возвращает начальный адрес до обмена
ILINE int64 DrxInterlockedCompareExchange64( int64* addr, int64 exchange, int64 comperand)
{
	return __sync_val_compare_and_swap(addr, comperand, exchange);
}

#if DRX_PLATFORM_64BIT
// Возвращает начальный адрес до обмена
// Chipset needs to support cmpxchg16b which most do
//https://blog.lse.epita.fr/articles/42-implementing-generic-double-word-compare-and-swap-.html
ILINE u8 DrxInterlockedCompareExchange128( int64* pDst, int64 exchangehigh, int64 exchangelow, int64* pComparandResult)
{
	#if DRX_PLATFORM_IOS
		#error Ensure DrxInterlockedCompareExchange128 is working on IOS also
	#endif
	DRX_ASSERT_MESSAGE((((int64)pDst) & 15) == 0, "The destination data must be 16-byte aligned to avoid a general protection fault.");
	#if DRX_PLATFORM_X64 || DRX_PLATFORM_X86
		bool bEquals;
		__asm__ __volatile__(
		"lock cmpxchg16b %1\n\t"
		"setz %0"
		: "=q" (bEquals), "+m" (*pDst), "+d" (pComparandResult[1]), "+a" (pComparandResult[0])
		: "c" (exchangehigh), "b" (exchangelow)
		: "cc");
		return (char)bEquals;
	#else
		// Use lock for targeted CPU architecture that does not support DCAS/CAS2/MCAS
		static pthread_mutex_t mutex;
		bool bResult = false;
		pthread_mutex_lock (&mutex);
		if (pDst[0] == pComparandResult[0] && pDst[1] == pComparandResult[1])
		{
			pDst[0] = exchangelow;
			pDst[1] = exchangehigh;
			bResult = true;
		}
		else
		{
			pComparandResult[0] = pDst[0];
			pComparandResult[1] = pDst[1];
		}
		pthread_mutex_unlock (&mutex);
		return bResult;
	#endif
}
#endif

// Возвращает начальный адрес до обмена
ILINE uk DrxInterlockedCompareExchangePointer(uk * pDst, uk pExchange, uk pComperand)
{
	return __sync_val_compare_and_swap(pDst, pComperand, pExchange);
}

#if DRX_PLATFORM_64BIT
//////////////////////////////////////////////////////////////////////////
// Linux 64-bit implementation of lockless single-linked list
//////////////////////////////////////////////////////////////////////////
typedef __uint128_t uint128;

//////////////////////////////////////////////////////////////////////////
// Implementation for Linux64 with gcc using __int128_t
//////////////////////////////////////////////////////////////////////////
ILINE void DrxInterlockedPushEntrySList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& element)
{
	uint64 curSetting[2];
	uint64 newSetting[2];
	uint64 newPointer = (uint64) & element;
	do
	{
		curSetting[0] = (uint64)list.pNext;
		curSetting[1] = list.salt;
		element.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
		newSetting[0] = newPointer;        // new pointer
		newSetting[1] = curSetting[1] + 1; // new salt
	}
	// while (false == __sync_bool_compare_and_swap( ( uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	while (0 == DrxInterlockedCompareExchange128(( int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0]));
}

//////////////////////////////////////////////////////////////////////////
ILINE void DrxInterlockedPushListSList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& first, SLockFreeSingleLinkedListEntry& last, u32 count)
{
	(void)count; // unused

	uint64 curSetting[2];
	uint64 newSetting[2];
	uint64 newPointer = (uint64) & first;
	do
	{
		curSetting[0] = (uint64)list.pNext;
		curSetting[1] = list.salt;
		last.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
		newSetting[0] = newPointer;        // new pointer
		newSetting[1] = curSetting[1] + 1; // new salt
	}
	while (0 == DrxInterlockedCompareExchange128(( int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0]));
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxInterlockedPopEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	uint64 curSetting[2];
	uint64 newSetting[2];
	do
	{
		curSetting[1] = list.salt;
		curSetting[0] = (uint64)list.pNext;
		if (curSetting[0] == 0)
			return NULL;
		newSetting[0] = *(uint64*)curSetting[0]; // new pointer
		newSetting[1] = curSetting[1] + 1;       // new salt
	}
	//while (false == __sync_bool_compare_and_swap( ( uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	while (0 == DrxInterlockedCompareExchange128(( int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0]));
	return (uk )curSetting[0];
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxRtlFirstEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	return list.pNext;
}

//////////////////////////////////////////////////////////////////////////
ILINE void DrxInitializeSListHead(SLockFreeSingleLinkedListHeader& list)
{
	list.salt = 0;
	list.pNext = NULL;
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list)
{
	uint64 curSetting[2];
	uint64 newSetting[2];
	//uint64 newSalt;
	//uint64 newPointer;
	do
	{
		curSetting[1] = list.salt;
		curSetting[0] = (uint64)list.pNext;
		if (curSetting[0] == 0)
			return NULL;
		newSetting[0] = 0;
		newSetting[1] = curSetting[1] + 1;
	}
	//	while (false == __sync_bool_compare_and_swap( ( uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	while (0 == DrxInterlockedCompareExchange128(( int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0]));
	return (uk )curSetting[0];
}
//////////////////////////////////////////////////////////////////////////
#elif DRX_PLATFORM_32BIT
//////////////////////////////////////////////////////////////////////////
// Implementation for Linux32 with gcc using uint64
//////////////////////////////////////////////////////////////////////////
ILINE void DrxInterlockedPushEntrySList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& element)
{
	u32 curSetting[2];
	u32 newSetting[2];
	u32 newPointer = (u32) & element;
	do
	{
		curSetting[0] = (u32)list.pNext;
		curSetting[1] = list.salt;
		element.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
		newSetting[0] = newPointer;        // new pointer
		newSetting[1] = curSetting[1] + 1; // new salt
	}
	while (false == __sync_bool_compare_and_swap(( uint64*)&list.pNext, *(uint64*)&curSetting[0], *(uint64*)&newSetting[0]));
}

//////////////////////////////////////////////////////////////////////////
ILINE void DrxInterlockedPushListSList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& first, SLockFreeSingleLinkedListEntry& last, u32 count)
{
	(void)count; //unused

	u32 curSetting[2];
	u32 newSetting[2];
	u32 newPointer = (u32) & first;
	do
	{
		curSetting[0] = (u32)list.pNext;
		curSetting[1] = list.salt;
		last.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
		newSetting[0] = newPointer;        // new pointer
		newSetting[1] = curSetting[1] + 1; // new salt
	}
	while (false == __sync_bool_compare_and_swap(( uint64*)&list.pNext, *(uint64*)&curSetting[0], *(uint64*)&newSetting[0]));
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxInterlockedPopEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	u32 curSetting[2];
	u32 newSetting[2];
	do
	{
		curSetting[1] = list.salt;
		curSetting[0] = (u32)list.pNext;
		if (curSetting[0] == 0)
			return NULL;
		newSetting[0] = *(u32*)curSetting[0]; // new pointer
		newSetting[1] = curSetting[1] + 1;       // new salt
	}
	while (false == __sync_bool_compare_and_swap(( uint64*)&list.pNext, *(uint64*)&curSetting[0], *(uint64*)&newSetting[0]));
	return (uk )curSetting[0];
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxRtlFirstEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	return list.pNext;
}

//////////////////////////////////////////////////////////////////////////
ILINE void DrxInitializeSListHead(SLockFreeSingleLinkedListHeader& list)
{
	list.salt = 0;
	list.pNext = NULL;
}

//////////////////////////////////////////////////////////////////////////
ILINE uk DrxInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list)
{
	u32 curSetting[2];
	u32 newSetting[2];
	u32 newSalt;
	u32 newPointer;
	do
	{
		curSetting[1] = list.salt;
		curSetting[0] = (u32)list.pNext;
		if (curSetting[0] == 0)
			return NULL;
		newSetting[0] = 0;
		newSetting[1] = curSetting[1] + 1;
	}
	while (false == __sync_bool_compare_and_swap(( uint64*)&list.pNext, *(uint64*)&curSetting[0], *(uint64*)&newSetting[0]));
	return (uk )curSetting[0];
}
#endif

//////////////////////////////////////////////////////////////////////////
// Helper
//////////////////////////////////////////////////////////////////////////

class CSimpleThreadBackOff
{
public:
	static u32k kSoftYieldInterval = 0x3FF;
	static u32k kHardYieldInterval = 0x1FFF;

public:
	CSimpleThreadBackOff() : m_counter(0) {}

	void reset() { m_counter = 0; }

	void backoff()
	{
#if !DRX_PLATFORM_ANDROID
		_mm_pause();
#endif

		if (!(++m_counter & kHardYieldInterval))
		{
			// give other threads with other prio right to run
			usleep(1);
		}
		else if (!(m_counter & kSoftYieldInterval))
		{
			// give threads with same prio chance to run
			sched_yield();
		}
	}

private:
	i32 m_counter;
};
