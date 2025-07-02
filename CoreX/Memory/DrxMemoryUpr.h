// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxMemoryUpr.h
//  Version:     v1.00
//  Created:     27/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   drux (DinrusPro's Universal Compiler)
//  Описание: Defines functions for DinrusX custom memory manager.
//               See also DrxMemoryUpr_impl.h, it must be included only once per module.
//               DrxMemoryUpr_impl.h is included by platform_impl.inl
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DrxMemoryUpr_h__
#define __DrxMemoryUpr_h__
#pragma once
#include <algorithm>
#include <cstddef>
#include <drx3D/CoreX/Assert/DrxAssert.h>

#if !defined(_RELEASE)
//! Enable this to check for heap corruption on windows systems by walking the crt.
	#define MEMORY_ENABLE_DEBUG_HEAP 0
#endif

// Scope based heap checker macro
#if DRX_PLATFORM_WINDOWS && MEMORY_ENABLE_DEBUG_HEAP
	#include <crtdbg.h>
	#define MEMORY_CHECK_HEAP() do { if (!_CrtCheckMemory()) __debugbreak(); } while (0);
struct _HeapChecker
{
	_HeapChecker() { MEMORY_CHECK_HEAP(); }
	~_HeapChecker() { MEMORY_CHECK_HEAP(); }
};
	#define MEMORY_SCOPE_CHECK_HEAP_NAME_EVAL(x, y) x ## y
	#define MEMORY_SCOPE_CHECK_HEAP_NAME MEMORY_SCOPE_CHECK_HEAP_NAME_EVAL(__heap_checker__, __LINE__)
	#define MEMORY_SCOPE_CHECK_HEAP()               _HeapChecker MMRM_SCOPE_CHECK_HEAP_NAME
#endif

#if !defined(MEMORY_CHECK_HEAP)
	#define MEMORY_CHECK_HEAP() void(NULL)
#endif
#if !defined(MEMORY_SCOPE_CHECK_HEAP)
	#define MEMORY_SCOPE_CHECK_HEAP() void(NULL)
#endif

//////////////////////////////////////////////////////////////////////////
// Type to allow users of the allocation-defines to specialize on these
// types of memory allocation.
// Additionally it prevents the pointer from being manipulated or accessed
// in a unintended fashion: ++,--,any manipulation of the pointer value etc.
// Members are public, and intended hacking is still possible.

template<class T>
struct SHeapAllocation
{
	size_t size;
	T*     address;
	operator T*() const { return address; }
	T* operator->() const { return address; }
};

// Workaround for: "template<class T> using stackmemory_t = memory_t<T>;"
template<class T>
struct SStackAllocation
{
	size_t size;
	T*     address;
	operator T*() const { return address; }
	T* operator->() const { return address; }

	operator SHeapAllocation<T> &() const { return *reinterpret_cast<SHeapAllocation<T>*>(this); }
};

//////////////////////////////////////////////////////////////////////////
// Some compilers/platforms do not guarantee any alignment for alloca, although standard says it should be sufficient for built-in types.
// Experimental results show that it's possible to get entirely unaligned results (ie, LSB of address set).
#define ALLOCA_ALIGN 1U

// Fire an assert when the allocation is large enough to risk a stack overflow
#define ALLOCA_LIMIT (128U * 1024)

// Needs to be a define, because _alloca() frees it's memory when going out of scope.
#define DrxStackAllocVector(Type, Count, Alignment)                                               \
  (Type*)(((uintptr_t)alloca(((Count) * sizeof(Type) + (Alignment - 1)) & ~(Alignment - 1))))

#define DrxStackAllocAlignedOffs(AlignmentFunc)                                                   \
  (AlignmentFunc(1) > ALLOCA_ALIGN ? AlignmentFunc(1) - ALLOCA_ALIGN : 0)
#define DrxStackAllocAlignedPtr(Type, Size, Offset, AlignmentFunc)                                \
  (Type*)AlignmentFunc((uintptr_t)alloca((Size) + (Offset)))

