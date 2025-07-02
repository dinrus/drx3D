#ifndef DRX3D_GEN_MINMAX_H
#define DRX3D_GEN_MINMAX_H

#include <drx3D/Maths/Linear/Scalar.h>

template <class T>
SIMD_FORCE_INLINE const T& d3Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template <class T>
SIMD_FORCE_INLINE const T& d3Max(const T& a, const T& b)
{
	return a > b ? a : b;
}

template <class T>
SIMD_FORCE_INLINE const T& Clamped(const T& a, const T& lb, const T& ub)
{
	return a < lb ? lb : (ub < a ? ub : a);
}

template <class T>
SIMD_FORCE_INLINE void SetMin(T& a, const T& b)
{
	if (b < a)
	{
		a = b;
	}
}

template <class T>
SIMD_FORCE_INLINE void SetMax(T& a, const T& b)
{
	if (a < b)
	{
		a = b;
	}
}

template <class T>
SIMD_FORCE_INLINE void Clamp(T& a, const T& lb, const T& ub)
{
	if (a < lb)
	{
		a = lb;
	}
	else if (ub < a)
	{
		a = ub;
	}
}

#endif  //DRX3D_GEN_MINMAX_H
