// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/Sys/GeneralMemoryHeap.h>
#include <drx3D/Sys/PageMappingHeap.h>

CGeneralMemoryHeap::CGeneralMemoryHeap()
	: m_nRefCount(0)
	, m_numAllocs(0)
	, m_isResizable(false)
	, m_mspace(0)
	, m_pHeap(NULL)
	, m_pBlock(NULL)
	, m_blockSize(0)
{
}

CGeneralMemoryHeap::CGeneralMemoryHeap(UINT_PTR base, size_t upperLimit, size_t reserveSize, tukk sUsage)
	: m_nRefCount(0)
	, m_numAllocs(0)
	, m_isResizable(true)
	, m_pHeap(NULL)
	, m_pBlock(NULL)
	, m_blockSize(0)
{
	if (base)
		m_pHeap = new CPageMappingHeap((tuk)base, upperLimit / CMemoryAddressRange::GetSystemPageSize(), CMemoryAddressRange::GetSystemPageSize(), sUsage);
	else
		m_pHeap = new CPageMappingHeap(upperLimit, sUsage);

	m_mspace = dlcreate_mspace(max(0, (i32)reserveSize - dlmspace_create_overhead()), 0, this, DLMMap, DLMUnMap);

#ifdef DRX_TRACE_HEAP
	UINT_PTR addressSpaceBase = reinterpret_cast<UINT_PTR>(m_mspace) & ~(m_pHeap->GetGranularity() - 1);
	m_nTraceHeapHandle = DrxGetIMemoryUpr()->TraceDefineHeap(sUsage, upperLimit, reinterpret_cast<uk>(addressSpaceBase));
#endif // DRX_TRACE_HEAP
}

CGeneralMemoryHeap::CGeneralMemoryHeap(uk base, size_t size, tukk sUsage)
	: m_nRefCount(0)
	, m_numAllocs(0)
	, m_isResizable(false)
	, m_pHeap(NULL)
	, m_pBlock(NULL)
	, m_blockSize(0)
{
	m_pBlock = base;
	m_blockSize = size;

	m_mspace = dlcreate_mspace_with_base(base, size, 1);

#ifdef DRX_TRACE_HEAP
	UINT_PTR addressSpaceBase = reinterpret_cast<UINT_PTR>(m_mspace);
	m_nTraceHeapHandle = DrxGetIMemoryUpr()->TraceDefineHeap(sUsage, size, reinterpret_cast<uk>(addressSpaceBase));
#endif // DRX_TRACE_HEAP
}

CGeneralMemoryHeap::~CGeneralMemoryHeap()
{
	if (m_mspace)
	{
		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
		dldestroy_mspace(m_mspace);
	}

	delete m_pHeap;
}

bool CGeneralMemoryHeap::Cleanup()
{
	bool bCleanedFully = false;

	if (m_isResizable)
	{
		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
		if (m_numAllocs == 0)
		{
			// There aren't any allocations remaining in dlmalloc, so reset it to fully
			// relinquish any memory it's holding onto.
			dldestroy_mspace(m_mspace);
			m_mspace = dlcreate_mspace(0, 0, this, DLMMap, DLMUnMap);

			bCleanedFully = true;
		}
		else
		{
			dlmspace_trim(m_mspace, 0);
		}
	}

	return bCleanedFully;
}

i32 CGeneralMemoryHeap::AddRef()
{
	return DrxInterlockedIncrement(&m_nRefCount);
}

i32 CGeneralMemoryHeap::Release()
{
	i32 nRef = DrxInterlockedDecrement(&m_nRefCount);

#if !defined(_RELEASE)
	//	IF (nRef < 0, 0)
	//	  __debugbreak();
#endif
	if (nRef == 0)
		delete this;

	return nRef;
}

