#ifndef DRX3D_COLLISION_CONFIGURATION
#define DRX3D_COLLISION_CONFIGURATION

#include <drxtypes.h>

struct CollisionAlgorithmCreateFunc;

class PoolAllocator;

//CollisionConfiguration allows to configure drx3D collision detection
///stack allocator size, default collision algorithms and persistent manifold pool size
///@todo: describe the meaning
class CollisionConfiguration
{
public:
	virtual ~CollisionConfiguration()
	{
	}

	///memory pools
	virtual PoolAllocator* getPersistentManifoldPool() = 0;

	virtual PoolAllocator* getCollisionAlgorithmPool() = 0;

	virtual CollisionAlgorithmCreateFunc* getCollisionAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1) = 0;

	virtual CollisionAlgorithmCreateFunc* getClosestPointsAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1) = 0;
};

#endif  //DRX3D_COLLISION_CONFIGURATION
