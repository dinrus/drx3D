#include <drx3D/Physics/Collision/Dispatch/DefaultCollisionConfiguration.h>

#include <drx3D/Physics/Collision/Dispatch/ConvexConvexAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/EmptyCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/ConvexConcaveCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CompoundCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/CompoundCompoundCollisionAlgorithm.h>

#include <drx3D/Physics/Collision/Dispatch/ConvexPlaneCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/BoxBoxCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/SphereSphereCollisionAlgorithm.h>
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
#include <drx3D/Physics/Collision/Dispatch/SphereBoxCollisionAlgorithm.h>
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM
#include <drx3D/Physics/Collision/Dispatch/SphereTriangleCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpaPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/MinkowskiPenetrationDepthSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>

#include <drx3D/Maths/Linear/PoolAllocator.h>

DefaultCollisionConfiguration::DefaultCollisionConfiguration(const DefaultCollisionConstructionInfo& constructionInfo)
//DefaultCollisionConfiguration::DefaultCollisionConfiguration(StackAlloc*	stackAlloc,PoolAllocator*	persistentManifoldPool,PoolAllocator*	collisionAlgorithmPool)
{
	uk mem = NULL;
	if (constructionInfo.m_useEpaPenetrationAlgorithm)
	{
		mem = AlignedAlloc(sizeof(GjkEpaPenetrationDepthSolver), 16);
		m_pdSolver = new (mem) GjkEpaPenetrationDepthSolver;
	}
	else
	{
		mem = AlignedAlloc(sizeof(MinkowskiPenetrationDepthSolver), 16);
		m_pdSolver = new (mem) MinkowskiPenetrationDepthSolver;
	}

	//default CreationFunctions, filling the m_doubleDispatch table
	mem = AlignedAlloc(sizeof(ConvexConvexAlgorithm::CreateFunc), 16);
	m_convexConvexCreateFunc = new (mem) ConvexConvexAlgorithm::CreateFunc(m_pdSolver);
	mem = AlignedAlloc(sizeof(ConvexConcaveCollisionAlgorithm::CreateFunc), 16);
	m_convexConcaveCreateFunc = new (mem) ConvexConcaveCollisionAlgorithm::CreateFunc;
	mem = AlignedAlloc(sizeof(ConvexConcaveCollisionAlgorithm::CreateFunc), 16);
	m_swappedConvexConcaveCreateFunc = new (mem) ConvexConcaveCollisionAlgorithm::SwappedCreateFunc;
	mem = AlignedAlloc(sizeof(CompoundCollisionAlgorithm::CreateFunc), 16);
	m_compoundCreateFunc = new (mem) CompoundCollisionAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(CompoundCompoundCollisionAlgorithm::CreateFunc), 16);
	m_compoundCompoundCreateFunc = new (mem) CompoundCompoundCollisionAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(CompoundCollisionAlgorithm::SwappedCreateFunc), 16);
	m_swappedCompoundCreateFunc = new (mem) CompoundCollisionAlgorithm::SwappedCreateFunc;
	mem = AlignedAlloc(sizeof(EmptyAlgorithm::CreateFunc), 16);
	m_emptyCreateFunc = new (mem) EmptyAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(SphereSphereCollisionAlgorithm::CreateFunc), 16);
	m_sphereSphereCF = new (mem) SphereSphereCollisionAlgorithm::CreateFunc;
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
	mem = AlignedAlloc(sizeof(SphereBoxCollisionAlgorithm::CreateFunc), 16);
	m_sphereBoxCF = new (mem) SphereBoxCollisionAlgorithm::CreateFunc;
	mem = AlignedAlloc(sizeof(SphereBoxCollisionAlgorithm::CreateFunc), 16);
	m_boxSphereCF = new (mem) SphereBoxCollisionAlgorithm::CreateFunc;
	m_boxSphereCF->m_swapped = true;
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM

	mem = AlignedAlloc(sizeof(SphereTriangleCollisionAlgorithm::CreateFunc), 16);
	m_sphereTriangleCF = new (mem) SphereTriangleCollisionAlgorithm::CreateFunc;
	mem = AlignedAlloc(sizeof(SphereTriangleCollisionAlgorithm::CreateFunc), 16);
	m_triangleSphereCF = new (mem) SphereTriangleCollisionAlgorithm::CreateFunc;
	m_triangleSphereCF->m_swapped = true;

	mem = AlignedAlloc(sizeof(BoxBoxCollisionAlgorithm::CreateFunc), 16);
	m_boxBoxCF = new (mem) BoxBoxCollisionAlgorithm::CreateFunc;

	//convex versus plane
	mem = AlignedAlloc(sizeof(ConvexPlaneCollisionAlgorithm::CreateFunc), 16);
	m_convexPlaneCF = new (mem) ConvexPlaneCollisionAlgorithm::CreateFunc;
	mem = AlignedAlloc(sizeof(ConvexPlaneCollisionAlgorithm::CreateFunc), 16);
	m_planeConvexCF = new (mem) ConvexPlaneCollisionAlgorithm::CreateFunc;
	m_planeConvexCF->m_swapped = true;

	///calculate maximum element size, big enough to fit any collision algorithm in the memory pool
	i32 maxSize = sizeof(ConvexConvexAlgorithm);
	i32 maxSize2 = sizeof(ConvexConcaveCollisionAlgorithm);
	i32 maxSize3 = sizeof(CompoundCollisionAlgorithm);
	i32 maxSize4 = sizeof(CompoundCompoundCollisionAlgorithm);

	i32 collisionAlgorithmMaxElementSize = d3Max(maxSize, constructionInfo.m_customCollisionAlgorithmMaxElementSize);
	collisionAlgorithmMaxElementSize = d3Max(collisionAlgorithmMaxElementSize, maxSize2);
	collisionAlgorithmMaxElementSize = d3Max(collisionAlgorithmMaxElementSize, maxSize3);
	collisionAlgorithmMaxElementSize = d3Max(collisionAlgorithmMaxElementSize, maxSize4);

	if (constructionInfo.m_persistentManifoldPool)
	{
		m_ownsPersistentManifoldPool = false;
		m_persistentManifoldPool = constructionInfo.m_persistentManifoldPool;
	}
	else
	{
		m_ownsPersistentManifoldPool = true;
		uk mem = AlignedAlloc(sizeof(PoolAllocator), 16);
		m_persistentManifoldPool = new (mem) PoolAllocator(sizeof(PersistentManifold), constructionInfo.m_defaultMaxPersistentManifoldPoolSize);
	}

	collisionAlgorithmMaxElementSize = (collisionAlgorithmMaxElementSize + 16) & 0xffffffffffff0;
	if (constructionInfo.m_collisionAlgorithmPool)
	{
		m_ownsCollisionAlgorithmPool = false;
		m_collisionAlgorithmPool = constructionInfo.m_collisionAlgorithmPool;
	}
	else
	{
		m_ownsCollisionAlgorithmPool = true;
		uk mem = AlignedAlloc(sizeof(PoolAllocator), 16);
		m_collisionAlgorithmPool = new (mem) PoolAllocator(collisionAlgorithmMaxElementSize, constructionInfo.m_defaultMaxCollisionAlgorithmPoolSize);
	}
}

