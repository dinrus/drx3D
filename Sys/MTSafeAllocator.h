// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _DRX_SYSTEM_MT_SAFE_ALLOCATOR_HDR_
#define _DRX_SYSTEM_MT_SAFE_ALLOCATOR_HDR_

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <drx3D/CoreX/Platform/Linux_Win32Wrapper.h>
#endif
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

////////////////////////////////////////////////////////////////////////////////
// Durango temporary pool configuration
#if DRX_PLATFORM_DURANGO
	#define MTSAFE_TEMPORARY_POOL_SIZE          (0U)
	#define MTSAFE_TEMPORARY_POOL_MINALLOC      (512U)
	#define MTSAFE_TEMPORARY_POOL_MAXALLOC      (14U << 20)
	#define MTSAFE_TEMPORARY_POOL_CONFIGURATION 1
	#define MTSAFE_USE_BIGPOOL                  0
	#define MTSAFE_USE_GENERAL_HEAP             1
	#define MTSAFE_GENERAL_HEAP_SIZE            (12U << 20)
	#define MTSAFE_DEFAULT_ALIGNMENT            16
#elif DRX_PLATFORM_MOBILE  // iOS/Android
	#define MTSAFE_TEMPORARY_POOL_SIZE          (0)
	#define MTSAFE_TEMPORARY_POOL_MINALLOC      (0)
	#define MTSAFE_TEMPORARY_POOL_MAXALLOC      ((128U << 10) - 1)
	#define MTSAFE_TEMPORARY_POOL_CONFIGURATION 1
	#define MTSAFE_USE_INPLACE_POOL             0
	#define MTSAFE_USE_BIGPOOL                  0
	#define MTSAFE_DEFAULT_ALIGNMENT            8
	#define MTSAFE_USE_GENERAL_HEAP             1
	#define MTSAFE_GENERAL_HEAP_SIZE            ((1U << 20) + (1U << 19))
#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_MAC || DRX_PLATFORM_ORBIS  // PC
	#define MTSAFE_TEMPORARY_POOL_SIZE          (0U)
	#define MTSAFE_TEMPORARY_POOL_MINALLOC      (512U)
	#define MTSAFE_TEMPORARY_POOL_MAXALLOC      (14U << 20)
	#define MTSAFE_TEMPORARY_POOL_CONFIGURATION 1
	#define MTSAFE_USE_BIGPOOL                  0
	#define MTSAFE_USE_GENERAL_HEAP             1
	#define MTSAFE_GENERAL_HEAP_SIZE            (12U << 20)
	#define MTSAFE_DEFAULT_ALIGNMENT            8
#else
	#error Unknown target platform
#endif

////////////////////////////////////////////////////////////////////////////////
// default malloc fallback if no configuration present
#if !MTSAFE_TEMPORARY_POOL_CONFIGURATION
	#if defined(_MSC_VER)
		#pragma message("no temporary pool configuration for current target platform, using malloc/free")
	#else
		#warning "no temporary pool configuration for current target platform, using malloc/free"
	#endif
	#define MTSAFE_TEMPORARY_POOL_SIZE          (0U)
	#define MTSAFE_TEMPORARY_POOL_MINALLOC      (0U)
	#define MTSAFE_TEMPORARY_POOL_MAXALLOC      (0U)
	#define MTSAFE_TEMPORARY_POOL_CONFIGURATION 1
#endif

////////////////////////////////////////////////////////////////////////////////
// Inplace pool configuration
#if MTSAFE_USE_INPLACE_POOL
	#if MTSAFE_TEMPORARY_POOL_SIZE <= 0U
		#error "MTSAFE_TEMPORARY_POOL_SIZE temporary pool size is 0!"
	#endif
	#if MTSAFE_TEMPORARY_POOL_MAXALLOC > MTSAFE_TEMPORARY_POOL_SIZE
		#error "MTSAFE_TEMPORARY_POOL_MAXALLOC larger than MTSAFE_TEMPORARY_POOL_SIZE!"
	#endif

	#include <drx3D/CoreX/Memory/Pool/PoolAlloc.h>
