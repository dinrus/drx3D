#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorldMt.h>

//collision detection
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Dynamics/SimulationIslandManagerMt.h>
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

///
/// ConstraintSolverPoolMt
///

ConstraintSolverPoolMt::ThreadSolver* ConstraintSolverPoolMt::getAndLockThreadSolver()
{
	i32 i = 0;
#if DRX3D_THREADSAFE
	i = GetCurrentThreadIndex() % m_solvers.size();
#endif  // #if DRX3D_THREADSAFE

	while ( true )
	{
		ThreadSolver& solver = m_solvers[i];

		if ( solver.mutex.tryLock() )
		{
			return &solver;
		}

		// failed, try the next one
		i = ( i + 1 ) % m_solvers.size();
	}

	return NULL;
}

void ConstraintSolverPoolMt::init ( ConstraintSolver** solvers, i32 numSolvers )
{
	m_solverType = DRX3D_SEQUENTIAL_IMPULSE_SOLVER;
	m_solvers.resize ( numSolvers );

	for ( i32 i = 0; i < numSolvers; ++i )
	{
		m_solvers[i].solver = solvers[i];
	}

	if ( numSolvers > 0 )
	{
		m_solverType = solvers[0]->getSolverType();
	}
}

// create the solvers for me
ConstraintSolverPoolMt::ConstraintSolverPoolMt ( i32 numSolvers )
{
	AlignedObjectArray<ConstraintSolver*> solvers;
	solvers.reserve ( numSolvers );

	for ( i32 i = 0; i < numSolvers; ++i )
	{
		ConstraintSolver* solver = new SequentialImpulseConstraintSolver();
		solvers.push_back ( solver );
	}

	init ( &solvers[0], numSolvers );
}

// pass in fully constructed solvers (destructor will delete them)
ConstraintSolverPoolMt::ConstraintSolverPoolMt ( ConstraintSolver** solvers, i32 numSolvers )
{
	init ( solvers, numSolvers );
}

ConstraintSolverPoolMt::~ConstraintSolverPoolMt()
{
	// delete all solvers
	for ( i32 i = 0; i < m_solvers.size(); ++i )
	{
		ThreadSolver& solver = m_solvers[i];
		delete solver.solver;
		solver.solver = NULL;
	}
}

///solve a group of constraints
Scalar ConstraintSolverPoolMt::solveGroup ( CollisionObject2** bodies,
		i32 numBodies,
		PersistentManifold** manifolds,
		i32 numManifolds,
		TypedConstraint** constraints,
		i32 numConstraints,
		const ContactSolverInfo& info,
		IDebugDraw* debugDrawer,
		Dispatcher* dispatcher )
{
	ThreadSolver* ts = getAndLockThreadSolver();
	ts->solver->solveGroup ( bodies, numBodies, manifolds, numManifolds, constraints, numConstraints, info, debugDrawer, dispatcher );
	ts->mutex.unlock();
	return 0.0f;
}

void ConstraintSolverPoolMt::reset()
{
	for ( i32 i = 0; i < m_solvers.size(); ++i )
	{
		ThreadSolver& solver = m_solvers[i];
		solver.mutex.lock();
		solver.solver->reset();
		solver.mutex.unlock();
	}
}

///
/// DiscreteDynamicsWorldMt
///

DiscreteDynamicsWorldMt::DiscreteDynamicsWorldMt ( Dispatcher* dispatcher,
		BroadphaseInterface* pairCache,
		ConstraintSolverPoolMt* solverPool,
		ConstraintSolver* constraintSolverMt,
		CollisionConfiguration* collisionConfiguration )
		: DiscreteDynamicsWorld ( dispatcher, pairCache, solverPool, collisionConfiguration )
{
	if ( m_ownsIslandManager )
	{
		m_islandManager->~SimulationIslandManager();
		AlignedFree ( m_islandManager );
	}

	{
		uk mem = AlignedAlloc ( sizeof ( SimulationIslandManagerMt ), 16 );
		SimulationIslandManagerMt* im = new ( mem ) SimulationIslandManagerMt();
		im->setMinimumSolverBatchSize ( m_solverInfo.m_minimumSolverBatchSize );
		m_islandManager = im;
	}

	m_constraintSolverMt = constraintSolverMt;
}

