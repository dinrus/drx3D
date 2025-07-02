// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/PolymorphicAllocator.h>

#include <algorithm>
#include <utility>

#include <drx3D/Animation/IAllocator.h>

namespace Memory
{

uk PolymorphicAllocator::Allocate(u32 length)
{
	return m_pAllocator->Allocate(length);
}

void PolymorphicAllocator::Free(uk pAddress)
{
	return m_pAllocator->Free(pAddress);
}

size_t PolymorphicAllocator::GetGuaranteedAlignment() const
{
	return m_pAllocator->GetGuaranteedAlignment();
}

PolymorphicAllocator::PolymorphicAllocator(const PolymorphicAllocator& other)
	: m_pAllocator(other.m_pAllocator->Clone())
{
}

PolymorphicAllocator& PolymorphicAllocator::operator=(const PolymorphicAllocator& other)
{
	m_pAllocator = other.m_pAllocator->Clone();
	return *this;
}

void PolymorphicAllocator::Swap(PolymorphicAllocator& other)
{
	std::swap(m_pAllocator, other.m_pAllocator);
}

} //endns Memory
