#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanFloat4CL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>
#define D3_PREFIXSCAN_FLOAT4_PROG_PATH "drx3D/Bullet3/OpenCL/ParallelPrimitives/kernels/PrefixScanFloat4Kernels.cl"

#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/OpenCL/ParallelPrimitives/kernels/PrefixScanKernelsFloat4CL.h>

b3PrefixScanFloat4CL::b3PrefixScanFloat4CL(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 size)
	: m_commandQueue(queue)
{
	tukk scanKernelSource = prefixScanKernelsFloat4CL;
	cl_int pErrNum;
	tuk additionalMacros = 0;

	m_workBuffer = new b3OpenCLArray<b3Vec3>(ctx, queue, size);
	cl_program scanProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, scanKernelSource, &pErrNum, additionalMacros, D3_PREFIXSCAN_FLOAT4_PROG_PATH);
	drx3DAssert(scanProg);

	m_localScanKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, scanKernelSource, "LocalScanKernel", &pErrNum, scanProg, additionalMacros);
	drx3DAssert(m_localScanKernel);
	m_blockSumKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, scanKernelSource, "TopLevelScanKernel", &pErrNum, scanProg, additionalMacros);
	drx3DAssert(m_blockSumKernel);
	m_propagationKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, scanKernelSource, "AddOffsetKernel", &pErrNum, scanProg, additionalMacros);
	drx3DAssert(m_propagationKernel);
}

b3PrefixScanFloat4CL::~b3PrefixScanFloat4CL()
{
	delete m_workBuffer;
	clReleaseKernel(m_localScanKernel);
	clReleaseKernel(m_blockSumKernel);
	clReleaseKernel(m_propagationKernel);
}

template <class T>
T b3NextPowerOf2(T n)
{
	n -= 1;
	for (i32 i = 0; i < sizeof(T) * 8; i++)
		n = n | (n >> i);
	return n + 1;
}

void b3PrefixScanFloat4CL::execute(b3OpenCLArray<b3Vec3>& src, b3OpenCLArray<b3Vec3>& dst, i32 n, b3Vec3* sum)
{
	//	drx3DAssert( data->m_option == EXCLUSIVE );
	u32k numBlocks = (u32k)((n + BLOCK_SIZE * 2 - 1) / (BLOCK_SIZE * 2));

	dst.resize(src.size());
	m_workBuffer->resize(src.size());

	b3Int4 constBuffer;
	constBuffer.x = n;
	constBuffer.y = numBlocks;
	constBuffer.z = (i32)b3NextPowerOf2(numBlocks);

	b3OpenCLArray<b3Vec3>* srcNative = &src;
	b3OpenCLArray<b3Vec3>* dstNative = &dst;

	{
		b3BufferInfoCL bInfo[] = {b3BufferInfoCL(dstNative->getBufferCL()), b3BufferInfoCL(srcNative->getBufferCL()), b3BufferInfoCL(m_workBuffer->getBufferCL())};

		b3LauncherCL launcher(m_commandQueue, m_localScanKernel, "m_localScanKernel");
		launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
		launcher.setConst(constBuffer);
		launcher.launch1D(numBlocks * BLOCK_SIZE, BLOCK_SIZE);
	}

	{
		b3BufferInfoCL bInfo[] = {b3BufferInfoCL(m_workBuffer->getBufferCL())};

		b3LauncherCL launcher(m_commandQueue, m_blockSumKernel, "m_blockSumKernel");
		launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
		launcher.setConst(constBuffer);
		launcher.launch1D(BLOCK_SIZE, BLOCK_SIZE);
	}

	if (numBlocks > 1)
	{
		b3BufferInfoCL bInfo[] = {b3BufferInfoCL(dstNative->getBufferCL()), b3BufferInfoCL(m_workBuffer->getBufferCL())};
		b3LauncherCL launcher(m_commandQueue, m_propagationKernel, "m_propagationKernel");
		launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
		launcher.setConst(constBuffer);
		launcher.launch1D((numBlocks - 1) * BLOCK_SIZE, BLOCK_SIZE);
	}

	if (sum)
	{
		clFinish(m_commandQueue);
		dstNative->copyToHostPointer(sum, 1, n - 1, true);
	}
}

void b3PrefixScanFloat4CL::executeHost(b3AlignedObjectArray<b3Vec3>& src, b3AlignedObjectArray<b3Vec3>& dst, i32 n, b3Vec3* sum)
{
	b3Vec3 s = b3MakeVector3(0, 0, 0);
	//if( data->m_option == EXCLUSIVE )
	{
		for (i32 i = 0; i < n; i++)
		{
			dst[i] = s;
			s += src[i];
		}
	}
	/*else
	{
		for(i32 i=0; i<n; i++)
		{
			s += hSrc[i];
			hDst[i] = s;
		}
	}
	*/

	if (sum)
	{
		*sum = dst[n - 1];
	}
}