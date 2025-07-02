
#ifndef DRX3D_GIMPACT_BVH_CONCAVE_COLLISION_ALGORITHM_H
#define DRX3D_GIMPACT_BVH_CONCAVE_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class CollisionDispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <drx3D/Physics/Collision/Gimpact/GImpactShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Dispatch/ConvexConvexAlgorithm.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

//! Collision Algorithm for GImpact Shapes
/*!
For DoRegister this algorithm in drx3D, proceed as following:
 \code
Dispatcher * dispatcher = static_cast<Dispatcher *>(m_dynamicsWorld ->getDispatcher());
GImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
 \endcode
*/
class GImpactCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
protected:
	CollisionAlgorithm* m_convex_algorithm;
	PersistentManifold* m_manifoldPtr;
	ManifoldResult* m_resultOut;
	const DispatcherInfo* m_dispatchInfo;
	i32 m_triface0;
	i32 m_part0;
	i32 m_triface1;
	i32 m_part1;

	//! Creates a new contact point
	SIMD_FORCE_INLINE PersistentManifold* newContactManifold(const CollisionObject2* body0, const CollisionObject2* body1)
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(body0, body1);
		return m_manifoldPtr;
	}

	SIMD_FORCE_INLINE void destroyConvexAlgorithm()
	{
		if (m_convex_algorithm)
		{
			m_convex_algorithm->~CollisionAlgorithm();
			m_dispatcher->freeCollisionAlgorithm(m_convex_algorithm);
			m_convex_algorithm = NULL;
		}
	}

	SIMD_FORCE_INLINE void destroyContactManifolds()
	{
		if (m_manifoldPtr == NULL) return;
		m_dispatcher->releaseManifold(m_manifoldPtr);
		m_manifoldPtr = NULL;
	}

	SIMD_FORCE_INLINE void clearCache()
	{
		destroyContactManifolds();
		destroyConvexAlgorithm();

		m_triface0 = -1;
		m_part0 = -1;
		m_triface1 = -1;
		m_part1 = -1;
	}

	SIMD_FORCE_INLINE PersistentManifold* getLastManifold()
	{
		return m_manifoldPtr;
	}

	// Call before process collision
	SIMD_FORCE_INLINE void checkManifold(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	{
		if (getLastManifold() == 0)
		{
			newContactManifold(body0Wrap->getCollisionObject(), body1Wrap->getCollisionObject());
		}

		m_resultOut->setPersistentManifold(getLastManifold());
	}

	// Call before process collision
	SIMD_FORCE_INLINE CollisionAlgorithm* newAlgorithm(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	{
		checkManifold(body0Wrap, body1Wrap);

		CollisionAlgorithm* convex_algorithm = m_dispatcher->findAlgorithm(
			body0Wrap, body1Wrap, getLastManifold(), DRX3D_CONTACT_POINT_ALGORITHMS);
		return convex_algorithm;
	}

	// Call before process collision
	SIMD_FORCE_INLINE void checkConvexAlgorithm(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
	{
		if (m_convex_algorithm) return;
		m_convex_algorithm = newAlgorithm(body0Wrap, body1Wrap);
	}

	void addContactPoint(const CollisionObject2Wrapper* body0Wrap,
						 const CollisionObject2Wrapper* body1Wrap,
						 const Vec3& point,
						 const Vec3& normal,
						 Scalar distance);

	//! Collision routines
	//!@{

	void collide_gjk_triangles(const CollisionObject2Wrapper* body0Wrap,
							   const CollisionObject2Wrapper* body1Wrap,
							   const GImpactMeshShapePart* shape0,
							   const GImpactMeshShapePart* shape1,
							   i32k* pairs, i32 pair_count);

	void collide_sat_triangles(const CollisionObject2Wrapper* body0Wrap,
							   const CollisionObject2Wrapper* body1Wrap,
							   const GImpactMeshShapePart* shape0,
							   const GImpactMeshShapePart* shape1,
							   i32k* pairs, i32 pair_count);

	void shape_vs_shape_collision(
		const CollisionObject2Wrapper* body0,
		const CollisionObject2Wrapper* body1,
		const CollisionShape* shape0,
		const CollisionShape* shape1);

	void convex_vs_convex_collision(const CollisionObject2Wrapper* body0Wrap,
									const CollisionObject2Wrapper* body1Wrap,
									const CollisionShape* shape0,
									const CollisionShape* shape1);

	void gimpact_vs_gimpact_find_pairs(
		const Transform2& trans0,
		const Transform2& trans1,
		const GImpactShapeInterface* shape0,
		const GImpactShapeInterface* shape1, PairSet& pairset);

	void gimpact_vs_shape_find_pairs(
		const Transform2& trans0,
		const Transform2& trans1,
		const GImpactShapeInterface* shape0,
		const CollisionShape* shape1,
		AlignedObjectArray<i32>& collided_primitives);

	void gimpacttrimeshpart_vs_plane_collision(
		const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactMeshShapePart* shape0,
		const StaticPlaneShape* shape1, bool swapped);

public:
	GImpactCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

	virtual ~GImpactCollisionAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		if (m_manifoldPtr)
			manifoldArray.push_back(m_manifoldPtr);
	}

	ManifoldResult* internalGetResultOut()
	{
		return m_resultOut;
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(GImpactCollisionAlgorithm));
			return new (mem) GImpactCollisionAlgorithm(ci, body0Wrap, body1Wrap);
		}
	};

	//! Use this function for DoRegister the algorithm externally
	static void registerAlgorithm(CollisionDispatcher* dispatcher);
