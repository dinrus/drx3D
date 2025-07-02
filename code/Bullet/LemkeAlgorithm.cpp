//The original version is here
//https://code.google.com/p/mbsim-env/source/browse/trunk/kernel/mbsim/numerics/linear_complementarity_problem/lemke_algorithm.cc
//This file is re-distributed under the ZLib license, with permission of the original author
//Math library was replaced from fmatvec to a the file src/LinearMathMatrixX.h
//STL/std::vector replaced by AlignedObjectArray

#include <drx3D/Physics/Dynamics/MLCPSolvers/LemkeAlgorithm.h>

#undef DRX3D_DEBUG_OSTREAM
#ifdef DRX3D_DEBUG_OSTREAM
using namespace std;
#endif  //DRX3D_DEBUG_OSTREAM

Scalar MachEps()
{
	static bool calculated = false;
	static Scalar machEps = Scalar(1.);
	if (!calculated)
	{
		do
		{
			machEps /= Scalar(2.0);
			// If next epsilon yields 1, then break, because current
			// epsilon is the machine epsilon.
		} while ((Scalar)(1.0 + (machEps / Scalar(2.0))) != Scalar(1.0));
		//		printf( "\nCalculated Machine epsilon: %G\n", machEps );
		calculated = true;
	}
	return machEps;
}

Scalar EpsRoot()
{
	static Scalar epsroot = 0.;
	static bool alreadyCalculated = false;

	if (!alreadyCalculated)
	{
		epsroot = Sqrt(MachEps());
		alreadyCalculated = true;
	}
	return epsroot;
}

