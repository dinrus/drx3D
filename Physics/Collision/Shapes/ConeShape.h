#ifndef DRX3D_CONE_MINKOWSKI_H
#define DRX3D_CONE_MINKOWSKI_H

#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/BroadPhase//BroadphaseProxy.h>  // for the types

///The ConeShape implements a cone shape primitive, centered around the origin and aligned with the Y axis. The ConeShapeX is aligned around the X axis and ConeShapeZ around the Z axis.
ATTRIBUTE_ALIGNED16(class)
ConeShape : public ConvexInternalShape

{
	Scalar m_sinAngle;
	Scalar m_radius;
	Scalar m_height;
	i32 m_coneIndices[3];
	Vec3 coneLocalSupport(const Vec3& v) const;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConeShape(Scalar radius, Scalar height);

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const;
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const;

	Scalar getRadius() const { return m_radius; }
	Scalar getHeight() const { return m_height; }

	void setRadius(const Scalar radius)
	{
		m_radius = radius;
	}
	void setHeight(const Scalar height)
	{
		m_height = height;
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const
	{
		Transform2 identity;
		identity.setIdentity();
		Vec3 aabbMin, aabbMax;
		getAabb(identity, aabbMin, aabbMax);

		Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);

		Scalar margin = getMargin();

		Scalar lx = Scalar(2.) * (halfExtents.x() + margin);
		Scalar ly = Scalar(2.) * (halfExtents.y() + margin);
		Scalar lz = Scalar(2.) * (halfExtents.z() + margin);
		const Scalar x2 = lx * lx;
		const Scalar y2 = ly * ly;
		const Scalar z2 = lz * lz;
		const Scalar scaledmass = mass * Scalar(0.08333333);

		inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));

		//		inertia.x() = scaledmass * (y2+z2);
		//		inertia.y() = scaledmass * (x2+z2);
		//		inertia.z() = scaledmass * (x2+y2);
	}

	virtual tukk getName() const
	{
		return "Cone";
	}

	///choose upAxis index
	void setConeUpIndex(i32 upIndex);

	i32 getConeUpIndex() const
	{
		return m_coneIndices[1];
	}

	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		return Vec3(0, 1, 0);
	}

	virtual void setLocalScaling(const Vec3& scaling);

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

//ConeShape implements a Cone shape, around the X axis
class ConeShapeX : public ConeShape
{
public:
	ConeShapeX(Scalar radius, Scalar height);

	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		return Vec3(1, 0, 0);
	}

	//debugging
	virtual tukk getName() const
	{
		return "ConeX";
	}
};

//ConeShapeZ implements a Cone shape, around the Z axis
class ConeShapeZ : public ConeShape
{
public:
	ConeShapeZ(Scalar radius, Scalar height);

	virtual Vec3 getAnisotropicRollingFrictionDirection() const
	{
		return Vec3(0, 0, 1);
	}

	//debugging
	virtual tukk getName() const
	{
		return "ConeZ";
	}
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct ConeShapeData
{
	ConvexInternalShapeData m_convexInternalShapeData;

	i32 m_upIndex;

	char m_padding[4];
};

SIMD_FORCE_INLINE i32 ConeShape::calculateSerializeBufferSize() const
{
	return sizeof(ConeShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk ConeShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	ConeShapeData* shapeData = (ConeShapeData*)dataBuffer;

	ConvexInternalShape::serialize(&shapeData->m_convexInternalShapeData, serializer);

	shapeData->m_upIndex = m_coneIndices[1];

	// Fill padding with zeros to appease msan.
	shapeData->m_padding[0] = 0;
	shapeData->m_padding[1] = 0;
	shapeData->m_padding[2] = 0;
	shapeData->m_padding[3] = 0;

	return "ConeShapeData";
}

#endif  //DRX3D_CONE_MINKOWSKI_H
