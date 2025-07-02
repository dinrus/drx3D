#ifndef DRX3D_SIMPLE_BROADPHASE_H
#define DRX3D_SIMPLE_BROADPHASE_H

#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>

struct SimpleBroadphaseProxy : public BroadphaseProxy
{
	i32 m_nextFree;

	//	i32			m_handleId;

	SimpleBroadphaseProxy(){};

	SimpleBroadphaseProxy(const Vec3& minpt, const Vec3& maxpt, i32 shapeType, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
		: BroadphaseProxy(minpt, maxpt, userPtr, collisionFilterGroup, collisionFilterMask)
	{
		(void)shapeType;
	}

	SIMD_FORCE_INLINE void SetNextFree(i32 next) { m_nextFree = next; }
	SIMD_FORCE_INLINE i32 GetNextFree() const { return m_nextFree; }
};

///The SimpleBroadphase is just a unit-test for AxisSweep3, drx3D32BitAxisSweep3, or DbvtBroadphase, so use those classes instead.
///It is a brute force aabb culling broadphase based on O(n^2) aabb checks
class SimpleBroadphase : public BroadphaseInterface
{
protected:
	i32 m_numHandles;  // number of active handles
	i32 m_maxHandles;  // max number of handles
	i32 m_LastHandleIndex;

	SimpleBroadphaseProxy* m_pHandles;  // handles pool

	uk m_pHandlesRawPtr;
	i32 m_firstFreeHandle;  // free handles list

	i32 allocHandle()
	{
		Assert(m_numHandles < m_maxHandles);
		i32 freeHandle = m_firstFreeHandle;
		m_firstFreeHandle = m_pHandles[freeHandle].GetNextFree();
		m_numHandles++;
		if (freeHandle > m_LastHandleIndex)
		{
			m_LastHandleIndex = freeHandle;
		}
		return freeHandle;
	}

	void freeHandle(SimpleBroadphaseProxy* proxy)
	{
		i32 handle = i32(proxy - m_pHandles);
		Assert(handle >= 0 && handle < m_maxHandles);
		if (handle == m_LastHandleIndex)
		{
			m_LastHandleIndex--;
		}
		proxy->SetNextFree(m_firstFreeHandle);
		m_firstFreeHandle = handle;

		proxy->m_clientObject = 0;

		m_numHandles--;
	}

	OverlappingPairCache* m_pairCache;
	bool m_ownsPairCache;

	i32 m_invalidPair;

	inline SimpleBroadphaseProxy* getSimpleProxyFromProxy(BroadphaseProxy* proxy)
	{
		SimpleBroadphaseProxy* proxy0 = static_cast<SimpleBroadphaseProxy*>(proxy);
		return proxy0;
	}

	inline const SimpleBroadphaseProxy* getSimpleProxyFromProxy(BroadphaseProxy* proxy) const
	{
		const SimpleBroadphaseProxy* proxy0 = static_cast<const SimpleBroadphaseProxy*>(proxy);
		return proxy0;
	}

	///reset broadphase internal structures, to ensure determinism/reproducability
	virtual void resetPool(Dispatcher* dispatcher);

	void validate();

protected:
public:
	SimpleBroadphase(i32 maxProxies = 16384, OverlappingPairCache* overlappingPairCache = 0);
	virtual ~SimpleBroadphase();

	static bool aabbOverlap(SimpleBroadphaseProxy* proxy0, SimpleBroadphaseProxy* proxy1);

	virtual BroadphaseProxy* createProxy(const Vec3& aabbMin, const Vec3& aabbMax, i32 shapeType, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask, Dispatcher* dispatcher);

	virtual void calculateOverlappingPairs(Dispatcher* dispatcher);

	virtual void destroyProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher);
	virtual void setAabb(BroadphaseProxy* proxy, const Vec3& aabbMin, const Vec3& aabbMax, Dispatcher* dispatcher);
	virtual void getAabb(BroadphaseProxy* proxy, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void rayTest(const Vec3& rayFrom, const Vec3& rayTo, BroadphaseRayCallback& rayCallback, const Vec3& aabbMin = Vec3(0, 0, 0), const Vec3& aabbMax = Vec3(0, 0, 0));
	virtual void aabbTest(const Vec3& aabbMin, const Vec3& aabbMax, BroadphaseAabbCallback& callback);

	OverlappingPairCache* getOverlappingPairCache()
	{
		return m_pairCache;
	}
	const OverlappingPairCache* getOverlappingPairCache() const
	{
		return m_pairCache;
	}

	bool testAabbOverlap(BroadphaseProxy* proxy0, BroadphaseProxy* proxy1);

	///getAabb returns the axis aligned bounding box in the 'global' coordinate frame
	///will add some transform later
	virtual void getBroadphaseAabb(Vec3& aabbMin, Vec3& aabbMax) const
	{
		aabbMin.setVal(-DRX3D_LARGE_FLOAT, -DRX3D_LARGE_FLOAT, -DRX3D_LARGE_FLOAT);
		aabbMax.setVal(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
	}

	virtual void printStats()
	{
		//		printf("SimpleBroadphase.h\n");
		//		printf("numHandles = %d, maxHandles = %d\n",m_numHandles,m_maxHandles);
	}
};

#endif  //DRX3D_SIMPLE_BROADPHASE_H
