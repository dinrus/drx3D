// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef _MSC_VER
	#include <drx3D/CoreX/Memory/DrxSizer.h>
#endif // missing include in PoolAllocator.h for non-lazy template parsing compilers
#include <drx3D/CoreX/Memory/PoolAllocator.h>

//////////////////////////////////////////////////////////////////////////
//!
//! \class CPoolObject
//!
//! \brief Base class that provides custom new and delete operators which
//! use a type-dedicated pool allocator.
//!
//! \details The CreateAllocator() static member function offers the
//! ability to lazily create the underlying pool allocator so that it can
//! be, for example, configured according to a CVar.
//!
//! \note This approach works only for Ts that are final (or at least
//! their potential derived classes are of the same size as T), violations
//! of this precondition will be caught at runtime with appropriate
//! assertions.
//!
//////////////////////////////////////////////////////////////////////////

namespace DrxAudio
{
template<typename T, typename SyncMechanism>
class CPoolObject
{
public:
	static uk operator new(std::size_t count, std::nothrow_t const&) noexcept;
	static uk operator new(std::size_t count);
	static void  operator delete(uk pObject) noexcept;

	// No support for array allocations in stl::SizePoolAllocator
	static uk operator new[](std::size_t count, std::nothrow_t const&) = delete;
	static uk operator new[](std::size_t count) = delete;
	static void  operator delete[](uk pObject) = delete;

	//! Type-erased allocator
	using Allocator = stl::SizePoolAllocator<stl::HeapAllocator<SyncMechanism>>;

	static Allocator& GetAllocator() noexcept;

	//! \param preallocatedObjects Use an underlying heap that uses pages
	//!                            that fit <VAR>preallocatedObjects</VAR>
	//!                            number of objects.
	static void CreateAllocator(u16 preallocatedObjects);

	static void FreeMemoryPool()
	{
		s_pAllocator->FreeMemoryForce();
	}

protected:
	CPoolObject() noexcept = default;
	~CPoolObject() noexcept = default;

private:
	static uk AllocateObjectStorage();

	static Allocator* s_pAllocator;
};
} //endns DrxAudio

//! \note MSVC (tested@14u3) does not seem to automatically instantiate the
//! operator delete and new CPoolObject member functions at the point of
//! the specialization of the GetAllocatorStorage() static member function
//! leading to missing-symbol linker error in non-uber builds. For this
//! reason we have to provide the definition of the template here for this
//! compiler.
#if defined(_MSC_VER) || defined(__clang__)
	#include <drx3D/Audio/PoolObject_impl.h>
#endif // _MSC_VER || __clang__
