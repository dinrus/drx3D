#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Maths/Linear/Quickprof.h>

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Maths/Linear/PoolAllocator.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionConfiguration.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

#ifdef DRX3D_DEBUG
#include <stdio.h>
#endif

CollisionDispatcher::CollisionDispatcher(CollisionConfiguration* collisionConfiguration) : m_dispatcherFlags(CollisionDispatcher::CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD),
																								 m_collisionConfiguration(collisionConfiguration)
{
	i32 i;

	setNearCallback(defaultNearCallback);

	m_collisionAlgorithmPoolAllocator = collisionConfiguration->getCollisionAlgorithmPool();

	m_persistentManifoldPoolAllocator = collisionConfiguration->getPersistentManifoldPool();

	for (i = 0; i < MAX_BROADPHASE_COLLISION_TYPES; i++)
	{
		for (i32 j = 0; j < MAX_BROADPHASE_COLLISION_TYPES; j++)
		{
			m_doubleDispatchContactPoints[i][j] = m_collisionConfiguration->getCollisionAlgorithmCreateFunc(i, j);
			Assert(m_doubleDispatchContactPoints[i][j]);
			m_doubleDispatchClosestPoints[i][j] = m_collisionConfiguration->getClosestPointsAlgorithmCreateFunc(i, j);
		}
	}
}

void CollisionDispatcher::registerCollisionCreateFunc(i32 proxyType0, i32 proxyType1, CollisionAlgorithmCreateFunc* createFunc)
{
	m_doubleDispatchContactPoints[proxyType0][proxyType1] = createFunc;
}

void CollisionDispatcher::registerClosestPointsCreateFunc(i32 proxyType0, i32 proxyType1, CollisionAlgorithmCreateFunc* createFunc)
{
	m_doubleDispatchClosestPoints[proxyType0][proxyType1] = createFunc;
}

CollisionDispatcher::~CollisionDispatcher()
{
}

PersistentManifold* CollisionDispatcher::getNewManifold(const CollisionObject2* body0, const CollisionObject2* body1)
{
	//Assert(gNumManifold < 65535);

	//optional relative contact breaking threshold, turned on by default (use setCollisionDispatcherFlags to switch off feature for improved performance)

	Scalar contactBreakingThreshold = (m_dispatcherFlags & CollisionDispatcher::CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD) ? d3Min(body0->getCollisionShape()->getContactBreakingThreshold(gContactBreakingThreshold), body1->getCollisionShape()->getContactBreakingThreshold(gContactBreakingThreshold))
																																: gContactBreakingThreshold;

	Scalar contactProcessingThreshold = d3Min(body0->getContactProcessingThreshold(), body1->getContactProcessingThreshold());

	uk mem = m_persistentManifoldPoolAllocator->allocate(sizeof(PersistentManifold));
	if (NULL == mem)
	{
		//we got a pool memory overflow, by default we fallback to dynamically allocate memory. If we require a contiguous contact pool then assert.
		if ((m_dispatcherFlags & CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION) == 0)
		{
			mem = AlignedAlloc(sizeof(PersistentManifold), 16);
		}
		else
		{
			Assert(0);
			//make sure to increase the m_defaultMaxPersistentManifoldPoolSize in the DefaultCollisionConstructionInfo/DefaultCollisionConfiguration
			return 0;
		}
	}
	PersistentManifold* manifold = new (mem) PersistentManifold(body0, body1, 0, contactBreakingThreshold, contactProcessingThreshold);
	manifold->m_index1a = m_manifoldsPtr.size();
	m_manifoldsPtr.push_back(manifold);

	return manifold;
}

void CollisionDispatcher::clearManifold(PersistentManifold* manifold)
{
	manifold->clearManifold();
}

void CollisionDispatcher::releaseManifold(PersistentManifold* manifold)
{
	//printf("releaseManifold: gNumManifold %d\n",gNumManifold);
	clearManifold(manifold);

	i32 findIndex = manifold->m_index1a;
	Assert(findIndex < m_manifoldsPtr.size());
	m_manifoldsPtr.swap(findIndex, m_manifoldsPtr.size() - 1);
	m_manifoldsPtr[findIndex]->m_index1a = findIndex;
	m_manifoldsPtr.pop_back();

	manifold->~PersistentManifold();
	if (m_persistentManifoldPoolAllocator->validPtr(manifold))
	{
		m_persistentManifoldPoolAllocator->freeMemory(manifold);
	}
	else
	{
		AlignedFree(manifold);
	}
}

CollisionAlgorithm* CollisionDispatcher::findAlgorithm(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, PersistentManifold* sharedManifold, eDispatcherQueryType algoType)
{
	CollisionAlgorithmConstructionInfo ci;

	ci.m_dispatcher1 = this;
	ci.m_manifold = sharedManifold;
	CollisionAlgorithm* algo = 0;
	if (algoType == DRX3D_CONTACT_POINT_ALGORITHMS)
	{
		algo = m_doubleDispatchContactPoints[body0Wrap->getCollisionShape()->getShapeType()][body1Wrap->getCollisionShape()->getShapeType()]->CreateCollisionAlgorithm(ci, body0Wrap, body1Wrap);
	}
	else
	{
		algo = m_doubleDispatchClosestPoints[body0Wrap->getCollisionShape()->getShapeType()][body1Wrap->getCollisionShape()->getShapeType()]->CreateCollisionAlgorithm(ci, body0Wrap, body1Wrap);
	}

	return algo;
}

