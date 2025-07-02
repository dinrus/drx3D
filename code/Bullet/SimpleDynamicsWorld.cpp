
#include <drx3D/Physics/Dynamics/SimpleDynamicsWorld.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>

/*
  Make sure this dummy function never changes so that it
  can be used by probes that are checking whether the
  library is actually installed.
*/
extern "C"
{
	void BulletDynamicsProbe();
	void BulletDynamicsProbe() {}
}

SimpleDynamicsWorld::SimpleDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration)
	: DynamicsWorld(dispatcher, pairCache, collisionConfiguration),
	  m_constraintSolver(constraintSolver),
	  m_ownsConstraintSolver(false),
	  m_gravity(0, 0, -10)
{
}

SimpleDynamicsWorld::~SimpleDynamicsWorld()
{
	if (m_ownsConstraintSolver)
		AlignedFree(m_constraintSolver);
}

i32 SimpleDynamicsWorld::stepSimulation(Scalar timeStep, i32 maxSubSteps, Scalar fixedTimeStep)
{
	(void)fixedTimeStep;
	(void)maxSubSteps;

	///apply gravity, predict motion
	predictUnconstraintMotion(timeStep);

	DispatcherInfo& dispatchInfo = getDispatchInfo();
	dispatchInfo.m_timeStep = timeStep;
	dispatchInfo.m_stepCount = 0;
	dispatchInfo.m_debugDraw = getDebugDrawer();

	///perform collision detection
	performDiscreteCollisionDetection();

	///solve contact constraints
	i32 numManifolds = m_dispatcher1->getNumManifolds();
	if (numManifolds)
	{
		PersistentManifold** manifoldPtr = ((Dispatcher*)m_dispatcher1)->getInternalManifoldPointer();

		ContactSolverInfo infoGlobal;
		infoGlobal.m_timeStep = timeStep;
		m_constraintSolver->prepareSolve(0, numManifolds);
		m_constraintSolver->solveGroup(&getCollisionObjectArray()[0], getNumCollisionObjects(), manifoldPtr, numManifolds, 0, 0, infoGlobal, m_debugDrawer, m_dispatcher1);
		m_constraintSolver->allSolved(infoGlobal, m_debugDrawer);
	}

	///integrate transforms
	integrateTransforms(timeStep);

	updateAabbs();

	synchronizeMotionStates();

	clearForces();

	return 1;
}

void SimpleDynamicsWorld::clearForces()
{
	///@todo: iterate over awake simulation islands!
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];

		RigidBody* body = RigidBody::upcast(colObj);
		if (body)
		{
			body->clearForces();
		}
	}
}

void SimpleDynamicsWorld::setGravity(const Vec3& gravity)
{
	m_gravity = gravity;
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body)
		{
			body->setGravity(gravity);
		}
	}
}

Vec3 SimpleDynamicsWorld::getGravity() const
{
	return m_gravity;
}

void SimpleDynamicsWorld::removeRigidBody(RigidBody* body)
{
	CollisionWorld::removeCollisionObject(body);
}

void SimpleDynamicsWorld::removeCollisionObject(CollisionObject2* collisionObject)
{
	RigidBody* body = RigidBody::upcast(collisionObject);
	if (body)
		removeRigidBody(body);
	else
		CollisionWorld::removeCollisionObject(collisionObject);
}

void SimpleDynamicsWorld::addRigidBody(RigidBody* body)
{
	body->setGravity(m_gravity);

	if (body->getCollisionShape())
	{
		addCollisionObject(body);
	}
}

void SimpleDynamicsWorld::addRigidBody(RigidBody* body, i32 group, i32 mask)
{
	body->setGravity(m_gravity);

	if (body->getCollisionShape())
	{
		addCollisionObject(body, group, mask);
	}
}

void SimpleDynamicsWorld::debugDrawWorld()
{
}

void SimpleDynamicsWorld::addAction(ActionInterface* action)
{
}

void SimpleDynamicsWorld::removeAction(ActionInterface* action)
{
}

void SimpleDynamicsWorld::updateAabbs()
{
	Transform2 predictedTrans;
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body)
		{
			if (body->isActive() && (!body->isStaticObject()))
			{
				Vec3 minAabb, maxAabb;
				colObj->getCollisionShape()->getAabb(colObj->getWorldTransform(), minAabb, maxAabb);
				BroadphaseInterface* bp = getBroadphase();
				bp->setAabb(body->getBroadphaseHandle(), minAabb, maxAabb, m_dispatcher1);
			}
		}
	}
}

void SimpleDynamicsWorld::integrateTransforms(Scalar timeStep)
{
	Transform2 predictedTrans;
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body)
		{
			if (body->isActive() && (!body->isStaticObject()))
			{
				body->predictIntegratedTransform2(timeStep, predictedTrans);
				body->proceedToTransform2(predictedTrans);
			}
		}
	}
}

void SimpleDynamicsWorld::predictUnconstraintMotion(Scalar timeStep)
{
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body)
		{
			if (!body->isStaticObject())
			{
				if (body->isActive())
				{
					body->applyGravity();
					body->integrateVelocities(timeStep);
					body->applyDamping(timeStep);
					body->predictIntegratedTransform2(timeStep, body->getInterpolationWorldTransform());
				}
			}
		}
	}
}

void SimpleDynamicsWorld::synchronizeMotionStates()
{
	///@todo: iterate over awake simulation islands!
	for (i32 i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		RigidBody* body = RigidBody::upcast(colObj);
		if (body && body->getMotionState())
		{
			if (body->getActivationState() != ISLAND_SLEEPING)
			{
				body->getMotionState()->setWorldTransform(body->getWorldTransform());
			}
		}
	}
}

void SimpleDynamicsWorld::setConstraintSolver(ConstraintSolver* solver)
{
	if (m_ownsConstraintSolver)
	{
		AlignedFree(m_constraintSolver);
	}
	m_ownsConstraintSolver = false;
	m_constraintSolver = solver;
}

ConstraintSolver* SimpleDynamicsWorld::getConstraintSolver()
{
	return m_constraintSolver;
}
