#ifndef DRX3D_SCALED_BVH_TRIANGLE_MESH_SHAPE_H
#define DRX3D_SCALED_BVH_TRIANGLE_MESH_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>

///The ScaledBvhTriangleMeshShape allows to instance a scaled version of an existing BvhTriangleMeshShape.
///Note that each BvhTriangleMeshShape still can have its own local scaling, independent from this ScaledBvhTriangleMeshShape 'localScaling'
ATTRIBUTE_ALIGNED16(class)
ScaledBvhTriangleMeshShape : public ConcaveShape
{
	Vec3 m_localScaling;

	BvhTriangleMeshShape* m_bvhTriMeshShape;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ScaledBvhTriangleMeshShape(BvhTriangleMeshShape * childShape, const Vec3& localScaling);

	virtual ~ScaledBvhTriangleMeshShape();

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;
	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const;
	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	BvhTriangleMeshShape* getChildShape()
	{
		return m_bvhTriMeshShape;
	}

	const BvhTriangleMeshShape* getChildShape() const
	{
		return m_bvhTriMeshShape;
	}

	//debugging
	virtual tukk getName() const { return "SCALEDBVHTRIANGLEMESH"; }

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct ScaledTriangleMeshShapeData
{
	TriangleMeshShapeData m_trimeshShapeData;

	Vec3FloatData m_localScaling;
};

SIMD_FORCE_INLINE i32 ScaledBvhTriangleMeshShape::calculateSerializeBufferSize() const
{
	return sizeof(ScaledTriangleMeshShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk ScaledBvhTriangleMeshShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	ScaledTriangleMeshShapeData* scaledMeshData = (ScaledTriangleMeshShapeData*)dataBuffer;
	m_bvhTriMeshShape->serialize(&scaledMeshData->m_trimeshShapeData, serializer);
	scaledMeshData->m_trimeshShapeData.m_collisionShapeData.m_shapeType = SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE;
	m_localScaling.serializeFloat(scaledMeshData->m_localScaling);
	return "ScaledTriangleMeshShapeData";
}

#endif  //DRX3D_SCALED_BVH_TRIANGLE_MESH_SHAPE_H