bool CGeneralMemoryHeap::IsInAddressRange(uk ptr) const
{
	if (m_isResizable)
	{
		return m_pHeap->IsInAddressRange(ptr);
	}
	else
	{
		UINT_PTR base = reinterpret_cast<UINT_PTR>(m_pBlock);
		UINT_PTR end = base + m_blockSize;
		UINT_PTR ptri = reinterpret_cast<UINT_PTR>(ptr);
		return base <= ptri && ptri < end;
	}
}

uk CGeneralMemoryHeap::Calloc(size_t nmemb, size_t size, tukk sUsage)
{
#if CAPTURE_REPLAY_LOG
	bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
	uk ptr = dlmspace_calloc(m_mspace, nmemb, size);

#ifdef DRX_TRACE_HEAP
	if (sUsage && m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		DrxGetIMemoryUpr()->TraceHeapAlloc(m_nTraceHeapHandle, ptr, size, size, sUsage);
	}
#endif // DRX_TRACE_HEAP

	if (ptr)
	{
		++m_numAllocs;
	}

#if CAPTURE_REPLAY_LOG
	if (ms)
		DrxGetIMemReplay()->ExitScope_Alloc((UINT_PTR)ptr, (UINT_PTR)(nmemb * size));
#endif

	return ptr;
}

uk CGeneralMemoryHeap::Malloc(size_t sz, tukk sUsage)
{
#if CAPTURE_REPLAY_LOG
	bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
	uk ptr = dlmspace_malloc(m_mspace, sz);

#ifdef DRX_TRACE_HEAP
	if (sUsage && m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		DrxGetIMemoryUpr()->TraceHeapAlloc(m_nTraceHeapHandle, ptr, sz, sz, sUsage);
	}
#endif // DRX_TRACE_HEAP

	if (ptr)
	{
		++m_numAllocs;
	}

#if CAPTURE_REPLAY_LOG
	if (ms)
		DrxGetIMemReplay()->ExitScope_Alloc((UINT_PTR)ptr, (UINT_PTR)sz);
#endif

	return ptr;
}

size_t CGeneralMemoryHeap::Free(uk ptr)
{
	UINT_PTR ptri = reinterpret_cast<UINT_PTR>(ptr);

	if (CGeneralMemoryHeap::IsInAddressRange(ptr))
	{
#if CAPTURE_REPLAY_LOG
		bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
		size_t sz = dlmspace_usable_size(ptr);
		dlmspace_free(m_mspace, ptr);

#ifdef DRX_TRACE_HEAP
		if (m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
		{
			DrxGetIMemoryUpr()->TraceHeapFree(m_nTraceHeapHandle, ptr, sz);
		}
#endif // DRX_TRACE_HEAP

		--m_numAllocs;

#if CAPTURE_REPLAY_LOG
		if (ms)
			DrxGetIMemReplay()->ExitScope_Free((UINT_PTR)ptr);
#endif

		return sz;
	}

	return 0;
}

uk CGeneralMemoryHeap::Realloc(uk ptr, size_t sz, tukk sUsage)
{
	if (!ptr)
	{
		return Malloc(sz, sUsage);
	}

	if (!sz)
	{
		Free(ptr);
		return NULL;
	}

#if CAPTURE_REPLAY_LOG
	bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);

#ifdef DRX_TRACE_HEAP
	if (m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		size_t szOld = dlmspace_usable_size(ptr);
		DrxGetIMemoryUpr()->TraceHeapFree(m_nTraceHeapHandle, ptr, szOld);
	}
#endif // DRX_TRACE_HEAP

	uk pNewPtr = dlmspace_realloc(m_mspace, ptr, sz);

#ifdef DRX_TRACE_HEAP
	if (sUsage && m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		DrxGetIMemoryUpr()->TraceHeapAlloc(m_nTraceHeapHandle, pNewPtr, sz, sz, sUsage);
	}
#endif // DRX_TRACE_HEAP

#if CAPTURE_REPLAY_LOG
	if (ms)
	{
		if (pNewPtr)
			DrxGetIMemReplay()->ExitScope_Realloc((UINT_PTR)ptr, (UINT_PTR)pNewPtr, (UINT_PTR)sz);
		else
			DrxGetIMemReplay()->ExitScope();
	}
#endif

	return pNewPtr;
}

