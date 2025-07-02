
#ifndef D3_RADIXSORT32_H
#define D3_RADIXSORT32_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>

struct b3SortData
{
	union {
		u32 m_key;
		u32 x;
	};

	union {
		u32 m_value;
		u32 y;
	};
};
#include <drx3D/OpenCL/ParallelPrimitives/b3BufferInfoCL.h>

class b3RadixSort32CL
{
	b3OpenCLArray<u32>* m_workBuffer1;
	b3OpenCLArray<u32>* m_workBuffer2;

	b3OpenCLArray<b3SortData>* m_workBuffer3;
	b3OpenCLArray<b3SortData>* m_workBuffer4;

	b3OpenCLArray<u32>* m_workBuffer3a;
	b3OpenCLArray<u32>* m_workBuffer4a;

	cl_command_queue m_commandQueue;

	cl_kernel m_streamCountSortDataKernel;
	cl_kernel m_streamCountKernel;

	cl_kernel m_prefixScanKernel;
	cl_kernel m_sortAndScatterSortDataKernel;
	cl_kernel m_sortAndScatterKernel;

	bool m_deviceCPU;

	class b3PrefixScanCL* m_scan;
	class b3FillCL* m_fill;

public:
	struct b3ConstData
	{
		i32 m_n;
		i32 m_nWGs;
		i32 m_startBit;
		i32 m_nBlocksPerWG;
	};
	enum
	{
		DATA_ALIGNMENT = 256,
		WG_SIZE = 64,
		BLOCK_SIZE = 256,
		ELEMENTS_PER_WORK_ITEM = (BLOCK_SIZE / WG_SIZE),
		BITS_PER_PASS = 4,
		NUM_BUCKET = (1 << BITS_PER_PASS),
		//	if you change this, change nPerWI in kernel as well
		NUM_WGS = 20 * 6,  //	cypress
						   //			NUM_WGS = 24*6,	//	cayman
						   //			NUM_WGS = 32*4,	//	nv
	};

private:
public:
	b3RadixSort32CL(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 initialCapacity = 0);

	virtual ~b3RadixSort32CL();

	void execute(b3OpenCLArray<u32>& keysIn, b3OpenCLArray<u32>& keysOut, b3OpenCLArray<u32>& valuesIn,
				 b3OpenCLArray<u32>& valuesOut, i32 n, i32 sortBits = 32);

	///keys only
	void execute(b3OpenCLArray<u32>& keysInOut, i32 sortBits = 32);

	void execute(b3OpenCLArray<b3SortData>& keyValuesInOut, i32 sortBits = 32);
	void executeHost(b3OpenCLArray<b3SortData>& keyValuesInOut, i32 sortBits = 32);
	void executeHost(b3AlignedObjectArray<b3SortData>& keyValuesInOut, i32 sortBits = 32);
};
#endif  //D3_RADIXSORT32_H