#define DrxStackAllocWithSize(Type, Name, AlignmentFunc)                                          \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type));                                        \
  const size_t Name ## Offset = DrxStackAllocAlignedOffs(AlignmentFunc);                          \
  assert(Name ## Size + Name ## Offset <= ALLOCA_LIMIT);                                          \
  Type* Name ## Mem = DrxStackAllocAlignedPtr(Type, Name ## Size, Name ## Offset, AlignmentFunc); \
  const SStackAllocation<Type> Name = { Name ## Size, Name ## Mem };

#define DrxStackAllocWithSizeCleared(Type, Name, AlignmentFunc)                                   \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type));                                        \
  const size_t Name ## Offset = DrxStackAllocAlignedOffs(AlignmentFunc);                          \
  assert(Name ## Size + Name ## Offset <= ALLOCA_LIMIT);                                          \
  Type* Name ## Mem = DrxStackAllocAlignedPtr(Type, Name ## Size, Name ## Offset, AlignmentFunc); \
  const SStackAllocation<Type> Name = { Name ## Size, Name ## Mem };                              \
  ZeroMemory(Name ## Mem, Name ## Size);

#define DrxStackAllocWithSizeVector(Type, Count, Name, AlignmentFunc)                             \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type) * (Count));                              \
  const size_t Name ## Offset = DrxStackAllocAlignedOffs(AlignmentFunc);                          \
  assert(Name ## Size + Name ## Offset <= ALLOCA_LIMIT);                                          \
  Type* Name ## Mem = DrxStackAllocAlignedPtr(Type, Name ## Size, Name ## Offset, AlignmentFunc); \
  const SStackAllocation<Type> Name = { Name ## Size, Name ## Mem };

#define DrxStackAllocWithSizeVectorCleared(Type, Count, Name, AlignmentFunc)                      \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type) * (Count));                              \
  const size_t Name ## Offset = DrxStackAllocAlignedOffs(AlignmentFunc);                          \
  assert(Name ## Size + Name ## Offset <= ALLOCA_LIMIT);                                          \
  Type* Name ## Mem = DrxStackAllocAlignedPtr(Type, Name ## Size, Name ## Offset, AlignmentFunc); \
  const SStackAllocation<Type> Name = { Name ## Size, Name ## Mem };                              \
  ZeroMemory(Name ## Mem, Name ## Size);

#include <memory>

namespace DrxStack
{

// A stl compliant stack allocator that uses the stack frame.
// It can only allocate a single time the full memory footprint
// of the stl container it's used for.
template<class T>
class CSingleBlockAllocator
{
public:
	typedef T                 value_type;
	typedef value_type*       pointer;
	typedef const value_type* const_pointer;
	typedef value_type&       reference;
	typedef const value_type& const_reference;
	typedef size_t            size_type;
	typedef ptrdiff_t         difference_type;
	typedef std::allocator<T> fallback_allocator_type;

	template<class value_type1> struct rebind
	{
		typedef CSingleBlockAllocator<value_type1> other;
	};

	CSingleBlockAllocator(const SStackAllocation<T>& stack_allocation)
	{
		assert(stack_allocation.address != nullptr);
		this->stack_capacity = stack_allocation.size;
		this->stack_memory = stack_allocation.address;
	}

	CSingleBlockAllocator(size_t stack_capacity, uk stack_memory)
	{
		assert(stack_memory != nullptr);
		this->stack_capacity = stack_capacity;
		this->stack_memory = reinterpret_cast<value_type*>(stack_memory);
	}

	CSingleBlockAllocator(const CSingleBlockAllocator<value_type>& other)
	{
		stack_capacity = other.stack_capacity;
		stack_memory = other.stack_memory;
	}

	CSingleBlockAllocator(const CSingleBlockAllocator<value_type>&& other)
	{
		stack_capacity = other.stack_capacity;
		stack_memory = other.stack_memory;
	}

	~CSingleBlockAllocator()
	{
	}

	// in visual studio when in debug configuration,
	// stl containers create a proxy allocator and call allocate(1) once during construction
	// this copy constructor is needed to cope with that behavior, and will use a standard
	// allocator in case this occurs
	template<class value_type1> CSingleBlockAllocator(const CSingleBlockAllocator<value_type1>& other)
	{
		stack_capacity = 0;
		stack_memory = nullptr;
	}

	pointer       address(reference x) const       { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	value_type*   allocate(size_type n, ukk hint = 0)
	{
		if (stack_memory == nullptr)
		{
			// allocating something else than the main block
			return fallback_allocator_type().allocate(n, hint);
		}
		if (n != max_size())
		{
			// main block allocation is of the wrong size, this is a performance hazard, but not fatal
			assert(0 && "Only a reserve of the correct size is possible on the stack, falling back to heap memory.");
			return fallback_allocator_type().allocate(n, hint);
		}
		return stack_memory;
	}

	void deallocate(pointer p, size_type n)
	{
		if (stack_memory != p)
			return fallback_allocator_type().deallocate(p, n);
	}

	size_type max_size() const           { return stack_capacity / sizeof(value_type); }

	template<class U, class... Args>
	void construct(U* p, Args&&... args) { new (p) U(std::forward<Args>(args)...); }

	void      destroy(pointer p)         { p->~value_type(); }

	void      cleanup()                  {}

	size_t    get_heap_size()            { return 0; }

	size_t    get_wasted_in_allocation() { return 0; }

	size_t    get_wasted_in_blocks()     { return 0; }

private:
	// in case a standard allocator is used (which should only occur in debug configurations)
	// stack_memory is set to nullptr
	size_t      stack_capacity;
	value_type* stack_memory;
};

// template<template<typename, class> class C, typename T> using CSingleBlockContainer = C<T, CSingleBlockAllocator<T>>;
// template<typename T> using CSingleBlockVector = CSingleBlockContainer<std::vector, T>;
}

#define DrxStackAllocatorWithSize(Type, Name, AlignmentFunc)                                      \
  DrxStackAllocWithSize(Type, Name ## T, AlignmentFunc);                                          \
  DrxStack::CSingleBlockAllocator<Type> Name(Name ## T);

#define DrxStackAllocatorWithSizeCleared(Type, Name, AlignmentFunc)                               \
  DrxStackAllocWithSizeCleared(Type, Name ## T, AlignmentFunc);                                   \
  DrxStack::CSingleBlockAllocator<Type> Name(Name ## T);

#define DrxStackAllocatorWithSizeVector(Type, Count, Name, AlignmentFunc)                         \
  DrxStackAllocWithSizeVector(Type, Count, Name ## T, AlignmentFunc);                             \
  DrxStack::CSingleBlockAllocator<Type> Name(Name ## T);

#define DrxStackAllocatorWithSizeVectorCleared(Type, Count, Name, AlignmentFunc)                  \
  DrxStackAllocWithSizeVectorCleared(Type, Count, Name ## T, AlignmentFunc);                      \
  DrxStack::CSingleBlockAllocator<Type> Name(Name ## T);

//////////////////////////////////////////////////////////////////////////
// variation of DrxStackAlloc behaving identical, except memory is taken from heap instead of stack
#define DrxScopedMem(Type, Size, Name, AlignmentFunc)                                             \
  struct Name ## SMemScoped {                                                                     \
    Name ## SMemScoped(size_t S) {                                                                \
      Name = reinterpret_cast<Type*>(DrxModuleMemalign(S, AlignmentFunc(1))); }                   \
    ~Name ## SMemScoped() {                                                                       \
      if (Name != nullptr) DrxModuleMemalignFree(Name); }                                         \
    Type* Name;                                                                                   \
  };                                                                                              \
  Name ## SMemScoped Name ## MemScoped(Size);                                                     \

#define DrxScopedAllocWithSize(Type, Name, AlignmentFunc)                                         \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type));                                        \
  DrxScopedMem(Type, Name ## Size, Name, AlignmentFunc);                                          \
  Type* Name ## Mem = Name ## MemScoped.Name;                                                     \
  const SHeapAllocation<Type> Name = { Name ## Size, Name ## Mem };

#define DrxScopedAllocWithSizeCleared(Type, Name, AlignmentFunc)                                  \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type));                                        \
  DrxScopedMem(Type, Name ## Size, Name, AlignmentFunc);                                          \
  Type* Name ## Mem = Name ## MemScoped.Name;                                                     \
  const SHeapAllocation<Type> Name = { Name ## Size, Name ## Mem };                               \
  ZeroMemory(Name ## Mem, Name ## Size);

#define DrxScopedAllocWithSizeVector(Type, Count, Name, AlignmentFunc)                            \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type) * (Count));                              \
  DrxScopedMem(Type, Name ## Size, Name, AlignmentFunc);                                          \
  Type* Name ## Mem = Name ## MemScoped.Name;                                                     \
  const SHeapAllocation<Type> Name = { Name ## Size, Name ## Mem };

#define DrxScopedAllocWithSizeVectorCleared(Type, Count, Name, AlignmentFunc)                     \
  const size_t Name ## Size = AlignmentFunc(sizeof(Type) * (Count));                              \
  DrxScopedMem(Type, Name ## Size, Name, AlignmentFunc);                                          \
  Type* Name ## Mem = Name ## MemScoped.Name;                                                     \
  const SHeapAllocation<Type> Name = { Name ## Size, Name ## Mem };                               \
  ZeroMemory(Name ## Mem, Name ## Size);

//////////////////////////////////////////////////////////////////////////
// Define this if you want to use slow debug memory manager in any config.
//////////////////////////////////////////////////////////////////////////
//#define DEBUG_MEMORY_MANAGER
//////////////////////////////////////////////////////////////////////////

// That mean we use node_allocator for all small allocations

#include <drx3D/CoreX/Platform/platform.h>

#include <stdarg.h>
#include <type_traits>
#include <new>

#define _DRX_DEFAULT_MALLOC_ALIGNMENT 4

#if !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
	#include <malloc.h>
#endif

// Allow launcher to export functions as e.g. render DLL is still linked dynamically
#if defined(DRXSYSTEM_EXPORTS) || (defined(_LIB) && defined(_LAUNCHER))
	#define DRXMEMORYMANAGER_API DLL_EXPORT
#else
	#define DRXMEMORYMANAGER_API DLL_IMPORT
#endif

#ifdef __cplusplus
//////////////////////////////////////////////////////////////////////////
	#ifdef DEBUG_MEMORY_MANAGER
		#ifdef _DEBUG
			#define _DEBUG_MODE
		#endif
	#endif

	#if defined(_DEBUG) && DRX_PLATFORM_WINAPI
		#include <crtdbg.h>
	#endif

//! Checks if the heap is valid in debug; in release, this function shouldn't be called.
//! \return Non-0 if it's valid and 0 if not valid.
ILINE i32 IsHeapValid()
{
	#if (defined(_DEBUG) && !defined(RELEASE_RUNTIME) && DRX_PLATFORM_WINAPI) || (defined(DEBUG_MEMORY_MANAGER))
	return _CrtCheckMemory();
	#else
	return true;
	#endif
}

	#ifdef DEBUG_MEMORY_MANAGER
// Restore debug mode define
		#ifndef _DEBUG_MODE
			#undef _DEBUG
		#endif
	#endif
//////////////////////////////////////////////////////////////////////////

#endif //__cplusplus

struct ICustomMemoryHeap;
class IGeneralMemoryHeap;
class IPageMappingHeap;
class IDefragAllocator;
class IMemoryAddressRange;

//! Interfaces that allow access to the DinrusX memory manager.
struct IMemoryUpr
{
	typedef u8 HeapHandle;
	enum { BAD_HEAP_HANDLE = 0xFF };

	struct SProcessMemInfo
	{
		uint64 PageFaultCount;
		uint64 PeakWorkingSetSize;
		uint64 WorkingSetSize;
		uint64 QuotaPeakPagedPoolUsage;
		uint64 QuotaPagedPoolUsage;
		uint64 QuotaPeakNonPagedPoolUsage;
		uint64 QuotaNonPagedPoolUsage;
		uint64 PagefileUsage;
		uint64 PeakPagefileUsage;

		uint64 TotalPhysicalMemory;
		int64  FreePhysicalMemory;

		uint64 TotalVideoMemory;
		int64  FreeVideoMemory;
	};

	enum EAllocPolicy
	{
		eapDefaultAllocator,
		eapPageMapped,
		eapCustomAlignment,
#if DRX_PLATFORM_DURANGO
		eapAPU,
#endif
	};

	virtual ~IMemoryUpr(){}

	virtual bool GetProcessMemInfo(SProcessMemInfo& minfo) = 0;

	//! Used to add memory block size allocated directly from the crt or OS to the memory manager statistics.
	virtual void FakeAllocation(long size) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Heap Tracing API.
	virtual HeapHandle TraceDefineHeap(tukk heapName, size_t size, ukk pBase) = 0;
	virtual void       TraceHeapAlloc(HeapHandle heap, uk mem, size_t size, size_t blockSize, tukk sUsage, tukk sNameHint = 0) = 0;
	virtual void       TraceHeapFree(HeapHandle heap, uk mem, size_t blockSize) = 0;
	virtual void       TraceHeapSetColor(u32 color) = 0;
	virtual u32     TraceHeapGetColor() = 0;
	virtual void       TraceHeapSetLabel(tukk sLabel) = 0;
	//////////////////////////////////////////////////////////////////////////

	//! Retrieve access to the MemReplay implementation class.
	virtual struct IMemReplay* GetIMemReplay() = 0;

	//! Create an instance of ICustomMemoryHeap.
	virtual ICustomMemoryHeap* const CreateCustomMemoryHeapInstance(EAllocPolicy const eAllocPolicy) = 0;

	virtual IGeneralMemoryHeap*      CreateGeneralExpandingMemoryHeap(size_t upperLimit, size_t reserveSize, tukk sUsage) = 0;
	virtual IGeneralMemoryHeap*      CreateGeneralMemoryHeap(uk base, size_t sz, tukk sUsage) = 0;

	virtual IMemoryAddressRange*     ReserveAddressRange(size_t capacity, tukk sName) = 0;
	virtual IPageMappingHeap*        CreatePageMappingHeap(size_t addressSpace, tukk sName) = 0;

	virtual IDefragAllocator*        CreateDefragAllocator() = 0;

	virtual uk                    AllocPages(size_t size) = 0;
	virtual void                     FreePages(uk p, size_t size) = 0;
};

//! Global function implemented in DrxMemoryUpr_impl.h.
IMemoryUpr* DrxGetIMemoryUpr();

class STraceHeapAllocatorAutoColor
{
public:
	explicit STraceHeapAllocatorAutoColor(u32 color) { m_color = DrxGetIMemoryUpr()->TraceHeapGetColor(); DrxGetIMemoryUpr()->TraceHeapSetColor(color); }
	~STraceHeapAllocatorAutoColor() { DrxGetIMemoryUpr()->TraceHeapSetColor(m_color); };
protected:
	u32 m_color;
	STraceHeapAllocatorAutoColor() {};
};

#define TRACEHEAP_AUTOCOLOR(color) STraceHeapAllocatorAutoColor _auto_color_(color);

//! Structure filled by call to DrxModuleGetMemoryInfo().
struct DrxModuleMemoryInfo
{
	uint64 requested;

	uint64 allocated;           //!< Total amount of memory allocated.
	uint64 freed;               //!< Total amount of memory freed.
	i32    num_allocations;     //!< Total number of memory allocations.
	uint64 DrxString_allocated; //!< Allocated in DrxString.
	uint64 STL_allocated;       //!< Allocated in STL.
	uint64 STL_wasted;          //!< Amount of memory wasted in pools in stl (not useful allocations).
};

struct DrxReplayInfo
{
	uint64      uncompressedLength;
	uint64      writtenLength;
	u32      trackingSize;
	tukk filename;
};

//////////////////////////////////////////////////////////////////////////
// Extern declarations of globals inside DinrusSystem.
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

DRXMEMORYMANAGER_API uk  DrxMalloc(size_t size, size_t& allocated, size_t alignment);
DRXMEMORYMANAGER_API uk  DrxRealloc(uk memblock, size_t size, size_t& allocated, size_t& oldsize, size_t alignment);
DRXMEMORYMANAGER_API size_t DrxFree(uk p, size_t alignment);
DRXMEMORYMANAGER_API size_t DrxGetMemSize(uk p, size_t size);
DRXMEMORYMANAGER_API i32    DrxStats(tuk buf);
DRXMEMORYMANAGER_API void   DrxFlushAll();
DRXMEMORYMANAGER_API void   DrxCleanup();
DRXMEMORYMANAGER_API i32    DrxGetUsedHeapSize();
DRXMEMORYMANAGER_API i32    DrxGetWastedHeapSize();
DRXMEMORYMANAGER_API uk  DrxSystemCrtMalloc(size_t size);
DRXMEMORYMANAGER_API uk  DrxSystemCrtRealloc(uk p, size_t size);
DRXMEMORYMANAGER_API size_t DrxSystemCrtFree(uk p);
DRXMEMORYMANAGER_API size_t DrxSystemCrtSize(uk p);
DRXMEMORYMANAGER_API size_t DrxSystemCrtGetUsedSpace();
DRXMEMORYMANAGER_API void   DrxGetIMemoryUprInterface(uk * pIMemoryUpr);

// This function is local in every module
/*DRXMEMORYMANAGER_API*/
void DrxGetMemoryInfoForModule(DrxModuleMemoryInfo* pInfo);

#ifdef __cplusplus
}
#endif //__cplusplus

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Drx Memory Upr accessible in all build modes.
//////////////////////////////////////////////////////////////////////////
#if !defined(USING_DRX_MEMORY_MANAGER)
	#define USING_DRX_MEMORY_MANAGER
#endif

#ifdef _LIB
	#define DRX_MEM_USAGE_API
#else
	#define DRX_MEM_USAGE_API extern "C" DLL_EXPORT
#endif

#include <drx3D/Sys/DrxMemReplay.h>

#if CAPTURE_REPLAY_LOG
	#define DRXMM_INLINE inline
#else
	#define DRXMM_INLINE ILINE
#endif

#if defined(NOT_USE_DRX_MEMORY_MANAGER)
DRXMM_INLINE uk DrxModuleMalloc(size_t size) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ptr = malloc(size);
	MEMREPLAY_SCOPE_ALLOC(ptr, size, 0);
	return ptr;
}

DRXMM_INLINE uk DrxModuleRealloc(uk memblock, size_t size) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = realloc(memblock, size);
	MEMREPLAY_SCOPE_REALLOC(memblock, ret, size, 0);
	return ret;
}

DRXMM_INLINE void DrxModuleFree(uk memblock) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	free(memblock);
	MEMREPLAY_SCOPE_FREE(memblock);
}

DRXMM_INLINE void DrxModuleMemalignFree(uk memblock) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	#if defined(__GNUC__) && !DRX_PLATFORM_APPLE
	free(memblock);
	#else
	_aligned_free(memblock);
	#endif
	MEMREPLAY_SCOPE_FREE(memblock);
}

DRXMM_INLINE uk DrxModuleReallocAlign(uk memblock, size_t size, size_t alignment) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	#if defined(__GNUC__)
	// realloc makes no guarantees about the alignment of memory.  Rather than unconditionally
	// copying data, if the new allocation we got back from realloc isn't properly aligned:
	// 1) Create new properly aligned allocation
	// 2) Copy original data to aligned allocation
	// 3) Free and replace unaligned allocation
	uk ret = realloc(memblock, size);
	if ((size_t(ret) & (std::max<size_t>(alignment, 1) - 1)) != 0)       // MAX handles alignment == 0
	{
		// Should be able to use malloc_usage_size() to copy the correct amount from the original allocation,
		// but it's left undefined in Android NDK.  Thanks...
		//const size_t oldSize = memblock ? _msize(memblock) : 0;
		const size_t oldSize = size;
		// memalign is deprecated but not all platforms have aligned_alloc()...
		uk alignedAlloc = aligned_alloc(size, alignment);
		if (alignedAlloc && oldSize > 0)
		{
			// We copy from the unaligned re-allocation rather than original memblock to ensure we
			// don't read beyond the end of any allocated memory.  Even if oldSize == newSize when
			// malloc_usage_size(memblock) < newSize, we know that malloc_usage_size(ret) >= newSize
			memcpy(alignedAlloc, ret, size);
		}
		free(ret);
		ret = alignedAlloc;
	}
	return ret;
	#else
	uk ret = _aligned_realloc(memblock, size, alignment);
	#endif
	MEMREPLAY_SCOPE_REALLOC(memblock, ret, size, alignment);
	return ret;
}

DRXMM_INLINE uk DrxModuleMemalign(size_t size, size_t alignment) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc);
	uk ret = aligned_alloc(size, alignment);
	MEMREPLAY_SCOPE_ALLOC(ret, size, alignment);
	return ret;
}

#else //NOT_USE_DRX_MEMORY_MANAGER

/////////////////////////////////////////////////////////////////////////
// Extern declarations,used by overridden new and delete operators.
//////////////////////////////////////////////////////////////////////////
extern "C"
{
	// Allocation set 1, uses system default alignment.
	// Never mix with functions from set 2.
	uk DrxModuleMalloc(size_t size) noexcept;
	uk DrxModuleRealloc(uk memblock, size_t size) noexcept;
	void  DrxModuleFree(uk ptr) noexcept;
	uk DrxModuleCalloc(size_t a, size_t b) noexcept;

	// Allocation set 2, uses custom alignment.
	// You may pass alignment 0 if you want system default alignment.
	// Never mix with functions from set 1.
	uk DrxModuleMemalign(size_t size, size_t alignment) noexcept;
	uk DrxModuleReallocAlign(uk memblock, size_t size, size_t alignment) noexcept;
	void  DrxModuleMemalignFree(uk ) noexcept;

	// When inspecting a block from set 1, you must pass alignment 0.
	// When inspecting a block from set 2, you must pass in the original alignment value (which could be 0).
	size_t DrxModuleMemSize(uk ptr, size_t alignment = 0) noexcept;
}

DRXMM_INLINE uk DrxModuleCRTMalloc(size_t s) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtMalloc);
	uk ret = DrxModuleMalloc(s);
	MEMREPLAY_SCOPE_ALLOC(ret, s, 0);
	return ret;
}

DRXMM_INLINE uk DrxModuleCRTRealloc(uk p, size_t s) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtMalloc);
	uk ret = DrxModuleRealloc(p, s);
	MEMREPLAY_SCOPE_REALLOC(p, ret, s, 0);
	return ret;
}

DRXMM_INLINE void DrxModuleCRTFree(uk p) noexcept
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtMalloc);
	DrxModuleFree(p);
	MEMREPLAY_SCOPE_FREE(p);
}

	#define malloc  DrxModuleCRTMalloc
	#define realloc DrxModuleCRTRealloc
	#define free    DrxModuleCRTFree
	#define calloc  DrxModuleCalloc

#endif //NOT_USE_DRX_MEMORY_MANAGER

DRX_MEM_USAGE_API void DrxModuleGetMemoryInfo(DrxModuleMemoryInfo* pMemInfo);

#if !defined(NOT_USE_DRX_MEMORY_MANAGER)
// Note: No need to override placement new, new[], delete, delete[]
PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new(std::size_t size);

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new(std::size_t size, const std::nothrow_t& nothrow_value) noexcept;

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new[](std::size_t size);

PREFAST_SUPPRESS_WARNING(28251)
uk __cdecl operator new[](std::size_t size, const std::nothrow_t& nothrow_value) noexcept;


void __cdecl operator delete (uk ptr) noexcept;
void __cdecl operator delete (uk ptr, const std::nothrow_t& nothrow_constant) noexcept;

void __cdecl operator delete[] (uk ptr) noexcept;
void __cdecl operator delete[] (uk ptr, const std::nothrow_t& nothrow_constant) noexcept;
#endif

//! Needed for our allocator to avoid deadlock in cleanup.
uk  DrxCrtMalloc(size_t size);
size_t DrxCrtFree(uk p);

//! Wrapper for _msize on PC.
size_t DrxCrtSize(uk p);

#if !defined(NOT_USE_DRX_MEMORY_MANAGER)
	#include "DrxMemoryAllocator.h"
#endif

//! These utility functions should be used for allocating objects with specific alignment requirements on the heap.
//! \note On MSVC before 2013, only zero to three argument are supported, because C++11 support is not complete.
#if !defined(_MSC_VER) || _MSC_VER >= 1800

template<typename T, typename ... Args>
inline T* DrxAlignedNew(Args&& ... args)
{
	uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);
	return new(pAlignedMemory) T(std::forward<Args>(args) ...);
}

#else

template<typename T>
inline T* DrxAlignedNew()
{
	uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);
	return new(pAlignedMemory) T();
}

template<typename T, typename A1>
inline T* DrxAlignedNew(A1&& a1)
{
	uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);
	return new(pAlignedMemory) T(std::forward<A1>(a1));
}

template<typename T, typename A1, typename A2>
inline T* DrxAlignedNew(A1&& a1, A2&& a2)
{
	uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);
	return new(pAlignedMemory) T(std::forward<A1>(a1), std::forward<A2>(a2));
}

template<typename T, typename A1, typename A2, typename A3>
inline T* DrxAlignedNew(A1&& a1, A2&& a2, A3&& a3)
{
	uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);
	return new(pAlignedMemory) T(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
}

#endif

//! This utility function should be used for allocating arrays of objects with specific alignment requirements on the heap.
//! \note The caller must remember the number of items in the array, since DrxAlignedDeleteArray needs this information.
template<typename T>
inline T* DrxAlignedNewArray(size_t count)
{
	T* const pAlignedMemory = reinterpret_cast<T*>(DrxModuleMemalign(sizeof(T) * count, std::alignment_of<T>::value));
	T* pCurrentItem = pAlignedMemory;
	for (size_t i = 0; i < count; ++i, ++pCurrentItem)
	{
		new(static_cast<uk>(pCurrentItem))T();
	}
	return pAlignedMemory;
}

//! Utility function that frees an object previously allocated with DrxAlignedNew.
template<typename T>
inline void DrxAlignedDelete(T* pObject)
{
	if (pObject)
	{
		pObject->~T();
		DrxModuleMemalignFree(pObject);
	}
}

//! Utility function that frees an array of objects previously allocated with DrxAlignedNewArray.
//! The same count used to allocate the array must be passed to this function.
template<typename T>
inline void DrxAlignedDeleteArray(T* pObject, size_t count)
{
	if (pObject)
	{
		for (size_t i = 0; i < count; ++i)
		{
			(pObject + i)->~T();
		}
		DrxModuleMemalignFree(pObject);
	}
}

#endif // __DrxMemoryUpr_h__

