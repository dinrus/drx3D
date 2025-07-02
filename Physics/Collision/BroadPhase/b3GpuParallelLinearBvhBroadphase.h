#ifndef D3_GPU_PARALLEL_LINEAR_BVH_BROADPHASE_H
#define D3_GPU_PARALLEL_LINEAR_BVH_BROADPHASE_H

#include <drx3D/Physics/Collision/BroadPhase/b3GpuBroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/b3GpuParallelLinearBvh.h>

class b3GpuParallelLinearBvhBroadphase : public b3GpuBroadphaseInterface
{
	b3GpuParallelLinearBvh m_plbvh;

	b3OpenCLArray<b3Int4> m_overlappingPairsGpu;

	b3OpenCLArray<b3SapAabb> m_aabbsGpu;
	b3OpenCLArray<i32> m_smallAabbsMappingGpu;
	b3OpenCLArray<i32> m_largeAabbsMappingGpu;

	b3AlignedObjectArray<b3SapAabb> m_aabbsCpu;
	b3AlignedObjectArray<i32> m_smallAabbsMappingCpu;
	b3AlignedObjectArray<i32> m_largeAabbsMappingCpu;

public:
	b3GpuParallelLinearBvhBroadphase(cl_context context, cl_device_id device, cl_command_queue queue);
	virtual ~b3GpuParallelLinearBvhBroadphase() {}

	virtual void createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);
	virtual void createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);

	virtual void calculateOverlappingPairs(i32 maxPairs);
	virtual void calculateOverlappingPairsHost(i32 maxPairs);

	//call writeAabbsToGpu after done making all changes (createProxy etc)
	virtual void writeAabbsToGpu();

	virtual i32 getNumOverlap() { return m_overlappingPairsGpu.size(); }
	virtual cl_mem getOverlappingPairBuffer() { return m_overlappingPairsGpu.getBufferCL(); }

	virtual cl_mem getAabbBufferWS() { return m_aabbsGpu.getBufferCL(); }
	virtual b3OpenCLArray<b3SapAabb>& getAllAabbsGPU() { return m_aabbsGpu; }

	virtual b3OpenCLArray<b3Int4>& getOverlappingPairsGPU() { return m_overlappingPairsGpu; }
	virtual b3OpenCLArray<i32>& getSmallAabbIndicesGPU() { return m_smallAabbsMappingGpu; }
	virtual b3OpenCLArray<i32>& getLargeAabbIndicesGPU() { return m_largeAabbsMappingGpu; }

	virtual b3AlignedObjectArray<b3SapAabb>& getAllAabbsCPU() { return m_aabbsCpu; }

	static b3GpuBroadphaseInterface* CreateFunc(cl_context context, cl_device_id device, cl_command_queue queue)
	{
		return new b3GpuParallelLinearBvhBroadphase(context, device, queue);
	}
};

#endif
