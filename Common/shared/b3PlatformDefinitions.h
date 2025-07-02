#ifndef D3_PLATFORM_DEFINITIONS_H
#define D3_PLATFORM_DEFINITIONS_H

#include <drxtypes.h>

struct MyTest
{
	i32 bla;
};

#ifdef __cplusplus
//#define b3ConstArray(a) const b3AlignedObjectArray<a>&
#define b3ConstArray(a) const a *
#define b3AtomicInc(a) ((*a)++)

inline i32 b3AtomicAdd( i32 *p, i32 val)
{
	i32 oldValue = *p;
	i32 newValue = oldValue + val;
	*p = newValue;
	return oldValue;
}

#define __global

#define D3_STATIC static
#else
//keep D3_LARGE_FLOAT*D3_LARGE_FLOAT < FLT_MAX
#define D3_LARGE_FLOAT 1e18f
#define D3_INFINITY 1e18f
#define drx3DAssert(a)
#define b3ConstArray(a) __global const a *
#define b3AtomicInc atomic_inc
#define b3AtomicAdd atomic_add
#define b3Fabs fabs
#define b3Sqrt native_sqrt
#define b3Sin native_sin
#define b3Cos native_cos

#define D3_STATIC
#endif

#endif
