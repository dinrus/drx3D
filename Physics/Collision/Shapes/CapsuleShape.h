#ifndef DRX3D_CAPSULE_SHAPE_H
#define DRX3D_CAPSULE_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/BroadPhase//BroadphaseProxy.h>  // for the types

///The CapsuleShape represents a capsule around the Y axis, there is also the CapsuleShapeX aligned around the X axis and CapsuleShapeZ around the Z axis.
///The total height is height+2*radius, so the height is just the height between the center of each 'sphere' of the capsule caps.
///The CapsuleShape is a convex hull of two spheres. The MultiSphereShape is a more general collision shape that takes the convex hull of multiple sphere, so it can also represent a capsule when just using two spheres.
ATTRIBUTE_ALIGNED16(class)
CapsuleShape : public ConvexInternalShape
{
protected:
	i32 m_upAxis;

protected:
	///only used for CapsuleShapeZ and CapsuleShapeX subclasses.
	CapsuleShape() : ConvexInternalShape() { m_shapeType = CAPSULE_SHAPE_PROXYTYPE; };

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	CapsuleShape(Scalar radius, Scalar height);

	///CollisionShape Interface
	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	/// ConvexShape Interface
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void setMargin(Scalar collisionMargin)
	{
		//don't override the margin for capsules, their entire radius == margin
		(void)collisionMargin;
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		Vec3 halfExtents(getRadius(), getRadius(), getRadius());
		halfExtents[m_upAxis] = getRadius() + getHalfHeight();
		Matrix3x3 abs_b = t.getBasis().absolute();
		Vec3 center = t.getOrigin();
		Vec3 extent = halfExtents.dot3(abs_b[0], abs_b[1], abs_b[2]);

		aabbMin = center - extent;
		aabbMax = center + extent;
	}

	virtual tukk getName() const
	{
		return "CapsuleShape";
	}

	i32 getUpAxis() const
	{
		return m_upAxis;
	}

	Scalar getRadius() const
	{
		i32 radiusAxis = (m_upAxis + 2) % 3;
		return m_implicitShapeDimensions[radiusAxis];
	}

	Scalar getHalfHeight() const
	{
		return m_implicitShapeDimensions[m_upAxis];
	}

	virtual void setLocalScaling(const Vec3& scaling)
	{
		Vec3 unScaledImplicitShapeDimensions = m_implicitShapeDimensions / m_localScaling;
		ConvexInternalShape::setLocalScaling(scaling);
		m_implicitShapeDimensions = (unScaledImplicitShapeDimensions * scaling);
		//update m_collisionMargin, since entire radius==margin
		i32 radiusAxis = (m_upAxis + 2) % 3;
		m_collisionMargin = m_implicitShapeDimensions[radiusAxis];
	}

	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		Vec3 aniDir(0, 0, 0);
		aniDir[getUpAxis()] = 1;
		return aniDir;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	SIMD_FORCE_INLINE void deSerializeFloat(struct CapsuleShapeData * dataBuffer);
};

//CapsuleShapeX represents a capsule around the Z axis
///the total height is height+2*radius, so the height is just the height between the center of each 'sphere' of the capsule caps.
class CapsuleShapeX : public CapsuleShape
{
public:
	CapsuleShapeX(Scalar radius, Scalar height);

	//debugging
	virtual tukk getName() const
	{
		return "CapsuleX";
	}
};

//CapsuleShapeZ represents a capsule around the Z axis
///the total height is height+2*radius, so the height is just the height between the center of each 'sphere' of the capsule caps.
class CapsuleShapeZ : public CapsuleShape
{
public:
	CapsuleShapeZ(Scalar radius, Scalar height);

	//debugging
	virtual tukk getName() const
	{
		return "CapsuleZ";
	}
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct CapsuleShapeData
{
	ConvexInternalShapeData m_convexInternalShapeData;

	i32 m_upAxis;

	char m_padding[4];
};

SIMD_FORCE_INLINE i32 CapsuleShape::calculateSerializeBufferSize() const
{
	return sizeof(CapsuleShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk CapsuleShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	CapsuleShapeData* shapeData = (CapsuleShapeData*)dataBuffer;

	ConvexInternalShape::serialize(&shapeData->m_convexInternalShapeData, serializer);

	shapeData->m_upAxis = m_upAxis;

	// Fill padding with zeros to appease msan.
	shapeData->m_padding[0] = 0;
	shapeData->m_padding[1] = 0;
	shapeData->m_padding[2] = 0;
	shapeData->m_padding[3] = 0;

	return "CapsuleShapeData";
}

SIMD_FORCE_INLINE void CapsuleShape::deSerializeFloat(CapsuleShapeData* dataBuffer)
{
	m_implicitShapeDimensions.deSerializeFloat(dataBuffer->m_convexInternalShapeData.m_implicitShapeDimensions);
	m_collisionMargin = dataBuffer->m_convexInternalShapeData.m_collisionMargin;
	m_localScaling.deSerializeFloat(dataBuffer->m_convexInternalShapeData.m_localScaling);
	//it is best to already pre-allocate the matching CapsuleShape*(X/Z) version to match m_upAxis
	m_upAxis = dataBuffer->m_upAxis;
}

#endif  //DRX3D_CAPSULE_SHAPE_H
