#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>

//collision detection
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Maths/Linear/Quickprof.h>

//rigidbody & constraints
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Point2PointConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/HingeConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ConeTwistConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SliderConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactConstraint.h>

#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

#include <drx3D/Physics/Dynamics/ActionInterface.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/MotionState.h>

#include <drx3D/Maths/Linear/Serializer.h>

#if 0
AlignedObjectArray<Vec3> debugContacts;
AlignedObjectArray<Vec3> debugNormals;
i32 startHit=2;
i32 firstHit=startHit;
#endif

SIMD_FORCE_INLINE i32 GetConstraintIslandId(const TypedConstraint* lhs)
{
	i32 islandId;

	const CollisionObject2& rcolObj0 = lhs->getRigidBodyA();
	const CollisionObject2& rcolObj1 = lhs->getRigidBodyB();
	islandId = rcolObj0.getIslandTag() >= 0 ? rcolObj0.getIslandTag() : rcolObj1.getIslandTag();
	return islandId;
}

class SortConstraintOnIslandPredicate
{
public:
	bool operator()(const TypedConstraint* lhs, const TypedConstraint* rhs) const
	{
		i32 rIslandId0, lIslandId0;
		rIslandId0 = GetConstraintIslandId(rhs);
		lIslandId0 = GetConstraintIslandId(lhs);
		return lIslandId0 < rIslandId0;
	}
};

struct InplaceSolverIslandCallback : public SimulationIslandManager::IslandCallback
{
	ContactSolverInfo* m_solverInfo;
	ConstraintSolver* m_solver;
	TypedConstraint** m_sortedConstraints;
	i32 m_numConstraints;
	IDebugDraw* m_debugDrawer;
	Dispatcher* m_dispatcher;

	AlignedObjectArray<CollisionObject2*> m_bodies;
	AlignedObjectArray<PersistentManifold*> m_manifolds;
	AlignedObjectArray<TypedConstraint*> m_constraints;

	InplaceSolverIslandCallback(
		ConstraintSolver* solver,
		StackAlloc* stackAlloc,
		Dispatcher* dispatcher)
		: m_solverInfo(NULL),
		  m_solver(solver),
		  m_sortedConstraints(NULL),
		  m_numConstraints(0),
		  m_debugDrawer(NULL),
		  m_dispatcher(dispatcher)
	{
	}

	InplaceSolverIslandCallback& operator=(InplaceSolverIslandCallback& other)
	{
		Assert(0);
		(void)other;
		return *this;
	}

	SIMD_FORCE_INLINE void setup(ContactSolverInfo* solverInfo, TypedConstraint** sortedConstraints, i32 numConstraints, IDebugDraw* debugDrawer)
	{
		Assert(solverInfo);
		m_solverInfo = solverInfo;
		m_sortedConstraints = sortedConstraints;
		m_numConstraints = numConstraints;
		m_debugDrawer = debugDrawer;
		m_bodies.resize(0);
		m_manifolds.resize(0);
		m_constraints.resize(0);
	}

	virtual void processIsland(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifolds, i32 numManifolds, i32 islandId)
	{
		if (islandId < 0)
		{
			///we don't split islands, so all constraints/contact manifolds/bodies are passed into the solver regardless the island id
			m_solver->solveGroup(bodies, numBodies, manifolds, numManifolds, &m_sortedConstraints[0], m_numConstraints, *m_solverInfo, m_debugDrawer, m_dispatcher);
		}
		else
		{
			//also add all non-contact constraints/joints for this island
			TypedConstraint** startConstraint = 0;
			i32 numCurConstraints = 0;
			i32 i;

			//find the first constraint for this island
			for (i = 0; i < m_numConstraints; i++)
			{
				if (GetConstraintIslandId(m_sortedConstraints[i]) == islandId)
				{
					startConstraint = &m_sortedConstraints[i];
					break;
				}
			}
			//count the number of constraints in this island
			for (; i < m_numConstraints; i++)
			{
				if (GetConstraintIslandId(m_sortedConstraints[i]) == islandId)
				{
					numCurConstraints++;
				}
			}

			if (m_solverInfo->m_minimumSolverBatchSize <= 1)
			{
				m_solver->solveGroup(bodies, numBodies, manifolds, numManifolds, startConstraint, numCurConstraints, *m_solverInfo, m_debugDrawer, m_dispatcher);
			}
			else
			{
				for (i = 0; i < numBodies; i++)
					m_bodies.push_back(bodies[i]);
				for (i = 0; i < numManifolds; i++)
					m_manifolds.push_back(manifolds[i]);
				for (i = 0; i < numCurConstraints; i++)
					m_constraints.push_back(startConstraint[i]);
				if ((m_constraints.size() + m_manifolds.size()) > m_solverInfo->m_minimumSolverBatchSize)
				{
					processConstraints();
				}
				else
				{
					//printf("deferred\n");
				}
			}
		}
	}
	void processConstraints()
	{
		CollisionObject2** bodies = m_bodies.size() ? &m_bodies[0] : 0;
		PersistentManifold** manifold = m_manifolds.size() ? &m_manifolds[0] : 0;
		TypedConstraint** constraints = m_constraints.size() ? &m_constraints[0] : 0;

		m_solver->solveGroup(bodies, m_bodies.size(), manifold, m_manifolds.size(), constraints, m_constraints.size(), *m_solverInfo, m_debugDrawer, m_dispatcher);
		m_bodies.resize(0);
		m_manifolds.resize(0);
		m_constraints.resize(0);
	}
};

DiscreteDynamicsWorld::DiscreteDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration)
	: DynamicsWorld(dispatcher, pairCache, collisionConfiguration),
	  m_sortedConstraints(),
	  m_solverIslandCallback(NULL),
	  m_constraintSolver(constraintSolver),
	  m_gravity(0, -10, 0),
	  m_localTime(0),
	  m_fixedTimeStep(0),
	  m_synchronizeAllMotionStates(false),
	  m_applySpeculativeContactRestitution(false),
	  m_profileTimings(0),
	  m_latencyMotionStateInterpolation(true)

{
	if (!m_constraintSolver)
	{
		uk mem = AlignedAlloc(sizeof(SequentialImpulseConstraintSolver), 16);
		m_constraintSolver = new (mem) SequentialImpulseConstraintSolver;
		m_ownsConstraintSolver = true;
	}
	else
	{
		m_ownsConstraintSolver = false;
	}

	{
		uk mem = AlignedAlloc(sizeof(SimulationIslandManager), 16);
		m_islandManager = new (mem) SimulationIslandManager();
	}

	m_ownsIslandManager = true;

	{
		uk mem = AlignedAlloc(sizeof(InplaceSolverIslandCallback), 16);
		m_solverIslandCallback = new (mem) InplaceSolverIslandCallback(m_constraintSolver, 0, dispatcher);
	}
}

