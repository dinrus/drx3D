#ifndef DRX3D_GEN_RANDOM_H
#define DRX3D_GEN_RANDOM_H

#ifdef MT19937

#include <limits.h>
#include <mt19937.h>

#define GEN_RAND_MAX UINT_MAX

SIMD_FORCE_INLINE void GEN_srand(u32 seed) { init_genrand(seed); }
SIMD_FORCE_INLINE u32 GEN_rand() { return genrand_int32(); }

#else

#include <stdlib.h>

#define GEN_RAND_MAX RAND_MAX

SIMD_FORCE_INLINE void GEN_srand(u32 seed) { srand(seed); }
SIMD_FORCE_INLINE u32 GEN_rand() { return rand(); }

#endif

#endif  //DRX3D_GEN_RANDOM_H
