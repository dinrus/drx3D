#ifndef D3_GPU_GRID_BROADPHASE_H
#define D3_GPU_GRID_BROADPHASE_H

#include <drx3D/Physics/Collision/BroadPhase/b3GpuBroadphaseInterface.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>

struct b3ParamsGridBroadphaseCL
{
	float m_invCellSize[4];
	i32 m_gridSize[4];

	i32 getMaxBodiesPerCell() const
	{
		return m_gridSize[3];
	}

	void setMaxBodiesPerCell(i32 maxOverlap)
	{
		m_gridSize[3] = maxOverlap;
	}
};

class b3GpuGridBroadphase : public b3GpuBroadphaseInterface
{
protected:
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

	b3OpenCLArray<b3SapAabb> m_allAabbsGPU1;
	b3AlignedObjectArray<b3SapAabb> m_allAabbsCPU1;

	b3OpenCLArray<i32> m_smallAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_smallAabbsMappingCPU;

	b3OpenCLArray<i32> m_largeAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_largeAabbsMappingCPU;

	b3AlignedObjectArray<b3Int4> m_hostPairs;
	b3OpenCLArray<b3Int4> m_gpuPairs;

	b3OpenCLArray<b3SortData> m_hashGpu;
	b3OpenCLArray<i32> m_cellStartGpu;

	b3ParamsGridBroadphaseCL m_paramsCPU;
	b3OpenCLArray<b3ParamsGridBroadphaseCL> m_paramsGPU;

	class b3RadixSort32CL* m_sorter;

public:
	b3GpuGridBroadphase(cl_context ctx, cl_device_id device, cl_command_queue q);
	virtual ~b3GpuGridBroadphase();

	static b3GpuBroadphaseInterface* CreateFunc(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuGridBroadphase(ctx, device, q);
	}

	virtual void createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);
	virtual void createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);

	virtual void calculateOverlappingPairs(i32 maxPairs);
	virtual void calculateOverlappingPairsHost(i32 maxPairs);

	//call writeAabbsToGpu after done making all changes (createProxy etc)
	virtual void writeAabbsToGpu();

	virtual cl_mem getAabbBufferWS();
	virtual i32 getNumOverlap();
	virtual cl_mem getOverlappingPairBuffer();

	virtual b3OpenCLArray<b3SapAabb>& getAllAabbsGPU();
	virtual b3AlignedObjectArray<b3SapAabb>& getAllAabbsCPU();

	virtual b3OpenCLArray<b3Int4>& getOverlappingPairsGPU();
	virtual b3OpenCLArray<i32>& getSmallAabbIndicesGPU();
	virtual b3OpenCLArray<i32>& getLargeAabbIndicesGPU();
};

#endif  //D3_GPU_GRID_BROADPHASE_H