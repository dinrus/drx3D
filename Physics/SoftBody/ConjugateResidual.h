#ifndef DRX3D_CONJUGATE_RESIDUAL_H
#define DRX3D_CONJUGATE_RESIDUAL_H
#include <drx3D/Physics/SoftBody/KrylovSolver.h>

template <class MatrixX>
class ConjugateResidual : public KrylovSolver<MatrixX>
{
	typedef AlignedObjectArray<Vec3> TVStack;
	typedef KrylovSolver<MatrixX> Base;
	TVStack r, p, z, temp_p, temp_r, best_x;
	// temp_r = A*r
	// temp_p = A*p
	// z = M^(-1) * temp_p = M^(-1) * A * p
	Scalar best_r;

public:
	ConjugateResidual(i32k max_it_in)
		: Base(max_it_in, 1e-8)
	{
	}

	virtual ~ConjugateResidual() {}

	// return the number of iterations taken
	i32 solve(MatrixX& A, TVStack& x, const TVStack& b, bool verbose = false)
	{
		DRX3D_PROFILE("CRSolve");
		Assert(x.size() == b.size());
		reinitialize(b);
		// r = b - A * x --with assigned dof zeroed out
		A.multiply(x, temp_r);  // borrow temp_r here to store A*x
		r = this->sub(b, temp_r);
		// z = M^(-1) * r
		A.precondition(r, z);  // borrow z to store preconditioned r
		r = z;
		Scalar residual_norm = this->norm(r);
		if (residual_norm <= Base::m_tolerance)
		{
			return 0;
		}
		p = r;
		Scalar r_dot_Ar, r_dot_Ar_new;
		// temp_p = A*p
		A.multiply(p, temp_p);
		// temp_r = A*r
		temp_r = temp_p;
		r_dot_Ar = this->dot(r, temp_r);
		for (i32 k = 1; k <= Base::m_maxIterations; k++)
		{
			// z = M^(-1) * Ap
			A.precondition(temp_p, z);
			// alpha = r^T * A * r / (Ap)^T * M^-1 * Ap)
			Scalar alpha = r_dot_Ar / this->dot(temp_p, z);
			//  x += alpha * p;
			this->multAndAddTo(alpha, p, x);
			//  r -= alpha * z;
			this->multAndAddTo(-alpha, z, r);
			Scalar norm_r = this->norm(r);
			if (norm_r < best_r)
			{
				best_x = x;
				best_r = norm_r;
				if (norm_r < Base::m_tolerance)
				{
					return k;
				}
			}
			// temp_r = A * r;
			A.multiply(r, temp_r);
			r_dot_Ar_new = this->dot(r, temp_r);
			Scalar beta = r_dot_Ar_new / r_dot_Ar;
			r_dot_Ar = r_dot_Ar_new;
			// p = beta*p + r;
			p = this->multAndAdd(beta, p, r);
			// temp_p = beta*temp_p + temp_r;
			temp_p = this->multAndAdd(beta, temp_p, temp_r);
		}
		if (verbose)
		{
			std::cout << "ConjugateResidual max iterations reached, residual = " << best_r << std::endl;
		}
		x = best_x;
		return Base::m_maxIterations;
	}

	void reinitialize(const TVStack& b)
	{
		r.resize(b.size());
		p.resize(b.size());
		z.resize(b.size());
		temp_p.resize(b.size());
		temp_r.resize(b.size());
		best_x.resize(b.size());
		best_r = SIMD_INFINITY;
	}
};
#endif /* ConjugateResidual_h */