DiscreteDynamicsWorld::~DiscreteDynamicsWorld()
{
	//only delete it when we created it
	if (m_ownsIslandManager)
	{
		m_islandManager->~SimulationIslandManager();
		AlignedFree(m_islandManager);
	}
	if (m_solverIslandCallback)
	{
		m_solverIslandCallback->~InplaceSolverIslandCallback();
		AlignedFree(m_solverIslandCallback);
	}
	if (m_ownsConstraintSolver)
	{
		m_constraintSolver->~ConstraintSolver();
		AlignedFree(m_constraintSolver);
	}
}

void DiscreteDynamicsWorld::saveKinematicState(Scalar timeStep)
{
	///would like to iterate over m_nonStaticRigidBodies, but unfortunately old API allows
	///to switch status _after_ adding kinematic objects to the world
	///fix it for drx3D 3.x release
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body && body->getActivationState() != ISLAND_SLEEPING)
		{
			if (body->isKinematicObject())
			{
				//to calculate velocities next frame
				body->saveKinematicState(timeStep);
			}
		}
	}
}

void DiscreteDynamicsWorld::debugDrawWorld()
{
	DRX3D_PROFILE("debugDrawWorld");

	CollisionWorld::debugDrawWorld();

	bool drawConstraints = false;
	if (getDebugDrawer())
	{
		i32 mode = getDebugDrawer()->getDebugMode();
		if (mode & (IDebugDraw::DBG_DrawConstraints | IDebugDraw::DBG_DrawConstraintLimits))
		{
			drawConstraints = true;
		}
	}
	if (drawConstraints)
	{
		for (i32 i = getNumConstraints() - 1; i >= 0; i--)
		{
			TypedConstraint* constraint = getConstraint(i);
			debugDrawConstraint(constraint);
		}
	}

	if (getDebugDrawer() && (getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe | IDebugDraw::DBG_DrawAabb | IDebugDraw::DBG_DrawNormals)))
	{
		i32 i;

		if (getDebugDrawer() && getDebugDrawer()->getDebugMode())
		{
			for (i = 0; i < m_actions.size(); i++)
			{
				m_actions[i]->debugDraw(m_debugDrawer);
			}
		}
	}
	if (getDebugDrawer())
		getDebugDrawer()->flushLines();
}

void DiscreteDynamicsWorld::clearForces()
{
	///@todo: iterate over awake simulation islands!
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		//need to check if next line is ok
		//it might break backward compatibility (people applying forces on sleeping objects get never cleared and accumulate on wake-up
		body->clearForces();
	}
}

///apply gravity, call this once per timestep
void DiscreteDynamicsWorld::applyGravity()
{
	///@todo: iterate over awake simulation islands!
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		if (body->isActive())
		{
			body->applyGravity();
		}
	}
}

void DiscreteDynamicsWorld::synchronizeSingleMotionState(RigidBody* body)
{
	Assert(body);

	if (body->getMotionState() && !body->isStaticOrKinematicObject())
	{
		//we need to call the update at least once, even for sleeping objects
		//otherwise the 'graphics' transform never updates properly
		///@todo: add 'dirty' flag
		//if (body->getActivationState() != ISLAND_SLEEPING)
		{
			Transform2 interpolatedTransform2;
			Transform2Util::integrateTransform(body->getInterpolationWorldTransform(),
												body->getInterpolationLinearVelocity(), body->getInterpolationAngularVelocity(),
												(m_latencyMotionStateInterpolation && m_fixedTimeStep) ? m_localTime - m_fixedTimeStep : m_localTime * body->getHitFraction(),
												interpolatedTransform2);
			body->getMotionState()->setWorldTransform(interpolatedTransform2);
		}
	}
}

void DiscreteDynamicsWorld::synchronizeMotionStates()
{
	//	DRX3D_PROFILE("synchronizeMotionStates");
	if (m_synchronizeAllMotionStates)
	{
		//iterate  over all collision objects
		for (i32 i = 0; i < m_collisionObjects.size(); i++)
		{
			CollisionObject2* colObj = m_collisionObjects[i];
			RigidBody* body = RigidBody::upcast(colObj);
			if (body)
				synchronizeSingleMotionState(body);
		}
	}
	else
	{
		//iterate over all active rigid bodies
		for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
		{
			RigidBody* body = m_nonStaticRigidBodies[i];
			if (body->isActive())
				synchronizeSingleMotionState(body);
		}
	}
}

i32 DiscreteDynamicsWorld::stepSimulation(Scalar timeStep, i32 maxSubSteps, Scalar fixedTimeStep)
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

		applyGravity();

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

void DiscreteDynamicsWorld::internalSingleStepSimulation(Scalar timeStep)
{
	DRX3D_PROFILE("internalSingleStepSimulation");

	if (0 != m_internalPreTickCallback)
	{
		(*m_internalPreTickCallback)(this, timeStep);
	}

	///apply gravity, predict motion
	predictUnconstraintMotion(timeStep);

	DispatcherInfo& dispatchInfo = getDispatchInfo();

	dispatchInfo.m_timeStep = timeStep;
	dispatchInfo.m_stepCount = 0;
	dispatchInfo.m_debugDraw = getDebugDrawer();

	createPredictiveContacts(timeStep);

	///perform collision detection
	performDiscreteCollisionDetection();

	calculateSimulationIslands();

	getSolverInfo().m_timeStep = timeStep;

	///solve contact and other joint constraints
	solveConstraints(getSolverInfo());

	///CallbackTriggers();

	///integrate transforms

	integrateTransforms(timeStep);

	///update vehicle simulation
	updateActions(timeStep);

	updateActivationState(timeStep);

	if (0 != m_internalTickCallback)
	{
		(*m_internalTickCallback)(this, timeStep);
	}
}

void DiscreteDynamicsWorld::setGravity(const Vec3& gravity)
{
	m_gravity = gravity;
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		if (body->isActive() && !(body->getFlags() & DRX3D_DISABLE_WORLD_GRAVITY))
		{
			body->setGravity(gravity);
		}
	}
}

Vec3 DiscreteDynamicsWorld::getGravity() const
{
	return m_gravity;
}

void DiscreteDynamicsWorld::addCollisionObject(CollisionObject2* collisionObject, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	CollisionWorld::addCollisionObject(collisionObject, collisionFilterGroup, collisionFilterMask);
}

void DiscreteDynamicsWorld::removeCollisionObject(CollisionObject2* collisionObject)
{
	RigidBody* body = RigidBody::upcast(collisionObject);
	if (body)
		removeRigidBody(body);
	else
		CollisionWorld::removeCollisionObject(collisionObject);
}

