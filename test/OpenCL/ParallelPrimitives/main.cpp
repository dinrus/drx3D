/*
Copyright (c) 2012 Advanced Micro Devices, Inc.  

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3FillCL.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3BoundSearchCL.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3RadixSort32CL.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3PrefixScanCL.h"
#include "Bullet3Common/b3CommandLineArgs.h"
#include "Bullet3Common/b3MinMax.h"

i32 g_nPassed = 0;
i32 g_nFailed = 0;
bool g_testFailed = 0;

#define TEST_INIT g_testFailed = 0;
#define TEST_ASSERT(x)    \
	if (!(x))             \
	{                     \
		g_testFailed = 1; \
	}
#define TEST_REPORT(testName)                                  \
	printf("[%s] %s\n", (g_testFailed) ? "X" : "O", testName); \
	if (g_testFailed)                                          \
		g_nFailed++;                                           \
	else                                                       \
		g_nPassed++;
#define NEXTMULTIPLEOF(num, alignment) (((num) / (alignment) + (((num) % (alignment) == 0) ? 0 : 1)) * (alignment))

cl_context g_context = 0;
cl_device_id g_device = 0;
cl_command_queue g_queue = 0;
tukk g_deviceName = 0;

void initCL(i32 preferredDeviceIndex, i32 preferredPlatformIndex)
{
	//uk glCtx=0;
	//uk glDC = 0;
	i32 ciErrNum = 0;
	//bound search and radix sort only work on GPU right now (assume 32 or 64 width workgroup without barriers)

	cl_device_type deviceType = CL_DEVICE_TYPE_ALL;

	g_context = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, 0, 0, preferredDeviceIndex, preferredPlatformIndex);
	oclCHECKERROR(ciErrNum, CL_SUCCESS);
	i32 numDev = b3OpenCLUtils::getNumDevices(g_context);
	if (numDev > 0)
	{
		b3OpenCLDeviceInfo info;
		g_device = b3OpenCLUtils::getDevice(g_context, 0);
		g_queue = clCreateCommandQueue(g_context, g_device, 0, &ciErrNum);
		oclCHECKERROR(ciErrNum, CL_SUCCESS);
		b3OpenCLUtils::printDeviceInfo(g_device);
		b3OpenCLUtils::getDeviceInfo(g_device, &info);
		g_deviceName = info.m_deviceName;
	}
}

void exitCL()
{
	clReleaseCommandQueue(g_queue);
	clReleaseContext(g_context);
}

inline void fillIntTest()
{
	TEST_INIT;

	b3FillCL* fillCL = new b3FillCL(g_context, g_device, g_queue);
	i32 maxSize = 1024 * 256;
	b3OpenCLArray<i32> intBuffer(g_context, g_queue, maxSize);
	intBuffer.resize(maxSize);

#define NUM_TESTS 7

	i32 dx = maxSize / NUM_TESTS;
	for (i32 iter = 0; iter < NUM_TESTS; iter++)
	{
		i32 size = b3Min(11 + dx * iter, maxSize);

		i32 value = 2;

		i32 offset = 0;
		fillCL->execute(intBuffer, value, size, offset);

		b3AlignedObjectArray<i32> hostBuf2;
		hostBuf2.resize(size);
		fillCL->executeHost(hostBuf2, value, size, offset);

		b3AlignedObjectArray<i32> hostBuf;
		intBuffer.copyToHost(hostBuf);

		for (i32 i = 0; i < size; i++)
		{
			TEST_ASSERT(hostBuf[i] == hostBuf2[i]);
			TEST_ASSERT(hostBuf[i] == hostBuf2[i]);
		}
	}

	delete fillCL;

	TEST_REPORT("fillIntTest");
}

__inline void seedRandom(i32 seed)
{
	srand(seed);
}

template <typename T>
__inline T getRandom(const T& minV, const T& maxV)
{
	float r = (rand() % 10000) / 10000.f;
	T range = maxV - minV;
	return (T)(minV + r * range);
}

struct b3SortDataCompare
{
	inline bool operator()(const b3SortData& first, const b3SortData& second) const
	{
		return (first.m_key < second.m_key) || (first.m_key == second.m_key && first.m_value < second.m_value);
	}
};

void boundSearchTest()
{
	TEST_INIT;

	i32 maxSize = 1024 * 256;
	i32 bucketSize = 256;

	b3OpenCLArray<b3SortData> srcCL(g_context, g_queue, maxSize);
	b3OpenCLArray<u32> upperCL(g_context, g_queue, maxSize);
	b3OpenCLArray<u32> lowerCL(g_context, g_queue, maxSize);

	b3AlignedObjectArray<b3SortData> srcHost;
	b3AlignedObjectArray<u32> upperHost;
	b3AlignedObjectArray<u32> lowerHost;
	b3AlignedObjectArray<u32> upperHostCompare;
	b3AlignedObjectArray<u32> lowerHostCompare;

	b3BoundSearchCL* search = new b3BoundSearchCL(g_context, g_device, g_queue, maxSize);

	i32 dx = maxSize / NUM_TESTS;
	for (i32 iter = 0; iter < NUM_TESTS; iter++)
	{
		i32 size = b3Min(128 + dx * iter, maxSize);

		upperHost.resize(bucketSize);
		lowerHost.resize(bucketSize);
		upperHostCompare.resize(bucketSize);
		lowerHostCompare.resize(bucketSize);

		srcHost.resize(size);

		for (i32 i = 0; i < size; i++)
		{
			b3SortData v;
			//			v.m_key = i<2? 0 : 5;
			v.m_key = getRandom(0, bucketSize);

			v.m_value = i;
			srcHost.at(i) = v;
		}

		srcHost.quickSort(b3SortDataCompare());
		srcCL.copyFromHost(srcHost);

		{
			for (i32 i = 0; i < bucketSize; i++)
			{
				lowerHost[i] = -1;
				lowerHostCompare[i] = -1;
				upperHost[i] = -1;
				upperHostCompare[i] = -1;
			}
			upperCL.copyFromHost(upperHost);
			lowerCL.copyFromHost(lowerHost);
		}

		search->execute(srcCL, size, upperCL, bucketSize, b3BoundSearchCL::BOUND_UPPER);
		search->execute(srcCL, size, lowerCL, bucketSize, b3BoundSearchCL::BOUND_LOWER);

		search->executeHost(srcHost, size, upperHostCompare, bucketSize, b3BoundSearchCL::BOUND_UPPER);
		search->executeHost(srcHost, size, lowerHostCompare, bucketSize, b3BoundSearchCL::BOUND_LOWER);

		lowerCL.copyToHost(lowerHost);
		upperCL.copyToHost(upperHost);
		for (i32 i = 0; i < bucketSize; i++)
		{
			TEST_ASSERT(upperHostCompare[i] == upperHost[i]);
			TEST_ASSERT(lowerHostCompare[i] == lowerHost[i]);
		}
		/*
		for(i32 i=1; i<bucketSize; i++)
		{
			i32 lhi_1 = lowerHost[i-1];
			i32 lhi = lowerHost[i];

			for(i32 j=lhi_1; j<lhi; j++)
			//for(i32 j=lowerHost[i-1]; j<lowerHost[i]; j++)
			{
				TEST_ASSERT( srcHost[j].m_key < i );
			}
		}

		for(i32 i=0; i<bucketSize; i++)
		{
			i32 jMin = (i==0)?0:upperHost[i-1];
			for(i32 j=jMin; j<upperHost[i]; j++)
			{
				TEST_ASSERT( srcHost[j].m_key <= i );
			}
		}
		*/

		for (i32 i = 0; i < bucketSize; i++)
		{
			i32 lhi = lowerHost[i];
			i32 uhi = upperHost[i];

			for (i32 j = lhi; j < uhi; j++)
			{
				if (srcHost[j].m_key != i)
				{
					printf("error %d != %d\n", srcHost[j].m_key, i);
				}
				TEST_ASSERT(srcHost[j].m_key == i);
			}
		}
	}

	delete search;

	TEST_REPORT("boundSearchTest");
}

