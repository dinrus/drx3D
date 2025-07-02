// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/BitFiddling.h>

#include <stdio.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Platform/platform.h>

#include <drx3D/CoreX/Memory/DrxMemoryAllocator.h>
#if DRX_PLATFORM_WINDOWS
	#include <drx3D/Sys/DebugCallStack.h>
#endif

#include <drx3D/Sys/MemReplay.h>
#include <drx3D/Sys/MemoryUpr.h>

 bool g_replayCleanedUp = false;

#ifdef DANGLING_POINTER_DETECTOR
	#if DRX_PLATFORM_ORBIS
		#include "DrxMemoryUpr_sce.h"
	#else
uk DanglingPointerDetectorTransformAlloc(uk ptr, size_t size) { return ptr; }
uk DanglingPointerDetectorTransformFree(uk ptr)               { return ptr; }
uk DanglingPointerDetectorTransformNull(uk ptr)               { return ptr; }
	#endif
#endif

const bool bProfileMemUpr = 0;

#ifdef DRXMM_SUPPORT_DEADLIST
namespace
{
struct DeadHeadSize
{
	DeadHeadSize* pPrev;
	UINT_PTR      nSize;
};
}

static DrxCriticalSectionNonRecursive s_deadListLock;
static DeadHeadSize* s_pDeadListFirst = NULL;
static DeadHeadSize* s_pDeadListLast = NULL;
static size_t s_nDeadListSize;

static void DrxFreeReal(uk p);

static void DeadListValidate(u8k* p, size_t sz, u8 val)
{
	for (u8k* pe = p + sz; p != pe; ++p)
	{
		if (*p != val)
		{
			DrxGetIMemReplay()->Stop();
			__debugbreak();
		}
	}
}

static void DeadListFlush_Locked()
{
	while ((s_nDeadListSize > (size_t)CDrxMemoryUpr::s_sys_MemoryDeadListSize) && s_pDeadListLast)
	{
		DeadHeadSize* pPrev = s_pDeadListLast->pPrev;

		size_t sz = s_pDeadListLast->nSize;
		uk pVal = s_pDeadListLast + 1;

		DeadListValidate((u8k*)pVal, sz - sizeof(DeadHeadSize), 0xfe);

		DrxFreeReal(s_pDeadListLast);

		s_pDeadListLast = pPrev;
		s_nDeadListSize -= sz;
	}

	if (!s_pDeadListLast)
		s_pDeadListFirst = NULL;
}

static void DeadListPush(uk p, size_t sz)
{
	if (sz >= sizeof(DeadHeadSize))
	{
		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(s_deadListLock);
		DeadHeadSize* pHead = (DeadHeadSize*)p;

		memset(pHead + 1, 0xfe, sz - sizeof(DeadHeadSize));
		pHead->nSize = sz;

		if (s_pDeadListFirst)
			s_pDeadListFirst->pPrev = pHead;

		pHead->pPrev = 0;
		s_pDeadListFirst = pHead;
		if (!s_pDeadListLast)
			s_pDeadListLast = pHead;

		s_nDeadListSize += sz;
		if (s_nDeadListSize > (size_t)CDrxMemoryUpr::s_sys_MemoryDeadListSize)
			DeadListFlush_Locked();
	}
	else
	{
		DrxFreeReal(p);
	}
}

#endif

//////////////////////////////////////////////////////////////////////////
// Some globals for fast profiling.
//////////////////////////////////////////////////////////////////////////
LONG g_TotalAllocatedMemory = 0;

#ifndef DRXMEMORYMANAGER_API
	#define DRXMEMORYMANAGER_API
#endif // DRXMEMORYMANAGER_API

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API size_t DrxFree(uk p, size_t alignment);
DRXMEMORYMANAGER_API void   DrxFreeSize(uk p, size_t size);

DRXMEMORYMANAGER_API void   DrxCleanup();