void DiscreteDynamicsWorld::removeRigidBody(RigidBody* body)
{
	m_nonStaticRigidBodies.remove(body);
	CollisionWorld::removeCollisionObject(body);
}

void DiscreteDynamicsWorld::addRigidBody(RigidBody* body)
{
	if (!body->isStaticOrKinematicObject() && !(body->getFlags() & DRX3D_DISABLE_WORLD_GRAVITY))
	{
		body->setGravity(m_gravity);
	}

	if (body->getCollisionShape())
	{
		if (!body->isStaticObject())
		{
			m_nonStaticRigidBodies.push_back(body);
		}
		else
		{
			body->setActivationState(ISLAND_SLEEPING);
		}

		bool isDynamic = !(body->isStaticObject() || body->isKinematicObject());
		i32 collisionFilterGroup = isDynamic ? i32(BroadphaseProxy::DefaultFilter) : i32(BroadphaseProxy::StaticFilter);
		i32 collisionFilterMask = isDynamic ? i32(BroadphaseProxy::AllFilter) : i32(BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

		addCollisionObject(body, collisionFilterGroup, collisionFilterMask);
	}
}

void DiscreteDynamicsWorld::addRigidBody(RigidBody* body, i32 group, i32 mask)
{
	if (!body->isStaticOrKinematicObject() && !(body->getFlags() & DRX3D_DISABLE_WORLD_GRAVITY))
	{
		body->setGravity(m_gravity);
	}

	if (body->getCollisionShape())
	{
		if (!body->isStaticObject())
		{
			m_nonStaticRigidBodies.push_back(body);
		}
		else
		{
			body->setActivationState(ISLAND_SLEEPING);
		}
		addCollisionObject(body, group, mask);
	}
}

void DiscreteDynamicsWorld::updateActions(Scalar timeStep)
{
	DRX3D_PROFILE("updateActions");

	for (i32 i = 0; i < m_actions.size(); i++)
	{
		m_actions[i]->updateAction(this, timeStep);
	}
}

void DiscreteDynamicsWorld::updateActivationState(Scalar timeStep)
{
	DRX3D_PROFILE("updateActivationState");

	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		if (body)
		{
			body->updateDeactivation(timeStep);

			if (body->wantsSleeping())
			{
				if (body->isStaticOrKinematicObject())
				{
					body->setActivationState(ISLAND_SLEEPING);
				}
				else
				{
					if (body->getActivationState() == ACTIVE_TAG)
						body->setActivationState(WANTS_DEACTIVATION);
					if (body->getActivationState() == ISLAND_SLEEPING)
					{
						body->setAngularVelocity(Vec3(0, 0, 0));
						body->setLinearVelocity(Vec3(0, 0, 0));
					}
				}
			}
			else
			{
				if (body->getActivationState() != DISABLE_DEACTIVATION)
					body->setActivationState(ACTIVE_TAG);
			}
		}
	}
}

void DiscreteDynamicsWorld::addConstraint(TypedConstraint* constraint, bool disableCollisionsBetweenLinkedBodies)
{
	m_constraints.push_back(constraint);
	//Make sure the two bodies of a type constraint are different (possibly add this to the TypedConstraint constructor?)
	Assert(&constraint->getRigidBodyA() != &constraint->getRigidBodyB());

	if (disableCollisionsBetweenLinkedBodies)
	{
		constraint->getRigidBodyA().addConstraintRef(constraint);
		constraint->getRigidBodyB().addConstraintRef(constraint);
	}
}

void DiscreteDynamicsWorld::removeConstraint(TypedConstraint* constraint)
{
	m_constraints.remove(constraint);
	constraint->getRigidBodyA().removeConstraintRef(constraint);
	constraint->getRigidBodyB().removeConstraintRef(constraint);
}

void DiscreteDynamicsWorld::addAction(ActionInterface* action)
{
	m_actions.push_back(action);
}

void DiscreteDynamicsWorld::removeAction(ActionInterface* action)
{
	m_actions.remove(action);
}

void DiscreteDynamicsWorld::addVehicle(ActionInterface* vehicle)
{
	addAction(vehicle);
}

void DiscreteDynamicsWorld::removeVehicle(ActionInterface* vehicle)
{
	removeAction(vehicle);
}

void DiscreteDynamicsWorld::addCharacter(ActionInterface* character)
{
	addAction(character);
}

void DiscreteDynamicsWorld::removeCharacter(ActionInterface* character)
{
	removeAction(character);
}

void DiscreteDynamicsWorld::solveConstraints(ContactSolverInfo& solverInfo)
{
	DRX3D_PROFILE("solveConstraints");

	m_sortedConstraints.resize(m_constraints.size());
	i32 i;
	for (i = 0; i < getNumConstraints(); i++)
	{
		m_sortedConstraints[i] = m_constraints[i];
	}

	//	Assert(0);

	m_sortedConstraints.quickSort(SortConstraintOnIslandPredicate());

	TypedConstraint** constraintsPtr = getNumConstraints() ? &m_sortedConstraints[0] : 0;

	m_solverIslandCallback->setup(&solverInfo, constraintsPtr, m_sortedConstraints.size(), getDebugDrawer());
	m_constraintSolver->prepareSolve(getCollisionWorld()->getNumCollisionObjects(), getCollisionWorld()->getDispatcher()->getNumManifolds());

	/// solve all the constraints for this island
	m_islandManager->buildAndProcessIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld(), m_solverIslandCallback);

	m_solverIslandCallback->processConstraints();

	m_constraintSolver->allSolved(solverInfo, m_debugDrawer);
}

void DiscreteDynamicsWorld::calculateSimulationIslands()
{
	DRX3D_PROFILE("calculateSimulationIslands");

	getSimulationIslandManager()->updateActivationState(getCollisionWorld(), getCollisionWorld()->getDispatcher());

	{
		//merge islands based on speculative contact manifolds too
		for (i32 i = 0; i < this->m_predictiveManifolds.size(); i++)
		{
			PersistentManifold* manifold = m_predictiveManifolds[i];

			const CollisionObject2* colObj0 = manifold->getBody0();
			const CollisionObject2* colObj1 = manifold->getBody1();

			if (((colObj0) && (!(colObj0)->isStaticOrKinematicObject())) &&
				((colObj1) && (!(colObj1)->isStaticOrKinematicObject())))
			{
				getSimulationIslandManager()->getUnionFind().unite((colObj0)->getIslandTag(), (colObj1)->getIslandTag());
			}
		}
	}

	{
		i32 i;
		i32 numConstraints = i32(m_constraints.size());
		for (i = 0; i < numConstraints; i++)
		{
			TypedConstraint* constraint = m_constraints[i];
			if (constraint->isEnabled())
			{
				const RigidBody* colObj0 = &constraint->getRigidBodyA();
				const RigidBody* colObj1 = &constraint->getRigidBodyB();

				if (((colObj0) && (!(colObj0)->isStaticOrKinematicObject())) &&
					((colObj1) && (!(colObj1)->isStaticOrKinematicObject())))
				{
					getSimulationIslandManager()->getUnionFind().unite((colObj0)->getIslandTag(), (colObj1)->getIslandTag());
				}
			}
		}
	}

	//Store the island id in each body
	getSimulationIslandManager()->storeIslandActivationState(getCollisionWorld());
}

