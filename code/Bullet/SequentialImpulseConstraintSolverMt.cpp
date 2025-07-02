
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolverMt.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

bool SequentialImpulseConstraintSolverMt::s_allowNestedParallelForLoops = false;  // some task schedulers don't like nested loops
i32 SequentialImpulseConstraintSolverMt::s_minimumContactManifoldsForBatching = 250;
i32 SequentialImpulseConstraintSolverMt::s_minBatchSize = 50;
i32 SequentialImpulseConstraintSolverMt::s_maxBatchSize = 100;
BatchedConstraints::BatchingMethod SequentialImpulseConstraintSolverMt::s_contactBatchingMethod = BatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_2D;
BatchedConstraints::BatchingMethod SequentialImpulseConstraintSolverMt::s_jointBatchingMethod = BatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_2D;

SequentialImpulseConstraintSolverMt::SequentialImpulseConstraintSolverMt()
{
	m_numFrictionDirections = 1;
	m_useBatching = false;
	m_useObsoleteJointConstraints = false;
}

SequentialImpulseConstraintSolverMt::~SequentialImpulseConstraintSolverMt()
{
}

void SequentialImpulseConstraintSolverMt::setupBatchedContactConstraints()
{
	DRX3D_PROFILE("setupBatchedContactConstraints");
	m_batchedContactConstraints.setup(&m_tmpSolverContactConstraintPool,
									  m_tmpSolverBodyPool,
									  s_contactBatchingMethod,
									  s_minBatchSize,
									  s_maxBatchSize,
									  &m_scratchMemory);
}

void SequentialImpulseConstraintSolverMt::setupBatchedJointConstraints()
{
	DRX3D_PROFILE("setupBatchedJointConstraints");
	m_batchedJointConstraints.setup(&m_tmpSolverNonContactConstraintPool,
									m_tmpSolverBodyPool,
									s_jointBatchingMethod,
									s_minBatchSize,
									s_maxBatchSize,
									&m_scratchMemory);
}

void SequentialImpulseConstraintSolverMt::internalSetupContactConstraints(i32 iContactConstraint, const ContactSolverInfo& infoGlobal)
{
	SolverConstraint& contactConstraint = m_tmpSolverContactConstraintPool[iContactConstraint];

	Vec3 rel_pos1;
	Vec3 rel_pos2;
	Scalar relaxation;

	i32 solverBodyIdA = contactConstraint.m_solverBodyIdA;
	i32 solverBodyIdB = contactConstraint.m_solverBodyIdB;

	SolverBody* solverBodyA = &m_tmpSolverBodyPool[solverBodyIdA];
	SolverBody* solverBodyB = &m_tmpSolverBodyPool[solverBodyIdB];

	RigidBody* colObj0 = solverBodyA->m_originalBody;
	RigidBody* colObj1 = solverBodyB->m_originalBody;

	ManifoldPoint& cp = *static_cast<ManifoldPoint*>(contactConstraint.m_originalContactPoint);

	const Vec3& pos1 = cp.getPositionWorldOnA();
	const Vec3& pos2 = cp.getPositionWorldOnB();

	rel_pos1 = pos1 - solverBodyA->getWorldTransform().getOrigin();
	rel_pos2 = pos2 - solverBodyB->getWorldTransform().getOrigin();

	Vec3 vel1;
	Vec3 vel2;

	solverBodyA->getVelocityInLocalPointNoDelta(rel_pos1, vel1);
	solverBodyB->getVelocityInLocalPointNoDelta(rel_pos2, vel2);

	Vec3 vel = vel1 - vel2;
	Scalar rel_vel = cp.m_normalWorldOnB.dot(vel);

	setupContactConstraint(contactConstraint, solverBodyIdA, solverBodyIdB, cp, infoGlobal, relaxation, rel_pos1, rel_pos2);

	// setup rolling friction constraints
	i32 rollingFrictionIndex = m_rollingFrictionIndexTable[iContactConstraint];
	if (rollingFrictionIndex >= 0)
	{
		SolverConstraint& spinningFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[rollingFrictionIndex];
		Assert(spinningFrictionConstraint.m_frictionIndex == iContactConstraint);
		setupTorsionalFrictionConstraint(spinningFrictionConstraint,
										 cp.m_normalWorldOnB,
										 solverBodyIdA,
										 solverBodyIdB,
										 cp,
										 cp.m_combinedSpinningFriction,
										 rel_pos1,
										 rel_pos2,
										 colObj0,
										 colObj1,
										 relaxation,
										 0.0f,
										 0.0f);
		Vec3 axis[2];
		PlaneSpace1(cp.m_normalWorldOnB, axis[0], axis[1]);
		axis[0].normalize();
		axis[1].normalize();

		applyAnisotropicFriction(colObj0, axis[0], CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);
		applyAnisotropicFriction(colObj1, axis[0], CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);
		applyAnisotropicFriction(colObj0, axis[1], CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);
		applyAnisotropicFriction(colObj1, axis[1], CollisionObject2::CF_ANISOTROPIC_ROLLING_FRICTION);
		// put the largest axis first
		if (axis[1].length2() > axis[0].length2())
		{
			Swap(axis[0], axis[1]);
		}
		const Scalar kRollingFrictionThreshold = 0.001f;
		for (i32 i = 0; i < 2; ++i)
		{
			i32 iRollingFric = rollingFrictionIndex + 1 + i;
			SolverConstraint& rollingFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[iRollingFric];
			Assert(rollingFrictionConstraint.m_frictionIndex == iContactConstraint);
			Vec3 dir = axis[i];
			if (dir.length() > kRollingFrictionThreshold)
			{
				setupTorsionalFrictionConstraint(rollingFrictionConstraint,
												 dir,
												 solverBodyIdA,
												 solverBodyIdB,
												 cp,
												 cp.m_combinedRollingFriction,
												 rel_pos1,
												 rel_pos2,
												 colObj0,
												 colObj1,
												 relaxation,
												 0.0f,
												 0.0f);
			}
			else
			{
				rollingFrictionConstraint.m_frictionIndex = -1;  // disable constraint
			}
		}
	}

	// setup friction constraints
	//	setupFrictionConstraint(solverConstraint, normalAxis, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal, desiredVelocity, cfmSlip);
	{
		///drx3D has several options to set the friction directions
		///By default, each contact has only a single friction direction that is recomputed automatically very frame
		///based on the relative linear velocity.
		///If the relative velocity it zero, it will automatically compute a friction direction.

		///You can also enable two friction directions, using the SOLVER_USE_2_FRICTION_DIRECTIONS.
		///In that case, the second friction direction will be orthogonal to both contact normal and first friction direction.
		///
		///If you choose SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION, then the friction will be independent from the relative projected velocity.
		///
		///The user can manually override the friction directions for certain contacts using a contact callback,
		///and set the cp.m_lateralFrictionInitialized to true
		///In that case, you can set the target relative motion in each friction direction (cp.m_contactMotion1 and cp.m_contactMotion2)
		///this will give a conveyor belt effect
		///
		SolverConstraint* frictionConstraint1 = &m_tmpSolverContactFrictionConstraintPool[contactConstraint.m_frictionIndex];
		Assert(frictionConstraint1->m_frictionIndex == iContactConstraint);

		SolverConstraint* frictionConstraint2 = NULL;
		if (infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS)
		{
			frictionConstraint2 = &m_tmpSolverContactFrictionConstraintPool[contactConstraint.m_frictionIndex + 1];
			Assert(frictionConstraint2->m_frictionIndex == iContactConstraint);
		}

		if (!(infoGlobal.m_solverMode & SOLVER_ENABLE_FRICTION_DIRECTION_CACHING) || !(cp.m_contactPointFlags & DRX3D_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED))
		{
			cp.m_lateralFrictionDir1 = vel - cp.m_normalWorldOnB * rel_vel;
			Scalar lat_rel_vel = cp.m_lateralFrictionDir1.length2();
			if (!(infoGlobal.m_solverMode & SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION) && lat_rel_vel > SIMD_EPSILON)
			{
				cp.m_lateralFrictionDir1 *= 1.f / Sqrt(lat_rel_vel);
				applyAnisotropicFriction(colObj0, cp.m_lateralFrictionDir1, CollisionObject2::CF_ANISOTROPIC_FRICTION);
				applyAnisotropicFriction(colObj1, cp.m_lateralFrictionDir1, CollisionObject2::CF_ANISOTROPIC_FRICTION);
				setupFrictionConstraint(*frictionConstraint1, cp.m_lateralFrictionDir1, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal);

				if (frictionConstraint2)
				{
					cp.m_lateralFrictionDir2 = cp.m_lateralFrictionDir1.cross(cp.m_normalWorldOnB);
					cp.m_lateralFrictionDir2.normalize();  //??
					applyAnisotropicFriction(colObj0, cp.m_lateralFrictionDir2, CollisionObject2::CF_ANISOTROPIC_FRICTION);
					applyAnisotropicFriction(colObj1, cp.m_lateralFrictionDir2, CollisionObject2::CF_ANISOTROPIC_FRICTION);
					setupFrictionConstraint(*frictionConstraint2, cp.m_lateralFrictionDir2, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal);
				}
			}
			else
			{
				PlaneSpace1(cp.m_normalWorldOnB, cp.m_lateralFrictionDir1, cp.m_lateralFrictionDir2);

				applyAnisotropicFriction(colObj0, cp.m_lateralFrictionDir1, CollisionObject2::CF_ANISOTROPIC_FRICTION);
				applyAnisotropicFriction(colObj1, cp.m_lateralFrictionDir1, CollisionObject2::CF_ANISOTROPIC_FRICTION);
				setupFrictionConstraint(*frictionConstraint1, cp.m_lateralFrictionDir1, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal);

				if (frictionConstraint2)
				{
					applyAnisotropicFriction(colObj0, cp.m_lateralFrictionDir2, CollisionObject2::CF_ANISOTROPIC_FRICTION);
					applyAnisotropicFriction(colObj1, cp.m_lateralFrictionDir2, CollisionObject2::CF_ANISOTROPIC_FRICTION);
					setupFrictionConstraint(*frictionConstraint2, cp.m_lateralFrictionDir2, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal);
				}

				if ((infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS) && (infoGlobal.m_solverMode & SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION))
				{
					cp.m_contactPointFlags |= DRX3D_CONTACT_FLAG_LATERAL_FRICTION_INITIALIZED;
				}
			}
		}
		else
		{
			setupFrictionConstraint(*frictionConstraint1, cp.m_lateralFrictionDir1, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal, cp.m_contactMotion1, cp.m_frictionCFM);
			if (frictionConstraint2)
			{
				setupFrictionConstraint(*frictionConstraint2, cp.m_lateralFrictionDir2, solverBodyIdA, solverBodyIdB, cp, rel_pos1, rel_pos2, colObj0, colObj1, relaxation, infoGlobal, cp.m_contactMotion2, cp.m_frictionCFM);
			}
		}
	}

	setFrictionConstraintImpulse(contactConstraint, solverBodyIdA, solverBodyIdB, cp, infoGlobal);
}

