#ifndef DRX3D_COLLISION_DISPATCHER_MT_H
#define DRX3D_COLLISION_DISPATCHER_MT_H

#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Maths/Linear/Threads.h>

class CollisionDispatcherMt : public CollisionDispatcher
{
public:
	CollisionDispatcherMt(CollisionConfiguration* config, i32 grainSize = 40);

	virtual PersistentManifold* getNewManifold(const CollisionObject2* body0, const CollisionObject2* body1) DRX3D_OVERRIDE;
	virtual void releaseManifold(PersistentManifold* manifold) DRX3D_OVERRIDE;

	virtual void dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& info, Dispatcher* dispatcher) DRX3D_OVERRIDE;

protected:
	AlignedObjectArray<AlignedObjectArray<PersistentManifold*> > m_batchManifoldsPtr;
	AlignedObjectArray<AlignedObjectArray<PersistentManifold*> > m_batchReleasePtr;
	bool m_batchUpdating;
	i32 m_grainSize;
};

#endif  //DRX3D_COLLISION_DISPATCHER_MT_H
