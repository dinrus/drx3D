// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleFixedSizeElementPool.cpp
//  Version:     v1.00
//  Created:     15/03/2010 by Corey.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Нефрагментирующий разместитель из пула фиксированного размера,
//               размещающий только элементы одинакового размера.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ParticleFixedSizeElementPool.h>
#include <drx3D/Eng3D/ParticleUtils.h>

#ifndef _RELEASE //Debugging code for specific issue CE-10725
 i32 g_allocCounter = 0;
#endif

///////////////////////////////////////////////////////////////////////////////
ParticleObjectPool::ParticleObjectPool() :
	m_pPoolMemory(NULL),
	m_nPoolCapacity(0)
{
	DrxInitializeSListHead(m_freeList4KB);
	DrxInitializeSListHead(m_freeList128Bytes);
	DrxInitializeSListHead(m_freeList256Bytes);
	DrxInitializeSListHead(m_freeList512Bytes);

#if defined(TRACK_PARTICLE_POOL_USAGE)
	m_nUsedMemory = 0;
	m_nMaxUsedMemory = 0;
	m_nMemory128Bytes = 0;
	m_nMemory256Bytes = 0;
	m_nMemory512Bytes = 0;
	m_nMemory128BytesUsed = 0;
	m_nMemory256BytesUsed = 0;
	m_nMemory512Bytesused = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////
ParticleObjectPool::~ParticleObjectPool()
{
	DrxInterlockedFlushSList(m_freeList4KB);
	DrxInterlockedFlushSList(m_freeList128Bytes);
	DrxInterlockedFlushSList(m_freeList256Bytes);
	DrxInterlockedFlushSList(m_freeList512Bytes);

	DrxModuleMemalignFree(m_pPoolMemory);
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Init(u32 nBytesToAllocate)
{
	m_nPoolCapacity = Align(nBytesToAllocate, 4 * 1024);

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Particles Pool");

	// allocate memory block to use as pool
	if (m_pPoolMemory)
	{
		DrxModuleMemalignFree(m_pPoolMemory);
		m_pPoolMemory = NULL;
	}
	if (m_nPoolCapacity)
	{
		m_pPoolMemory = DrxModuleMemalign(m_nPoolCapacity, 128);
	}

	// fill 4KB freelist
	Init4KBFreeList();
}

///////////////////////////////////////////////////////////////////////////////
uk ParticleObjectPool::Allocate(u32 nSize)
{
	uk pRet = NULL;

	if (nSize <= 128)
		pRet = Allocate_128Byte();
	else if (nSize <= 256)
		pRet = Allocate_256Byte();
	else if (nSize <= 512)
		pRet = Allocate_512Byte();
	else // fallback for the case that a a particle histy is bigger than 512 bytes
		pRet = malloc(nSize);

	return pRet;
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Deallocate(uk a_memoryToDeallocate, u32 nSize)
{
	if (nSize <= 128)
		Deallocate_128Byte(a_memoryToDeallocate);
	else if (nSize <= 256)
		Deallocate_256Byte(a_memoryToDeallocate);
	else if (nSize <= 512)
		Deallocate_512Byte(a_memoryToDeallocate);
	else
		free(a_memoryToDeallocate);
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Init4KBFreeList()
{
	DrxInterlockedFlushSList(m_freeList4KB);
	DrxInterlockedFlushSList(m_freeList128Bytes);
	DrxInterlockedFlushSList(m_freeList256Bytes);
	DrxInterlockedFlushSList(m_freeList512Bytes);

	tuk pElementMemory = (tuk)m_pPoolMemory;
	for (i32 i = (m_nPoolCapacity / (4 * 1024)) - 1; i >= 0; --i)
	{
		// Build up the linked list of free nodes to begin with - do it in
		// reverse order so that consecutive allocations will be given chunks
		// in a cache-friendly order
		SLockFreeSingleLinkedListEntry* pNewNode = (SLockFreeSingleLinkedListEntry*)(pElementMemory + (i * (4 * 1024)));
		DrxInterlockedPushEntrySList(m_freeList4KB, *pNewNode);
	}
}

///////////////////////////////////////////////////////////////////////////////
uk ParticleObjectPool::Allocate_128Byte()
{
	u32k nAllocationSize = 128;
	do
	{
		uk pListEntry = DrxInterlockedPopEntrySList(m_freeList128Bytes);
		if (pListEntry)
		{
#if defined(TRACK_PARTICLE_POOL_USAGE)
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, 1);
#endif
			DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory128BytesUsed), nAllocationSize);
			i32k nUsedMemory = DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), nAllocationSize);
			i32 nMaxUsed = ~0;
			do
			{
				nMaxUsed = *alias_cast< i32*>(&m_nMaxUsedMemory);
				if (nUsedMemory <= nMaxUsed)
					break;

			}
			while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nMaxUsedMemory), nUsedMemory, nMaxUsed) == nMaxUsed);
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, -1);
#endif
#endif
			return pListEntry;
		}

		// allocation failed, refill list from a 4KB block if we have one
		uk pMemoryBlock = DrxInterlockedPopEntrySList(m_freeList4KB);

		// stop if we ran out of pool memory
		IF (pMemoryBlock == NULL, 0)
			return NULL;

#if defined(TRACK_PARTICLE_POOL_USAGE)
		DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory128Bytes), 4 * 1024);
