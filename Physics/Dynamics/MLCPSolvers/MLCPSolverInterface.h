#ifndef DRX3D_MLCP_SOLVER_INTERFACE_H
#define DRX3D_MLCP_SOLVER_INTERFACE_H

#include <drx3D/Maths/Linear/MatrixX.h>

class MLCPSolverInterface
{
public:
	virtual ~MLCPSolverInterface()
	{
	}

	//return true is it solves the problem successfully
	virtual bool solveMLCP(const MatrixXu& A, const VectorXu& b, VectorXu& x, const VectorXu& lo, const VectorXu& hi, const AlignedObjectArray<i32>& limitDependency, i32 numIterations, bool useSparsity = true) = 0;
};

#endif  //DRX3D_MLCP_SOLVER_INTERFACE_H
