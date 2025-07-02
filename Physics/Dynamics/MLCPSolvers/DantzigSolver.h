
#ifndef DRX3D_DANTZIG_SOLVER_H
#define DRX3D_DANTZIG_SOLVER_H

#include "MLCPSolverInterface.h"
#include "DantzigLCP.h"

class DantzigSolver : public MLCPSolverInterface
{
protected:
	Scalar m_acceptableUpperLimitSolution;

	AlignedObjectArray<char> m_tempBuffer;

	AlignedObjectArray<Scalar> m_A;
	AlignedObjectArray<Scalar> m_b;
	AlignedObjectArray<Scalar> m_x;
	AlignedObjectArray<Scalar> m_lo;
	AlignedObjectArray<Scalar> m_hi;
	AlignedObjectArray<i32> m_dependencies;
	DantzigScratchMemory m_scratchMemory;

public:
	DantzigSolver()
		: m_acceptableUpperLimitSolution(Scalar(1000))
	{
	}

	virtual bool solveMLCP(const MatrixXu& A, const VectorXu& b, VectorXu& x, const VectorXu& lo, const VectorXu& hi, const AlignedObjectArray<i32>& limitDependency, i32 numIterations, bool useSparsity = true)
	{
		bool result = true;
		i32 n = b.rows();
		if (n)
		{
			i32 nub = 0;
			AlignedObjectArray<Scalar> ww;
			ww.resize(n);

			const Scalar* Aptr = A.getBufferPointer();
			m_A.resize(n * n);
			for (i32 i = 0; i < n * n; i++)
			{
				m_A[i] = Aptr[i];
			}

			m_b.resize(n);
			m_x.resize(n);
			m_lo.resize(n);
			m_hi.resize(n);
			m_dependencies.resize(n);
			for (i32 i = 0; i < n; i++)
			{
				m_lo[i] = lo[i];
				m_hi[i] = hi[i];
				m_b[i] = b[i];
				m_x[i] = x[i];
				m_dependencies[i] = limitDependency[i];
			}

			result = SolveDantzigLCP(n, &m_A[0], &m_x[0], &m_b[0], &ww[0], nub, &m_lo[0], &m_hi[0], &m_dependencies[0], m_scratchMemory);
			if (!result)
				return result;

			//			printf("numAllocas = %d\n",numAllocas);
			for (i32 i = 0; i < n; i++)
			{
				 Scalar xx = m_x[i];
				if (xx != m_x[i])
					return false;
				if (x[i] >= m_acceptableUpperLimitSolution)
				{
					return false;
				}

				if (x[i] <= -m_acceptableUpperLimitSolution)
				{
					return false;
				}
			}

			for (i32 i = 0; i < n; i++)
			{
				x[i] = m_x[i];
			}
		}

		return result;
	}
};

#endif  //DRX3D_DANTZIG_SOLVER_H
