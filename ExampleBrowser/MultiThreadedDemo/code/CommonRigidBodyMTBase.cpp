
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>

#include <stdio.h>
#include <algorithm>

class CollisionShape;

#include "../CommonRigidBodyMTBase.h"
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/PoolAllocator.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcherMt.h>
#include <drx3D/Physics/Dynamics/SimulationIslandManagerMt.h>  // for setSplitIslands()
#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorldMt.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolverMt.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/NNCGConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/DantzigSolver.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/LemkeSolver.h>

static i32 gNumIslands = 0;
bool gAllowNestedParallelForLoops = false;

class Profiler
{
public:
	enum RecordType
	{
		kRecordInternalTimeStep,
		kRecordDispatchAllCollisionPairs,
		kRecordDispatchIslands,
		kRecordPredictUnconstrainedMotion,
		kRecordCreatePredictiveContacts,
		kRecordIntegrateTransforms,
		kRecordSolverTotal,
		kRecordSolverSetup,
		kRecordSolverIterations,
		kRecordSolverFinish,
		kRecordCount
	};

private:
	Clock mClock;

	struct Record
	{
		i32 mCallCount;
		zu64 mAccum;
		u32 mStartTime;
		u32 mHistory[8];

		void begin(u32 curTime)
		{
			mStartTime = curTime;
		}
		void end(u32 curTime)
		{
			u32 endTime = curTime;
			u32 elapsed = endTime - mStartTime;
			mAccum += elapsed;
			mHistory[mCallCount & 7] = elapsed;
			++mCallCount;
		}
		float getAverageTime() const
		{
			i32 count = d3Min(8, mCallCount);
			if (count > 0)
			{
				u32 sum = 0;
				for (i32 i = 0; i < count; ++i)
				{
					sum += mHistory[i];
				}
				float avg = float(sum) / float(count);
				return avg;
			}
			return 0.0;
		}
	};
	Record mRecords[kRecordCount];

public:
	void begin(RecordType rt)
	{
		mRecords[rt].begin(mClock.getTimeMicroseconds());
	}
	void end(RecordType rt)
	{
		mRecords[rt].end(mClock.getTimeMicroseconds());
	}
	float getAverageTime(RecordType rt) const
	{
		return mRecords[rt].getAverageTime();
	}
};

static Profiler gProfiler;

class ProfileHelper
{
	Profiler::RecordType mRecType;

public:
	ProfileHelper(Profiler::RecordType rt)
	{
		mRecType = rt;
		gProfiler.begin(mRecType);
	}
	~ProfileHelper()
	{
		gProfiler.end(mRecType);
	}
};

static void profileBeginCallback(DynamicsWorld* world, Scalar timeStep)
{
	gProfiler.begin(Profiler::kRecordInternalTimeStep);
}

static void profileEndCallback(DynamicsWorld* world, Scalar timeStep)
{
	gProfiler.end(Profiler::kRecordInternalTimeStep);
}

class MySequentialImpulseConstraintSolverMt : public SequentialImpulseConstraintSolverMt
{
	typedef SequentialImpulseConstraintSolverMt ParentClass;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MySequentialImpulseConstraintSolverMt() {}

	// for profiling
	virtual Scalar solveGroupCacheFriendlySetup(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordSolverSetup);
		Scalar ret = ParentClass::solveGroupCacheFriendlySetup(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
		return ret;
	}
	virtual Scalar solveGroupCacheFriendlyIterations(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordSolverIterations);
		Scalar ret = ParentClass::solveGroupCacheFriendlyIterations(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
		return ret;
	}
	virtual Scalar solveGroupCacheFriendlyFinish(CollisionObject2** bodies, i32 numBodies, const ContactSolverInfo& infoGlobal) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordSolverFinish);
		Scalar ret = ParentClass::solveGroupCacheFriendlyFinish(bodies, numBodies, infoGlobal);
		return ret;
	}
	virtual Scalar solveGroup(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordSolverTotal);
		Scalar ret = ParentClass::solveGroup(bodies, numBodies, manifold, numManifolds, constraints, numConstraints, info, debugDrawer, dispatcher);
		return ret;
	}
};

