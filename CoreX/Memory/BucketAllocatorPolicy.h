// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef BUCKETALLOCATORPOLICY_H
#define BUCKETALLOCATORPOLICY_H

#define BUCKET_ALLOCATOR_DEFAULT_MAX_SEGMENTS 8

#include <drx3D/CoreX/Platform/DrxWindows.h>
#ifndef MEMORY_ALLOCATION_ALIGNMENT
	#error MEMORY ALLOCATION_ALIGNMENT is not defined
#endif

namespace BucketAllocatorDetail
{

struct AllocHeader
{
	AllocHeader*  next;

#ifdef BUCKET_ALLOCATOR_TRAP_DOUBLE_DELETES
	UINT_PTR magic;
#endif
};

struct SyncPolicyLocked
{
	typedef SLockFreeSingleLinkedListHeader FreeListHeader;

	typedef DrxCriticalSectionNonRecursive  Lock;

	Lock& GetRefillLock()
	{
		static Lock m_refillLock;
		return m_refillLock;
	}

	struct RefillLock
	{
		RefillLock(SyncPolicyLocked& policy) : m_policy(policy) { policy.GetRefillLock().Lock(); }
		~RefillLock() { m_policy.GetRefillLock().Unlock(); }

	private:
		SyncPolicyLocked& m_policy;
	};

	static void PushOnto(FreeListHeader& list, AllocHeader* ptr)
	{
		DrxInterlockedPushEntrySList(list, *reinterpret_cast<SLockFreeSingleLinkedListEntry*>(ptr));
	}

	static void PushListOnto(FreeListHeader& list, AllocHeader* head, AllocHeader* tail, size_t count)
	{
		DrxInterlockedPushListSList(list, *reinterpret_cast<SLockFreeSingleLinkedListEntry*>(head), *reinterpret_cast<SLockFreeSingleLinkedListEntry*>(tail), (u32)count);
	}

	static AllocHeader* PopOff(FreeListHeader& list)
	{
		return reinterpret_cast<AllocHeader*>(DrxInterlockedPopEntrySList(list));
	}

	static AllocHeader* PopListOff(FreeListHeader& list)
	{
		return reinterpret_cast<AllocHeader*>(DrxInterlockedFlushSList(list));
	}

	ILINE static bool IsFreeListEmpty(FreeListHeader& list)
	{
		return DrxRtlFirstEntrySList(list) == 0;
	}
};

struct SyncPolicyUnlocked
{
	typedef AllocHeader* FreeListHeader;

	struct RefillLock
	{
		RefillLock(SyncPolicyUnlocked&) {}
	};

	ILINE static void PushOnto(FreeListHeader& list, AllocHeader* ptr)
	{
		ptr->next = list;
		list = ptr;
	}

	ILINE static void PushListOnto(FreeListHeader& list, AllocHeader* head, AllocHeader* tail, size_t count)
	{
		tail->next = list;
		list = head;
	}

	ILINE static AllocHeader* PopOff(FreeListHeader& list)
	{
		AllocHeader* top = list;
		if (top)
			list = *(AllocHeader**)(&top->next);   // cast away the 
		return top;
	}

	ILINE static AllocHeader* PopListOff(FreeListHeader& list)
	{
		AllocHeader* pRet = list;
		list = NULL;
		return pRet;
	}

	ILINE static bool IsFreeListEmpty(FreeListHeader& list)
	{
		return list == NULL;
	}
};

template<size_t Size, typename SyncingPolicy, bool FallbackOnCRT = true, size_t MaxSegments = BUCKET_ALLOCATOR_DEFAULT_MAX_SEGMENTS>
struct DefaultTraits
{
	enum
	{
		MaxSize = 512,

#ifdef BUCKET_ALLOCATOR_PACK_SMALL_SIZES
		NumBuckets = 32 / 4 + (512 - 32) / 8,
#else
		NumBuckets = 512 / MEMORY_ALLOCATION_ALIGNMENT,
#endif

		PageLength           = 64 * 1024,
		SmallBlockLength     = 1024,
		SmallBlocksPerPage   = 64,

		NumGenerations       = 4,
		MaxNumSegments       = MaxSegments,

		NumPages             = Size / PageLength,

		FallbackOnCRTAllowed = FallbackOnCRT,
	};

	typedef SyncingPolicy SyncPolicy;

	static u8 GetBucketForSize(size_t sz)
	{
#ifdef BUCKET_ALLOCATOR_PACK_SMALL_SIZES
		if (sz <= 32)
		{
			i32k alignment = 4;
			size_t alignedSize = (sz + (alignment - 1)) & ~(alignment - 1);
			return alignedSize / alignment - 1;
		}
		else
		{
			i32k alignment = 8;
			size_t alignedSize = (sz + (alignment - 1)) & ~(alignment - 1);
			alignedSize -= 32;
			return alignedSize / alignment + 7;
		}
#else
		i32k alignment = MEMORY_ALLOCATION_ALIGNMENT;
		size_t alignedSize = (sz + (alignment - 1)) & ~(alignment - 1);
		return static_cast<u8>(alignedSize / alignment - 1);
#endif
	}

	static size_t GetSizeForBucket(u8 bucket)
	{
		size_t sz;

#ifdef BUCKET_ALLOCATOR_PACK_SMALL_SIZES
		if (bucket <= 7)
			sz = (bucket + 1) * 4;
		else
			sz = (bucket - 7) * 8 + 32;
#else
		sz = (bucket + 1) * MEMORY_ALLOCATION_ALIGNMENT;
#endif

#ifdef BUCKET_ALLOCATOR_TRAP_DOUBLE_DELETES
		return sz < sizeof(AllocHeader) ? sizeof(AllocHeader) : sz;
#else
		return sz;
#endif
	}

	static size_t GetGenerationForStability(u8 stability)
	{
		return 3 - stability / 64;
	}
};
}

#endif
