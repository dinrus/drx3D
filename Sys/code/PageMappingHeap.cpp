// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/PageMappingHeap.h>

#include <drx3D/CoreX/FindZeroRanges.h>

namespace
{
struct DLMMapFindBest
{
	DLMMapFindBest(size_t requiredLength)
		: requiredLength(requiredLength)
		, bestPosition(-1)
		, bestFragmentLength(INT_MAX)
	{
	}

	bool operator()(size_t position, size_t length)
	{
		if (length == requiredLength)
		{
			bestPosition = position;
			bestFragmentLength = 0;
			return false;
		}
		else if (length > requiredLength)
		{
			size_t fragment = length - requiredLength;
			if (fragment < bestFragmentLength)
			{
				bestPosition = position;
				bestFragmentLength = fragment;
			}
		}

		return true;
	}

	size_t    requiredLength;
	ptrdiff_t bestPosition;
	size_t    bestFragmentLength;
};

struct FindLargest
{
	FindLargest()
		: largest(0)
	{
	}
	bool operator()(size_t, size_t length)
	{
		largest = max(largest, length);
		return true;
	}

	size_t largest;
};
}

CPageMappingHeap::CPageMappingHeap(tuk pAddressSpace, size_t nNumPages, size_t nPageSize, tukk sName)
	: m_addrRange(pAddressSpace, nPageSize, nNumPages, sName)
{
	Init();
}

CPageMappingHeap::CPageMappingHeap(size_t addressSpace, tukk sName)
	: m_addrRange(addressSpace, sName)
{
	Init();
}

CPageMappingHeap::~CPageMappingHeap()
{
}

void CPageMappingHeap::Release()
{
	delete this;
}

size_t CPageMappingHeap::GetGranularity() const
{
	return m_addrRange.GetPageSize();
}

bool CPageMappingHeap::IsInAddressRange(uk ptr) const
{
	return m_addrRange.IsInRange(ptr);
}

size_t CPageMappingHeap::FindLargestFreeBlockSize() const
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);

	const size_t pageSize = m_addrRange.GetPageSize();

	FindLargest findLargest;
	FindZeroRanges(&m_pageBitmap[0], m_pageBitmap.size(), findLargest);

	return findLargest.largest * pageSize;
}

uk CPageMappingHeap::Map(size_t length)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);

	const size_t pageBitmapElemBitSize = (sizeof(u32) * 8);
	const size_t pageSize = m_addrRange.GetPageSize();
	const size_t numPages = m_addrRange.GetPageCount();

	if (length % pageSize)
	{
		__debugbreak();
		length = (length + (pageSize - 1)) & ~(pageSize - 1);
	}

	DLMMapFindBest findBest(length / pageSize);
	FindZeroRanges(&m_pageBitmap[0], m_pageBitmap.size(), findBest);

	if ((findBest.bestPosition == -1) || (findBest.bestPosition >= (i32)numPages))
	{
		return NULL;
	}

	uk mapAddress = m_addrRange.GetBaseAddress() + pageSize * findBest.bestPosition;

	for (size_t pageIdx = findBest.bestPosition, pageIdxEnd = pageIdx + length / pageSize; pageIdx != pageIdxEnd; ++pageIdx)
	{
		if (!m_addrRange.MapPage(pageIdx))
		{
			// Unwind the pages we've already mapped.
			for (; pageIdx > static_cast<size_t>(findBest.bestPosition); --pageIdx)
				m_addrRange.UnmapPage(pageIdx - 1);

			return NULL;
		}
	}

	for (size_t pageIdx = findBest.bestPosition, pageIdxEnd = pageIdx + length / pageSize; pageIdx != pageIdxEnd; ++pageIdx)
	{
		size_t pageSegment = pageIdx / pageBitmapElemBitSize;
		u32 pageMask = 1U << static_cast<u32>(pageIdx % pageBitmapElemBitSize);

		m_pageBitmap[pageSegment] |= pageMask;
	}

	return mapAddress;
}

void CPageMappingHeap::Unmap(uk mem, size_t length)
{
	DrxAutoLock<DrxCriticalSectionNonRecursive> lock(m_lock);
	const size_t pageSize = m_addrRange.GetPageSize();

	if (length % pageSize)
	{
		__debugbreak();
		length = (length + (pageSize - 1)) & ~(pageSize - 1);
	}

	tuk mapAddress = reinterpret_cast<tuk>(mem);
	for (size_t pageIdx = (mapAddress - m_addrRange.GetBaseAddress()) / pageSize, pageIdxEnd = pageIdx + length / pageSize; pageIdx != pageIdxEnd; ++pageIdx)
	{
		m_addrRange.UnmapPage(pageIdx);

		const size_t pageBitmapElemBitSize = (sizeof(u32) * 8);

		size_t pageSegment = pageIdx / pageBitmapElemBitSize;
		u32 pageMask = ~(1U << static_cast<u32>(pageIdx % pageBitmapElemBitSize));

		m_pageBitmap[pageSegment] &= pageMask;
	}
}

void CPageMappingHeap::Init()
{
	UINT_PTR start = (UINT_PTR)m_addrRange.GetBaseAddress();
	UINT_PTR end = start + m_addrRange.GetPageCount() * m_addrRange.GetPageSize();

	size_t addressSpace = end - start;
	size_t pageSize = m_addrRange.GetPageSize();
	size_t numPages = (addressSpace + pageSize - 1) / pageSize;
	m_pageBitmap.resize((numPages + 31) / 32);

	size_t pageCapacity = m_pageBitmap.size() * 32;
	size_t numUnavailablePages = pageCapacity - numPages;
	if (numUnavailablePages > 0)
		m_pageBitmap.back() = ~((1 << (32 - numUnavailablePages)) - 1);
}
