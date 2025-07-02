#ifndef DRX3D_SOLVE_PROJECTED_GAUSS_SEIDEL_H
#define DRX3D_SOLVE_PROJECTED_GAUSS_SEIDEL_H

#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolverInterface.h>

///This solver is mainly for debug/learning purposes: it is functionally equivalent to the SequentialImpulseConstraintSolver solver, but much slower (it builds the full LCP matrix)
class SolveProjectedGaussSeidel : public MLCPSolverInterface
{
public:
	Scalar m_leastSquaresResidualThreshold;
	Scalar m_leastSquaresResidual;

	SolveProjectedGaussSeidel()
		: m_leastSquaresResidualThreshold(0),
		  m_leastSquaresResidual(0)
	{
	}

	virtual bool solveMLCP(const MatrixXu& A, const VectorXu& b, VectorXu& x, const VectorXu& lo, const VectorXu& hi, const AlignedObjectArray<i32>& limitDependency, i32 numIterations, bool useSparsity = true)
	{
		if (!A.rows())
			return true;
		//the A matrix is sparse, so compute the non-zero elements
		A.rowComputeNonZeroElements();

		//A is a m-n matrix, m rows, n columns
		Assert(A.rows() == b.rows());

		i32 i, j, numRows = A.rows();

		Scalar delta;

		for (i32 k = 0; k < numIterations; k++)
		{
			m_leastSquaresResidual = 0.f;
			for (i = 0; i < numRows; i++)
			{
				delta = 0.0f;
				if (useSparsity)
				{
					for (i32 h = 0; h < A.m_rowNonZeroElements1[i].size(); h++)
					{
						j = A.m_rowNonZeroElements1[i][h];
						if (j != i)  //skip main diagonal
						{
							delta += A(i, j) * x[j];
						}
					}
				}
				else
				{
					for (j = 0; j < i; j++)
						delta += A(i, j) * x[j];
					for (j = i + 1; j < numRows; j++)
						delta += A(i, j) * x[j];
				}

				Scalar aDiag = A(i, i);
				Scalar xOld = x[i];
				x[i] = (b[i] - delta) / aDiag;
				Scalar s = 1.f;

				if (limitDependency[i] >= 0)
				{
					s = x[limitDependency[i]];
					if (s < 0)
						s = 1;
				}

				if (x[i] < lo[i] * s)
					x[i] = lo[i] * s;
				if (x[i] > hi[i] * s)
					x[i] = hi[i] * s;
				Scalar diff = x[i] - xOld;
				m_leastSquaresResidual += diff * diff;
			}

			Scalar eps = m_leastSquaresResidualThreshold;
			if ((m_leastSquaresResidual < eps) || (k >= (numIterations - 1)))
			{
#ifdef VERBOSE_PRINTF_RESIDUAL
				printf("totalLenSqr = %f at iteration #%d\n", m_leastSquaresResidual, k);
#endif
				break;
			}
		}
		return true;
	}
};

#endif  //DRX3D_SOLVE_PROJECTED_GAUSS_SEIDEL_H
