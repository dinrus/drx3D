/* ====== Overview of the Deformable Algorithm ====== */

/*
A single step of the deformable body simulation contains the following main components:
Call internalStepSimulation multiple times, to achieve 240Hz (4 steps of 60Hz).
1. Deformable maintaintenance of rest lengths and volume preservation. Forces only depend on position: Update velocity to a temporary state v_{n+1}^* = v_n + explicit_force * dt / mass, where explicit forces include gravity and elastic forces.
2. Detect discrete collisions between rigid and deformable bodies at position x_{n+1}^* = x_n + dt * v_{n+1}^*.

3a. Solve all constraints, including LCP. Contact, position correction due to numerical drift, friction, and anchors for deformable.

3b. 5 Newton steps (multiple step). Conjugent Gradient solves linear system. Deformable Damping: Then velocities of deformable bodies v_{n+1} are solved in
        M(v_{n+1} - v_{n+1}^*) = damping_force * dt / mass,
   by a conjugate gradient solver, where the damping force is implicit and depends on v_{n+1}.
   Make sure contact constraints are not violated in step b by performing velocity projections as in the paper by Baraff and Witkin https://www.cs.cmu.edu/~baraff/papers/sig98.pdf. Dynamic frictions are treated as a force and added to the rhs of the CG solve, whereas static frictions are treated as constraints similar to contact.
4. Position is updated via x_{n+1} = x_n + dt * v_{n+1}.


The algorithm also closely resembles the one in http://physbam.stanford.edu/~fedkiw/papers/stanford2008-03.pdf
 */

#include <stdio.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/DeformableBodyInplaceSolverIslandCallback.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>

DeformableMultiBodyDynamicsWorld::DeformableMultiBodyDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, DeformableMultiBodyConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration, DeformableBodySolver* deformableBodySolver)
	: MultiBodyDynamicsWorld(dispatcher, pairCache, (MultiBodyConstraintSolver*)constraintSolver, collisionConfiguration),
	  m_deformableBodySolver(deformableBodySolver),
	  m_solverCallback(0)
{
	m_drawFlags = fDrawFlags::Std;
	m_drawNodeTree = true;
	m_drawFaceTree = false;
	m_drawClusterTree = false;
	m_sbi.m_broadphase = pairCache;
	m_sbi.m_dispatcher = dispatcher;
	m_sbi.m_sparsesdf.Initialize();
	m_sbi.m_sparsesdf.setDefaultVoxelsz(0.005);
	m_sbi.m_sparsesdf.Reset();

	m_sbi.air_density = (Scalar)1.2;
	m_sbi.water_density = 0;
	m_sbi.water_offset = 0;
	m_sbi.water_normal = Vec3(0, 0, 0);
	m_sbi.m_gravity.setVal(0, -9.8, 0);
	m_internalTime = 0.0;
	m_implicit = false;
	m_lineSearch = false;
	m_useProjection = false;
	m_ccdIterations = 5;
	m_solverDeformableBodyIslandCallback = new DeformableBodyInplaceSolverIslandCallback(constraintSolver, dispatcher);
}

DeformableMultiBodyDynamicsWorld::~DeformableMultiBodyDynamicsWorld()
{
	delete m_solverDeformableBodyIslandCallback;
}

void DeformableMultiBodyDynamicsWorld::internalSingleStepSimulation(Scalar timeStep)
{
	DRX3D_PROFILE("internalSingleStepSimulation");
	if (0 != m_internalPreTickCallback)
	{
		(*m_internalPreTickCallback)(this, timeStep);
	}
	reinitialize(timeStep);

	// add gravity to velocity of rigid and multi bodys
	applyRigidBodyGravity(timeStep);

	///apply gravity and explicit force to velocity, predict motion
	predictUnconstraintMotion(timeStep);

	///perform collision detection that involves rigid/multi bodies
	MultiBodyDynamicsWorld::performDiscreteCollisionDetection();

	MultiBodyDynamicsWorld::calculateSimulationIslands();

	beforeSolverCallbacks(timeStep);

	// ///solve contact constraints and then deformable bodies momemtum equation
	solveConstraints(timeStep);

	afterSolverCallbacks(timeStep);

	performDeformableCollisionDetection();

	applyRepulsionForce(timeStep);

	performGeometricCollisions(timeStep);

	integrateTransforms(timeStep);

	///update vehicle simulation
	MultiBodyDynamicsWorld::updateActions(timeStep);

	updateActivationState(timeStep);
	// End solver-wise simulation step
	// ///////////////////////////////
}

