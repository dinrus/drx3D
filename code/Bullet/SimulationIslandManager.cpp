#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>

//#include <stdio.h>
#include <drx3D/Maths/Linear/Quickprof.h>

SimulationIslandManager::SimulationIslandManager() : m_splitIslands(true)
{
}

SimulationIslandManager::~SimulationIslandManager()
{
}

void SimulationIslandManager::initUnionFind(i32 n)
{
	m_unionFind.reset(n);
}

void SimulationIslandManager::findUnions(Dispatcher* /* dispatcher */, CollisionWorld* colWorld)
{
	{
		OverlappingPairCache* pairCachePtr = colWorld->getPairCache();
		i32k numOverlappingPairs = pairCachePtr->getNumOverlappingPairs();
		if (numOverlappingPairs)
		{
			BroadphasePair* pairPtr = pairCachePtr->getOverlappingPairArrayPtr();

			for (i32 i = 0; i < numOverlappingPairs; i++)
			{
				const BroadphasePair& collisionPair = pairPtr[i];
				CollisionObject2* colObj0 = (CollisionObject2*)collisionPair.m_pProxy0->m_clientObject;
				CollisionObject2* colObj1 = (CollisionObject2*)collisionPair.m_pProxy1->m_clientObject;

				if (((colObj0) && ((colObj0)->mergesSimulationIslands())) &&
					((colObj1) && ((colObj1)->mergesSimulationIslands())))
				{
					m_unionFind.unite((colObj0)->getIslandTag(),
									  (colObj1)->getIslandTag());
				}
			}
		}
	}
}

#ifdef STATIC_SIMULATION_ISLAND_OPTIMIZATION
void SimulationIslandManager::updateActivationState(CollisionWorld* colWorld, Dispatcher* dispatcher)
{
	// put the index into m_controllers into m_tag
	i32 index = 0;
	{
		i32 i;
		for (i = 0; i < colWorld->getCollisionObjectArray().size(); i++)
		{
			CollisionObject2* collisionObject = colWorld->getCollisionObjectArray()[i];
			//Adding filtering here
			if (!collisionObject->isStaticOrKinematicObject())
			{
				collisionObject->setIslandTag(index++);
			}
			collisionObject->setCompanionId(-1);
			collisionObject->setHitFraction(Scalar(1.));
		}
	}
	// do the union find

	initUnionFind(index);

	findUnions(dispatcher, colWorld);
}

void SimulationIslandManager::storeIslandActivationState(CollisionWorld* colWorld)
{
	// put the islandId ('find' value) into m_tag
	{
		i32 index = 0;
		i32 i;
		for (i = 0; i < colWorld->getCollisionObjectArray().size(); i++)
		{
			CollisionObject2* collisionObject = colWorld->getCollisionObjectArray()[i];
			if (!collisionObject->isStaticOrKinematicObject())
			{
				collisionObject->setIslandTag(m_unionFind.find(index));
				//Set the correct object offset in Collision Object Array
				m_unionFind.getElement(index).m_sz = i;
				collisionObject->setCompanionId(-1);
				index++;
			}
			else
			{
				collisionObject->setIslandTag(-1);
				collisionObject->setCompanionId(-2);
			}
		}
	}
}

#else  //STATIC_SIMULATION_ISLAND_OPTIMIZATION
void SimulationIslandManager::updateActivationState(CollisionWorld* colWorld, Dispatcher* dispatcher)
{
	initUnionFind(i32(colWorld->getCollisionObjectArray().size()));

	// put the index into m_controllers into m_tag
	{
		i32 index = 0;
		i32 i;
		for (i = 0; i < colWorld->getCollisionObjectArray().size(); i++)
		{
			CollisionObject2* collisionObject = colWorld->getCollisionObjectArray()[i];
			collisionObject->setIslandTag(index);
			collisionObject->setCompanionId(-1);
			collisionObject->setHitFraction(Scalar(1.));
			index++;
		}
	}
	// do the union find

	findUnions(dispatcher, colWorld);
}

void SimulationIslandManager::storeIslandActivationState(CollisionWorld* colWorld)
{
	// put the islandId ('find' value) into m_tag
	{
		i32 index = 0;
		i32 i;
		for (i = 0; i < colWorld->getCollisionObjectArray().size(); i++)
		{
			CollisionObject2* collisionObject = colWorld->getCollisionObjectArray()[i];
			if (!collisionObject->isStaticOrKinematicObject())
			{
				collisionObject->setIslandTag(m_unionFind.find(index));
				collisionObject->setCompanionId(-1);
			}
			else
			{
				collisionObject->setIslandTag(-1);
				collisionObject->setCompanionId(-2);
			}
			index++;
		}
	}
}

#endif  //STATIC_SIMULATION_ISLAND_OPTIMIZATION

inline i32 getIslandId(const PersistentManifold* lhs)
{
	i32 islandId;
	const CollisionObject2* rcolObj0 = static_cast<const CollisionObject2*>(lhs->getBody0());
	const CollisionObject2* rcolObj1 = static_cast<const CollisionObject2*>(lhs->getBody1());
	islandId = rcolObj0->getIslandTag() >= 0 ? rcolObj0->getIslandTag() : rcolObj1->getIslandTag();
	return islandId;
}

/// function object that routes calls to operator<
class PersistentManifoldSortPredicate
{
public:
	SIMD_FORCE_INLINE bool operator()(const PersistentManifold* lhs, const PersistentManifold* rhs) const
	{
		return getIslandId(lhs) < getIslandId(rhs);
	}
};