// Undefine malloc for memory manager itself..
#undef malloc
#undef realloc
#undef free

#define VIRTUAL_ALLOC_SIZE 524288

#include <drx3D/CoreX/Memory/DrxMemoryAllocator.h>

#if defined(USE_GLOBAL_BUCKET_ALLOCATOR)
	#include <drx3D/CoreX/Memory/BucketAllocatorImpl.h>
BucketAllocator<BucketAllocatorDetail::DefaultTraits<BUCKET_ALLOCATOR_DEFAULT_SIZE, BucketAllocatorDetail::SyncPolicyLocked, true, 8>> g_GlobPageBucketAllocator;
#else
node_alloc<eDrxMallocDrxFreeCRTCleanup, true, VIRTUAL_ALLOC_SIZE> g_GlobPageBucketAllocator;
#endif // defined(USE_GLOBAL_BUCKET_ALLOCATOR)

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API uk DrxMalloc(size_t size, size_t& allocated, size_t alignment)
{
	if (!size)
	{
		allocated = 0;
		return 0;
	}

	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

	u8* p;
	size_t sizePlus = size;

	if (!alignment || g_GlobPageBucketAllocator.CanGuaranteeAlignment(sizePlus, alignment))
	{
		if (alignment)
		{
			p = (u8*)g_GlobPageBucketAllocator.allocate(sizePlus, alignment);
		}
		else
		{
			p = (u8*)g_GlobPageBucketAllocator.alloc(sizePlus);
		}

		// The actual allocated memory will be the size of a whole bucket, which might be bigger than the requested size.
		// e.g. if we request 56bytes and the next fitting bucket is 64bytes, we will be allocating 64bytes.
		//
		// Having the correct value is important because the whole bucket size is later used by DrxFree when
		// deallocating and both values should match for stats tracking to be accurate.
		//
		// We use getSize and not getSizeEx because in the case of an allocation with "alignment" == 0 and "size" bigger than the bucket size
		// the bucket allocator will end up allocating memory using DrxCrtMalloc which falls outside the address range of the bucket allocator
		sizePlus = g_GlobPageBucketAllocator.getSize(p);
	}
	else
	{
		alignment = max(alignment, (size_t)16);

		// emulate alignment
		sizePlus += alignment;
		p = (u8*) DrxSystemCrtMalloc(sizePlus);

		if (alignment && p)
		{
			u32 offset = (u32)(alignment - ((UINT_PTR)p & (alignment - 1)));
			p += offset;
			reinterpret_cast<u32*>(p)[-1] = offset;
		}
	}

	//////////////////////////////////////////////////////////////////////////
#if !defined(USE_GLOBAL_BUCKET_ALLOCATOR)
	if (sizePlus < __MAX_BYTES + 1)
	{
		sizePlus = ((sizePlus - size_t(1)) >> (i32)_ALIGN_SHIFT);
		sizePlus = (sizePlus + 1) << _ALIGN_SHIFT;
	}
#endif //!defined(USE_GLOBAL_BUCKET_ALLOCATOR)

	if (!p)
	{
		allocated = 0;
		gEnv->bIsOutOfMemory = true;
		DrxFatalError("**** Memory allocation for %" PRISIZE_T " bytes failed ****", sizePlus);
		return 0;   // don't crash - allow caller to react
	}

	DrxInterlockedExchangeAdd(&g_TotalAllocatedMemory, sizePlus);
	allocated = sizePlus;

	MEMREPLAY_SCOPE_ALLOC(p, sizePlus, 0);

	assert(alignment == 0 || (reinterpret_cast<UINT_PTR>(p) & (alignment - 1)) == 0);

#ifdef DANGLING_POINTER_DETECTOR
	return DanglingPointerDetectorTransformAlloc(p, size);
#endif

	return p;
}

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API size_t DrxGetMemSize(uk memblock, size_t alignment)
{
	//	ReadLock lock(g_lockMemMan);
#ifdef DANGLING_POINTER_DETECTOR
	memblock = DanglingPointerDetectorTransformNull(memblock);
#endif
	
	if (!alignment || g_GlobPageBucketAllocator.IsInAddressRange(memblock))
	{
		return g_GlobPageBucketAllocator.getSize(memblock);
	}
	else
	{
		u8* pb = static_cast<u8*>(memblock);
		u32 adj = reinterpret_cast<u32*>(pb)[-1];
		return DrxSystemCrtSize(pb - adj) - adj;
	}
}

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API uk DrxRealloc(uk memblock, size_t size, size_t& allocated, size_t& oldsize, size_t alignment)
{
	if (memblock == NULL)
	{
		oldsize = 0;
		return DrxMalloc(size, allocated, alignment);
	}
	else
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtMalloc);