VectorXu LemkeAlgorithm::solve(u32 maxloops /* = 0*/)
{
	steps = 0;

	i32 dim = m_q.size();
#ifdef DRX3D_DEBUG_OSTREAM
	if (DEBUGLEVEL >= 1)
	{
		cout << "Dimension = " << dim << endl;
	}
#endif  //DRX3D_DEBUG_OSTREAM

	VectorXu solutionVector(2 * dim);
	solutionVector.setZero();

	//, INIT, 0.);

	MatrixXu ident(dim, dim);
	ident.setIdentity();
#ifdef DRX3D_DEBUG_OSTREAM
	cout << m_M << std::endl;
#endif

	MatrixXu mNeg = m_M.negative();

	MatrixXu A(dim, 2 * dim + 2);
	//
	A.setSubMatrix(0, 0, dim - 1, dim - 1, ident);
	A.setSubMatrix(0, dim, dim - 1, 2 * dim - 1, mNeg);
	A.setSubMatrix(0, 2 * dim, dim - 1, 2 * dim, -1.f);
	A.setSubMatrix(0, 2 * dim + 1, dim - 1, 2 * dim + 1, m_q);

#ifdef DRX3D_DEBUG_OSTREAM
	cout << A << std::endl;
#endif  //DRX3D_DEBUG_OSTREAM

	//   VectorXu q_;
	//   q_ >> A(0, 2 * dim + 1, dim - 1, 2 * dim + 1);

	AlignedObjectArray<i32> basis;
	//At first, all w-values are in the basis
	for (i32 i = 0; i < dim; i++)
		basis.push_back(i);

	i32 pivotRowIndex = -1;
	Scalar minValue = 1e30f;
	bool greaterZero = true;
	for (i32 i = 0; i < dim; i++)
	{
		Scalar v = A(i, 2 * dim + 1);
		if (v < minValue)
		{
			minValue = v;
			pivotRowIndex = i;
		}
		if (v < 0)
			greaterZero = false;
	}

	//  i32 pivotRowIndex = q_.minIndex();//minIndex(q_);     // first row is that with lowest q-value
	i32 z0Row = pivotRowIndex;    // remember the col of z0 for ending algorithm afterwards
	i32 pivotColIndex = 2 * dim;  // first col is that of z0

#ifdef DRX3D_DEBUG_OSTREAM
	if (DEBUGLEVEL >= 3)
	{
		//  cout << "A: " << A << endl;
		cout << "pivotRowIndex " << pivotRowIndex << endl;
		cout << "pivotColIndex " << pivotColIndex << endl;
		cout << "Basis: ";
		for (i32 i = 0; i < basis.size(); i++)
			cout << basis[i] << " ";
		cout << endl;
	}
#endif  //DRX3D_DEBUG_OSTREAM

	if (!greaterZero)
	{
		if (maxloops == 0)
		{
			maxloops = 100;
			//        maxloops = UINT_MAX; //TODO: not a really nice way, problem is: maxloops should be 2^dim (=1<<dim), but this could exceed UINT_MAX and thus the result would be 0 and therefore the lemke algorithm wouldn't start but probably would find a solution within less then UINT_MAX steps. Therefore this constant is used as a upper border right now...
		}

		/*start looping*/
		for (steps = 0; steps < maxloops; steps++)
		{
			GaussJordanEliminationStep(A, pivotRowIndex, pivotColIndex, basis);
#ifdef DRX3D_DEBUG_OSTREAM
			if (DEBUGLEVEL >= 3)
			{
				//  cout << "A: " << A << endl;
				cout << "pivotRowIndex " << pivotRowIndex << endl;
				cout << "pivotColIndex " << pivotColIndex << endl;
				cout << "Basis: ";
				for (i32 i = 0; i < basis.size(); i++)
					cout << basis[i] << " ";
				cout << endl;
			}
#endif  //DRX3D_DEBUG_OSTREAM

			i32 pivotColIndexOld = pivotColIndex;

			/*find new column index */
			if (basis[pivotRowIndex] < dim)  //if a w-value left the basis get in the correspondent z-value
				pivotColIndex = basis[pivotRowIndex] + dim;
			else
				//else do it the other way round and get in the corresponding w-value
				pivotColIndex = basis[pivotRowIndex] - dim;

			/*the column becomes part of the basis*/
			basis[pivotRowIndex] = pivotColIndexOld;
			bool isRayTermination = false;
			pivotRowIndex = findLexicographicMinimum(A, pivotColIndex, z0Row, isRayTermination);
			if (isRayTermination)
			{
				break; // ray termination
			}
			if (z0Row == pivotRowIndex)
			{  //if z0 leaves the basis the solution is found --> one last elimination step is necessary
				GaussJordanEliminationStep(A, pivotRowIndex, pivotColIndex, basis);
				basis[pivotRowIndex] = pivotColIndex;  //update basis
				break;
			}
		}
#ifdef DRX3D_DEBUG_OSTREAM
		if (DEBUGLEVEL >= 1)
		{
			cout << "Number of loops: " << steps << endl;
			cout << "Number of maximal loops: " << maxloops << endl;
		}
#endif  //DRX3D_DEBUG_OSTREAM

		if (!validBasis(basis))
		{
			info = -1;
#ifdef DRX3D_DEBUG_OSTREAM
			if (DEBUGLEVEL >= 1)
				cerr << "Lemke-Algorithm ended with Ray-Termination (no valid solution)." << endl;
#endif  //DRX3D_DEBUG_OSTREAM

			return solutionVector;
		}
	}
#ifdef DRX3D_DEBUG_OSTREAM
	if (DEBUGLEVEL >= 2)
	{
		// cout << "A: " << A << endl;
		cout << "pivotRowIndex " << pivotRowIndex << endl;
		cout << "pivotColIndex " << pivotColIndex << endl;
	}
#endif  //DRX3D_DEBUG_OSTREAM

	for (i32 i = 0; i < basis.size(); i++)
	{
		solutionVector[basis[i]] = A(i, 2 * dim + 1);  //q_[i];
	}

	info = 0;

	return solutionVector;
}

