#ifndef DRX3D_CONJUGATE_GRADIENT_H
#define DRX3D_CONJUGATE_GRADIENT_H
#include <drx3D/Physics/SoftBody/KrylovSolver.h>
template <class MatrixX>
class ConjugateGradient : public KrylovSolver<MatrixX>
{
	typedef AlignedObjectArray<Vec3> TVStack;
	typedef KrylovSolver<MatrixX> Base;
	TVStack r, p, z, temp;

public:
	ConjugateGradient(i32k max_it_in)
		: KrylovSolver<MatrixX>(max_it_in, SIMD_EPSILON)
	{
	}

	virtual ~ConjugateGradient() {}

	// return the number of iterations taken
	i32 solve(MatrixX& A, TVStack& x, const TVStack& b, bool verbose = false)
	{
		DRX3D_PROFILE("CGSolve");
		Assert(x.size() == b.size());
		reinitialize(b);
		temp = b;
		A.project(temp);
		p = temp;
		A.precondition(p, z);
		Scalar d0 = this->dot(z, temp);
		d0 = d3Min(Scalar(1), d0);
		// r = b - A * x --with assigned dof zeroed out
		A.multiply(x, temp);
		r = this->sub(b, temp);
		A.project(r);
		// z = M^(-1) * r
		A.precondition(r, z);
		A.project(z);
		Scalar r_dot_z = this->dot(z, r);
		if (r_dot_z <= Base::m_tolerance * d0)
		{
			if (verbose)
			{
				std::cout << "Iteration = 0" << std::endl;
				std::cout << "Two norm of the residual = " << r_dot_z << std::endl;
			}
			return 0;
		}
		p = z;
		Scalar r_dot_z_new = r_dot_z;
		for (i32 k = 1; k <= Base::m_maxIterations; k++)
		{
			// temp = A*p
			A.multiply(p, temp);
			A.project(temp);
			if (this->dot(p, temp) < 0)
			{
				if (verbose)
					std::cout << "Encountered negative direction in CG!" << std::endl;
				if (k == 1)
				{
					x = b;
				}
				return k;
			}
			// alpha = r^T * z / (p^T * A * p)
			Scalar alpha = r_dot_z_new / this->dot(p, temp);
			//  x += alpha * p;
			this->multAndAddTo(alpha, p, x);
			//  r -= alpha * temp;
			this->multAndAddTo(-alpha, temp, r);
			// z = M^(-1) * r
			A.precondition(r, z);
			r_dot_z = r_dot_z_new;
			r_dot_z_new = this->dot(r, z);
			if (r_dot_z_new < Base::m_tolerance * d0)
			{
				if (verbose)
				{
					std::cout << "ConjugateGradient iterations " << k << " residual = " << r_dot_z_new << std::endl;
				}
				return k;
			}

			Scalar beta = r_dot_z_new / r_dot_z;
			p = this->multAndAdd(beta, p, z);
		}
		if (verbose)
		{
			std::cout << "ConjugateGradient max iterations reached " << Base::m_maxIterations << " error = " << r_dot_z_new << std::endl;
		}
		return Base::m_maxIterations;
	}

	void reinitialize(const TVStack& b)
	{
		r.resize(b.size());
		p.resize(b.size());
		z.resize(b.size());
		temp.resize(b.size());
	}
};
#endif /* ConjugateGradient_h */
