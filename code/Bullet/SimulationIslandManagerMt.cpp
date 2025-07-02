#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Physics/Dynamics/SimulationIslandManagerMt.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolverMt.h>  // for s_minimumContactManifoldsForBatching

//#include <stdio.h>
#include <drx3D/Maths/Linear/Quickprof.h>

SIMD_FORCE_INLINE i32 calcBatchCost(i32 bodies, i32 manifolds, i32 constraints)
{
	// rough estimate of the cost of a batch, used for merging
	i32 batchCost = bodies + 8 * manifolds + 4 * constraints;
	return batchCost;
}

SIMD_FORCE_INLINE i32 calcBatchCost(const SimulationIslandManagerMt::Island* island)
{
	return calcBatchCost(island->bodyArray.size(), island->manifoldArray.size(), island->constraintArray.size());
}

SimulationIslandManagerMt::SimulationIslandManagerMt()
{
	m_minimumSolverBatchSize = calcBatchCost(0, 128, 0);
	m_batchIslandMinBodyCount = 32;
	m_islandDispatch = parallelIslandDispatch;
	m_batchIsland = NULL;
}

SimulationIslandManagerMt::~SimulationIslandManagerMt()
{
	for (i32 i = 0; i < m_allocatedIslands.size(); ++i)
	{
		delete m_allocatedIslands[i];
	}
	m_allocatedIslands.resize(0);
	m_activeIslands.resize(0);
	m_freeIslands.resize(0);
}

inline i32 getIslandId(const PersistentManifold* lhs)
{
	const CollisionObject2* rcolObj0 = static_cast<const CollisionObject2*>(lhs->getBody0());
	const CollisionObject2* rcolObj1 = static_cast<const CollisionObject2*>(lhs->getBody1());
	i32 islandId = rcolObj0->getIslandTag() >= 0 ? rcolObj0->getIslandTag() : rcolObj1->getIslandTag();
	return islandId;
}

SIMD_FORCE_INLINE i32 GetConstraintIslandId1(const TypedConstraint* lhs)
{
	const CollisionObject2& rcolObj0 = lhs->getRigidBodyA();
	const CollisionObject2& rcolObj1 = lhs->getRigidBodyB();
	i32 islandId = rcolObj0.getIslandTag() >= 0 ? rcolObj0.getIslandTag() : rcolObj1.getIslandTag();
	return islandId;
}

/// function object that routes calls to operator<
class IslandBatchSizeSortPredicate
{
public:
	bool operator()(const SimulationIslandManagerMt::Island* lhs, const SimulationIslandManagerMt::Island* rhs) const
	{
		i32 lCost = calcBatchCost(lhs);
		i32 rCost = calcBatchCost(rhs);
		return lCost > rCost;
	}
};

class IslandBodyCapacitySortPredicate
{
public:
	bool operator()(const SimulationIslandManagerMt::Island* lhs, const SimulationIslandManagerMt::Island* rhs) const
	{
		return lhs->bodyArray.capacity() > rhs->bodyArray.capacity();
	}
};

void SimulationIslandManagerMt::Island::append(const Island& other)
{
	// append bodies
	for (i32 i = 0; i < other.bodyArray.size(); ++i)
	{
		bodyArray.push_back(other.bodyArray[i]);
	}
	// append manifolds
	for (i32 i = 0; i < other.manifoldArray.size(); ++i)
	{
		manifoldArray.push_back(other.manifoldArray[i]);
	}
	// append constraints
	for (i32 i = 0; i < other.constraintArray.size(); ++i)
	{
		constraintArray.push_back(other.constraintArray[i]);
	}
}

bool IsBodyInIsland(const SimulationIslandManagerMt::Island& island, const CollisionObject2* obj)
{
	for (i32 i = 0; i < island.bodyArray.size(); ++i)
	{
		if (island.bodyArray[i] == obj)
		{
			return true;
		}
	}
	return false;
}

