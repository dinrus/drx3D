#ifndef DRX3D_NNCG_CONSTRAINT_SOLVER_H
#define DRX3D_NNCG_CONSTRAINT_SOLVER_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>

ATTRIBUTE_ALIGNED16(class)
NNCGConstraintSolver : public SequentialImpulseConstraintSolver
{
protected:
	Scalar m_deltafLengthSqrPrev;

	AlignedObjectArray<Scalar> m_pNC;   // p for None Contact constraints
	AlignedObjectArray<Scalar> m_pC;    // p for Contact constraints
	AlignedObjectArray<Scalar> m_pCF;   // p for ContactFriction constraints
	AlignedObjectArray<Scalar> m_pCRF;  // p for ContactRollingFriction constraints

	//These are recalculated in every iterations. We just keep these to prevent reallocation in each iteration.
	AlignedObjectArray<Scalar> m_deltafNC;   // deltaf for NoneContact constraints
	AlignedObjectArray<Scalar> m_deltafC;    // deltaf for Contact constraints
	AlignedObjectArray<Scalar> m_deltafCF;   // deltaf for ContactFriction constraints
	AlignedObjectArray<Scalar> m_deltafCRF;  // deltaf for ContactRollingFriction constraints

protected:
	virtual Scalar solveGroupCacheFriendlyFinish(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal);
	virtual Scalar solveSingleIteration(i32 iteration, CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	NNCGConstraintSolver() : SequentialImpulseConstraintSolver(), m_onlyForNoneContact(false) {}

	virtual ConstraintSolverType getSolverType() const
	{
		return DRX3D_NNCG_SOLVER;
	}

	bool m_onlyForNoneContact;
};

#endif  //DRX3D_NNCG_CONSTRAINT_SOLVER_H