void DeformableMultiBodyDynamicsWorld::performDeformableCollisionDetection()
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		m_softBodies[i]->m_softSoftCollision = true;
	}

	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		for (i32 j = i; j < m_softBodies.size(); ++j)
		{
			m_softBodies[i]->defaultCollisionHandler(m_softBodies[j]);
		}
	}

	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		m_softBodies[i]->m_softSoftCollision = false;
	}
}

void DeformableMultiBodyDynamicsWorld::updateActivationState(Scalar timeStep)
{
	for (i32 i = 0; i < m_softBodies.size(); i++)
	{
		SoftBody* psb = m_softBodies[i];
		psb->updateDeactivation(timeStep);
		if (psb->wantsSleeping())
		{
			if (psb->getActivationState() == ACTIVE_TAG)
				psb->setActivationState(WANTS_DEACTIVATION);
			if (psb->getActivationState() == ISLAND_SLEEPING)
			{
				psb->setZeroVelocity();
			}
		}
		else
		{
			if (psb->getActivationState() != DISABLE_DEACTIVATION)
				psb->setActivationState(ACTIVE_TAG);
		}
	}
	MultiBodyDynamicsWorld::updateActivationState(timeStep);
}

void DeformableMultiBodyDynamicsWorld::applyRepulsionForce(Scalar timeStep)
{
	DRX3D_PROFILE("DeformableMultiBodyDynamicsWorld::applyRepulsionForce");
	for (i32 i = 0; i < m_softBodies.size(); i++)
	{
		SoftBody* psb = m_softBodies[i];
		if (psb->isActive())
		{
			psb->applyRepulsionForce(timeStep, true);
		}
	}
}

void DeformableMultiBodyDynamicsWorld::performGeometricCollisions(Scalar timeStep)
{
	DRX3D_PROFILE("DeformableMultiBodyDynamicsWorld::performGeometricCollisions");
	// refit the BVH tree for CCD
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (psb->isActive())
		{
			m_softBodies[i]->updateFaceTree(true, false);
			m_softBodies[i]->updateNodeTree(true, false);
			for (i32 j = 0; j < m_softBodies[i]->m_faces.size(); ++j)
			{
				SoftBody::Face& f = m_softBodies[i]->m_faces[j];
				f.m_n0 = (f.m_n[1]->m_x - f.m_n[0]->m_x).cross(f.m_n[2]->m_x - f.m_n[0]->m_x);
			}
		}
	}

	// clear contact points & update DBVT
	for (i32 r = 0; r < m_ccdIterations; ++r)
	{
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (psb->isActive())
			{
				// clear contact points in the previous iteration
				psb->m_faceNodeContactsCCD.clear();

				// update m_q and normals for CCD calculation
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = psb->m_nodes[j].m_x + timeStep * psb->m_nodes[j].m_v;
				}
				for (i32 j = 0; j < psb->m_faces.size(); ++j)
				{
					SoftBody::Face& f = psb->m_faces[j];
					f.m_n1 = (f.m_n[1]->m_q - f.m_n[0]->m_q).cross(f.m_n[2]->m_q - f.m_n[0]->m_q);
					f.m_vn = (f.m_n[1]->m_v - f.m_n[0]->m_v).cross(f.m_n[2]->m_v - f.m_n[0]->m_v) * timeStep * timeStep;
				}
			}
		}

		// apply CCD to DoRegister new contact points
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			for (i32 j = i; j < m_softBodies.size(); ++j)
			{
				SoftBody* psb1 = m_softBodies[i];
				SoftBody* psb2 = m_softBodies[j];
				if (psb1->isActive() && psb2->isActive())
				{
					m_softBodies[i]->geometricCollisionHandler(m_softBodies[j]);
				}
			}
		}

		i32 penetration_count = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (psb->isActive())
			{
				penetration_count += psb->m_faceNodeContactsCCD.size();
				;
			}
		}
		if (penetration_count == 0)
		{
			break;
		}

		// apply inelastic impulse
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (psb->isActive())
			{
				psb->applyRepulsionForce(timeStep, false);
			}
		}
	}
}

