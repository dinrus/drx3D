
bool searchIncremental3dSapOnGpu = true;
#include <limits.h>
#include <drx3D/Physics/Collision/BroadPhase/b3GpuSapBroadphase.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanFloat4CL.h>

#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/Physics/Collision/BroadPhase/kernels/sapKernels.h>

#include <drx3D/Maths/Linear/MinMax.h>

#define D3_BROADPHASE_SAP_PATH "drx3D/Physics/Collision/BroadPhase/kernels/sap.cl"

/*
	
 
	
	
	
 
	b3OpenCLArray<i32> m_pairCount;
 
 
	b3OpenCLArray<b3SapAabb>	m_allAabbsGPU;
	b3AlignedObjectArray<b3SapAabb>	m_allAabbsCPU;
 
	virtual b3OpenCLArray<b3SapAabb>&	getAllAabbsGPU()
	{
 return m_allAabbsGPU;
	}
	virtual b3AlignedObjectArray<b3SapAabb>&	getAllAabbsCPU()
	{
 return m_allAabbsCPU;
	}
 
	b3OpenCLArray<b3Vec3>	m_sum;
	b3OpenCLArray<b3Vec3>	m_sum2;
	b3OpenCLArray<b3Vec3>	m_dst;
 
	b3OpenCLArray<i32>	m_smallAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_smallAabbsMappingCPU;
 
	b3OpenCLArray<i32>	m_largeAabbsMappingGPU;
	b3AlignedObjectArray<i32> m_largeAabbsMappingCPU;
 
	
	b3OpenCLArray<b3Int4>		m_overlappingPairs;
 
	//temporary gpu work memory
	b3OpenCLArray<b3SortData>	m_gpuSmallSortData;
	b3OpenCLArray<b3SapAabb>	m_gpuSmallSortedAabbs;
 
	class b3PrefixScanFloat4CL*		m_prefixScanFloat4;
 */

b3GpuSapBroadphase::b3GpuSapBroadphase(cl_context ctx, cl_device_id device, cl_command_queue q, b3GpuSapKernelType kernelType)
	: m_context(ctx),
	  m_device(device),
	  m_queue(q),

	  m_objectMinMaxIndexGPUaxis0(ctx, q),
	  m_objectMinMaxIndexGPUaxis1(ctx, q),
	  m_objectMinMaxIndexGPUaxis2(ctx, q),
	  m_objectMinMaxIndexGPUaxis0prev(ctx, q),
	  m_objectMinMaxIndexGPUaxis1prev(ctx, q),
	  m_objectMinMaxIndexGPUaxis2prev(ctx, q),
	  m_sortedAxisGPU0(ctx, q),
	  m_sortedAxisGPU1(ctx, q),
	  m_sortedAxisGPU2(ctx, q),
	  m_sortedAxisGPU0prev(ctx, q),
	  m_sortedAxisGPU1prev(ctx, q),
	  m_sortedAxisGPU2prev(ctx, q),
	  m_addedHostPairsGPU(ctx, q),
	  m_removedHostPairsGPU(ctx, q),
	  m_addedCountGPU(ctx, q),
	  m_removedCountGPU(ctx, q),
	  m_currentBuffer(-1),
	  m_pairCount(ctx, q),
	  m_allAabbsGPU(ctx, q),
	  m_sum(ctx, q),
	  m_sum2(ctx, q),
	  m_dst(ctx, q),
	  m_smallAabbsMappingGPU(ctx, q),
	  m_largeAabbsMappingGPU(ctx, q),
	  m_overlappingPairs(ctx, q),
	  m_gpuSmallSortData(ctx, q),
	  m_gpuSmallSortedAabbs(ctx, q)
{
	tukk sapSrc = sapCL;

	cl_int errNum = 0;

	drx3DAssert(m_context);
	drx3DAssert(m_device);
	cl_program sapProg = b3OpenCLUtils::compileCLProgramFromString(m_context, m_device, sapSrc, &errNum, "", D3_BROADPHASE_SAP_PATH);
	drx3DAssert(errNum == CL_SUCCESS);

	drx3DAssert(errNum == CL_SUCCESS);
#ifndef __APPLE__
	m_prefixScanFloat4 = new b3PrefixScanFloat4CL(m_context, m_device, m_queue);
#else
	m_prefixScanFloat4 = 0;
#endif
	m_sapKernel = 0;

	switch (kernelType)
	{
		case D3_GPU_SAP_KERNEL_BRUTE_FORCE_CPU:
		{
			m_sapKernel = 0;
			break;
		}
		case D3_GPU_SAP_KERNEL_BRUTE_FORCE_GPU:
		{
			m_sapKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelBruteForce", &errNum, sapProg);
			break;
		}

		case D3_GPU_SAP_KERNEL_ORIGINAL:
		{
			m_sapKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelOriginal", &errNum, sapProg);
			break;
		}
		case D3_GPU_SAP_KERNEL_BARRIER:
		{
			m_sapKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelBarrier", &errNum, sapProg);
			break;
		}
		case D3_GPU_SAP_KERNEL_LOCAL_SHARED_MEMORY:
		{
			m_sapKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelLocalSharedMemory", &errNum, sapProg);
			break;
		}

		default:
		{
			m_sapKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelLocalSharedMemory", &errNum, sapProg);
			drx3DError("Unknown 3D GPU SAP provided, fallback to computePairsKernelLocalSharedMemory");
		}
	};

	m_sap2Kernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "computePairsKernelTwoArrays", &errNum, sapProg);
	drx3DAssert(errNum == CL_SUCCESS);

	m_prepareSumVarianceKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "prepareSumVarianceKernel", &errNum, sapProg);
	drx3DAssert(errNum == CL_SUCCESS);

	m_flipFloatKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "flipFloatKernel", &errNum, sapProg);

	m_copyAabbsKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "copyAabbsKernel", &errNum, sapProg);

	m_scatterKernel = b3OpenCLUtils::compileCLKernelFromString(m_context, m_device, sapSrc, "scatterKernel", &errNum, sapProg);

	m_sorter = new b3RadixSort32CL(m_context, m_device, m_queue);
}

b3GpuSapBroadphase::~b3GpuSapBroadphase()
{
	delete m_sorter;
	delete m_prefixScanFloat4;

	clReleaseKernel(m_scatterKernel);
	clReleaseKernel(m_flipFloatKernel);
	clReleaseKernel(m_copyAabbsKernel);
	clReleaseKernel(m_sapKernel);
	clReleaseKernel(m_sap2Kernel);
	clReleaseKernel(m_prepareSumVarianceKernel);
}

