// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef GENERALMEMORYHEAP_H
#define GENERALMEMORYHEAP_H

#include <drx3D/CoreX/Memory/IMemory.h>
#include <drx3D/CoreX/Platform/DrxDLMalloc.h>

class CPageMappingHeap;

class CGeneralMemoryHeap : public IGeneralMemoryHeap
{
public:
	CGeneralMemoryHeap();

	// Create a heap that will map/unmap pages in the range [baseAddress, baseAddress + upperLimit).
	CGeneralMemoryHeap(UINT_PTR baseAddress, size_t upperLimit, size_t reserveSize, tukk sUsage);

	// Create a heap that will assumes all memory in the range [base, base + size) is already mapped.
	CGeneralMemoryHeap(uk base, size_t size, tukk sUsage);

	~CGeneralMemoryHeap();

public: // IGeneralMemoryHeap Members
	bool   Cleanup();

	i32    AddRef();
	i32    Release();

	bool   IsInAddressRange(uk ptr) const;

	uk  Calloc(size_t nmemb, size_t size, tukk sUsage = NULL);
	uk  Malloc(size_t sz, tukk sUsage = NULL);
	size_t Free(uk ptr);
	uk  Realloc(uk ptr, size_t sz, tukk sUsage = NULL);
	uk  ReallocAlign(uk ptr, size_t size, size_t alignment, tukk sUsage = NULL);
	uk  Memalign(size_t boundary, size_t size, tukk sUsage = NULL);
	size_t UsableSize(uk ptr) const;

private:
	static uk DLMMap(uk self, size_t sz);
	static i32   DLMUnMap(uk self, uk mem, size_t sz);

private:
	CGeneralMemoryHeap(const CGeneralMemoryHeap&);
	CGeneralMemoryHeap& operator=(const CGeneralMemoryHeap&);

private:
	 i32                   m_nRefCount;

	bool                           m_isResizable;

	DrxCriticalSectionNonRecursive m_mspaceLock;
	dlmspace                       m_mspace;

	CPageMappingHeap*              m_pHeap;
	uk                          m_pBlock;
	size_t                         m_blockSize;

	i32                            m_numAllocs;

#ifdef DRX_TRACE_HEAP
	IMemoryUpr::HeapHandle m_nTraceHeapHandle;
#endif
};

#endif