struct SetupContactConstraintsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;
	const ContactSolverInfo* m_infoGlobal;

	SetupContactConstraintsLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc, const ContactSolverInfo& infoGlobal)
	{
		m_solver = solver;
		m_bc = bc;
		m_infoGlobal = &infoGlobal;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("SetupContactConstraintsLoop");
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			for (i32 i = batch.begin; i < batch.end; ++i)
			{
				i32 iContact = m_bc->m_constraintIndices[i];
				m_solver->internalSetupContactConstraints(iContact, *m_infoGlobal);
			}
		}
	}
};

void SequentialImpulseConstraintSolverMt::setupAllContactConstraints(const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("setupAllContactConstraints");
	if (m_useBatching)
	{
		const BatchedConstraints& batchedCons = m_batchedContactConstraints;
		SetupContactConstraintsLoop loop(this, &batchedCons, infoGlobal);
		for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
		{
			i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
			const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
			i32 grainSize = 1;
			ParallelFor(phase.begin, phase.end, grainSize, loop);
		}
	}
	else
	{
		for (i32 i = 0; i < m_tmpSolverContactConstraintPool.size(); ++i)
		{
			internalSetupContactConstraints(i, infoGlobal);
		}
	}
}

i32 SequentialImpulseConstraintSolverMt::getOrInitSolverBodyThreadsafe(CollisionObject2& body, Scalar timeStep)
{
	//
	// getOrInitSolverBody is threadsafe only for a single thread per solver (with potentially multiple solvers)
	//
	// getOrInitSolverBodyThreadsafe -- attempts to be fully threadsafe (however may affect determinism)
	//
	i32 solverBodyId = -1;
	bool isRigidBodyType = RigidBody::upcast(&body) != NULL;
	if (isRigidBodyType && !body.isStaticOrKinematicObject())
	{
		// dynamic body
		// Dynamic bodies can only be in one island, so it's safe to write to the companionId
		solverBodyId = body.getCompanionId();
		if (solverBodyId < 0)
		{
			m_bodySolverArrayMutex.lock();
			// now that we have the lock, check again
			solverBodyId = body.getCompanionId();
			if (solverBodyId < 0)
			{
				solverBodyId = m_tmpSolverBodyPool.size();
				SolverBody& solverBody = m_tmpSolverBodyPool.expand();
				initSolverBody(&solverBody, &body, timeStep);
				body.setCompanionId(solverBodyId);
			}
			m_bodySolverArrayMutex.unlock();
		}
	}
	else if (isRigidBodyType && body.isKinematicObject())
	{
		//
		// NOTE: must test for kinematic before static because some kinematic objects also
		//   identify as "static"
		//
		// Kinematic bodies can be in multiple islands at once, so it is a
		// race condition to write to them, so we use an alternate method
		// to record the solverBodyId
		i32 uniqueId = body.getWorldArrayIndex();
		i32k INVALID_SOLVER_BODY_ID = -1;
		if (m_kinematicBodyUniqueIdToSolverBodyTable.size() <= uniqueId)
		{
			m_kinematicBodyUniqueIdToSolverBodyTableMutex.lock();
			// now that we have the lock, check again
			if (m_kinematicBodyUniqueIdToSolverBodyTable.size() <= uniqueId)
			{
				m_kinematicBodyUniqueIdToSolverBodyTable.resize(uniqueId + 1, INVALID_SOLVER_BODY_ID);
			}
			m_kinematicBodyUniqueIdToSolverBodyTableMutex.unlock();
		}
		solverBodyId = m_kinematicBodyUniqueIdToSolverBodyTable[uniqueId];
		// if no table entry yet,
		if (INVALID_SOLVER_BODY_ID == solverBodyId)
		{
			// need to acquire both locks
			m_kinematicBodyUniqueIdToSolverBodyTableMutex.lock();
			m_bodySolverArrayMutex.lock();
			// now that we have the lock, check again
			solverBodyId = m_kinematicBodyUniqueIdToSolverBodyTable[uniqueId];
			if (INVALID_SOLVER_BODY_ID == solverBodyId)
			{
				// create a table entry for this body
				solverBodyId = m_tmpSolverBodyPool.size();
				SolverBody& solverBody = m_tmpSolverBodyPool.expand();
				initSolverBody(&solverBody, &body, timeStep);
				m_kinematicBodyUniqueIdToSolverBodyTable[uniqueId] = solverBodyId;
			}
			m_bodySolverArrayMutex.unlock();
			m_kinematicBodyUniqueIdToSolverBodyTableMutex.unlock();
		}
	}
	else
	{
		// all fixed bodies (inf mass) get mapped to a single solver id
		if (m_fixedBodyId < 0)
		{
			m_bodySolverArrayMutex.lock();
			// now that we have the lock, check again
			if (m_fixedBodyId < 0)
			{
				m_fixedBodyId = m_tmpSolverBodyPool.size();
				SolverBody& fixedBody = m_tmpSolverBodyPool.expand();
				initSolverBody(&fixedBody, 0, timeStep);
			}
			m_bodySolverArrayMutex.unlock();
		}
		solverBodyId = m_fixedBodyId;
	}
	Assert(solverBodyId >= 0 && solverBodyId < m_tmpSolverBodyPool.size());
	return solverBodyId;
}