#endif

		tuk pElementMemory = (tuk)pMemoryBlock;
		for (i32 i = ((4 * 1024) / (nAllocationSize)) - 1; i >= 0; --i)
		{
			// Build up the linked list of free nodes to begin with - do it in
			// reverse order so that consecutive allocations will be given chunks
			// in a cache-friendly order
			SLockFreeSingleLinkedListEntry* pNewNode = (SLockFreeSingleLinkedListEntry*)(pElementMemory + (i * (nAllocationSize)));
			DrxInterlockedPushEntrySList(m_freeList128Bytes, *pNewNode);
		}

	}
	while (true);
}

///////////////////////////////////////////////////////////////////////////////
uk ParticleObjectPool::Allocate_256Byte()
{
	u32k nAllocationSize = 256;
	do
	{
		uk pListEntry = DrxInterlockedPopEntrySList(m_freeList256Bytes);
		if (pListEntry)
		{
#if defined(TRACK_PARTICLE_POOL_USAGE)			
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, 1);
#endif
			DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory256BytesUsed), nAllocationSize);
			i32k nUsedMemory = DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), nAllocationSize);
			i32 nMaxUsed = ~0;
			do
			{
				nMaxUsed = *alias_cast< i32*>(&m_nMaxUsedMemory);
				if (nUsedMemory <= nMaxUsed)
					break;

			}
			while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nMaxUsedMemory), nUsedMemory, nMaxUsed) == nMaxUsed);
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, -1);
#endif
#endif
			return pListEntry;
		}

		// allocation failed, refill list from a 4KB block if we have one
		uk pMemoryBlock = DrxInterlockedPopEntrySList(m_freeList4KB);
		// stop if we ran out of pool memory
		IF (pMemoryBlock == NULL, 0)
			return NULL;

#if defined(TRACK_PARTICLE_POOL_USAGE)
		DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory256Bytes), 4 * 1024);
#endif

		tuk pElementMemory = (tuk)pMemoryBlock;
		for (i32 i = ((4 * 1024) / (nAllocationSize)) - 1; i >= 0; --i)
		{
			// Build up the linked list of free nodes to begin with - do it in
			// reverse order so that consecutive allocations will be given chunks
			// in a cache-friendly order
			SLockFreeSingleLinkedListEntry* pNewNode = (SLockFreeSingleLinkedListEntry*)(pElementMemory + (i * (nAllocationSize)));
			DrxInterlockedPushEntrySList(m_freeList256Bytes, *pNewNode);
		}

	}
	while (true);
}

///////////////////////////////////////////////////////////////////////////////
uk ParticleObjectPool::Allocate_512Byte()
{
	u32k nAllocationSize = 512;
	do
	{
		uk pListEntry = DrxInterlockedPopEntrySList(m_freeList512Bytes);
		if (pListEntry)
		{
#if defined(TRACK_PARTICLE_POOL_USAGE)
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, 1);
#endif
			DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory512Bytesused), nAllocationSize);
			i32k nUsedMemory = DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), nAllocationSize);
			i32 nMaxUsed = ~0;
			do
			{
				nMaxUsed = *alias_cast< i32*>(&m_nMaxUsedMemory);
				if (nUsedMemory <= nMaxUsed)
					break;

			}
			while (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nMaxUsedMemory), nUsedMemory, nMaxUsed) == nMaxUsed);
#ifndef _RELEASE //Debugging code for specific issue CE-10725			
			DrxInterlockedAdd(&g_allocCounter, -1);
#endif
#endif
			return pListEntry;
		}

		// allocation failed, refill list from a 4KB block if we have one
		uk pMemoryBlock = DrxInterlockedPopEntrySList(m_freeList4KB);
		// stop if we ran out of pool memory
		IF (pMemoryBlock == NULL, 0)
			return NULL;

#if defined(TRACK_PARTICLE_POOL_USAGE)
		DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory512Bytes), 4 * 1024);
#endif

		tuk pElementMemory = (tuk)pMemoryBlock;
		for (i32 i = ((4 * 1024) / (nAllocationSize)) - 1; i >= 0; --i)
		{
			// Build up the linked list of free nodes to begin with - do it in
			// reverse order so that consecutive allocations will be given chunks
			// in a cache-friendly order
			SLockFreeSingleLinkedListEntry* pNewNode = (SLockFreeSingleLinkedListEntry*)(pElementMemory + (i * (nAllocationSize)));
			DrxInterlockedPushEntrySList(m_freeList512Bytes, *pNewNode);
		}

	}
	while (true);
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Deallocate_128Byte(uk pPtr)
{
	IF (!pPtr, 0)
		return;

	DrxInterlockedPushEntrySList(m_freeList128Bytes, *alias_cast<SLockFreeSingleLinkedListEntry*>(pPtr));
#if defined(TRACK_PARTICLE_POOL_USAGE)
	DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), -128);
	DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory128BytesUsed), -128);
#endif
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Deallocate_256Byte(uk pPtr)
{
	IF (!pPtr, 0)
		return;

	DrxInterlockedPushEntrySList(m_freeList256Bytes, *alias_cast<SLockFreeSingleLinkedListEntry*>(pPtr));
#if defined(TRACK_PARTICLE_POOL_USAGE)
	DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), -256);
	DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory256BytesUsed), -256);
#endif
}

///////////////////////////////////////////////////////////////////////////////
void ParticleObjectPool::Deallocate_512Byte(uk pPtr)
{
	IF (!pPtr, 0)
		return;

	DrxInterlockedPushEntrySList(m_freeList512Bytes, *alias_cast<SLockFreeSingleLinkedListEntry*>(pPtr));
#if defined(TRACK_PARTICLE_POOL_USAGE)
	DrxInterlockedAdd(alias_cast< i32*>(&m_nUsedMemory), -512);
	DrxInterlockedAdd(alias_cast< i32*>(&m_nMemory512Bytesused), -512);
#endif
}
