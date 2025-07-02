
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>

#define RADIXSORT32_PATH "drx3D//drx3D/3OpenCL/ParallelPrimitives/kernels/RadixSort32Kernels.cl"

#include <drx3D/OpenCL/ParallelPrimitives/kernels/RadixSort32KernelsCL.h>

b3RadixSort32CL::b3RadixSort32CL(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 initialCapacity)
	: m_commandQueue(queue)
{
	b3OpenCLDeviceInfo info;
	b3OpenCLUtils::getDeviceInfo(device, &info);
	m_deviceCPU = (info.m_deviceType & CL_DEVICE_TYPE_CPU) != 0;

	m_workBuffer1 = new b3OpenCLArray<u32>(ctx, queue);
	m_workBuffer2 = new b3OpenCLArray<u32>(ctx, queue);
	m_workBuffer3 = new b3OpenCLArray<b3SortData>(ctx, queue);
	m_workBuffer3a = new b3OpenCLArray<u32>(ctx, queue);
	m_workBuffer4 = new b3OpenCLArray<b3SortData>(ctx, queue);
	m_workBuffer4a = new b3OpenCLArray<u32>(ctx, queue);

	if (initialCapacity > 0)
	{
		m_workBuffer1->resize(initialCapacity);
		m_workBuffer3->resize(initialCapacity);
		m_workBuffer3a->resize(initialCapacity);
		m_workBuffer4->resize(initialCapacity);
		m_workBuffer4a->resize(initialCapacity);
	}

	m_scan = new b3PrefixScanCL(ctx, device, queue);
	m_fill = new b3FillCL(ctx, device, queue);

	tukk additionalMacros = "";

	cl_int pErrNum;
	tukk kernelSource = radixSort32KernelsCL;

	cl_program sortProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, kernelSource, &pErrNum, additionalMacros, RADIXSORT32_PATH);
	drx3DAssert(sortProg);

	m_streamCountSortDataKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "StreamCountSortDataKernel", &pErrNum, sortProg, additionalMacros);
	drx3DAssert(m_streamCountSortDataKernel);

	m_streamCountKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "StreamCountKernel", &pErrNum, sortProg, additionalMacros);
	drx3DAssert(m_streamCountKernel);

	if (m_deviceCPU)
	{
		m_sortAndScatterSortDataKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "SortAndScatterSortDataKernelSerial", &pErrNum, sortProg, additionalMacros);
		drx3DAssert(m_sortAndScatterSortDataKernel);
		m_sortAndScatterKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "SortAndScatterKernelSerial", &pErrNum, sortProg, additionalMacros);
		drx3DAssert(m_sortAndScatterKernel);
	}
	else
	{
		m_sortAndScatterSortDataKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "SortAndScatterSortDataKernel", &pErrNum, sortProg, additionalMacros);
		drx3DAssert(m_sortAndScatterSortDataKernel);
		m_sortAndScatterKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "SortAndScatterKernel", &pErrNum, sortProg, additionalMacros);
		drx3DAssert(m_sortAndScatterKernel);
	}

	m_prefixScanKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "PrefixScanKernel", &pErrNum, sortProg, additionalMacros);
	drx3DAssert(m_prefixScanKernel);
}

b3RadixSort32CL::~b3RadixSort32CL()
{
	delete m_scan;
	delete m_fill;
	delete m_workBuffer1;
	delete m_workBuffer2;
	delete m_workBuffer3;
	delete m_workBuffer3a;
	delete m_workBuffer4;
	delete m_workBuffer4a;

	clReleaseKernel(m_streamCountSortDataKernel);
	clReleaseKernel(m_streamCountKernel);
	clReleaseKernel(m_sortAndScatterSortDataKernel);
	clReleaseKernel(m_sortAndScatterKernel);
	clReleaseKernel(m_prefixScanKernel);
}