void SequentialImpulseConstraintSolverMt::internalCollectContactManifoldCachedInfo(ContactManifoldCachedInfo* cachedInfoArray, PersistentManifold** manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalCollectContactManifoldCachedInfo");
	for (i32 i = 0; i < numManifolds; ++i)
	{
		ContactManifoldCachedInfo* cachedInfo = &cachedInfoArray[i];
		PersistentManifold* manifold = manifoldPtr[i];
		CollisionObject2* colObj0 = (CollisionObject2*)manifold->getBody0();
		CollisionObject2* colObj1 = (CollisionObject2*)manifold->getBody1();

		i32 solverBodyIdA = getOrInitSolverBodyThreadsafe(*colObj0, infoGlobal.m_timeStep);
		i32 solverBodyIdB = getOrInitSolverBodyThreadsafe(*colObj1, infoGlobal.m_timeStep);

		cachedInfo->solverBodyIds[0] = solverBodyIdA;
		cachedInfo->solverBodyIds[1] = solverBodyIdB;
		cachedInfo->numTouchingContacts = 0;

		SolverBody* solverBodyA = &m_tmpSolverBodyPool[solverBodyIdA];
		SolverBody* solverBodyB = &m_tmpSolverBodyPool[solverBodyIdB];

		// A contact manifold between 2 static object should not exist!
		// check the collision flags of your objects if this assert fires.
		// Incorrectly set collision object flags can degrade performance in various ways.
		Assert(!m_tmpSolverBodyPool[solverBodyIdA].m_invMass.isZero() || !m_tmpSolverBodyPool[solverBodyIdB].m_invMass.isZero());

		i32 iContact = 0;
		for (i32 j = 0; j < manifold->getNumContacts(); j++)
		{
			ManifoldPoint& cp = manifold->getContactPoint(j);

			if (cp.getDistance() <= manifold->getContactProcessingThreshold())
			{
				cachedInfo->contactPoints[iContact] = &cp;
				cachedInfo->contactHasRollingFriction[iContact] = (cp.m_combinedRollingFriction > 0.f);
				iContact++;
			}
		}
		cachedInfo->numTouchingContacts = iContact;
	}
}

struct CollectContactManifoldCachedInfoLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	SequentialImpulseConstraintSolverMt::ContactManifoldCachedInfo* m_cachedInfoArray;
	PersistentManifold** m_manifoldPtr;
	const ContactSolverInfo* m_infoGlobal;

	CollectContactManifoldCachedInfoLoop(SequentialImpulseConstraintSolverMt* solver, SequentialImpulseConstraintSolverMt::ContactManifoldCachedInfo* cachedInfoArray, PersistentManifold** manifoldPtr, const ContactSolverInfo& infoGlobal)
	{
		m_solver = solver;
		m_cachedInfoArray = cachedInfoArray;
		m_manifoldPtr = manifoldPtr;
		m_infoGlobal = &infoGlobal;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalCollectContactManifoldCachedInfo(m_cachedInfoArray + iBegin, m_manifoldPtr + iBegin, iEnd - iBegin, *m_infoGlobal);
	}
};

void SequentialImpulseConstraintSolverMt::internalAllocContactConstraints(const ContactManifoldCachedInfo* cachedInfoArray, i32 numManifolds)
{
	DRX3D_PROFILE("internalAllocContactConstraints");
	// possibly parallel part
	for (i32 iManifold = 0; iManifold < numManifolds; ++iManifold)
	{
		const ContactManifoldCachedInfo& cachedInfo = cachedInfoArray[iManifold];
		i32 contactIndex = cachedInfo.contactIndex;
		i32 frictionIndex = contactIndex * m_numFrictionDirections;
		i32 rollingFrictionIndex = cachedInfo.rollingFrictionIndex;
		for (i32 i = 0; i < cachedInfo.numTouchingContacts; i++)
		{
			SolverConstraint& contactConstraint = m_tmpSolverContactConstraintPool[contactIndex];
			contactConstraint.m_solverBodyIdA = cachedInfo.solverBodyIds[0];
			contactConstraint.m_solverBodyIdB = cachedInfo.solverBodyIds[1];
			contactConstraint.m_originalContactPoint = cachedInfo.contactPoints[i];

			// allocate the friction constraints
			contactConstraint.m_frictionIndex = frictionIndex;
			for (i32 iDir = 0; iDir < m_numFrictionDirections; ++iDir)
			{
				SolverConstraint& frictionConstraint = m_tmpSolverContactFrictionConstraintPool[frictionIndex];
				frictionConstraint.m_frictionIndex = contactIndex;
				frictionIndex++;
			}

			// allocate rolling friction constraints
			if (cachedInfo.contactHasRollingFriction[i])
			{
				m_rollingFrictionIndexTable[contactIndex] = rollingFrictionIndex;
				// allocate 3 (although we may use only 2 sometimes)
				for (i32 i = 0; i < 3; i++)
				{
					m_tmpSolverContactRollingFrictionConstraintPool[rollingFrictionIndex].m_frictionIndex = contactIndex;
					rollingFrictionIndex++;
				}
			}
			else
			{
				// indicate there is no rolling friction for this contact point
				m_rollingFrictionIndexTable[contactIndex] = -1;
			}
			contactIndex++;
		}
	}
}

struct AllocContactConstraintsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const SequentialImpulseConstraintSolverMt::ContactManifoldCachedInfo* m_cachedInfoArray;

	AllocContactConstraintsLoop(SequentialImpulseConstraintSolverMt* solver, SequentialImpulseConstraintSolverMt::ContactManifoldCachedInfo* cachedInfoArray)
	{
		m_solver = solver;
		m_cachedInfoArray = cachedInfoArray;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalAllocContactConstraints(m_cachedInfoArray + iBegin, iEnd - iBegin);
	}
};