uk CGeneralMemoryHeap::ReallocAlign(uk ptr, size_t size, size_t alignment, tukk sUsage)
{
	if (!ptr)
	{
		return Memalign(alignment, size, sUsage);
	}

	if (!size)
	{
		Free(ptr);
		return NULL;
	}

#if CAPTURE_REPLAY_LOG
	bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);

	size_t oldSize = dlmspace_usable_size(ptr);
	uk newPtr = dlmspace_memalign(m_mspace, alignment, size);

	// Check if the new allocation failed before copying to new location
	if (newPtr == NULL)
		return NULL;

	memcpy(newPtr, ptr, std::min<size_t>(oldSize, size));
	dlmspace_free(m_mspace, ptr);

#ifdef DRX_TRACE_HEAP
	if (sUsage && m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		DrxGetIMemoryUpr()->TraceHeapFree(m_nTraceHeapHandle, ptr, oldSize);
		DrxGetIMemoryUpr()->TraceHeapAlloc(m_nTraceHeapHandle, newPtr, size, size, sUsage);
	}
#endif // DRX_TRACE_HEAP

#if CAPTURE_REPLAY_LOG
	if (ms)
		DrxGetIMemReplay()->ExitScope_Realloc((UINT_PTR)ptr, (UINT_PTR)newPtr, (UINT_PTR)size, (UINT_PTR)alignment);
#endif

	return newPtr;
}

uk CGeneralMemoryHeap::Memalign(size_t boundary, size_t size, tukk sUsage)
{
#if CAPTURE_REPLAY_LOG
	bool ms = m_isResizable ? DrxGetIMemReplay()->EnterScope(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_DrxMalloc, eDrxModule) : 0;
#endif

	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_mspaceLock);
	uk ptr = dlmspace_memalign(m_mspace, boundary, size);

#ifdef DRX_TRACE_HEAP
	if (sUsage && m_nTraceHeapHandle != IMemoryUpr::BAD_HEAP_HANDLE)
	{
		DrxGetIMemoryUpr()->TraceHeapAlloc(m_nTraceHeapHandle, ptr, size, Align(size, boundary), sUsage);
	}
#endif // DRX_TRACE_HEAP

	if (ptr)
	{
		++m_numAllocs;
	}

#if CAPTURE_REPLAY_LOG
	if (ms)
		DrxGetIMemReplay()->ExitScope_Alloc((UINT_PTR)ptr, (UINT_PTR)size, (UINT_PTR)boundary);
#endif

	return ptr;
}

size_t CGeneralMemoryHeap::UsableSize(uk ptr) const
{
	UINT_PTR ptri = reinterpret_cast<UINT_PTR>(ptr);

	if (CGeneralMemoryHeap::IsInAddressRange(ptr))
	{
		DrxAutoLock<DrxCriticalSectionNonRecursive> lock(const_cast<DrxCriticalSectionNonRecursive&>(m_mspaceLock));
		return dlmspace_usable_size(ptr);
	}

	return 0;
}

uk CGeneralMemoryHeap::DLMMap(uk vself, size_t length)
{
	CGeneralMemoryHeap* heap = reinterpret_cast<CGeneralMemoryHeap*>(vself);
	uk ptr = heap->m_pHeap->Map(length);
	if (ptr)
		return ptr;
	else
		return dlmmap_error;
}

i32 CGeneralMemoryHeap::DLMUnMap(uk vself, uk mem, size_t length)
{
	CGeneralMemoryHeap* heap = reinterpret_cast<CGeneralMemoryHeap*>(vself);
	heap->m_pHeap->Unmap(mem, length);
	return 0;
}
