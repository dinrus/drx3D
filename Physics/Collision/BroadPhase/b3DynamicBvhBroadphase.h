
#ifndef D3_DBVT_BROADPHASE_H
#define D3_DBVT_BROADPHASE_H

#include <drx3D/Physics/Collision/BroadPhase/b3DynamicBvh.h>
#include <drx3D/Physics/Collision/BroadPhase/b3OverlappingPairCache.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Collision/BroadPhase/b3BroadphaseCallback.h>

//
// Compile time config
//

#define D3_DBVT_BP_PROFILE 0
//#define D3_DBVT_BP_SORTPAIRS				1
#define D3_DBVT_BP_PREVENTFALSEUPDATE 0
#define D3_DBVT_BP_ACCURATESLEEPING 0
#define D3_DBVT_BP_ENABLE_BENCHMARK 0
#define D3_DBVT_BP_MARGIN (b3Scalar)0.05

#if D3_DBVT_BP_PROFILE
#define D3_DBVT_BP_PROFILING_RATE 256

#endif

D3_ATTRIBUTE_ALIGNED16(struct)
b3BroadphaseProxy
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	///optional filtering to cull potential collisions
	enum CollisionFilterGroups
	{
		DefaultFilter = 1,
		StaticFilter = 2,
		KinematicFilter = 4,
		DebrisFilter = 8,
		SensorTrigger = 16,
		CharacterFilter = 32,
		AllFilter = -1  //all bits sets: DefaultFilter | StaticFilter | KinematicFilter | DebrisFilter | SensorTrigger
	};

	//Usually the client b3CollisionObject2 or Rigidbody class
	uk m_clientObject;
	i32 m_collisionFilterGroup;
	i32 m_collisionFilterMask;
	i32 m_uniqueId;  //m_uniqueId is introduced for paircache. could get rid of this, by calculating the address offset etc.

	b3Vec3 m_aabbMin;
	b3Vec3 m_aabbMax;

	D3_FORCE_INLINE i32 getUid() const
	{
		return m_uniqueId;
	}

	//used for memory pools
	b3BroadphaseProxy() : m_clientObject(0)
	{
	}

	b3BroadphaseProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
		: m_clientObject(userPtr),
		  m_collisionFilterGroup(collisionFilterGroup),
		  m_collisionFilterMask(collisionFilterMask),
		  m_aabbMin(aabbMin),
		  m_aabbMax(aabbMax)
	{
	}
};

//
// b3DbvtProxy
//
struct b3DbvtProxy : b3BroadphaseProxy
{
	/* Fields		*/
	//b3DbvtAabbMm	aabb;
	b3DbvtNode* leaf;
	b3DbvtProxy* links[2];
	i32 stage;
	/* ctor			*/

	explicit b3DbvtProxy() {}
	b3DbvtProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask) : b3BroadphaseProxy(aabbMin, aabbMax, userPtr, collisionFilterGroup, collisionFilterMask)
	{
		links[0] = links[1] = 0;
	}
};

typedef b3AlignedObjectArray<b3DbvtProxy*> b3DbvtProxyArray;

///The b3DynamicBvhBroadphase implements a broadphase using two dynamic AABB bounding volume hierarchies/trees (see b3DynamicBvh).
///One tree is used for static/non-moving objects, and another tree is used for dynamic objects. Objects can move from one tree to the other.
///This is a very fast broadphase, especially for very dynamic worlds where many objects are moving. Its insert/add and remove of objects is generally faster than the sweep and prune broadphases b3AxisSweep3 and b332BitAxisSweep3.
struct b3DynamicBvhBroadphase
{
	/* Config		*/
	enum
	{
		DYNAMIC_SET = 0, /* Dynamic set index	*/
		FIXED_SET = 1,   /* Fixed set index		*/
		STAGECOUNT = 2   /* Number of stages		*/
	};
	/* Fields		*/
	b3DynamicBvh m_sets[2];                     // Dbvt sets
	b3DbvtProxy* m_stageRoots[STAGECOUNT + 1];  // Stages list