void SequentialImpulseConstraintSolverMt::allocAllContactConstraints(PersistentManifold** manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("allocAllContactConstraints");
	AlignedObjectArray<ContactManifoldCachedInfo> cachedInfoArray;  // = m_manifoldCachedInfoArray;
	cachedInfoArray.resizeNoInitialize(numManifolds);
	if (/* DISABLES CODE */ (false))
	{
		// sequential
		internalCollectContactManifoldCachedInfo(&cachedInfoArray[0], manifoldPtr, numManifolds, infoGlobal);
	}
	else
	{
		// may alter ordering of bodies which affects determinism
		CollectContactManifoldCachedInfoLoop loop(this, &cachedInfoArray[0], manifoldPtr, infoGlobal);
		i32 grainSize = 200;
		ParallelFor(0, numManifolds, grainSize, loop);
	}

	{
		// serial part
		i32 numContacts = 0;
		i32 numRollingFrictionConstraints = 0;
		for (i32 iManifold = 0; iManifold < numManifolds; ++iManifold)
		{
			ContactManifoldCachedInfo& cachedInfo = cachedInfoArray[iManifold];
			cachedInfo.contactIndex = numContacts;
			cachedInfo.rollingFrictionIndex = numRollingFrictionConstraints;
			numContacts += cachedInfo.numTouchingContacts;
			for (i32 i = 0; i < cachedInfo.numTouchingContacts; ++i)
			{
				if (cachedInfo.contactHasRollingFriction[i])
				{
					numRollingFrictionConstraints += 3;
				}
			}
		}
		{
			DRX3D_PROFILE("allocPools");
			if (m_tmpSolverContactConstraintPool.capacity() < numContacts)
			{
				// if we need to reallocate, reserve some extra so we don't have to reallocate again next frame
				i32 extraReserve = numContacts / 16;
				m_tmpSolverContactConstraintPool.reserve(numContacts + extraReserve);
				m_rollingFrictionIndexTable.reserve(numContacts + extraReserve);
				m_tmpSolverContactFrictionConstraintPool.reserve((numContacts + extraReserve) * m_numFrictionDirections);
				m_tmpSolverContactRollingFrictionConstraintPool.reserve(numRollingFrictionConstraints + extraReserve);
			}
			m_tmpSolverContactConstraintPool.resizeNoInitialize(numContacts);
			m_rollingFrictionIndexTable.resizeNoInitialize(numContacts);
			m_tmpSolverContactFrictionConstraintPool.resizeNoInitialize(numContacts * m_numFrictionDirections);
			m_tmpSolverContactRollingFrictionConstraintPool.resizeNoInitialize(numRollingFrictionConstraints);
		}
	}
	{
		AllocContactConstraintsLoop loop(this, &cachedInfoArray[0]);
		i32 grainSize = 200;
		ParallelFor(0, numManifolds, grainSize, loop);
	}
}

void SequentialImpulseConstraintSolverMt::convertContacts(PersistentManifold** manifoldPtr, i32 numManifolds, const ContactSolverInfo& infoGlobal)
{
	if (!m_useBatching)
	{
		SequentialImpulseConstraintSolver::convertContacts(manifoldPtr, numManifolds, infoGlobal);
		return;
	}
	DRX3D_PROFILE("convertContacts");
	if (numManifolds > 0)
	{
		if (m_fixedBodyId < 0)
		{
			m_fixedBodyId = m_tmpSolverBodyPool.size();
			SolverBody& fixedBody = m_tmpSolverBodyPool.expand();
			initSolverBody(&fixedBody, 0, infoGlobal.m_timeStep);
		}
		allocAllContactConstraints(manifoldPtr, numManifolds, infoGlobal);
		if (m_useBatching)
		{
			setupBatchedContactConstraints();
		}
		setupAllContactConstraints(infoGlobal);
	}
}

void SequentialImpulseConstraintSolverMt::internalInitMultipleJoints(TypedConstraint** constraints, i32 iBegin, i32 iEnd)
{
	DRX3D_PROFILE("internalInitMultipleJoints");
	for (i32 i = iBegin; i < iEnd; i++)
	{
		TypedConstraint* constraint = constraints[i];
		TypedConstraint::ConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
		if (constraint->isEnabled())
		{
			constraint->buildJacobian();
			constraint->internalSetAppliedImpulse(0.0f);
			JointFeedback* fb = constraint->getJointFeedback();
			if (fb)
			{
				fb->m_appliedForceBodyA.setZero();
				fb->m_appliedTorqueBodyA.setZero();
				fb->m_appliedForceBodyB.setZero();
				fb->m_appliedTorqueBodyB.setZero();
			}
			constraint->getInfo1(&info1);
		}
		else
		{
			info1.m_numConstraintRows = 0;
			info1.nub = 0;
		}
	}
}

struct InitJointsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	TypedConstraint** m_constraints;

	InitJointsLoop(SequentialImpulseConstraintSolverMt* solver, TypedConstraint** constraints)
	{
		m_solver = solver;
		m_constraints = constraints;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalInitMultipleJoints(m_constraints, iBegin, iEnd);
	}
};

void SequentialImpulseConstraintSolverMt::internalConvertMultipleJoints(const AlignedObjectArray<JointParams>& jointParamsArray, TypedConstraint** constraints, i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalConvertMultipleJoints");
	for (i32 i = iBegin; i < iEnd; ++i)
	{
		const JointParams& jointParams = jointParamsArray[i];
		i32 currentRow = jointParams.m_solverConstraint;
		if (currentRow != -1)
		{
			const TypedConstraint::ConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];
			Assert(currentRow < m_tmpSolverNonContactConstraintPool.size());
			Assert(info1.m_numConstraintRows > 0);

			SolverConstraint* currentConstraintRow = &m_tmpSolverNonContactConstraintPool[currentRow];
			TypedConstraint* constraint = constraints[i];

			convertJoint(currentConstraintRow, constraint, info1, jointParams.m_solverBodyA, jointParams.m_solverBodyB, infoGlobal);
		}
	}
}

struct ConvertJointsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const AlignedObjectArray<SequentialImpulseConstraintSolverMt::JointParams>& m_jointParamsArray;
	TypedConstraint** m_srcConstraints;
	const ContactSolverInfo& m_infoGlobal;

	ConvertJointsLoop(SequentialImpulseConstraintSolverMt* solver,
					  const AlignedObjectArray<SequentialImpulseConstraintSolverMt::JointParams>& jointParamsArray,
					  TypedConstraint** srcConstraints,
					  const ContactSolverInfo& infoGlobal) : m_jointParamsArray(jointParamsArray),
															   m_infoGlobal(infoGlobal)
	{
		m_solver = solver;
		m_srcConstraints = srcConstraints;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalConvertMultipleJoints(m_jointParamsArray, m_srcConstraints, iBegin, iEnd, m_infoGlobal);
	}
};

