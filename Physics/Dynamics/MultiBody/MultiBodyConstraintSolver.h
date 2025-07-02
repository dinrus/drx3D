#ifndef DRX3D_MULTIBODY_CONSTRAINT_SOLVER_H
#define DRX3D_MULTIBODY_CONSTRAINT_SOLVER_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySolverConstraint.h>

#define DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS

class MultiBody;

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>

ATTRIBUTE_ALIGNED16(class)
MultiBodyConstraintSolver : public SequentialImpulseConstraintSolver
{
protected:
	MultiBodyConstraintArray m_multiBodyNonContactConstraints;

	MultiBodyConstraintArray m_multiBodyNormalContactConstraints;
	MultiBodyConstraintArray m_multiBodyFrictionContactConstraints;
	MultiBodyConstraintArray m_multiBodyTorsionalFrictionContactConstraints;
	MultiBodyConstraintArray m_multiBodySpinningFrictionContactConstraints;

	MultiBodyJacobianData m_data;

	//temp storage for multi body constraints for a specific island/group called by 'solveGroup'
	MultiBodyConstraint** m_tmpMultiBodyConstraints;
	i32 m_tmpNumMultiBodyConstraints;

	Scalar resolveSingleConstraintRowGeneric(const MultiBodySolverConstraint& c);

	//solve 2 friction directions and clamp against the implicit friction cone
	Scalar resolveConeFrictionConstraintRows(const MultiBodySolverConstraint& cA1, const MultiBodySolverConstraint& cB);

	void convertContacts(PersistentManifold * *manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal);

	MultiBodySolverConstraint& addMultiBodyFrictionConstraint(const Vec3& normalAxis, const Scalar& appliedImpulse, PersistentManifold* manifold, i32 frictionIndex, ManifoldPoint& cp, CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation, const ContactSolverInfo& infoGlobal, Scalar desiredVelocity = 0, Scalar cfmSlip = 0);

	MultiBodySolverConstraint& addMultiBodyTorsionalFrictionConstraint(const Vec3& normalAxis, PersistentManifold* manifold, i32 frictionIndex, ManifoldPoint& cp,
																		 Scalar combinedTorsionalFriction,
																		 CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation, const ContactSolverInfo& infoGlobal, Scalar desiredVelocity = 0, Scalar cfmSlip = 0);

	MultiBodySolverConstraint& addMultiBodySpinningFrictionConstraint(const Vec3& normalAxis, PersistentManifold* manifold, i32 frictionIndex, ManifoldPoint& cp,
		Scalar combinedTorsionalFriction,
		CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation, const ContactSolverInfo& infoGlobal, Scalar desiredVelocity = 0, Scalar cfmSlip = 0);

	void setupMultiBodyJointLimitConstraint(MultiBodySolverConstraint & constraintRow,
											Scalar * jacA, Scalar * jacB,
											Scalar penetration, Scalar combinedFrictionCoeff, Scalar combinedRestitutionCoeff,
											const ContactSolverInfo& infoGlobal);

	void setupMultiBodyContactConstraint(MultiBodySolverConstraint & solverConstraint,
										 const Vec3& contactNormal,
                     const Scalar& appliedImpulse,
										 ManifoldPoint& cp,
                     const ContactSolverInfo& infoGlobal,
										 Scalar& relaxation,
										 bool isFriction, Scalar desiredVelocity = 0, Scalar cfmSlip = 0);

	//either rolling or spinning friction
	void setupMultiBodyTorsionalFrictionConstraint(MultiBodySolverConstraint & solverConstraint,
												   const Vec3& contactNormal,
												   ManifoldPoint& cp,
												   Scalar combinedTorsionalFriction,
												   const ContactSolverInfo& infoGlobal,
												   Scalar& relaxation,
												   bool isFriction, Scalar desiredVelocity = 0, Scalar cfmSlip = 0);

	void convertMultiBodyContact(PersistentManifold * manifold, const ContactSolverInfo& infoGlobal);
	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);
	//	virtual Scalar solveGroupCacheFriendlyIterations(CollisionObject2** bodies,i32 numBodies,PersistentManifold** manifoldPtr, i32 numManifolds,TypedConstraint** constraints,i32 numConstraints,const ContactSolverInfo& infoGlobal,IDebugDraw* debugDrawer);
	virtual Scalar solveSingleIteration(i32 iteration, CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);
	void applyDeltaVee(Scalar * deltaV, Scalar impulse, i32 velocityIndex, i32 ndof);
	void writeBackSolverBodyToMultiBody(MultiBodySolverConstraint & constraint, Scalar deltaTime);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	///this method should not be called, it was just used during porting/integration of MultiBody MultiBody, providing backwards compatibility but no support for MultiBodyConstraint (only contact constraints)
	virtual Scalar solveGroup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher);
	virtual Scalar solveGroupCacheFriendlyFinish(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal);

	virtual void solveMultiBodyGroup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, MultiBodyConstraint** multiBodyConstraints, i32 numMultiBodyConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher);
};

#endif  //DRX3D_MULTIBODY_CONSTRAINT_SOLVER_H