using NDrxPoolAlloc::CFirstFit;         // speed of allocations are crucial, so simply use the first fitting free allocation
using NDrxPoolAlloc::CInPlace;          // Inplace - accessible by cpu
using NDrxPoolAlloc::CMemoryDynamic;    // the pool itself will be dynamically allocated
using NDrxPoolAlloc::CListItemInPlace;  // store the storage
typedef CFirstFit<CInPlace<CMemoryDynamic>, CListItemInPlace> TMTSafePool;
#endif

#ifndef MTSAFE_USE_VIRTUALALLOC
	#define MTSAFE_USE_VIRTUALALLOC 0
#endif

#ifndef MTSAFE_USE_GENERAL_HEAP
	#define MTSAFE_USE_GENERAL_HEAP 0
#endif

////////////////////////////////////////////////////////////////////////////////
// BigPool configuration
#if MTSAFE_USE_BIGPOOL
	#define NUM_POOLS 16
#endif

class CMTSafeHeap
{
public:
	// Constructor
	CMTSafeHeap();

	// Destructor
	~CMTSafeHeap();

	// Performs a persisistent (in other words, non-temporary) allocation.
	uk PersistentAlloc(size_t nSize);

	// Retrieves system memory allocation size for any call to PersistentAlloc.
	// Required to not count virtual memory usage inside DrxSizer
	size_t PersistentAllocSize(size_t nSize);

	// Frees memory allocation
	void FreePersistent(uk p);

	// Perform a allocation that is considered temporary and will be handled by
	// the pool itself.
	// Note: It is important that these temporary allocations are actually
	// temporary and do not persist for a long persiod of time.
	uk TempAlloc(size_t nSize, tukk szDbgSource, u32 align = 0)
	{
		bool bFallbackToMalloc = true;
		return TempAlloc(nSize, szDbgSource, bFallbackToMalloc, align);
	}

	uk TempAlloc(size_t nSize, tukk szDbgSource, bool& bFallBackToMalloc, u32 align = 0);

#if MTSAFE_USE_GENERAL_HEAP
	bool IsInGeneralHeap(ukk p)
	{
		return m_pGeneralHeapStorage <= p && p < m_pGeneralHeapStorageEnd;
	}
#endif

	// Free a temporary allocaton.
	void FreeTemporary(uk p);

	// The number of live allocations allocation within the temporary pool
	size_t NumAllocations() const { return m_LiveTempAllocations; }

	// The memory usage of the mtsafe allocator
	void GetMemoryUsage(IDrxSizer* pSizer);

	// zlib-compatible stubs
	static uk StaticAlloc(uk pOpaque, unsigned nItems, unsigned nSize);
	static void  StaticFree(uk pOpaque, uk pAddress);

	// Dump some statistics to the drx log
	void PrintStats();

#if MTSAFE_USE_VIRTUALALLOC
private:
	static const bool IsVirtualAlloced = true;

private:
	uk TempVirtualAlloc(size_t nSize);
	void  TempVirtualFree(uk ptr);
#endif

private:
	friend class CSystem;

	// Inplace pool member variables
#if MTSAFE_USE_INPLACE_POOL
	// Prevents concurrent access to the temporary pool
	DrxCriticalSectionNonRecursive m_LockTemporary;

	// The actual temporary pool
	TMTSafePool m_TemporaryPool;
#endif
	// Big pool member variables
#if MTSAFE_USE_BIGPOOL
	LONG        m_iBigPoolUsed[NUM_POOLS];
	LONG        m_iBigPoolSize[NUM_POOLS];
	tukk m_sBigPoolDescription[NUM_POOLS];
	uk       m_pBigPool[NUM_POOLS];
#endif

#if MTSAFE_USE_GENERAL_HEAP
	IGeneralMemoryHeap* m_pGeneralHeap;
	tuk               m_pGeneralHeapStorage;
	tuk               m_pGeneralHeapStorageEnd;
#endif

#if MTSAFE_USE_VIRTUALALLOC
	HANDLE m_smallHeap;
#endif

	// The number of temporary allocations currently active within the pool
	size_t m_LiveTempAllocations;

	// The total number of allocations performed in the pool
	size_t m_TotalAllocations;

	// The total bytes that weren't temporarily allocated
	size_t        m_TempAllocationsFailed;
	// The total number of temporary allocations that fell back to global system memory
	LARGE_INTEGER m_TempAllocationsTime;
};

#endif
