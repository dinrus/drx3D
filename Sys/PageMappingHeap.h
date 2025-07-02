// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef PAGEMAPPINGHEAP_H
#define PAGEMAPPINGHEAP_H

#include <drx3D/Sys/MemoryAddressRange.h>

#include <drx3D/CoreX/Memory/IMemory.h>

class CPageMappingHeap : public IPageMappingHeap
{
public:
	CPageMappingHeap(tuk pAddressSpace, size_t nNumPages, size_t nPageSize, tukk sName);
	CPageMappingHeap(size_t addressSpace, tukk sName);
	~CPageMappingHeap();

public: // IPageMappingHeap Members
	virtual void   Release();

	virtual size_t GetGranularity() const;
	virtual bool   IsInAddressRange(uk ptr) const;

	virtual size_t FindLargestFreeBlockSize() const;

	virtual uk  Map(size_t sz);
	virtual void   Unmap(uk ptr, size_t sz);

private:
	CPageMappingHeap(const CPageMappingHeap&);
	CPageMappingHeap& operator=(const CPageMappingHeap&);

private:
	void Init();

private:
	mutable DrxCriticalSectionNonRecursive m_lock;
	CMemoryAddressRange                    m_addrRange;
	std::vector<u32>                    m_pageBitmap;
};

#endif