#ifdef DANGLING_POINTER_DETECTOR
		memblock = DanglingPointerDetectorTransformFree(memblock);
#endif

		uk np;
		np = DrxMalloc(size, allocated, alignment);

		// get old size
		if (g_GlobPageBucketAllocator.IsInAddressRange(memblock))
		{
			oldsize = g_GlobPageBucketAllocator.getSize(memblock);
		}
		else
		{
			u8* pb = static_cast<u8*>(memblock);
			i32 adj = 0;
			if (alignment)
				adj = reinterpret_cast<u32*>(pb)[-1];
			oldsize = DrxSystemCrtSize(pb - adj) - adj;
		}

		if (!np && size)
		{
			gEnv->bIsOutOfMemory = true;
			DrxFatalError("**** Memory allocation for %" PRISIZE_T " bytes failed ****", size);
			return 0;   // don't crash - allow caller to react
		}

		// copy data over
		memcpy(np, memblock, size > oldsize ? oldsize : size);
		DrxFree(memblock, alignment);

		MEMREPLAY_SCOPE_REALLOC(memblock, np, size, alignment);

		assert(alignment == 0 || (reinterpret_cast<UINT_PTR>(np) & (alignment - 1)) == 0);
		return np;
	}
}

#ifdef DRXMM_SUPPORT_DEADLIST
static void DrxFreeReal(uk p)
{
	UINT_PTR pid = (UINT_PTR)p;

	if (p != NULL)
	{
		if (g_GlobPageBucketAllocator.IsInAddressRange(p))
			g_GlobPageBucketAllocator.deallocate(p);
		else
			free(p);
	}
}
#endif

//////////////////////////////////////////////////////////////////////////
size_t DrxFree(uk p, size_t alignment)
{
#ifdef DANGLING_POINTER_DETECTOR
	p = DanglingPointerDetectorTransformFree(p);
#endif

#ifdef DRXMM_SUPPORT_DEADLIST

	if (CDrxMemoryUpr::s_sys_MemoryDeadListSize > 0)
	{
		size_t size = 0;

		UINT_PTR pid = (UINT_PTR)p;

		if (p != NULL)
		{
			MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

			if (g_GlobPageBucketAllocator.IsInAddressRange(p))
			{
				size = g_GlobPageBucketAllocator.getSizeEx(p);
				DeadListPush(p, size);
			}
			else
			{
				if (alignment)
				{
					u8* pb = static_cast<u8*>(p);
					pb -= reinterpret_cast<u32*>(pb)[-1];
					size = DrxSystemCrtSize(pb);
					DeadListPush(pb, size);
				}
				else
				{
					size = DrxSystemCrtSize(p);
					DeadListPush(p, size);
				}
			}

			LONG lsize = size;
			DrxInterlockedExchangeAdd(&g_TotalAllocatedMemory, -lsize);

			MEMREPLAY_SCOPE_FREE(pid);
		}

		return size;
	}
#endif

	size_t size = 0;

	UINT_PTR pid = (UINT_PTR)p;

	if (p != NULL)
	{
		MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);

		if (g_GlobPageBucketAllocator.IsInAddressRange(p))
		{
			size = g_GlobPageBucketAllocator.deallocate(p);
		}
		else
		{
			if (alignment)
			{
				u8* pb = static_cast<u8*>(p);
				pb -= reinterpret_cast<u32*>(pb)[-1];
				size = DrxSystemCrtFree(pb);
			}
			else
			{
				size = DrxSystemCrtFree(p);
			}
		}

		LONG lsize = size;
		DrxInterlockedExchangeAdd(&g_TotalAllocatedMemory, -lsize);

		MEMREPLAY_SCOPE_FREE(pid);
	}

	return size;
}