///
/// MyCollisionDispatcher -- subclassed for profiling purposes
///
class MyCollisionDispatcher : public CollisionDispatcherMt
{
	typedef CollisionDispatcherMt ParentClass;

public:
	MyCollisionDispatcher(CollisionConfiguration* config, i32 grainSize) : CollisionDispatcherMt(config, grainSize)
	{
	}

	virtual void dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& info, Dispatcher* dispatcher) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordDispatchAllCollisionPairs);
		ParentClass::dispatchAllCollisionPairs(pairCache, info, dispatcher);
	}
};

///
/// myParallelIslandDispatch -- wrap default parallel dispatch for profiling and to get the number of simulation islands
//
void myParallelIslandDispatch(AlignedObjectArray<SimulationIslandManagerMt::Island*>* islandsPtr, const SimulationIslandManagerMt::SolverParams& solverParams)
{
	ProfileHelper prof(Profiler::kRecordDispatchIslands);
	gNumIslands = islandsPtr->size();
	SimulationIslandManagerMt::parallelIslandDispatch(islandsPtr, solverParams);
}

///
/// MyDiscreteDynamicsWorld -- subclassed for profiling purposes
///
ATTRIBUTE_ALIGNED16(class)
MyDiscreteDynamicsWorld : public DiscreteDynamicsWorldMt
{
	typedef DiscreteDynamicsWorldMt ParentClass;

protected:
	virtual void predictUnconstraintMotion(Scalar timeStep) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordPredictUnconstrainedMotion);
		ParentClass::predictUnconstraintMotion(timeStep);
	}
	virtual void createPredictiveContacts(Scalar timeStep) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordCreatePredictiveContacts);
		ParentClass::createPredictiveContacts(timeStep);
	}
	virtual void integrateTransforms(Scalar timeStep) DRX3D_OVERRIDE
	{
		ProfileHelper prof(Profiler::kRecordIntegrateTransforms);
		ParentClass::integrateTransforms(timeStep);
	}

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MyDiscreteDynamicsWorld(Dispatcher * dispatcher,
							BroadphaseInterface * pairCache,
							ConstraintSolverPoolMt * constraintSolver,
							SequentialImpulseConstraintSolverMt * constraintSolverMt,
							CollisionConfiguration * collisionConfiguration) : DiscreteDynamicsWorldMt(dispatcher, pairCache, constraintSolver, constraintSolverMt, collisionConfiguration)
	{
		SimulationIslandManagerMt* islandMgr = static_cast<SimulationIslandManagerMt*>(m_islandManager);
		islandMgr->setIslandDispatchFunction(myParallelIslandDispatch);
	}
};

ConstraintSolver* createSolverByType(SolverType t)
{
	MLCPSolverInterface* mlcpSolver = NULL;
	switch (t)
	{
		case SOLVER_TYPE_SEQUENTIAL_IMPULSE:
			return new SequentialImpulseConstraintSolver();
		case SOLVER_TYPE_SEQUENTIAL_IMPULSE_MT:
			return new MySequentialImpulseConstraintSolverMt();
		case SOLVER_TYPE_NNCG:
			return new NNCGConstraintSolver();
		case SOLVER_TYPE_MLCP_PGS:
			mlcpSolver = new SolveProjectedGaussSeidel();
			break;
		case SOLVER_TYPE_MLCP_DANTZIG:
			mlcpSolver = new DantzigSolver();
			break;
		case SOLVER_TYPE_MLCP_LEMKE:
			mlcpSolver = new LemkeSolver();
			break;
		default:
		{
		}
	}
	if (mlcpSolver)
	{
		return new MLCPSolver(mlcpSolver);
	}
	return NULL;
}