void SimulationIslandManagerMt::initIslandPools()
{
	// reset island pools
	i32 numElem = getUnionFind().getNumElements();
	m_lookupIslandFromId.resize(numElem);
	for (i32 i = 0; i < m_lookupIslandFromId.size(); ++i)
	{
		m_lookupIslandFromId[i] = NULL;
	}
	m_activeIslands.resize(0);
	m_freeIslands.resize(0);
	// check whether allocated islands are sorted by body capacity (largest to smallest)
	i32 lastCapacity = 0;
	bool isSorted = true;
	for (i32 i = 0; i < m_allocatedIslands.size(); ++i)
	{
		Island* island = m_allocatedIslands[i];
		i32 cap = island->bodyArray.capacity();
		if (cap > lastCapacity)
		{
			isSorted = false;
			break;
		}
		lastCapacity = cap;
	}
	if (!isSorted)
	{
		m_allocatedIslands.quickSort(IslandBodyCapacitySortPredicate());
	}

	m_batchIsland = NULL;
	// mark all islands free (but avoid deallocation)
	for (i32 i = 0; i < m_allocatedIslands.size(); ++i)
	{
		Island* island = m_allocatedIslands[i];
		island->bodyArray.resize(0);
		island->manifoldArray.resize(0);
		island->constraintArray.resize(0);
		island->id = -1;
		island->isSleeping = true;
		m_freeIslands.push_back(island);
	}
}

SimulationIslandManagerMt::Island* SimulationIslandManagerMt::getIsland(i32 id)
{
	Assert(id >= 0);
	Assert(id < m_lookupIslandFromId.size());
	Island* island = m_lookupIslandFromId[id];
	if (island == NULL)
	{
		// search for existing island
		for (i32 i = 0; i < m_activeIslands.size(); ++i)
		{
			if (m_activeIslands[i]->id == id)
			{
				island = m_activeIslands[i];
				break;
			}
		}
		m_lookupIslandFromId[id] = island;
	}
	return island;
}

SimulationIslandManagerMt::Island* SimulationIslandManagerMt::allocateIsland(i32 id, i32 numBodies)
{
	Island* island = NULL;
	i32 allocSize = numBodies;
	if (numBodies < m_batchIslandMinBodyCount)
	{
		if (m_batchIsland)
		{
			island = m_batchIsland;
			m_lookupIslandFromId[id] = island;
			// if we've made a large enough batch,
			if (island->bodyArray.size() + numBodies >= m_batchIslandMinBodyCount)
			{
				// next time start a new batch
				m_batchIsland = NULL;
			}
			return island;
		}
		else
		{
			// need to allocate a batch island
			allocSize = m_batchIslandMinBodyCount * 2;
		}
	}
	AlignedObjectArray<Island*>& freeIslands = m_freeIslands;

	// search for free island
	if (freeIslands.size() > 0)
	{
		// try to reuse a previously allocated island
		i32 iFound = freeIslands.size();
		// linear search for smallest island that can hold our bodies
		for (i32 i = freeIslands.size() - 1; i >= 0; --i)
		{
			if (freeIslands[i]->bodyArray.capacity() >= allocSize)
			{
				iFound = i;
				island = freeIslands[i];
				island->id = id;
				break;
			}
		}
		// if found, shrink array while maintaining ordering
		if (island)
		{
			i32 iDest = iFound;
			i32 iSrc = iDest + 1;
			while (iSrc < freeIslands.size())
			{
				freeIslands[iDest++] = freeIslands[iSrc++];
			}
			freeIslands.pop_back();
		}
	}
	if (island == NULL)
	{
		// no free island found, allocate
		island = new Island();  // TODO: change this to use the pool allocator
		island->id = id;
		island->bodyArray.reserve(allocSize);
		m_allocatedIslands.push_back(island);
	}
	m_lookupIslandFromId[id] = island;
	if (numBodies < m_batchIslandMinBodyCount)
	{
		m_batchIsland = island;
	}
	m_activeIslands.push_back(island);
	return island;
}

