
#ifndef DRX3D_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H
#define DRX3D_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H

#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>

class DeformableBodySolver;

// DeformableMultiBodyConstraintSolver extendsn MultiBodyConstraintSolver to solve for the contact among rigid/multibody and deformable bodies. Notice that the following constraints
// 1. rigid/multibody against rigid/multibody
// 2. rigid/multibody against deforamble
// 3. deformable against deformable
// 4. deformable self collision
// 5. joint constraints
// are all coupled in this solve.
ATTRIBUTE_ALIGNED16(class)
DeformableMultiBodyConstraintSolver : public MultiBodyConstraintSolver
{
	DeformableBodySolver* m_deformableSolver;

protected:
	// override the iterations method to include deformable/multibody contact
	//    virtual Scalar solveGroupCacheFriendlyIterations(CollisionObject2** bodies,i32 numBodies,PersistentManifold** manifoldPtr, i32 numManifolds,TypedConstraint** constraints,i32 numConstraints,const ContactSolverInfo& infoGlobal,IDebugDraw* debugDrawer);

	// write the velocity of the the solver body to the underlying rigid body
	void solverBodyWriteBack(const ContactSolverInfo& infoGlobal);

	// write the velocity of the underlying rigid body to the the the solver body
	void writeToSolverBody(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal);

	// let each deformable body knows which solver body is in constact
	void pairDeformableAndSolverBody(CollisionObject2** bodies, i32 numBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal);

	virtual void solveGroupCacheFriendlySplitImpulseIterations(CollisionObject2 * *bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

	virtual Scalar solveDeformableGroupIterations(CollisionObject2 * *bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	void setDeformableSolver(DeformableBodySolver * deformableSolver)
	{
		m_deformableSolver = deformableSolver;
	}

	virtual void solveDeformableBodyGroup(CollisionObject2 * *bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, MultiBodyConstraint** multiBodyConstraints, i32 numMultiBodyConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher);
};

#endif /* DRX3D_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H */