#ifdef TRI_COLLISION_PROFILING
	//! Gets the average time in miliseconds of tree collisions
	static float getAverageTreeCollisionTime();

	//! Gets the average time in miliseconds of triangle collisions
	static float getAverageTriangleCollisionTime();
#endif  //TRI_COLLISION_PROFILING

	//! Collides two gimpact shapes
	/*!
	\pre shape0 and shape1 couldn't be GImpactMeshShape objects
	*/

	void gimpact_vs_gimpact(const CollisionObject2Wrapper* body0Wrap,
							const CollisionObject2Wrapper* body1Wrap,
							const GImpactShapeInterface* shape0,
							const GImpactShapeInterface* shape1);

	void gimpact_vs_shape(const CollisionObject2Wrapper* body0Wrap,
						  const CollisionObject2Wrapper* body1Wrap,
						  const GImpactShapeInterface* shape0,
						  const CollisionShape* shape1, bool swapped);

	void gimpact_vs_compoundshape(const CollisionObject2Wrapper* body0Wrap,
								  const CollisionObject2Wrapper* body1Wrap,
								  const GImpactShapeInterface* shape0,
								  const CompoundShape* shape1, bool swapped);

	void gimpact_vs_concave(
		const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactShapeInterface* shape0,
		const ConcaveShape* shape1, bool swapped);

	/// Accessor/Mutator pairs for Part and triangleID
	void setFace0(i32 value)
	{
		m_triface0 = value;
	}
	i32 getFace0()
	{
		return m_triface0;
	}
	void setFace1(i32 value)
	{
		m_triface1 = value;
	}
	i32 getFace1()
	{
		return m_triface1;
	}
	void setPart0(i32 value)
	{
		m_part0 = value;
	}
	i32 getPart0()
	{
		return m_part0;
	}
	void setPart1(i32 value)
	{
		m_part1 = value;
	}
	i32 getPart1()
	{
		return m_part1;
	}
};

//algorithm details
//#define DRX3D_TRIANGLE_COLLISION 1
#define GIMPACT_VS_PLANE_COLLISION 1

#endif  //DRX3D_GIMPACT_BVH_CONCAVE_COLLISION_ALGORITHM_H