/// conservative test for overlap between two aabbs
static bool TestAabbAgainstAabb2(const b3Vec3& aabbMin1, const b3Vec3& aabbMax1,
								 const b3Vec3& aabbMin2, const b3Vec3& aabbMax2)
{
	bool overlap = true;
	overlap = (aabbMin1.getX() > aabbMax2.getX() || aabbMax1.getX() < aabbMin2.getX()) ? false : overlap;
	overlap = (aabbMin1.getZ() > aabbMax2.getZ() || aabbMax1.getZ() < aabbMin2.getZ()) ? false : overlap;
	overlap = (aabbMin1.getY() > aabbMax2.getY() || aabbMax1.getY() < aabbMin2.getY()) ? false : overlap;
	return overlap;
}

//http://stereopsis.com/radix.html
static u32 FloatFlip(float fl)
{
	u32 f = *(u32*)&fl;
	u32 mask = -(i32)(f >> 31) | 0x80000000;
	return f ^ mask;
};

void b3GpuSapBroadphase::init3dSap()
{
	if (m_currentBuffer < 0)
	{
		m_allAabbsGPU.copyToHost(m_allAabbsCPU);

		m_currentBuffer = 0;
		for (i32 axis = 0; axis < 3; axis++)
		{
			for (i32 buf = 0; buf < 2; buf++)
			{
				i32 totalNumAabbs = m_allAabbsCPU.size();
				i32 numEndPoints = 2 * totalNumAabbs;
				m_sortedAxisCPU[axis][buf].resize(numEndPoints);

				if (buf == m_currentBuffer)
				{
					for (i32 i = 0; i < totalNumAabbs; i++)
					{
						m_sortedAxisCPU[axis][buf][i * 2].m_key = FloatFlip(m_allAabbsCPU[i].m_min[axis]) - 1;
						m_sortedAxisCPU[axis][buf][i * 2].m_value = i * 2;
						m_sortedAxisCPU[axis][buf][i * 2 + 1].m_key = FloatFlip(m_allAabbsCPU[i].m_max[axis]) + 1;
						m_sortedAxisCPU[axis][buf][i * 2 + 1].m_value = i * 2 + 1;
					}
				}
			}
		}

		for (i32 axis = 0; axis < 3; axis++)
		{
			m_sorter->executeHost(m_sortedAxisCPU[axis][m_currentBuffer]);
		}

		for (i32 axis = 0; axis < 3; axis++)
		{
			//i32 totalNumAabbs = m_allAabbsCPU.size();
			i32 numEndPoints = m_sortedAxisCPU[axis][m_currentBuffer].size();
			m_objectMinMaxIndexCPU[axis][m_currentBuffer].resize(numEndPoints);
			for (i32 i = 0; i < numEndPoints; i++)
			{
				i32 destIndex = m_sortedAxisCPU[axis][m_currentBuffer][i].m_value;
				i32 newDest = destIndex / 2;
				if (destIndex & 1)
				{
					m_objectMinMaxIndexCPU[axis][m_currentBuffer][newDest].y = i;
				}
				else
				{
					m_objectMinMaxIndexCPU[axis][m_currentBuffer][newDest].x = i;
				}
			}
		}
	}
}

static bool b3PairCmp(const b3Int4& p, const b3Int4& q)
{
	return ((p.x < q.x) || ((p.x == q.x) && (p.y < q.y)));
}

static bool operator==(const b3Int4& a, const b3Int4& b)
{
	return a.x == b.x && a.y == b.y;
};

static bool operator<(const b3Int4& a, const b3Int4& b)
{
	return a.x < b.x || (a.x == b.x && a.y < b.y);
};

static bool operator>(const b3Int4& a, const b3Int4& b)
{
	return a.x > b.x || (a.x == b.x && a.y > b.y);
};

b3AlignedObjectArray<b3Int4> addedHostPairs;
b3AlignedObjectArray<b3Int4> removedHostPairs;

b3AlignedObjectArray<b3SapAabb> preAabbs;

