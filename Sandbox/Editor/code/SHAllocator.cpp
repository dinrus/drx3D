// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include <PRT/SHAllocator.h>

#if defined(USE_MEM_ALLOCATOR)

static uk SHMalloc(size_t Size)
{
	return malloc(Size);
}

static void SHFreeSize(uk pPtr, size_t Size)
{
	return free(pPtr);
}

void LoadAllocatorModule(FNC_SHMalloc& pfnMalloc, FNC_SHFreeSize& pfnFree)
{
	pfnMalloc = &SHMalloc;
	pfnFree = &SHFreeSize;
}

CSHAllocator<u8> gsByteAllocator;

#endif