void SimulationIslandManagerMt::buildIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld)
{
	DRX3D_PROFILE("buildIslands");

	CollisionObject2Array& collisionObjects = collisionWorld->getCollisionObjectArray();

	//we are going to sort the unionfind array, and store the element id in the size
	//afterwards, we clean unionfind, to make sure no-one uses it anymore

	getUnionFind().sortIslands();
	i32 numElem = getUnionFind().getNumElements();

	i32 endIslandIndex = 1;
	i32 startIslandIndex;

	//update the sleeping state for bodies, if all are sleeping
	for (startIslandIndex = 0; startIslandIndex < numElem; startIslandIndex = endIslandIndex)
	{
		i32 islandId = getUnionFind().getElement(startIslandIndex).m_id;
		for (endIslandIndex = startIslandIndex + 1; (endIslandIndex < numElem) && (getUnionFind().getElement(endIslandIndex).m_id == islandId); endIslandIndex++)
		{
		}

		//i32 numSleeping = 0;

		bool allSleeping = true;

		i32 idx;
		for (idx = startIslandIndex; idx < endIslandIndex; idx++)
		{
			i32 i = getUnionFind().getElement(idx).m_sz;

			CollisionObject2* colObj0 = collisionObjects[i];
			if ((colObj0->getIslandTag() != islandId) && (colObj0->getIslandTag() != -1))
			{
				//				printf("error in island management\n");
			}

			Assert((colObj0->getIslandTag() == islandId) || (colObj0->getIslandTag() == -1));
			if (colObj0->getIslandTag() == islandId)
			{
				if (colObj0->getActivationState() == ACTIVE_TAG ||
					colObj0->getActivationState() == DISABLE_DEACTIVATION)
				{
					allSleeping = false;
					break;
				}
			}
		}

		if (allSleeping)
		{
			i32 idx;
			for (idx = startIslandIndex; idx < endIslandIndex; idx++)
			{
				i32 i = getUnionFind().getElement(idx).m_sz;
				CollisionObject2* colObj0 = collisionObjects[i];
				if ((colObj0->getIslandTag() != islandId) && (colObj0->getIslandTag() != -1))
				{
					//					printf("error in island management\n");
				}

				Assert((colObj0->getIslandTag() == islandId) || (colObj0->getIslandTag() == -1));

				if (colObj0->getIslandTag() == islandId)
				{
					colObj0->setActivationState(ISLAND_SLEEPING);
				}
			}
		}
		else
		{
			i32 idx;
			for (idx = startIslandIndex; idx < endIslandIndex; idx++)
			{
				i32 i = getUnionFind().getElement(idx).m_sz;

				CollisionObject2* colObj0 = collisionObjects[i];
				if ((colObj0->getIslandTag() != islandId) && (colObj0->getIslandTag() != -1))
				{
					//					printf("error in island management\n");
				}

				Assert((colObj0->getIslandTag() == islandId) || (colObj0->getIslandTag() == -1));

				if (colObj0->getIslandTag() == islandId)
				{
					if (colObj0->getActivationState() == ISLAND_SLEEPING)
					{
						colObj0->setActivationState(WANTS_DEACTIVATION);
						colObj0->setDeactivationTime(0.f);
					}
				}
			}
		}
	}
}

void SimulationIslandManagerMt::addBodiesToIslands(CollisionWorld* collisionWorld)
{
	CollisionObject2Array& collisionObjects = collisionWorld->getCollisionObjectArray();
	i32 endIslandIndex = 1;
	i32 startIslandIndex;
	i32 numElem = getUnionFind().getNumElements();

	// create explicit islands and add bodies to each
	for (startIslandIndex = 0; startIslandIndex < numElem; startIslandIndex = endIslandIndex)
	{
		i32 islandId = getUnionFind().getElement(startIslandIndex).m_id;

		// find end index
		for (endIslandIndex = startIslandIndex; (endIslandIndex < numElem) && (getUnionFind().getElement(endIslandIndex).m_id == islandId); endIslandIndex++)
		{
		}
		// check if island is sleeping
		bool islandSleeping = true;
		for (i32 iElem = startIslandIndex; iElem < endIslandIndex; iElem++)
		{
			i32 i = getUnionFind().getElement(iElem).m_sz;
			CollisionObject2* colObj = collisionObjects[i];
			if (colObj->isActive())
			{
				islandSleeping = false;
			}
		}
		if (!islandSleeping)
		{
			// want to count the number of bodies before allocating the island to optimize memory usage of the Island structures
			i32 numBodies = endIslandIndex - startIslandIndex;
			Island* island = allocateIsland(islandId, numBodies);
			island->isSleeping = false;

			// add bodies to island
			for (i32 iElem = startIslandIndex; iElem < endIslandIndex; iElem++)
			{
				i32 i = getUnionFind().getElement(iElem).m_sz;
				CollisionObject2* colObj = collisionObjects[i];
				island->bodyArray.push_back(colObj);
			}
		}
	}
}