void SequentialImpulseConstraintSolverMt::convertJoints(TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal)
{
	if (!m_useBatching)
	{
		SequentialImpulseConstraintSolver::convertJoints(constraints, numConstraints, infoGlobal);
		return;
	}
	DRX3D_PROFILE("convertJoints");
	bool parallelJointSetup = true;
	m_tmpConstraintSizesPool.resizeNoInitialize(numConstraints);
	if (parallelJointSetup)
	{
		InitJointsLoop loop(this, constraints);
		i32 grainSize = 40;
		ParallelFor(0, numConstraints, grainSize, loop);
	}
	else
	{
		internalInitMultipleJoints(constraints, 0, numConstraints);
	}

	i32 totalNumRows = 0;
	AlignedObjectArray<JointParams> jointParamsArray;
	jointParamsArray.resizeNoInitialize(numConstraints);

	//calculate the total number of contraint rows
	for (i32 i = 0; i < numConstraints; i++)
	{
		TypedConstraint* constraint = constraints[i];

		JointParams& params = jointParamsArray[i];
		const TypedConstraint::ConstraintInfo1& info1 = m_tmpConstraintSizesPool[i];

		if (info1.m_numConstraintRows)
		{
			params.m_solverConstraint = totalNumRows;
			params.m_solverBodyA = getOrInitSolverBody(constraint->getRigidBodyA(), infoGlobal.m_timeStep);
			params.m_solverBodyB = getOrInitSolverBody(constraint->getRigidBodyB(), infoGlobal.m_timeStep);
		}
		else
		{
			params.m_solverConstraint = -1;
		}
		totalNumRows += info1.m_numConstraintRows;
	}
	m_tmpSolverNonContactConstraintPool.resizeNoInitialize(totalNumRows);

	///setup the SolverConstraints
	if (parallelJointSetup)
	{
		ConvertJointsLoop loop(this, jointParamsArray, constraints, infoGlobal);
		i32 grainSize = 20;
		ParallelFor(0, numConstraints, grainSize, loop);
	}
	else
	{
		internalConvertMultipleJoints(jointParamsArray, constraints, 0, numConstraints, infoGlobal);
	}
	setupBatchedJointConstraints();
}

void SequentialImpulseConstraintSolverMt::internalConvertBodies(CollisionObject2** bodies, i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalConvertBodies");
	for (i32 i = iBegin; i < iEnd; i++)
	{
		CollisionObject2* obj = bodies[i];
		obj->setCompanionId(i);
		SolverBody& solverBody = m_tmpSolverBodyPool[i];
		initSolverBody(&solverBody, obj, infoGlobal.m_timeStep);

		RigidBody* body = RigidBody::upcast(obj);
		if (body && body->getInvMass())
		{
			Vec3 gyroForce(0, 0, 0);
			if (body->getFlags() & DRX3D_ENABLE_GYROSCOPIC_FORCE_EXPLICIT)
			{
				gyroForce = body->computeGyroscopicForceExplicit(infoGlobal.m_maxGyroscopicForce);
				solverBody.m_externalTorqueImpulse -= gyroForce * body->getInvInertiaTensorWorld() * infoGlobal.m_timeStep;
			}
			if (body->getFlags() & DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_WORLD)
			{
				gyroForce = body->computeGyroscopicImpulseImplicit_World(infoGlobal.m_timeStep);
				solverBody.m_externalTorqueImpulse += gyroForce;
			}
			if (body->getFlags() & DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY)
			{
				gyroForce = body->computeGyroscopicImpulseImplicit_Body(infoGlobal.m_timeStep);
				solverBody.m_externalTorqueImpulse += gyroForce;
			}
		}
	}
}

struct ConvertBodiesLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	CollisionObject2** m_bodies;
	i32 m_numBodies;
	const ContactSolverInfo& m_infoGlobal;

	ConvertBodiesLoop(SequentialImpulseConstraintSolverMt* solver,
					  CollisionObject2** bodies,
					  i32 numBodies,
					  const ContactSolverInfo& infoGlobal) : m_infoGlobal(infoGlobal)
	{
		m_solver = solver;
		m_bodies = bodies;
		m_numBodies = numBodies;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalConvertBodies(m_bodies, iBegin, iEnd, m_infoGlobal);
	}
};

void SequentialImpulseConstraintSolverMt::convertBodies(CollisionObject2** bodies, i32 numBodies, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("convertBodies");
	m_kinematicBodyUniqueIdToSolverBodyTable.resize(0);

	m_tmpSolverBodyPool.resizeNoInitialize(numBodies + 1);

	m_fixedBodyId = numBodies;
	{
		SolverBody& fixedBody = m_tmpSolverBodyPool[m_fixedBodyId];
		initSolverBody(&fixedBody, NULL, infoGlobal.m_timeStep);
	}

	bool parallelBodySetup = true;
	if (parallelBodySetup)
	{
		ConvertBodiesLoop loop(this, bodies, numBodies, infoGlobal);
		i32 grainSize = 40;
		ParallelFor(0, numBodies, grainSize, loop);
	}
	else
	{
		internalConvertBodies(bodies, 0, numBodies, infoGlobal);
	}
}

Scalar SequentialImpulseConstraintSolverMt::solveGroupCacheFriendlySetup(
	CollisionObject2** bodies,
	i32 numBodies,
	PersistentManifold** manifoldPtr,
	i32 numManifolds,
	TypedConstraint** constraints,
	i32 numConstraints,
	const ContactSolverInfo& infoGlobal,
	IDebugDraw* debugDrawer)
{
	m_numFrictionDirections = (infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS) ? 2 : 1;
	m_useBatching = false;
	if (numManifolds >= s_minimumContactManifoldsForBatching &&
		(s_allowNestedParallelForLoops || !ThreadsAreRunning()))
	{
		m_useBatching = true;
		m_batchedContactConstraints.m_debugDrawer = debugDrawer;
		m_batchedJointConstraints.m_debugDrawer = debugDrawer;
	}
	SequentialImpulseConstraintSolver::solveGroupCacheFriendlySetup(bodies,
																	  numBodies,
																	  manifoldPtr,
																	  numManifolds,
																	  constraints,
																	  numConstraints,
																	  infoGlobal,
																	  debugDrawer);
	return 0.0f;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleContactSplitPenetrationImpulseConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd)
{
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiCons = batchBegin; iiCons < batchEnd; ++iiCons)
	{
		i32 iCons = consIndices[iiCons];
		const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[iCons];
		SolverBody& bodyA = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA];
		SolverBody& bodyB = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB];
		Scalar residual = resolveSplitPenetrationImpulse(bodyA, bodyB, solveManifold);
		leastSquaresResidual += residual * residual;
	}
	return leastSquaresResidual;
}

struct ContactSplitPenetrationImpulseSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;

	ContactSplitPenetrationImpulseSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc)
	{
		m_solver = solver;
		m_bc = bc;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("ContactSplitPenetrationImpulseSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleContactSplitPenetrationImpulseConstraints(m_bc->m_constraintIndices, batch.begin, batch.end);
		}
		return sum;
	}
};