///
/// btTaskSchedulerManager -- manage a number of task schedulers so we can switch between them
///
class TaskSchedulerManager
{
	AlignedObjectArray<ITaskScheduler*> m_taskSchedulers;
	AlignedObjectArray<ITaskScheduler*> m_allocatedTaskSchedulers;

public:
	TaskSchedulerManager() {}
	void init()
	{
		addTaskScheduler(GetSequentialTaskScheduler());
#if DRX3D_THREADSAFE
		if (ITaskScheduler* ts = CreateDefaultTaskScheduler())
		{
			m_allocatedTaskSchedulers.push_back(ts);
			addTaskScheduler(ts);
		}
		addTaskScheduler(GetOpenMPTaskScheduler());
		addTaskScheduler(GetTBBTaskScheduler());
		addTaskScheduler(GetPPLTaskScheduler());
		if (getNumTaskSchedulers() > 1)
		{
			// prefer a non-sequential scheduler if available
			SetTaskScheduler(m_taskSchedulers[1]);
		}
		else
		{
			SetTaskScheduler(m_taskSchedulers[0]);
		}
#endif  // #if DRX3D_THREADSAFE
	}
	void shutdown()
	{
		for (i32 i = 0; i < m_allocatedTaskSchedulers.size(); ++i)
		{
			delete m_allocatedTaskSchedulers[i];
		}
		m_allocatedTaskSchedulers.clear();
	}

	void addTaskScheduler(ITaskScheduler* ts)
	{
		if (ts)
		{
#if DRX3D_THREADSAFE
			// if initial number of threads is 0 or 1,
			if (ts->getNumThreads() <= 1)
			{
				// for OpenMP, TBB, PPL set num threads to number of logical cores
				ts->setNumThreads(ts->getMaxNumThreads());
			}
#endif  // #if DRX3D_THREADSAFE
			m_taskSchedulers.push_back(ts);
		}
	}
	i32 getNumTaskSchedulers() const { return m_taskSchedulers.size(); }
	ITaskScheduler* getTaskScheduler(i32 i) { return m_taskSchedulers[i]; }
};

static TaskSchedulerManager gTaskSchedulerMgr;

#if DRX3D_THREADSAFE
static bool gMultithreadedWorld = true;
static bool gDisplayProfileInfo = true;
static SolverType gSolverType = SOLVER_TYPE_SEQUENTIAL_IMPULSE_MT;
#else
static bool gMultithreadedWorld = false;
static bool gDisplayProfileInfo = false;
static SolverType gSolverType = SOLVER_TYPE_SEQUENTIAL_IMPULSE;
#endif
static i32 gSolverMode = SOLVER_SIMD |
						 SOLVER_USE_WARMSTARTING |
						 // SOLVER_RANDMIZE_ORDER |
						 // SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS |
						 // SOLVER_USE_2_FRICTION_DIRECTIONS |
						 0;
static Scalar gSliderSolverIterations = 10.0f;                                                        // should be i32
static Scalar gSliderNumThreads = 1.0f;                                                               // should be i32
static Scalar gSliderIslandBatchingThreshold = 0.0f;                                                  // should be i32
static Scalar gSliderMinBatchSize = Scalar(SequentialImpulseConstraintSolverMt::s_minBatchSize);  // should be i32
static Scalar gSliderMaxBatchSize = Scalar(SequentialImpulseConstraintSolverMt::s_maxBatchSize);  // should be i32
static Scalar gSliderLeastSquaresResidualThreshold = 0.0f;

////////////////////////////////////
CommonRigidBodyMTBase::CommonRigidBodyMTBase(struct GUIHelperInterface* helper)
	: m_broadphase(0),
	  m_dispatcher(0),
	  m_solver(0),
	  m_collisionConfiguration(0),
	  m_dynamicsWorld(0),
	  m_pickedBody(0),
	  m_pickedConstraint(0),
	  m_guiHelper(helper)
{
	m_multithreadedWorld = false;
	m_multithreadCapable = false;
	if (gTaskSchedulerMgr.getNumTaskSchedulers() == 0)
	{
		gTaskSchedulerMgr.init();
	}
}

CommonRigidBodyMTBase::~CommonRigidBodyMTBase()
{
}

static void boolPtrButtonCallback(i32 buttonId, bool buttonState, uk userPointer)
{
	if (bool* val = static_cast<bool*>(userPointer))
	{
		*val = !*val;
	}
}

