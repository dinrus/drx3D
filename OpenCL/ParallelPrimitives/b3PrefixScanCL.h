
#ifndef D3_PREFIX_SCAN_CL_H
#define D3_PREFIX_SCAN_CL_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3BufferInfoCL.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

class b3PrefixScanCL
{
	enum
	{
		BLOCK_SIZE = 128
	};

	//	Option m_option;

	cl_command_queue m_commandQueue;

	cl_kernel m_localScanKernel;
	cl_kernel m_blockSumKernel;
	cl_kernel m_propagationKernel;

	b3OpenCLArray<u32>* m_workBuffer;

public:
	b3PrefixScanCL(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 size = 0);

	virtual ~b3PrefixScanCL();

	void execute(b3OpenCLArray<u32>& src, b3OpenCLArray<u32>& dst, i32 n, u32* sum = 0);
	void executeHost(b3AlignedObjectArray<u32>& src, b3AlignedObjectArray<u32>& dst, i32 n, u32* sum = 0);
};

#endif  //D3_PREFIX_SCAN_CL_H
