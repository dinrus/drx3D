// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef DRX_DLMALLOC_H
#define DRX_DLMALLOC_H
#pragma once

typedef uk dlmspace;

#ifdef __cplusplus
extern "C"
{
#endif

typedef uk (* dlmmap_handler)(uk user, size_t sz);
typedef i32 (*   dlmunmap_handler)(uk user, uk mem, size_t sz);
static uk const dlmmap_error = (uk )(INT_PTR)-1;

i32      dlmspace_create_overhead(void);
dlmspace dlcreate_mspace(size_t capacity, i32 locked, uk user = NULL, dlmmap_handler mmap = NULL, dlmunmap_handler munmap = NULL);
size_t   dldestroy_mspace(dlmspace msp);
dlmspace dlcreate_mspace_with_base(uk base, size_t capacity, i32 locked);
i32      dlmspace_track_large_chunks(dlmspace msp, i32 enable);
uk    dlmspace_malloc(dlmspace msp, size_t bytes);
void     dlmspace_free(dlmspace msp, uk mem);
uk    dlmspace_realloc(dlmspace msp, uk mem, size_t newsize);
uk    dlmspace_calloc(dlmspace msp, size_t n_elements, size_t elem_size);
uk    dlmspace_memalign(dlmspace msp, size_t alignment, size_t bytes);
uk *   dlmspace_independent_calloc(dlmspace msp, size_t n_elements, size_t elem_size, uk chunks[]);
uk *   dlmspace_independent_comalloc(dlmspace msp, size_t n_elements, size_t sizes[], uk chunks[]);
size_t   dlmspace_footprint(dlmspace msp);
size_t   dlmspace_max_footprint(dlmspace msp);
size_t   dlmspace_usable_size(uk mem);
void     dlmspace_malloc_stats(dlmspace msp);
size_t   dlmspace_get_used_space(dlmspace msp);
i32      dlmspace_trim(dlmspace msp, size_t pad);
i32      dlmspace_mallopt(i32, i32);

#ifdef __cplusplus
}
#endif

#endif
