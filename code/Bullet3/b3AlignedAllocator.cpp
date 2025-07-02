#include <drx3D/Common/b3AlignedAllocator.h>

#ifdef D3_ALLOCATOR_STATISTICS
i32 b3g_numAlignedAllocs = 0;
i32 b3g_numAlignedFree = 0;
i32 b3g_totalBytesAlignedAllocs = 0;  //detect memory leaks
#endif

static uk b3AllocDefault(size_t size)
{
	return malloc(size);
}

static void b3FreeDefault(uk ptr)
{
	free(ptr);
}

static b3AllocFunc *b3s_allocFunc = b3AllocDefault;
static b3FreeFunc *b3s_freeFunc = b3FreeDefault;

#if defined(D3_HAS_ALIGNED_ALLOCATOR)
#include <malloc.h>
static uk b3AlignedAllocDefault(size_t size, i32 alignment)
{
	return _aligned_malloc(size, (size_t)alignment);
}

static void b3AlignedFreeDefault(uk ptr)
{
	_aligned_free(ptr);
}
#elif defined(__CELLOS_LV2__)
#include <stdlib.h>

static inline uk b3AlignedAllocDefault(size_t size, i32 alignment)
{
	return memalign(alignment, size);
}

static inline void b3AlignedFreeDefault(uk ptr)
{
	free(ptr);
}
#else

static inline uk b3AlignedAllocDefault(size_t size, i32 alignment)
{
	uk ret;
	char *real;
	real = (char *)b3s_allocFunc(size + sizeof(uk ) + (alignment - 1));
	if (real)
	{
		ret = b3AlignPointer(real + sizeof(uk ), alignment);
		*((uk *)(ret)-1) = (uk )(real);
	}
	else
	{
		ret = (uk )(real);
	}
	return (ret);
}

static inline void b3AlignedFreeDefault(uk ptr)
{
	uk real;

	if (ptr)
	{
		real = *((uk *)(ptr)-1);
		b3s_freeFunc(real);
	}
}
#endif

static b3AlignedAllocFunc *b3s_alignedAllocFunc = b3AlignedAllocDefault;
static b3AlignedFreeFunc *b3s_alignedFreeFunc = b3AlignedFreeDefault;

void b3AlignedAllocSetCustomAligned(b3AlignedAllocFunc *allocFunc, b3AlignedFreeFunc *freeFunc)
{
	b3s_alignedAllocFunc = allocFunc ? allocFunc : b3AlignedAllocDefault;
	b3s_alignedFreeFunc = freeFunc ? freeFunc : b3AlignedFreeDefault;
}

void b3AlignedAllocSetCustom(b3AllocFunc *allocFunc, b3FreeFunc *freeFunc)
{
	b3s_allocFunc = allocFunc ? allocFunc : b3AllocDefault;
	b3s_freeFunc = freeFunc ? freeFunc : b3FreeDefault;
}

#ifdef D3_DEBUG_MEMORY_ALLOCATIONS
//this generic allocator provides the total allocated number of bytes
#include <stdio.h>

uk b3AlignedAllocInternal(size_t size, i32 alignment, i32 line, char *filename)
{
	uk ret;
	char *real;
#ifdef D3_ALLOCATOR_STATISTICS
	b3g_totalBytesAlignedAllocs += size;
	b3g_numAlignedAllocs++;
#endif
	real = (char *)b3s_allocFunc(size + 2 * sizeof(uk ) + (alignment - 1));
	if (real)
	{
		ret = (uk )b3AlignPointer(real + 2 * sizeof(uk ), alignment);
		*((uk *)(ret)-1) = (uk )(real);
		*((i32 *)(ret)-2) = size;
	}
	else
	{
		ret = (uk )(real);  //??
	}

	drx3DPrintf("allocation#%d at address %x, from %s,line %d, size %d\n", b3g_numAlignedAllocs, real, filename, line, size);

	i32 *ptr = (i32 *)ret;
	*ptr = 12;
	return (ret);
}

void b3AlignedFreeInternal(uk ptr, i32 line, char *filename)
{
	uk real;
#ifdef D3_ALLOCATOR_STATISTICS
	b3g_numAlignedFree++;
#endif
	if (ptr)
	{
		real = *((uk *)(ptr)-1);
		i32 size = *((i32 *)(ptr)-2);
#ifdef D3_ALLOCATOR_STATISTICS
		b3g_totalBytesAlignedAllocs -= size;
#endif
		drx3DPrintf("free #%d at address %x, from %s,line %d, size %d\n", b3g_numAlignedFree, real, filename, line, size);

		b3s_freeFunc(real);
	}
	else
	{
		drx3DPrintf("NULL ptr\n");
	}
}

#else  //D3_DEBUG_MEMORY_ALLOCATIONS

uk b3AlignedAllocInternal(size_t size, i32 alignment)
{
#ifdef D3_ALLOCATOR_STATISTICS
	b3g_numAlignedAllocs++;
#endif
	uk ptr;
	ptr = b3s_alignedAllocFunc(size, alignment);
	//	drx3DPrintf("b3AlignedAllocInternal %d, %x\n",size,ptr);
	return ptr;
}

void b3AlignedFreeInternal(uk ptr)
{
	if (!ptr)
	{
		return;
	}
#ifdef D3_ALLOCATOR_STATISTICS
	b3g_numAlignedFree++;
#endif
	//	drx3DPrintf("b3AlignedFreeInternal %x\n",ptr);
	b3s_alignedFreeFunc(ptr);
}

#endif  //D3_DEBUG_MEMORY_ALLOCATIONS