	b3AlignedObjectArray<b3DbvtProxy> m_proxies;
	b3OverlappingPairCache* m_paircache;  // Pair cache
	b3Scalar m_prediction;                // Velocity prediction
	i32 m_stageCurrent;                   // Current stage
	i32 m_fupdates;                       // % of fixed updates per frame
	i32 m_dupdates;                       // % of dynamic updates per frame
	i32 m_cupdates;                       // % of cleanup updates per frame
	i32 m_newpairs;                       // Number of pairs created
	i32 m_fixedleft;                      // Fixed optimization left
	unsigned m_updates_call;              // Number of updates call
	unsigned m_updates_done;              // Number of updates done
	b3Scalar m_updates_ratio;             // m_updates_done/m_updates_call
	i32 m_pid;                            // Parse id
	i32 m_cid;                            // Cleanup index
	bool m_releasepaircache;              // Release pair cache on delete
	bool m_deferedcollide;                // Defere dynamic/static collision to collide call
	bool m_needcleanup;                   // Need to run cleanup?
#if D3_DBVT_BP_PROFILE
	b3Clock m_clock;
	struct
	{
		u64 m_total;
		u64 m_ddcollide;
		u64 m_fdcollide;
		u64 m_cleanup;
		u64 m_jobcount;
	} m_profiling;
#endif
	/* Methods		*/
	b3DynamicBvhBroadphase(i32 proxyCapacity, b3OverlappingPairCache* paircache = 0);
	virtual ~b3DynamicBvhBroadphase();
	void collide(b3Dispatcher* dispatcher);
	void optimize();

	/* b3BroadphaseInterface Implementation	*/
	b3BroadphaseProxy* createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 objectIndex, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);
	virtual void destroyProxy(b3BroadphaseProxy* proxy, b3Dispatcher* dispatcher);
	virtual void setAabb(i32 objectId, const b3Vec3& aabbMin, const b3Vec3& aabbMax, b3Dispatcher* dispatcher);
	virtual void rayTest(const b3Vec3& rayFrom, const b3Vec3& rayTo, b3BroadphaseRayCallback& rayCallback, const b3Vec3& aabbMin = b3MakeVector3(0, 0, 0), const b3Vec3& aabbMax = b3MakeVector3(0, 0, 0));
	virtual void aabbTest(const b3Vec3& aabbMin, const b3Vec3& aabbMax, b3BroadphaseAabbCallback& callback);

	//virtual void					getAabb(b3BroadphaseProxy* proxy,b3Vec3& aabbMin, b3Vec3& aabbMax ) const;
	virtual void getAabb(i32 objectId, b3Vec3& aabbMin, b3Vec3& aabbMax) const;
	virtual void calculateOverlappingPairs(b3Dispatcher* dispatcher = 0);
	virtual b3OverlappingPairCache* getOverlappingPairCache();
	virtual const b3OverlappingPairCache* getOverlappingPairCache() const;
	virtual void getBroadphaseAabb(b3Vec3& aabbMin, b3Vec3& aabbMax) const;
	virtual void printStats();

	///reset broadphase internal structures, to ensure determinism/reproducability
	virtual void resetPool(b3Dispatcher* dispatcher);

	void performDeferredRemoval(b3Dispatcher* dispatcher);

	void setVelocityPrediction(b3Scalar prediction)
	{
		m_prediction = prediction;
	}
	b3Scalar getVelocityPrediction() const
	{
		return m_prediction;
	}

	///this setAabbForceUpdate is similar to setAabb but always forces the aabb update.
	///it is not part of the b3BroadphaseInterface but specific to b3DynamicBvhBroadphase.
	///it bypasses certain optimizations that prevent aabb updates (when the aabb shrinks), see
	///http://code.google.com/p/bullet/issues/detail?id=223
	void setAabbForceUpdate(b3BroadphaseProxy* absproxy, const b3Vec3& aabbMin, const b3Vec3& aabbMax, b3Dispatcher* /*dispatcher*/);

	//static void						benchmark(b3BroadphaseInterface*);
};

#endif
