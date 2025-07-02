#ifndef DRX3D_COMPOUND_COMPOUND_COLLISION_ALGORITHM_H
#define DRX3D_COMPOUND_COMPOUND_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/CompoundCollisionAlgorithm.h>

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>

#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class Dispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/Dispatch/HashedSimplePairCache.h>
class Dispatcher;
class CollisionObject2;

class CollisionShape;

extern ShapePairCallback gCompoundCompoundChildShapePairCallback;

/// CompoundCompoundCollisionAlgorithm  supports collision between two CompoundCollisionShape shapes
class CompoundCompoundCollisionAlgorithm : public CompoundCollisionAlgorithm
{
	class HashedSimplePairCache* m_childCollisionAlgorithmCache;
	SimplePairArray m_removePairs;

	i32 m_compoundShapeRevision0;  //to keep track of changes, so that childAlgorithm array can be updated
	i32 m_compoundShapeRevision1;

	void removeChildAlgorithms();

	//	void	preallocateChildAlgorithms(const CollisionObject2Wrapper* body0Wrap,const CollisionObject2Wrapper* body1Wrap);

public:
	CompoundCompoundCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	virtual ~CompoundCompoundCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray);

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(CompoundCompoundCollisionAlgorithm));
			return new (mem) CompoundCompoundCollisionAlgorithm(ci, body0Wrap, body1Wrap, false);
		}
	};

	struct SwappedCreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(CompoundCompoundCollisionAlgorithm));
			return new (mem) CompoundCompoundCollisionAlgorithm(ci, body0Wrap, body1Wrap, true);
		}
	};
};

#endif  //DRX3D_COMPOUND_COMPOUND_COLLISION_ALGORITHM_H