DefaultCollisionConfiguration::~DefaultCollisionConfiguration()
{
	if (m_ownsCollisionAlgorithmPool)
	{
		m_collisionAlgorithmPool->~PoolAllocator();
		AlignedFree(m_collisionAlgorithmPool);
	}
	if (m_ownsPersistentManifoldPool)
	{
		m_persistentManifoldPool->~PoolAllocator();
		AlignedFree(m_persistentManifoldPool);
	}

	m_convexConvexCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_convexConvexCreateFunc);

	m_convexConcaveCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_convexConcaveCreateFunc);
	m_swappedConvexConcaveCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_swappedConvexConcaveCreateFunc);

	m_compoundCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_compoundCreateFunc);

	m_compoundCompoundCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_compoundCompoundCreateFunc);

	m_swappedCompoundCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_swappedCompoundCreateFunc);

	m_emptyCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_emptyCreateFunc);

	m_sphereSphereCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_sphereSphereCF);

#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
	m_sphereBoxCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_sphereBoxCF);
	m_boxSphereCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_boxSphereCF);
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM

	m_sphereTriangleCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_sphereTriangleCF);
	m_triangleSphereCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_triangleSphereCF);
	m_boxBoxCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_boxBoxCF);

	m_convexPlaneCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_convexPlaneCF);
	m_planeConvexCF->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_planeConvexCF);

	m_pdSolver->~ConvexPenetrationDepthSolver();

	AlignedFree(m_pdSolver);
}