static void toggleSolverModeCallback(i32 buttonId, bool buttonState, uk userPointer)
{
	if (buttonState)
	{
		gSolverMode |= buttonId;
	}
	else
	{
		gSolverMode &= ~buttonId;
	}
	if (CommonRigidBodyMTBase* crb = reinterpret_cast<CommonRigidBodyMTBase*>(userPointer))
	{
		if (crb->m_dynamicsWorld)
		{
			crb->m_dynamicsWorld->getSolverInfo().m_solverMode = gSolverMode;
		}
	}
}

void setSolverTypeComboBoxCallback(i32 combobox, tukk item, uk userPointer)
{
	tukk* items = static_cast<tukk*>(userPointer);
	for (i32 i = 0; i < SOLVER_TYPE_COUNT; ++i)
	{
		if (strcmp(item, items[i]) == 0)
		{
			gSolverType = static_cast<SolverType>(i);
			break;
		}
	}
}

static void setNumThreads(i32 numThreads)
{
#if DRX3D_THREADSAFE
	i32 newNumThreads = (std::min)(numThreads, i32(DRX3D_MAX_THREAD_COUNT));
	i32 oldNumThreads = GetTaskScheduler()->getNumThreads();
	// only call when the thread count is different
	if (newNumThreads != oldNumThreads)
	{
		GetTaskScheduler()->setNumThreads(newNumThreads);
	}
#endif  // #if DRX3D_THREADSAFE
}

void setTaskSchedulerComboBoxCallback(i32 combobox, tukk item, uk userPointer)
{
#if DRX3D_THREADSAFE
	tukk* items = static_cast<tukk*>(userPointer);
	for (i32 i = 0; i < 20; ++i)
	{
		if (strcmp(item, items[i]) == 0)
		{
			// change the task scheduler
			ITaskScheduler* ts = gTaskSchedulerMgr.getTaskScheduler(i);
			SetTaskScheduler(ts);
			gSliderNumThreads = float(ts->getNumThreads());
			break;
		}
	}
#endif  // #if DRX3D_THREADSAFE
}

void setBatchingMethodComboBoxCallback(i32 combobox, tukk item, uk userPointer)
{
#if DRX3D_THREADSAFE
	tukk* items = static_cast<tukk*>(userPointer);
	for (i32 i = 0; i < BatchedConstraints::BATCHING_METHOD_COUNT; ++i)
	{
		if (strcmp(item, items[i]) == 0)
		{
			// change the task scheduler
			SequentialImpulseConstraintSolverMt::s_contactBatchingMethod = static_cast<BatchedConstraints::BatchingMethod>(i);
			break;
		}
	}
#endif  // #if DRX3D_THREADSAFE
}

static void setThreadCountCallback(float val, uk userPtr)
{
#if DRX3D_THREADSAFE
	setNumThreads(i32(gSliderNumThreads));
	gSliderNumThreads = float(GetTaskScheduler()->getNumThreads());
#endif  // #if DRX3D_THREADSAFE
}

static void setSolverIterationCountCallback(float val, uk userPtr)
{
	if (DiscreteDynamicsWorld* world = reinterpret_cast<DiscreteDynamicsWorld*>(userPtr))
	{
		world->getSolverInfo().m_numIterations = d3Max(1, i32(gSliderSolverIterations));
	}
}

static void setLargeIslandManifoldCountCallback(float val, uk userPtr)
{
	SequentialImpulseConstraintSolverMt::s_minimumContactManifoldsForBatching = i32(gSliderIslandBatchingThreshold);
}

static void setMinBatchSizeCallback(float val, uk userPtr)
{
	gSliderMaxBatchSize = (std::max)(gSliderMinBatchSize, gSliderMaxBatchSize);
	SequentialImpulseConstraintSolverMt::s_minBatchSize = i32(gSliderMinBatchSize);
	SequentialImpulseConstraintSolverMt::s_maxBatchSize = i32(gSliderMaxBatchSize);
}

static void setMaxBatchSizeCallback(float val, uk userPtr)
{
	gSliderMinBatchSize = (std::min)(gSliderMinBatchSize, gSliderMaxBatchSize);
	SequentialImpulseConstraintSolverMt::s_minBatchSize = i32(gSliderMinBatchSize);
	SequentialImpulseConstraintSolverMt::s_maxBatchSize = i32(gSliderMaxBatchSize);
}

