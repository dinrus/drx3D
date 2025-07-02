#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/SoftBody/SoftRigidCollisionAlgorithm.h>
#include <drx3D/Physics/SoftBody/SoftBodyConcaveCollisionAlgorithm.h>
#include <drx3D/Physics/SoftBody/SoftSoftCollisionAlgorithm.h>

#include <drx3D/Maths/Linear/PoolAllocator.h>

#define ENABLE_SOFTBODY_CONCAVE_COLLISIONS 1

SoftBodyRigidBodyCollisionConfiguration::SoftBodyRigidBodyCollisionConfiguration(const DefaultCollisionConstructionInfo& constructionInfo)
	: DefaultCollisionConfiguration(constructionInfo)
{
	uk mem;

	mem = AlignedAlloc(sizeof(SoftSoftCollisionAlgorithm::CreateFunc), 16);
	m_softSoftCreateFunc = new (mem) SoftSoftCollisionAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(SoftRigidCollisionAlgorithm::CreateFunc), 16);
	m_softRigidConvexCreateFunc = new (mem) SoftRigidCollisionAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(SoftRigidCollisionAlgorithm::CreateFunc), 16);
	m_swappedSoftRigidConvexCreateFunc = new (mem) SoftRigidCollisionAlgorithm::CreateFunc;
	m_swappedSoftRigidConvexCreateFunc->m_swapped = true;

#ifdef ENABLE_SOFTBODY_CONCAVE_COLLISIONS
	mem = AlignedAlloc(sizeof(SoftBodyConcaveCollisionAlgorithm::CreateFunc), 16);
	m_softRigidConcaveCreateFunc = new (mem) SoftBodyConcaveCollisionAlgorithm::CreateFunc;

	mem = AlignedAlloc(sizeof(SoftBodyConcaveCollisionAlgorithm::CreateFunc), 16);
	m_swappedSoftRigidConcaveCreateFunc = new (mem) SoftBodyConcaveCollisionAlgorithm::SwappedCreateFunc;
	m_swappedSoftRigidConcaveCreateFunc->m_swapped = true;
#endif

	//replace pool by a new one, with potential larger size

	if (m_ownsCollisionAlgorithmPool && m_collisionAlgorithmPool)
	{
		i32 curElemSize = m_collisionAlgorithmPool->getElementSize();
		///calculate maximum element size, big enough to fit any collision algorithm in the memory pool

		i32 maxSize0 = sizeof(SoftSoftCollisionAlgorithm);
		i32 maxSize1 = sizeof(SoftRigidCollisionAlgorithm);
		i32 maxSize2 = sizeof(SoftBodyConcaveCollisionAlgorithm);

		i32 collisionAlgorithmMaxElementSize = d3Max(maxSize0, maxSize1);
		collisionAlgorithmMaxElementSize = d3Max(collisionAlgorithmMaxElementSize, maxSize2);

		if (collisionAlgorithmMaxElementSize > curElemSize)
		{
			m_collisionAlgorithmPool->~PoolAllocator();
			AlignedFree(m_collisionAlgorithmPool);
			uk mem = AlignedAlloc(sizeof(PoolAllocator), 16);
			m_collisionAlgorithmPool = new (mem) PoolAllocator(collisionAlgorithmMaxElementSize, constructionInfo.m_defaultMaxCollisionAlgorithmPoolSize);
		}
	}
}

SoftBodyRigidBodyCollisionConfiguration::~SoftBodyRigidBodyCollisionConfiguration()
{
	m_softSoftCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_softSoftCreateFunc);

	m_softRigidConvexCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_softRigidConvexCreateFunc);

	m_swappedSoftRigidConvexCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_swappedSoftRigidConvexCreateFunc);

#ifdef ENABLE_SOFTBODY_CONCAVE_COLLISIONS
	m_softRigidConcaveCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_softRigidConcaveCreateFunc);

	m_swappedSoftRigidConcaveCreateFunc->~CollisionAlgorithmCreateFunc();
	AlignedFree(m_swappedSoftRigidConcaveCreateFunc);
#endif
}

///creation of soft-soft and soft-rigid, and otherwise fallback to base class implementation
CollisionAlgorithmCreateFunc* SoftBodyRigidBodyCollisionConfiguration::getCollisionAlgorithmCreateFunc(i32 proxyType0, i32 proxyType1)
{
	///try to handle the softbody interactions first

	if ((proxyType0 == SOFTBODY_SHAPE_PROXYTYPE) && (proxyType1 == SOFTBODY_SHAPE_PROXYTYPE))
	{
		return m_softSoftCreateFunc;
	}

	///softbody versus convex
	if (proxyType0 == SOFTBODY_SHAPE_PROXYTYPE && BroadphaseProxy::isConvex(proxyType1))
	{
		return m_softRigidConvexCreateFunc;
	}

	///convex versus soft body
	if (BroadphaseProxy::isConvex(proxyType0) && proxyType1 == SOFTBODY_SHAPE_PROXYTYPE)
	{
		return m_swappedSoftRigidConvexCreateFunc;
	}

#ifdef ENABLE_SOFTBODY_CONCAVE_COLLISIONS
	///softbody versus convex
	if (proxyType0 == SOFTBODY_SHAPE_PROXYTYPE && BroadphaseProxy::isConcave(proxyType1))
	{
		return m_softRigidConcaveCreateFunc;
	}

	///convex versus soft body
	if (BroadphaseProxy::isConcave(proxyType0) && proxyType1 == SOFTBODY_SHAPE_PROXYTYPE)
	{
		return m_swappedSoftRigidConcaveCreateFunc;
	}
#endif

	///fallback to the regular rigid collision shape
	return DefaultCollisionConfiguration::getCollisionAlgorithmCreateFunc(proxyType0, proxyType1);
}
