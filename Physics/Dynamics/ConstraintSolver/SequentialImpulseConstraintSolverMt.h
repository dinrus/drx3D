#ifndef DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_MT_H
#define DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_MT_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/BatchedConstraints.h>
#include <drx3D/Maths/Linear/Threads.h>

///
/// SequentialImpulseConstraintSolverMt
///
///  A multithreaded variant of the sequential impulse constraint solver. The constraints to be solved are grouped into
///  batches and phases where each batch of constraints within a given phase can be solved in parallel with the rest.
///  Ideally we want as few phases as possible, and each phase should have many batches, and all of the batches should
///  have about the same number of constraints.
///  This method works best on a large island of many constraints.
///
///  Supports all of the features of the normal sequential impulse solver such as:
///    - split penetration impulse
///    - rolling friction
///    - interleaving constraints
///    - warmstarting
///    - 2 friction directions
///    - randomized constraint ordering
///    - early termination when leastSquaresResidualThreshold is satisfied
///
///  When the SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS flag is enabled, unlike the normal SequentialImpulse solver,
///  the rolling friction is interleaved as well.
///  Interleaving the contact penetration constraints with friction reduces the number of parallel loops that need to be done,
///  which reduces threading overhead so it can be a performance win, however, it does seem to produce a less stable simulation,
///  at least on stacks of blocks.
///
///  When the SOLVER_RANDMIZE_ORDER flag is enabled, the ordering of phases, and the ordering of constraints within each batch
///  is randomized, however it does not swap constraints between batches.
///  This is to avoid regenerating the batches for each solver iteration which would be quite costly in performance.
///
///  Note that a non-zero leastSquaresResidualThreshold could possibly affect the determinism of the simulation
///  if the task scheduler's parallelSum operation is non-deterministic. The parallelSum operation can be non-deterministic
///  because floating point addition is not associative due to rounding errors.
///  The task scheduler can and should ensure that the result of any parallelSum operation is deterministic.
///
ATTRIBUTE_ALIGNED16(class)
SequentialImpulseConstraintSolverMt : public SequentialImpulseConstraintSolver
{
public:
	virtual void solveGroupCacheFriendlySplitImpulseIterations(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer) DRX3D_OVERRIDE;
	virtual Scalar solveSingleIteration(i32 iteration, CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer) DRX3D_OVERRIDE;
	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2 * *bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer) DRX3D_OVERRIDE;
	virtual Scalar solveGroupCacheFriendlyFinish(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal) DRX3D_OVERRIDE;

	// temp struct used to collect info from persistent manifolds into a cache-friendly struct using multiple threads
	struct ContactManifoldCachedInfo
	{
		static i32k MAX_NUM_CONTACT_POINTS = 4;

		i32 numTouchingContacts;
		i32 solverBodyIds[2];
		i32 contactIndex;
		i32 rollingFrictionIndex;
		bool contactHasRollingFriction[MAX_NUM_CONTACT_POINTS];
		ManifoldPoint* contactPoints[MAX_NUM_CONTACT_POINTS];
	};
	// temp struct used for setting up joint constraints in parallel
	struct JointParams
	{
		i32 m_solverConstraint;
		i32 m_solverBodyA;
		i32 m_solverBodyB;
	};
	void internalInitMultipleJoints(TypedConstraint * *constraints, i32 iBegin, i32 iEnd);
	void internalConvertMultipleJoints(const AlignedObjectArray<JointParams>& jointParamsArray, TypedConstraint** constraints, i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);

	// parameters to control batching
	static bool s_allowNestedParallelForLoops;        // whether to allow nested parallel operations
	static i32 s_minimumContactManifoldsForBatching;  // don't even try to batch if fewer manifolds than this
	static BatchedConstraints::BatchingMethod s_contactBatchingMethod;
	static BatchedConstraints::BatchingMethod s_jointBatchingMethod;
	static i32 s_minBatchSize;  // desired number of constraints per batch
	static i32 s_maxBatchSize;

protected:
	static i32k CACHE_LINE_SIZE = 64;

	BatchedConstraints m_batchedContactConstraints;
	BatchedConstraints m_batchedJointConstraints;
	i32 m_numFrictionDirections;
	bool m_useBatching;
	bool m_useObsoleteJointConstraints;
	AlignedObjectArray<ContactManifoldCachedInfo> m_manifoldCachedInfoArray;
	AlignedObjectArray<i32> m_rollingFrictionIndexTable;  // lookup table mapping contact index to rolling friction index
	SpinMutex m_bodySolverArrayMutex;
	char m_antiFalseSharingPadding[CACHE_LINE_SIZE];  // padding to keep mutexes in separate cachelines
	SpinMutex m_kinematicBodyUniqueIdToSolverBodyTableMutex;
	AlignedObjectArray<char> m_scratchMemory;

	virtual void randomizeConstraintOrdering(i32 iteration, i32 numIterations);
	virtual Scalar resolveAllJointConstraints(i32 iteration);
	virtual Scalar resolveAllContactConstraints();
	virtual Scalar resolveAllContactFrictionConstraints();
	virtual Scalar resolveAllContactConstraintsInterleaved();
	virtual Scalar resolveAllRollingFrictionConstraints();

	virtual void setupBatchedContactConstraints();
	virtual void setupBatchedJointConstraints();
	virtual void convertJoints(TypedConstraint * *constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal) DRX3D_OVERRIDE;
	virtual void convertContacts(PersistentManifold * *manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal) DRX3D_OVERRIDE;
	virtual void convertBodies(CollisionObject2 * *bodies, i32 numBodies, const ContactSolverInfo& infoGlobal) DRX3D_OVERRIDE;

	i32 getOrInitSolverBodyThreadsafe(CollisionObject2 & body, Scalar timeStep);
	void allocAllContactConstraints(PersistentManifold * *manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal);
	void setupAllContactConstraints(const ContactSolverInfo& infoGlobal);
	void randomizeBatchedConstraintOrdering(BatchedConstraints * batchedConstraints);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	SequentialImpulseConstraintSolverMt();
	virtual ~SequentialImpulseConstraintSolverMt();

	Scalar resolveMultipleJointConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd, i32 iteration);
	Scalar resolveMultipleContactConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd);
	Scalar resolveMultipleContactSplitPenetrationImpulseConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd);
	Scalar resolveMultipleContactFrictionConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd);
	Scalar resolveMultipleContactRollingFrictionConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd);
	Scalar resolveMultipleContactConstraintsInterleaved(const AlignedObjectArray<i32>& contactIndices, i32 batchBegin, i32 batchEnd);

	void internalCollectContactManifoldCachedInfo(ContactManifoldCachedInfo * cachedInfoArray, PersistentManifold * *manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal);
	void internalAllocContactConstraints(const ContactManifoldCachedInfo* cachedInfoArray, i32 numManifolds);
	void internalSetupContactConstraints(i32 iContactConstraint, const ContactSolverInfo& infoGlobal);
	void internalConvertBodies(CollisionObject2 * *bodies, i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	void internalWriteBackContacts(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	void internalWriteBackJoints(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
	void internalWriteBackBodies(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal);
};

#endif  //DRX3D_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_MT_H