class ClosestNotMeConvexResultCallback : public CollisionWorld::ClosestConvexResultCallback
{
public:
	CollisionObject2* m_me;
	Scalar m_allowedPenetration;
	OverlappingPairCache* m_pairCache;
	Dispatcher* m_dispatcher;

public:
	ClosestNotMeConvexResultCallback(CollisionObject2* me, const Vec3& fromA, const Vec3& toA, OverlappingPairCache* pairCache, Dispatcher* dispatcher) : CollisionWorld::ClosestConvexResultCallback(fromA, toA),
																																										   m_me(me),
																																										   m_allowedPenetration(0.0f),
																																										   m_pairCache(pairCache),
																																										   m_dispatcher(dispatcher)
	{
	}

	virtual Scalar addSingleResult(CollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		if (convexResult.m_hitCollisionObject2 == m_me)
			return 1.0f;

		//ignore result if there is no contact response
		if (!convexResult.m_hitCollisionObject2->hasContactResponse())
			return 1.0f;

		Vec3 linVelA, linVelB;
		linVelA = m_convexToWorld - m_convexFromWorld;
		linVelB = Vec3(0, 0, 0);  //toB.getOrigin()-fromB.getOrigin();

		Vec3 relativeVelocity = (linVelA - linVelB);
		//don't report time of impact for motion away from the contact normal (or causes minor penetration)
		if (convexResult.m_hitNormalLocal.dot(relativeVelocity) >= -m_allowedPenetration)
			return 1.f;

		return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}

	virtual bool needsCollision(BroadphaseProxy* proxy0) const
	{
		//don't collide with itself
		if (proxy0->m_clientObject == m_me)
			return false;

		///don't do CCD when the collision filters are not matching
		if (!ClosestConvexResultCallback::needsCollision(proxy0))
			return false;
		if (m_pairCache->getOverlapFilterCallback()) {
			BroadphaseProxy* proxy1 = m_me->getBroadphaseHandle();
			bool collides = m_pairCache->needsBroadphaseCollision(proxy0, proxy1);
			if (!collides)
			{
				return false;
			}
		}

		CollisionObject2* otherObj = (CollisionObject2*)proxy0->m_clientObject;

		if (!m_dispatcher->needsCollision(m_me, otherObj))
			return false;

		//call needsResponse, see http://code.google.com/p/bullet/issues/detail?id=179
		if (m_dispatcher->needsResponse(m_me, otherObj))
		{
#if 0
			///don't do CCD when there are already contact points (touching contact/penetration)
			AlignedObjectArray<PersistentManifold*> manifoldArray;
			BroadphasePair* collisionPair = m_pairCache->findPair(m_me->getBroadphaseHandle(),proxy0);
			if (collisionPair)
			{
				if (collisionPair->m_algorithm)
				{
					manifoldArray.resize(0);
					collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
					for (i32 j=0;j<manifoldArray.size();j++)
					{
						PersistentManifold* manifold = manifoldArray[j];
						if (manifold->getNumContacts()>0)
							return false;
					}
				}
			}
#endif
			return true;
		}

		return false;
	}
};

///internal debugging variable. this value shouldn't be too high
i32 gNumClampedCcdMotions = 0;

void DiscreteDynamicsWorld::createPredictiveContactsInternal(RigidBody** bodies, i32 numBodies, Scalar timeStep)
{
	Transform2 predictedTrans;
	for (i32 i = 0; i < numBodies; i++)
	{
		RigidBody* body = bodies[i];
		body->setHitFraction(1.f);

		if (body->isActive() && (!body->isStaticOrKinematicObject()))
		{
			body->predictIntegratedTransform2(timeStep, predictedTrans);

			Scalar squareMotion = (predictedTrans.getOrigin() - body->getWorldTransform().getOrigin()).length2();

			if (getDispatchInfo().m_useContinuous && body->getCcdSquareMotionThreshold() && body->getCcdSquareMotionThreshold() < squareMotion)
			{
				DRX3D_PROFILE("predictive convexSweepTest");
				if (body->getCollisionShape()->isConvex())
				{
					gNumClampedCcdMotions++;
#ifdef PREDICTIVE_CONTACT_USE_STATIC_ONLY
					class StaticOnlyCallback : public ClosestNotMeConvexResultCallback
					{
					public:
						StaticOnlyCallback(CollisionObject2* me, const Vec3& fromA, const Vec3& toA, OverlappingPairCache* pairCache, Dispatcher* dispatcher) : ClosestNotMeConvexResultCallback(me, fromA, toA, pairCache, dispatcher)
						{
						}

						virtual bool needsCollision(BroadphaseProxy* proxy0) const
						{
							CollisionObject2* otherObj = (CollisionObject2*)proxy0->m_clientObject;
							if (!otherObj->isStaticOrKinematicObject())
								return false;
							return ClosestNotMeConvexResultCallback::needsCollision(proxy0);
						}
					};

					StaticOnlyCallback sweepResults(body, body->getWorldTransform().getOrigin(), predictedTrans.getOrigin(), getBroadphase()->getOverlappingPairCache(), getDispatcher());
#else
					ClosestNotMeConvexResultCallback sweepResults(body, body->getWorldTransform().getOrigin(), predictedTrans.getOrigin(), getBroadphase()->getOverlappingPairCache(), getDispatcher());
#endif
					//ConvexShape* convexShape = static_cast<ConvexShape*>(body->getCollisionShape());
					SphereShape tmpSphere(body->getCcdSweptSphereRadius());  //ConvexShape* convexShape = static_cast<ConvexShape*>(body->getCollisionShape());
					sweepResults.m_allowedPenetration = getDispatchInfo().m_allowedCcdPenetration;

					sweepResults.m_collisionFilterGroup = body->getBroadphaseProxy()->m_collisionFilterGroup;
					sweepResults.m_collisionFilterMask = body->getBroadphaseProxy()->m_collisionFilterMask;
					Transform2 modifiedPredictedTrans = predictedTrans;
					modifiedPredictedTrans.setBasis(body->getWorldTransform().getBasis());

					convexSweepTest(&tmpSphere, body->getWorldTransform(), modifiedPredictedTrans, sweepResults);
					if (sweepResults.hasHit() && (sweepResults.m_closestHitFraction < 1.f))
					{
						Vec3 distVec = (predictedTrans.getOrigin() - body->getWorldTransform().getOrigin()) * sweepResults.m_closestHitFraction;
						Scalar distance = distVec.dot(-sweepResults.m_hitNormalWorld);

						MutexLock(&m_predictiveManifoldsMutex);
						PersistentManifold* manifold = m_dispatcher1->getNewManifold(body, sweepResults.m_hitCollisionObject2);
						m_predictiveManifolds.push_back(manifold);
						MutexUnlock(&m_predictiveManifoldsMutex);

						Vec3 worldPointB = body->getWorldTransform().getOrigin() + distVec;
						Vec3 localPointB = sweepResults.m_hitCollisionObject2->getWorldTransform().inverse() * worldPointB;

						ManifoldPoint newPoint(Vec3(0, 0, 0), localPointB, sweepResults.m_hitNormalWorld, distance);

						bool isPredictive = true;
						i32 index = manifold->addManifoldPoint(newPoint, isPredictive);
						ManifoldPoint& pt = manifold->getContactPoint(index);
						pt.m_combinedRestitution = 0;
						pt.m_combinedFriction = gCalculateCombinedFrictionCallback(body, sweepResults.m_hitCollisionObject2);
						pt.m_positionWorldOnA = body->getWorldTransform().getOrigin();
						pt.m_positionWorldOnB = worldPointB;
					}
				}
			}
		}
	}
}