class PersistentManifoldSortPredicateDeterministic
{
public:
	SIMD_FORCE_INLINE bool operator()(const PersistentManifold* lhs, const PersistentManifold* rhs) const
	{
		return (
			(getIslandId(lhs) < getIslandId(rhs)) || ((getIslandId(lhs) == getIslandId(rhs)) && lhs->getBody0()->getBroadphaseHandle()->m_uniqueId < rhs->getBody0()->getBroadphaseHandle()->m_uniqueId) || ((getIslandId(lhs) == getIslandId(rhs)) && (lhs->getBody0()->getBroadphaseHandle()->m_uniqueId == rhs->getBody0()->getBroadphaseHandle()->m_uniqueId) && (lhs->getBody1()->getBroadphaseHandle()->m_uniqueId < rhs->getBody1()->getBroadphaseHandle()->m_uniqueId)));
	}
};

void SimulationIslandManager::buildIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld)
{
	DRX3D_PROFILE("islandUnionFindAndQuickSort");

	CollisionObject2Array& collisionObjects = collisionWorld->getCollisionObjectArray();

	m_islandmanifold.resize(0);

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

	i32 i;
	i32 maxNumManifolds = dispatcher->getNumManifolds();

	//#define SPLIT_ISLANDS 1
	//#ifdef SPLIT_ISLANDS

	//#endif //SPLIT_ISLANDS

	for (i = 0; i < maxNumManifolds; i++)
	{
		PersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);
		if (collisionWorld->getDispatchInfo().m_deterministicOverlappingPairs)
		{
			if (manifold->getNumContacts() == 0)
				continue;
		}

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
			if (m_splitIslands)
			{
				//filtering for response
				if (dispatcher->needsResponse(colObj0, colObj1))
					m_islandmanifold.push_back(manifold);
			}
		}
	}
}


///@todo: this is random access, it can be walked 'cache friendly'!
void SimulationIslandManager::buildAndProcessIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld, IslandCallback* callback)
{
	buildIslands(dispatcher, collisionWorld);
    processIslands(dispatcher, collisionWorld, callback);
}

void SimulationIslandManager::processIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld, IslandCallback* callback)
{
    CollisionObject2Array& collisionObjects = collisionWorld->getCollisionObjectArray();
	i32 endIslandIndex = 1;
	i32 startIslandIndex;
	i32 numElem = getUnionFind().getNumElements();

	DRX3D_PROFILE("processIslands");

	if (!m_splitIslands)
	{
		PersistentManifold** manifold = dispatcher->getInternalManifoldPointer();
		i32 maxNumManifolds = dispatcher->getNumManifolds();
		callback->processIsland(&collisionObjects[0], collisionObjects.size(), manifold, maxNumManifolds, -1);
	}
	else
	{
		// Sort manifolds, based on islands
		// Sort the vector using predicate and std::sort
		//std::sort(islandmanifold.begin(), islandmanifold.end(), PersistentManifoldSortPredicate);

		i32 numManifolds = i32(m_islandmanifold.size());

		//tried a radix sort, but quicksort/heapsort seems still faster
		//@todo rewrite island management
		//PersistentManifoldSortPredicateDeterministic sorts contact manifolds based on islandid,
		//but also based on object0 unique id and object1 unique id
		if (collisionWorld->getDispatchInfo().m_deterministicOverlappingPairs)
		{
			m_islandmanifold.quickSort(PersistentManifoldSortPredicateDeterministic());
		}
		else
		{
			m_islandmanifold.quickSort(PersistentManifoldSortPredicate());
		}

		//m_islandmanifold.heapSort(PersistentManifoldSortPredicate());

		//now process all active islands (sets of manifolds for now)

		i32 startManifoldIndex = 0;
		i32 endManifoldIndex = 1;

		//i32 islandId;

		//	printf("Start Islands\n");

		//traverse the simulation islands, and call the solver, unless all objects are sleeping/deactivated
		for (startIslandIndex = 0; startIslandIndex < numElem; startIslandIndex = endIslandIndex)
		{
			i32 islandId = getUnionFind().getElement(startIslandIndex).m_id;

			bool islandSleeping = true;

			for (endIslandIndex = startIslandIndex; (endIslandIndex < numElem) && (getUnionFind().getElement(endIslandIndex).m_id == islandId); endIslandIndex++)
			{
				i32 i = getUnionFind().getElement(endIslandIndex).m_sz;
				CollisionObject2* colObj0 = collisionObjects[i];
				m_islandBodies.push_back(colObj0);
				if (colObj0->isActive())
					islandSleeping = false;
			}

			//find the accompanying contact manifold for this islandId
			i32 numIslandManifolds = 0;
			PersistentManifold** startManifold = 0;

			if (startManifoldIndex < numManifolds)
			{
				i32 curIslandId = getIslandId(m_islandmanifold[startManifoldIndex]);
				if (curIslandId == islandId)
				{
					startManifold = &m_islandmanifold[startManifoldIndex];

					for (endManifoldIndex = startManifoldIndex + 1; (endManifoldIndex < numManifolds) && (islandId == getIslandId(m_islandmanifold[endManifoldIndex])); endManifoldIndex++)
					{
					}
					/// Process the actual simulation, only if not sleeping/deactivated
					numIslandManifolds = endManifoldIndex - startManifoldIndex;
				}
			}

			if (!islandSleeping)
			{
				callback->processIsland(&m_islandBodies[0], m_islandBodies.size(), startManifold, numIslandManifolds, islandId);
				//			printf("Island callback of size:%d bodies, %d manifolds\n",islandBodies.size(),numIslandManifolds);
			}

			if (numIslandManifolds)
			{
				startManifoldIndex = endManifoldIndex;
			}

			m_islandBodies.resize(0);
		}
	}  // else if(!splitIslands)
}
