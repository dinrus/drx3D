#ifndef DRX3D_OPTIMIZED_BVH_H
#define DRX3D_OPTIMIZED_BVH_H

#include <drx3D/Physics/Collision/BroadPhase/QuantizedBvh.h>

class StridingMeshInterface;

///The OptimizedBvh extends the QuantizedBvh to create AABB tree for triangle meshes, through the StridingMeshInterface.
ATTRIBUTE_ALIGNED16(class)
OptimizedBvh : public QuantizedBvh
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

protected:
public:
	OptimizedBvh();

	virtual ~OptimizedBvh();

	void build(StridingMeshInterface * triangles, bool useQuantizedAabbCompression, const Vec3& bvhAabbMin, const Vec3& bvhAabbMax);

	void refit(StridingMeshInterface * triangles, const Vec3& aabbMin, const Vec3& aabbMax);

	void refitPartial(StridingMeshInterface * triangles, const Vec3& aabbMin, const Vec3& aabbMax);

	void updateBvhNodes(StridingMeshInterface * meshInterface, i32 firstNode, i32 endNode, i32 index);

	/// Data buffer MUST be 16 byte aligned
	virtual bool serializeInPlace(uk o_alignedDataBuffer, unsigned i_dataBufferSize, bool i_swapEndian) const
	{
		return QuantizedBvh::serialize(o_alignedDataBuffer, i_dataBufferSize, i_swapEndian);
	}

	///deSerializeInPlace loads and initializes a BVH from a buffer in memory 'in place'
	static OptimizedBvh* deSerializeInPlace(uk i_alignedDataBuffer, u32 i_dataBufferSize, bool i_swapEndian);
};

#endif  //DRX3D_OPTIMIZED_BVH_H
