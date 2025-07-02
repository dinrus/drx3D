//those header files need to be at the top, because of conflict __global and STL

#include "PairBench.h"
#include <drx3D/Common/b3Quat.h"

#include "Bullet3OpenCL/BroadphaseCollision/b3GpuSapBroadphase.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuGridBroadphase.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuParallelLinearBvhBroadphase.h"
#include <drx3D/Common/b3Clock.h>

//#include "../GpuDemoInternalData.h"
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"

#include "Bullet3OpenCL/ParallelPrimitives/b3LauncherCL.h"

#include "../OpenGLWindow/OpenGLInclude.h"
#include "../OpenGLWindow/ShapeData.h"

#include <string.h>

#include "pairsKernel.h"

extern i32 gPreferredOpenCLDeviceIndex;
extern i32 gPreferredOpenCLPlatformIndex;

#include <drx3D/Common/Interfaces/CommonExampleInterface.h"
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h"
#include <drx3D/Common/Interfaces/CommonRenderInterface.h"
#include <drx3D/Common/Interfaces/CommonCameraInterface.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h"
#include <drx3D/Common/Interfaces/CommonWindowInterface.h"
#include "../CommonOpenCL/CommonOpenCLBase.h"
#include "../OpenGLWindow/GLInstancingRenderer.h"
#include "../OpenGLWindow/GLInstanceRendererInternalData.h"

tuk gPairBenchFileName = 0;

class PairBench : public CommonOpenCLBase
{
	struct PairBenchInternalData* m_data;

public:
	PairBench(GUIHelperInterface* helper);
	virtual ~PairBench();

	virtual void initPhysics();
	virtual void exitPhysics();

	void createBroadphase(i32 xdim, i32 ydim, i32 zdim);
	void deleteBroadphase();

	virtual void stepSimulation(float deltaTime);

	virtual void renderScene();

