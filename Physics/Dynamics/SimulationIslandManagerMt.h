#ifndef DRX3D_SIMULATION_ISLAND_MANAGER_MT_H
#define DRX3D_SIMULATION_ISLAND_MANAGER_MT_H

#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>

class TypedConstraint;
class ConstraintSolver;
struct ContactSolverInfo;
class IDebugDraw;

///
/// SimulationIslandManagerMt -- Multithread capable version of SimulationIslandManager
///                       Splits the world up into islands which can be solved in parallel.
///                       In order to solve islands in parallel, an IslandDispatch function
///                       must be provided which will dispatch calls to multiple threads.
///                       The amount of parallelism that can be achieved depends on the number
///                       of islands. If only a single island exists, then no parallelism is
///                       possible.
///
class SimulationIslandManagerMt : public SimulationIslandManager
{
public:
	struct Island
	{
		// a simulation island consisting of bodies, manifolds and constraints,
		// to be passed into a constraint solver.
		AlignedObjectArray<CollisionObject2*> bodyArray;
		AlignedObjectArray<PersistentManifold*> manifoldArray;
		AlignedObjectArray<TypedConstraint*> constraintArray;
		i32 id;  // island id
		bool isSleeping;

		void append(const Island& other);  // add bodies, manifolds, constraints to my own
	};
	struct SolverParams
	{
		ConstraintSolver* m_solverPool;
		ConstraintSolver* m_solverMt;
		ContactSolverInfo* m_solverInfo;
		IDebugDraw* m_debugDrawer;
		Dispatcher* m_dispatcher;
	};
	static void solveIsland(ConstraintSolver* solver, Island& island, const SolverParams& solverParams);

	typedef void (*IslandDispatchFunc)(AlignedObjectArray<Island*>* islands, const SolverParams& solverParams);
	static void serialIslandDispatch(AlignedObjectArray<Island*>* islandsPtr, const SolverParams& solverParams);
	static void parallelIslandDispatch(AlignedObjectArray<Island*>* islandsPtr, const SolverParams& solverParams);

protected:
	AlignedObjectArray<Island*> m_allocatedIslands;    // owner of all Islands
	AlignedObjectArray<Island*> m_activeIslands;       // islands actively in use
	AlignedObjectArray<Island*> m_freeIslands;         // islands ready to be reused
	AlignedObjectArray<Island*> m_lookupIslandFromId;  // big lookup table to map islandId to Island pointer
	Island* m_batchIsland;
	i32 m_minimumSolverBatchSize;
	i32 m_batchIslandMinBodyCount;
	IslandDispatchFunc m_islandDispatch;

	Island* getIsland(i32 id);
	virtual Island* allocateIsland(i32 id, i32 numBodies);
	virtual void initIslandPools();
	virtual void addBodiesToIslands(CollisionWorld* collisionWorld);
	virtual void addManifoldsToIslands(Dispatcher* dispatcher);
	virtual void addConstraintsToIslands(AlignedObjectArray<TypedConstraint*>& constraints);
	virtual void mergeIslands();

public:
	SimulationIslandManagerMt();
	virtual ~SimulationIslandManagerMt();

	virtual void buildAndProcessIslands(Dispatcher* dispatcher,
										CollisionWorld* collisionWorld,
										AlignedObjectArray<TypedConstraint*>& constraints,
										const SolverParams& solverParams);

	virtual void buildIslands(Dispatcher* dispatcher, CollisionWorld* colWorld);

	i32 getMinimumSolverBatchSize() const
	{
		return m_minimumSolverBatchSize;
	}
	void setMinimumSolverBatchSize(i32 sz)
	{
		m_minimumSolverBatchSize = sz;
	}
	IslandDispatchFunc getIslandDispatchFunction() const
	{
		return m_islandDispatch;
	}
	// allow users to set their own dispatch function for multithreaded dispatch
	void setIslandDispatchFunction(IslandDispatchFunc func)
	{
		m_islandDispatch = func;
	}
};

#endif  //DRX3D_SIMULATION_ISLAND_MANAGER_H