void DiscreteDynamicsWorld::releasePredictiveContacts()
{
	DRX3D_PROFILE("release predictive contact manifolds");

	for (i32 i = 0; i < m_predictiveManifolds.size(); i++)
	{
		PersistentManifold* manifold = m_predictiveManifolds[i];
		this->m_dispatcher1->releaseManifold(manifold);
	}
	m_predictiveManifolds.clear();
}

void DiscreteDynamicsWorld::createPredictiveContacts(Scalar timeStep)
{
	DRX3D_PROFILE("createPredictiveContacts");
	releasePredictiveContacts();
	if (m_nonStaticRigidBodies.size() > 0)
	{
		createPredictiveContactsInternal(&m_nonStaticRigidBodies[0], m_nonStaticRigidBodies.size(), timeStep);
	}
}

void DiscreteDynamicsWorld::integrateTransformsInternal(RigidBody** bodies, i32 numBodies, Scalar timeStep)
{
	Transform2 predictedTrans;
	for (i32 i = 0; i < numBodies; i++)
	{
		RigidBody* body = bodies[i];
		body->setHitFraction(1.f);

		if (body->isActive() && (!body->isStaticOrKinematicObject()))
		{
			body->predictIntegratedTransform2(timeStep, predictedTrans);

			Scalar squareMotion = (predictedTrans.getOrigin() - body->getWorldTransform().getOrigin()).length2();

			if (getDispatchInfo().m_useContinuous && body->getCcdSquareMotionThreshold() && body->getCcdSquareMotionThreshold() < squareMotion)
			{
				DRX3D_PROFILE("CCD motion clamping");
				if (body->getCollisionShape()->isConvex())
				{
					gNumClampedCcdMotions++;
#ifdef USE_STATIC_ONLY
					class StaticOnlyCallback : public ClosestNotMeConvexResultCallback
					{
					public:
						StaticOnlyCallback(CollisionObject2* me, const Vec3& fromA, const Vec3& toA, OverlappingPairCache* pairCache, Dispatcher* dispatcher) : ClosestNotMeConvexResultCallback(me, fromA, toA, pairCache, dispatcher)
						{
						}

						virtual bool needsCollision(BroadphaseProxy* proxy0) const
						{
							CollisionObject2* otherObj = (CollisionObject2*)proxy0->m_clientObject;
							if (!otherObj->isStaticOrKinematicObject())
								return false;
							return ClosestNotMeConvexResultCallback::needsCollision(proxy0);
						}
					};

					StaticOnlyCallback sweepResults(body, body->getWorldTransform().getOrigin(), predictedTrans.getOrigin(), getBroadphase()->getOverlappingPairCache(), getDispatcher());
#else
					ClosestNotMeConvexResultCallback sweepResults(body, body->getWorldTransform().getOrigin(), predictedTrans.getOrigin(), getBroadphase()->getOverlappingPairCache(), getDispatcher());
#endif
					//ConvexShape* convexShape = static_cast<ConvexShape*>(body->getCollisionShape());
					SphereShape tmpSphere(body->getCcdSweptSphereRadius());  //ConvexShape* convexShape = static_cast<ConvexShape*>(body->getCollisionShape());
					sweepResults.m_allowedPenetration = getDispatchInfo().m_allowedCcdPenetration;

					sweepResults.m_collisionFilterGroup = body->getBroadphaseProxy()->m_collisionFilterGroup;
					sweepResults.m_collisionFilterMask = body->getBroadphaseProxy()->m_collisionFilterMask;
					Transform2 modifiedPredictedTrans = predictedTrans;
					modifiedPredictedTrans.setBasis(body->getWorldTransform().getBasis());

					convexSweepTest(&tmpSphere, body->getWorldTransform(), modifiedPredictedTrans, sweepResults);
					if (sweepResults.hasHit() && (sweepResults.m_closestHitFraction < 1.f))
					{
						//printf("clamped integration to hit fraction = %f\n",fraction);
						body->setHitFraction(sweepResults.m_closestHitFraction);
						body->predictIntegratedTransform2(timeStep * body->getHitFraction(), predictedTrans);
						body->setHitFraction(0.f);
						body->proceedToTransform2(predictedTrans);

#if 0
						Vec3 linVel = body->getLinearVelocity();

						Scalar maxSpeed = body->getCcdMotionThreshold()/getSolverInfo().m_timeStep;
						Scalar maxSpeedSqr = maxSpeed*maxSpeed;
						if (linVel.length2()>maxSpeedSqr)
						{
							linVel.normalize();
							linVel*= maxSpeed;
							body->setLinearVelocity(linVel);
							Scalar ms2 = body->getLinearVelocity().length2();
							body->predictIntegratedTransform2(timeStep, predictedTrans);

							Scalar sm2 = (predictedTrans.getOrigin()-body->getWorldTransform().getOrigin()).length2();
							Scalar smt = body->getCcdSquareMotionThreshold();
							printf("sm2=%f\n",sm2);
						}
#else

						//don't apply the collision response right now, it will happen next frame
						//if you really need to, you can uncomment next 3 lines. Note that is uses zero restitution.
						//Scalar appliedImpulse = 0.f;
						//Scalar depth = 0.f;
						//appliedImpulse = resolveSingleCollision(body,(CollisionObject2*)sweepResults.m_hitCollisionObject2,sweepResults.m_hitPointWorld,sweepResults.m_hitNormalWorld,getSolverInfo(), depth);

#endif

						continue;
					}
				}
			}

			body->proceedToTransform2(predictedTrans);
		}
	}
}