void DeformableMultiBodyDynamicsWorld::softBodySelfCollision()
{
	DRX3D_PROFILE("DeformableMultiBodyDynamicsWorld::softBodySelfCollision");
	for (i32 i = 0; i < m_softBodies.size(); i++)
	{
		SoftBody* psb = m_softBodies[i];
		if (psb->isActive())
		{
			psb->defaultCollisionHandler(psb);
		}
	}
}

void DeformableMultiBodyDynamicsWorld::positionCorrection(Scalar timeStep)
{
	// correct the position of rigid bodies with temporary velocity generated from split impulse
	ContactSolverInfo infoGlobal;
	Vec3 zero(0, 0, 0);
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); ++i)
	{
		RigidBody* rb = m_nonStaticRigidBodies[i];
		//correct the position/orientation based on push/turn recovery
		Transform2 newTransform2;
		Vec3 pushVelocity = rb->getPushVelocity();
		Vec3 turnVelocity = rb->getTurnVelocity();
		if (pushVelocity[0] != 0.f || pushVelocity[1] != 0 || pushVelocity[2] != 0 || turnVelocity[0] != 0.f || turnVelocity[1] != 0 || turnVelocity[2] != 0)
		{
			Transform2Util::integrateTransform(rb->getWorldTransform(), pushVelocity, turnVelocity * infoGlobal.m_splitImpulseTurnErp, timeStep, newTransform2);
			rb->setWorldTransform(newTransform2);
			rb->setPushVelocity(zero);
			rb->setTurnVelocity(zero);
		}
	}
}

void DeformableMultiBodyDynamicsWorld::integrateTransforms(Scalar timeStep)
{
	DRX3D_PROFILE("integrateTransforms");
	positionCorrection(timeStep);
	MultiBodyDynamicsWorld::integrateTransforms(timeStep);
	m_deformableBodySolver->applyTransforms(timeStep);
}

void DeformableMultiBodyDynamicsWorld::solveConstraints(Scalar timeStep)
{
	DRX3D_PROFILE("DeformableMultiBodyDynamicsWorld::solveConstraints");
	// save v_{n+1}^* velocity after explicit forces
	m_deformableBodySolver->backupVelocity();

	// set up constraints among multibodies and between multibodies and deformable bodies
	setupConstraints();

	// solve contact constraints
	solveContactConstraints();

	// set up the directions in which the velocity does not change in the momentum solve
	if (m_useProjection)
		m_deformableBodySolver->setProjection();
	else
		m_deformableBodySolver->setLagrangeMultiplier();

	// for explicit scheme, m_backupVelocity = v_{n+1}^*
	// for implicit scheme, m_backupVelocity = v_n
	// Here, set dv = v_{n+1} - v_n for nodes in contact
	m_deformableBodySolver->setupDeformableSolve(m_implicit);

	// At this point, dv should be golden for nodes in contact
	// proceed to solve deformable momentum equation
	m_deformableBodySolver->solveDeformableConstraints(timeStep);
}

void DeformableMultiBodyDynamicsWorld::setupConstraints()
{
	// set up constraints between multibody and deformable bodies
	m_deformableBodySolver->setConstraints(m_solverInfo);

	// set up constraints among multibodies
	{
		sortConstraints();
		// setup the solver callback
		MultiBodyConstraint** sortedMultiBodyConstraints = m_sortedMultiBodyConstraints.size() ? &m_sortedMultiBodyConstraints[0] : 0;
		TypedConstraint** constraintsPtr = getNumConstraints() ? &m_sortedConstraints[0] : 0;
		m_solverDeformableBodyIslandCallback->setup(&m_solverInfo, constraintsPtr, m_sortedConstraints.size(), sortedMultiBodyConstraints, m_sortedMultiBodyConstraints.size(), getDebugDrawer());

		// build islands
		m_islandManager->buildIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld());
	}
}