static void setLeastSquaresResidualThresholdCallback(float val, uk userPtr)
{
	if (DiscreteDynamicsWorld* world = reinterpret_cast<DiscreteDynamicsWorld*>(userPtr))
	{
		world->getSolverInfo().m_leastSquaresResidualThreshold = gSliderLeastSquaresResidualThreshold;
	}
}

void CommonRigidBodyMTBase::createEmptyDynamicsWorld()
{
	gNumIslands = 0;
	m_solverType = gSolverType;
#if DRX3D_THREADSAFE
	Assert(GetTaskScheduler() != NULL);
	if (NULL != GetTaskScheduler() && gTaskSchedulerMgr.getNumTaskSchedulers() > 1)
	{
		m_multithreadCapable = true;
	}
#endif
	if (gMultithreadedWorld)
	{
#if DRX3D_THREADSAFE
		m_dispatcher = NULL;
		DefaultCollisionConstructionInfo cci;
		cci.m_defaultMaxPersistentManifoldPoolSize = 80000;
		cci.m_defaultMaxCollisionAlgorithmPoolSize = 80000;
		m_collisionConfiguration = new DefaultCollisionConfiguration(cci);

		m_dispatcher = new MyCollisionDispatcher(m_collisionConfiguration, 40);
		m_broadphase = new DbvtBroadphase();

		ConstraintSolverPoolMt* solverPool;
		{
			SolverType poolSolverType = m_solverType;
			if (poolSolverType == SOLVER_TYPE_SEQUENTIAL_IMPULSE_MT)
			{
				// pool solvers shouldn't be parallel solvers, we don't allow that kind of
				// nested parallelism because of performance issues
				poolSolverType = SOLVER_TYPE_SEQUENTIAL_IMPULSE;
			}
			ConstraintSolver* solvers[DRX3D_MAX_THREAD_COUNT];
			i32 maxThreadCount = DRX3D_MAX_THREAD_COUNT;
			for (i32 i = 0; i < maxThreadCount; ++i)
			{
				solvers[i] = createSolverByType(poolSolverType);
			}
			solverPool = new ConstraintSolverPoolMt(solvers, maxThreadCount);
			m_solver = solverPool;
		}
		SequentialImpulseConstraintSolverMt* solverMt = NULL;
		if (m_solverType == SOLVER_TYPE_SEQUENTIAL_IMPULSE_MT)
		{
			solverMt = new MySequentialImpulseConstraintSolverMt();
		}
		DiscreteDynamicsWorld* world = new MyDiscreteDynamicsWorld(m_dispatcher, m_broadphase, solverPool, solverMt, m_collisionConfiguration);
		m_dynamicsWorld = world;
		m_multithreadedWorld = true;
		Assert(GetTaskScheduler() != NULL);
#endif  // #if DRX3D_THREADSAFE
	}
	else
	{
		// single threaded world
		m_multithreadedWorld = false;

		///collision configuration contains default setup for memory, collision setup
		m_collisionConfiguration = new DefaultCollisionConfiguration();
		//m_collisionConfiguration->setConvexConvexMultipointIterations();

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

		m_broadphase = new DbvtBroadphase();

		SolverType solverType = m_solverType;
		if (solverType == SOLVER_TYPE_SEQUENTIAL_IMPULSE_MT)
		{
			// using the parallel solver with the single-threaded world works, but is
			// disabled here to avoid confusion
			solverType = SOLVER_TYPE_SEQUENTIAL_IMPULSE;
		}
		m_solver = createSolverByType(solverType);

		m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	}
	m_dynamicsWorld->setInternalTickCallback(profileBeginCallback, NULL, true);
	m_dynamicsWorld->setInternalTickCallback(profileEndCallback, NULL, false);
	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	m_dynamicsWorld->getSolverInfo().m_solverMode = gSolverMode;
	m_dynamicsWorld->getSolverInfo().m_numIterations = d3Max(1, i32(gSliderSolverIterations));
	createDefaultParameters();
}

