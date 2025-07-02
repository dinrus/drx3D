// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/CustomMemoryHeap.h>

#if DRX_PLATFORM_DURANGO
	#include <apu.h>
#endif // DRX_PLATFORM_DURANGO

//////////////////////////////////////////////////////////////////////////
CCustomMemoryHeapBlock::CCustomMemoryHeapBlock(CCustomMemoryHeap* pHeap, uk pData, size_t const allocateSize, char const* const szUsage)
	: m_pHeap(pHeap)
	, m_pData(pData)
	, m_size(allocateSize)
#if !defined(_RELEASE)
	, m_usage(szUsage)
#endif
{
}

//////////////////////////////////////////////////////////////////////////
CCustomMemoryHeapBlock::~CCustomMemoryHeapBlock()
{
	m_pHeap->DeallocateBlock(this);
}

//////////////////////////////////////////////////////////////////////////
uk CCustomMemoryHeapBlock::GetData()
{
	return m_pData;
}

//////////////////////////////////////////////////////////////////////////
void CCustomMemoryHeapBlock::CopyMemoryRegion(uk pOutputBuffer, size_t offset, size_t size)
{
	DRX_ASSERT(offset + size <= m_size);
	memcpy(pOutputBuffer, (u8*)m_pData + offset, size);
}

//////////////////////////////////////////////////////////////////////////
ICustomMemoryBlock* CCustomMemoryHeap::AllocateBlock(size_t const allocateSize, char const* const szUsage, size_t const alignment /* = 16 */)
{
	uk pCreatedData;

	switch (m_allocPolicy)
	{
	case IMemoryUpr::eapDefaultAllocator:
		pCreatedData = malloc(allocateSize);
		break;
	case IMemoryUpr::eapPageMapped:
		pCreatedData = DrxGetIMemoryUpr()->AllocPages(allocateSize);
		break;
	case IMemoryUpr::eapCustomAlignment:
		DRX_ASSERT_MESSAGE(alignment != 0, "CCustomMemoryHeap: trying to allocate memory via eapCustomAlignment with an alignment of zero!");
		pCreatedData = DrxModuleMemalign(allocateSize, alignment);
		break;
#if DRX_PLATFORM_DURANGO
	case IMemoryUpr::eapAPU:
		if (ApuAlloc(&pCreatedData, nullptr, Align(allocateSize, alignment), static_cast<UINT32>(alignment)) != S_OK)
		{
			DrxFatalError("CCustomMemoryHeap: failed to allocate APU memory! (%" PRISIZE_T " bytes) (%" PRISIZE_T " alignment)", allocateSize, alignment);
		}
		break;
#endif // DRX_PLATFORM_DURANGO
	default:
		DrxFatalError("CCustomMemoryHeap: unknown allocation policy during AllocateBlock!");
		break;
	}

	DrxInterlockedAdd(&m_allocatedSize, allocateSize);
	return new CCustomMemoryHeapBlock(this, pCreatedData, allocateSize, szUsage);
}

//////////////////////////////////////////////////////////////////////////
void CCustomMemoryHeap::DeallocateBlock(CCustomMemoryHeapBlock* pBlock)
{
	switch (m_allocPolicy)
	{
	case IMemoryUpr::eapDefaultAllocator:
		free(pBlock->GetData());
		break;
	case IMemoryUpr::eapPageMapped:
		DrxGetIMemoryUpr()->FreePages(pBlock->GetData(), pBlock->GetSize());
		break;
	case IMemoryUpr::eapCustomAlignment:
		DrxModuleMemalignFree(pBlock->GetData());
		break;
#if DRX_PLATFORM_DURANGO
	case IMemoryUpr::eapAPU:
		ApuFree(pBlock->GetData());
		break;
#endif // DRX_PLATFORM_DURANGO
	default:
		DrxFatalError("CCustomMemoryHeap: unknown allocation policy during DeallocateBlock!");
		break;
	}

	i32k allocateSize = pBlock->GetSize();
	DrxInterlockedAdd(&m_allocatedSize, -allocateSize);
}

//////////////////////////////////////////////////////////////////////////
void CCustomMemoryHeap::GetMemoryUsage(IDrxSizer* pSizer)
{
	pSizer->AddObject(this, m_allocatedSize);
}

//////////////////////////////////////////////////////////////////////////
size_t CCustomMemoryHeap::GetAllocated()
{
	return static_cast<size_t>(m_allocatedSize);
}

//////////////////////////////////////////////////////////////////////////
CCustomMemoryHeap::CCustomMemoryHeap(IMemoryUpr::EAllocPolicy const allocPolicy)
	: m_allocatedSize(0)
	, m_allocPolicy(allocPolicy)
{
}
