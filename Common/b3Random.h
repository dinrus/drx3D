#ifndef D3_GEN_RANDOM_H
#define D3_GEN_RANDOM_H

#include "b3Scalar.h>

#ifdef MT19937

#include <limits.h>
#include <mt19937.h>

#define D3_RAND_MAX UINT_MAX

D3_FORCE_INLINE void b3Srand(u32 seed) { init_genrand(seed); }
D3_FORCE_INLINE u32 b3rand() { return genrand_int32(); }

#else

#include <stdlib.h>

#define D3_RAND_MAX RAND_MAX

D3_FORCE_INLINE void b3Srand(u32 seed) { srand(seed); }
D3_FORCE_INLINE u32 b3rand() { return rand(); }

#endif

inline b3Scalar b3RandRange(b3Scalar minRange, b3Scalar maxRange)
{
	return (b3rand() / (b3Scalar(D3_RAND_MAX) + b3Scalar(1.0))) * (maxRange - minRange) + minRange;
}

#endif  //D3_GEN_RANDOM_H