void DeformableMultiBodyDynamicsWorld::sortConstraints()
{
	m_sortedConstraints.resize(m_constraints.size());
	i32 i;
	for (i = 0; i < getNumConstraints(); i++)
	{
		m_sortedConstraints[i] = m_constraints[i];
	}
	m_sortedConstraints.quickSort(SortConstraintOnIslandPredicate2());

	m_sortedMultiBodyConstraints.resize(m_multiBodyConstraints.size());
	for (i = 0; i < m_multiBodyConstraints.size(); i++)
	{
		m_sortedMultiBodyConstraints[i] = m_multiBodyConstraints[i];
	}
	m_sortedMultiBodyConstraints.quickSort(SortMultiBodyConstraintOnIslandPredicate());
}

void DeformableMultiBodyDynamicsWorld::solveContactConstraints()
{
	// process constraints on each island
	m_islandManager->processIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld(), m_solverDeformableBodyIslandCallback);

	// process deferred
	m_solverDeformableBodyIslandCallback->processConstraints();
	m_constraintSolver->allSolved(m_solverInfo, m_debugDrawer);

	// write joint feedback
	{
		for (i32 i = 0; i < this->m_multiBodies.size(); i++)
		{
			MultiBody* bod = m_multiBodies[i];

			bool isSleeping = false;

			if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
			{
				isSleeping = true;
			}
			for (i32 b = 0; b < bod->getNumLinks(); b++)
			{
				if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
					isSleeping = true;
			}

			if (!isSleeping)
			{
				//useless? they get resized in stepVelocities once again (AND DIFFERENTLY)
				m_scratch_r.resize(bod->getNumLinks() + 1);  //multidof? ("Y"s use it and it is used to store qdd)
				m_scratch_v.resize(bod->getNumLinks() + 1);
				m_scratch_m.resize(bod->getNumLinks() + 1);

				if (bod->internalNeedsJointFeedback())
				{
					if (!bod->isUsingRK4Integration())
					{
						if (bod->internalNeedsJointFeedback())
						{
							bool isConstraintPass = true;
							bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(m_solverInfo.m_timeStep, m_scratch_r, m_scratch_v, m_scratch_m, isConstraintPass,
																					  getSolverInfo().m_jointFeedbackInWorldSpace,
																					  getSolverInfo().m_jointFeedbackInJointFrame);
						}
					}
				}
			}
		}
	}

	for (i32 i = 0; i < this->m_multiBodies.size(); i++)
	{
		MultiBody* bod = m_multiBodies[i];
		bod->processDeltaVeeMultiDof2();
	}
}

void DeformableMultiBodyDynamicsWorld::addSoftBody(SoftBody* body, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	m_softBodies.push_back(body);

	// Set the soft body solver that will deal with this body
	// to be the world's solver
	body->setSoftBodySolver(m_deformableBodySolver);

	CollisionWorld::addCollisionObject(body,
										 collisionFilterGroup,
										 collisionFilterMask);
}

void DeformableMultiBodyDynamicsWorld::predictUnconstraintMotion(Scalar timeStep)
{
	DRX3D_PROFILE("predictUnconstraintMotion");
	MultiBodyDynamicsWorld::predictUnconstraintMotion(timeStep);
	m_deformableBodySolver->predictMotion(timeStep);
}

void DeformableMultiBodyDynamicsWorld::setGravity(const Vec3& gravity)
{
	DiscreteDynamicsWorld::setGravity(gravity);
	m_deformableBodySolver->setGravity(gravity);
}

void DeformableMultiBodyDynamicsWorld::reinitialize(Scalar timeStep)
{
	m_internalTime += timeStep;
	m_deformableBodySolver->setImplicit(m_implicit);
	m_deformableBodySolver->setLineSearch(m_lineSearch);
	m_deformableBodySolver->reinitialize(m_softBodies, timeStep);
	DispatcherInfo& dispatchInfo = MultiBodyDynamicsWorld::getDispatchInfo();
	dispatchInfo.m_timeStep = timeStep;
	dispatchInfo.m_stepCount = 0;
	dispatchInfo.m_debugDraw = MultiBodyDynamicsWorld::getDebugDrawer();
	MultiBodyDynamicsWorld::getSolverInfo().m_timeStep = timeStep;
	if (m_useProjection)
	{
		m_deformableBodySolver->m_useProjection = true;
		m_deformableBodySolver->setStrainLimiting(true);
		m_deformableBodySolver->setPreconditioner(DeformableBackwardEulerObjective::Mass_preconditioner);
	}
	else
	{
		m_deformableBodySolver->m_useProjection = false;
		m_deformableBodySolver->setStrainLimiting(false);
		m_deformableBodySolver->setPreconditioner(DeformableBackwardEulerObjective::KKT_preconditioner);
	}
}

