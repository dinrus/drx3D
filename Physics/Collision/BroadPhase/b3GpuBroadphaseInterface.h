
#ifndef D3_GPU_BROADPHASE_INTERFACE_H
#define D3_GPU_BROADPHASE_INTERFACE_H

#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Physics/Collision/BroadPhase/b3SapAabb.h>
#include <drx3D/Common/shared/b3Int2.h>
#include <drx3D/Common/shared/b3Int4.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>

class b3GpuBroadphaseInterface
{
public:
	typedef class b3GpuBroadphaseInterface*(CreateFunc)(cl_context ctx, cl_device_id device, cl_command_queue q);

	virtual ~b3GpuBroadphaseInterface()
	{
	}

	virtual void createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask) = 0;
	virtual void createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask) = 0;

	virtual void calculateOverlappingPairs(i32 maxPairs) = 0;
	virtual void calculateOverlappingPairsHost(i32 maxPairs) = 0;

	//call writeAabbsToGpu after done making all changes (createProxy etc)
	virtual void writeAabbsToGpu() = 0;

	virtual cl_mem getAabbBufferWS() = 0;
	virtual i32 getNumOverlap() = 0;
	virtual cl_mem getOverlappingPairBuffer() = 0;

	virtual b3OpenCLArray<b3SapAabb>& getAllAabbsGPU() = 0;
	virtual b3AlignedObjectArray<b3SapAabb>& getAllAabbsCPU() = 0;

	virtual b3OpenCLArray<b3Int4>& getOverlappingPairsGPU() = 0;
	virtual b3OpenCLArray<i32>& getSmallAabbIndicesGPU() = 0;
	virtual b3OpenCLArray<i32>& getLargeAabbIndicesGPU() = 0;
};

#endif  //D3_GPU_BROADPHASE_INTERFACE_H
