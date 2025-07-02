// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/PoolAllocator.h>

#include <drx3D/Animation/Pool.h>

namespace Memory
{

uk PoolAllocator::Allocate(u32 length)
{
	assert(m_pPool);
	return m_pPool->Allocate(length);
}

void PoolAllocator::Free(uk pAddress)
{
	assert(m_pPool);
	m_pPool->Free(pAddress);
}

size_t PoolAllocator::GetGuaranteedAlignment() const
{
	assert(m_pPool);
	return m_pPool->GetGuaranteedAlignment();
}

std::unique_ptr<IAllocator> PoolAllocator::Clone() const
{
	return std::unique_ptr<IAllocator>(new PoolAllocator(*this));
}

} //endns Memory
