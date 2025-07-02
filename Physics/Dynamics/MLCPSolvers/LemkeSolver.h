#ifndef DRX3D_LEMKE_SOLVER_H
#define DRX3D_LEMKE_SOLVER_H

#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolverInterface.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/LemkeAlgorithm.h>

///The btLemkeSolver is based on "Fast Implementation of Lemke’s Algorithm for Rigid Body Contact Simulation (John E. Lloyd) "
///It is a slower but more accurate solver. Increase the m_maxLoops for better convergence, at the cost of more CPU time.
///The original implementation of the btLemkeAlgorithm was done by Kilian Grundl from the MBSim team
class LemkeSolver : public MLCPSolverInterface
{
protected:
public:
	Scalar m_maxValue;
	i32 m_debugLevel;
	i32 m_maxLoops;
	bool m_useLoHighBounds;

	LemkeSolver()
		: m_maxValue(100000),
		  m_debugLevel(0),
		  m_maxLoops(1000),
		  m_useLoHighBounds(true)
	{
	}
	virtual bool solveMLCP(const MatrixXu& A, const VectorXu& b, VectorXu& x, const VectorXu& lo, const VectorXu& hi, const AlignedObjectArray<i32>& limitDependency, i32 numIterations, bool useSparsity = true)
	{
		if (m_useLoHighBounds)
		{
			DRX3D_PROFILE("LemkeSolver::solveMLCP");
			i32 n = A.rows();
			if (0 == n)
				return true;

			bool fail = false;

			VectorXu solution(n);
			VectorXu q1;
			q1.resize(n);
			for (i32 row = 0; row < n; row++)
			{
				q1[row] = -b[row];
			}

			//		cout << "A" << endl;
			//		cout << A << endl;

			/////////////////////////////////////

			//slow matrix inversion, replace with LU decomposition
			MatrixXu A1;
			MatrixXu B(n, n);
			{
				//DRX3D_PROFILE("inverse(slow)");
				A1.resize(A.rows(), A.cols());
				for (i32 row = 0; row < A.rows(); row++)
				{
					for (i32 col = 0; col < A.cols(); col++)
					{
						A1.setElem(row, col, A(row, col));
					}
				}

				MatrixXu matrix;
				matrix.resize(n, 2 * n);
				for (i32 row = 0; row < n; row++)
				{
					for (i32 col = 0; col < n; col++)
					{
						matrix.setElem(row, col, A1(row, col));
					}
				}

				Scalar ratio, a;
				i32 i, j, k;
				for (i = 0; i < n; i++)
				{
					for (j = n; j < 2 * n; j++)
					{
						if (i == (j - n))
							matrix.setElem(i, j, 1.0);
						else
							matrix.setElem(i, j, 0.0);
					}
				}
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						if (i != j)
						{
							Scalar v = matrix(i, i);
							if (FuzzyZero(v))
							{
								a = 0.000001f;
							}
							ratio = matrix(j, i) / matrix(i, i);
							for (k = 0; k < 2 * n; k++)
							{
								matrix.addElem(j, k, -ratio * matrix(i, k));
							}
						}
					}
				}
				for (i = 0; i < n; i++)
				{
					a = matrix(i, i);
					if (FuzzyZero(a))
					{
						a = 0.000001f;
					}
					Scalar invA = 1.f / a;
					for (j = 0; j < 2 * n; j++)
					{
						matrix.mulElem(i, j, invA);
					}
				}

				for (i32 row = 0; row < n; row++)
				{
					for (i32 col = 0; col < n; col++)
					{
						B.setElem(row, col, matrix(row, n + col));
					}
				}
			}

			MatrixXu b1(n, 1);

			MatrixXu M(n * 2, n * 2);
			for (i32 row = 0; row < n; row++)
			{
				b1.setElem(row, 0, -b[row]);
				for (i32 col = 0; col < n; col++)
				{
					Scalar v = B(row, col);
					M.setElem(row, col, v);
					M.setElem(n + row, n + col, v);
					M.setElem(n + row, col, -v);
					M.setElem(row, n + col, -v);
				}
			}

			MatrixXu Bb1 = B * b1;
			//		q = [ (-B*b1 - lo)'   (hi + B*b1)' ]'

			VectorXu qq;
			qq.resize(n * 2);
			for (i32 row = 0; row < n; row++)
			{
				qq[row] = -Bb1(row, 0) - lo[row];
				qq[n + row] = Bb1(row, 0) + hi[row];
			}

			VectorXu z1;

