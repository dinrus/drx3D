#ifndef DRX3D_POINT_COLLECTOR_H
#define DRX3D_POINT_COLLECTOR_H

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>

struct PointCollector : public DiscreteCollisionDetectorInterface::Result
{
	Vec3 m_normalOnBInWorld;
	Vec3 m_pointInWorld;
	Scalar m_distance;  //negative means penetration

	bool m_hasResult;

	PointCollector()
		: m_distance(Scalar(DRX3D_LARGE_FLOAT)), m_hasResult(false)
	{
	}

	virtual void setShapeIdentifiersA(i32 partId0, i32 index0)
	{
		(void)partId0;
		(void)index0;
	}
	virtual void setShapeIdentifiersB(i32 partId1, i32 index1)
	{
		(void)partId1;
		(void)index1;
	}

	virtual void addContactPoint(const Vec3& normalOnBInWorld, const Vec3& pointInWorld, Scalar depth)
	{
		if (depth < m_distance)
		{
			m_hasResult = true;
			m_normalOnBInWorld = normalOnBInWorld;
			m_pointInWorld = pointInWorld;
			//negative means penetration
			m_distance = depth;
		}
	}
};

#endif  //DRX3D_POINT_COLLECTOR_H
