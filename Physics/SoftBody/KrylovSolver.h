#ifndef DRX3D_KRYLOV_SOLVER_H
#define DRX3D_KRYLOV_SOLVER_H
#include <iostream>
#include <cmath>
#include <limits>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/Quickprof.h>

template <class MatrixX>
class KrylovSolver
{
	typedef AlignedObjectArray<Vec3> TVStack;

public:
	i32 m_maxIterations;
	Scalar m_tolerance;
	KrylovSolver(i32 maxIterations, Scalar tolerance)
		: m_maxIterations(maxIterations), m_tolerance(tolerance)
	{
	}

	virtual ~KrylovSolver() {}

	virtual i32 solve(MatrixX& A, TVStack& x, const TVStack& b, bool verbose = false) = 0;

	virtual void reinitialize(const TVStack& b) = 0;

	virtual SIMD_FORCE_INLINE TVStack sub(const TVStack& a, const TVStack& b)
	{
		// c = a-b
		Assert(a.size() == b.size());
		TVStack c;
		c.resize(a.size());
		for (i32 i = 0; i < a.size(); ++i)
		{
			c[i] = a[i] - b[i];
		}
		return c;
	}

	virtual SIMD_FORCE_INLINE Scalar squaredNorm(const TVStack& a)
	{
		return dot(a, a);
	}

	virtual SIMD_FORCE_INLINE Scalar norm(const TVStack& a)
	{
		Scalar ret = 0;
		for (i32 i = 0; i < a.size(); ++i)
		{
			for (i32 d = 0; d < 3; ++d)
			{
				ret = d3Max(ret, Fabs(a[i][d]));
			}
		}
		return ret;
	}

	virtual SIMD_FORCE_INLINE Scalar dot(const TVStack& a, const TVStack& b)
	{
		Scalar ans(0);
		for (i32 i = 0; i < a.size(); ++i)
			ans += a[i].dot(b[i]);
		return ans;
	}

	virtual SIMD_FORCE_INLINE void multAndAddTo(Scalar s, const TVStack& a, TVStack& result)
	{
		//        result += s*a
		Assert(a.size() == result.size());
		for (i32 i = 0; i < a.size(); ++i)
			result[i] += s * a[i];
	}

	virtual SIMD_FORCE_INLINE TVStack multAndAdd(Scalar s, const TVStack& a, const TVStack& b)
	{
		// result = a*s + b
		TVStack result;
		result.resize(a.size());
		for (i32 i = 0; i < a.size(); ++i)
			result[i] = s * a[i] + b[i];
		return result;
	}

	virtual SIMD_FORCE_INLINE void setTolerance(Scalar tolerance)
	{
		m_tolerance = tolerance;
	}
};
#endif /* DRX3D_KRYLOV_SOLVER_H */
