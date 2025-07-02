#ifndef DRX3D_MLCP_SOLVER_H
#define DRX3D_MLCP_SOLVER_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Maths/Linear/MatrixX.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolverInterface.h>

class MLCPSolver : public SequentialImpulseConstraintSolver
{
protected:
	MatrixXu m_A;
	VectorXu m_b;
	VectorXu m_x;
	VectorXu m_lo;
	VectorXu m_hi;

	///when using 'split impulse' we solve two separate (M)LCPs
	VectorXu m_bSplit;
	VectorXu m_xSplit;
	VectorXu m_bSplit1;
	VectorXu m_xSplit2;

	AlignedObjectArray<i32> m_limitDependencies;
	AlignedObjectArray<SolverConstraint*> m_allConstraintPtrArray;
	MLCPSolverInterface* m_solver;
	i32 m_fallback;

	/// The following scratch variables are not stateful -- contents are cleared prior to each use.
	/// They are only cached here to avoid extra memory allocations and deallocations and to ensure
	/// that multiple instances of the solver can be run in parallel.
	MatrixXu m_scratchJ3;
	MatrixXu m_scratchJInvM3;
	AlignedObjectArray<i32> m_scratchOfs;
	MatrixXu m_scratchMInv;
	MatrixXu m_scratchJ;
	MatrixXu m_scratchJTranspose;
	MatrixXu m_scratchTmp;

	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);
	virtual Scalar solveGroupCacheFriendlyIterations(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

	virtual void createMLCP(const ContactSolverInfo& infoGlobal);
	virtual void createMLCPFast(const ContactSolverInfo& infoGlobal);

	//return true is it solves the problem successfully
	virtual bool solveMLCP(const ContactSolverInfo& infoGlobal);

public:
	MLCPSolver(MLCPSolverInterface* solver);
	virtual ~MLCPSolver();

	void setMLCPSolver(MLCPSolverInterface* solver)
	{
		m_solver = solver;
	}

	i32 getNumFallbacks() const
	{
		return m_fallback;
	}
	void setNumFallbacks(i32 num)
	{
		m_fallback = num;
	}

	virtual ConstraintSolverType getSolverType() const
	{
		return DRX3D_MLCP_SOLVER;
	}
};

#endif  //DRX3D_MLCP_SOLVER_H
