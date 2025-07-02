#ifndef DRX3D_CYLINDER_MINKOWSKI_H
#define DRX3D_CYLINDER_MINKOWSKI_H

#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>  // for the types
#include <drx3D/Maths/Linear/Vec3.h>

/// The CylinderShape class implements a cylinder shape primitive, centered around the origin. Its central axis aligned with the Y axis. CylinderShapeX is aligned with the X axis and CylinderShapeZ around the Z axis.
ATTRIBUTE_ALIGNED16(class)
CylinderShape : public ConvexInternalShape

{
protected:
	i32 m_upAxis;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Vec3 getHalfExtentsWithMargin() const
	{
		Vec3 halfExtents = getHalfExtentsWithoutMargin();
		Vec3 margin(getMargin(), getMargin(), getMargin());
		halfExtents += margin;
		return halfExtents;
	}

	const Vec3& getHalfExtentsWithoutMargin() const
	{
		return m_implicitShapeDimensions;  //changed in drx3D 2.63: assume the scaling and margin are included
	}

	CylinderShape(const Vec3& halfExtents);

	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;

	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	virtual void setMargin(Scalar collisionMargin)
	{
		//correct the m_implicitShapeDimensions for the margin
		Vec3 oldMargin(getMargin(), getMargin(), getMargin());
		Vec3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions + oldMargin;

		ConvexInternalShape::setMargin(collisionMargin);
		Vec3 newMargin(getMargin(), getMargin(), getMargin());
		m_implicitShapeDimensions = implicitShapeDimensionsWithMargin - newMargin;
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const
	{
		Vec3 supVertex;
		supVertex = localGetSupportingVertexWithoutMargin(vec);

		if (getMargin() != Scalar(0.))
		{
			Vec3 vecnorm = vec;
			if (vecnorm.length2() < (SIMD_EPSILON * SIMD_EPSILON))
			{
				vecnorm.setVal(Scalar(-1.), Scalar(-1.), Scalar(-1.));
			}
			vecnorm.normalize();
			supVertex += getMargin() * vecnorm;
		}
		return supVertex;
	}

	//use box inertia
	//	virtual void	calculateLocalInertia(Scalar mass,Vec3& inertia) const;

	i32 getUpAxis() const
	{
		return m_upAxis;
	}

	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		Vec3 aniDir(0, 0, 0);
		aniDir[getUpAxis()] = 1;
		return aniDir;
	}

	virtual Scalar getRadius() const
	{
		return getHalfExtentsWithMargin().getX();
	}

	virtual void setLocalScaling(const Vec3& scaling)
	{
		Vec3 oldMargin(getMargin(), getMargin(), getMargin());
		Vec3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions + oldMargin;
		Vec3 unScaledImplicitShapeDimensionsWithMargin = implicitShapeDimensionsWithMargin / m_localScaling;

		ConvexInternalShape::setLocalScaling(scaling);

		m_implicitShapeDimensions = (unScaledImplicitShapeDimensionsWithMargin * m_localScaling) - oldMargin;
	}

	//debugging
	virtual tukk getName() const
	{
		return "CylinderY";
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

class CylinderShapeX : public CylinderShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	CylinderShapeX(const Vec3& halfExtents);

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	//debugging
	virtual tukk getName() const
	{
		return "CylinderX";
	}

	virtual Scalar getRadius() const
	{
		return getHalfExtentsWithMargin().getY();
	}
};

class CylinderShapeZ : public CylinderShape
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	CylinderShapeZ(const Vec3& halfExtents);

	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	//debugging
	virtual tukk getName() const
	{
		return "CylinderZ";
	}

	virtual Scalar getRadius() const
	{
		return getHalfExtentsWithMargin().getX();
	}
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct CylinderShapeData
{
	ConvexInternalShapeData m_convexInternalShapeData;

	i32 m_upAxis;

	char m_padding[4];
};

SIMD_FORCE_INLINE i32 CylinderShape::calculateSerializeBufferSize() const
{
	return sizeof(CylinderShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk CylinderShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	CylinderShapeData* shapeData = (CylinderShapeData*)dataBuffer;

	ConvexInternalShape::serialize(&shapeData->m_convexInternalShapeData, serializer);

	shapeData->m_upAxis = m_upAxis;

	// Fill padding with zeros to appease msan.
	shapeData->m_padding[0] = 0;
	shapeData->m_padding[1] = 0;
	shapeData->m_padding[2] = 0;
	shapeData->m_padding[3] = 0;

	return "CylinderShapeData";
}

#endif  //DRX3D_CYLINDER_MINKOWSKI_H
