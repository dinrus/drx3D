
#ifndef D3_PREFIX_SCAN_CL_H
#define D3_PREFIX_SCAN_CL_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3BufferInfoCL.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Vec3.h>

class b3PrefixScanFloat4CL
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

	b3OpenCLArray<b3Vec3>* m_workBuffer;

public:
	b3PrefixScanFloat4CL(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 size = 0);

	virtual ~b3PrefixScanFloat4CL();

	void execute(b3OpenCLArray<b3Vec3>& src, b3OpenCLArray<b3Vec3>& dst, i32 n, b3Vec3* sum = 0);
	void executeHost(b3AlignedObjectArray<b3Vec3>& src, b3AlignedObjectArray<b3Vec3>& dst, i32 n, b3Vec3* sum);
};

#endif  //D3_PREFIX_SCAN_CL_H