			MatrixXu y1;
			y1.resize(n, 1);
			LemkeAlgorithm lemke(M, qq, m_debugLevel);
			{
				//DRX3D_PROFILE("lemke.solve");
				lemke.setSystem(M, qq);
				z1 = lemke.solve(m_maxLoops);
			}
			for (i32 row = 0; row < n; row++)
			{
				y1.setElem(row, 0, z1[2 * n + row] - z1[3 * n + row]);
			}
			MatrixXu y1_b1(n, 1);
			for (i32 i = 0; i < n; i++)
			{
				y1_b1.setElem(i, 0, y1(i, 0) - b1(i, 0));
			}

			MatrixXu x1;

			x1 = B * (y1_b1);

			for (i32 row = 0; row < n; row++)
			{
				solution[row] = x1(row, 0);  //n];
			}

			i32 errorIndexMax = -1;
			i32 errorIndexMin = -1;
			float errorValueMax = -1e30;
			float errorValueMin = 1e30;

			for (i32 i = 0; i < n; i++)
			{
				x[i] = solution[i];
				 Scalar check = x[i];
				if (x[i] != check)
				{
					//printf("Lemke result is #NAN\n");
					x.setZero();
					return false;
				}

				//this is some hack/safety mechanism, to discard invalid solutions from the Lemke solver
				//we need to figure out why it happens, and fix it, or detect it properly)
				if (x[i] > m_maxValue)
				{
					if (x[i] > errorValueMax)
					{
						fail = true;
						errorIndexMax = i;
						errorValueMax = x[i];
					}
					////printf("x[i] = %f,",x[i]);
				}
				if (x[i] < -m_maxValue)
				{
					if (x[i] < errorValueMin)
					{
						errorIndexMin = i;
						errorValueMin = x[i];
						fail = true;
						//printf("x[i] = %f,",x[i]);
					}
				}
			}
			if (fail)
			{
				i32 m_errorCountTimes = 0;
				if (errorIndexMin < 0)
					errorValueMin = 0.f;
				if (errorIndexMax < 0)
					errorValueMax = 0.f;
				m_errorCountTimes++;
				//	printf("Error (x[%d] = %f, x[%d] = %f), resetting %d times\n", errorIndexMin,errorValueMin, errorIndexMax, errorValueMax, errorCountTimes++);
				for (i32 i = 0; i < n; i++)
				{
					x[i] = 0.f;
				}
			}
			return !fail;
		}
		else

		{
			i32 dimension = A.rows();
			if (0 == dimension)
				return true;

			//		printf("================ solving using Lemke/Newton/Fixpoint\n");

			VectorXu q;
			q.resize(dimension);
			for (i32 row = 0; row < dimension; row++)
			{
				q[row] = -b[row];
			}

			LemkeAlgorithm lemke(A, q, m_debugLevel);

			lemke.setSystem(A, q);

			VectorXu solution = lemke.solve(m_maxLoops);

			//check solution

			bool fail = false;
			i32 errorIndexMax = -1;
			i32 errorIndexMin = -1;
			float errorValueMax = -1e30;
			float errorValueMin = 1e30;

			for (i32 i = 0; i < dimension; i++)
			{
				x[i] = solution[i + dimension];
				 Scalar check = x[i];
				if (x[i] != check)
				{
					x.setZero();
					return false;
				}

				//this is some hack/safety mechanism, to discard invalid solutions from the Lemke solver
				//we need to figure out why it happens, and fix it, or detect it properly)
				if (x[i] > m_maxValue)
				{
					if (x[i] > errorValueMax)
					{
						fail = true;
						errorIndexMax = i;
						errorValueMax = x[i];
					}
					////printf("x[i] = %f,",x[i]);
				}
				if (x[i] < -m_maxValue)
				{
					if (x[i] < errorValueMin)
					{
						errorIndexMin = i;
						errorValueMin = x[i];
						fail = true;
						//printf("x[i] = %f,",x[i]);
					}
				}
			}
			if (fail)
			{
				static i32 errorCountTimes = 0;
				if (errorIndexMin < 0)
					errorValueMin = 0.f;
				if (errorIndexMax < 0)
					errorValueMax = 0.f;
				printf("Error (x[%d] = %f, x[%d] = %f), resetting %d times\n", errorIndexMin, errorValueMin, errorIndexMax, errorValueMax, errorCountTimes++);
				for (i32 i = 0; i < dimension; i++)
				{
					x[i] = 0.f;
				}
			}

			return !fail;
		}
		return true;
	}
};

#endif  //DRX3D_LEMKE_SOLVER_H
