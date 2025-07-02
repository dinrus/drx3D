// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <memory>

#include <drx3D/Animation/IAllocator.h>
#include <drx3D/Animation/PolymorphicAllocator.h>

namespace Memory {
class CPool;
}

namespace Memory
{

/**
 * An IAllocator wrapper for the CPool class.
 */
class PoolAllocator : public IAllocator
{
public:

	/**
	 * Constructs a new PoolAllocator object wrapped with the PolymorphicAllocator class.
	 * @param pool Reference to a Memory::CPool instance to wrap within this allocator.
	 */
	static PolymorphicAllocator Create(CPool& pool);

	/*
	 * Constructs a new PoolAllocator object.
	 * @param pool Reference to a Memory::CPool instance to wrap within this allocator.
	 */
	PoolAllocator(CPool& pool);

private:

	// IAllocator
	virtual uk                       Allocate(u32 length) override;
	virtual void                        Free(uk pAddress) override;
	virtual size_t                      GetGuaranteedAlignment() const override;
	virtual std::unique_ptr<IAllocator> Clone() const override;

	CPool* m_pPool;
};

} //endns Memory

//////////////////////////////////////////////////////////////////////////

namespace Memory
{

inline PolymorphicAllocator PoolAllocator::Create(CPool& pool)
{
	return PolymorphicAllocator(std::unique_ptr<PoolAllocator>(new PoolAllocator(pool)));
}

inline PoolAllocator::PoolAllocator(CPool& pool)
	: m_pPool(&pool)
{}

} //endns Memory
