#ifndef DRX3D_COLLISION_CREATE_FUNC
#define DRX3D_COLLISION_CREATE_FUNC

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
class CollisionAlgorithm;
class CollisionObject2;
struct CollisionObject2Wrapper;
struct CollisionAlgorithmConstructionInfo;

///Used by the Dispatcher to DoRegister and create instances for CollisionAlgorithm
struct CollisionAlgorithmCreateFunc
{
	bool m_swapped;

	CollisionAlgorithmCreateFunc()
		: m_swapped(false)
	{
	}
	virtual ~CollisionAlgorithmCreateFunc(){};

	virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo&, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	{
		(void)body0Wrap;
		(void)body1Wrap;
		return 0;
	}
};
#endif  //DRX3D_COLLISION_CREATE_FUNC
