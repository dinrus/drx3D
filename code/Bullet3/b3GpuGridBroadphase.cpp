
#include <drx3D/Physics/Collision/BroadPhase/b3GpuGridBroadphase.h>
#include <drx3D/Geometry/b3AabbUtil.h>
#include <drx3D/Physics/Collision/BroadPhase/kernels/gridBroadphaseKernels.h>
#include <drx3D/Physics/Collision/BroadPhase/kernels/sapKernels.h>
//#include drx3D/Physics/Collision/BroadPhase/kernels/gridBroadphase.cl"

#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>

#define D3_BROADPHASE_SAP_PATH "drx3D/Physics/Collision/BroadPhase/kernels/sap.cl"
#define D3_GRID_BROADPHASE_PATH "drx3D/Physics/Collision/BroadPhase/kernels/gridBroadphase.cl"

cl_kernel kCalcHashAABB;
cl_kernel kClearCellStart;
cl_kernel kFindCellStart;
cl_kernel kFindOverlappingPairs;
cl_kernel m_copyAabbsKernel;
cl_kernel m_sap2Kernel;

//i32 maxPairsPerBody = 64;
i32 maxBodiesPerCell = 256;  //??

b3GpuGridBroadphase::b3GpuGridBroadphase(cl_context ctx, cl_device_id device, cl_command_queue q)
	: m_context(ctx),
	  m_device(device),
	  m_queue(q),
	  m_allAabbsGPU1(ctx, q),
	  m_smallAabbsMappingGPU(ctx, q),
	  m_largeAabbsMappingGPU(ctx, q),
	  m_gpuPairs(ctx, q),

	  m_hashGpu(ctx, q),

	  m_cellStartGpu(ctx, q),
	  m_paramsGPU(ctx, q)
{
	b3Vec3 gridSize = b3MakeVector3(3, 3, 3);
	b3Vec3 invGridSize = b3MakeVector3(1.f / gridSize[0], 1.f / gridSize[1], 1.f / gridSize[2]);

	m_paramsCPU.m_gridSize[0] = 128;
	m_paramsCPU.m_gridSize[1] = 128;
	m_paramsCPU.m_gridSize[2] = 128;
	m_paramsCPU.m_gridSize[3] = maxBodiesPerCell;
	m_paramsCPU.setMaxBodiesPerCell(maxBodiesPerCell);
	m_paramsCPU.m_invCellSize[0] = invGridSize[0];
	m_paramsCPU.m_invCellSize[1] = invGridSize[1];
	m_paramsCPU.m_invCellSize[2] = invGridSize[2];
	m_paramsCPU.m_invCellSize[3] = 0.f;
	m_paramsGPU.push_back(m_paramsCPU);

	cl_int errNum = 0;

	{
		tukk sapSrc = sapCL;
		cl_program sapProg = b3OpenCLUtils::compileCLProgramFromString(m_context, m_device, sapSrc, &errNum, "", D3_BROADPHASE_SAP_PATH);
		drx3DAssert(errNum == CL_SUCCESS);
		m_copyAabbsKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "copyAabbsKernel", &errNum, sapProg);
		m_sap2Kernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelTwoArrays", &errNum, sapProg);
		drx3DAssert(errNum == CL_SUCCESS);
	}

	{
		cl_program gridProg = b3OpenCLUtils::compileCLProgramFromString(m_context, m_device, gridBroadphaseCL, &errNum, "", D3_GRID_BROADPHASE_PATH);
		drx3DAssert(errNum == CL_SUCCESS);

		kCalcHashAABB = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, gridBroadphaseCL, "kCalcHashAABB", &errNum, gridProg);
		drx3DAssert(errNum == CL_SUCCESS);

		kClearCellStart = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, gridBroadphaseCL, "kClearCellStart", &errNum, gridProg);
		drx3DAssert(errNum == CL_SUCCESS);

		kFindCellStart = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, gridBroadphaseCL, "kFindCellStart", &errNum, gridProg);
		drx3DAssert(errNum == CL_SUCCESS);

		kFindOverlappingPairs = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, gridBroadphaseCL, "kFindOverlappingPairs", &errNum, gridProg);
		drx3DAssert(errNum == CL_SUCCESS);
	}

	m_sorter = new b3RadixSort32CL(m_context, m_device, m_queue);
}
b3GpuGridBroadphase::~b3GpuGridBroadphase()
{
	clReleaseKernel(kCalcHashAABB);
	clReleaseKernel(kClearCellStart);
	clReleaseKernel(kFindCellStart);
	clReleaseKernel(kFindOverlappingPairs);
	clReleaseKernel(m_sap2Kernel);
	clReleaseKernel(m_copyAabbsKernel);

	delete m_sorter;
}

