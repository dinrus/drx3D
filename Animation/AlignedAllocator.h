// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <memory>
#include <drx3D/CoreX/Memory/STLAlignedAlloc.h>

#include <drx3D/Animation/IAllocator.h>
#include <drx3D/Animation/PolymorphicAllocator.h>

namespace Memory
{

/**
 * A simple IAllocator implementation that allocates aligned memory on the heap.
 */
template<size_t IAlignment>
class AlignedAllocator : public IAllocator
{

public:

	/**
	 * Constructs a new AlignedAllocator object wrapped with the PolymorphicAllocator class.
	 */
	static PolymorphicAllocator Create();

private:

	// IAllocator
	virtual uk                       Allocate(u32 length) override;
	virtual void                        Free(uk pAddress) override;
	virtual size_t                      GetGuaranteedAlignment() const override;
	virtual std::unique_ptr<IAllocator> Clone() const override;

};

} //endns Memory

//////////////////////////////////////////////////////////////////////////

namespace Memory
{

template<size_t IAlignment>
inline PolymorphicAllocator AlignedAllocator<IAlignment >::Create()
{
	return PolymorphicAllocator(std::unique_ptr<AlignedAllocator<IAlignment>>(new AlignedAllocator<IAlignment> ));
}

template<size_t IAlignment>
inline uk AlignedAllocator<IAlignment >::Allocate(u32 length)
{
	return (length > 0) ? (stl::aligned_alloc<char, IAlignment>().allocate(length)) : (nullptr);
}

template<size_t IAlignment>
inline void AlignedAllocator<IAlignment >::Free(uk pAddress)
{
	return stl::aligned_alloc<char, IAlignment>().deallocate(static_cast<tuk>(pAddress), 0);
}

template<size_t IAlignment>
inline std::unique_ptr<IAllocator> AlignedAllocator<IAlignment >::Clone() const
{
	return std::unique_ptr<IAllocator>(new AlignedAllocator((*this)));
}

template<size_t IAlignment>
inline size_t AlignedAllocator<IAlignment >::GetGuaranteedAlignment() const
{
	return IAlignment;
}

} //endns Memory