void DeformableMultiBodyDynamicsWorld::debugDrawWorld()
{
	MultiBodyDynamicsWorld::debugDrawWorld();

	for (i32 i = 0; i < getSoftBodyArray().size(); i++)
	{
		SoftBody* psb = (SoftBody*)getSoftBodyArray()[i];
		{
			SoftBodyHelpers::DrawFrame(psb, getDebugDrawer());
			SoftBodyHelpers::Draw(psb, getDebugDrawer(), getDrawFlags());
		}
	}
}

void DeformableMultiBodyDynamicsWorld::applyRigidBodyGravity(Scalar timeStep)
{
	// Gravity is applied in stepSimulation and then cleared here and then applied here and then cleared here again
	// so that 1) gravity is applied to velocity before constraint solve and 2) gravity is applied in each substep
	// when there are multiple substeps
	MultiBodyDynamicsWorld::applyGravity();
	// integrate rigid body gravity
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); ++i)
	{
		RigidBody* rb = m_nonStaticRigidBodies[i];
		rb->integrateVelocities(timeStep);
	}

	// integrate multibody gravity
	{
		forwardKinematics();
		clearMultiBodyConstraintForces();
		{
			for (i32 i = 0; i < this->m_multiBodies.size(); i++)
			{
				MultiBody* bod = m_multiBodies[i];

				bool isSleeping = false;

				if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
				{
					isSleeping = true;
				}
				for (i32 b = 0; b < bod->getNumLinks(); b++)
				{
					if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
						isSleeping = true;
				}

				if (!isSleeping)
				{
					m_scratch_r.resize(bod->getNumLinks() + 1);
					m_scratch_v.resize(bod->getNumLinks() + 1);
					m_scratch_m.resize(bod->getNumLinks() + 1);
					bool isConstraintPass = false;
					{
						if (!bod->isUsingRK4Integration())
						{
							bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(m_solverInfo.m_timeStep,
																					  m_scratch_r, m_scratch_v, m_scratch_m, isConstraintPass,
																					  getSolverInfo().m_jointFeedbackInWorldSpace,
																					  getSolverInfo().m_jointFeedbackInJointFrame);
						}
						else
						{
							Assert(" RK4Integration is not supported");
						}
					}
				}
			}
		}
	}
	clearGravity();
}

void DeformableMultiBodyDynamicsWorld::clearGravity()
{
	DRX3D_PROFILE("MultiBody clearGravity");
	// clear rigid body gravity
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		if (body->isActive())
		{
			body->clearGravity();
		}
	}
	// clear multibody gravity
	for (i32 i = 0; i < this->m_multiBodies.size(); i++)
	{
		MultiBody* bod = m_multiBodies[i];

		bool isSleeping = false;

		if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
		{
			isSleeping = true;
		}
		for (i32 b = 0; b < bod->getNumLinks(); b++)
		{
			if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
				isSleeping = true;
		}

		if (!isSleeping)
		{
			bod->addBaseForce(-m_gravity * bod->getBaseMass());

			for (i32 j = 0; j < bod->getNumLinks(); ++j)
			{
				bod->addLinkForce(j, -m_gravity * bod->getLinkMass(j));
			}
		}
	}
}

void DeformableMultiBodyDynamicsWorld::beforeSolverCallbacks(Scalar timeStep)
{
	if (0 != m_internalTickCallback)
	{
		(*m_internalTickCallback)(this, timeStep);
	}

	if (0 != m_solverCallback)
	{
		(*m_solverCallback)(m_internalTime, this);
	}
}

void DeformableMultiBodyDynamicsWorld::afterSolverCallbacks(Scalar timeStep)
{
	if (0 != m_solverCallback)
	{
		(*m_solverCallback)(m_internalTime, this);
	}
}

