#ifndef DRX3D_SIMULATION_ISLAND_MANAGER_H
#define DRX3D_SIMULATION_ISLAND_MANAGER_H

#include <drx3D/Physics/Collision/Dispatch/UnionFind.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

class CollisionObject2;
class CollisionWorld;
class Dispatcher;
class PersistentManifold;

///SimulationIslandManager creates and handles simulation islands, using UnionFind
class SimulationIslandManager
{
	UnionFind m_unionFind;

	AlignedObjectArray<PersistentManifold*> m_islandmanifold;
	AlignedObjectArray<CollisionObject2*> m_islandBodies;

	bool m_splitIslands;

public:
	SimulationIslandManager();
	virtual ~SimulationIslandManager();

	void initUnionFind(i32 n);

	UnionFind& getUnionFind() { return m_unionFind; }

	virtual void updateActivationState(CollisionWorld* colWorld, Dispatcher* dispatcher);
	virtual void storeIslandActivationState(CollisionWorld* world);

	void findUnions(Dispatcher* dispatcher, CollisionWorld* colWorld);

	struct IslandCallback
	{
		virtual ~IslandCallback(){};

		virtual void processIsland(CollisionObject2** bodies, i32 numBodies, class PersistentManifold** manifolds, i32 numManifolds, i32 islandId) = 0;
	};

	void buildAndProcessIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld, IslandCallback* callback);
    
	void buildIslands(Dispatcher* dispatcher, CollisionWorld* colWorld);

    void processIslands(Dispatcher* dispatcher, CollisionWorld* collisionWorld, IslandCallback* callback);
    
	bool getSplitIslands()
	{
		return m_splitIslands;
	}
	void setSplitIslands(bool doSplitIslands)
	{
		m_splitIslands = doSplitIslands;
	}
};

#endif  //DRX3D_SIMULATION_ISLAND_MANAGER_H