void b3GpuSapBroadphase::calculateOverlappingPairsHostIncremental3Sap()
{
	//static i32 framepje = 0;
	//printf("framepje=%d\n",framepje++);

	D3_PROFILE("calculateOverlappingPairsHostIncremental3Sap");

	addedHostPairs.resize(0);
	removedHostPairs.resize(0);

	drx3DAssert(m_currentBuffer >= 0);

	{
		preAabbs.resize(m_allAabbsCPU.size());
		for (i32 i = 0; i < preAabbs.size(); i++)
		{
			preAabbs[i] = m_allAabbsCPU[i];
		}
	}

	if (m_currentBuffer < 0)
		return;
	{
		D3_PROFILE("m_allAabbsGPU.copyToHost");
		m_allAabbsGPU.copyToHost(m_allAabbsCPU);
	}

	b3AlignedObjectArray<b3Int4> allPairs;
	{
		D3_PROFILE("m_overlappingPairs.copyToHost");
		m_overlappingPairs.copyToHost(allPairs);
	}
	if (0)
	{
		{
			printf("ab[40].min=%f,%f,%f,ab[40].max=%f,%f,%f\n",
				   m_allAabbsCPU[40].m_min[0], m_allAabbsCPU[40].m_min[1], m_allAabbsCPU[40].m_min[2],
				   m_allAabbsCPU[40].m_max[0], m_allAabbsCPU[40].m_max[1], m_allAabbsCPU[40].m_max[2]);
		}

		{
			printf("ab[53].min=%f,%f,%f,ab[53].max=%f,%f,%f\n",
				   m_allAabbsCPU[53].m_min[0], m_allAabbsCPU[53].m_min[1], m_allAabbsCPU[53].m_min[2],
				   m_allAabbsCPU[53].m_max[0], m_allAabbsCPU[53].m_max[1], m_allAabbsCPU[53].m_max[2]);
		}

		{
			b3Int4 newPair;
			newPair.x = 40;
			newPair.y = 53;
			i32 index = allPairs.findBinarySearch(newPair);
			printf("hasPair(40,53)=%d out of %d\n", index, allPairs.size());

			{
				i32 overlap = TestAabbAgainstAabb2((const b3Vec3&)m_allAabbsCPU[40].m_min, (const b3Vec3&)m_allAabbsCPU[40].m_max, (const b3Vec3&)m_allAabbsCPU[53].m_min, (const b3Vec3&)m_allAabbsCPU[53].m_max);
				printf("overlap=%d\n", overlap);
			}

			if (preAabbs.size())
			{
				i32 prevOverlap = TestAabbAgainstAabb2((const b3Vec3&)preAabbs[40].m_min, (const b3Vec3&)preAabbs[40].m_max, (const b3Vec3&)preAabbs[53].m_min, (const b3Vec3&)preAabbs[53].m_max);
				printf("prevoverlap=%d\n", prevOverlap);
			}
			else
			{
				printf("unknown prevoverlap\n");
			}
		}
	}

	if (0)
	{
		for (i32 i = 0; i < m_allAabbsCPU.size(); i++)
		{
			//printf("aabb[%d] min=%f,%f,%f max=%f,%f,%f\n",i,m_allAabbsCPU[i].m_min[0],m_allAabbsCPU[i].m_min[1],m_allAabbsCPU[i].m_min[2],			m_allAabbsCPU[i].m_max[0],m_allAabbsCPU[i].m_max[1],m_allAabbsCPU[i].m_max[2]);
		}

		for (i32 axis = 0; axis < 3; axis++)
		{
			for (i32 buf = 0; buf < 2; buf++)
			{
				drx3DAssert(m_sortedAxisCPU[axis][buf].size() == m_allAabbsCPU.size() * 2);
			}
		}
	}

	m_currentBuffer = 1 - m_currentBuffer;

	i32 totalNumAabbs = m_allAabbsCPU.size();

	{
		D3_PROFILE("assign m_sortedAxisCPU(FloatFlip)");
		for (i32 i = 0; i < totalNumAabbs; i++)
		{
			u32 keyMin[3];
			u32 keyMax[3];
			for (i32 axis = 0; axis < 3; axis++)
			{
				float vmin = m_allAabbsCPU[i].m_min[axis];
				float vmax = m_allAabbsCPU[i].m_max[axis];
				keyMin[axis] = FloatFlip(vmin);
				keyMax[axis] = FloatFlip(vmax);

				m_sortedAxisCPU[axis][m_currentBuffer][i * 2].m_key = keyMin[axis] - 1;
				m_sortedAxisCPU[axis][m_currentBuffer][i * 2].m_value = i * 2;
				m_sortedAxisCPU[axis][m_currentBuffer][i * 2 + 1].m_key = keyMax[axis] + 1;
				m_sortedAxisCPU[axis][m_currentBuffer][i * 2 + 1].m_value = i * 2 + 1;
			}
			//printf("aabb[%d] min=%u,%u,%u max %u,%u,%u\n", i,keyMin[0],keyMin[1],keyMin[2],keyMax[0],keyMax[1],keyMax[2]);
		}
	}

	{
		D3_PROFILE("sort m_sortedAxisCPU");
		for (i32 axis = 0; axis < 3; axis++)
			m_sorter->executeHost(m_sortedAxisCPU[axis][m_currentBuffer]);
	}

#if 0
	if (0)
	{
		for (i32 axis=0;axis<3;axis++)
		{
			//printf("axis %d\n",axis);
			for (i32 i=0;i<m_sortedAxisCPU[axis][m_currentBuffer].size();i++)
			{
				//i32 key = m_sortedAxisCPU[axis][m_currentBuffer][i].m_key;
				//i32 value = m_sortedAxisCPU[axis][m_currentBuffer][i].m_value;
				//printf("[%d]=%d\n",i,value);
			}

		}
	}
#endif

	{
		D3_PROFILE("assign m_objectMinMaxIndexCPU");
		for (i32 axis = 0; axis < 3; axis++)
		{
			i32 totalNumAabbs = m_allAabbsCPU.size();
			i32 numEndPoints = m_sortedAxisCPU[axis][m_currentBuffer].size();
			m_objectMinMaxIndexCPU[axis][m_currentBuffer].resize(totalNumAabbs);
			for (i32 i = 0; i < numEndPoints; i++)
			{
				i32 destIndex = m_sortedAxisCPU[axis][m_currentBuffer][i].m_value;
				i32 newDest = destIndex / 2;
				if (destIndex & 1)
				{
					m_objectMinMaxIndexCPU[axis][m_currentBuffer][newDest].y = i;
				}
				else
				{
					m_objectMinMaxIndexCPU[axis][m_currentBuffer][newDest].x = i;
				}
			}
		}
	}

#if 0
	if (0)
	{	
		printf("==========================\n");
		for (i32 axis=0;axis<3;axis++)
		{
			u32 curMinIndex40 = m_objectMinMaxIndexCPU[axis][m_currentBuffer][40].x;
			u32 curMaxIndex40 = m_objectMinMaxIndexCPU[axis][m_currentBuffer][40].y;
			u32 prevMaxIndex40 = m_objectMinMaxIndexCPU[axis][1-m_currentBuffer][40].y;
			u32 prevMinIndex40 = m_objectMinMaxIndexCPU[axis][1-m_currentBuffer][40].x;

			i32 dmin40 = curMinIndex40 - prevMinIndex40;
			i32 dmax40 = curMinIndex40 - prevMinIndex40;
			printf("axis %d curMinIndex40=%d prevMinIndex40=%d\n",axis,curMinIndex40, prevMinIndex40);
			printf("axis %d curMaxIndex40=%d prevMaxIndex40=%d\n",axis,curMaxIndex40, prevMaxIndex40);
		}
		printf(".........................\n");
		for (i32 axis=0;axis<3;axis++)
		{
			u32 curMinIndex53 = m_objectMinMaxIndexCPU[axis][m_currentBuffer][53].x;
			u32 curMaxIndex53 = m_objectMinMaxIndexCPU[axis][m_currentBuffer][53].y;
			u32 prevMaxIndex53 = m_objectMinMaxIndexCPU[axis][1-m_currentBuffer][53].y;
			u32 prevMinIndex53 = m_objectMinMaxIndexCPU[axis][1-m_currentBuffer][53].x;

			i32 dmin40 = curMinIndex53 - prevMinIndex53;
			i32 dmax40 = curMinIndex53 - prevMinIndex53;
			printf("axis %d curMinIndex53=%d prevMinIndex53=%d\n",axis,curMinIndex53, prevMinIndex53);
			printf("axis %d curMaxIndex53=%d prevMaxIndex53=%d\n",axis,curMaxIndex53, prevMaxIndex53);
		}

	}
#endif

	i32 a = m_objectMinMaxIndexCPU[0][m_currentBuffer].size();
	i32 b = m_objectMinMaxIndexCPU[1][m_currentBuffer].size();
	i32 c = m_objectMinMaxIndexCPU[2][m_currentBuffer].size();
	drx3DAssert(a == b);
	drx3DAssert(b == c);
	/*
	if (searchIncremental3dSapOnGpu)
	{
		D3_PROFILE("computePairsIncremental3dSapKernelGPU");
		i32 numObjects = m_objectMinMaxIndexCPU[0][m_currentBuffer].size();
		i32 maxCapacity = 1024*1024;
		{
			D3_PROFILE("copy from host");
			m_objectMinMaxIndexGPUaxis0.copyFromHost(m_objectMinMaxIndexCPU[0][m_currentBuffer]);
			m_objectMinMaxIndexGPUaxis1.copyFromHost(m_objectMinMaxIndexCPU[1][m_currentBuffer]);
			m_objectMinMaxIndexGPUaxis2.copyFromHost(m_objectMinMaxIndexCPU[2][m_currentBuffer]);
			m_objectMinMaxIndexGPUaxis0prev.copyFromHost(m_objectMinMaxIndexCPU[0][1-m_currentBuffer]);
			m_objectMinMaxIndexGPUaxis1prev.copyFromHost(m_objectMinMaxIndexCPU[1][1-m_currentBuffer]);
			m_objectMinMaxIndexGPUaxis2prev.copyFromHost(m_objectMinMaxIndexCPU[2][1-m_currentBuffer]);

			m_sortedAxisGPU0.copyFromHost(m_sortedAxisCPU[0][m_currentBuffer]);
			m_sortedAxisGPU1.copyFromHost(m_sortedAxisCPU[1][m_currentBuffer]);
			m_sortedAxisGPU2.copyFromHost(m_sortedAxisCPU[2][m_currentBuffer]);
			m_sortedAxisGPU0prev.copyFromHost(m_sortedAxisCPU[0][1-m_currentBuffer]);
			m_sortedAxisGPU1prev.copyFromHost(m_sortedAxisCPU[1][1-m_currentBuffer]);
			m_sortedAxisGPU2prev.copyFromHost(m_sortedAxisCPU[2][1-m_currentBuffer]);

		
			m_addedHostPairsGPU.resize(maxCapacity);
			m_removedHostPairsGPU.resize(maxCapacity);

			m_addedCountGPU.resize(0);
			m_addedCountGPU.push_back(0);
			m_removedCountGPU.resize(0);
			m_removedCountGPU.push_back(0);
		}

		{
			D3_PROFILE("launch1D");
			b3LauncherCL launcher(m_queue,  m_computePairsIncremental3dSapKernel,"m_computePairsIncremental3dSapKernel");
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis0.getBufferCL());
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis1.getBufferCL());
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis2.getBufferCL());
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis0prev.getBufferCL());
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis1prev.getBufferCL());
			launcher.setBuffer(m_objectMinMaxIndexGPUaxis2prev.getBufferCL());

			launcher.setBuffer(m_sortedAxisGPU0.getBufferCL());
			launcher.setBuffer(m_sortedAxisGPU1.getBufferCL());
			launcher.setBuffer(m_sortedAxisGPU2.getBufferCL());
			launcher.setBuffer(m_sortedAxisGPU0prev.getBufferCL());
			launcher.setBuffer(m_sortedAxisGPU1prev.getBufferCL());
			launcher.setBuffer(m_sortedAxisGPU2prev.getBufferCL());

		
			launcher.setBuffer(m_addedHostPairsGPU.getBufferCL());
			launcher.setBuffer(m_removedHostPairsGPU.getBufferCL());
			launcher.setBuffer(m_addedCountGPU.getBufferCL());
			launcher.setBuffer(m_removedCountGPU.getBufferCL());
			launcher.setConst(maxCapacity);
			launcher.setConst( numObjects);
			launcher.launch1D( numObjects);
			clFinish(m_queue);
		}

		{
			D3_PROFILE("copy to host");
			i32 addedCountGPU = m_addedCountGPU.at(0);
			m_addedHostPairsGPU.resize(addedCountGPU);
			m_addedHostPairsGPU.copyToHost(addedHostPairs);

			//printf("addedCountGPU=%d\n",addedCountGPU);
			i32 removedCountGPU = m_removedCountGPU.at(0);
			m_removedHostPairsGPU.resize(removedCountGPU);
			m_removedHostPairsGPU.copyToHost(removedHostPairs);
			//printf("removedCountGPU=%d\n",removedCountGPU);

		}



	} 
	else
	*/
	{
		i32 numObjects = m_objectMinMaxIndexCPU[0][m_currentBuffer].size();

		D3_PROFILE("actual search");
		for (i32 i = 0; i < numObjects; i++)
		{
			//i32 numObjects = m_objectMinMaxIndexCPU[axis][m_currentBuffer].size();
			//i32 checkObjects[]={40,53};
			//i32 numCheckObjects = sizeof(checkObjects)/sizeof(i32);

			//for (i32 a=0;a<numCheckObjects ;a++)

			for (i32 axis = 0; axis < 3; axis++)
			{
				//i32 i = checkObjects[a];

				u32 curMinIndex = m_objectMinMaxIndexCPU[axis][m_currentBuffer][i].x;
				u32 curMaxIndex = m_objectMinMaxIndexCPU[axis][m_currentBuffer][i].y;
				u32 prevMinIndex = m_objectMinMaxIndexCPU[axis][1 - m_currentBuffer][i].x;
				i32 dmin = curMinIndex - prevMinIndex;

				u32 prevMaxIndex = m_objectMinMaxIndexCPU[axis][1 - m_currentBuffer][i].y;

				i32 dmax = curMaxIndex - prevMaxIndex;
				if (dmin != 0)
				{
					//printf("for object %d, dmin=%d\n",i,dmin);
				}
				if (dmax != 0)
				{
					//printf("for object %d, dmax=%d\n",i,dmax);
				}
				for (i32 otherbuffer = 0; otherbuffer < 2; otherbuffer++)
				{
					if (dmin != 0)
					{
						i32 stepMin = dmin < 0 ? -1 : 1;
						for (i32 j = prevMinIndex; j != curMinIndex; j += stepMin)
						{
							i32 otherIndex2 = m_sortedAxisCPU[axis][otherbuffer][j].y;
							i32 otherIndex = otherIndex2 / 2;
							if (otherIndex != i)
							{
								bool otherIsMax = ((otherIndex2 & 1) != 0);

								if (otherIsMax)
								{
									//bool overlap = TestAabbAgainstAabb2((const b3Vec3&)m_allAabbsCPU[i].m_min, (const b3Vec3&)m_allAabbsCPU[i].m_max,(const b3Vec3&)m_allAabbsCPU[otherIndex].m_min,(const b3Vec3&)m_allAabbsCPU[otherIndex].m_max);
									//bool prevOverlap = TestAabbAgainstAabb2((const b3Vec3&)preAabbs[i].m_min, (const b3Vec3&)preAabbs[i].m_max,(const b3Vec3&)preAabbs[otherIndex].m_min,(const b3Vec3&)preAabbs[otherIndex].m_max);

									bool overlap = true;

									for (i32 ax = 0; ax < 3; ax++)
									{
										if ((m_objectMinMaxIndexCPU[ax][m_currentBuffer][i].x > m_objectMinMaxIndexCPU[ax][m_currentBuffer][otherIndex].y) ||
											(m_objectMinMaxIndexCPU[ax][m_currentBuffer][i].y < m_objectMinMaxIndexCPU[ax][m_currentBuffer][otherIndex].x))
											overlap = false;
									}

									//	drx3DAssert(overlap2==overlap);

									bool prevOverlap = true;

									for (i32 ax = 0; ax < 3; ax++)
									{
										if ((m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][i].x > m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][otherIndex].y) ||
											(m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][i].y < m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][otherIndex].x))
											prevOverlap = false;
									}

									//drx3DAssert(overlap==overlap2);

									if (dmin < 0)
									{
										if (overlap && !prevOverlap)
										{
											//add a pair
											b3Int4 newPair;
											if (i <= otherIndex)
											{
												newPair.x = i;
												newPair.y = otherIndex;
											}
											else
											{
												newPair.x = otherIndex;
												newPair.y = i;
											}
											addedHostPairs.push_back(newPair);
										}
									}
									else
									{
										if (!overlap && prevOverlap)
										{
											//remove a pair
											b3Int4 removedPair;
											if (i <= otherIndex)
											{
												removedPair.x = i;
												removedPair.y = otherIndex;
											}
											else
											{
												removedPair.x = otherIndex;
												removedPair.y = i;
											}
											removedHostPairs.push_back(removedPair);
										}
									}  //otherisMax
								}      //if (dmin<0)
							}          //if (otherIndex!=i)
						}              //for (i32 j=
					}

					if (dmax != 0)
					{
						i32 stepMax = dmax < 0 ? -1 : 1;
						for (i32 j = prevMaxIndex; j != curMaxIndex; j += stepMax)
						{
							i32 otherIndex2 = m_sortedAxisCPU[axis][otherbuffer][j].y;
							i32 otherIndex = otherIndex2 / 2;
							if (otherIndex != i)
							{
								//bool otherIsMin = ((otherIndex2&1)==0);
								//if (otherIsMin)
								{
									//bool overlap = TestAabbAgainstAabb2((const b3Vec3&)m_allAabbsCPU[i].m_min, (const b3Vec3&)m_allAabbsCPU[i].m_max,(const b3Vec3&)m_allAabbsCPU[otherIndex].m_min,(const b3Vec3&)m_allAabbsCPU[otherIndex].m_max);
									//bool prevOverlap = TestAabbAgainstAabb2((const b3Vec3&)preAabbs[i].m_min, (const b3Vec3&)preAabbs[i].m_max,(const b3Vec3&)preAabbs[otherIndex].m_min,(const b3Vec3&)preAabbs[otherIndex].m_max);

									bool overlap = true;

									for (i32 ax = 0; ax < 3; ax++)
									{
										if ((m_objectMinMaxIndexCPU[ax][m_currentBuffer][i].x > m_objectMinMaxIndexCPU[ax][m_currentBuffer][otherIndex].y) ||
											(m_objectMinMaxIndexCPU[ax][m_currentBuffer][i].y < m_objectMinMaxIndexCPU[ax][m_currentBuffer][otherIndex].x))
											overlap = false;
									}
									//drx3DAssert(overlap2==overlap);

									bool prevOverlap = true;

									for (i32 ax = 0; ax < 3; ax++)
									{
										if ((m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][i].x > m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][otherIndex].y) ||
											(m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][i].y < m_objectMinMaxIndexCPU[ax][1 - m_currentBuffer][otherIndex].x))
											prevOverlap = false;
									}

									if (dmax > 0)
									{
										if (overlap && !prevOverlap)
										{
											//add a pair
											b3Int4 newPair;
											if (i <= otherIndex)
											{
												newPair.x = i;
												newPair.y = otherIndex;
											}
											else
											{
												newPair.x = otherIndex;
												newPair.y = i;
											}
											addedHostPairs.push_back(newPair);
										}
									}
									else
									{
										if (!overlap && prevOverlap)
										{
											//if (otherIndex2&1==0) -> min?
											//remove a pair
											b3Int4 removedPair;
											if (i <= otherIndex)
											{
												removedPair.x = i;
												removedPair.y = otherIndex;
											}
											else
											{
												removedPair.x = otherIndex;
												removedPair.y = i;
											}
											removedHostPairs.push_back(removedPair);
										}
									}

								}  //if (dmin<0)
							}      //if (otherIndex!=i)
						}          //for (i32 j=
					}
				}  //for (i32 otherbuffer
			}      //for (i32 axis=0;
		}          //for (i32 i=0;i<numObjects
	}

	//remove duplicates and add/remove then to existing m_overlappingPairs

	{
		{
			D3_PROFILE("sort allPairs");
			allPairs.quickSort(b3PairCmp);
		}
		{
			D3_PROFILE("sort addedHostPairs");
			addedHostPairs.quickSort(b3PairCmp);
		}
		{
			D3_PROFILE("sort removedHostPairs");
			removedHostPairs.quickSort(b3PairCmp);
		}
	}

	b3Int4 prevPair;
	prevPair.x = -1;
	prevPair.y = -1;

	i32 uniqueRemovedPairs = 0;

	b3AlignedObjectArray<i32> removedPositions;

	{
		D3_PROFILE("actual removing");
		for (i32 i = 0; i < removedHostPairs.size(); i++)
		{
			b3Int4 removedPair = removedHostPairs[i];
			if ((removedPair.x != prevPair.x) || (removedPair.y != prevPair.y))
			{
				i32 index1 = allPairs.findBinarySearch(removedPair);

				//#ifdef _DEBUG

				i32 index2 = allPairs.findLinearSearch(removedPair);
				drx3DAssert(index1 == index2);

				//drx3DAssert(index1!=allPairs.size());
				if (index1 < allPairs.size())
				//#endif//_DEBUG
				{
					uniqueRemovedPairs++;
					removedPositions.push_back(index1);
					{
						//printf("framepje(%d) remove pair(%d):%d,%d\n",framepje,i,removedPair.x,removedPair.y);
					}
				}
			}
			prevPair = removedPair;
		}

		if (uniqueRemovedPairs)
		{
			for (i32 i = 0; i < removedPositions.size(); i++)
			{
				allPairs[removedPositions[i]].x = INT_MAX;
				allPairs[removedPositions[i]].y = INT_MAX;
			}
			allPairs.quickSort(b3PairCmp);
			allPairs.resize(allPairs.size() - uniqueRemovedPairs);
		}
	}
	//if (uniqueRemovedPairs)
	//	printf("uniqueRemovedPairs=%d\n",uniqueRemovedPairs);
	//printf("removedHostPairs.size = %d\n",removedHostPairs.size());

	prevPair.x = -1;
	prevPair.y = -1;

	i32 uniqueAddedPairs = 0;
	b3AlignedObjectArray<b3Int4> actualAddedPairs;

	{
		D3_PROFILE("actual adding");
		for (i32 i = 0; i < addedHostPairs.size(); i++)
		{
			b3Int4 newPair = addedHostPairs[i];
			if ((newPair.x != prevPair.x) || (newPair.y != prevPair.y))
			{
				//#ifdef _DEBUG
				i32 index1 = allPairs.findBinarySearch(newPair);

				i32 index2 = allPairs.findLinearSearch(newPair);
				drx3DAssert(index1 == index2);

				drx3DAssert(index1 == allPairs.size());
				if (index1 != allPairs.size())
				{
					printf("??\n");
				}

				if (index1 == allPairs.size())
				//#endif //_DEBUG
				{
					uniqueAddedPairs++;
					actualAddedPairs.push_back(newPair);
				}
			}
			prevPair = newPair;
		}
		for (i32 i = 0; i < actualAddedPairs.size(); i++)
		{
			//printf("framepje (%d), new pair(%d):%d,%d\n",framepje,i,actualAddedPairs[i].x,actualAddedPairs[i].y);
			allPairs.push_back(actualAddedPairs[i]);
		}
	}

	//if (uniqueAddedPairs)
	//	printf("uniqueAddedPairs=%d\n", uniqueAddedPairs);

	{
		D3_PROFILE("m_overlappingPairs.copyFromHost");
		m_overlappingPairs.copyFromHost(allPairs);
	}
}

