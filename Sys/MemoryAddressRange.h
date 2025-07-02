// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef MEMORYADDRESSRANGE_H
#define MEMORYADDRESSRANGE_H

#include <drx3D/CoreX/Memory/IMemory.h>

class CMemoryAddressRange : public IMemoryAddressRange
{
public:
	static uk  ReserveSpace(size_t sz);
	static size_t GetSystemPageSize();

public:
	CMemoryAddressRange(tuk pBaseAddress, size_t nPageSize, size_t nPageCount, tukk sName);
	CMemoryAddressRange(size_t capacity, tukk name);
	~CMemoryAddressRange();

	ILINE bool IsInRange(uk p) const
	{
		return m_pBaseAddress <= p && p < (m_pBaseAddress + m_nPageSize * m_nPageCount);
	}

public:
	void   Release();

	tuk  GetBaseAddress() const;
	size_t GetPageCount() const;
	size_t GetPageSize() const;

	uk  MapPage(size_t pageIdx);
	void   UnmapPage(size_t pageIdx);

private:
	CMemoryAddressRange(const CMemoryAddressRange&);
	CMemoryAddressRange& operator=(const CMemoryAddressRange&);

private:
	tuk  m_pBaseAddress;
	size_t m_nPageSize;
	size_t m_nPageCount;
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	size_t m_allocatedSpace; // Required to unmap latter on
#endif
};

#endif