void DiscreteDynamicsWorld::integrateTransforms(Scalar timeStep)
{
	DRX3D_PROFILE("integrateTransforms");
	if (m_nonStaticRigidBodies.size() > 0)
	{
		integrateTransformsInternal(&m_nonStaticRigidBodies[0], m_nonStaticRigidBodies.size(), timeStep);
	}

	///this should probably be switched on by default, but it is not well tested yet
	if (m_applySpeculativeContactRestitution)
	{
		DRX3D_PROFILE("apply speculative contact restitution");
		for (i32 i = 0; i < m_predictiveManifolds.size(); i++)
		{
			PersistentManifold* manifold = m_predictiveManifolds[i];
			RigidBody* body0 = RigidBody::upcast((CollisionObject2*)manifold->getBody0());
			RigidBody* body1 = RigidBody::upcast((CollisionObject2*)manifold->getBody1());

			for (i32 p = 0; p < manifold->getNumContacts(); p++)
			{
				const ManifoldPoint& pt = manifold->getContactPoint(p);
				Scalar combinedRestitution = gCalculateCombinedRestitutionCallback(body0, body1);

				if (combinedRestitution > 0 && pt.m_appliedImpulse != 0.f)
				//if (pt.getDistance()>0 && combinedRestitution>0 && pt.m_appliedImpulse != 0.f)
				{
					Vec3 imp = -pt.m_normalWorldOnB * pt.m_appliedImpulse * combinedRestitution;

					const Vec3& pos1 = pt.getPositionWorldOnA();
					const Vec3& pos2 = pt.getPositionWorldOnB();

					Vec3 rel_pos0 = pos1 - body0->getWorldTransform().getOrigin();
					Vec3 rel_pos1 = pos2 - body1->getWorldTransform().getOrigin();

					if (body0)
						body0->applyImpulse(imp, rel_pos0);
					if (body1)
						body1->applyImpulse(-imp, rel_pos1);
				}
			}
		}
	}
}

void DiscreteDynamicsWorld::predictUnconstraintMotion(Scalar timeStep)
{
	DRX3D_PROFILE("predictUnconstraintMotion");
	for (i32 i = 0; i < m_nonStaticRigidBodies.size(); i++)
	{
		RigidBody* body = m_nonStaticRigidBodies[i];
		if (!body->isStaticOrKinematicObject())
		{
			//don't integrate/update velocities here, it happens in the constraint solver

			body->applyDamping(timeStep);

			body->predictIntegratedTransform2(timeStep, body->getInterpolationWorldTransform());
		}
	}
}

void DiscreteDynamicsWorld::startProfiling(Scalar timeStep)
{
	(void)timeStep;

#ifndef DRX3D_NO_PROFILE
	CProfileManager::Reset();
#endif  //DRX3D_NO_PROFILE
}