	virtual void resetCamera()
	{
		float dist = 10;

		if (gPairBenchFileName)
		{
			dist = 830;
		}
		else
		{
			dist = 130;
		}

		float pitch = -33;
		float yaw = 62;
		float targetPos[4] = {15.5, 12.5, 15.5, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

//we use an offset, just for testing to make sure there is no assumption in the broadphase that 'index' starts at 0
#define TEST_INDEX_OFFSET 1024

extern bool useShadowMap;
float maxExtents = -1e30f;
i32 largeCount = 0;

float timeStepPos = 0.000166666;
float mAmplitude = 251.f;
i32 dimensions[3] = {10, 10, 10};  //initialized with x_dim/y_dim/z_dim
tukk axisNames[3] = {"# x-axis", "# y-axis", "# z-axis"};
extern bool gReset;

static i32 curUseLargeAabbOption = 0;
tukk useLargeAabbOptions[] =
	{
		"NoLargeAabb",
		"UseLargeAabb",
};

struct BroadphaseEntry
{
	tukk m_name;
	b3GpuBroadphaseInterface::CreateFunc* m_createFunc;
};

static PairBench* sPairDemo = 0;

#define BP_COMBO_INDEX 123

static i32 curSelectedBroadphase = 0;
static BroadphaseEntry allBroadphases[] =
	{
		{"Gpu Grid", b3GpuGridBroadphase::CreateFunc},
		{"Parallel Linear BVH", b3GpuParallelLinearBvhBroadphase::CreateFunc},
		{"CPU Brute Force", b3GpuSapBroadphase::CreateFuncBruteForceCpu},
		{"GPU Brute Force", b3GpuSapBroadphase::CreateFuncBruteForceGpu},
		{"GPU 1-SAP Original", b3GpuSapBroadphase::CreateFuncOriginal},
		{"GPU 1-SAP Barrier", b3GpuSapBroadphase::CreateFuncBarrier},
		{"GPU 1-SAP LDS", b3GpuSapBroadphase::CreateFuncLocalMemory}};

struct PairBenchInternalData
{
	b3GpuBroadphaseInterface* m_broadphaseGPU;
	b3GpuBroadphaseInterface* m_validationBroadphase;

	cl_kernel m_moveObjectsKernel;
	cl_kernel m_sineWaveKernel;
	cl_kernel m_colorPairsKernel;
	cl_kernel m_updateAabbSimple;

	b3OpenCLArray<b3Vec4>* m_instancePosOrnColor;
	b3OpenCLArray<float>* m_bodyTimes;
	PairBenchInternalData()
		: m_broadphaseGPU(0),
		  m_moveObjectsKernel(0),
		  m_sineWaveKernel(0),
		  m_colorPairsKernel(0),
		  m_instancePosOrnColor(0),
		  m_bodyTimes(0),
		  m_updateAabbSimple(0)
	{
	}

	i32 m_oldYposition;
};

PairBench::PairBench(GUIHelperInterface* helper)
	: CommonOpenCLBase(helper)
{
	m_data = new PairBenchInternalData;

	m_data->m_validationBroadphase = 0;
}
PairBench::~PairBench()
{
	delete m_data;
}

static inline float parseFloat(tukk & token)
{
	token += strspn(token, " \t");
	float f = (float)atof(token);
	token += strcspn(token, " \t\r");
	return f;
}

enum PairToggleButtons
{
	MY_RESET = 1024,
};

#define PAIRS_CL_PROGRAM_PATH "Demos3/GpuDemos/broadphase/pairsKernel.cl"

void PairBench::initPhysics()
{
	dimensions[0] = 10;
	dimensions[1] = 10;
	dimensions[2] = 10;

	//m_guiHelper->getRenderInterface() = ci.m_guiHelper->getRenderInterface();
	sPairDemo = this;
	useShadowMap = false;

	initCL(gPreferredOpenCLDeviceIndex, gPreferredOpenCLPlatformIndex);

	if (m_clData->m_clContext)
	{
		cl_int err;
		cl_program pairBenchProg = b3OpenCLUtils::compileCLProgramFromString(m_clData->m_clContext, m_clData->m_clDevice, pairsKernelsCL, &err, "", PAIRS_CL_PROGRAM_PATH);
		i32 errNum = 0;
		m_data->m_moveObjectsKernel = b3OpenCLUtils::compileCLKernelFromString(m_clData->m_clContext, m_clData->m_clDevice, pairsKernelsCL, "moveObjectsKernel", &errNum, pairBenchProg);
		m_data->m_sineWaveKernel = b3OpenCLUtils::compileCLKernelFromString(m_clData->m_clContext, m_clData->m_clDevice, pairsKernelsCL, "sineWaveKernel", &errNum, pairBenchProg);
		m_data->m_colorPairsKernel = b3OpenCLUtils::compileCLKernelFromString(m_clData->m_clContext, m_clData->m_clDevice, pairsKernelsCL, "colorPairsKernel2", &errNum, pairBenchProg);
		m_data->m_updateAabbSimple = b3OpenCLUtils::compileCLKernelFromString(m_clData->m_clContext, m_clData->m_clDevice, pairsKernelsCL, "updateAabbSimple", &errNum, pairBenchProg);

		//Method for validating the overlapping pairs requires that the
		//reference broadphase does not maintain internal state aside from AABB data.
		//That is, overwriting the AABB state in the broadphase using
		//	b3GpuBroadphaseInterface::getAllAabbsGPU(),
		//	b3GpuBroadphaseInterface::getSmallAabbIndicesGPU(), and
		//	b3GpuBroadphaseInterface::getLargeAabbIndicesGPU()
		//and then calling b3GpuBroadphaseInterface::calculateOverlappingPairs() should
		//always produce the same result regardless of the current state of the broadphase.
		m_data->m_validationBroadphase = b3GpuParallelLinearBvhBroadphase::CreateFunc(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue);
	}

	createBroadphase(dimensions[0], dimensions[1], dimensions[2]);
}

void PairBench::createBroadphase(i32 arraySizeX, i32 arraySizeY, i32 arraySizeZ)
{
	m_data->m_broadphaseGPU = (allBroadphases[curSelectedBroadphase].m_createFunc)(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue);

	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_vertices) / sizeof(i32);
	i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&cube_vertices[0], numVertices, cube_indices, numIndices);
	i32 group = 1;
	i32 mask = 1;
	i32 index = TEST_INDEX_OFFSET;

	if (gPairBenchFileName)
	{
		//tuk fileName = "32006GPUAABBs.txt";
		char relativeFileName[1024];
		tukk prefix[] = {"./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
		i32 prefixIndex = -1;
		{
			i32 numPrefixes = sizeof(prefix) / sizeof(tuk);

			for (i32 i = 0; i < numPrefixes; i++)
			{
				FILE* f = 0;
				sprintf(relativeFileName, "%s%s", prefix[i], gPairBenchFileName);
				f = fopen(relativeFileName, "rb");
				if (f)
				{
					fseek(f, 0L, SEEK_END);
					i32 size = ftell(f);
					rewind(f);
					tuk buf = (tuk)malloc(size);

					i32 actualReadBytes = 0;

					while (actualReadBytes < size)
					{
						i32 left = size - actualReadBytes;
						i32 chunk = 8192;
						i32 numPlannedRead = left < chunk ? left : chunk;
						actualReadBytes += fread(&buf[actualReadBytes], 1, numPlannedRead, f);
					}

					fclose(f);

					char pattern[1024];
					pattern[0] = 0x0a;
					pattern[1] = 0;
					size_t const patlen = strlen(pattern);
					size_t patcnt = 0;
					tuk oriptr;
					tuk patloc;

					for (oriptr = buf; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
					{
						if (patloc)
						{
							*patloc = 0;
							tukk token = oriptr;

							b3Vec3 aabbMin;
							b3Vec3 aabbMax;

							aabbMin.x = parseFloat(token);
							aabbMin.y = parseFloat(token);
							aabbMin.z = parseFloat(token);
							aabbMin.w = 0.f;
							aabbMax.x = parseFloat(token);
							aabbMax.y = parseFloat(token);
							aabbMax.z = parseFloat(token);
							aabbMax.w = 0.f;

							aabbMin *= 0.1;
							aabbMax *= 0.1;

							b3Vec3 extents = aabbMax - aabbMin;

							//printf("%s\n", oriptr);

							b3Vec3 position = 0.5 * (aabbMax + aabbMin);
							b3Quat orn(0, 0, 0, 1);

							b3Vec4 scaling = b3MakeVector4(0.5 * extents.x, 0.5 * extents.y, 0.5 * extents.z, 1);  //b3MakeVector4(1,1,1,1);

							float l = extents.length();
							if (l > 500)
							{
								b3Vec4 color = b3MakeVector4(0, 1, 0, 0.1);
								i32 id;
								id = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
								m_data->m_broadphaseGPU->createLargeProxy(aabbMin, aabbMax, index, group, mask);
							}
							else
							{
								b3Vec4 color = b3MakeVector4(1, 0, 0, 1);
								i32 id;
								id = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
								m_data->m_broadphaseGPU->createProxy(aabbMin, aabbMax, index, group, mask);
								index++;
							}

							patcnt++;
						}
					}
					prefixIndex = i;
					break;
				}
			}

			if (prefixIndex < 0)
			{
				drx3DPrintf("Cannot find %s\n", gPairBenchFileName);
			}
		}
	}
	else
	{
		for (i32 i = 0; i < arraySizeX; i++)
		{
			for (i32 j = 0; j < arraySizeY; j++)
			{
				for (i32 k = 0; k < arraySizeZ; k++)
				{
					b3Vec3 position = b3MakeVector3(k * 3, i * 3, j * 3);
					b3Quat orn(0, 0, 0, 1);

					b3Vec4 color = b3MakeVector4(0, 1, 0, 1);
					b3Vec4 scaling = b3MakeVector4(1, 1, 1, 1);
					bool large = false;

					if (curUseLargeAabbOption)
					{
						if (i == 0 && j == 0 && k == 0)
						{
							large = true;
							scaling[0] = 1000;
							scaling[1] = 1000;
							scaling[2] = 1000;
						}
					}
					/*if (j==0)
					{
						large=true;
						scaling[1] = 10000;
					}
					if (k==0)
					{
						large=true;
						scaling[2] = 10000;
					}*/

					i32 id;
					id = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);

					b3Vec3 aabbMin = position - scaling;
					b3Vec3 aabbMax = position + scaling;

					if (large)
					{
						m_data->m_broadphaseGPU->createLargeProxy(aabbMin, aabbMax, index, group, mask);
					}
					else
					{
						m_data->m_broadphaseGPU->createProxy(aabbMin, aabbMax, index, group, mask);
					}
					index++;
				}
			}
		}
	}

	m_guiHelper->getRenderInterface()->writeTransforms();
	m_data->m_broadphaseGPU->writeAabbsToGpu();
}

void PairBench::deleteBroadphase()
{
	delete m_data->m_broadphaseGPU;
	m_data->m_broadphaseGPU = 0;
	delete m_data->m_instancePosOrnColor;
	m_data->m_instancePosOrnColor = 0;
	delete m_data->m_bodyTimes;
	m_data->m_bodyTimes = 0;

	m_data->m_broadphaseGPU = 0;
	m_guiHelper->getRenderInterface()->removeAllInstances();
}

void PairBench::exitPhysics()
{
	//reset the state to 'on'
	useShadowMap = true;
	if (m_data->m_validationBroadphase)
	{
		delete m_data->m_validationBroadphase;
		m_data->m_validationBroadphase = 0;
	}

	sPairDemo = 0;

	exitCL();
}

void PairBench::renderScene()
{
	m_guiHelper->getRenderInterface()->renderScene();
}

struct OverlappingPairSortPredicate
{
	inline bool operator()(const b3Int4& a, const b3Int4& b) const
	{
		if (a.x != b.x) return (a.x < b.x);
		if (a.y != b.y) return (a.y < b.y);
		if (a.z != b.z) return (a.z < b.z);
		return (a.w < b.w);
	}
};

void PairBench::stepSimulation(float deltaTime)
{
	//color all objects blue

	GLInstanceRendererInternalData* internalData = m_guiHelper->getRenderInterface()->getInternalData();

	if (internalData == 0)
		return;

	//bool animate=true;
	i32 numObjects = 0;
	{
		D3_PROFILE("Num Objects");
		numObjects = internalData->m_totalNumInstances;
	}
	b3Vec4* positions = 0;
	if (numObjects)
	{
		D3_PROFILE("Sync");
		GLuint vbo = internalData->m_vbo;

		i32 arraySizeInBytes = numObjects * (3) * sizeof(b3Vec4);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//	cl_bool blocking=  CL_TRUE;
		tuk hostPtr = 0;
		{
			D3_PROFILE("glMapBufferRange");
			hostPtr = (tuk)glMapBufferRange(GL_ARRAY_BUFFER, internalData->m_maxShapeCapacityInBytes, arraySizeInBytes, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);  //GL_READ_WRITE);//GL_WRITE_ONLY
		}
		GLint err = glGetError();
		assert(err == GL_NO_ERROR);
		positions = (b3Vec4*)hostPtr;

		if (m_data->m_instancePosOrnColor && m_data->m_instancePosOrnColor->size() != 3 * numObjects)
		{
			delete m_data->m_instancePosOrnColor;
			m_data->m_instancePosOrnColor = 0;
		}
		if (!m_data->m_instancePosOrnColor)
		{
			m_data->m_instancePosOrnColor = new b3OpenCLArray<b3Vec4>(m_clData->m_clContext, m_clData->m_clQueue);
			m_data->m_instancePosOrnColor->resize(3 * numObjects);
			m_data->m_instancePosOrnColor->copyFromHostPointer(positions, 3 * numObjects, 0);
			m_data->m_bodyTimes = new b3OpenCLArray<float>(m_clData->m_clContext, m_clData->m_clQueue);
			m_data->m_bodyTimes->resize(numObjects);
			b3AlignedObjectArray<float> tmp;
			tmp.resize(numObjects);
			for (i32 i = 0; i < numObjects; i++)
			{
				tmp[i] = float(i) * (1024.f / numObjects);
			}
			m_data->m_bodyTimes->copyFromHost(tmp);
		}

		if (!gPairBenchFileName)
		{
			if (1)
			{
				if (1)
				{
					b3LauncherCL launcher(m_clData->m_clQueue, m_data->m_sineWaveKernel, "m_sineWaveKernel");
					launcher.setBuffer(m_data->m_instancePosOrnColor->getBufferCL());
					launcher.setBuffer(m_data->m_bodyTimes->getBufferCL());

					launcher.setConst(timeStepPos);
					launcher.setConst(mAmplitude);
					launcher.setConst(numObjects);
					launcher.launch1D(numObjects);
					clFinish(m_clData->m_clQueue);
				}
				else
				{
					b3LauncherCL launcher(m_clData->m_clQueue, m_data->m_moveObjectsKernel, "m_moveObjectsKernel");
					launcher.setBuffer(m_data->m_instancePosOrnColor->getBufferCL());
					launcher.setConst(numObjects);
					launcher.launch1D(numObjects);
					clFinish(m_clData->m_clQueue);
				}
			}
		}
	}

	bool updateOnGpu = true;

	if (1)
	{
		if (updateOnGpu)
		{
			D3_PROFILE("updateOnGpu");
			b3LauncherCL launcher(m_clData->m_clQueue, m_data->m_updateAabbSimple, "m_updateAabbSimple");
			launcher.setBuffer(m_data->m_instancePosOrnColor->getBufferCL());
			launcher.setConst(numObjects);
			launcher.setBuffer(m_data->m_broadphaseGPU->getAabbBufferWS());
			launcher.launch1D(numObjects);
			clFinish(m_clData->m_clQueue);
		}
		else
		{
			D3_PROFILE("updateOnCpu");
			if (!gPairBenchFileName)
			{
				//	i32 allAabbs = m_data->m_broadphaseGPU->getAllAabbsCPU().size();

				b3AlignedObjectArray<b3Vec4> posOrnColorsCpu;
				if (m_data->m_instancePosOrnColor)
					m_data->m_instancePosOrnColor->copyToHost(posOrnColorsCpu);

				for (i32 nodeId = 0; nodeId < numObjects; nodeId++)
				{
					{
						b3Vec3 position = posOrnColorsCpu[nodeId];

						b3SapAabb orgAabb = m_data->m_broadphaseGPU->getAllAabbsCPU()[nodeId];
						b3Vec3 halfExtents = 0.5f * (orgAabb.m_maxVec - orgAabb.m_minVec);
						i32 orgNodeIndex = orgAabb.m_minIndices[3];
						i32 orgBroadphaseIndex = orgAabb.m_signedMaxIndices[3];

						m_data->m_broadphaseGPU->getAllAabbsCPU()[nodeId].m_minVec = position - halfExtents;
						m_data->m_broadphaseGPU->getAllAabbsCPU()[nodeId].m_minIndices[3] = orgNodeIndex;
						m_data->m_broadphaseGPU->getAllAabbsCPU()[nodeId].m_maxVec = position + halfExtents;
						m_data->m_broadphaseGPU->getAllAabbsCPU()[nodeId].m_signedMaxIndices[3] = orgBroadphaseIndex;
					}
				}
				m_data->m_broadphaseGPU->writeAabbsToGpu();
			}
		}
	}

	i32 prealloc = 3 * 1024 * 1024;
	i32 maxOverlap = d3Min(prealloc, 16 * numObjects);

	u64 dt = 0;
	if (numObjects)
	{
		b3Clock cl;
		dt = cl.getTimeMicroseconds();
		D3_PROFILE("calculateOverlappingPairs");
		//i32 sz = sizeof(b3Int4)*64*numObjects;

		m_data->m_broadphaseGPU->calculateOverlappingPairs(maxOverlap);
		i32 numPairs;
		numPairs = m_data->m_broadphaseGPU->getNumOverlap();
		//printf("numPairs = %d\n", numPairs);
		dt = cl.getTimeMicroseconds() - dt;
	}

	const bool VALIDATE_BROADPHASE = false;  //Check that overlapping pairs of 2 broadphases are the same
	if (numObjects && VALIDATE_BROADPHASE)
	{
		D3_PROFILE("validate broadphases");

		{
			D3_PROFILE("calculateOverlappingPairs m_validationBroadphase");
			//m_data->m_validationBroadphase->getAllAabbsCPU() = m_data->m_broadphaseGPU->getAllAabbsCPU();

			m_data->m_validationBroadphase->getAllAabbsGPU().copyFromOpenCLArray(m_data->m_broadphaseGPU->getAllAabbsGPU());
			m_data->m_validationBroadphase->getSmallAabbIndicesGPU().copyFromOpenCLArray(m_data->m_broadphaseGPU->getSmallAabbIndicesGPU());
			m_data->m_validationBroadphase->getLargeAabbIndicesGPU().copyFromOpenCLArray(m_data->m_broadphaseGPU->getLargeAabbIndicesGPU());

			m_data->m_validationBroadphase->calculateOverlappingPairs(maxOverlap);
		}

		static b3AlignedObjectArray<b3Int4> overlappingPairs;
		static b3AlignedObjectArray<b3Int4> overlappingPairsReference;
		m_data->m_broadphaseGPU->getOverlappingPairsGPU().copyToHost(overlappingPairs);
		m_data->m_validationBroadphase->getOverlappingPairsGPU().copyToHost(overlappingPairsReference);

		//Reorder pairs so that (pair.x < pair.y) is always true
		{
			D3_PROFILE("reorder pairs");

			for (i32 i = 0; i < overlappingPairs.size(); ++i)
			{
				b3Int4 pair = overlappingPairs[i];
				if (pair.x > pair.y)
				{
					b3Swap(pair.x, pair.y);
					b3Swap(pair.z, pair.w);
					overlappingPairs[i] = pair;
				}
			}
			for (i32 i = 0; i < overlappingPairsReference.size(); ++i)
			{
				b3Int4 pair = overlappingPairsReference[i];
				if (pair.x > pair.y)
				{
					b3Swap(pair.x, pair.y);
					b3Swap(pair.z, pair.w);
					overlappingPairsReference[i] = pair;
				}
			}
		}

		//
		{
			D3_PROFILE("Sort overlapping pairs from most to least significant bit");

			overlappingPairs.quickSort(OverlappingPairSortPredicate());
			overlappingPairsReference.quickSort(OverlappingPairSortPredicate());
		}

		//Compare
		{
			D3_PROFILE("compare pairs");

			i32 numPairs = overlappingPairs.size();
			i32 numPairsReference = overlappingPairsReference.size();

			bool success = true;

			if (numPairs == numPairsReference)
			{
				for (i32 i = 0; i < numPairsReference; ++i)
				{
					const b3Int4& pairA = overlappingPairs[i];
					const b3Int4& pairB = overlappingPairsReference[i];
					if (pairA.x != pairB.x || pairA.y != pairB.y || pairA.z != pairB.z || pairA.w != pairB.w)
					{
						drx3DError("Ошибка: one or more overlappingPairs differs from reference.\n");
						success = false;
						break;
					}
				}
			}
			else
			{
				drx3DError("Ошибка: numPairs %d != numPairsReference %d \n", numPairs, numPairsReference);
				success = false;
			}

			printf("Broadphase validation: %d \n", success);
		}
	}

	/*
	if (m_data->m_gui)
	{
		D3_PROFILE("update Gui");
		i32 allAabbs = m_data->m_broadphaseGPU->getAllAabbsCPU().size();
		i32 numOverlap = m_data->m_broadphaseGPU->getNumOverlap();

		float time = dt/1000.f;
		//printf("time = %f\n", time);

		char msg[1024];
		sprintf(msg,"#objects = %d, #overlapping pairs = %d, time = %f ms", allAabbs,numOverlap,time );
		//printf("msg=%s\n",msg);
		m_data->m_gui->setStatusBarMessage(msg,true);
	}
	*/

	if (numObjects)
	{
		D3_PROFILE("animate");
		GLint err = glGetError();
		assert(err == GL_NO_ERROR);
		//color overlapping objects in red

		if (m_data->m_broadphaseGPU->getNumOverlap())
		{
			bool colorPairsOnHost = false;
			if (colorPairsOnHost)
			{
			}
			else
			{
				i32 numPairs = m_data->m_broadphaseGPU->getNumOverlap();
				cl_mem pairBuf = m_data->m_broadphaseGPU->getOverlappingPairBuffer();

				b3LauncherCL launcher(m_clData->m_clQueue, m_data->m_colorPairsKernel, "m_colorPairsKernel");
				launcher.setBuffer(m_data->m_instancePosOrnColor->getBufferCL());
				launcher.setConst(numObjects);
				launcher.setBuffer(pairBuf);
				i32 indexOffset = TEST_INDEX_OFFSET;
				launcher.setConst(indexOffset);
				launcher.setConst(numPairs);
				launcher.launch1D(numPairs);
				clFinish(m_clData->m_clQueue);
			}
		}

		if (numObjects)
		{
			m_data->m_instancePosOrnColor->copyToHostPointer(positions, 3 * numObjects, 0);
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);
		err = glGetError();
		assert(err == GL_NO_ERROR);
	}
}

class CommonExampleInterface* PairBenchOpenCLCreateFunc(struct CommonExampleOptions& options)
{
	return new PairBench(options.m_guiHelper);
}
