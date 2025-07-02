#ifndef D3_PGS_JACOBI_SOLVER
#define D3_PGS_JACOBI_SOLVER

struct b3Contact4;
struct b3ContactPoint;

class b3Dispatcher;

#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3ContactSolverInfo.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3SolverBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3SolverConstraint.h>

struct b3RigidBodyData;
struct b3InertiaData;

class b3PgsJacobiSolver
{
protected:
	b3AlignedObjectArray<b3SolverBody> m_tmpSolverBodyPool;
	b3ConstraintArray m_tmpSolverContactConstraintPool;
	b3ConstraintArray m_tmpSolverNonContactConstraintPool;
	b3ConstraintArray m_tmpSolverContactFrictionConstraintPool;
	b3ConstraintArray m_tmpSolverContactRollingFrictionConstraintPool;

	b3AlignedObjectArray<i32> m_orderTmpConstraintPool;
	b3AlignedObjectArray<i32> m_orderNonContactConstraintPool;
	b3AlignedObjectArray<i32> m_orderFrictionConstraintPool;
	b3AlignedObjectArray<b3TypedConstraint::b3ConstraintInfo1> m_tmpConstraintSizesPool;

	b3AlignedObjectArray<i32> m_bodyCount;
	b3AlignedObjectArray<i32> m_bodyCountCheck;

	b3AlignedObjectArray<b3Vec3> m_deltaLinearVelocities;
	b3AlignedObjectArray<b3Vec3> m_deltaAngularVelocities;

	bool m_usePgs;
	void averageVelocities();

	i32 m_maxOverrideNumSolverIterations;

	i32 m_numSplitImpulseRecoveries;

	b3Scalar getContactProcessingThreshold(b3Contact4* contact)
	{
		return 0.02f;
	}
	void setupFrictionConstraint(b3RigidBodyData* bodies, b3InertiaData* inertias, b3SolverConstraint& solverConstraint, const b3Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB,
								 b3ContactPoint& cp, const b3Vec3& rel_pos1, const b3Vec3& rel_pos2,
								 b3RigidBodyData* colObj0, b3RigidBodyData* colObj1, b3Scalar relaxation,
								 b3Scalar desiredVelocity = 0., b3Scalar cfmSlip = 0.);

	void setupRollingFrictionConstraint(b3RigidBodyData* bodies, b3InertiaData* inertias, b3SolverConstraint& solverConstraint, const b3Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB,
										b3ContactPoint& cp, const b3Vec3& rel_pos1, const b3Vec3& rel_pos2,
										b3RigidBodyData* colObj0, b3RigidBodyData* colObj1, b3Scalar relaxation,
										b3Scalar desiredVelocity = 0., b3Scalar cfmSlip = 0.);

	b3SolverConstraint& addFrictionConstraint(b3RigidBodyData* bodies, b3InertiaData* inertias, const b3Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB, i32 frictionIndex, b3ContactPoint& cp, const b3Vec3& rel_pos1, const b3Vec3& rel_pos2, b3RigidBodyData* colObj0, b3RigidBodyData* colObj1, b3Scalar relaxation, b3Scalar desiredVelocity = 0., b3Scalar cfmSlip = 0.);
	b3SolverConstraint& addRollingFrictionConstraint(b3RigidBodyData* bodies, b3InertiaData* inertias, const b3Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB, i32 frictionIndex, b3ContactPoint& cp, const b3Vec3& rel_pos1, const b3Vec3& rel_pos2, b3RigidBodyData* colObj0, b3RigidBodyData* colObj1, b3Scalar relaxation, b3Scalar desiredVelocity = 0, b3Scalar cfmSlip = 0.f);

	void setupContactConstraint(b3RigidBodyData* bodies, b3InertiaData* inertias,
								b3SolverConstraint& solverConstraint, i32 solverBodyIdA, i32 solverBodyIdB, b3ContactPoint& cp,
								const b3ContactSolverInfo& infoGlobal, b3Vec3& vel, b3Scalar& rel_vel, b3Scalar& relaxation,
								b3Vec3& rel_pos1, b3Vec3& rel_pos2);

	void setFrictionConstraintImpulse(b3RigidBodyData* bodies, b3InertiaData* inertias, b3SolverConstraint& solverConstraint, i32 solverBodyIdA, i32 solverBodyIdB,
									  b3ContactPoint& cp, const b3ContactSolverInfo& infoGlobal);

	///m_Seed2 is used for re-arranging the constraint rows. improves convergence/quality of friction
	u64 m_Seed2;

	b3Scalar restitutionCurve(b3Scalar rel_vel, b3Scalar restitution);

	void convertContact(b3RigidBodyData* bodies, b3InertiaData* inertias, b3Contact4* manifold, const b3ContactSolverInfo& infoGlobal);

	void resolveSplitPenetrationSIMD(
		b3SolverBody& bodyA, b3SolverBody& bodyB,
		const b3SolverConstraint& contactConstraint);

	void resolveSplitPenetrationImpulseCacheFriendly(
		b3SolverBody& bodyA, b3SolverBody& bodyB,
		const b3SolverConstraint& contactConstraint);

	//internal method
	i32 getOrInitSolverBody(i32 bodyIndex, b3RigidBodyData* bodies, b3InertiaData* inertias);
	void initSolverBody(i32 bodyIndex, b3SolverBody* solverBody, b3RigidBodyData* collisionObject);

	void resolveSingleConstraintRowGeneric(b3SolverBody& bodyA, b3SolverBody& bodyB, const b3SolverConstraint& contactConstraint);

	void resolveSingleConstraintRowGenericSIMD(b3SolverBody& bodyA, b3SolverBody& bodyB, const b3SolverConstraint& contactConstraint);

	void resolveSingleConstraintRowLowerLimit(b3SolverBody& bodyA, b3SolverBody& bodyB, const b3SolverConstraint& contactConstraint);

	void resolveSingleConstraintRowLowerLimitSIMD(b3SolverBody& bodyA, b3SolverBody& bodyB, const b3SolverConstraint& contactConstraint);

protected:
	virtual b3Scalar solveGroupCacheFriendlySetup(b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numBodies, b3Contact4* manifoldPtr, i32 numManifolds, b3TypedConstraint** constraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);

	virtual b3Scalar solveGroupCacheFriendlyIterations(b3TypedConstraint** constraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);
	virtual void solveGroupCacheFriendlySplitImpulseIterations(b3TypedConstraint** constraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);
	b3Scalar solveSingleIteration(i32 iteration, b3TypedConstraint** constraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);

	virtual b3Scalar solveGroupCacheFriendlyFinish(b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numBodies, const b3ContactSolverInfo& infoGlobal);

public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	b3PgsJacobiSolver(bool usePgs);
	virtual ~b3PgsJacobiSolver();

	//	void	solveContacts(i32 numBodies, b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numContacts, b3Contact4* contacts);
	void solveContacts(i32 numBodies, b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numContacts, b3Contact4* contacts, i32 numConstraints, b3TypedConstraint** constraints);

	b3Scalar solveGroup(b3RigidBodyData* bodies, b3InertiaData* inertias, i32 numBodies, b3Contact4* manifoldPtr, i32 numManifolds, b3TypedConstraint** constraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);

	///clear internal cached data and reset random seed
	virtual void reset();

	u64 b3Rand2();

	i32 b3RandInt2(i32 n);

	void setRandSeed(u64 seed)
	{
		m_Seed2 = seed;
	}
	u64 getRandSeed() const
	{
		return m_Seed2;
	}
};

#endif  //D3_PGS_JACOBI_SOLVER
