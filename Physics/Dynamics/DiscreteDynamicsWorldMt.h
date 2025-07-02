#ifndef DRX3D_DISCRETE_DYNAMICS_WORLD_MT_H
#define DRX3D_DISCRETE_DYNAMICS_WORLD_MT_H

#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/SimulationIslandManagerMt.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ConstraintSolver.h>

///
/// ConstraintSolverPoolMt - masquerades as a constraint solver, but really it is a threadsafe pool of them.
///
///  Each solver in the pool is protected by a mutex.  When solveGroup is called from a thread,
///  the pool looks for a solver that isn't being used by another thread, locks it, and dispatches the
///  call to the solver.
///  So long as there are at least as many solvers as there are hardware threads, it should never need to
///  spin wait.
///
class ConstraintSolverPoolMt : public ConstraintSolver
{
public:
	// create the solvers for me
	explicit ConstraintSolverPoolMt(i32 numSolvers);

	// pass in fully constructed solvers (destructor will delete them)
	ConstraintSolverPoolMt(ConstraintSolver** solvers, i32 numSolvers);

	virtual ~ConstraintSolverPoolMt();

	///solve a group of constraints
	virtual Scalar solveGroup(CollisionObject2** bodies,
								i32 numBodies,
								PersistentManifold** manifolds,
								i32 numManifolds,
								TypedConstraint** constraints,
								i32 numConstraints,
								const ContactSolverInfo& info,
								IDebugDraw* debugDrawer,
								Dispatcher* dispatcher) DRX3D_OVERRIDE;

	virtual void reset() DRX3D_OVERRIDE;
	virtual ConstraintSolverType getSolverType() const DRX3D_OVERRIDE { return m_solverType; }

private:
	const static size_t kCacheLineSize = 128;
	struct ThreadSolver
	{
		ConstraintSolver* solver;
		SpinMutex mutex;
		char _cachelinePadding[kCacheLineSize - sizeof(SpinMutex) - sizeof(uk )];  // keep mutexes from sharing a cache line
	};
	AlignedObjectArray<ThreadSolver> m_solvers;
	ConstraintSolverType m_solverType;

	ThreadSolver* getAndLockThreadSolver();
	void init(ConstraintSolver** solvers, i32 numSolvers);
};

///
/// DiscreteDynamicsWorldMt -- a version of DiscreteDynamicsWorld with some minor changes to support
///                              solving simulation islands on multiple threads.
///
///  Should function exactly like DiscreteDynamicsWorld.
///  Also 3 methods that iterate over all of the rigidbodies can run in parallel:
///     - predictUnconstraintMotion
///     - integrateTransforms
///     - createPredictiveContacts
///
ATTRIBUTE_ALIGNED16(class)
DiscreteDynamicsWorldMt : public DiscreteDynamicsWorld
{
protected:
	ConstraintSolver* m_constraintSolverMt;

	virtual void solveConstraints(ContactSolverInfo & solverInfo) DRX3D_OVERRIDE;

	virtual void predictUnconstraintMotion(Scalar timeStep) DRX3D_OVERRIDE;

	struct UpdaterCreatePredictiveContacts : public IParallelForBody
	{
		Scalar timeStep;
		RigidBody** rigidBodies;
		DiscreteDynamicsWorldMt* world;

		void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
		{
			world->createPredictiveContactsInternal(&rigidBodies[iBegin], iEnd - iBegin, timeStep);
		}
	};
	virtual void createPredictiveContacts(Scalar timeStep) DRX3D_OVERRIDE;

	struct UpdaterIntegrateTransforms : public IParallelForBody
	{
		Scalar timeStep;
		RigidBody** rigidBodies;
		DiscreteDynamicsWorldMt* world;

		void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
		{
			world->integrateTransformsInternal(&rigidBodies[iBegin], iEnd - iBegin, timeStep);
		}
	};
	virtual void integrateTransforms(Scalar timeStep) DRX3D_OVERRIDE;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	DiscreteDynamicsWorldMt(Dispatcher * dispatcher,
							  BroadphaseInterface * pairCache,
							  ConstraintSolverPoolMt * solverPool,        // Note this should be a solver-pool for multi-threading
							  ConstraintSolver * constraintSolverMt,      // single multi-threaded solver for large islands (or NULL)
							  CollisionConfiguration * collisionConfiguration);
	virtual ~DiscreteDynamicsWorldMt();

	virtual i32 stepSimulation(Scalar timeStep, i32 maxSubSteps, Scalar fixedTimeStep) DRX3D_OVERRIDE;
};

#endif  //DRX3D_DISCRETE_DYNAMICS_WORLD_H
