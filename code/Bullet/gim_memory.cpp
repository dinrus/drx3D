#include <drx3D/Physics/Collision/Gimpact/gim_memory.h>
#include <stdlib.h>

#ifdef GIM_SIMD_MEMORY
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#endif

static gim_alloc_function *g_allocfn = 0;
static gim_alloca_function *g_allocafn = 0;
static gim_realloc_function *g_reallocfn = 0;
static gim_free_function *g_freefn = 0;

void gim_set_alloc_handler(gim_alloc_function *fn)
{
	g_allocfn = fn;
}

void gim_set_alloca_handler(gim_alloca_function *fn)
{
	g_allocafn = fn;
}

void gim_set_realloc_handler(gim_realloc_function *fn)
{
	g_reallocfn = fn;
}

void gim_set_free_handler(gim_free_function *fn)
{
	g_freefn = fn;
}

gim_alloc_function *gim_get_alloc_handler()
{
	return g_allocfn;
}

gim_alloca_function *gim_get_alloca_handler()
{
	return g_allocafn;
}

gim_realloc_function *gim_get_realloc_handler()
{
	return g_reallocfn;
}

gim_free_function *gim_get_free_handler()
{
	return g_freefn;
}

uk gim_alloc(size_t size)
{
	uk ptr;
	if (g_allocfn)
	{
		ptr = g_allocfn(size);
	}
	else
	{
#ifdef GIM_SIMD_MEMORY
		ptr = AlignedAlloc(size, 16);
#else
		ptr = malloc(size);
#endif
	}
	return ptr;
}

uk gim_alloca(size_t size)
{
	if (g_allocafn)
		return g_allocafn(size);
	else
		return gim_alloc(size);
}

uk gim_realloc(uk ptr, size_t oldsize, size_t newsize)
{
	uk newptr = gim_alloc(newsize);
	size_t copysize = oldsize < newsize ? oldsize : newsize;
	gim_simd_memcpy(newptr, ptr, copysize);
	gim_free(ptr);
	return newptr;
}

void gim_free(uk ptr)
{
	if (!ptr) return;
	if (g_freefn)
	{
		g_freefn(ptr);
	}
	else
	{
#ifdef GIM_SIMD_MEMORY
		AlignedFree(ptr);
#else
		free(ptr);
#endif
	}
}
