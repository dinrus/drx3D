#ifndef DRX3D_DISCRETE_COLLISION_DETECTOR1_INTERFACE_H
#define DRX3D_DISCRETE_COLLISION_DETECTOR1_INTERFACE_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>

/// This interface is made to be used by an iterative approach to do TimeOfImpact calculations
/// This interface allows to query for closest points and penetration depth between two (convex) objects
/// the closest point is on the second object (B), and the normal points from the surface on B towards A.
/// distance is between closest points on B and closest point on A. So you can calculate closest point on A
/// by taking closestPointInA = closestPointInB + m_distance * m_normalOnSurfaceB
struct DiscreteCollisionDetectorInterface
{
	struct Result
	{
		virtual ~Result() {}

		///setShapeIdentifiersA/B provides experimental support for per-triangle material / custom material combiner
		virtual void setShapeIdentifiersA(i32 partId0, i32 index0) = 0;
		virtual void setShapeIdentifiersB(i32 partId1, i32 index1) = 0;
		virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth) = 0;
	};

	struct ClosestPointInput
	{
		ClosestPointInput()
			: m_maximumDistanceSquared(Scalar(DRX3D_LARGE_FLOAT))
		{
		}

		Transform2 m_transformA;
		Transform2 m_transformB;
		Scalar m_maximumDistanceSquared;
	};

	virtual ~DiscreteCollisionDetectorInterface(){};

	//
	// give either closest points (distance > 0) or penetration (distance)
	// the normal always points from B towards A
	//
	virtual void getClosestPoints(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw, bool swapResults = false) = 0;
};

struct StorageResult : public DiscreteCollisionDetectorInterface::Result
{
	Vec3 m_normalOnSurfaceB;
	Vec3 m_closestPointInB;
	Scalar m_distance;  //negative means penetration !

protected:
	StorageResult() : m_distance(Scalar(DRX3D_LARGE_FLOAT))
	{
	}

public:
	virtual ~StorageResult(){};

	virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
	{
		if (depth < m_distance)
		{
			m_normalOnSurfaceB = normalOnBInWorld;
			m_closestPointInB = pointInWorld;
			m_distance = depth;
		}
	}
};

#endif  //DRX3D_DISCRETE_COLLISION_DETECTOR1_INTERFACE_H
