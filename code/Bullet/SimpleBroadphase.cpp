#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

#include <new>

void SimpleBroadphase::validate()
{
	for (i32 i = 0; i < m_numHandles; i++)
	{
		for (i32 j = i + 1; j < m_numHandles; j++)
		{
			Assert(&m_pHandles[i] != &m_pHandles[j]);
		}
	}
}

SimpleBroadphase::SimpleBroadphase(i32 maxProxies, OverlappingPairCache* overlappingPairCache)
	: m_pairCache(overlappingPairCache),
	  m_ownsPairCache(false),
	  m_invalidPair(0)
{
	if (!overlappingPairCache)
	{
		uk mem = AlignedAlloc(sizeof(HashedOverlappingPairCache), 16);
		m_pairCache = new (mem) HashedOverlappingPairCache();
		m_ownsPairCache = true;
	}

	// allocate handles buffer and put all handles on free list
	m_pHandlesRawPtr = AlignedAlloc(sizeof(SimpleBroadphaseProxy) * maxProxies, 16);
	m_pHandles = new (m_pHandlesRawPtr) SimpleBroadphaseProxy[maxProxies];
	m_maxHandles = maxProxies;
	m_numHandles = 0;
	m_firstFreeHandle = 0;
	m_LastHandleIndex = -1;

	{
		for (i32 i = m_firstFreeHandle; i < maxProxies; i++)
		{
			m_pHandles[i].SetNextFree(i + 1);
			m_pHandles[i].m_uniqueId = i + 2;  //any UID will do, we just avoid too trivial values (0,1) for debugging purposes
		}
		m_pHandles[maxProxies - 1].SetNextFree(0);
	}
}

SimpleBroadphase::~SimpleBroadphase()
{
	AlignedFree(m_pHandlesRawPtr);

	if (m_ownsPairCache)
	{
		m_pairCache->~OverlappingPairCache();
		AlignedFree(m_pairCache);
	}
}

BroadphaseProxy* SimpleBroadphase::createProxy(const Vec3& aabbMin, const Vec3& aabbMax, i32 shapeType, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask, Dispatcher* /*dispatcher*/)
{
	if (m_numHandles >= m_maxHandles)
	{
		Assert(0);
		return 0;  //should never happen, but don't let the game crash ;-)
	}
	Assert(aabbMin[0] <= aabbMax[0] && aabbMin[1] <= aabbMax[1] && aabbMin[2] <= aabbMax[2]);

	i32 newHandleIndex = allocHandle();
	SimpleBroadphaseProxy* proxy = new (&m_pHandles[newHandleIndex]) SimpleBroadphaseProxy(aabbMin, aabbMax, shapeType, userPtr, collisionFilterGroup, collisionFilterMask);

	return proxy;
}

class RemovingOverlapCallback : public OverlapCallback
{
protected:
	virtual bool processOverlap(BroadphasePair& pair)
	{
		(void)pair;
		Assert(0);
		return false;
	}
};

class RemovePairContainingProxy
{
	BroadphaseProxy* m_targetProxy;

public:
	virtual ~RemovePairContainingProxy()
	{
	}

protected:
	virtual bool processOverlap(BroadphasePair& pair)
	{
		SimpleBroadphaseProxy* proxy0 = static_cast<SimpleBroadphaseProxy*>(pair.m_pProxy0);
		SimpleBroadphaseProxy* proxy1 = static_cast<SimpleBroadphaseProxy*>(pair.m_pProxy1);

		return ((m_targetProxy == proxy0 || m_targetProxy == proxy1));
	};
};

void SimpleBroadphase::destroyProxy(BroadphaseProxy* proxyOrg, Dispatcher* dispatcher)
{
	m_pairCache->removeOverlappingPairsContainingProxy(proxyOrg, dispatcher);

	SimpleBroadphaseProxy* proxy0 = static_cast<SimpleBroadphaseProxy*>(proxyOrg);
	freeHandle(proxy0);

	//validate();
}

void SimpleBroadphase::getAabb(BroadphaseProxy* proxy, Vec3& aabbMin, Vec3& aabbMax) const
{
	const SimpleBroadphaseProxy* sbp = getSimpleProxyFromProxy(proxy);
	aabbMin = sbp->m_aabbMin;
	aabbMax = sbp->m_aabbMax;
}

void SimpleBroadphase::setAabb(BroadphaseProxy* proxy, const Vec3& aabbMin, const Vec3& aabbMax, Dispatcher* /*dispatcher*/)
{
	SimpleBroadphaseProxy* sbp = getSimpleProxyFromProxy(proxy);
	sbp->m_aabbMin = aabbMin;
	sbp->m_aabbMax = aabbMax;
}

void SimpleBroadphase::rayTest(const Vec3& rayFrom, const Vec3& rayTo, BroadphaseRayCallback& rayCallback, const Vec3& aabbMin, const Vec3& aabbMax)
{
	for (i32 i = 0; i <= m_LastHandleIndex; i++)
	{
		SimpleBroadphaseProxy* proxy = &m_pHandles[i];
		if (!proxy->m_clientObject)
		{
			continue;
		}
		rayCallback.process(proxy);
	}
}

void SimpleBroadphase::aabbTest(const Vec3& aabbMin, const Vec3& aabbMax, BroadphaseAabbCallback& callback)
{
	for (i32 i = 0; i <= m_LastHandleIndex; i++)
	{
		SimpleBroadphaseProxy* proxy = &m_pHandles[i];
		if (!proxy->m_clientObject)
		{
			continue;
		}
		if (TestAabbAgainstAabb2(aabbMin, aabbMax, proxy->m_aabbMin, proxy->m_aabbMax))
		{
			callback.process(proxy);
		}
	}
}