void CommonRigidBodyMTBase::createDefaultParameters()
{
	if (m_multithreadCapable)
	{
		// create a button to toggle multithreaded world
		ButtonParams button("Multithreaded world enable", 0, true);
		bool* ptr = &gMultithreadedWorld;
		button.m_initialState = *ptr;
		button.m_userPointer = ptr;
		button.m_callback = boolPtrButtonCallback;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		// create a button to toggle profile printing
		ButtonParams button("Display solver info", 0, true);
		bool* ptr = &gDisplayProfileInfo;
		button.m_initialState = *ptr;
		button.m_userPointer = ptr;
		button.m_callback = boolPtrButtonCallback;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}

	{
		// create a combo box for selecting the solver type
		static tukk sSolverTypeComboBoxItems[SOLVER_TYPE_COUNT];
		for (i32 i = 0; i < SOLVER_TYPE_COUNT; ++i)
		{
			SolverType solverType = static_cast<SolverType>(i);
			sSolverTypeComboBoxItems[i] = getSolverTypeName(solverType);
		}
		ComboBoxParams comboParams;
		comboParams.m_userPointer = sSolverTypeComboBoxItems;
		comboParams.m_numItems = SOLVER_TYPE_COUNT;
		comboParams.m_startItem = gSolverType;
		comboParams.m_items = sSolverTypeComboBoxItems;
		comboParams.m_callback = setSolverTypeComboBoxCallback;
		m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
	}
	{
		// a slider for the number of solver iterations
		SliderParams slider("Solver iterations", &gSliderSolverIterations);
		slider.m_minVal = 1.0f;
		slider.m_maxVal = 30.0f;
		slider.m_callback = setSolverIterationCountCallback;
		slider.m_userPointer = m_dynamicsWorld;
		slider.m_clampToIntegers = true;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		// a slider for the solver leastSquaresResidualThreshold (used to run fewer solver iterations when convergence is good)
		SliderParams slider("Solver residual thresh", &gSliderLeastSquaresResidualThreshold);
		slider.m_minVal = 0.0f;
		slider.m_maxVal = 0.25f;
		slider.m_callback = setLeastSquaresResidualThresholdCallback;
		slider.m_userPointer = m_dynamicsWorld;
		slider.m_clampToIntegers = false;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		ButtonParams button("Solver use SIMD", 0, true);
		button.m_buttonId = SOLVER_SIMD;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		ButtonParams button("Solver randomize order", 0, true);
		button.m_buttonId = SOLVER_RANDMIZE_ORDER;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		ButtonParams button("Solver interleave contact/friction", 0, true);
		button.m_buttonId = SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		ButtonParams button("Solver 2 friction directions", 0, true);
		button.m_buttonId = SOLVER_USE_2_FRICTION_DIRECTIONS;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		ButtonParams button("Solver friction dir caching", 0, true);
		button.m_buttonId = SOLVER_ENABLE_FRICTION_DIRECTION_CACHING;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	{
		ButtonParams button("Solver warmstarting", 0, true);
		button.m_buttonId = SOLVER_USE_WARMSTARTING;
		button.m_initialState = !!(gSolverMode & button.m_buttonId);
		button.m_callback = toggleSolverModeCallback;
		button.m_userPointer = this;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}
	if (m_multithreadedWorld)
	{
#if DRX3D_THREADSAFE
		if (gTaskSchedulerMgr.getNumTaskSchedulers() >= 1)
		{
			// create a combo box for selecting the task scheduler
			i32k maxNumTaskSchedulers = 20;
			static tukk sTaskSchedulerComboBoxItems[maxNumTaskSchedulers];
			i32 startingItem = 0;
			for (i32 i = 0; i < gTaskSchedulerMgr.getNumTaskSchedulers(); ++i)
			{
				sTaskSchedulerComboBoxItems[i] = gTaskSchedulerMgr.getTaskScheduler(i)->getName();
				if (gTaskSchedulerMgr.getTaskScheduler(i) == GetTaskScheduler())
				{
					startingItem = i;
				}
			}
			ComboBoxParams comboParams;
			comboParams.m_userPointer = sTaskSchedulerComboBoxItems;
			comboParams.m_numItems = gTaskSchedulerMgr.getNumTaskSchedulers();
			comboParams.m_startItem = startingItem;
			comboParams.m_items = sTaskSchedulerComboBoxItems;
			comboParams.m_callback = setTaskSchedulerComboBoxCallback;
			m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
		}
		{
			// if slider has not been set yet (by another demo),
			if (gSliderNumThreads <= 1.0f)
			{
				// create a slider to set the number of threads to use
				i32 numThreads = GetTaskScheduler()->getNumThreads();
				gSliderNumThreads = float(numThreads);
			}
			i32 maxNumThreads = GetTaskScheduler()->getMaxNumThreads();
			SliderParams slider("Thread count", &gSliderNumThreads);
			slider.m_minVal = 1.0f;
			slider.m_maxVal = float(maxNumThreads);
			slider.m_callback = setThreadCountCallback;
			slider.m_clampToIntegers = true;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}
		{
			// a slider for the number of manifolds an island needs to be too large for parallel dispatch
			if (gSliderIslandBatchingThreshold < 1.0)
			{
				gSliderIslandBatchingThreshold = float(SequentialImpulseConstraintSolverMt::s_minimumContactManifoldsForBatching);
			}
			SliderParams slider("IslandBatchThresh", &gSliderIslandBatchingThreshold);
			slider.m_minVal = 1.0f;
			slider.m_maxVal = 2000.0f;
			slider.m_callback = setLargeIslandManifoldCountCallback;
			slider.m_userPointer = NULL;
			slider.m_clampToIntegers = true;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}
		{
			// create a combo box for selecting the batching method
			static tukk sBatchingMethodComboBoxItems[BatchedConstraints::BATCHING_METHOD_COUNT];
			{
				sBatchingMethodComboBoxItems[BatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_2D] = "Batching: 2D Grid";
				sBatchingMethodComboBoxItems[BatchedConstraints::BATCHING_METHOD_SPATIAL_GRID_3D] = "Batching: 3D Grid";
			};
			ComboBoxParams comboParams;
			comboParams.m_userPointer = sBatchingMethodComboBoxItems;
			comboParams.m_numItems = BatchedConstraints::BATCHING_METHOD_COUNT;
			comboParams.m_startItem = static_cast<i32>(SequentialImpulseConstraintSolverMt::s_contactBatchingMethod);
			comboParams.m_items = sBatchingMethodComboBoxItems;
			comboParams.m_callback = setBatchingMethodComboBoxCallback;
			m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
		}
		{
			// a slider for the sequentialImpulseConstraintSolverMt min batch size (when batching)
			SliderParams slider("d3Min batch size", &gSliderMinBatchSize);
			slider.m_minVal = 1.0f;
			slider.m_maxVal = 1000.0f;
			slider.m_callback = setMinBatchSizeCallback;
			slider.m_userPointer = NULL;
			slider.m_clampToIntegers = true;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}
		{
			// a slider for the sequentialImpulseConstraintSolverMt max batch size (when batching)
			SliderParams slider("d3Max batch size", &gSliderMaxBatchSize);
			slider.m_minVal = 1.0f;
			slider.m_maxVal = 1000.0f;
			slider.m_callback = setMaxBatchSizeCallback;
			slider.m_userPointer = NULL;
			slider.m_clampToIntegers = true;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}
		{
			// create a button to toggle debug drawing of batching visualization
			ButtonParams button("Visualize batching", 0, true);
			bool* ptr = &BatchedConstraints::s_debugDrawBatches;
			button.m_initialState = *ptr;
			button.m_userPointer = ptr;
			button.m_callback = boolPtrButtonCallback;
			m_guiHelper->getParameterInterface()->registerButtonParameter(button);
		}
		{
			ButtonParams button("Allow Nested ParallelFor", 0, true);
			button.m_initialState = SequentialImpulseConstraintSolverMt::s_allowNestedParallelForLoops;
			button.m_userPointer = &SequentialImpulseConstraintSolverMt::s_allowNestedParallelForLoops;
			button.m_callback = boolPtrButtonCallback;
			m_guiHelper->getParameterInterface()->registerButtonParameter(button);
		}
#endif  // #if DRX3D_THREADSAFE
	}
}

void CommonRigidBodyMTBase::drawScreenText()
{
	char msg[1024];
	i32 xCoord = 400;
	i32 yCoord = 30;
	i32 yStep = 30;
	i32 indent = 30;
	if (m_solverType != gSolverType)
	{
		sprintf(msg, "restart example to change solver type");
		m_guiHelper->getAppInterface()->drawText(msg, 300, yCoord, 0.4f);
		yCoord += yStep;
	}
	if (m_multithreadCapable)
	{
		if (m_multithreadedWorld != gMultithreadedWorld)
		{
			sprintf(msg, "restart example to begin in %s mode",
					gMultithreadedWorld ? "multithreaded" : "single threaded");
			m_guiHelper->getAppInterface()->drawText(msg, 300, yCoord, 0.4f);
			yCoord += yStep;
		}
	}
	if (gDisplayProfileInfo)
	{
		if (m_multithreadedWorld)
		{
#if DRX3D_THREADSAFE
			i32 numManifolds = m_dispatcher->getNumManifolds();
			i32 numContacts = 0;
			for (i32 i = 0; i < numManifolds; ++i)
			{
				const PersistentManifold* man = m_dispatcher->getManifoldByIndexInternal(i);
				numContacts += man->getNumContacts();
			}
			tukk mtApi = GetTaskScheduler()->getName();
			sprintf(msg, "islands=%d bodies=%d manifolds=%d contacts=%d [%s] threads=%d",
					gNumIslands,
					m_dynamicsWorld->getNumCollisionObjects(),
					numManifolds,
					numContacts,
					mtApi,
					GetTaskScheduler()->getNumThreads());
			m_guiHelper->getAppInterface()->drawText(msg, 100, yCoord, 0.4f);
			yCoord += yStep;
#endif  // #if DRX3D_THREADSAFE
		}
		{
			i32 sm = gSolverMode;
			sprintf(msg, "solver %s mode [%s%s%s%s%s%s]",
					getSolverTypeName(m_solverType),
					sm & SOLVER_SIMD ? "SIMD" : "",
					sm & SOLVER_RANDMIZE_ORDER ? " randomize" : "",
					sm & SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS ? " interleave" : "",
					sm & SOLVER_USE_2_FRICTION_DIRECTIONS ? " friction2x" : "",
					sm & SOLVER_ENABLE_FRICTION_DIRECTION_CACHING ? " frictionDirCaching" : "",
					sm & SOLVER_USE_WARMSTARTING ? " warm" : "");
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;
		}
		sprintf(msg, "internalSimStep %5.3f ms",
				gProfiler.getAverageTime(Profiler::kRecordInternalTimeStep) * 0.001f);
		m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
		yCoord += yStep;

		if (m_multithreadedWorld)
		{
			sprintf(msg,
					"DispatchCollisionPairs %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordDispatchAllCollisionPairs) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"SolveAllIslands %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordDispatchIslands) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"SolverTotal %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordSolverTotal) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"SolverSetup %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordSolverSetup) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord + indent, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"SolverIterations %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordSolverIterations) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord + indent, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"SolverFinish %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordSolverFinish) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord + indent, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"PredictUnconstrainedMotion %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordPredictUnconstrainedMotion) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"CreatePredictiveContacts %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordCreatePredictiveContacts) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;

			sprintf(msg,
					"IntegrateTransforms %5.3f ms",
					gProfiler.getAverageTime(Profiler::kRecordIntegrateTransforms) * 0.001f);
			m_guiHelper->getAppInterface()->drawText(msg, xCoord, yCoord, 0.4f);
			yCoord += yStep;
		}
	}
}

void CommonRigidBodyMTBase::physicsDebugDraw(i32 debugFlags)
{
	if (m_dynamicsWorld && m_dynamicsWorld->getDebugDrawer())
	{
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(debugFlags);
		m_dynamicsWorld->debugDrawWorld();
	}
	drawScreenText();
}

void CommonRigidBodyMTBase::renderScene()
{
	m_guiHelper->syncPhysicsToGraphics(m_dynamicsWorld);
	m_guiHelper->render(m_dynamicsWorld);
	drawScreenText();
}