void b3RadixSort32CL::executeHost(b3AlignedObjectArray<b3SortData>& inout, i32 sortBits /* = 32 */)
{
	i32 n = inout.size();
	i32k BITS_PER_PASS = 8;
	i32k NUM_TABLES = (1 << BITS_PER_PASS);

	i32 tables[NUM_TABLES];
	i32 counter[NUM_TABLES];

	b3SortData* src = &inout[0];
	b3AlignedObjectArray<b3SortData> workbuffer;
	workbuffer.resize(inout.size());
	b3SortData* dst = &workbuffer[0];

	i32 count = 0;
	for (i32 startBit = 0; startBit < sortBits; startBit += BITS_PER_PASS)
	{
		for (i32 i = 0; i < NUM_TABLES; i++)
		{
			tables[i] = 0;
		}

		for (i32 i = 0; i < n; i++)
		{
			i32 tableIdx = (src[i].m_key >> startBit) & (NUM_TABLES - 1);
			tables[tableIdx]++;
		}
//#define TEST
#ifdef TEST
		printf("histogram size=%d\n", NUM_TABLES);
		for (i32 i = 0; i < NUM_TABLES; i++)
		{
			if (tables[i] != 0)
			{
				printf("tables[%d]=%d]\n", i, tables[i]);
			}
		}
#endif  //TEST \
	//	prefix scan
		i32 sum = 0;
		for (i32 i = 0; i < NUM_TABLES; i++)
		{
			i32 iData = tables[i];
			tables[i] = sum;
			sum += iData;
			counter[i] = 0;
		}

		//	distribute
		for (i32 i = 0; i < n; i++)
		{
			i32 tableIdx = (src[i].m_key >> startBit) & (NUM_TABLES - 1);

			dst[tables[tableIdx] + counter[tableIdx]] = src[i];
			counter[tableIdx]++;
		}

		b3Swap(src, dst);
		count++;
	}

	if (count & 1)
	{
		drx3DAssert(0);  //need to copy
	}
}

void b3RadixSort32CL::executeHost(b3OpenCLArray<b3SortData>& keyValuesInOut, i32 sortBits /* = 32 */)
{
	b3AlignedObjectArray<b3SortData> inout;
	keyValuesInOut.copyToHost(inout);

	executeHost(inout, sortBits);

	keyValuesInOut.copyFromHost(inout);
}

void b3RadixSort32CL::execute(b3OpenCLArray<u32>& keysIn, b3OpenCLArray<u32>& keysOut, b3OpenCLArray<u32>& valuesIn,
							  b3OpenCLArray<u32>& valuesOut, i32 n, i32 sortBits)
{
}

//#define DEBUG_RADIXSORT
//#define DEBUG_RADIXSORT2

void b3RadixSort32CL::execute(b3OpenCLArray<b3SortData>& keyValuesInOut, i32 sortBits /* = 32 */)
{
	i32 originalSize = keyValuesInOut.size();
	i32 workingSize = originalSize;

	i32 dataAlignment = DATA_ALIGNMENT;

#ifdef DEBUG_RADIXSORT2
	b3AlignedObjectArray<b3SortData> test2;
	keyValuesInOut.copyToHost(test2);
	printf("numElem = %d\n", test2.size());
	for (i32 i = 0; i < test2.size(); i++)
	{
		printf("test2[%d].m_key=%d\n", i, test2[i].m_key);
		printf("test2[%d].m_value=%d\n", i, test2[i].m_value);
	}
#endif  //DEBUG_RADIXSORT2

	b3OpenCLArray<b3SortData>* src = 0;

	if (workingSize % dataAlignment)
	{
		workingSize += dataAlignment - (workingSize % dataAlignment);
		m_workBuffer4->copyFromOpenCLArray(keyValuesInOut);
		m_workBuffer4->resize(workingSize);
		b3SortData fillValue;
		fillValue.m_key = 0xffffffff;
		fillValue.m_value = 0xffffffff;

#define USE_BTFILL
#ifdef USE_BTFILL
		m_fill->execute((b3OpenCLArray<b3Int2>&)*m_workBuffer4, (b3Int2&)fillValue, workingSize - originalSize, originalSize);
#else
		//fill the remaining bits (very slow way, todo: fill on GPU/OpenCL side)

		for (i32 i = originalSize; i < workingSize; i++)
		{
			m_workBuffer4->copyFromHostPointer(&fillValue, 1, i);
		}
#endif  //USE_BTFILL

		src = m_workBuffer4;
	}
	else
	{
		src = &keyValuesInOut;
		m_workBuffer4->resize(0);
	}

	drx3DAssert(workingSize % DATA_ALIGNMENT == 0);
	i32 minCap = NUM_BUCKET * NUM_WGS;

	i32 n = workingSize;

	m_workBuffer1->resize(minCap);
	m_workBuffer3->resize(workingSize);

	//	ADLASSERT( ELEMENTS_PER_WORK_ITEM == 4 );
	drx3DAssert(BITS_PER_PASS == 4);
	drx3DAssert(WG_SIZE == 64);
	drx3DAssert((sortBits & 0x3) == 0);

	b3OpenCLArray<b3SortData>* dst = m_workBuffer3;

	b3OpenCLArray<u32>* srcHisto = m_workBuffer1;
	b3OpenCLArray<u32>* destHisto = m_workBuffer2;

	i32 nWGs = NUM_WGS;
	b3ConstData cdata;

	{
		i32 blockSize = ELEMENTS_PER_WORK_ITEM * WG_SIZE;  //set at 256
		i32 nBlocks = (n + blockSize - 1) / (blockSize);
		cdata.m_n = n;
		cdata.m_nWGs = NUM_WGS;
		cdata.m_startBit = 0;
		cdata.m_nBlocksPerWG = (nBlocks + cdata.m_nWGs - 1) / cdata.m_nWGs;
		if (nBlocks < NUM_WGS)
		{
			cdata.m_nBlocksPerWG = 1;
			nWGs = nBlocks;
		}
	}

	i32 count = 0;
	for (i32 ib = 0; ib < sortBits; ib += 4)
	{
#ifdef DEBUG_RADIXSORT2
		keyValuesInOut.copyToHost(test2);
		printf("numElem = %d\n", test2.size());
		for (i32 i = 0; i < test2.size(); i++)
		{
			if (test2[i].m_key != test2[i].m_value)
			{
				printf("test2[%d].m_key=%d\n", i, test2[i].m_key);
				printf("test2[%d].m_value=%d\n", i, test2[i].m_value);
			}
		}
#endif  //DEBUG_RADIXSORT2

		cdata.m_startBit = ib;

		if (src->size())
		{
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src->getBufferCL(), true), b3BufferInfoCL(srcHisto->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_streamCountSortDataKernel, "m_streamCountSortDataKernel");

			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);

			i32 num = NUM_WGS * WG_SIZE;
			launcher.launch1D(num, WG_SIZE);
		}

