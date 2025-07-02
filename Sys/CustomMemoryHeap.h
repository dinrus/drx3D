// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/IMemory.h>

class CCustomMemoryHeap;

//////////////////////////////////////////////////////////////////////////
class CCustomMemoryHeapBlock final : public ICustomMemoryBlock
{
public:
	CCustomMemoryHeapBlock(CCustomMemoryHeap* pHeap, uk pData, size_t const allocateSize, char const* const szUsage);
	virtual ~CCustomMemoryHeapBlock();

	//////////////////////////////////////////////////////////////////////////
	// ICustomMemoryBlock implementation
	virtual uk GetData() override;
	virtual i32   GetSize() override { return static_cast<i32>(m_size); }
	virtual void  CopyMemoryRegion(uk pOutputBuffer, size_t offset, size_t size) override;
	//////////////////////////////////////////////////////////////////////////

private:
	CCustomMemoryHeap* m_pHeap;
	uk              m_pData;
	const size_t       m_size;
#if !defined(_RELEASE)
	const string       m_usage;
#endif
};

//////////////////////////////////////////////////////////////////////////
class CCustomMemoryHeap final : public ICustomMemoryHeap
{
public:
	explicit CCustomMemoryHeap(IMemoryUpr::EAllocPolicy const allocPolicy);

	//////////////////////////////////////////////////////////////////////////
	// ICustomMemoryHeap implementation
	virtual ICustomMemoryBlock* AllocateBlock(size_t const allocateSize, char const* const szUsage, size_t const alignment = 16) override;
	virtual void                GetMemoryUsage(IDrxSizer* pSizer) override;
	virtual size_t              GetAllocated() override;
	//////////////////////////////////////////////////////////////////////////

	void DeallocateBlock(CCustomMemoryHeapBlock* pBlock);

private:
	i32 m_allocatedSize;
	IMemoryUpr::EAllocPolicy const m_allocPolicy;
};
