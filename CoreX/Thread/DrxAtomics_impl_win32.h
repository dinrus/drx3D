// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <drx3D/CoreX/Assert/DrxAssert.h>

#if !(defined(NTDDI_VERSION) && defined(NTDDI_WIN8) && (NTDDI_VERSION >= NTDDI_WIN8))
// This declaration is missing from older Windows SDKs
// However, it's documented on MSDN as available starting with WinVista/Server2008
extern "C" WINBASEAPI PSLIST_ENTRY __fastcall InterlockedPushListSList(_Inout_ PSLIST_HEADER ListHead, _Inout_ PSLIST_ENTRY List, _Inout_ PSLIST_ENTRY ListEnd, _In_ ULONG Count);
#endif

//////////////////////////////////////////////////////////////////////////
void DrxInterlockedPushEntrySList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& element)
{
	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");
	static_assert(sizeof(SLockFreeSingleLinkedListEntry) >= sizeof(SLIST_ENTRY), "DRX_INTERLOCKED_SLIST_ENTRY_HAS_WRONG_SIZE");

	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");
	DRX_ASSERT_MESSAGE(IsAligned(&element, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Entry has wrong Alignment");
	InterlockedPushEntrySList(alias_cast<PSLIST_HEADER>(&list), alias_cast<PSLIST_ENTRY>(&element));
}

//////////////////////////////////////////////////////////////////////////
void DrxInterlockedPushListSList(SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry& first, SLockFreeSingleLinkedListEntry& last, u32 count)
{
	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");
	static_assert(sizeof(SLockFreeSingleLinkedListEntry) >= sizeof(SLIST_ENTRY), "DRX_INTERLOCKED_SLIST_ENTRY_HAS_WRONG_SIZE");

	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");
	DRX_ASSERT_MESSAGE(IsAligned(&first, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Entry has wrong Alignment");
	DRX_ASSERT_MESSAGE(IsAligned(&last, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Entry has wrong Alignment");
	InterlockedPushListSList(alias_cast<PSLIST_HEADER>(&list), alias_cast<PSLIST_ENTRY>(&first), alias_cast<PSLIST_ENTRY>(&last), (ULONG)count);
}

//////////////////////////////////////////////////////////////////////////
uk DrxInterlockedPopEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");

	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");
	return reinterpret_cast<uk>(InterlockedPopEntrySList(alias_cast<PSLIST_HEADER>(&list)));
}

//////////////////////////////////////////////////////////////////////////
uk DrxRtlFirstEntrySList(SLockFreeSingleLinkedListHeader& list)
{
	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");
	static_assert(sizeof(SLockFreeSingleLinkedListEntry) >= sizeof(SLIST_ENTRY), "DRX_INTERLOCKED_SLIST_ENTRY_HAS_WRONG_SIZE");

	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");
#if DRX_PLATFORM_DURANGO
	// This is normally implemented in NTDLL, but that can't be linked on Durango
	// However, we know that the X64 version of the header is used, so just access it directly
	return (uk )(((PSLIST_HEADER)&list)->HeaderX64.NextEntry << 4);
#else
	return reinterpret_cast<uk>(RtlFirstEntrySList(alias_cast<PSLIST_HEADER>(&list)));
#endif
}

//////////////////////////////////////////////////////////////////////////
void DrxInitializeSListHead(SLockFreeSingleLinkedListHeader& list)
{
	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");

	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");
	InitializeSListHead(alias_cast<PSLIST_HEADER>(&list));
}

//////////////////////////////////////////////////////////////////////////
uk DrxInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list)
{
	DRX_ASSERT_MESSAGE(IsAligned(&list, MEMORY_ALLOCATION_ALIGNMENT), "LockFree SingleLink List Header has wrong Alignment");

	static_assert(sizeof(SLockFreeSingleLinkedListHeader) == sizeof(SLIST_HEADER), "DRX_INTERLOCKED_SLIST_HEADER_HAS_WRONG_SIZE");
	return InterlockedFlushSList(alias_cast<PSLIST_HEADER>(&list));
}