void prefixScanTest()
{
	TEST_INIT;

	i32 maxSize = 1024 * 256;

	b3AlignedObjectArray<u32> buf0Host;
	b3AlignedObjectArray<u32> buf1Host;

	b3OpenCLArray<u32> buf2CL(g_context, g_queue, maxSize);
	b3OpenCLArray<u32> buf3CL(g_context, g_queue, maxSize);

	b3PrefixScanCL* scan = new b3PrefixScanCL(g_context, g_device, g_queue, maxSize);

	i32 dx = maxSize / NUM_TESTS;
	for (i32 iter = 0; iter < NUM_TESTS; iter++)
	{
		i32 size = b3Min(128 + dx * iter, maxSize);
		buf0Host.resize(size);
		buf1Host.resize(size);

		for (i32 i = 0; i < size; i++)
			buf0Host[i] = 1;

		buf2CL.copyFromHost(buf0Host);

		u32 sumHost, sumGPU;

		scan->executeHost(buf0Host, buf1Host, size, &sumHost);
		scan->execute(buf2CL, buf3CL, size, &sumGPU);

		buf3CL.copyToHost(buf0Host);

		TEST_ASSERT(sumHost == sumGPU);
		for (i32 i = 0; i < size; i++)
			TEST_ASSERT(buf1Host[i] == buf0Host[i]);
	}

	delete scan;

	TEST_REPORT("scanTest");
}