CollisionAlgorithmCreateFunc* DefaultCollisionConfiguration::getClosestPointsAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1)
{
	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_sphereSphereCF;
	}
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == BOX_SHAPE_PROXYTYPE))
	{
		return m_sphereBoxCF;
	}

	if ((proxyType0 == BOX_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_boxSphereCF;
	}
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM

	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == TRIANGLE_SHAPE_PROXYTYPE))
	{
		return m_sphereTriangleCF;
	}

	if ((proxyType0 == TRIANGLE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_triangleSphereCF;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && (proxyType1 == STATIC_PLANE_PROXYTYPE))
	{
		return m_convexPlaneCF;
	}

	if (BroadphaseProxy::isConvex(proxyType1) && (proxyType0 == STATIC_PLANE_PROXYTYPE))
	{
		return m_planeConvexCF;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && BroadphaseProxy::isConvex(proxyType1))
	{
		return m_convexConvexCreateFunc;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && BroadphaseProxy::isConcave(proxyType1))
	{
		return m_convexConcaveCreateFunc;
	}

	if (BroadphaseProxy::isConvex(proxyType1) && BroadphaseProxy::isConcave(proxyType0))
	{
		return m_swappedConvexConcaveCreateFunc;
	}

	if (BroadphaseProxy::isCompound(proxyType0) && BroadphaseProxy::isCompound(proxyType1))
	{
		return m_compoundCompoundCreateFunc;
	}

	if (BroadphaseProxy::isCompound(proxyType0))
	{
		return m_compoundCreateFunc;
	}
	else
	{
		if (BroadphaseProxy::isCompound(proxyType1))
		{
			return m_swappedCompoundCreateFunc;
		}
	}

	//failed to find an algorithm
	return m_emptyCreateFunc;
}

CollisionAlgorithmCreateFunc* DefaultCollisionConfiguration::getCollisionAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1)
{
	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_sphereSphereCF;
	}
#ifdef USE_BUGGY_SPHERE_BOX_ALGORITHM
	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == BOX_SHAPE_PROXYTYPE))
	{
		return m_sphereBoxCF;
	}

	if ((proxyType0 == BOX_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_boxSphereCF;
	}
#endif  //USE_BUGGY_SPHERE_BOX_ALGORITHM

	if ((proxyType0 == SPHERE_SHAPE_PROXYTYPE) && (proxyType1 == TRIANGLE_SHAPE_PROXYTYPE))
	{
		return m_sphereTriangleCF;
	}

	if ((proxyType0 == TRIANGLE_SHAPE_PROXYTYPE) && (proxyType1 == SPHERE_SHAPE_PROXYTYPE))
	{
		return m_triangleSphereCF;
	}

	if ((proxyType0 == BOX_SHAPE_PROXYTYPE) && (proxyType1 == BOX_SHAPE_PROXYTYPE))
	{
		return m_boxBoxCF;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && (proxyType1 == STATIC_PLANE_PROXYTYPE))
	{
		return m_convexPlaneCF;
	}

	if (BroadphaseProxy::isConvex(proxyType1) && (proxyType0 == STATIC_PLANE_PROXYTYPE))
	{
		return m_planeConvexCF;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && BroadphaseProxy::isConvex(proxyType1))
	{
		return m_convexConvexCreateFunc;
	}

	if (BroadphaseProxy::isConvex(proxyType0) && BroadphaseProxy::isConcave(proxyType1))
	{
		return m_convexConcaveCreateFunc;
	}

	if (BroadphaseProxy::isConvex(proxyType1) && BroadphaseProxy::isConcave(proxyType0))
	{
		return m_swappedConvexConcaveCreateFunc;
	}

	if (BroadphaseProxy::isCompound(proxyType0) && BroadphaseProxy::isCompound(proxyType1))
	{
		return m_compoundCompoundCreateFunc;
	}

	if (BroadphaseProxy::isCompound(proxyType0))
	{
		return m_compoundCreateFunc;
	}
	else
	{
		if (BroadphaseProxy::isCompound(proxyType1))
		{
			return m_swappedCompoundCreateFunc;
		}
	}

	//failed to find an algorithm
	return m_emptyCreateFunc;
}

void DefaultCollisionConfiguration::setConvexConvexMultipointIterations(i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold)
{
	ConvexConvexAlgorithm::CreateFunc* convexConvex = (ConvexConvexAlgorithm::CreateFunc*)m_convexConvexCreateFunc;
	convexConvex->m_numPerturbationIterations = numPerturbationIterations;
	convexConvex->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;
}

void DefaultCollisionConfiguration::setPlaneConvexMultipointIterations(i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold)
{
	ConvexPlaneCollisionAlgorithm::CreateFunc* cpCF = (ConvexPlaneCollisionAlgorithm::CreateFunc*)m_convexPlaneCF;
	cpCF->m_numPerturbationIterations = numPerturbationIterations;
	cpCF->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;

	ConvexPlaneCollisionAlgorithm::CreateFunc* pcCF = (ConvexPlaneCollisionAlgorithm::CreateFunc*)m_planeConvexCF;
	pcCF->m_numPerturbationIterations = numPerturbationIterations;
	pcCF->m_minimumPointsPerturbationThreshold = minimumPointsPerturbationThreshold;
}
