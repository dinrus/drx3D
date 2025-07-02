#ifndef DRX3D_STATIC_PLANE_SHAPE_H
#define DRX3D_STATIC_PLANE_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>

///The StaticPlaneShape simulates an infinite non-moving (static) collision plane.
ATTRIBUTE_ALIGNED16(class)
StaticPlaneShape : public ConcaveShape
{
protected:
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;

	Vec3 m_planeNormal;
	Scalar m_planeConstant;
	Vec3 m_localScaling;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	StaticPlaneShape(const Vec3& planeNormal, Scalar planeConstant);

	virtual ~StaticPlaneShape();

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;

	const Vec3& getPlaneNormal() const
	{
		return m_planeNormal;
	}

	const Scalar& getPlaneConstant() const
	{
		return m_planeConstant;
	}

	//debugging
	virtual tukk getName() const { return "STATICPLANE"; }

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct StaticPlaneShapeData
{
	CollisionShapeData m_collisionShapeData;

	Vec3FloatData m_localScaling;
	Vec3FloatData m_planeNormal;
	float m_planeConstant;
	char m_pad[4];
};

SIMD_FORCE_INLINE i32 StaticPlaneShape::calculateSerializeBufferSize() const
{
	return sizeof(StaticPlaneShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk StaticPlaneShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	StaticPlaneShapeData* planeData = (StaticPlaneShapeData*)dataBuffer;
	CollisionShape::serialize(&planeData->m_collisionShapeData, serializer);

	m_localScaling.serializeFloat(planeData->m_localScaling);
	m_planeNormal.serializeFloat(planeData->m_planeNormal);
	planeData->m_planeConstant = float(m_planeConstant);

	// Fill padding with zeros to appease msan.
	planeData->m_pad[0] = 0;
	planeData->m_pad[1] = 0;
	planeData->m_pad[2] = 0;
	planeData->m_pad[3] = 0;

	return "StaticPlaneShapeData";
}

#endif  //DRX3D_STATIC_PLANE_SHAPE_H