void DiscreteDynamicsWorld::debugDrawConstraint(TypedConstraint* constraint)
{
	bool drawFrames = (getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawConstraints) != 0;
	bool drawLimits = (getDebugDrawer()->getDebugMode() & IDebugDraw::DBG_DrawConstraintLimits) != 0;
	Scalar dbgDrawSize = constraint->getDbgDrawSize();
	if (dbgDrawSize <= Scalar(0.f))
	{
		return;
	}

	switch (constraint->getConstraintType())
	{
		case POINT2POINT_CONSTRAINT_TYPE:
		{
			Point2PointConstraint* p2pC = (Point2PointConstraint*)constraint;
			Transform2 tr;
			tr.setIdentity();
			Vec3 pivot = p2pC->getPivotInA();
			pivot = p2pC->getRigidBodyA().getCenterOfMassTransform() * pivot;
			tr.setOrigin(pivot);
			getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			// that ideally should draw the same frame
			pivot = p2pC->getPivotInB();
			pivot = p2pC->getRigidBodyB().getCenterOfMassTransform() * pivot;
			tr.setOrigin(pivot);
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
		}
		break;
		case HINGE_CONSTRAINT_TYPE:
		{
			HingeConstraint* pHinge = (HingeConstraint*)constraint;
			Transform2 tr = pHinge->getRigidBodyA().getCenterOfMassTransform() * pHinge->getAFrame();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			tr = pHinge->getRigidBodyB().getCenterOfMassTransform() * pHinge->getBFrame();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			Scalar minAng = pHinge->getLowerLimit();
			Scalar maxAng = pHinge->getUpperLimit();
			if (minAng == maxAng)
			{
				break;
			}
			bool drawSect = true;
			if (!pHinge->hasLimit())
			{
				minAng = Scalar(0.f);
				maxAng = SIMD_2_PI;
				drawSect = false;
			}
			if (drawLimits)
			{
				Vec3& center = tr.getOrigin();
				Vec3 normal = tr.getBasis().getColumn(2);
				Vec3 axis = tr.getBasis().getColumn(0);
				getDebugDrawer()->drawArc(center, normal, axis, dbgDrawSize, dbgDrawSize, minAng, maxAng, Vec3(0, 0, 0), drawSect);
			}
		}
		break;
		case CONETWIST_CONSTRAINT_TYPE:
		{
			ConeTwistConstraint* pCT = (ConeTwistConstraint*)constraint;
			Transform2 tr = pCT->getRigidBodyA().getCenterOfMassTransform() * pCT->getAFrame();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			tr = pCT->getRigidBodyB().getCenterOfMassTransform() * pCT->getBFrame();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			if (drawLimits)
			{
				//const Scalar length = Scalar(5);
				const Scalar length = dbgDrawSize;
				static i32 nSegments = 8 * 4;
				Scalar fAngleInRadians = Scalar(2. * 3.1415926) * (Scalar)(nSegments - 1) / Scalar(nSegments);
				Vec3 pPrev = pCT->GetPointForAngle(fAngleInRadians, length);
				pPrev = tr * pPrev;
				for (i32 i = 0; i < nSegments; i++)
				{
					fAngleInRadians = Scalar(2. * 3.1415926) * (Scalar)i / Scalar(nSegments);
					Vec3 pCur = pCT->GetPointForAngle(fAngleInRadians, length);
					pCur = tr * pCur;
					getDebugDrawer()->drawLine(pPrev, pCur, Vec3(0, 0, 0));

					if (i % (nSegments / 8) == 0)
						getDebugDrawer()->drawLine(tr.getOrigin(), pCur, Vec3(0, 0, 0));

					pPrev = pCur;
				}
				Scalar tws = pCT->getTwistSpan();
				Scalar twa = pCT->getTwistAngle();
				bool useFrameB = (pCT->getRigidBodyB().getInvMass() > Scalar(0.f));
				if (useFrameB)
				{
					tr = pCT->getRigidBodyB().getCenterOfMassTransform() * pCT->getBFrame();
				}
				else
				{
					tr = pCT->getRigidBodyA().getCenterOfMassTransform() * pCT->getAFrame();
				}
				Vec3 pivot = tr.getOrigin();
				Vec3 normal = tr.getBasis().getColumn(0);
				Vec3 axis1 = tr.getBasis().getColumn(1);
				getDebugDrawer()->drawArc(pivot, normal, axis1, dbgDrawSize, dbgDrawSize, -twa - tws, -twa + tws, Vec3(0, 0, 0), true);
			}
		}
		break;
		case D6_SPRING_CONSTRAINT_TYPE:
		case D6_CONSTRAINT_TYPE:
		{
			Generic6DofConstraint* p6DOF = (Generic6DofConstraint*)constraint;
			Transform2 tr = p6DOF->getCalculatedTransform2A();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			tr = p6DOF->getCalculatedTransform2B();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			if (drawLimits)
			{
				tr = p6DOF->getCalculatedTransform2A();
				const Vec3& center = p6DOF->getCalculatedTransform2B().getOrigin();
				Vec3 up = tr.getBasis().getColumn(2);
				Vec3 axis = tr.getBasis().getColumn(0);
				Scalar minTh = p6DOF->getRotationalLimitMotor(1)->m_loLimit;
				Scalar maxTh = p6DOF->getRotationalLimitMotor(1)->m_hiLimit;
				Scalar minPs = p6DOF->getRotationalLimitMotor(2)->m_loLimit;
				Scalar maxPs = p6DOF->getRotationalLimitMotor(2)->m_hiLimit;
				getDebugDrawer()->drawSpherePatch(center, up, axis, dbgDrawSize * Scalar(.9f), minTh, maxTh, minPs, maxPs, Vec3(0, 0, 0));
				axis = tr.getBasis().getColumn(1);
				Scalar ay = p6DOF->getAngle(1);
				Scalar az = p6DOF->getAngle(2);
				Scalar cy = Cos(ay);
				Scalar sy = Sin(ay);
				Scalar cz = Cos(az);
				Scalar sz = Sin(az);
				Vec3 ref;
				ref[0] = cy * cz * axis[0] + cy * sz * axis[1] - sy * axis[2];
				ref[1] = -sz * axis[0] + cz * axis[1];
				ref[2] = cz * sy * axis[0] + sz * sy * axis[1] + cy * axis[2];
				tr = p6DOF->getCalculatedTransform2B();
				Vec3 normal = -tr.getBasis().getColumn(0);
				Scalar minFi = p6DOF->getRotationalLimitMotor(0)->m_loLimit;
				Scalar maxFi = p6DOF->getRotationalLimitMotor(0)->m_hiLimit;
				if (minFi > maxFi)
				{
					getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, -SIMD_PI, SIMD_PI, Vec3(0, 0, 0), false);
				}
				else if (minFi < maxFi)
				{
					getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, minFi, maxFi, Vec3(0, 0, 0), true);
				}
				tr = p6DOF->getCalculatedTransform2A();
				Vec3 bbMin = p6DOF->getTranslationalLimitMotor()->m_lowerLimit;
				Vec3 bbMax = p6DOF->getTranslationalLimitMotor()->m_upperLimit;
				getDebugDrawer()->drawBox(bbMin, bbMax, tr, Vec3(0, 0, 0));
			}
		}
		break;
		///note: the code for D6_SPRING_2_CONSTRAINT_TYPE is identical to D6_CONSTRAINT_TYPE, the D6_CONSTRAINT_TYPE+D6_SPRING_CONSTRAINT_TYPE will likely become obsolete/deprecated at some stage
		case D6_SPRING_2_CONSTRAINT_TYPE:
		{
			{
				Generic6DofSpring2Constraint* p6DOF = (Generic6DofSpring2Constraint*)constraint;
				Transform2 tr = p6DOF->getCalculatedTransform2A();
				if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
				tr = p6DOF->getCalculatedTransform2B();
				if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
				if (drawLimits)
				{
					tr = p6DOF->getCalculatedTransform2A();
					const Vec3& center = p6DOF->getCalculatedTransform2B().getOrigin();
					Vec3 up = tr.getBasis().getColumn(2);
					Vec3 axis = tr.getBasis().getColumn(0);
					Scalar minTh = p6DOF->getRotationalLimitMotor(1)->m_loLimit;
					Scalar maxTh = p6DOF->getRotationalLimitMotor(1)->m_hiLimit;
					if (minTh <= maxTh)
					{
						Scalar minPs = p6DOF->getRotationalLimitMotor(2)->m_loLimit;
						Scalar maxPs = p6DOF->getRotationalLimitMotor(2)->m_hiLimit;
						getDebugDrawer()->drawSpherePatch(center, up, axis, dbgDrawSize * Scalar(.9f), minTh, maxTh, minPs, maxPs, Vec3(0, 0, 0));
					}
					axis = tr.getBasis().getColumn(1);
					Scalar ay = p6DOF->getAngle(1);
					Scalar az = p6DOF->getAngle(2);
					Scalar cy = Cos(ay);
					Scalar sy = Sin(ay);
					Scalar cz = Cos(az);
					Scalar sz = Sin(az);
					Vec3 ref;
					ref[0] = cy * cz * axis[0] + cy * sz * axis[1] - sy * axis[2];
					ref[1] = -sz * axis[0] + cz * axis[1];
					ref[2] = cz * sy * axis[0] + sz * sy * axis[1] + cy * axis[2];
					tr = p6DOF->getCalculatedTransform2B();
					Vec3 normal = -tr.getBasis().getColumn(0);
					Scalar minFi = p6DOF->getRotationalLimitMotor(0)->m_loLimit;
					Scalar maxFi = p6DOF->getRotationalLimitMotor(0)->m_hiLimit;
					if (minFi > maxFi)
					{
						getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, -SIMD_PI, SIMD_PI, Vec3(0, 0, 0), false);
					}
					else if (minFi < maxFi)
					{
						getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, minFi, maxFi, Vec3(0, 0, 0), true);
					}
					tr = p6DOF->getCalculatedTransform2A();
					Vec3 bbMin = p6DOF->getTranslationalLimitMotor()->m_lowerLimit;
					Vec3 bbMax = p6DOF->getTranslationalLimitMotor()->m_upperLimit;
					getDebugDrawer()->drawBox(bbMin, bbMax, tr, Vec3(0, 0, 0));
				}
			}
			break;
		}
		case SLIDER_CONSTRAINT_TYPE:
		{
			SliderConstraint* pSlider = (SliderConstraint*)constraint;
			Transform2 tr = pSlider->getCalculatedTransform2A();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			tr = pSlider->getCalculatedTransform2B();
			if (drawFrames) getDebugDrawer()->drawTransform2(tr, dbgDrawSize);
			if (drawLimits)
			{
				Transform2 tr = pSlider->getUseLinearReferenceFrameA() ? pSlider->getCalculatedTransform2A() : pSlider->getCalculatedTransform2B();
				Vec3 li_min = tr * Vec3(pSlider->getLowerLinLimit(), 0.f, 0.f);
				Vec3 li_max = tr * Vec3(pSlider->getUpperLinLimit(), 0.f, 0.f);
				getDebugDrawer()->drawLine(li_min, li_max, Vec3(0, 0, 0));
				Vec3 normal = tr.getBasis().getColumn(0);
				Vec3 axis = tr.getBasis().getColumn(1);
				Scalar a_min = pSlider->getLowerAngLimit();
				Scalar a_max = pSlider->getUpperAngLimit();
				const Vec3& center = pSlider->getCalculatedTransform2B().getOrigin();
				getDebugDrawer()->drawArc(center, normal, axis, dbgDrawSize, dbgDrawSize, a_min, a_max, Vec3(0, 0, 0), true);
			}
		}
		break;
		default:
			break;
	}
	return;
}

