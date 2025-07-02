#ifndef DRX3D_BVH_TRIANGLE_MESH_SHAPE_H
#define DRX3D_BVH_TRIANGLE_MESH_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/TriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/OptimizedBvh.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Physics/Collision/Shapes/TriangleInfoMap.h>

///The BvhTriangleMeshShape is a static-triangle mesh shape, it can only be used for fixed/non-moving objects.
///If you required moving concave triangle meshes, it is recommended to perform convex decomposition
///using HACD, see drx3D/Demos/ConvexDecompositionDemo.
///Alternatively, you can use GimpactMeshShape for moving concave triangle meshes.
//BvhTriangleMeshShape has several optimizations, such as bounding volume hierarchy and
///cache friendly traversal for PlayStation 3 Cell SPU.
///It is recommended to enable useQuantizedAabbCompression for better memory usage.
///It takes a triangle mesh as input, for example a TriangleMesh or TriangleIndexVertexArray. The BvhTriangleMeshShape class allows for triangle mesh deformations by a refit or partialRefit method.
///Instead of building the bounding volume hierarchy acceleration structure, it is also possible to serialize (save) and deserialize (load) the structure from disk.
///See Demos\ConcaveDemo\ConcavePhysicsDemo.cpp for an example.
ATTRIBUTE_ALIGNED16(class)
BvhTriangleMeshShape : public TriangleMeshShape
{
	OptimizedBvh* m_bvh;
	TriangleInfoMap* m_triangleInfoMap;

	bool m_useQuantizedAabbCompression;
	bool m_ownsBvh;
#ifdef __clang__
	bool m_pad[11] __attribute__((unused));  ////need padding due to alignment
#else
	bool m_pad[11];  ////need padding due to alignment
#endif

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	BvhTriangleMeshShape(StridingMeshInterface * meshInterface, bool useQuantizedAabbCompression, bool buildBvh = true);

	///optionally pass in a larger bvh aabb, used for quantization. This allows for deformations within this aabb
	BvhTriangleMeshShape(StridingMeshInterface * meshInterface, bool useQuantizedAabbCompression, const Vec3& bvhAabbMin, const Vec3& bvhAabbMax, bool buildBvh = true);

	virtual ~BvhTriangleMeshShape();

	bool getOwnsBvh() const
	{
		return m_ownsBvh;
	}

	void performRaycast(TriangleCallback * callback, const Vec3& raySource, const Vec3& rayTarget);
	void performConvexcast(TriangleCallback * callback, const Vec3& boxSource, const Vec3& boxTarget, const Vec3& boxMin, const Vec3& boxMax);

	virtual void processAllTriangles(TriangleCallback * callback, const Vec3& aabbMin, const Vec3& aabbMax) const;

	void refitTree(const Vec3& aabbMin, const Vec3& aabbMax);

	///for a fast incremental refit of parts of the tree. Note: the entire AABB of the tree will become more conservative, it never shrinks
	void partialRefitTree(const Vec3& aabbMin, const Vec3& aabbMax);

	//debugging
	virtual tukk getName() const { return "BVHTRIANGLEMESH"; }

	virtual void setLocalScaling(const Vec3& scaling);

	OptimizedBvh* getOptimizedBvh()
	{
		return m_bvh;
	}

	void setOptimizedBvh(OptimizedBvh * bvh, const Vec3& localScaling = Vec3(1, 1, 1));

	void buildOptimizedBvh();

	bool usesQuantizedAabbCompression() const
	{
		return m_useQuantizedAabbCompression;
	}

	void setTriangleInfoMap(TriangleInfoMap * triangleInfoMap)
	{
		m_triangleInfoMap = triangleInfoMap;
	}

	const TriangleInfoMap* getTriangleInfoMap() const
	{
		return m_triangleInfoMap;
	}

	TriangleInfoMap* getTriangleInfoMap()
	{
		return m_triangleInfoMap;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	virtual void serializeSingleBvh(Serializer * serializer) const;

	virtual void serializeSingleTriangleInfoMap(Serializer * serializer) const;
};

// clang-format off

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	TriangleMeshShapeData
{
	CollisionShapeData	m_collisionShapeData;

	StridingMeshInterfaceData m_meshInterface;

	QuantizedBvhFloatData		*m_quantizedFloatBvh;
	QuantizedBvhDoubleData	*m_quantizedDoubleBvh;

	TriangleInfoMapData	*m_triangleInfoMap;
	
	float	m_collisionMargin;

	char m_pad3[4];
	
};

// clang-format on

SIMD_FORCE_INLINE i32 BvhTriangleMeshShape::calculateSerializeBufferSize() const
{
	return sizeof(TriangleMeshShapeData);
}

#endif  //DRX3D_BVH_TRIANGLE_MESH_SHAPE_H