void SimulationIslandManagerMt::addManifoldsToIslands(Dispatcher* dispatcher)
{
	// walk all the manifolds, activating bodies touched by kinematic objects, and add each manifold to its Island
	i32 maxNumManifolds = dispatcher->getNumManifolds();
	for (i32 i = 0; i < maxNumManifolds; i++)
	{
		PersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);

		const CollisionObject2* colObj0 = static_cast<const CollisionObject2*>(manifold->getBody0());
		const CollisionObject2* colObj1 = static_cast<const CollisionObject2*>(manifold->getBody1());

		///@todo: check sleeping conditions!
		if (((colObj0) && colObj0->getActivationState() != ISLAND_SLEEPING) ||
			((colObj1) && colObj1->getActivationState() != ISLAND_SLEEPING))
		{
			//kinematic objects don't merge islands, but wake up all connected objects
			if (colObj0->isKinematicObject() && colObj0->getActivationState() != ISLAND_SLEEPING)
			{
				if (colObj0->hasContactResponse())
					colObj1->activate();
			}
			if (colObj1->isKinematicObject() && colObj1->getActivationState() != ISLAND_SLEEPING)
			{
				if (colObj1->hasContactResponse())
					colObj0->activate();
			}
			//filtering for response
			if (dispatcher->needsResponse(colObj0, colObj1))
			{
				// scatter manifolds into various islands
				i32 islandId = getIslandId(manifold);
				// if island not sleeping,
				if (Island* island = getIsland(islandId))
				{
					island->manifoldArray.push_back(manifold);
				}
			}
		}
	}
}

void SimulationIslandManagerMt::addConstraintsToIslands(AlignedObjectArray<TypedConstraint*>& constraints)
{
	// walk constraints
	for (i32 i = 0; i < constraints.size(); i++)
	{
		// scatter constraints into various islands
		TypedConstraint* constraint = constraints[i];
		if (constraint->isEnabled())
		{
			i32 islandId = GetConstraintIslandId1(constraint);
			// if island is not sleeping,
			if (Island* island = getIsland(islandId))
			{
				island->constraintArray.push_back(constraint);
			}
		}
	}
}

void SimulationIslandManagerMt::mergeIslands()
{
	// sort islands in order of decreasing batch size
	m_activeIslands.quickSort(IslandBatchSizeSortPredicate());

	// merge small islands to satisfy minimum batch size
	// find first small batch island
	i32 destIslandIndex = m_activeIslands.size();
	for (i32 i = 0; i < m_activeIslands.size(); ++i)
	{
		Island* island = m_activeIslands[i];
		i32 batchSize = calcBatchCost(island);
		if (batchSize < m_minimumSolverBatchSize)
		{
			destIslandIndex = i;
			break;
		}
	}
	i32 lastIndex = m_activeIslands.size() - 1;
	while (destIslandIndex < lastIndex)
	{
		// merge islands from the back of the list
		Island* island = m_activeIslands[destIslandIndex];
		i32 numBodies = island->bodyArray.size();
		i32 numManifolds = island->manifoldArray.size();
		i32 numConstraints = island->constraintArray.size();
		i32 firstIndex = lastIndex;
		// figure out how many islands we want to merge and find out how many bodies, manifolds and constraints we will have
		while (true)
		{
			Island* src = m_activeIslands[firstIndex];
			numBodies += src->bodyArray.size();
			numManifolds += src->manifoldArray.size();
			numConstraints += src->constraintArray.size();
			i32 batchCost = calcBatchCost(numBodies, numManifolds, numConstraints);
			if (batchCost >= m_minimumSolverBatchSize)
			{
				break;
			}
			if (firstIndex - 1 == destIslandIndex)
			{
				break;
			}
			firstIndex--;
		}
		// reserve space for these pointers to minimize reallocation
		island->bodyArray.reserve(numBodies);
		island->manifoldArray.reserve(numManifolds);
		island->constraintArray.reserve(numConstraints);
		// merge islands
		for (i32 i = firstIndex; i <= lastIndex; ++i)
		{
			island->append(*m_activeIslands[i]);
		}
		// shrink array to exclude the islands that were merged from
		m_activeIslands.resize(firstIndex);
		lastIndex = firstIndex - 1;
		destIslandIndex++;
	}
}

