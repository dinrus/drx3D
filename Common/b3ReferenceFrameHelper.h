#ifndef D3_REFERENCEFRAMEHELPER_H
#define D3_REFERENCEFRAMEHELPER_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>

class b3ReferenceFrameHelper
{
public:
	static Vec3 getPointWorldToLocal(const Transform2& localObjectCenterOfMassTransform, const Vec3& point)
	{
		return localObjectCenterOfMassTransform.inverse() * point;  // transforms the point from the world frame into the local frame
	}

	static Vec3 getPointLocalToWorld(const Transform2& localObjectCenterOfMassTransform, const Vec3& point)
	{
		return localObjectCenterOfMassTransform * point;  // transforms the point from the world frame into the local frame
	}

	static Vec3 getAxisWorldToLocal(const Transform2& localObjectCenterOfMassTransform, const Vec3& axis)
	{
		Transform2 local1 = localObjectCenterOfMassTransform.inverse();  // transforms the axis from the local frame into the world frame
		Vec3 zero(0, 0, 0);
		local1.setOrigin(zero);
		return local1 * axis;
	}

	static Vec3 getAxisLocalToWorld(const Transform2& localObjectCenterOfMassTransform, const Vec3& axis)
	{
		Transform2 local1 = localObjectCenterOfMassTransform;  // transforms the axis from the local frame into the world frame
		Vec3 zero(0, 0, 0);
		local1.setOrigin(zero);
		return local1 * axis;
	}

	static Transform2 getTransformWorldToLocal(const Transform2& localObjectCenterOfMassTransform, const Transform2& transform)
	{
		return localObjectCenterOfMassTransform.inverse() * transform;  // transforms the axis from the local frame into the world frame
	}

	static Transform2 getTransformLocalToWorld(const Transform2& localObjectCenterOfMassTransform, const Transform2& transform)
	{
		return localObjectCenterOfMassTransform * transform;  // transforms the axis from the local frame into the world frame
	}
};

#endif /* D3_REFERENCEFRAMEHELPER_H */