void b3GpuSapBroadphase::calculateOverlappingPairsHost(i32 maxPairs)
{
	//test
	//	if (m_currentBuffer>=0)
	//	return calculateOverlappingPairsHostIncremental3Sap();

	drx3DAssert(m_allAabbsCPU.size() == m_allAabbsGPU.size());
	m_allAabbsGPU.copyToHost(m_allAabbsCPU);

	i32 axis = 0;
	{
		D3_PROFILE("CPU compute best variance axis");
		b3Vec3 s = b3MakeVector3(0, 0, 0), s2 = b3MakeVector3(0, 0, 0);
		i32 numRigidBodies = m_smallAabbsMappingCPU.size();

		for (i32 i = 0; i < numRigidBodies; i++)
		{
			b3SapAabb aabb = this->m_allAabbsCPU[m_smallAabbsMappingCPU[i]];

			b3Vec3 maxAabb = b3MakeVector3(aabb.m_max[0], aabb.m_max[1], aabb.m_max[2]);
			b3Vec3 minAabb = b3MakeVector3(aabb.m_min[0], aabb.m_min[1], aabb.m_min[2]);
			b3Vec3 centerAabb = (maxAabb + minAabb) * 0.5f;

			s += centerAabb;
			s2 += centerAabb * centerAabb;
		}
		b3Vec3 v = s2 - (s * s) / (float)numRigidBodies;

		if (v[1] > v[0])
			axis = 1;
		if (v[2] > v[axis])
			axis = 2;
	}

	b3AlignedObjectArray<b3Int4> hostPairs;

	{
		i32 numSmallAabbs = m_smallAabbsMappingCPU.size();
		for (i32 i = 0; i < numSmallAabbs; i++)
		{
			b3SapAabb smallAabbi = m_allAabbsCPU[m_smallAabbsMappingCPU[i]];
			//float reference = smallAabbi.m_max[axis];

			for (i32 j = i + 1; j < numSmallAabbs; j++)
			{
				b3SapAabb smallAabbj = m_allAabbsCPU[m_smallAabbsMappingCPU[j]];

				if (TestAabbAgainstAabb2((b3Vec3&)smallAabbi.m_min, (b3Vec3&)smallAabbi.m_max,
										 (b3Vec3&)smallAabbj.m_min, (b3Vec3&)smallAabbj.m_max))
				{
					b3Int4 pair;
					i32 a = smallAabbi.m_minIndices[3];
					i32 b = smallAabbj.m_minIndices[3];
					if (a <= b)
					{
						pair.x = a;  //store the original index in the unsorted aabb array
						pair.y = b;
					}
					else
					{
						pair.x = b;  //store the original index in the unsorted aabb array
						pair.y = a;
					}
					hostPairs.push_back(pair);
				}
			}
		}
	}

	{
		i32 numSmallAabbs = m_smallAabbsMappingCPU.size();
		for (i32 i = 0; i < numSmallAabbs; i++)
		{
			b3SapAabb smallAabbi = m_allAabbsCPU[m_smallAabbsMappingCPU[i]];

			//float reference = smallAabbi.m_max[axis];
			i32 numLargeAabbs = m_largeAabbsMappingCPU.size();

			for (i32 j = 0; j < numLargeAabbs; j++)
			{
				b3SapAabb largeAabbj = m_allAabbsCPU[m_largeAabbsMappingCPU[j]];
				if (TestAabbAgainstAabb2((b3Vec3&)smallAabbi.m_min, (b3Vec3&)smallAabbi.m_max,
										 (b3Vec3&)largeAabbj.m_min, (b3Vec3&)largeAabbj.m_max))
				{
					b3Int4 pair;
					i32 a = largeAabbj.m_minIndices[3];
					i32 b = smallAabbi.m_minIndices[3];
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

					hostPairs.push_back(pair);
				}
			}
		}
	}

	if (hostPairs.size() > maxPairs)
	{
		hostPairs.resize(maxPairs);
	}

	if (hostPairs.size())
	{
		m_overlappingPairs.copyFromHost(hostPairs);
	}
	else
	{
		m_overlappingPairs.resize(0);
	}

	//init3dSap();
}

