#ifndef DRX3D_RAYCAST_TRI_CALLBACK_H
#define DRX3D_RAYCAST_TRI_CALLBACK_H

#include <drx3D/Physics/Collision/Shapes/TriangleCallback.h>
#include <drx3D/Maths/Linear/Transform2.h>
struct BroadphaseProxy;
class ConvexShape;

class TriangleRaycastCallback : public TriangleCallback
{
public:
	//input
	Vec3 m_from;
	Vec3 m_to;

	//@BP Mod - allow backface filtering and unflipped normals
	enum EFlags
	{
		kF_None = 0,
		kF_FilterBackfaces = 1 << 0,
		kF_KeepUnflippedNormal = 1 << 1,             // Prevents returned face normal getting flipped when a ray hits a back-facing triangle
													 ///SubSimplexConvexCastRaytest is the default, even if kF_None is set.
		kF_UseSubSimplexConvexCastRaytest = 1 << 2,  // Uses an approximate but faster ray versus convex intersection algorithm
		kF_UseGjkConvexCastRaytest = 1 << 3,
		kF_DisableHeightfieldAccelerator  = 1 << 4, //don't use the heightfield raycast accelerator. See https://github.com/bulletphysics/bullet3/pull/2062
		kF_Terminator = 0xFFFFFFFF
	};
	u32 m_flags;

	Scalar m_hitFraction;

	TriangleRaycastCallback(const Vec3& from, const Vec3& to, u32 flags = 0);

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex);

	virtual Scalar reportHit(const Vec3& hitNormalLocal, Scalar hitFraction, i32 partId, i32 triangleIndex) = 0;
};

class TriangleConvexcastCallback : public TriangleCallback
{
public:
	const ConvexShape* m_convexShape;
	Transform2 m_convexShapeFrom;
	Transform2 m_convexShapeTo;
	Transform2 m_triangleToWorld;
	Scalar m_hitFraction;
	Scalar m_triangleCollisionMargin;
	Scalar m_allowedPenetration;

	TriangleConvexcastCallback(const ConvexShape* convexShape, const Transform2& convexShapeFrom, const Transform2& convexShapeTo, const Transform2& triangleToWorld, const Scalar triangleCollisionMargin);

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex);

	virtual Scalar reportHit(const Vec3& hitNormalLocal, const Vec3& hitPointLocal, Scalar hitFraction, i32 partId, i32 triangleIndex) = 0;
};

#endif  //DRX3D_RAYCAST_TRI_CALLBACK_H
