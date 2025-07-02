// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleFixedSizeElementPool.h
//  Version:     v1.00
//  Created:     15/03/2010 by Corey.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: A fragmentation-free allocator from a fixed-size pool and which
//							 only allocates elements of the same size.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlefixedsizeelementpool_h__
#define __particlefixedsizeelementpool_h__
#pragma once

#if !defined(_RELEASE)
	#define TRACK_PARTICLE_POOL_USAGE
#endif

///////////////////////////////////////////////////////////////////////////////
// thread safe pool for faster allocation/deallocation of particle container and alike objects
// allocate/deallocate are only a single atomic operations on a freelist
class ParticleObjectPool
{
public:
	ParticleObjectPool();
	~ParticleObjectPool();

	void                  Init(u32 nBytesToAllocate);

	uk                 Allocate(u32 nSize);
	void                  Deallocate(uk a_memoryToDeallocate, u32 nSize);

	stl::SPoolMemoryUsage GetTotalMemory() const;
	void                  GetMemoryUsage(IDrxSizer* pSizer);

	void                  ResetUsage();
private:
	void                  Init4KBFreeList();

	uk                 Allocate_128Byte();
	uk                 Allocate_256Byte();
	uk                 Allocate_512Byte();

	void                  Deallocate_128Byte(uk pPtr);
	void                  Deallocate_256Byte(uk pPtr);
	void                  Deallocate_512Byte(uk pPtr);

	SLockFreeSingleLinkedListHeader m_freeList4KB;  // master freelist of 4KB blocks they are splitted into the sub lists (merged back during ResetUsage)
	SLockFreeSingleLinkedListHeader m_freeList128Bytes;
	SLockFreeSingleLinkedListHeader m_freeList256Bytes;
	SLockFreeSingleLinkedListHeader m_freeList512Bytes;

	uk                           m_pPoolMemory;
	u32                          m_nPoolCapacity;

#if defined(TRACK_PARTICLE_POOL_USAGE)
	i32 m_nUsedMemory;
	i32 m_nMaxUsedMemory;
	i32 m_nMemory128Bytes;
	i32 m_nMemory256Bytes;
	i32 m_nMemory512Bytes;
	i32 m_nMemory128BytesUsed;
	i32 m_nMemory256BytesUsed;
	i32 m_nMemory512Bytesused;
#endif
};

///////////////////////////////////////////////////////////////////////////////
inline stl::SPoolMemoryUsage ParticleObjectPool::GetTotalMemory() const
{
#if defined(TRACK_PARTICLE_POOL_USAGE)
	return stl::SPoolMemoryUsage(
	  (size_t)m_nPoolCapacity,
	  (size_t)m_nMaxUsedMemory,
	  (size_t)m_nUsedMemory);
#else
	return stl::SPoolMemoryUsage(0, 0, 0);
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline void ParticleObjectPool::GetMemoryUsage(IDrxSizer* pSizer)
{
	pSizer->AddObject(m_pPoolMemory, m_nPoolCapacity);
}

///////////////////////////////////////////////////////////////////////////////
inline void ParticleObjectPool::ResetUsage()
{
	// merge all allocations back into 4KB
	Init4KBFreeList();
#if defined(TRACK_PARTICLE_POOL_USAGE)
	m_nUsedMemory = 0;
	m_nMaxUsedMemory = 0;
#endif
}

#endif // __fixedsizeelementpool_h__