bool CollisionDispatcher::needsResponse(const CollisionObject2* body0, const CollisionObject2* body1)
{
	//here you can do filtering
	bool hasResponse =
		(body0->hasContactResponse() && body1->hasContactResponse());
	//no response between two static/kinematic bodies:
	hasResponse = hasResponse &&
				  ((!body0->isStaticOrKinematicObject()) || (!body1->isStaticOrKinematicObject()));
	return hasResponse;
}

bool CollisionDispatcher::needsCollision(const CollisionObject2* body0, const CollisionObject2* body1)
{
	Assert(body0);
	Assert(body1);

	bool needsCollision = true;

#ifdef DRX3D_DEBUG
	if (!(m_dispatcherFlags & CollisionDispatcher::CD_STATIC_STATIC_REPORTED))
	{
		//broadphase filtering already deals with this
		if (body0->isStaticOrKinematicObject() && body1->isStaticOrKinematicObject())
		{
			m_dispatcherFlags |= CollisionDispatcher::CD_STATIC_STATIC_REPORTED;
			printf("warning CollisionDispatcher::needsCollision: static-static collision!\n");
		}
	}
#endif  //DRX3D_DEBUG

	if ((!body0->isActive()) && (!body1->isActive()))
		needsCollision = false;
	else if ((!body0->checkCollideWith(body1)) || (!body1->checkCollideWith(body0)))
		needsCollision = false;

	return needsCollision;
}

///interface for iterating all overlapping collision pairs, no matter how those pairs are stored (array, set, map etc)
///this is useful for the collision dispatcher.
class CollisionPairCallback : public OverlapCallback
{
	const DispatcherInfo& m_dispatchInfo;
	CollisionDispatcher* m_dispatcher;

public:
	CollisionPairCallback(const DispatcherInfo& dispatchInfo, CollisionDispatcher* dispatcher)
		: m_dispatchInfo(dispatchInfo),
		  m_dispatcher(dispatcher)
	{
	}

	/*CollisionPairCallback& operator=(CollisionPairCallback& other)
	{
		m_dispatchInfo = other.m_dispatchInfo;
		m_dispatcher = other.m_dispatcher;
		return *this;
	}
	*/

	virtual ~CollisionPairCallback() {}

	virtual bool processOverlap(BroadphasePair& pair)
	{
		(*m_dispatcher->getNearCallback())(pair, *m_dispatcher, m_dispatchInfo);
		return false;
	}
};

void CollisionDispatcher::dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& dispatchInfo, Dispatcher* dispatcher)
{
	//m_blockedForChanges = true;

	CollisionPairCallback collisionCallback(dispatchInfo, this);

	{
		DRX3D_PROFILE("processAllOverlappingPairs");
		pairCache->processAllOverlappingPairs(&collisionCallback, dispatcher, dispatchInfo);
	}

	//m_blockedForChanges = false;
}

//by default, drx3D will use this near callback
void CollisionDispatcher::defaultNearCallback(BroadphasePair& collisionPair, CollisionDispatcher& dispatcher, const DispatcherInfo& dispatchInfo)
{
	CollisionObject2* colObj0 = (CollisionObject2*)collisionPair.m_pProxy0->m_clientObject;
	CollisionObject2* colObj1 = (CollisionObject2*)collisionPair.m_pProxy1->m_clientObject;

	if (dispatcher.needsCollision(colObj0, colObj1))
	{
		CollisionObject2Wrapper obj0Wrap(0, colObj0->getCollisionShape(), colObj0, colObj0->getWorldTransform(), -1, -1);
		CollisionObject2Wrapper obj1Wrap(0, colObj1->getCollisionShape(), colObj1, colObj1->getWorldTransform(), -1, -1);

		//dispatcher will keep algorithms persistent in the collision pair
		if (!collisionPair.m_algorithm)
		{
			collisionPair.m_algorithm = dispatcher.findAlgorithm(&obj0Wrap, &obj1Wrap, 0, DRX3D_CONTACT_POINT_ALGORITHMS);
		}

		if (collisionPair.m_algorithm)
		{
			ManifoldResult contactPointResult(&obj0Wrap, &obj1Wrap);

			if (dispatchInfo.m_dispatchFunc == DispatcherInfo::DISPATCH_DISCRETE)
			{
				//discrete collision detection query

				collisionPair.m_algorithm->processCollision(&obj0Wrap, &obj1Wrap, dispatchInfo, &contactPointResult);
			}
			else
			{
				//continuous collision detection query, time of impact (toi)
				Scalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0, colObj1, dispatchInfo, &contactPointResult);
				if (dispatchInfo.m_timeOfImpact > toi)
					dispatchInfo.m_timeOfImpact = toi;
			}
		}
	}
}

uk CollisionDispatcher::allocateCollisionAlgorithm(i32 size)
{
	uk mem = m_collisionAlgorithmPoolAllocator->allocate(size);
	if (NULL == mem)
	{
		//warn user for overflow?
		return AlignedAlloc(static_cast<size_t>(size), 16);
	}
	return mem;
}

void CollisionDispatcher::freeCollisionAlgorithm(uk ptr)
{
	if (m_collisionAlgorithmPoolAllocator->validPtr(ptr))
	{
		m_collisionAlgorithmPoolAllocator->freeMemory(ptr);
	}
	else
	{
		AlignedFree(ptr);
	}
}
