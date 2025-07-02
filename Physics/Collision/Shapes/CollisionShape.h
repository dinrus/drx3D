#ifndef DRX3D_COLLISION_SHAPE_H
#define DRX3D_COLLISION_SHAPE_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  //for the shape types
class Serializer;

///The CollisionShape class provides an interface for collision shapes that can be shared among CollisionObjects.
ATTRIBUTE_ALIGNED16(class)
CollisionShape
{
protected:
	i32 m_shapeType;
	uk m_userPointer;
	i32 m_userIndex;
	i32 m_userIndex2;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	CollisionShape() : m_shapeType(INVALID_SHAPE_PROXYTYPE), m_userPointer(0), m_userIndex(-1), m_userIndex2(-1)
	{
	}

	virtual ~CollisionShape()
	{
	}

	///getAabb returns the axis aligned bounding box in the coordinate frame of the given transform t.
	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const = 0;

	virtual void getBoundingSphere(Vec3 & center, Scalar & radius) const;

	///getAngularMotionDisc returns the maximum radius needed for Conservative Advancement to handle time-of-impact with rotations.
	virtual Scalar getAngularMotionDisc() const;

	virtual Scalar getContactBreakingThreshold(Scalar defaultContactThresholdFactor) const;

	///calculateTemporalAabb calculates the enclosing aabb for the moving object over interval [0..timeStep)
	///result is conservative
	void calculateTemporalAabb(const Transform2& curTrans, const Vec3& linvel, const Vec3& angvel, Scalar timeStep, Vec3& temporalAabbMin, Vec3& temporalAabbMax) const;

	SIMD_FORCE_INLINE bool isPolyhedral() const
	{
		return BroadphaseProxy::isPolyhedral(getShapeType());
	}

	SIMD_FORCE_INLINE bool isConvex2d() const
	{
		return BroadphaseProxy::isConvex2d(getShapeType());
	}

	SIMD_FORCE_INLINE bool isConvex() const
	{
		return BroadphaseProxy::isConvex(getShapeType());
	}
	SIMD_FORCE_INLINE bool isNonMoving() const
	{
		return BroadphaseProxy::isNonMoving(getShapeType());
	}
	SIMD_FORCE_INLINE bool isConcave() const
	{
		return BroadphaseProxy::isConcave(getShapeType());
	}
	SIMD_FORCE_INLINE bool isCompound() const
	{
		return BroadphaseProxy::isCompound(getShapeType());
	}

	SIMD_FORCE_INLINE bool isSoftBody() const
	{
		return BroadphaseProxy::isSoftBody(getShapeType());
	}

	///isInfinite is used to catch simulation error (aabb check)
	SIMD_FORCE_INLINE bool isInfinite() const
	{
		return BroadphaseProxy::isInfinite(getShapeType());
	}

#ifndef __SPU__
	virtual void setLocalScaling(const Vec3& scaling) = 0;
	virtual const Vec3& getLocalScaling() const = 0;
	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const = 0;

	//debugging support
	virtual tukk getName() const = 0;
#endif  //__SPU__

	i32 getShapeType() const
	{
		return m_shapeType;
	}

	///the getAnisotropicRollingFrictionDirection can be used in combination with setAnisotropicFriction
	///See drx3D/Demos/RollingFrictionDemo for an example
	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		return Vec3(1, 1, 1);
	}
	virtual void setMargin(Scalar margin) = 0;
	virtual Scalar getMargin() const = 0;

	///optional user data pointer
	void setUserPointer(uk userPtr)
	{
		m_userPointer = userPtr;
	}

	uk getUserPointer() const
	{
		return m_userPointer;
	}
	void setUserIndex(i32 index)
	{
		m_userIndex = index;
	}

	i32 getUserIndex() const
	{
		return m_userIndex;
	}

	void setUserIndex2(i32 index)
	{
		m_userIndex2 = index;
	}

	i32 getUserIndex2() const
	{
		return m_userIndex2;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	virtual void serializeSingleShape(Serializer * serializer) const;
};

// clang-format off
// parser needs * with the name
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	CollisionShapeData
{
	char	*m_name;
	i32		m_shapeType;
	char	m_padding[4];
};
// clang-format on
SIMD_FORCE_INLINE i32 CollisionShape::calculateSerializeBufferSize() const
{
	return sizeof(CollisionShapeData);
}

#endif  //DRX3D_COLLISION_SHAPE_H
