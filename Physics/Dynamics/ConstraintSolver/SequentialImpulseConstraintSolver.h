#ifndef DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H
#define DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H

class IDebugDraw;
class PersistentManifold;
class Dispatcher;
class CollisionObject2;
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverConstraint.h>
#include <drx3D/Physics/Collision/NarrowPhase/ManifoldPoint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ConstraintSolver.h>

typedef Scalar (*SingleConstraintRowSolver)(SolverBody&, SolverBody&, const SolverConstraint&);

struct SolverAnalyticsData
{
	SolverAnalyticsData()
	{
		m_numSolverCalls = 0;
		m_numIterationsUsed = -1;
		m_remainingLeastSquaresResidual = -1;
		m_islandId = -2;
	}
	i32 m_islandId;
	i32 m_numBodies;
	i32 m_numContactManifolds;
	i32 m_numSolverCalls;
	i32 m_numIterationsUsed;
	double m_remainingLeastSquaresResidual;
};

///The SequentialImpulseConstraintSolver is a fast SIMD implementation of the Projected Gauss Seidel (iterative LCP) method.
ATTRIBUTE_ALIGNED16(class)
SequentialImpulseConstraintSolver : public ConstraintSolver
{
	

protected:
	AlignedObjectArray<SolverBody> m_tmpSolverBodyPool;
	ConstraintArray m_tmpSolverContactConstraintPool;
	ConstraintArray m_tmpSolverNonContactConstraintPool;
	ConstraintArray m_tmpSolverContactFrictionConstraintPool;
	ConstraintArray m_tmpSolverContactRollingFrictionConstraintPool;

	AlignedObjectArray<i32> m_orderTmpConstraintPool;
	AlignedObjectArray<i32> m_orderNonContactConstraintPool;
	AlignedObjectArray<i32> m_orderFrictionConstraintPool;
	AlignedObjectArray<TypedConstraint::ConstraintInfo1> m_tmpConstraintSizesPool;
	i32 m_maxOverrideNumSolverIterations;
	i32 m_fixedBodyId;
	// When running solvers on multiple threads, a race condition exists for Kinematic objects that
	// participate in more than one solver.
	// The getOrInitSolverBody() function writes the companionId of each body (storing the index of the solver body
	// for the current solver). For normal dynamic bodies it isn't an issue because they can only be in one island
	// (and therefore one thread) at a time. But kinematic bodies can be in multiple islands at once.
	// To avoid this race condition, this solver does not write the companionId, instead it stores the solver body
	// index in this solver-local table, indexed by the uniqueId of the body.
	AlignedObjectArray<i32> m_kinematicBodyUniqueIdToSolverBodyTable;  // only used for multithreading

	SingleConstraintRowSolver m_resolveSingleConstraintRowGeneric;
	SingleConstraintRowSolver m_resolveSingleConstraintRowLowerLimit;
	SingleConstraintRowSolver m_resolveSplitPenetrationImpulse;
	i32 m_cachedSolverMode;  // used to check if SOLVER_SIMD flag has been changed
	void setupSolverFunctions(bool useSimd);

	Scalar m_leastSquaresResidual;

	void setupFrictionConstraint(SolverConstraint & solverConstraint, const Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB,
		ManifoldPoint& cp, const Vec3& rel_pos1, const Vec3& rel_pos2,
		CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation,
		const ContactSolverInfo& infoGlobal,
		Scalar desiredVelocity = 0., Scalar cfmSlip = 0.);

	void setupTorsionalFrictionConstraint(SolverConstraint & solverConstraint, const Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB,
		ManifoldPoint& cp, Scalar combinedTorsionalFriction, const Vec3& rel_pos1, const Vec3& rel_pos2,
		CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation,
		Scalar desiredVelocity = 0., Scalar cfmSlip = 0.);

	SolverConstraint& addFrictionConstraint(const Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB, i32 frictionIndex, ManifoldPoint& cp, const Vec3& rel_pos1, const Vec3& rel_pos2, CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation, const ContactSolverInfo& infoGlobal, Scalar desiredVelocity = 0., Scalar cfmSlip = 0.);
	SolverConstraint& addTorsionalFrictionConstraint(const Vec3& normalAxis, i32 solverBodyIdA, i32 solverBodyIdB, i32 frictionIndex, ManifoldPoint& cp, Scalar torsionalFriction, const Vec3& rel_pos1, const Vec3& rel_pos2, CollisionObject2* colObj0, CollisionObject2* colObj1, Scalar relaxation, Scalar desiredVelocity = 0, Scalar cfmSlip = 0.f);

	void setupContactConstraint(SolverConstraint & solverConstraint, i32 solverBodyIdA, i32 solverBodyIdB, ManifoldPoint& cp,
		const ContactSolverInfo& infoGlobal, Scalar& relaxation, const Vec3& rel_pos1, const Vec3& rel_pos2);

	static void applyAnisotropicFriction(CollisionObject2 * colObj, Vec3 & frictionDirection, i32 frictionMode);

	void setFrictionConstraintImpulse(SolverConstraint & solverConstraint, i32 solverBodyIdA, i32 solverBodyIdB,
		ManifoldPoint& cp, const ContactSolverInfo& infoGlobal);

	///m_Seed2 is used for re-arranging the constraint rows. improves convergence/quality of friction
	u64 m_Seed2;

	Scalar restitutionCurve(Scalar rel_vel, Scalar restitution, Scalar velocityThreshold);

	virtual void convertContacts(PersistentManifold * *manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal);

	void convertContact(PersistentManifold * manifold, const ContactSolverInfo& infoGlobal);

	virtual void convertJoints(TypedConstraint * *constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal);
	void convertJoint(SolverConstraint * currentConstraintRow, TypedConstraint * constraint, const TypedConstraint::ConstraintInfo1& info1, i32 solverBodyIdA, i32 solverBodyIdB, const ContactSolverInfo& infoGlobal);

	virtual void convertBodies(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal);

	Scalar resolveSplitPenetrationSIMD(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint)
	{
		return m_resolveSplitPenetrationImpulse(bodyA, bodyB, contactConstraint);
	}

	Scalar resolveSplitPenetrationImpulseCacheFriendly(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint)
	{
		return m_resolveSplitPenetrationImpulse(bodyA, bodyB, contactConstraint);
	}

	//internal method
	i32 getOrInitSolverBody(CollisionObject2 & body, Scalar timeStep);
	void initSolverBody(SolverBody * solverBody, CollisionObject2 * collisionObject, Scalar timeStep);

	Scalar resolveSingleConstraintRowGeneric(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint);
	Scalar resolveSingleConstraintRowGenericSIMD(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint);
	Scalar resolveSingleConstraintRowLowerLimit(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint);
	Scalar resolveSingleConstraintRowLowerLimitSIMD(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint);
	Scalar resolveSplitPenetrationImpulse(SolverBody & bodyA, SolverBody & bodyB, const SolverConstraint& contactConstraint)
	{
		return m_resolveSplitPenetrationImpulse(bodyA, bodyB, contactConstraint);
	}

protected:
	void writeBackContacts(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	void writeBackJoints(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	void writeBackBodies(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	virtual void solveGroupCacheFriendlySplitImpulseIterations(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);
	virtual Scalar solveGroupCacheFriendlyFinish(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal);
	virtual Scalar solveSingleIteration(i32 iteration, CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);
	virtual Scalar solveGroupCacheFriendlyIterations(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	SequentialImpulseConstraintSolver();
	virtual ~SequentialImpulseConstraintSolver();

	virtual Scalar solveGroup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher);

	///clear internal cached data and reset random seed
	virtual void reset();

	u64 Rand2();

	i32 RandInt2(i32 n);

	void setRandSeed(u64 seed)
	{
		m_Seed2 = seed;
	}
	u64 getRandSeed() const
	{
		return m_Seed2;
	}

	virtual ConstraintSolverType getSolverType() const
	{
		return DRX3D_SEQUENTIAL_IMPULSE_SOLVER;
	}

	SingleConstraintRowSolver getActiveConstraintRowSolverGeneric()
	{
		return m_resolveSingleConstraintRowGeneric;
	}
	void setConstraintRowSolverGeneric(SingleConstraintRowSolver rowSolver)
	{
		m_resolveSingleConstraintRowGeneric = rowSolver;
	}
	SingleConstraintRowSolver getActiveConstraintRowSolverLowerLimit()
	{
		return m_resolveSingleConstraintRowLowerLimit;
	}
	void setConstraintRowSolverLowerLimit(SingleConstraintRowSolver rowSolver)
	{
		m_resolveSingleConstraintRowLowerLimit = rowSolver;
	}



	///Various implementations of solving a single constraint row using a generic equality constraint, using scalar reference, SSE2 or SSE4
	SingleConstraintRowSolver getScalarConstraintRowSolverGeneric();
	SingleConstraintRowSolver getSSE2ConstraintRowSolverGeneric();
	SingleConstraintRowSolver getSSE4_1ConstraintRowSolverGeneric();

	///Various implementations of solving a single constraint row using an inequality (lower limit) constraint, using scalar reference, SSE2 or SSE4
	SingleConstraintRowSolver getScalarConstraintRowSolverLowerLimit();
	SingleConstraintRowSolver getSSE2ConstraintRowSolverLowerLimit();
	SingleConstraintRowSolver getSSE4_1ConstraintRowSolverLowerLimit();
	SolverAnalyticsData m_analyticsData;
};

#endif  //DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H