void b3GpuSapBroadphase::reset()
{
	m_allAabbsGPU.resize(0);
	m_allAabbsCPU.resize(0);

	m_smallAabbsMappingGPU.resize(0);
	m_smallAabbsMappingCPU.resize(0);

	m_pairCount.resize(0);

	m_largeAabbsMappingGPU.resize(0);
	m_largeAabbsMappingCPU.resize(0);
}

void b3GpuSapBroadphase::calculateOverlappingPairs(i32 maxPairs)
{
	if (m_sapKernel == 0)
	{
		calculateOverlappingPairsHost(maxPairs);
		return;
	}

	//if (m_currentBuffer>=0)
	//	return calculateOverlappingPairsHostIncremental3Sap();

	//calculateOverlappingPairsHost(maxPairs);

	D3_PROFILE("GPU 1-axis SAP calculateOverlappingPairs");

	i32 axis = 0;

	{
		//bool syncOnHost = false;

		i32 numSmallAabbs = m_smallAabbsMappingCPU.size();
		if (m_prefixScanFloat4 && numSmallAabbs)
		{
			D3_PROFILE("GPU compute best variance axis");

			if (m_dst.size() != (numSmallAabbs + 1))
			{
				m_dst.resize(numSmallAabbs + 128);
				m_sum.resize(numSmallAabbs + 128);
				m_sum2.resize(numSmallAabbs + 128);
				m_sum.at(numSmallAabbs) = b3MakeVector3(0, 0, 0);   //slow?
				m_sum2.at(numSmallAabbs) = b3MakeVector3(0, 0, 0);  //slow?
			}

			b3LauncherCL launcher(m_queue, m_prepareSumVarianceKernel, "m_prepareSumVarianceKernel");
			launcher.setBuffer(m_allAabbsGPU.getBufferCL());

			launcher.setBuffer(m_smallAabbsMappingGPU.getBufferCL());
			launcher.setBuffer(m_sum.getBufferCL());
			launcher.setBuffer(m_sum2.getBufferCL());
			launcher.setConst(numSmallAabbs);
			i32 num = numSmallAabbs;
			launcher.launch1D(num);

			b3Vec3 s;
			b3Vec3 s2;
			m_prefixScanFloat4->execute(m_sum, m_dst, numSmallAabbs + 1, &s);
			m_prefixScanFloat4->execute(m_sum2, m_dst, numSmallAabbs + 1, &s2);

			b3Vec3 v = s2 - (s * s) / (float)numSmallAabbs;

			if (v[1] > v[0])
				axis = 1;
			if (v[2] > v[axis])
				axis = 2;
		}

		m_gpuSmallSortData.resize(numSmallAabbs);

#if 1
		if (m_smallAabbsMappingGPU.size())
		{
			D3_PROFILE("flipFloatKernel");
			b3BufferInfoCL bInfo[] = {
				b3BufferInfoCL(m_allAabbsGPU.getBufferCL(), true),
				b3BufferInfoCL(m_smallAabbsMappingGPU.getBufferCL(), true),
				b3BufferInfoCL(m_gpuSmallSortData.getBufferCL())};
			b3LauncherCL launcher(m_queue, m_flipFloatKernel, "m_flipFloatKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numSmallAabbs);
			launcher.setConst(axis);

			i32 num = numSmallAabbs;
			launcher.launch1D(num);
			clFinish(m_queue);
		}

		if (m_gpuSmallSortData.size())
		{
			D3_PROFILE("gpu radix sort");
			m_sorter->execute(m_gpuSmallSortData);
			clFinish(m_queue);
		}

		m_gpuSmallSortedAabbs.resize(numSmallAabbs);
		if (numSmallAabbs)
		{
			D3_PROFILE("scatterKernel");

			b3BufferInfoCL bInfo[] = {
				b3BufferInfoCL(m_allAabbsGPU.getBufferCL(), true),
				b3BufferInfoCL(m_smallAabbsMappingGPU.getBufferCL(), true),
				b3BufferInfoCL(m_gpuSmallSortData.getBufferCL(), true),
				b3BufferInfoCL(m_gpuSmallSortedAabbs.getBufferCL())};
			b3LauncherCL launcher(m_queue, m_scatterKernel, "m_scatterKernel ");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numSmallAabbs);
			i32 num = numSmallAabbs;
			launcher.launch1D(num);
			clFinish(m_queue);
		}

		m_overlappingPairs.resize(maxPairs);

		m_pairCount.resize(0);
		m_pairCount.push_back(0);
		i32 numPairs = 0;

		{
			i32 numLargeAabbs = m_largeAabbsMappingGPU.size();
			if (numLargeAabbs && numSmallAabbs)
			{
				//@todo
				D3_PROFILE("sap2Kernel");
				b3BufferInfoCL bInfo[] = {
					b3BufferInfoCL(m_allAabbsGPU.getBufferCL()),
					b3BufferInfoCL(m_largeAabbsMappingGPU.getBufferCL()),
					b3BufferInfoCL(m_smallAabbsMappingGPU.getBufferCL()),
					b3BufferInfoCL(m_overlappingPairs.getBufferCL()),
					b3BufferInfoCL(m_pairCount.getBufferCL())};
				b3LauncherCL launcher(m_queue, m_sap2Kernel, "m_sap2Kernel");
				launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
				launcher.setConst(numLargeAabbs);
				launcher.setConst(numSmallAabbs);
				launcher.setConst(axis);
				launcher.setConst(maxPairs);
				//@todo: use actual maximum work item sizes of the device instead of hardcoded values
				launcher.launch2D(numLargeAabbs, numSmallAabbs, 4, 64);

				numPairs = m_pairCount.at(0);
				if (numPairs > maxPairs)
				{
					drx3DError("Error running out of pairs: numPairs = %d, maxPairs = %d.\n", numPairs, maxPairs);
					numPairs = maxPairs;
				}
			}
		}
		if (m_gpuSmallSortedAabbs.size())
		{
			D3_PROFILE("sapKernel");
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(m_gpuSmallSortedAabbs.getBufferCL()), b3BufferInfoCL(m_overlappingPairs.getBufferCL()), b3BufferInfoCL(m_pairCount.getBufferCL())};
			b3LauncherCL launcher(m_queue, m_sapKernel, "m_sapKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(numSmallAabbs);
			launcher.setConst(axis);
			launcher.setConst(maxPairs);

			i32 num = numSmallAabbs;
#if 0                
                i32 buffSize = launcher.getSerializationBufferSize();
                u8* buf = new u8[buffSize+sizeof(i32)];
                for (i32 i=0;i<buffSize+1;i++)
                {
                    u8* ptr = (u8*)&buf[i];
                    *ptr = 0xff;
                }
                i32 actualWrite = launcher.serializeArguments(buf,buffSize);
                
                u8* cptr = (u8*)&buf[buffSize];
    //            printf("buf[buffSize] = %d\n",*cptr);
                
                assert(buf[buffSize]==0xff);//check for buffer overrun
                i32* ptr = (i32*)&buf[buffSize];
                
                *ptr = num;
                
                FILE* f = fopen("m_sapKernelArgs.bin","wb");
                fwrite(buf,buffSize+sizeof(i32),1,f);
                fclose(f);
#endif  //

			launcher.launch1D(num);
			clFinish(m_queue);

			numPairs = m_pairCount.at(0);
			if (numPairs > maxPairs)
			{
				drx3DError("Error running out of pairs: numPairs = %d, maxPairs = %d.\n", numPairs, maxPairs);
				numPairs = maxPairs;
				m_pairCount.resize(0);
				m_pairCount.push_back(maxPairs);
			}
		}

#else
		i32 numPairs = 0;

		b3LauncherCL launcher(m_queue, m_sapKernel);

		tukk fileName = "m_sapKernelArgs.bin";
		FILE* f = fopen(fileName, "rb");
		if (f)
		{
			i32 sizeInBytes = 0;
			if (fseek(f, 0, SEEK_END) || (sizeInBytes = ftell(f)) == EOF || fseek(f, 0, SEEK_SET))
			{
				printf("error, cannot get file size\n");
				exit(0);
			}

			u8* buf = (u8*)malloc(sizeInBytes);
			fread(buf, sizeInBytes, 1, f);
			i32 serializedBytes = launcher.deserializeArgs(buf, sizeInBytes, m_context);
			i32 num = *(i32*)&buf[serializedBytes];
			launcher.launch1D(num);

			b3OpenCLArray<i32> pairCount(m_context, m_queue);
			i32 numElements = launcher.m_arrays[2]->size() / sizeof(i32);
			pairCount.setFromOpenCLBuffer(launcher.m_arrays[2]->getBufferCL(), numElements);
			numPairs = pairCount.at(0);
			//printf("overlapping pairs = %d\n",numPairs);
			b3AlignedObjectArray<b3Int4> hostOoverlappingPairs;
			b3OpenCLArray<b3Int4> tmpGpuPairs(m_context, m_queue);
			tmpGpuPairs.setFromOpenCLBuffer(launcher.m_arrays[1]->getBufferCL(), numPairs);

			tmpGpuPairs.copyToHost(hostOoverlappingPairs);
			m_overlappingPairs.copyFromHost(hostOoverlappingPairs);
			//printf("hello %d\n", m_overlappingPairs.size());
			free(buf);
			fclose(f);
		}
		else
		{
			printf("ошибка: cannot find file %s\n", fileName);
		}

		clFinish(m_queue);

#endif

		m_overlappingPairs.resize(numPairs);

	}  //D3_PROFILE("GPU_RADIX SORT");
	   //init3dSap();
}