DRXMEMORYMANAGER_API void DrxFlushAll()  // releases/resets ALL memory... this is useful for restarting the game
{
	g_TotalAllocatedMemory = 0;
};

//////////////////////////////////////////////////////////////////////////
// Returns amount of memory allocated with DrxMalloc/DrxFree functions.
//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API i32 DrxMemoryGetAllocatedSize()
{
	return g_TotalAllocatedMemory;
}

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API i32 DrxMemoryGetPoolSize()
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
DRXMEMORYMANAGER_API i32 DrxStats(tuk buf)
{
	return 0;
}

DRXMEMORYMANAGER_API i32 DrxGetUsedHeapSize()
{
	return g_GlobPageBucketAllocator.get_heap_size();
}

DRXMEMORYMANAGER_API i32 DrxGetWastedHeapSize()
{
	return g_GlobPageBucketAllocator.get_wasted_in_allocation();
}

DRXMEMORYMANAGER_API void DrxCleanup()
{
	g_GlobPageBucketAllocator.cleanup();
}

#if defined(USE_GLOBAL_BUCKET_ALLOCATOR)
void EnableDynamicBucketCleanups(bool enable)
{
	g_GlobPageBucketAllocator.EnableExpandCleanups(enable);
}
void BucketAllocatorReplayRegisterAddressRange(tukk name)
{
	#if CAPTURE_REPLAY_LOG
	g_GlobPageBucketAllocator.ReplayRegisterAddressRange(name);
	#endif //CAPTURE_REPLAY_LOG
}
#endif //defined(USE_GLOBAL_BUCKET_ALLOCATOR)

#if DRX_PLATFORM_ORBIS
	#define CRT_IS_DLMALLOC
#endif

#ifdef CRT_IS_DLMALLOC
	#include <drx3D/CoreX/Platform/DrxDLMalloc.h>

static dlmspace s_dlHeap = 0;
static DrxCriticalSection s_dlMallocLock;

static uk crt_dlmmap_handler(uk user, size_t sz)
{
	uk base = VirtualAllocator::AllocateVirtualAddressSpace(sz);
	if (!base)
		return dlmmap_error;

	if (!VirtualAllocator::MapPageBlock(base, sz, PAGE_SIZE))
		return dlmmap_error;

	return base;
}

static i32 crt_dlmunmap_handler(uk user, uk mem, size_t sz)
{
	VirtualAllocator::FreeVirtualAddressSpace(mem, sz);
	return 0;
}

static bool DrxSystemCrtInitialiseHeap(void)
{
	size_t heapSize = 256 * 1024 * 1024; // 256MB
	s_dlHeap = dlcreate_mspace(heapSize, 0, NULL, crt_dlmmap_handler, crt_dlmunmap_handler);

	if (s_dlHeap)
	{
		return true;
	}

	__debugbreak();
	return false;
}
#endif

DRXMEMORYMANAGER_API uk DrxSystemCrtMalloc(size_t size)
{
	uk ret = NULL;
#ifdef CRT_IS_DLMALLOC
	AUTO_LOCK(s_dlMallocLock);
	if (!s_dlHeap)
		DrxSystemCrtInitialiseHeap();
	return dlmspace_malloc(s_dlHeap, size);
#elif DRX_PLATFORM_ORBIS || DRX_PLATFORM_ANDROID || (defined(NOT_USE_DRX_MEMORY_MANAGER) && (DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX))
	size_t* allocSize = (size_t*)malloc(size + 2 * sizeof(size_t));
	*allocSize = size;
	ret = (uk )(allocSize + 2);
#else
	ret = malloc(size);
#endif
	return ret;
}

