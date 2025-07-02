#ifndef DRX3D_COLLISION_ALGORITHM_H
#define DRX3D_COLLISION_ALGORITHM_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct BroadphaseProxy;
class Dispatcher;
class ManifoldResult;
class CollisionObject2;
struct CollisionObject2Wrapper;
struct DispatcherInfo;
class PersistentManifold;

typedef AlignedObjectArray<PersistentManifold*> ManifoldArray;

struct CollisionAlgorithmConstructionInfo
{
	CollisionAlgorithmConstructionInfo()
		: m_dispatcher1(0),
		  m_manifold(0)
	{
	}
	CollisionAlgorithmConstructionInfo(Dispatcher* dispatcher, i32 temp)
		: m_dispatcher1(dispatcher)
	{
		(void)temp;
	}

	Dispatcher* m_dispatcher1;
	PersistentManifold* m_manifold;

	//	i32	getDispatcherId();
};

//CollisionAlgorithm is an collision interface that is compatible with the Broadphase and Dispatcher.
///It is persistent over frames
class CollisionAlgorithm
{
protected:
	Dispatcher* m_dispatcher;

protected:
	//	i32	getDispatcherId();

public:
	CollisionAlgorithm(){};

	CollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci);

	virtual ~CollisionAlgorithm(){};

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut) = 0;

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut) = 0;

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray) = 0;
};

#endif  //DRX3D_COLLISION_ALGORITHM_H