#ifdef DEBUG_RADIXSORT
		b3AlignedObjectArray<u32> testHist;
		srcHisto->copyToHost(testHist);
		printf("ib = %d, testHist size = %d, non zero elements:\n", ib, testHist.size());
		for (i32 i = 0; i < testHist.size(); i++)
		{
			if (testHist[i] != 0)
				printf("testHist[%d]=%d\n", i, testHist[i]);
		}
#endif  //DEBUG_RADIXSORT

//fast prefix scan is not working properly on Mac OSX yet
#ifdef __APPLE__
		bool fastScan = false;
#else
		bool fastScan = !m_deviceCPU;  //only use fast scan on GPU
#endif

		if (fastScan)
		{  //	prefix scan group histogram
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(srcHisto->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_prefixScanKernel, "m_prefixScanKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);
			launcher.launch1D(128, 128);
			destHisto = srcHisto;
		}
		else
		{
			//u32 sum; //for debugging
			m_scan->execute(*srcHisto, *destHisto, 1920, 0);  //,&sum);
		}

#ifdef DEBUG_RADIXSORT
		destHisto->copyToHost(testHist);
		printf("ib = %d, testHist size = %d, non zero elements:\n", ib, testHist.size());
		for (i32 i = 0; i < testHist.size(); i++)
		{
			if (testHist[i] != 0)
				printf("testHist[%d]=%d\n", i, testHist[i]);
		}

		for (i32 i = 0; i < testHist.size(); i += NUM_WGS)
		{
			printf("testHist[%d]=%d\n", i / NUM_WGS, testHist[i]);
		}

#endif  //DEBUG_RADIXSORT

#define USE_GPU
#ifdef USE_GPU

		if (src->size())
		{  //	local sort and distribute
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src->getBufferCL(), true), b3BufferInfoCL(destHisto->getBufferCL(), true), b3BufferInfoCL(dst->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_sortAndScatterSortDataKernel, "m_sortAndScatterSortDataKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);
			launcher.launch1D(nWGs * WG_SIZE, WG_SIZE);
		}
#else
		{
#define NUM_TABLES 16
//#define SEQUENTIAL
#ifdef SEQUENTIAL
			i32 counter2[NUM_TABLES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			i32 tables[NUM_TABLES];
			i32 startBit = ib;

			destHisto->copyToHost(testHist);
			b3AlignedObjectArray<b3SortData> srcHost;
			b3AlignedObjectArray<b3SortData> dstHost;
			dstHost.resize(src->size());

			src->copyToHost(srcHost);

			for (i32 i = 0; i < NUM_TABLES; i++)
			{
				tables[i] = testHist[i * NUM_WGS];
			}

			//	distribute
			for (i32 i = 0; i < n; i++)
			{
				i32 tableIdx = (srcHost[i].m_key >> startBit) & (NUM_TABLES - 1);

				dstHost[tables[tableIdx] + counter2[tableIdx]] = srcHost[i];
				counter2[tableIdx]++;
			}

#else

			i32 counter2[NUM_TABLES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			i32 tables[NUM_TABLES];
			b3AlignedObjectArray<b3SortData> dstHostOK;
			dstHostOK.resize(src->size());

			destHisto->copyToHost(testHist);
			b3AlignedObjectArray<b3SortData> srcHost;
			src->copyToHost(srcHost);

			i32 blockSize = 256;
			i32 nBlocksPerWG = cdata.m_nBlocksPerWG;
			i32 startBit = ib;

			{
				for (i32 i = 0; i < NUM_TABLES; i++)
				{
					tables[i] = testHist[i * NUM_WGS];
				}

				//	distribute
				for (i32 i = 0; i < n; i++)
				{
					i32 tableIdx = (srcHost[i].m_key >> startBit) & (NUM_TABLES - 1);

					dstHostOK[tables[tableIdx] + counter2[tableIdx]] = srcHost[i];
					counter2[tableIdx]++;
				}
			}

			b3AlignedObjectArray<b3SortData> dstHost;
			dstHost.resize(src->size());

			i32 counter[NUM_TABLES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			for (i32 wgIdx = 0; wgIdx < NUM_WGS; wgIdx++)
			{
				i32 counter[NUM_TABLES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

				i32 nBlocks = (n) / blockSize - nBlocksPerWG * wgIdx;

				for (i32 iblock = 0; iblock < d3Min(cdata.m_nBlocksPerWG, nBlocks); iblock++)
				{
					for (i32 lIdx = 0; lIdx < 64; lIdx++)
					{
						i32 addr = iblock * blockSize + blockSize * cdata.m_nBlocksPerWG * wgIdx + ELEMENTS_PER_WORK_ITEM * lIdx;

						//	MY_HISTOGRAM( localKeys.x ) ++ is much expensive than atomic add as it requires read and write while atomics can just add on AMD
						//	Using registers didn't perform well. It seems like use localKeys to address requires a lot of alu ops
						//	AMD: AtomInc performs better while NV prefers ++
						for (i32 j = 0; j < ELEMENTS_PER_WORK_ITEM; j++)
						{
							if (addr + j < n)
							{
								//  printf ("addr+j=%d\n", addr+j);

								i32 i = addr + j;

								i32 tableIdx = (srcHost[i].m_key >> startBit) & (NUM_TABLES - 1);

								i32 destIndex = testHist[tableIdx * NUM_WGS + wgIdx] + counter[tableIdx];

								b3SortData ok = dstHostOK[destIndex];

								if (ok.m_key != srcHost[i].m_key)
								{
									printf("ok.m_key = %d, srcHost[i].m_key = %d\n", ok.m_key, srcHost[i].m_key);
									printf("(ok.m_value = %d, srcHost[i].m_value = %d)\n", ok.m_value, srcHost[i].m_value);
								}
								if (ok.m_value != srcHost[i].m_value)
								{
									printf("ok.m_value = %d, srcHost[i].m_value = %d\n", ok.m_value, srcHost[i].m_value);
									printf("(ok.m_key = %d, srcHost[i].m_key = %d)\n", ok.m_key, srcHost[i].m_key);
								}

								dstHost[destIndex] = srcHost[i];
								counter[tableIdx]++;
							}
						}
					}
				}
			}

#endif  //SEQUENTIAL

			dst->copyFromHost(dstHost);
		}
#endif  //USE_GPU

#ifdef DEBUG_RADIXSORT
		destHisto->copyToHost(testHist);
		printf("ib = %d, testHist size = %d, non zero elements:\n", ib, testHist.size());
		for (i32 i = 0; i < testHist.size(); i++)
		{
			if (testHist[i] != 0)
				printf("testHist[%d]=%d\n", i, testHist[i]);
		}
#endif  //DEBUG_RADIXSORT
		b3Swap(src, dst);
		b3Swap(srcHisto, destHisto);

#ifdef DEBUG_RADIXSORT2
		keyValuesInOut.copyToHost(test2);
		printf("numElem = %d\n", test2.size());
		for (i32 i = 0; i < test2.size(); i++)
		{
			if (test2[i].m_key != test2[i].m_value)
			{
				printf("test2[%d].m_key=%d\n", i, test2[i].m_key);
				printf("test2[%d].m_value=%d\n", i, test2[i].m_value);
			}
		}
#endif  //DEBUG_RADIXSORT2

		count++;
	}

	if (count & 1)
	{
		drx3DAssert(0);  //need to copy from workbuffer to keyValuesInOut
	}

	if (m_workBuffer4->size())
	{
		m_workBuffer4->resize(originalSize);
		keyValuesInOut.copyFromOpenCLArray(*m_workBuffer4);
	}

#ifdef DEBUG_RADIXSORT
	keyValuesInOut.copyToHost(test2);

	printf("numElem = %d\n", test2.size());
	for (i32 i = 0; i < test2.size(); i++)
	{
		printf("test2[%d].m_key=%d\n", i, test2[i].m_key);
		printf("test2[%d].m_value=%d\n", i, test2[i].m_value);
	}
#endif
}

void b3RadixSort32CL::execute(b3OpenCLArray<u32>& keysInOut, i32 sortBits /* = 32 */)
{
	i32 originalSize = keysInOut.size();
	i32 workingSize = originalSize;

	i32 dataAlignment = DATA_ALIGNMENT;

	b3OpenCLArray<u32>* src = 0;

	if (workingSize % dataAlignment)
	{
		workingSize += dataAlignment - (workingSize % dataAlignment);
		m_workBuffer4a->copyFromOpenCLArray(keysInOut);
		m_workBuffer4a->resize(workingSize);
		u32 fillValue = 0xffffffff;

		m_fill->execute(*m_workBuffer4a, fillValue, workingSize - originalSize, originalSize);

		src = m_workBuffer4a;
	}
	else
	{
		src = &keysInOut;
		m_workBuffer4a->resize(0);
	}

	drx3DAssert(workingSize % DATA_ALIGNMENT == 0);
	i32 minCap = NUM_BUCKET * NUM_WGS;

	i32 n = workingSize;

	m_workBuffer1->resize(minCap);
	m_workBuffer3->resize(workingSize);
	m_workBuffer3a->resize(workingSize);

	//	ADLASSERT( ELEMENTS_PER_WORK_ITEM == 4 );
	drx3DAssert(BITS_PER_PASS == 4);
	drx3DAssert(WG_SIZE == 64);
	drx3DAssert((sortBits & 0x3) == 0);

	b3OpenCLArray<u32>* dst = m_workBuffer3a;

	b3OpenCLArray<u32>* srcHisto = m_workBuffer1;
	b3OpenCLArray<u32>* destHisto = m_workBuffer2;

	i32 nWGs = NUM_WGS;
	b3ConstData cdata;

	{
		i32 blockSize = ELEMENTS_PER_WORK_ITEM * WG_SIZE;  //set at 256
		i32 nBlocks = (n + blockSize - 1) / (blockSize);
		cdata.m_n = n;
		cdata.m_nWGs = NUM_WGS;
		cdata.m_startBit = 0;
		cdata.m_nBlocksPerWG = (nBlocks + cdata.m_nWGs - 1) / cdata.m_nWGs;
		if (nBlocks < NUM_WGS)
		{
			cdata.m_nBlocksPerWG = 1;
			nWGs = nBlocks;
		}
	}

	i32 count = 0;
	for (i32 ib = 0; ib < sortBits; ib += 4)
	{
		cdata.m_startBit = ib;

		if (src->size())
		{
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src->getBufferCL(), true), b3BufferInfoCL(srcHisto->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_streamCountKernel, "m_streamCountKernel");

			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);

			i32 num = NUM_WGS * WG_SIZE;
			launcher.launch1D(num, WG_SIZE);
		}

//fast prefix scan is not working properly on Mac OSX yet
#ifdef __APPLE__
		bool fastScan = false;
#else
		bool fastScan = !m_deviceCPU;
#endif

		if (fastScan)
		{  //	prefix scan group histogram
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(srcHisto->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_prefixScanKernel, "m_prefixScanKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);
			launcher.launch1D(128, 128);
			destHisto = srcHisto;
		}
		else
		{
			//u32 sum; //for debugging
			m_scan->execute(*srcHisto, *destHisto, 1920, 0);  //,&sum);
		}

		if (src->size())
		{  //	local sort and distribute
			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src->getBufferCL(), true), b3BufferInfoCL(destHisto->getBufferCL(), true), b3BufferInfoCL(dst->getBufferCL())};
			b3LauncherCL launcher(m_commandQueue, m_sortAndScatterKernel, "m_sortAndScatterKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			launcher.setConst(cdata);
			launcher.launch1D(nWGs * WG_SIZE, WG_SIZE);
		}

		b3Swap(src, dst);
		b3Swap(srcHisto, destHisto);

		count++;
	}

	if (count & 1)
	{
		drx3DAssert(0);  //need to copy from workbuffer to keyValuesInOut
	}

	if (m_workBuffer4a->size())
	{
		m_workBuffer4a->resize(originalSize);
		keysInOut.copyFromOpenCLArray(*m_workBuffer4a);
	}
}
