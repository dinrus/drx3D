#ifndef DRX3D_BROADPHASE_PROXY_H
#define DRX3D_BROADPHASE_PROXY_H

#include <drx3D/Maths/Linear/Scalar.h>  //for SIMD_FORCE_INLINE
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>

/// Dispatcher uses these types
/// IMPORTANT NOTE:The types are ordered polyhedral, implicit convex and concave
/// to facilitate type checking
/// CUSTOM_POLYHEDRAL_SHAPE_TYPE,CUSTOM_CONVEX_SHAPE_TYPE and CUSTOM_CONCAVE_SHAPE_TYPE can be used to extend drx3D without modifying source code
enum BroadphaseNativeTypes
{
	// polyhedral convex shapes
	BOX_SHAPE_PROXYTYPE,
	TRIANGLE_SHAPE_PROXYTYPE,
	TETRAHEDRAL_SHAPE_PROXYTYPE,
	CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE,
	CONVEX_HULL_SHAPE_PROXYTYPE,
	CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE,
	CUSTOM_POLYHEDRAL_SHAPE_TYPE,
	//implicit convex shapes
	IMPLICIT_CONVEX_SHAPES_START_HERE,
	SPHERE_SHAPE_PROXYTYPE,
	MULTI_SPHERE_SHAPE_PROXYTYPE,
	CAPSULE_SHAPE_PROXYTYPE,
	CONE_SHAPE_PROXYTYPE,
	CONVEX_SHAPE_PROXYTYPE,
	CYLINDER_SHAPE_PROXYTYPE,
	UNIFORM_SCALING_SHAPE_PROXYTYPE,
	MINKOWSKI_SUM_SHAPE_PROXYTYPE,
	MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE,
	BOX_2D_SHAPE_PROXYTYPE,
	CONVEX_2D_SHAPE_PROXYTYPE,
	CUSTOM_CONVEX_SHAPE_TYPE,
	//concave shapes
	CONCAVE_SHAPES_START_HERE,
	//keep all the convex shapetype below here, for the check IsConvexShape in broadphase proxy!
	TRIANGLE_MESH_SHAPE_PROXYTYPE,
	SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE,
	///used for demo integration FAST/Swift collision library and drx3D
	FAST_CONCAVE_MESH_PROXYTYPE,
	//terrain
	TERRAIN_SHAPE_PROXYTYPE,
	///Used for GIMPACT Trimesh integration
	GIMPACT_SHAPE_PROXYTYPE,
	///Multimaterial mesh
	MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE,

	EMPTY_SHAPE_PROXYTYPE,
	STATIC_PLANE_PROXYTYPE,
	CUSTOM_CONCAVE_SHAPE_TYPE,
	SDF_SHAPE_PROXYTYPE = CUSTOM_CONCAVE_SHAPE_TYPE,
	CONCAVE_SHAPES_END_HERE,

	COMPOUND_SHAPE_PROXYTYPE,

	SOFTBODY_SHAPE_PROXYTYPE,
	HFFLUID_SHAPE_PROXYTYPE,
	HFFLUID_BUOYANT_CONVEX_SHAPE_PROXYTYPE,
	INVALID_SHAPE_PROXYTYPE,

	MAX_BROADPHASE_COLLISION_TYPES

};