void SimulationIslandManagerMt::solveIsland(ConstraintSolver* solver, Island& island, const SolverParams& solverParams)
{
	PersistentManifold** manifolds = island.manifoldArray.size() ? &island.manifoldArray[0] : NULL;
	TypedConstraint** constraintsPtr = island.constraintArray.size() ? &island.constraintArray[0] : NULL;
	solver->solveGroup(&island.bodyArray[0],
					   island.bodyArray.size(),
					   manifolds,
					   island.manifoldArray.size(),
					   constraintsPtr,
					   island.constraintArray.size(),
					   *solverParams.m_solverInfo,
					   solverParams.m_debugDrawer,
					   solverParams.m_dispatcher);
}

void SimulationIslandManagerMt::serialIslandDispatch(AlignedObjectArray<Island*>* islandsPtr, const SolverParams& solverParams)
{
	DRX3D_PROFILE("serialIslandDispatch");
	// serial dispatch
	AlignedObjectArray<Island*>& islands = *islandsPtr;
	ConstraintSolver* solver = solverParams.m_solverMt ? solverParams.m_solverMt : solverParams.m_solverPool;
	for (i32 i = 0; i < islands.size(); ++i)
	{
		solveIsland(solver, *islands[i], solverParams);
	}
}

struct UpdateIslandDispatcher : public IParallelForBody
{
	AlignedObjectArray<SimulationIslandManagerMt::Island*>& m_islandsPtr;
	const SimulationIslandManagerMt::SolverParams& m_solverParams;

	UpdateIslandDispatcher(AlignedObjectArray<SimulationIslandManagerMt::Island*>& islandsPtr, const SimulationIslandManagerMt::SolverParams& solverParams)
		: m_islandsPtr(islandsPtr), m_solverParams(solverParams)
	{
	}

	void forLoop(i32 iBegin, i32 iEnd) const DRX3D_OVERRIDE
	{
		ConstraintSolver* solver = m_solverParams.m_solverPool;
		for (i32 i = iBegin; i < iEnd; ++i)
		{
			SimulationIslandManagerMt::Island* island = m_islandsPtr[i];
			SimulationIslandManagerMt::solveIsland(solver, *island, m_solverParams);
		}
	}
};