bool radixSortTest()
{
	TEST_INIT;

	i32 maxSize = 1024 * 256;

	b3AlignedObjectArray<b3SortData> buf0Host;
	buf0Host.resize(maxSize);
	b3AlignedObjectArray<b3SortData> buf1Host;
	buf1Host.resize(maxSize);
	b3OpenCLArray<b3SortData> buf2CL(g_context, g_queue, maxSize);

	b3RadixSort32CL* sort = new b3RadixSort32CL(g_context, g_device, g_queue, maxSize);

	i32 dx = maxSize / NUM_TESTS;
	for (i32 iter = 0; iter < NUM_TESTS; iter++)
	{
		i32 size = b3Min(128 + dx * iter, maxSize - 512);
		size = NEXTMULTIPLEOF(size, 512);  //not necessary

		buf0Host.resize(size);

		for (i32 i = 0; i < size; i++)
		{
			b3SortData v;
			v.m_key = getRandom(0, 0xff);
			v.m_value = i;
			buf0Host[i] = v;
		}

		buf2CL.copyFromHost(buf0Host);

		sort->executeHost(buf0Host);
		sort->execute(buf2CL);

		buf2CL.copyToHost(buf1Host);

		for (i32 i = 0; i < size; i++)
		{
			TEST_ASSERT(buf0Host[i].m_value == buf1Host[i].m_value && buf0Host[i].m_key == buf1Host[i].m_key);
		}
	}

	delete sort;

	TEST_REPORT("radixSort");

	return g_testFailed;
}

i32 main(i32 argc, tuk* argv)
{
	i32 preferredDeviceIndex = -1;
	i32 preferredPlatformIndex = -1;

	b3CommandLineArgs args(argc, argv);
	args.GetCmdLineArgument("deviceId", preferredDeviceIndex);
	args.GetCmdLineArgument("platformId", preferredPlatformIndex);

	initCL(preferredDeviceIndex, preferredPlatformIndex);

	fillIntTest();

	boundSearchTest();

	prefixScanTest();

	radixSortTest();

	exitCL();

	printf("%d tests passed, %d tests failed\n", g_nPassed, g_nFailed);
	printf("End, press <enter>\n");
	getchar();
}