DiscreteDynamicsWorldMt::~DiscreteDynamicsWorldMt()
{
}

void DiscreteDynamicsWorldMt::solveConstraints ( ContactSolverInfo& solverInfo )
{
	DRX3D_PROFILE ( "solveConstraints" );

	m_constraintSolver->prepareSolve ( getCollisionWorld()->getNumCollisionObjects(), getCollisionWorld()->getDispatcher()->getNumManifolds() );

	/// solve all the constraints for this island
	SimulationIslandManagerMt* im = static_cast<SimulationIslandManagerMt*> ( m_islandManager );
	SimulationIslandManagerMt::SolverParams solverParams;
	solverParams.m_solverPool = m_constraintSolver;
	solverParams.m_solverMt = m_constraintSolverMt;
	solverParams.m_solverInfo = &solverInfo;
	solverParams.m_debugDrawer = m_debugDrawer;
	solverParams.m_dispatcher = getCollisionWorld()->getDispatcher();
	im->buildAndProcessIslands ( getCollisionWorld()->getDispatcher(), getCollisionWorld(), m_constraints, solverParams );

	m_constraintSolver->allSolved ( solverInfo, m_debugDrawer );
}

struct UpdaterUnconstrainedMotion : public IParallelForBody
{
	Scalar timeStep;
	RigidBody** rigidBodies;

	void forLoop ( i32 iBegin, i32 iEnd ) const DRX3D_OVERRIDE
	{
		for ( i32 i = iBegin; i < iEnd; ++i )
{
	RigidBody* body = rigidBodies[i];

		if ( !body->isStaticOrKinematicObject() )
		{
			//don't integrate/update velocities here, it happens in the constraint solver
			body->applyDamping ( timeStep );
			body->predictIntegratedTransform2 ( timeStep, body->getInterpolationWorldTransform() );
		}
	}
	}
};

void DiscreteDynamicsWorldMt::predictUnconstraintMotion ( Scalar timeStep )
{
	DRX3D_PROFILE ( "predictUnconstraintMotion" );

	if ( m_nonStaticRigidBodies.size() > 0 )
	{
		UpdaterUnconstrainedMotion update;
		update.timeStep = timeStep;
		update.rigidBodies = &m_nonStaticRigidBodies[0];
		i32 grainSize = 50;  // num of iterations per task for task scheduler
		ParallelFor ( 0, m_nonStaticRigidBodies.size(), grainSize, update );
	}
}

void DiscreteDynamicsWorldMt::createPredictiveContacts ( Scalar timeStep )
{
	DRX3D_PROFILE ( "createPredictiveContacts" );
	releasePredictiveContacts();

	if ( m_nonStaticRigidBodies.size() > 0 )
	{
		UpdaterCreatePredictiveContacts update;
		update.world = this;
		update.timeStep = timeStep;
		update.rigidBodies = &m_nonStaticRigidBodies[0];
		i32 grainSize = 50;  // num of iterations per task for task scheduler
		ParallelFor ( 0, m_nonStaticRigidBodies.size(), grainSize, update );
	}
}

void DiscreteDynamicsWorldMt::integrateTransforms ( Scalar timeStep )
{
	DRX3D_PROFILE ( "integrateTransforms" );

	if ( m_nonStaticRigidBodies.size() > 0 )
	{
		UpdaterIntegrateTransforms update;
		update.world = this;
		update.timeStep = timeStep;
		update.rigidBodies = &m_nonStaticRigidBodies[0];
		i32 grainSize = 50;  // num of iterations per task for task scheduler
		ParallelFor ( 0, m_nonStaticRigidBodies.size(), grainSize, update );
	}
}

i32 DiscreteDynamicsWorldMt::stepSimulation ( Scalar timeStep, i32 maxSubSteps, Scalar fixedTimeStep )
{
	i32 numSubSteps = DiscreteDynamicsWorld::stepSimulation ( timeStep, maxSubSteps, fixedTimeStep );

	if ( ITaskScheduler* scheduler = GetTaskScheduler() )
	{
		// tell drx3D's threads to sleep, so other threads can run
		scheduler->sleepWorkerThreadsHint();
	}

	return numSubSteps;
}