void SequentialImpulseConstraintSolverMt::solveGroupCacheFriendlySplitImpulseIterations(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	DRX3D_PROFILE("solveGroupCacheFriendlySplitImpulseIterations");
	if (infoGlobal.m_splitImpulse)
	{
		for (i32 iteration = 0; iteration < infoGlobal.m_numIterations; iteration++)
		{
			Scalar leastSquaresResidual = 0.f;
			if (m_useBatching)
			{
				const BatchedConstraints& batchedCons = m_batchedContactConstraints;
				ContactSplitPenetrationImpulseSolverLoop loop(this, &batchedCons);
				Scalar leastSquaresResidual = 0.f;
				for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
				{
					i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
					const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
					i32 grainSize = batchedCons.m_phaseGrainSize[iPhase];
					leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
				}
			}
			else
			{
				// non-batched
				leastSquaresResidual = resolveMultipleContactSplitPenetrationImpulseConstraints(m_orderTmpConstraintPool, 0, m_tmpSolverContactConstraintPool.size());
			}
			if (leastSquaresResidual <= infoGlobal.m_leastSquaresResidualThreshold || iteration >= (infoGlobal.m_numIterations - 1))
			{
#ifdef VERBOSE_RESIDUAL_PRINTF
				printf("residual = %f at iteration #%d\n", leastSquaresResidual, iteration);
#endif
				break;
			}
		}
	}
}

Scalar SequentialImpulseConstraintSolverMt::solveSingleIteration(i32 iteration, CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	if (!m_useBatching)
	{
		return SequentialImpulseConstraintSolver::solveSingleIteration(iteration, bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
	}
	DRX3D_PROFILE("solveSingleIterationMt");
	Scalar leastSquaresResidual = 0.f;

	if (infoGlobal.m_solverMode & SOLVER_RANDMIZE_ORDER)
	{
		if (1)  // uncomment this for a bit less random ((iteration & 7) == 0)
		{
			randomizeConstraintOrdering(iteration, infoGlobal.m_numIterations);
		}
	}

	{
		///solve all joint constraints
		leastSquaresResidual += resolveAllJointConstraints(iteration);

		if (iteration < infoGlobal.m_numIterations)
		{
			// this loop is only used for cone-twist constraints,
			// it would be nice to skip this loop if none of the constraints need it
			if (m_useObsoleteJointConstraints)
			{
				for (i32 j = 0; j < numConstraints; j++)
				{
					if (constraints[j]->isEnabled())
					{
						i32 bodyAid = getOrInitSolverBody(constraints[j]->getRigidBodyA(), infoGlobal.m_timeStep);
						i32 bodyBid = getOrInitSolverBody(constraints[j]->getRigidBodyB(), infoGlobal.m_timeStep);
						SolverBody& bodyA = m_tmpSolverBodyPool[bodyAid];
						SolverBody& bodyB = m_tmpSolverBodyPool[bodyBid];
						constraints[j]->solveConstraintObsolete(bodyA, bodyB, infoGlobal.m_timeStep);
					}
				}
			}

			if (infoGlobal.m_solverMode & SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS)
			{
				// solve all contact, contact-friction, and rolling friction constraints interleaved
				leastSquaresResidual += resolveAllContactConstraintsInterleaved();
			}
			else  //SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS
			{
				// don't interleave them
				// solve all contact constraints
				leastSquaresResidual += resolveAllContactConstraints();

				// solve all contact friction constraints
				leastSquaresResidual += resolveAllContactFrictionConstraints();

				// solve all rolling friction constraints
				leastSquaresResidual += resolveAllRollingFrictionConstraints();
			}
		}
	}
	return leastSquaresResidual;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleJointConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd, i32 iteration)
{
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiCons = batchBegin; iiCons < batchEnd; ++iiCons)
	{
		i32 iCons = consIndices[iiCons];
		const SolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[iCons];
		if (iteration < constraint.m_overrideNumSolverIterations)
		{
			SolverBody& bodyA = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
			SolverBody& bodyB = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
			Scalar residual = resolveSingleConstraintRowGeneric(bodyA, bodyB, constraint);
			leastSquaresResidual += residual * residual;
		}
	}
	return leastSquaresResidual;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleContactConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd)
{
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiCons = batchBegin; iiCons < batchEnd; ++iiCons)
	{
		i32 iCons = consIndices[iiCons];
		const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[iCons];
		SolverBody& bodyA = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA];
		SolverBody& bodyB = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB];
		Scalar residual = resolveSingleConstraintRowLowerLimit(bodyA, bodyB, solveManifold);
		leastSquaresResidual += residual * residual;
	}
	return leastSquaresResidual;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleContactFrictionConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd)
{
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiCons = batchBegin; iiCons < batchEnd; ++iiCons)
	{
		i32 iContact = consIndices[iiCons];
		Scalar totalImpulse = m_tmpSolverContactConstraintPool[iContact].m_appliedImpulse;

		// apply sliding friction
		if (totalImpulse > 0.0f)
		{
			i32 iBegin = iContact * m_numFrictionDirections;
			i32 iEnd = iBegin + m_numFrictionDirections;
			for (i32 iFriction = iBegin; iFriction < iEnd; ++iFriction)
			{
				SolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[iFriction++];
				Assert(solveManifold.m_frictionIndex == iContact);

				solveManifold.m_lowerLimit = -(solveManifold.m_friction * totalImpulse);
				solveManifold.m_upperLimit = solveManifold.m_friction * totalImpulse;

				SolverBody& bodyA = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA];
				SolverBody& bodyB = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB];
				Scalar residual = resolveSingleConstraintRowGeneric(bodyA, bodyB, solveManifold);
				leastSquaresResidual += residual * residual;
			}
		}
	}
	return leastSquaresResidual;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleContactRollingFrictionConstraints(const AlignedObjectArray<i32>& consIndices, i32 batchBegin, i32 batchEnd)
{
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiCons = batchBegin; iiCons < batchEnd; ++iiCons)
	{
		i32 iContact = consIndices[iiCons];
		i32 iFirstRollingFriction = m_rollingFrictionIndexTable[iContact];
		if (iFirstRollingFriction >= 0)
		{
			Scalar totalImpulse = m_tmpSolverContactConstraintPool[iContact].m_appliedImpulse;
			// apply rolling friction
			if (totalImpulse > 0.0f)
			{
				i32 iBegin = iFirstRollingFriction;
				i32 iEnd = iBegin + 3;
				for (i32 iRollingFric = iBegin; iRollingFric < iEnd; ++iRollingFric)
				{
					SolverConstraint& rollingFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[iRollingFric];
					if (rollingFrictionConstraint.m_frictionIndex != iContact)
					{
						break;
					}
					Scalar rollingFrictionMagnitude = rollingFrictionConstraint.m_friction * totalImpulse;
					if (rollingFrictionMagnitude > rollingFrictionConstraint.m_friction)
					{
						rollingFrictionMagnitude = rollingFrictionConstraint.m_friction;
					}

					rollingFrictionConstraint.m_lowerLimit = -rollingFrictionMagnitude;
					rollingFrictionConstraint.m_upperLimit = rollingFrictionMagnitude;

					Scalar residual = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdA], m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdB], rollingFrictionConstraint);
					leastSquaresResidual += residual * residual;
				}
			}
		}
	}
	return leastSquaresResidual;
}

