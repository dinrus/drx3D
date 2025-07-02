#ifndef D3_GPU_SAP_BROADPHASE_H
#define D3_GPU_SAP_BROADPHASE_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>  //b3Int2
class b3Vec3;
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>

#include <drx3D/Physics/Collision/BroadPhase/b3SapAabb.h>
#include <drx3D/Common/shared/b3Int2.h>

#include <drx3D/Physics/Collision/BroadPhase/b3GpuBroadphaseInterface.h>

class b3GpuSapBroadphase : public b3GpuBroadphaseInterface
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;
	cl_kernel m_flipFloatKernel;
	cl_kernel m_scatterKernel;
	cl_kernel m_copyAabbsKernel;
	cl_kernel m_sapKernel;
	cl_kernel m_sap2Kernel;
	cl_kernel m_prepareSumVarianceKernel;

	class b3RadixSort32CL* m_sorter;

	///test for 3d SAP
	b3AlignedObjectArray<b3SortData> m_sortedAxisCPU[3][2];
	b3AlignedObjectArray<b3UnsignedInt2> m_objectMinMaxIndexCPU[3][2];
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis0;
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis1;
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis2;
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis0prev;
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis1prev;
	b3OpenCLArray<b3UnsignedInt2> m_objectMinMaxIndexGPUaxis2prev;

	b3OpenCLArray<b3SortData> m_sortedAxisGPU0;
	b3OpenCLArray<b3SortData> m_sortedAxisGPU1;
	b3OpenCLArray<b3SortData> m_sortedAxisGPU2;
	b3OpenCLArray<b3SortData> m_sortedAxisGPU0prev;
	b3OpenCLArray<b3SortData> m_sortedAxisGPU1prev;
	b3OpenCLArray<b3SortData> m_sortedAxisGPU2prev;

	b3OpenCLArray<b3Int4> m_addedHostPairsGPU;
	b3OpenCLArray<b3Int4> m_removedHostPairsGPU;
	b3OpenCLArray<i32> m_addedCountGPU;
	b3OpenCLArray<i32> m_removedCountGPU;

	i32 m_currentBuffer;

public:
	b3OpenCLArray<i32> m_pairCount;

	b3OpenCLArray<b3SapAabb> m_allAabbsGPU;
	b3AlignedObjectArray<b3SapAabb> m_allAabbsCPU;

	virtual b3OpenCLArray<b3SapAabb>& getAllAabbsGPU()
	{
		return m_allAabbsGPU;
	}
	virtual b3AlignedObjectArray<b3SapAabb>& getAllAabbsCPU()
	{
		return m_allAabbsCPU;
	}

	b3OpenCLArray<b3Vec3> m_sum;
	b3OpenCLArray<b3Vec3> m_sum2;
	b3OpenCLArray<b3Vec3> m_dst;

	b3OpenCLArray<i32> m_smallAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_smallAabbsMappingCPU;

	b3OpenCLArray<i32> m_largeAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_largeAabbsMappingCPU;

	b3OpenCLArray<b3Int4> m_overlappingPairs;

	//temporary gpu work memory
	b3OpenCLArray<b3SortData> m_gpuSmallSortData;
	b3OpenCLArray<b3SapAabb> m_gpuSmallSortedAabbs;

	class b3PrefixScanFloat4CL* m_prefixScanFloat4;

	enum b3GpuSapKernelType
	{
		D3_GPU_SAP_KERNEL_BRUTE_FORCE_CPU = 1,
		D3_GPU_SAP_KERNEL_BRUTE_FORCE_GPU,
		D3_GPU_SAP_KERNEL_ORIGINAL,
		D3_GPU_SAP_KERNEL_BARRIER,
		D3_GPU_SAP_KERNEL_LOCAL_SHARED_MEMORY
	};

	b3GpuSapBroadphase(cl_context ctx, cl_device_id device, cl_command_queue q, b3GpuSapKernelType kernelType = D3_GPU_SAP_KERNEL_LOCAL_SHARED_MEMORY);
	virtual ~b3GpuSapBroadphase();

	static b3GpuBroadphaseInterface* CreateFuncBruteForceCpu(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuSapBroadphase(ctx, device, q, D3_GPU_SAP_KERNEL_BRUTE_FORCE_CPU);
	}

	static b3GpuBroadphaseInterface* CreateFuncBruteForceGpu(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuSapBroadphase(ctx, device, q, D3_GPU_SAP_KERNEL_BRUTE_FORCE_GPU);
	}

	static b3GpuBroadphaseInterface* CreateFuncOriginal(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuSapBroadphase(ctx, device, q, D3_GPU_SAP_KERNEL_ORIGINAL);
	}
	static b3GpuBroadphaseInterface* CreateFuncBarrier(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuSapBroadphase(ctx, device, q, D3_GPU_SAP_KERNEL_BARRIER);
	}
	static b3GpuBroadphaseInterface* CreateFuncLocalMemory(cl_context ctx, cl_device_id device, cl_command_queue q)
	{
		return new b3GpuSapBroadphase(ctx, device, q, D3_GPU_SAP_KERNEL_LOCAL_SHARED_MEMORY);
	}

	virtual void calculateOverlappingPairs(i32 maxPairs);
	virtual void calculateOverlappingPairsHost(i32 maxPairs);

	void reset();

	void init3dSap();
	virtual void calculateOverlappingPairsHostIncremental3Sap();

	virtual void createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);
	virtual void createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask);

	//call writeAabbsToGpu after done making all changes (createProxy etc)
	virtual void writeAabbsToGpu();

	virtual cl_mem getAabbBufferWS();
	virtual i32 getNumOverlap();
	virtual cl_mem getOverlappingPairBuffer();

	virtual b3OpenCLArray<b3Int4>& getOverlappingPairsGPU();
	virtual b3OpenCLArray<i32>& getSmallAabbIndicesGPU();
	virtual b3OpenCLArray<i32>& getLargeAabbIndicesGPU();
};

#endif  //D3_GPU_SAP_BROADPHASE_H