DRXMEMORYMANAGER_API uk DrxSystemCrtRealloc(uk p, size_t size)
{
	uk ret;
#ifdef CRT_IS_DLMALLOC
	AUTO_LOCK(s_dlMallocLock);
	if (!s_dlHeap)
		DrxSystemCrtInitialiseHeap();
	#ifndef _RELEASE
	if (p && !VirtualAllocator::InAllocatedSpace(p))
		__debugbreak();
	#endif
	return dlmspace_realloc(s_dlHeap, p, size);
#elif DRX_PLATFORM_ORBIS || DRX_PLATFORM_ANDROID || (defined(NOT_USE_DRX_MEMORY_MANAGER) && (DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX))
	size_t* origPtr = (size_t*)p;
	size_t* newPtr = (size_t*)realloc(origPtr - 2, size + 2 * sizeof(size_t));
	*newPtr = size;
	ret = (uk )(newPtr + 2);
#else
	ret = realloc(p, size);
#endif
	return ret;
}

DRXMEMORYMANAGER_API size_t DrxSystemCrtFree(uk p)
{
	size_t n = 0;
#ifdef CRT_IS_DLMALLOC
	AUTO_LOCK(s_dlMallocLock);
	if (p)
	{
	#ifndef _RELEASE
		if (!VirtualAllocator::InAllocatedSpace(p))
			__debugbreak();
	#endif
		const size_t size = dlmspace_usable_size(p);
		dlmspace_free(s_dlHeap, p);
		return size;
	}
	return 0;
#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
	n = _msize(p);
#elif DRX_PLATFORM_ORBIS || DRX_PLATFORM_ANDROID || (defined(NOT_USE_DRX_MEMORY_MANAGER) && (DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX))
	size_t* ptr = (size_t*)p;
	ptr -= 2;
	n = *ptr;
	p = (uk )ptr;
#elif DRX_PLATFORM_APPLE
	n = malloc_size(p);
#else
	n = malloc_usable_size(p);
#endif
	free(p);
	return n;
}

DRXMEMORYMANAGER_API size_t DrxSystemCrtSize(uk p)
{
#ifdef CRT_IS_DLMALLOC
	AUTO_LOCK(s_dlMallocLock);
	return dlmspace_usable_size(p);
#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
	return _msize(p);
#elif DRX_PLATFORM_ORBIS || DRX_PLATFORM_ANDROID || (defined(NOT_USE_DRX_MEMORY_MANAGER) && (DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX))
	size_t* ptr = (size_t*)p;
	return *(ptr - 2);
#elif DRX_PLATFORM_APPLE
	return malloc_size(p);
#else
	return malloc_usable_size(p);
#endif
}

size_t DrxSystemCrtGetUsedSpace()
{
	size_t used = 0;
#if defined(USE_GLOBAL_BUCKET_ALLOCATOR)
	used += g_GlobPageBucketAllocator.GetBucketConsumedSize();
#endif
#ifdef CRT_IS_DLMALLOC
	{
		AUTO_LOCK(s_dlMallocLock);
		if (s_dlHeap)
		{
			used += dlmspace_get_used_space(s_dlHeap);
		}
	}
#endif
	return used;
}

DRXMEMORYMANAGER_API void DrxResetStats(void)
{
}

i32 GetPageBucketAlloc_wasted_in_allocation()
{
	return g_GlobPageBucketAllocator.get_wasted_in_allocation();
}

i32 GetPageBucketAlloc_get_free()
{
	return g_GlobPageBucketAllocator._S_get_free();
}