Scalar SequentialImpulseConstraintSolverMt::resolveMultipleContactConstraintsInterleaved(const AlignedObjectArray<i32>& contactIndices,
																							 i32 batchBegin,
																							 i32 batchEnd)
{
	Scalar leastSquaresResidual = 0.f;
	i32 numPoolConstraints = m_tmpSolverContactConstraintPool.size();

	for (i32 iiCons = batchBegin; iiCons < batchEnd; iiCons++)
	{
		Scalar totalImpulse = 0;
		i32 iContact = contactIndices[iiCons];
		// apply penetration constraint
		{
			const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[iContact];
			Scalar residual = resolveSingleConstraintRowLowerLimit(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
			leastSquaresResidual += residual * residual;
			totalImpulse = solveManifold.m_appliedImpulse;
		}

		// apply sliding friction
		if (totalImpulse > 0.0f)
		{
			i32 iBegin = iContact * m_numFrictionDirections;
			i32 iEnd = iBegin + m_numFrictionDirections;
			for (i32 iFriction = iBegin; iFriction < iEnd; ++iFriction)
			{
				SolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[iFriction];
				Assert(solveManifold.m_frictionIndex == iContact);

				solveManifold.m_lowerLimit = -(solveManifold.m_friction * totalImpulse);
				solveManifold.m_upperLimit = solveManifold.m_friction * totalImpulse;

				SolverBody& bodyA = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA];
				SolverBody& bodyB = m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB];
				Scalar residual = resolveSingleConstraintRowGeneric(bodyA, bodyB, solveManifold);
				leastSquaresResidual += residual * residual;
			}
		}

		// apply rolling friction
		i32 iFirstRollingFriction = m_rollingFrictionIndexTable[iContact];
		if (totalImpulse > 0.0f && iFirstRollingFriction >= 0)
		{
			i32 iBegin = iFirstRollingFriction;
			i32 iEnd = iBegin + 3;
			for (i32 iRollingFric = iBegin; iRollingFric < iEnd; ++iRollingFric)
			{
				SolverConstraint& rollingFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[iRollingFric];
				if (rollingFrictionConstraint.m_frictionIndex != iContact)
				{
					break;
				}
				Scalar rollingFrictionMagnitude = rollingFrictionConstraint.m_friction * totalImpulse;
				if (rollingFrictionMagnitude > rollingFrictionConstraint.m_friction)
				{
					rollingFrictionMagnitude = rollingFrictionConstraint.m_friction;
				}

				rollingFrictionConstraint.m_lowerLimit = -rollingFrictionMagnitude;
				rollingFrictionConstraint.m_upperLimit = rollingFrictionMagnitude;

				Scalar residual = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdA], m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdB], rollingFrictionConstraint);
				leastSquaresResidual += residual * residual;
			}
		}
	}
	return leastSquaresResidual;
}

void SequentialImpulseConstraintSolverMt::randomizeBatchedConstraintOrdering(BatchedConstraints* batchedConstraints)
{
	BatchedConstraints& bc = *batchedConstraints;
	// randomize ordering of phases
	for (i32 ii = 1; ii < bc.m_phaseOrder.size(); ++ii)
	{
		i32 iSwap = RandInt2(ii + 1);
		bc.m_phaseOrder.swap(ii, iSwap);
	}

	// for each batch,
	for (i32 iBatch = 0; iBatch < bc.m_batches.size(); ++iBatch)
	{
		// randomize ordering of constraints within the batch
		const BatchedConstraints::Range& batch = bc.m_batches[iBatch];
		for (i32 iiCons = batch.begin; iiCons < batch.end; ++iiCons)
		{
			i32 iSwap = batch.begin + RandInt2(iiCons - batch.begin + 1);
			Assert(iSwap >= batch.begin && iSwap < batch.end);
			bc.m_constraintIndices.swap(iiCons, iSwap);
		}
	}
}

void SequentialImpulseConstraintSolverMt::randomizeConstraintOrdering(i32 iteration, i32 numIterations)
{
	// randomize ordering of joint constraints
	randomizeBatchedConstraintOrdering(&m_batchedJointConstraints);

	//contact/friction constraints are not solved more than numIterations
	if (iteration < numIterations)
	{
		randomizeBatchedConstraintOrdering(&m_batchedContactConstraints);
	}
}

struct JointSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;
	i32 m_iteration;

	JointSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc, i32 iteration)
	{
		m_solver = solver;
		m_bc = bc;
		m_iteration = iteration;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("JointSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleJointConstraints(m_bc->m_constraintIndices, batch.begin, batch.end, m_iteration);
		}
		return sum;
	}
};

Scalar SequentialImpulseConstraintSolverMt::resolveAllJointConstraints(i32 iteration)
{
	DRX3D_PROFILE("resolveAllJointConstraints");
	const BatchedConstraints& batchedCons = m_batchedJointConstraints;
	JointSolverLoop loop(this, &batchedCons, iteration);
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
	{
		i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
		const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
		i32 grainSize = 1;
		leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
	}
	return leastSquaresResidual;
}

struct ContactSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;

	ContactSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc)
	{
		m_solver = solver;
		m_bc = bc;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("ContactSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleContactConstraints(m_bc->m_constraintIndices, batch.begin, batch.end);
		}
		return sum;
	}
};

Scalar SequentialImpulseConstraintSolverMt::resolveAllContactConstraints()
{
	DRX3D_PROFILE("resolveAllContactConstraints");
	const BatchedConstraints& batchedCons = m_batchedContactConstraints;
	ContactSolverLoop loop(this, &batchedCons);
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
	{
		i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
		const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
		i32 grainSize = batchedCons.m_phaseGrainSize[iPhase];
		leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
	}
	return leastSquaresResidual;
}

struct ContactFrictionSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;

	ContactFrictionSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc)
	{
		m_solver = solver;
		m_bc = bc;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("ContactFrictionSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleContactFrictionConstraints(m_bc->m_constraintIndices, batch.begin, batch.end);
		}
		return sum;
	}
};

Scalar SequentialImpulseConstraintSolverMt::resolveAllContactFrictionConstraints()
{
	DRX3D_PROFILE("resolveAllContactFrictionConstraints");
	const BatchedConstraints& batchedCons = m_batchedContactConstraints;
	ContactFrictionSolverLoop loop(this, &batchedCons);
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
	{
		i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
		const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
		i32 grainSize = batchedCons.m_phaseGrainSize[iPhase];
		leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
	}
	return leastSquaresResidual;
}

struct InterleavedContactSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;

	InterleavedContactSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc)
	{
		m_solver = solver;
		m_bc = bc;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("InterleavedContactSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleContactConstraintsInterleaved(m_bc->m_constraintIndices, batch.begin, batch.end);
		}
		return sum;
	}
};

Scalar SequentialImpulseConstraintSolverMt::resolveAllContactConstraintsInterleaved()
{
	DRX3D_PROFILE("resolveAllContactConstraintsInterleaved");
	const BatchedConstraints& batchedCons = m_batchedContactConstraints;
	InterleavedContactSolverLoop loop(this, &batchedCons);
	Scalar leastSquaresResidual = 0.f;
	for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
	{
		i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
		const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
		i32 grainSize = 1;
		leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
	}
	return leastSquaresResidual;
}

struct ContactRollingFrictionSolverLoop : public IParallelSumBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const BatchedConstraints* m_bc;

	ContactRollingFrictionSolverLoop(SequentialImpulseConstraintSolverMt* solver, const BatchedConstraints* bc)
	{
		m_solver = solver;
		m_bc = bc;
	}
	Scalar sumLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		DRX3D_PROFILE("ContactFrictionSolverLoop");
		Scalar sum = 0;
		for (i32 iBatch = iBegin; iBatch < iEnd; ++iBatch)
		{
			const BatchedConstraints::Range& batch = m_bc->m_batches[iBatch];
			sum += m_solver->resolveMultipleContactRollingFrictionConstraints(m_bc->m_constraintIndices, batch.begin, batch.end);
		}
		return sum;
	}
};