void b3GpuGridBroadphase::createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	b3SapAabb aabb;
	aabb.m_minVec = aabbMin;
	aabb.m_maxVec = aabbMax;
	aabb.m_minIndices[3] = userPtr;
	aabb.m_signedMaxIndices[3] = m_allAabbsCPU1.size();  //NOT userPtr;
	m_smallAabbsMappingCPU.push_back(m_allAabbsCPU1.size());

	m_allAabbsCPU1.push_back(aabb);
}
void b3GpuGridBroadphase::createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	b3SapAabb aabb;
	aabb.m_minVec = aabbMin;
	aabb.m_maxVec = aabbMax;
	aabb.m_minIndices[3] = userPtr;
	aabb.m_signedMaxIndices[3] = m_allAabbsCPU1.size();  //NOT userPtr;
	m_largeAabbsMappingCPU.push_back(m_allAabbsCPU1.size());

	m_allAabbsCPU1.push_back(aabb);
}

void b3GpuGridBroadphase::calculateOverlappingPairs(i32 maxPairs)
{
	D3_PROFILE("b3GpuGridBroadphase::calculateOverlappingPairs");

	if (0)
	{
		calculateOverlappingPairsHost(maxPairs);
		/*
		b3AlignedObjectArray<b3Int4> cpuPairs;
		m_gpuPairs.copyToHost(cpuPairs);
		printf("host m_gpuPairs.size()=%d\n",m_gpuPairs.size());
		for (i32 i=0;i<m_gpuPairs.size();i++)
		{
			printf("host pair %d = %d,%d\n",i,cpuPairs[i].x,cpuPairs[i].y);
		}
		*/
		return;
	}

	i32 numSmallAabbs = m_smallAabbsMappingGPU.size();

	b3OpenCLArray<i32> pairCount(m_context, m_queue);
	pairCount.push_back(0);
	m_gpuPairs.resize(maxPairs);  //numSmallAabbs*maxPairsPerBody);

	{
		i32 numLargeAabbs = m_largeAabbsMappingGPU.size();
		if (numLargeAabbs && numSmallAabbs)
		{
			D3_PROFILE("sap2Kernel");
			b3BufferInfoCL bInfo[] = {
				b3BufferInfoCL(m_allAabbsGPU1.getBufferCL()),
				b3BufferInfoCL(m_largeAabbsMappingGPU.getBufferCL()),
				b3BufferInfoCL(m_smallAabbsMappingGPU.getBufferCL()),
				b3BufferInfoCL(m_gpuPairs.getBufferCL()),
				b3BufferInfoCL(pairCount.getBufferCL())};
			b3LauncherCL launcher(m_queue, m_sap2Kernel, "m_sap2Kernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numLargeAabbs);
			launcher.setConst(numSmallAabbs);
			launcher.setConst(0);  //axis is not used
			launcher.setConst(maxPairs);
			//@todo: use actual maximum work item sizes of the device instead of hardcoded values
			launcher.launch2D(numLargeAabbs, numSmallAabbs, 4, 64);

			i32 numPairs = pairCount.at(0);

			if (numPairs > maxPairs)
			{
				drx3DError("Error running out of pairs: numPairs = %d, maxPairs = %d.\n", numPairs, maxPairs);
				numPairs = maxPairs;
			}
		}
	}

	if (numSmallAabbs)
	{
		D3_PROFILE("gridKernel");
		m_hashGpu.resize(numSmallAabbs);
		{
			D3_PROFILE("kCalcHashAABB");
			b3LauncherCL launch(m_queue, kCalcHashAABB, "kCalcHashAABB");
			launch.setConst(numSmallAabbs);
			launch.setBuffer(m_allAabbsGPU1.getBufferCL());
			launch.setBuffer(m_smallAabbsMappingGPU.getBufferCL());
			launch.setBuffer(m_hashGpu.getBufferCL());
			launch.setBuffer(this->m_paramsGPU.getBufferCL());
			launch.launch1D(numSmallAabbs);
		}

		m_sorter->execute(m_hashGpu);

		i32 numCells = this->m_paramsCPU.m_gridSize[0] * this->m_paramsCPU.m_gridSize[1] * this->m_paramsCPU.m_gridSize[2];
		m_cellStartGpu.resize(numCells);
		//b3AlignedObjectArray<i32 >			cellStartCpu;

		{
			D3_PROFILE("kClearCellStart");
			b3LauncherCL launch(m_queue, kClearCellStart, "kClearCellStart");
			launch.setConst(numCells);
			launch.setBuffer(m_cellStartGpu.getBufferCL());
			launch.launch1D(numCells);
			//m_cellStartGpu.copyToHost(cellStartCpu);
			//printf("??\n");
		}

		{
			D3_PROFILE("kFindCellStart");
			b3LauncherCL launch(m_queue, kFindCellStart, "kFindCellStart");
			launch.setConst(numSmallAabbs);
			launch.setBuffer(m_hashGpu.getBufferCL());
			launch.setBuffer(m_cellStartGpu.getBufferCL());
			launch.launch1D(numSmallAabbs);
			//m_cellStartGpu.copyToHost(cellStartCpu);
			//printf("??\n");
		}

		{
			D3_PROFILE("kFindOverlappingPairs");

			b3LauncherCL launch(m_queue, kFindOverlappingPairs, "kFindOverlappingPairs");
			launch.setConst(numSmallAabbs);
			launch.setBuffer(m_allAabbsGPU1.getBufferCL());
			launch.setBuffer(m_smallAabbsMappingGPU.getBufferCL());
			launch.setBuffer(m_hashGpu.getBufferCL());
			launch.setBuffer(m_cellStartGpu.getBufferCL());

			launch.setBuffer(m_paramsGPU.getBufferCL());
			//launch.setBuffer(0);
			launch.setBuffer(pairCount.getBufferCL());
			launch.setBuffer(m_gpuPairs.getBufferCL());

			launch.setConst(maxPairs);
			launch.launch1D(numSmallAabbs);

			i32 numPairs = pairCount.at(0);
			if (numPairs > maxPairs)
			{
				drx3DError("Error running out of pairs: numPairs = %d, maxPairs = %d.\n", numPairs, maxPairs);
				numPairs = maxPairs;
			}

			m_gpuPairs.resize(numPairs);

			if (0)
			{
				b3AlignedObjectArray<b3Int4> pairsCpu;
				m_gpuPairs.copyToHost(pairsCpu);

				i32 sz = m_gpuPairs.size();
				printf("m_gpuPairs.size()=%d\n", sz);
				for (i32 i = 0; i < m_gpuPairs.size(); i++)
				{
					printf("pair %d = %d,%d\n", i, pairsCpu[i].x, pairsCpu[i].y);
				}

				printf("?!?\n");
			}
		}
	}

	//calculateOverlappingPairsHost(maxPairs);
}
void b3GpuGridBroadphase::calculateOverlappingPairsHost(i32 maxPairs)
{
	m_hostPairs.resize(0);
	m_allAabbsGPU1.copyToHost(m_allAabbsCPU1);
	for (i32 i = 0; i < m_allAabbsCPU1.size(); i++)
	{
		for (i32 j = i + 1; j < m_allAabbsCPU1.size(); j++)
		{
			if (b3TestAabbAgainstAabb2(m_allAabbsCPU1[i].m_minVec, m_allAabbsCPU1[i].m_maxVec,
									   m_allAabbsCPU1[j].m_minVec, m_allAabbsCPU1[j].m_maxVec))
			{
				b3Int4 pair;
				i32 a = m_allAabbsCPU1[j].m_minIndices[3];
				i32 b = m_allAabbsCPU1[i].m_minIndices[3];
				if (a <= b)
				{
					pair.x = a;
					pair.y = b;  //store the original index in the unsorted aabb array
				}
				else
				{
					pair.x = b;
					pair.y = a;  //store the original index in the unsorted aabb array
				}

				if (m_hostPairs.size() < maxPairs)
				{
					m_hostPairs.push_back(pair);
				}
			}
		}
	}

	m_gpuPairs.copyFromHost(m_hostPairs);
}

//call writeAabbsToGpu after done making all changes (createProxy etc)
void b3GpuGridBroadphase::writeAabbsToGpu()
{
	m_allAabbsGPU1.copyFromHost(m_allAabbsCPU1);
	m_smallAabbsMappingGPU.copyFromHost(m_smallAabbsMappingCPU);
	m_largeAabbsMappingGPU.copyFromHost(m_largeAabbsMappingCPU);
}

cl_mem b3GpuGridBroadphase::getAabbBufferWS()
{
	return this->m_allAabbsGPU1.getBufferCL();
}
i32 b3GpuGridBroadphase::getNumOverlap()
{
	return m_gpuPairs.size();
}
cl_mem b3GpuGridBroadphase::getOverlappingPairBuffer()
{
	return m_gpuPairs.getBufferCL();
}

b3OpenCLArray<b3SapAabb>& b3GpuGridBroadphase::getAllAabbsGPU()
{
	return m_allAabbsGPU1;
}

b3AlignedObjectArray<b3SapAabb>& b3GpuGridBroadphase::getAllAabbsCPU()
{
	return m_allAabbsCPU1;
}

b3OpenCLArray<b3Int4>& b3GpuGridBroadphase::getOverlappingPairsGPU()
{
	return m_gpuPairs;
}
b3OpenCLArray<i32>& b3GpuGridBroadphase::getSmallAabbIndicesGPU()
{
	return m_smallAabbsMappingGPU;
}
b3OpenCLArray<i32>& b3GpuGridBroadphase::getLargeAabbIndicesGPU()
{
	return m_largeAabbsMappingGPU;
}