///The BroadphaseProxy is the main class that can be used with the drx3D broadphases.
///It stores collision shape type information, collision filter information and a client object, typically a CollisionObject2 or RigidBody.
ATTRIBUTE_ALIGNED16(struct)
BroadphaseProxy
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

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

	//Usually the client CollisionObject2 or Rigidbody class
	uk m_clientObject;
	i32 m_collisionFilterGroup;
	i32 m_collisionFilterMask;

	i32 m_uniqueId;  //m_uniqueId is introduced for paircache. could get rid of this, by calculating the address offset etc.

	Vec3 m_aabbMin;
	Vec3 m_aabbMax;

	SIMD_FORCE_INLINE i32 getUid() const
	{
		return m_uniqueId;
	}

	//used for memory pools
	BroadphaseProxy() : m_clientObject(0)
	{
	}

	BroadphaseProxy(const Vec3& aabbMin, const Vec3& aabbMax, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
		: m_clientObject(userPtr),
		  m_collisionFilterGroup(collisionFilterGroup),
		  m_collisionFilterMask(collisionFilterMask),
		  m_aabbMin(aabbMin),
		  m_aabbMax(aabbMax)
	{
	}

	static SIMD_FORCE_INLINE bool isPolyhedral(i32 proxyType)
	{
		return (proxyType < IMPLICIT_CONVEX_SHAPES_START_HERE);
	}

	static SIMD_FORCE_INLINE bool isConvex(i32 proxyType)
	{
		return (proxyType < CONCAVE_SHAPES_START_HERE);
	}

	static SIMD_FORCE_INLINE bool isNonMoving(i32 proxyType)
	{
		return (isConcave(proxyType) && !(proxyType == GIMPACT_SHAPE_PROXYTYPE));
	}

	static SIMD_FORCE_INLINE bool isConcave(i32 proxyType)
	{
		return ((proxyType > CONCAVE_SHAPES_START_HERE) &&
				(proxyType < CONCAVE_SHAPES_END_HERE));
	}
	static SIMD_FORCE_INLINE bool isCompound(i32 proxyType)
	{
		return (proxyType == COMPOUND_SHAPE_PROXYTYPE);
	}

	static SIMD_FORCE_INLINE bool isSoftBody(i32 proxyType)
	{
		return (proxyType == SOFTBODY_SHAPE_PROXYTYPE);
	}

	static SIMD_FORCE_INLINE bool isInfinite(i32 proxyType)
	{
		return (proxyType == STATIC_PLANE_PROXYTYPE);
	}

	static SIMD_FORCE_INLINE bool isConvex2d(i32 proxyType)
	{
		return (proxyType == BOX_2D_SHAPE_PROXYTYPE) || (proxyType == CONVEX_2D_SHAPE_PROXYTYPE);
	}
};

class CollisionAlgorithm;

struct BroadphaseProxy;

///The BroadphasePair class contains a pair of aabb-overlapping objects.
///A Dispatcher can search a CollisionAlgorithm that performs exact/narrowphase collision detection on the actual collision shapes.
ATTRIBUTE_ALIGNED16(struct)
BroadphasePair
{
	BroadphasePair()
		: m_pProxy0(0),
		  m_pProxy1(0),
		  m_algorithm(0),
		  m_internalInfo1(0)
	{
	}

	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	BroadphasePair(BroadphaseProxy & proxy0, BroadphaseProxy & proxy1)
	{
		//keep them sorted, so the std::set operations work
		if (proxy0.m_uniqueId < proxy1.m_uniqueId)
		{
			m_pProxy0 = &proxy0;
			m_pProxy1 = &proxy1;
		}
		else
		{
			m_pProxy0 = &proxy1;
			m_pProxy1 = &proxy0;
		}

		m_algorithm = 0;
		m_internalInfo1 = 0;
	}

	BroadphaseProxy* m_pProxy0;
	BroadphaseProxy* m_pProxy1;

	mutable CollisionAlgorithm* m_algorithm;
	union {
		uk m_internalInfo1;
		i32 m_internalTmpValue;
	};  //don't use this data, it will be removed in future version.
};

/*
//comparison for set operation, see Solid DT_Encounter
SIMD_FORCE_INLINE bool operator<(const BroadphasePair& a, const BroadphasePair& b) 
{ 
    return a.m_pProxy0 < b.m_pProxy0 || 
        (a.m_pProxy0 == b.m_pProxy0 && a.m_pProxy1 < b.m_pProxy1); 
}
*/

class BroadphasePairSortPredicate
{
public:
	bool operator()(const BroadphasePair& a, const BroadphasePair& b) const
	{
		i32k uidA0 = a.m_pProxy0 ? a.m_pProxy0->m_uniqueId : -1;
		i32k uidB0 = b.m_pProxy0 ? b.m_pProxy0->m_uniqueId : -1;
		i32k uidA1 = a.m_pProxy1 ? a.m_pProxy1->m_uniqueId : -1;
		i32k uidB1 = b.m_pProxy1 ? b.m_pProxy1->m_uniqueId : -1;

		return uidA0 > uidB0 ||
			   (a.m_pProxy0 == b.m_pProxy0 && uidA1 > uidB1) ||
			   (a.m_pProxy0 == b.m_pProxy0 && a.m_pProxy1 == b.m_pProxy1 && a.m_algorithm > b.m_algorithm);
	}
};

SIMD_FORCE_INLINE bool operator==(const BroadphasePair& a, const BroadphasePair& b)
{
	return (a.m_pProxy0 == b.m_pProxy0) && (a.m_pProxy1 == b.m_pProxy1);
}

#endif  //DRX3D_BROADPHASE_PROXY_H