void DiscreteDynamicsWorld::setConstraintSolver(ConstraintSolver* solver)
{
	if (m_ownsConstraintSolver)
	{
		AlignedFree(m_constraintSolver);
	}
	m_ownsConstraintSolver = false;
	m_constraintSolver = solver;
	m_solverIslandCallback->m_solver = solver;
}

ConstraintSolver* DiscreteDynamicsWorld::getConstraintSolver()
{
	return m_constraintSolver;
}

i32 DiscreteDynamicsWorld::getNumConstraints() const
{
	return i32(m_constraints.size());
}
TypedConstraint* DiscreteDynamicsWorld::getConstraint(i32 index)
{
	return m_constraints[index];
}
const TypedConstraint* DiscreteDynamicsWorld::getConstraint(i32 index) const
{
	return m_constraints[index];
}

void DiscreteDynamicsWorld::serializeRigidBodies(Serializer* serializer)
{
	i32 i;
	//serialize all collision objects
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		if (colObj->getInternalType() & CollisionObject2::CO_RIGID_BODY)
		{
			i32 len = colObj->calculateSerializeBufferSize();
			Chunk* chunk = serializer->allocate(len, 1);
			tukk structType = colObj->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_RIGIDBODY_CODE, colObj);
		}
	}

	for (i = 0; i < m_constraints.size(); i++)
	{
		TypedConstraint* constraint = m_constraints[i];
		i32 size = constraint->calculateSerializeBufferSize();
		Chunk* chunk = serializer->allocate(size, 1);
		tukk structType = constraint->serialize(chunk->m_oldPtr, serializer);
		serializer->finalizeChunk(chunk, structType, DRX3D_CONSTRAINT_CODE, constraint);
	}
}

void DiscreteDynamicsWorld::serializeDynamicsWorldInfo(Serializer* serializer)
{
#ifdef DRX3D_USE_DOUBLE_PRECISION
	i32 len = sizeof(DynamicsWorldDoubleData);
	Chunk* chunk = serializer->allocate(len, 1);
	DynamicsWorldDoubleData* worldInfo = (DynamicsWorldDoubleData*)chunk->m_oldPtr;
#else   //DRX3D_USE_DOUBLE_PRECISION
	i32 len = sizeof(DynamicsWorldFloatData);
	Chunk* chunk = serializer->allocate(len, 1);
	DynamicsWorldFloatData* worldInfo = (DynamicsWorldFloatData*)chunk->m_oldPtr;
#endif  //DRX3D_USE_DOUBLE_PRECISION

	memset(worldInfo, 0x00, len);

	m_gravity.serialize(worldInfo->m_gravity);
	worldInfo->m_solverInfo.m_tau = getSolverInfo().m_tau;
	worldInfo->m_solverInfo.m_damping = getSolverInfo().m_damping;
	worldInfo->m_solverInfo.m_friction = getSolverInfo().m_friction;
	worldInfo->m_solverInfo.m_timeStep = getSolverInfo().m_timeStep;

	worldInfo->m_solverInfo.m_restitution = getSolverInfo().m_restitution;
	worldInfo->m_solverInfo.m_maxErrorReduction = getSolverInfo().m_maxErrorReduction;
	worldInfo->m_solverInfo.m_sor = getSolverInfo().m_sor;
	worldInfo->m_solverInfo.m_erp = getSolverInfo().m_erp;

	worldInfo->m_solverInfo.m_erp2 = getSolverInfo().m_erp2;
	worldInfo->m_solverInfo.m_globalCfm = getSolverInfo().m_globalCfm;
	worldInfo->m_solverInfo.m_splitImpulsePenetrationThreshold = getSolverInfo().m_splitImpulsePenetrationThreshold;
	worldInfo->m_solverInfo.m_splitImpulseTurnErp = getSolverInfo().m_splitImpulseTurnErp;

	worldInfo->m_solverInfo.m_linearSlop = getSolverInfo().m_linearSlop;
	worldInfo->m_solverInfo.m_warmstartingFactor = getSolverInfo().m_warmstartingFactor;
	worldInfo->m_solverInfo.m_maxGyroscopicForce = getSolverInfo().m_maxGyroscopicForce;
	worldInfo->m_solverInfo.m_singleAxisRollingFrictionThreshold = getSolverInfo().m_singleAxisRollingFrictionThreshold;

	worldInfo->m_solverInfo.m_numIterations = getSolverInfo().m_numIterations;
	worldInfo->m_solverInfo.m_solverMode = getSolverInfo().m_solverMode;
	worldInfo->m_solverInfo.m_restingContactRestitutionThreshold = getSolverInfo().m_restingContactRestitutionThreshold;
	worldInfo->m_solverInfo.m_minimumSolverBatchSize = getSolverInfo().m_minimumSolverBatchSize;

	worldInfo->m_solverInfo.m_splitImpulse = getSolverInfo().m_splitImpulse;

	
#ifdef DRX3D_USE_DOUBLE_PRECISION
	tukk structType = "DynamicsWorldDoubleData";
#else   //DRX3D_USE_DOUBLE_PRECISION
	tukk structType = "DynamicsWorldFloatData";
#endif  //DRX3D_USE_DOUBLE_PRECISION
	serializer->finalizeChunk(chunk, structType, DRX3D_DYNAMICSWORLD_CODE, worldInfo);
}

void DiscreteDynamicsWorld::serialize(Serializer* serializer)
{
	serializer->startSerialization();

	serializeDynamicsWorldInfo(serializer);

	serializeCollisionObjects(serializer);

	serializeRigidBodies(serializer);

	serializeContactManifolds(serializer);

	serializer->finishSerialization();
}
