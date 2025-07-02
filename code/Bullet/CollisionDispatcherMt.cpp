#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcherMt.h>
#include <drx3D/Maths/Linear/Quickprof.h>

#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>
#include <drx3D/Maths/Linear/PoolAllocator.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionConfiguration.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

CollisionDispatcherMt::CollisionDispatcherMt(CollisionConfiguration* config, i32 grainSize)
	: CollisionDispatcher(config)
{
	m_batchManifoldsPtr.resize(GetTaskScheduler()->getNumThreads());
	m_batchReleasePtr.resize(GetTaskScheduler()->getNumThreads());

	m_batchUpdating = false;
	m_grainSize = grainSize;  // iterations per task
}

PersistentManifold* CollisionDispatcherMt::getNewManifold(const CollisionObject2* body0, const CollisionObject2* body1)
{
	//optional relative contact breaking threshold, turned on by default (use setDispatcherFlags to switch off feature for improved performance)

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
	if (!m_batchUpdating)
	{
		// batch updater will update manifold pointers array after finishing, so
		// only need to update array when not batch-updating
		//Assert( !ThreadsAreRunning() );
		manifold->m_index1a = m_manifoldsPtr.size();
		m_manifoldsPtr.push_back(manifold);
	}
	else
	{
		m_batchManifoldsPtr[GetCurrentThreadIndex()].push_back(manifold);
	}

	return manifold;
}

void CollisionDispatcherMt::releaseManifold(PersistentManifold* manifold)
{
	//Assert( !ThreadsAreRunning() );
	
	if (!m_batchUpdating)
	{
		clearManifold(manifold);
		// batch updater will update manifold pointers array after finishing, so
		// only need to update array when not batch-updating
		i32 findIndex = manifold->m_index1a;
		Assert(findIndex < m_manifoldsPtr.size());
		m_manifoldsPtr.swap(findIndex, m_manifoldsPtr.size() - 1);
		m_manifoldsPtr[findIndex]->m_index1a = findIndex;
		m_manifoldsPtr.pop_back();
	} else {
		m_batchReleasePtr[GetCurrentThreadIndex()].push_back(manifold);
		return;
	}

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

struct DispatcherUpdater : public IParallelForBody
{
	BroadphasePair* mPairArray;
	NearCallback mCallback;
	CollisionDispatcher* mDispatcher;
	const DispatcherInfo* mInfo;

	DispatcherUpdater()
	{
		mPairArray = NULL;
		mCallback = NULL;
		mDispatcher = NULL;
		mInfo = NULL;
	}
	void forLoop(i32 iBegin, i32 iEnd) const
	{
		for (i32 i = iBegin; i < iEnd; ++i)
		{
			BroadphasePair* pair = &mPairArray[i];
			mCallback(*pair, *mDispatcher, *mInfo);
		}
	}
};

void CollisionDispatcherMt::dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& info, Dispatcher* dispatcher)
{
	i32k pairCount = pairCache->getNumOverlappingPairs();
	if (pairCount == 0)
	{
		return;
	}
	DispatcherUpdater updater;
	updater.mCallback = getNearCallback();
	updater.mPairArray = pairCache->getOverlappingPairArrayPtr();
	updater.mDispatcher = this;
	updater.mInfo = &info;

	m_batchUpdating = true;
	ParallelFor(0, pairCount, m_grainSize, updater);
	m_batchUpdating = false;

	// merge new manifolds, if any
	for (i32 i = 0; i < m_batchManifoldsPtr.size(); ++i)
	{
		AlignedObjectArray<PersistentManifold*>& batchManifoldsPtr = m_batchManifoldsPtr[i];

		for (i32 j = 0; j < batchManifoldsPtr.size(); ++j)
		{
			m_manifoldsPtr.push_back(batchManifoldsPtr[j]);
		}

		batchManifoldsPtr.resizeNoInitialize(0);
	}

	// remove batched remove manifolds.
	for (i32 i = 0; i < m_batchReleasePtr.size(); ++i)
	{
		AlignedObjectArray<PersistentManifold*>& batchManifoldsPtr = m_batchReleasePtr[i];
		for (i32 j = 0; j < batchManifoldsPtr.size(); ++j)
		{
			releaseManifold(batchManifoldsPtr[j]);
		}
		batchManifoldsPtr.resizeNoInitialize(0);
	}

	// update the indices (used when releasing manifolds)
	for (i32 i = 0; i < m_manifoldsPtr.size(); ++i)
	{
		m_manifoldsPtr[i]->m_index1a = i;
	}
}