Scalar SequentialImpulseConstraintSolverMt::resolveAllRollingFrictionConstraints()
{
	DRX3D_PROFILE("resolveAllRollingFrictionConstraints");
	Scalar leastSquaresResidual = 0.f;
	//
	// We do not generate batches for rolling friction constraints. We assume that
	// one of two cases is true:
	//
	//  1. either most bodies in the simulation have rolling friction, in which case we can use the
	//     batches for contacts and use a lookup table to translate contact indices to rolling friction
	//     (ignoring any contact indices that don't map to a rolling friction constraint). As long as
	//     most contacts have a corresponding rolling friction constraint, this should parallelize well.
	//
	//  -OR-
	//
	//  2. few bodies in the simulation have rolling friction, so it is not worth trying to use the
	//     batches from contacts as most of the contacts won't have corresponding rolling friction
	//     constraints and most threads would end up doing very little work. Most of the time would
	//     go to threading overhead, so we don't bother with threading.
	//
	i32 numRollingFrictionPoolConstraints = m_tmpSolverContactRollingFrictionConstraintPool.size();
	if (numRollingFrictionPoolConstraints >= m_tmpSolverContactConstraintPool.size())
	{
		// use batching if there are many rolling friction constraints
		const BatchedConstraints& batchedCons = m_batchedContactConstraints;
		ContactRollingFrictionSolverLoop loop(this, &batchedCons);
		Scalar leastSquaresResidual = 0.f;
		for (i32 iiPhase = 0; iiPhase < batchedCons.m_phases.size(); ++iiPhase)
		{
			i32 iPhase = batchedCons.m_phaseOrder[iiPhase];
			const BatchedConstraints::Range& phase = batchedCons.m_phases[iPhase];
			i32 grainSize = 1;
			leastSquaresResidual += ParallelSum(phase.begin, phase.end, grainSize, loop);
		}
	}
	else
	{
		// no batching, also ignores SOLVER_RANDMIZE_ORDER
		for (i32 j = 0; j < numRollingFrictionPoolConstraints; j++)
		{
			SolverConstraint& rollingFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[j];
			if (rollingFrictionConstraint.m_frictionIndex >= 0)
			{
				Scalar totalImpulse = m_tmpSolverContactConstraintPool[rollingFrictionConstraint.m_frictionIndex].m_appliedImpulse;
				if (totalImpulse > 0.0f)
				{
					Scalar rollingFrictionMagnitude = rollingFrictionConstraint.m_friction * totalImpulse;
					if (rollingFrictionMagnitude > rollingFrictionConstraint.m_friction)
						rollingFrictionMagnitude = rollingFrictionConstraint.m_friction;

					rollingFrictionConstraint.m_lowerLimit = -rollingFrictionMagnitude;
					rollingFrictionConstraint.m_upperLimit = rollingFrictionMagnitude;

					Scalar residual = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdA], m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdB], rollingFrictionConstraint);
					leastSquaresResidual += residual * residual;
				}
			}
		}
	}
	return leastSquaresResidual;
}

void SequentialImpulseConstraintSolverMt::internalWriteBackContacts(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalWriteBackContacts");
	writeBackContacts(iBegin, iEnd, infoGlobal);
	//for ( i32 iContact = iBegin; iContact < iEnd; ++iContact)
	//{
	//    const SolverConstraint& contactConstraint = m_tmpSolverContactConstraintPool[ iContact ];
	//    ManifoldPoint* pt = (ManifoldPoint*) contactConstraint.m_originalContactPoint;
	//    Assert( pt );
	//    pt->m_appliedImpulse = contactConstraint.m_appliedImpulse;
	//    pt->m_appliedImpulseLateral1 = m_tmpSolverContactFrictionConstraintPool[ contactConstraint.m_frictionIndex ].m_appliedImpulse;
	//    if ( m_numFrictionDirections == 2 )
	//    {
	//        pt->m_appliedImpulseLateral2 = m_tmpSolverContactFrictionConstraintPool[ contactConstraint.m_frictionIndex + 1 ].m_appliedImpulse;
	//    }
	//}
}

void SequentialImpulseConstraintSolverMt::internalWriteBackJoints(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalWriteBackJoints");
	writeBackJoints(iBegin, iEnd, infoGlobal);
}

void SequentialImpulseConstraintSolverMt::internalWriteBackBodies(i32 iBegin, i32 iEnd, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("internalWriteBackBodies");
	writeBackBodies(iBegin, iEnd, infoGlobal);
}

struct WriteContactPointsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const ContactSolverInfo* m_infoGlobal;

	WriteContactPointsLoop(SequentialImpulseConstraintSolverMt* solver, const ContactSolverInfo& infoGlobal)
	{
		m_solver = solver;
		m_infoGlobal = &infoGlobal;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalWriteBackContacts(iBegin, iEnd, *m_infoGlobal);
	}
};

struct WriteJointsLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const ContactSolverInfo* m_infoGlobal;

	WriteJointsLoop(SequentialImpulseConstraintSolverMt* solver, const ContactSolverInfo& infoGlobal)
	{
		m_solver = solver;
		m_infoGlobal = &infoGlobal;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalWriteBackJoints(iBegin, iEnd, *m_infoGlobal);
	}
};

struct WriteBodiesLoop : public IParallelForBody
{
	SequentialImpulseConstraintSolverMt* m_solver;
	const ContactSolverInfo* m_infoGlobal;

	WriteBodiesLoop(SequentialImpulseConstraintSolverMt* solver, const ContactSolverInfo& infoGlobal)
	{
		m_solver = solver;
		m_infoGlobal = &infoGlobal;
	}
	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		m_solver->internalWriteBackBodies(iBegin, iEnd, *m_infoGlobal);
	}
};

Scalar SequentialImpulseConstraintSolverMt::solveGroupCacheFriendlyFinish(CollisionObject2** bodies, i32 numBodies, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("solveGroupCacheFriendlyFinish");

	if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
	{
		WriteContactPointsLoop loop(this, infoGlobal);
		i32 grainSize = 500;
		ParallelFor(0, m_tmpSolverContactConstraintPool.size(), grainSize, loop);
	}

	{
		WriteJointsLoop loop(this, infoGlobal);
		i32 grainSize = 400;
		ParallelFor(0, m_tmpSolverNonContactConstraintPool.size(), grainSize, loop);
	}
	{
		WriteBodiesLoop loop(this, infoGlobal);
		i32 grainSize = 100;
		ParallelFor(0, m_tmpSolverBodyPool.size(), grainSize, loop);
	}

	m_tmpSolverContactConstraintPool.resizeNoInitialize(0);
	m_tmpSolverNonContactConstraintPool.resizeNoInitialize(0);
	m_tmpSolverContactFrictionConstraintPool.resizeNoInitialize(0);
	m_tmpSolverContactRollingFrictionConstraintPool.resizeNoInitialize(0);

	m_tmpSolverBodyPool.resizeNoInitialize(0);
	return 0.f;
}