i32 LemkeAlgorithm::findLexicographicMinimum(const MatrixXu& A, i32k& pivotColIndex, i32k& z0Row, bool& isRayTermination)
{
	isRayTermination = false;
	AlignedObjectArray<i32> activeRows;

        bool firstRow = true;
	Scalar currentMin = 0.0;

	i32 dim = A.rows();

	for (i32 row = 0; row < dim; row++)
	{
		const Scalar denom = A(row, pivotColIndex);

		if (denom > MachEps())
		{
			const Scalar q = A(row, dim + dim + 1) / denom;
			if (firstRow)
			{
				currentMin = q;
				activeRows.push_back(row);
				firstRow = false;
			}
			else if (fabs(currentMin - q) < MachEps())
			{
				activeRows.push_back(row);
			}
			else if (currentMin > q)
			{
				currentMin = q;
				activeRows.clear();
				activeRows.push_back(row);
			}
		}
	}

	if (activeRows.size() == 0)
	{
		isRayTermination = true;
		return 0;
	}
	else if (activeRows.size() == 1)
	{
		return activeRows[0];
	}

	// if there are multiple rows, check if they contain the row for z_0.
	for (i32 i = 0; i < activeRows.size(); i++)
	{
		if (activeRows[i] == z0Row)
		{
			return z0Row;
		}
	}

	// look through the columns of the inverse of the basic matrix from left to right until the tie is broken.
	for (i32 col = 0; col < dim ; col++)
	{
		AlignedObjectArray<i32> activeRowsCopy(activeRows);
		activeRows.clear();
		firstRow = true;
		for (i32 i = 0; i<activeRowsCopy.size();i++)
		{
			i32k row = activeRowsCopy[i];

			// denom is positive here as an invariant.
			const Scalar denom = A(row, pivotColIndex);
			const Scalar ratio = A(row, col) / denom;
			if (firstRow)
			{
				currentMin = ratio;
				activeRows.push_back(row);
				firstRow = false;
			}
			else if (fabs(currentMin - ratio) < MachEps())
			{
				activeRows.push_back(row);
			}
			else if (currentMin > ratio)
			{
				currentMin = ratio;
				activeRows.clear();
				activeRows.push_back(row);
			}
		}

		if (activeRows.size() == 1)
		{
			return activeRows[0];
		}
	}
	// must not reach here.
	isRayTermination = true;
	return 0;
}

void LemkeAlgorithm::GaussJordanEliminationStep(MatrixXu& A, i32 pivotRowIndex, i32 pivotColumnIndex, const AlignedObjectArray<i32>& basis)
{
	Scalar a = -1 / A(pivotRowIndex, pivotColumnIndex);
#ifdef DRX3D_DEBUG_OSTREAM
	cout << A << std::endl;
#endif

	for (i32 i = 0; i < A.rows(); i++)
	{
		if (i != pivotRowIndex)
		{
			for (i32 j = 0; j < A.cols(); j++)
			{
				if (j != pivotColumnIndex)
				{
					Scalar v = A(i, j);
					v += A(pivotRowIndex, j) * A(i, pivotColumnIndex) * a;
					A.setElem(i, j, v);
				}
			}
		}
	}

#ifdef DRX3D_DEBUG_OSTREAM
	cout << A << std::endl;
#endif  //DRX3D_DEBUG_OSTREAM
	for (i32 i = 0; i < A.cols(); i++)
	{
		A.mulElem(pivotRowIndex, i, -a);
	}
#ifdef DRX3D_DEBUG_OSTREAM
	cout << A << std::endl;
#endif  //#ifdef DRX3D_DEBUG_OSTREAM

	for (i32 i = 0; i < A.rows(); i++)
	{
		if (i != pivotRowIndex)
		{
			A.setElem(i, pivotColumnIndex, 0);
		}
	}
#ifdef DRX3D_DEBUG_OSTREAM
	cout << A << std::endl;
#endif  //#ifdef DRX3D_DEBUG_OSTREAM
}

bool LemkeAlgorithm::greaterZero(const VectorXu& vector)
{
	bool isGreater = true;
	for (i32 i = 0; i < vector.size(); i++)
	{
		if (vector[i] < 0)
		{
			isGreater = false;
			break;
		}
	}

	return isGreater;
}

bool LemkeAlgorithm::validBasis(const AlignedObjectArray<i32>& basis)
{
	bool isValid = true;
	for (i32 i = 0; i < basis.size(); i++)
	{
		if (basis[i] >= basis.size() * 2)
		{  //then z0 is in the base
			isValid = false;
			break;
		}
	}

	return isValid;
}
