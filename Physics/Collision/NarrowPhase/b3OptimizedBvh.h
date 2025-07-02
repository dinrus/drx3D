#ifndef D3_OPTIMIZED_BVH_H
#define D3_OPTIMIZED_BVH_H

#include <drx3D/Physics/Collision/NarrowPhase/b3QuantizedBvh.h>

class b3StridingMeshInterface;

///The b3OptimizedBvh extends the b3QuantizedBvh to create AABB tree for triangle meshes, through the b3StridingMeshInterface.
D3_ATTRIBUTE_ALIGNED16(class)
b3OptimizedBvh : public b3QuantizedBvh
{
public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

protected:
public:
	b3OptimizedBvh();

	virtual ~b3OptimizedBvh();

	void build(b3StridingMeshInterface * triangles, bool useQuantizedAabbCompression, const b3Vec3& bvhAabbMin, const b3Vec3& bvhAabbMax);

	void refit(b3StridingMeshInterface * triangles, const b3Vec3& aabbMin, const b3Vec3& aabbMax);

	void refitPartial(b3StridingMeshInterface * triangles, const b3Vec3& aabbMin, const b3Vec3& aabbMax);

	void updateBvhNodes(b3StridingMeshInterface * meshInterface, i32 firstNode, i32 endNode, i32 index);

	/// Data buffer MUST be 16 byte aligned
	virtual bool serializeInPlace(uk o_alignedDataBuffer, unsigned i_dataBufferSize, bool i_swapEndian) const
	{
		return b3QuantizedBvh::serialize(o_alignedDataBuffer, i_dataBufferSize, i_swapEndian);
	}

	///deSerializeInPlace loads and initializes a BVH from a buffer in memory 'in place'
	static b3OptimizedBvh* deSerializeInPlace(uk i_alignedDataBuffer, u32 i_dataBufferSize, bool i_swapEndian);
};

#endif  //D3_OPTIMIZED_BVH_H
