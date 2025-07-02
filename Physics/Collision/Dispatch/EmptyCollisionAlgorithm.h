#ifndef DRX3D_EMPTY_ALGORITH
#define DRX3D_EMPTY_ALGORITH
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

#define ATTRIBUTE_ALIGNED(a)

///EmptyAlgorithm is a stub for unsupported collision pairs.
///The dispatcher can dispatch a persistent EmptyAlgorithm to avoid a search every frame.
class EmptyAlgorithm : public CollisionAlgorithm
{
public:
	EmptyAlgorithm(const CollisionAlgorithmConstructionInfo& ci);

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			(void)body0Wrap;
			(void)body1Wrap;
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(EmptyAlgorithm));
			return new (mem) EmptyAlgorithm(ci);
		}
	};

} ATTRIBUTE_ALIGNED(16);

#endif  //DRX3D_EMPTY_ALGORITH