void DeformableMultiBodyDynamicsWorld::addForce(SoftBody* psb, DeformableLagrangianForce* force)
{
	AlignedObjectArray<DeformableLagrangianForce*>& forces = *m_deformableBodySolver->getLagrangianForceArray();
	bool added = false;
	for (i32 i = 0; i < forces.size(); ++i)
	{
		if (forces[i]->getForceType() == force->getForceType())
		{
			forces[i]->addSoftBody(psb);
			added = true;
			break;
		}
	}
	if (!added)
	{
		force->addSoftBody(psb);
		force->setIndices(m_deformableBodySolver->getIndices());
		forces.push_back(force);
	}
}

void DeformableMultiBodyDynamicsWorld::removeForce(SoftBody* psb, DeformableLagrangianForce* force)
{
	AlignedObjectArray<DeformableLagrangianForce*>& forces = *m_deformableBodySolver->getLagrangianForceArray();
	i32 removed_index = -1;
	for (i32 i = 0; i < forces.size(); ++i)
	{
		if (forces[i]->getForceType() == force->getForceType())
		{
			forces[i]->removeSoftBody(psb);
			if (forces[i]->m_softBodies.size() == 0)
				removed_index = i;
			break;
		}
	}
	if (removed_index >= 0)
		forces.removeAtIndex(removed_index);
}

void DeformableMultiBodyDynamicsWorld::removeSoftBodyForce(SoftBody* psb)
{
	AlignedObjectArray<DeformableLagrangianForce*>& forces = *m_deformableBodySolver->getLagrangianForceArray();
	for (i32 i = 0; i < forces.size(); ++i)
	{
		forces[i]->removeSoftBody(psb);
	}
}

void DeformableMultiBodyDynamicsWorld::removeSoftBody(SoftBody* body)
{
	removeSoftBodyForce(body);
	m_softBodies.remove(body);
	CollisionWorld::removeCollisionObject(body);
	// force a reinitialize so that node indices get updated.
	m_deformableBodySolver->reinitialize(m_softBodies, Scalar(-1));
}

void DeformableMultiBodyDynamicsWorld::removeCollisionObject(CollisionObject2* collisionObject)
{
	SoftBody* body = SoftBody::upcast(collisionObject);
	if (body)
		removeSoftBody(body);
	else
		DiscreteDynamicsWorld::removeCollisionObject(collisionObject);
}

i32 DeformableMultiBodyDynamicsWorld::stepSimulation(Scalar timeStep, i32 maxSubSteps, Scalar fixedTimeStep)
{
	startProfiling(timeStep);

	i32 numSimulationSubSteps = 0;

	if (maxSubSteps)
	{
		//fixed timestep with interpolation
		m_fixedTimeStep = fixedTimeStep;
		m_localTime += timeStep;
		if (m_localTime >= fixedTimeStep)
		{
			numSimulationSubSteps = i32(m_localTime / fixedTimeStep);
			m_localTime -= numSimulationSubSteps * fixedTimeStep;
		}
	}
	else
	{
		//variable timestep
		fixedTimeStep = timeStep;
		m_localTime = m_latencyMotionStateInterpolation ? 0 : timeStep;
		m_fixedTimeStep = 0;
		if (FuzzyZero(timeStep))
		{
			numSimulationSubSteps = 0;
			maxSubSteps = 0;
		}
		else
		{
			numSimulationSubSteps = 1;
			maxSubSteps = 1;
		}
	}

	//process some debugging flags
	if (getDebugDrawer())
	{
		IDebugDraw* debugDrawer = getDebugDrawer();
		gDisableDeactivation = (debugDrawer->getDebugMode() & IDebugDraw::DBG_NoDeactivation) != 0;
	}
	if (numSimulationSubSteps)
	{
		//clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
		i32 clampedSimulationSteps = (numSimulationSubSteps > maxSubSteps) ? maxSubSteps : numSimulationSubSteps;

		saveKinematicState(fixedTimeStep * clampedSimulationSteps);

		for (i32 i = 0; i < clampedSimulationSteps; i++)
		{
			internalSingleStepSimulation(fixedTimeStep);
			synchronizeMotionStates();
		}
	}
	else
	{
		synchronizeMotionStates();
	}

	clearForces();

#ifndef DRX3D_NO_PROFILE
	CProfileManager::Increment_Frame_Counter();
#endif  //DRX3D_NO_PROFILE

	return numSimulationSubSteps;
}
