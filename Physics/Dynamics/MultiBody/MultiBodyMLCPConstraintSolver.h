#ifndef DRX3D_MULTIBODY_MLCP_CONSTRAINT_SOLVER_H
#define DRX3D_MULTIBODY_MLCP_CONSTRAINT_SOLVER_H

#include <drx3D/Maths/Linear/MatrixX.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>

class MLCPSolverInterface;
class MultiBody;

class MultiBodyMLCPConstraintSolver : public MultiBodyConstraintSolver
{
protected:
	/// \name MLCP Formulation for Rigid Bodies
	/// \{

	/// A matrix in the MLCP formulation
	MatrixXu m_A;

	/// b vector in the MLCP formulation.
	VectorXu m_b;

	/// Constraint impulse, which is an output of MLCP solving.
	VectorXu m_x;

	/// Lower bound of constraint impulse, \c m_x.
	VectorXu m_lo;

	/// Upper bound of constraint impulse, \c m_x.
	VectorXu m_hi;

	/// \}

	/// \name Cache Variables for Split Impulse for Rigid Bodies
	/// When using 'split impulse' we solve two separate (M)LCPs
	/// \{

	/// Split impulse Cache vector corresponding to \c m_b.
	VectorXu m_bSplit;

	/// Split impulse cache vector corresponding to \c m_x.
	VectorXu m_xSplit;

	/// \}

	/// \name MLCP Formulation for Multibodies
	/// \{

	/// A matrix in the MLCP formulation
	MatrixXu m_multiBodyA;

	/// b vector in the MLCP formulation.
	VectorXu m_multiBodyB;

	/// Constraint impulse, which is an output of MLCP solving.
	VectorXu m_multiBodyX;

	/// Lower bound of constraint impulse, \c m_x.
	VectorXu m_multiBodyLo;

	/// Upper bound of constraint impulse, \c m_x.
	VectorXu m_multiBodyHi;

	/// \}

	/// Indices of normal contact constraint associated with frictional contact constraint for rigid bodies.
	///
	/// This is used by the MLCP solver to update the upper bounds of frictional contact impulse given intermediate
	/// normal contact impulse. For example, i-th element represents the index of a normal constraint that is
	/// accosiated with i-th frictional contact constraint if i-th constraint is a frictional contact constraint.
	/// Otherwise, -1.
	AlignedObjectArray<i32> m_limitDependencies;

	/// Indices of normal contact constraint associated with frictional contact constraint for multibodies.
	///
	/// This is used by the MLCP solver to update the upper bounds of frictional contact impulse given intermediate
	/// normal contact impulse. For example, i-th element represents the index of a normal constraint that is
	/// accosiated with i-th frictional contact constraint if i-th constraint is a frictional contact constraint.
	/// Otherwise, -1.
	AlignedObjectArray<i32> m_multiBodyLimitDependencies;

	/// Array of all the rigid body constraints
	AlignedObjectArray<SolverConstraint*> m_allConstraintPtrArray;

	/// Array of all the multibody constraints
	AlignedObjectArray<MultiBodySolverConstraint*> m_multiBodyAllConstraintPtrArray;

	/// MLCP solver
	MLCPSolverInterface* m_solver;

	/// Count of fallbacks of using SequentialImpulseConstraintSolver, which happens when the MLCP solver fails.
	i32 m_fallback;

	/// \name MLCP Scratch Variables
	/// The following scratch variables are not stateful -- contents are cleared prior to each use.
	/// They are only cached here to avoid extra memory allocations and deallocations and to ensure
	/// that multiple instances of the solver can be run in parallel.
	///
	/// \{

	/// Cache variable for constraint Jacobian matrix.
	MatrixXu m_scratchJ3;

	/// Cache variable for constraint Jacobian times inverse mass matrix.
	MatrixXu m_scratchJInvM3;

	/// Cache variable for offsets.
	AlignedObjectArray<i32> m_scratchOfs;

	/// \}

	/// Constructs MLCP terms, which are \c m_A, \c m_b, \c m_lo, and \c m_hi.
	virtual void createMLCPFast(const ContactSolverInfo& infoGlobal);

	/// Constructs MLCP terms for constraints of two rigid bodies
	void createMLCPFastRigidBody(const ContactSolverInfo& infoGlobal);

	/// Constructs MLCP terms for constraints of two multi-bodies or one rigid body and one multibody
	void createMLCPFastMultiBody(const ContactSolverInfo& infoGlobal);

	/// Solves MLCP and returns the success
	virtual bool solveMLCP(const ContactSolverInfo& infoGlobal);

	// Documentation inherited
	Scalar solveGroupCacheFriendlySetup(
		CollisionObject2** bodies,
		i32 numBodies,
		PersistentManifold** manifoldPtr,
		i32 numManifolds,
		TypedConstraint** constraints,
		i32 numConstraints,
		const ContactSolverInfo& infoGlobal,
		IDebugDraw* debugDrawer) DRX3D_OVERRIDE;

	// Documentation inherited
	Scalar solveGroupCacheFriendlyIterations(
		CollisionObject2** bodies,
		i32 numBodies,
		PersistentManifold** manifoldPtr,
		i32 numManifolds,
		TypedConstraint** constraints,
		i32 numConstraints,
		const ContactSolverInfo& infoGlobal,
		IDebugDraw* debugDrawer) ;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR()

	/// Constructor
	///
	/// \param[in] solver MLCP solver. Assumed it's not null.
	/// \param[in] maxLCPSize Maximum size of LCP to solve using MLCP solver. If the MLCP size exceeds this number, sequaltial impulse method will be used.
	explicit MultiBodyMLCPConstraintSolver(MLCPSolverInterface* solver);

	/// Destructor
	virtual ~MultiBodyMLCPConstraintSolver();

	/// Sets MLCP solver. Assumed it's not null.
	void setMLCPSolver(MLCPSolverInterface* solver);

	/// Returns the number of fallbacks of using SequentialImpulseConstraintSolver, which happens when the MLCP
	/// solver fails.
	i32 getNumFallbacks() const;

	/// Sets the number of fallbacks. This function may be used to reset the number to zero.
	void setNumFallbacks(i32 num);

	/// Returns the constraint solver type.
	virtual ConstraintSolverType getSolverType() const;
};

#endif  // DRX3D_MULTIBODY_MLCP_CONSTRAINT_SOLVER_H