void SimulationIslandManagerMt::parallelIslandDispatch(AlignedObjectArray<Island*>* islandsPtr, const SolverParams& solverParams)
{
	DRX3D_PROFILE("parallelIslandDispatch");
	//
	// if there are islands with many contacts, it may be faster to submit these
	// large islands *serially* to a single parallel constraint solver, and then later
	// submit the remaining smaller islands in parallel to multiple sequential solvers.
	//
	// Some task schedulers do not deal well with nested parallelFor loops. One implementation
	// of OpenMP was actually slower than doing everything single-threaded. Intel TBB
	// on the other hand, seems to do a pretty respectable job with it.
	//
	// When solving islands in parallel, the worst case performance happens when there
	// is one very large island and then perhaps a smattering of very small
	// islands -- one worker thread takes the large island and the remaining workers
	// tear through the smaller islands and then sit idle waiting for the first worker
	// to finish. Solving islands in parallel works best when there are numerous small
	// islands, roughly equal in size.
	//
	// By contrast, the other approach -- the parallel constraint solver -- is only
	// able to deliver a worthwhile speedup when the island is large. For smaller islands,
	// it is difficult to extract a useful amount of parallelism -- the overhead of grouping
	// the constraints into batches and sending the batches to worker threads can nullify
	// any gains from parallelism.
	//

	UpdateIslandDispatcher dispatcher(*islandsPtr, solverParams);
	// We take advantage of the fact the islands are sorted in order of decreasing size
	i32 iBegin = 0;
	if (solverParams.m_solverMt)
	{
		while (iBegin < islandsPtr->size())
		{
			SimulationIslandManagerMt::Island* island = (*islandsPtr)[iBegin];
			if (island->manifoldArray.size() < SequentialImpulseConstraintSolverMt::s_minimumContactManifoldsForBatching)
			{
				// OK to submit the rest of the array in parallel
				break;
			}
			// serial dispatch to parallel solver for large islands (if any)
			solveIsland(solverParams.m_solverMt, *island, solverParams);
			++iBegin;
		}
	}
	// parallel dispatch to sequential solvers for rest
	ParallelFor(iBegin, islandsPtr->size(), 1, dispatcher);
}

///@todo: this is random access, it can be walked 'cache friendly'!
void SimulationIslandManagerMt::buildAndProcessIslands(Dispatcher* dispatcher,
														 CollisionWorld* collisionWorld,
														 AlignedObjectArray<TypedConstraint*>& constraints,
														 const SolverParams& solverParams)
{
	DRX3D_PROFILE("buildAndProcessIslands");
	CollisionObject2Array& collisionObjects = collisionWorld->getCollisionObjectArray();

	buildIslands(dispatcher, collisionWorld);

	if (!getSplitIslands())
	{
		PersistentManifold** manifolds = dispatcher->getInternalManifoldPointer();
		i32 maxNumManifolds = dispatcher->getNumManifolds();

		for (i32 i = 0; i < maxNumManifolds; i++)
		{
			PersistentManifold* manifold = manifolds[i];

			const CollisionObject2* colObj0 = static_cast<const CollisionObject2*>(manifold->getBody0());
			const CollisionObject2* colObj1 = static_cast<const CollisionObject2*>(manifold->getBody1());

			///@todo: check sleeping conditions!
			if (((colObj0) && colObj0->getActivationState() != ISLAND_SLEEPING) ||
				((colObj1) && colObj1->getActivationState() != ISLAND_SLEEPING))
			{
				//kinematic objects don't merge islands, but wake up all connected objects
				if (colObj0->isKinematicObject() && colObj0->getActivationState() != ISLAND_SLEEPING)
				{
					if (colObj0->hasContactResponse())
						colObj1->activate();
				}
				if (colObj1->isKinematicObject() && colObj1->getActivationState() != ISLAND_SLEEPING)
				{
					if (colObj1->hasContactResponse())
						colObj0->activate();
				}
			}
		}
		TypedConstraint** constraintsPtr = constraints.size() ? &constraints[0] : NULL;
		ConstraintSolver* solver = solverParams.m_solverMt ? solverParams.m_solverMt : solverParams.m_solverPool;
		solver->solveGroup(&collisionObjects[0],
						   collisionObjects.size(),
						   manifolds,
						   maxNumManifolds,
						   constraintsPtr,
						   constraints.size(),
						   *solverParams.m_solverInfo,
						   solverParams.m_debugDrawer,
						   solverParams.m_dispatcher);
	}
	else
	{
		initIslandPools();

		//traverse the simulation islands, and call the solver, unless all objects are sleeping/deactivated
		addBodiesToIslands(collisionWorld);
		addManifoldsToIslands(dispatcher);
		addConstraintsToIslands(constraints);

		// m_activeIslands array should now contain all non-sleeping Islands, and each Island should
		// have all the necessary bodies, manifolds and constraints.

		// if we want to merge islands with small batch counts,
		if (m_minimumSolverBatchSize > 1)
		{
			mergeIslands();
		}
		// dispatch islands to solver
		m_islandDispatch(&m_activeIslands, solverParams);
	}
}
