// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/PoolObject.h>

#if !defined(_RELEASE)
	#define INCLUDE_AUDIO_ALLOCATOR_PRODUCTION_CODE
#endif // _RELEASE

#ifdef INCLUDE_AUDIO_ALLOCATOR_PRODUCTION_CODE
	#include <drx3D/CoreX/Audio/IAudioSystem.h>
	#include <drx3D/Sys/ISystem.h>
#endif // INCLUDE_AUDIO_ALLOCATOR_PRODUCTION_CODE

namespace DrxAudio
{
template<typename T, typename SyncMechanism>
typename CPoolObject<T, SyncMechanism>::Allocator * CPoolObject<T, SyncMechanism>::s_pAllocator = nullptr;

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism>
typename CPoolObject<T, SyncMechanism>::Allocator & CPoolObject<T, SyncMechanism>::GetAllocator() noexcept
{
	//return m_allocator;
	DRX_ASSERT_MESSAGE(s_pAllocator, "Trying to get allocator before calling CreateAllocator()");
	return *s_pAllocator;
}

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism>
void CPoolObject<T, SyncMechanism >::CreateAllocator(u16 const preallocatedObjects)
{
	DRX_ASSERT_MESSAGE(s_pAllocator == nullptr, "Trying to re-create the pool object allocator");
	DRX_ASSERT_MESSAGE(preallocatedObjects > 0, "Trying to create a pool object allocator with zero objects");
	static CPoolObject<T, SyncMechanism>::Allocator allocator(sizeof(T), alignof(T), stl::FHeap().PageSize(preallocatedObjects).FreeWhenEmpty(true));
	s_pAllocator = &allocator;
}

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism>
uk CPoolObject<T, SyncMechanism >::AllocateObjectStorage()
{
	// Poor man's replacement for lack of a proper __attribute__((malloc))/
	// restrict function attribute.
	auto* __restrict const pMemory(GetAllocator().Allocate());
	return pMemory;
}

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism>
uk CPoolObject<T, SyncMechanism >::operator new(std::size_t const count, std::nothrow_t const&) noexcept
{
	DRX_ASSERT_MESSAGE(count % sizeof(T) == 0, "Allocating a partial object!?");
	DRX_ASSERT_MESSAGE(count / sizeof(T) == 1, "Allocating more than one object not supported with pool allocators.");
	auto* __restrict const pMemory(AllocateObjectStorage());
	return pMemory;
}

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism>
uk CPoolObject<T, SyncMechanism >::operator new(std::size_t const count)
{
	auto* __restrict const pMemory(CPoolObject<T, SyncMechanism>::operator new(count, std::nothrow));
	return pMemory;
}

//////////////////////////////////////////////////////////////////////////
template<typename T, typename SyncMechanism> NO_INLINE
void CPoolObject<T, SyncMechanism >::operator delete(uk const pObject) noexcept
{
	GetAllocator().Deallocate(pObject);
}
} //endns DrxAudio
