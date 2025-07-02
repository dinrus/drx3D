#ifndef DRX3D_DEFAULT_COLLISION_CONFIGURATION
#define DRX3D_DEFAULT_COLLISION_CONFIGURATION

#include <drxtypes.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionConfiguration.h>

class VoronoiSimplexSolver;
class ConvexPenetrationDepthSolver;

struct DefaultCollisionConstructionInfo
{
	PoolAllocator* m_persistentManifoldPool;
	PoolAllocator* m_collisionAlgorithmPool;
	i32 m_defaultMaxPersistentManifoldPoolSize;
	i32 m_defaultMaxCollisionAlgorithmPoolSize;
	i32 m_customCollisionAlgorithmMaxElementSize;
	i32 m_useEpaPenetrationAlgorithm;

	DefaultCollisionConstructionInfo()
		: m_persistentManifoldPool(0),
		  m_collisionAlgorithmPool(0),
		  m_defaultMaxPersistentManifoldPoolSize(4096),
		  m_defaultMaxCollisionAlgorithmPoolSize(4096),
		  m_customCollisionAlgorithmMaxElementSize(0),
		  m_useEpaPenetrationAlgorithm(true)
	{
	}
};

//CollisionConfiguration allows to configure drx3D collision detection
///stack allocator, pool memory allocators
///@todo: describe the meaning
class DefaultCollisionConfiguration : public CollisionConfiguration
{
protected:
	i32 m_persistentManifoldPoolSize;

	PoolAllocator* m_persistentManifoldPool;
	bool m_ownsPersistentManifoldPool;

	PoolAllocator* m_collisionAlgorithmPool;
	bool m_ownsCollisionAlgorithmPool;

	//default penetration depth solver
	ConvexPenetrationDepthSolver* m_pdSolver;

	//default CreationFunctions, filling the m_doubleDispatch table
	CollisionAlgorithmCreateFunc* m_convexConvexCreateFunc;
	CollisionAlgorithmCreateFunc* m_convexConcaveCreateFunc;
	CollisionAlgorithmCreateFunc* m_swappedConvexConcaveCreateFunc;
	CollisionAlgorithmCreateFunc* m_compoundCreateFunc;
	CollisionAlgorithmCreateFunc* m_compoundCompoundCreateFunc;

	CollisionAlgorithmCreateFunc* m_swappedCompoundCreateFunc;
	CollisionAlgorithmCreateFunc* m_emptyCreateFunc;
	CollisionAlgorithmCreateFunc* m_sphereSphereCF;
	CollisionAlgorithmCreateFunc* m_sphereBoxCF;
	CollisionAlgorithmCreateFunc* m_boxSphereCF;

	CollisionAlgorithmCreateFunc* m_boxBoxCF;
	CollisionAlgorithmCreateFunc* m_sphereTriangleCF;
	CollisionAlgorithmCreateFunc* m_triangleSphereCF;
	CollisionAlgorithmCreateFunc* m_planeConvexCF;
	CollisionAlgorithmCreateFunc* m_convexPlaneCF;

public:
	DefaultCollisionConfiguration(const DefaultCollisionConstructionInfo& constructionInfo = DefaultCollisionConstructionInfo());

	virtual ~DefaultCollisionConfiguration();

	///memory pools
	virtual PoolAllocator* getPersistentManifoldPool()
	{
		return m_persistentManifoldPool;
	}

	virtual PoolAllocator* getCollisionAlgorithmPool()
	{
		return m_collisionAlgorithmPool;
	}

	virtual CollisionAlgorithmCreateFunc* getCollisionAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1);

	virtual CollisionAlgorithmCreateFunc* getClosestPointsAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1);

	///Use this method to allow to generate multiple contact points between at once, between two objects using the generic convex-convex algorithm.
	///By default, this feature is disabled for best performance.
	///@param numPerturbationIterations controls the number of collision queries. Set it to zero to disable the feature.
	///@param minimumPointsPerturbationThreshold is the minimum number of points in the contact cache, above which the feature is disabled
	///3 is a good value for both params, if you want to enable the feature. This is because the default contact cache contains a maximum of 4 points, and one collision query at the unperturbed orientation is performed first.
	///See drx3D/Demos/CollisionDemo for an example how this feature gathers multiple points.
	///@todo we could add a per-object setting of those parameters, for level-of-detail collision detection.
	void setConvexConvexMultipointIterations(i32 numPerturbationIterations = 3, i32 minimumPointsPerturbationThreshold = 3);

	void setPlaneConvexMultipointIterations(i32 numPerturbationIterations = 3, i32 minimumPointsPerturbationThreshold = 3);
};

#endif  //DRX3D_DEFAULT_COLLISION_CONFIGURATION