bool SimpleBroadphase::aabbOverlap(SimpleBroadphaseProxy* proxy0, SimpleBroadphaseProxy* proxy1)
{
	return proxy0->m_aabbMin[0] <= proxy1->m_aabbMax[0] && proxy1->m_aabbMin[0] <= proxy0->m_aabbMax[0] &&
		   proxy0->m_aabbMin[1] <= proxy1->m_aabbMax[1] && proxy1->m_aabbMin[1] <= proxy0->m_aabbMax[1] &&
		   proxy0->m_aabbMin[2] <= proxy1->m_aabbMax[2] && proxy1->m_aabbMin[2] <= proxy0->m_aabbMax[2];
}

//then remove non-overlapping ones
class CheckOverlapCallback : public OverlapCallback
{
public:
	virtual bool processOverlap(BroadphasePair& pair)
	{
		return (!SimpleBroadphase::aabbOverlap(static_cast<SimpleBroadphaseProxy*>(pair.m_pProxy0), static_cast<SimpleBroadphaseProxy*>(pair.m_pProxy1)));
	}
};

void SimpleBroadphase::calculateOverlappingPairs(Dispatcher* dispatcher)
{
	//first check for new overlapping pairs
	i32 i, j;
	if (m_numHandles >= 0)
	{
		i32 new_largest_index = -1;
		for (i = 0; i <= m_LastHandleIndex; i++)
		{
			SimpleBroadphaseProxy* proxy0 = &m_pHandles[i];
			if (!proxy0->m_clientObject)
			{
				continue;
			}
			new_largest_index = i;
			for (j = i + 1; j <= m_LastHandleIndex; j++)
			{
				SimpleBroadphaseProxy* proxy1 = &m_pHandles[j];
				Assert(proxy0 != proxy1);
				if (!proxy1->m_clientObject)
				{
					continue;
				}

				SimpleBroadphaseProxy* p0 = getSimpleProxyFromProxy(proxy0);
				SimpleBroadphaseProxy* p1 = getSimpleProxyFromProxy(proxy1);

				if (aabbOverlap(p0, p1))
				{
					if (!m_pairCache->findPair(proxy0, proxy1))
					{
						m_pairCache->addOverlappingPair(proxy0, proxy1);
					}
				}
				else
				{
					if (!m_pairCache->hasDeferredRemoval())
					{
						if (m_pairCache->findPair(proxy0, proxy1))
						{
							m_pairCache->removeOverlappingPair(proxy0, proxy1, dispatcher);
						}
					}
				}
			}
		}

		m_LastHandleIndex = new_largest_index;

		if (m_ownsPairCache && m_pairCache->hasDeferredRemoval())
		{
			BroadphasePairArray& overlappingPairArray = m_pairCache->getOverlappingPairArray();

			//perform a sort, to find duplicates and to sort 'invalid' pairs to the end
			overlappingPairArray.quickSort(BroadphasePairSortPredicate());

			overlappingPairArray.resize(overlappingPairArray.size() - m_invalidPair);
			m_invalidPair = 0;

			BroadphasePair previousPair;
			previousPair.m_pProxy0 = 0;
			previousPair.m_pProxy1 = 0;
			previousPair.m_algorithm = 0;

			for (i = 0; i < overlappingPairArray.size(); i++)
			{
				BroadphasePair& pair = overlappingPairArray[i];

				bool isDuplicate = (pair == previousPair);

				previousPair = pair;

				bool needsRemoval = false;

				if (!isDuplicate)
				{
					bool hasOverlap = testAabbOverlap(pair.m_pProxy0, pair.m_pProxy1);

					if (hasOverlap)
					{
						needsRemoval = false;  //callback->processOverlap(pair);
					}
					else
					{
						needsRemoval = true;
					}
				}
				else
				{
					//remove duplicate
					needsRemoval = true;
					//should have no algorithm
					Assert(!pair.m_algorithm);
				}

				if (needsRemoval)
				{
					m_pairCache->cleanOverlappingPair(pair, dispatcher);

					//		m_overlappingPairArray.swap(i,m_overlappingPairArray.size()-1);
					//		m_overlappingPairArray.pop_back();
					pair.m_pProxy0 = 0;
					pair.m_pProxy1 = 0;
					m_invalidPair++;
				}
			}

			///if you don't like to skip the invalid pairs in the array, execute following code:
#define CLEAN_INVALID_PAIRS 1
#ifdef CLEAN_INVALID_PAIRS

			//perform a sort, to sort 'invalid' pairs to the end
			overlappingPairArray.quickSort(BroadphasePairSortPredicate());

			overlappingPairArray.resize(overlappingPairArray.size() - m_invalidPair);
			m_invalidPair = 0;
#endif  //CLEAN_INVALID_PAIRS
		}
	}
}

bool SimpleBroadphase::testAabbOverlap(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1)
{
	SimpleBroadphaseProxy* p0 = getSimpleProxyFromProxy(proxy0);
	SimpleBroadphaseProxy* p1 = getSimpleProxyFromProxy(proxy1);
	return aabbOverlap(p0, p1);
}

void SimpleBroadphase::resetPool(Dispatcher* dispatcher)
{
	//not yet
}
