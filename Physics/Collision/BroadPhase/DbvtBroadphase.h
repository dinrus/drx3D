#ifndef DRX3D_DBVT_BROADPHASE_H
#define DRX3D_DBVT_BROADPHASE_H

#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Physics/Collision/BroadPhase/OverlappingPairCache.h>

//
// Compile time config
//

#define DBVT_BP_PROFILE 0
//#define DBVT_BP_SORTPAIRS				1
#define DBVT_BP_PREVENTFALSEUPDATE 0
#define DBVT_BP_ACCURATESLEEPING 0
#define DBVT_BP_ENABLE_BENCHMARK 0
//#define DBVT_BP_MARGIN					(Scalar)0.05
extern Scalar gDbvtMargin;

#if DBVT_BP_PROFILE
#define DBVT_BP_PROFILING_RATE 256
#include <drx3D/Maths/Linear/Quickprof.h>
#endif

//
// DbvtProxy
//
struct DbvtProxy : BroadphaseProxy
{
	/* Fields		*/
	//DbvtAabbMm	aabb;
	DbvtNode* leaf;
	DbvtProxy* links[2];
	i32 stage;
	/* ctor			*/
	DbvtProxy(const Vec3& aabbMin, const Vec3& aabbMax, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask) : BroadphaseProxy(aabbMin, aabbMax, userPtr, collisionFilterGroup, collisionFilterMask)
	{
		links[0] = links[1] = 0;
	}
};

typedef AlignedObjectArray<DbvtProxy*> DbvtProxyArray;

///The DbvtBroadphase implements a broadphase using two dynamic AABB bounding volume hierarchies/trees (see Dbvt).
///One tree is used for static/non-moving objects, and another tree is used for dynamic objects. Objects can move from one tree to the other.
///This is a very fast broadphase, especially for very dynamic worlds where many objects are moving. Its insert/add and remove of objects is generally faster than the sweep and prune broadphases AxisSweep3 and drx3D32BitAxisSweep3.
struct DbvtBroadphase : BroadphaseInterface
{
	/* Config		*/
	enum
	{
		DYNAMIC_SET = 0, /* Dynamic set index	*/
		FIXED_SET = 1,   /* Fixed set index		*/
		STAGECOUNT = 2   /* Number of stages		*/
	};
	/* Fields		*/
	Dbvt m_sets[2];                           // Dbvt sets
	DbvtProxy* m_stageRoots[STAGECOUNT + 1];  // Stages list
	OverlappingPairCache* m_paircache;        // Pair cache
	Scalar m_prediction;                      // Velocity prediction
	i32 m_stageCurrent;                         // Current stage
	i32 m_fupdates;                             // % of fixed updates per frame
	i32 m_dupdates;                             // % of dynamic updates per frame
	i32 m_cupdates;                             // % of cleanup updates per frame
	i32 m_newpairs;                             // Number of pairs created
	i32 m_fixedleft;                            // Fixed optimization left
	unsigned m_updates_call;                    // Number of updates call
	unsigned m_updates_done;                    // Number of updates done
	Scalar m_updates_ratio;                   // m_updates_done/m_updates_call
	i32 m_pid;                                  // Parse id
	i32 m_cid;                                  // Cleanup index
	i32 m_gid;                                  // Gen id
	bool m_releasepaircache;                    // Release pair cache on delete
	bool m_deferedcollide;                      // Defere dynamic/static collision to collide call
	bool m_needcleanup;                         // Need to run cleanup?
	AlignedObjectArray<AlignedObjectArray<const DbvtNode*> > m_rayTestStacks;
#if DBVT_BP_PROFILE
	Clock m_clock;
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
	DbvtBroadphase(OverlappingPairCache* paircache = 0);
	~DbvtBroadphase();
	void collide(Dispatcher* dispatcher);
	void optimize();

	/* BroadphaseInterface Implementation	*/
	BroadphaseProxy* createProxy(const Vec3& aabbMin, const Vec3& aabbMax, i32 shapeType, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask, Dispatcher* dispatcher);
	virtual void destroyProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher);
	virtual void setAabb(BroadphaseProxy* proxy, const Vec3& aabbMin, const Vec3& aabbMax, Dispatcher* dispatcher);
	virtual void rayTest(const Vec3& rayFrom, const Vec3& rayTo, BroadphaseRayCallback& rayCallback, const Vec3& aabbMin = Vec3(0, 0, 0), const Vec3& aabbMax = Vec3(0, 0, 0));
	virtual void aabbTest(const Vec3& aabbMin, const Vec3& aabbMax, BroadphaseAabbCallback& callback);

	virtual void getAabb(BroadphaseProxy* proxy, Vec3& aabbMin, Vec3& aabbMax) const;
	virtual void calculateOverlappingPairs(Dispatcher* dispatcher);
	virtual OverlappingPairCache* getOverlappingPairCache();
	virtual const OverlappingPairCache* getOverlappingPairCache() const;
	virtual void getBroadphaseAabb(Vec3& aabbMin, Vec3& aabbMax) const;
	virtual void printStats();

	///reset broadphase internal structures, to ensure determinism/reproducability
	virtual void resetPool(Dispatcher* dispatcher);

	void performDeferredRemoval(Dispatcher* dispatcher);

	void setVelocityPrediction(Scalar prediction)
	{
		m_prediction = prediction;
	}
	Scalar getVelocityPrediction() const
	{
		return m_prediction;
	}

	///this setAabbForceUpdate is similar to setAabb but always forces the aabb update.
	///it is not part of the BroadphaseInterface but specific to DbvtBroadphase.
	///it bypasses certain optimizations that prevent aabb updates (when the aabb shrinks), see
	///http://code.google.com/p/bullet/issues/detail?id=223
	void setAabbForceUpdate(BroadphaseProxy* absproxy, const Vec3& aabbMin, const Vec3& aabbMax, Dispatcher* /*dispatcher*/);

	static void benchmark(BroadphaseInterface*);
};

#endif