void b3GpuSapBroadphase::writeAabbsToGpu()
{
	m_smallAabbsMappingGPU.copyFromHost(m_smallAabbsMappingCPU);
	m_largeAabbsMappingGPU.copyFromHost(m_largeAabbsMappingCPU);

	m_allAabbsGPU.copyFromHost(m_allAabbsCPU);  //might not be necessary, the 'setupGpuAabbsFull' already takes care of this
}

void b3GpuSapBroadphase::createLargeProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	i32 index = userPtr;
	b3SapAabb aabb;
	for (i32 i = 0; i < 4; i++)
	{
		aabb.m_min[i] = aabbMin[i];
		aabb.m_max[i] = aabbMax[i];
	}
	aabb.m_minIndices[3] = index;
	aabb.m_signedMaxIndices[3] = m_allAabbsCPU.size();
	m_largeAabbsMappingCPU.push_back(m_allAabbsCPU.size());

	m_allAabbsCPU.push_back(aabb);
}

void b3GpuSapBroadphase::createProxy(const b3Vec3& aabbMin, const b3Vec3& aabbMax, i32 userPtr, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	i32 index = userPtr;
	b3SapAabb aabb;
	for (i32 i = 0; i < 4; i++)
	{
		aabb.m_min[i] = aabbMin[i];
		aabb.m_max[i] = aabbMax[i];
	}
	aabb.m_minIndices[3] = index;
	aabb.m_signedMaxIndices[3] = m_allAabbsCPU.size();
	m_smallAabbsMappingCPU.push_back(m_allAabbsCPU.size());

	m_allAabbsCPU.push_back(aabb);
}

cl_mem b3GpuSapBroadphase::getAabbBufferWS()
{
	return m_allAabbsGPU.getBufferCL();
}

i32 b3GpuSapBroadphase::getNumOverlap()
{
	return m_overlappingPairs.size();
}
cl_mem b3GpuSapBroadphase::getOverlappingPairBuffer()
{
	return m_overlappingPairs.getBufferCL();
}

b3OpenCLArray<b3Int4>& b3GpuSapBroadphase::getOverlappingPairsGPU()
{
	return m_overlappingPairs;
}
b3OpenCLArray<i32>& b3GpuSapBroadphase::getSmallAabbIndicesGPU()
{
	return m_smallAabbsMappingGPU;
}
b3OpenCLArray<i32>& b3GpuSapBroadphase::getLargeAabbIndicesGPU()
{
	return m_largeAabbsMappingGPU;
}
