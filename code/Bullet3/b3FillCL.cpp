#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>
#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3BufferInfoCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>

#define FILL_CL_PROGRAM_PATH "drx3D/Bullet3/OpenCL/ParallelPrimitives/kernels/FillKernels.cl"

#include <drx3D/OpenCL/ParallelPrimitives/kernels/FillKernelsCL.h>

b3FillCL::b3FillCL(cl_context ctx, cl_device_id device, cl_command_queue queue)
	: m_commandQueue(queue)
{
	tukk kernelSource = fillKernelsCL;
	cl_int pErrNum;
	tukk additionalMacros = "";

	cl_program fillProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, kernelSource, &pErrNum, additionalMacros, FILL_CL_PROGRAM_PATH);
	drx3DAssert(fillProg);

	m_fillIntKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "FillIntKernel", &pErrNum, fillProg, additionalMacros);
	drx3DAssert(m_fillIntKernel);

	m_fillUnsignedIntKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "FillUnsignedIntKernel", &pErrNum, fillProg, additionalMacros);
	drx3DAssert(m_fillIntKernel);

	m_fillFloatKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "FillFloatKernel", &pErrNum, fillProg, additionalMacros);
	drx3DAssert(m_fillFloatKernel);

	m_fillKernelInt2 = b3OpenCLUtils::compileCLKernelFromString(ctx, device, kernelSource, "FillInt2Kernel", &pErrNum, fillProg, additionalMacros);
	drx3DAssert(m_fillKernelInt2);
}

b3FillCL::~b3FillCL()
{
	clReleaseKernel(m_fillKernelInt2);
	clReleaseKernel(m_fillIntKernel);
	clReleaseKernel(m_fillUnsignedIntKernel);
	clReleaseKernel(m_fillFloatKernel);
}

void b3FillCL::execute(b3OpenCLArray<float>& src, const float value, i32 n, i32 offset)
{
	drx3DAssert(n > 0);

	{
		b3LauncherCL launcher(m_commandQueue, m_fillFloatKernel, "m_fillFloatKernel");
		launcher.setBuffer(src.getBufferCL());
		launcher.setConst(n);
		launcher.setConst(value);
		launcher.setConst(offset);

		launcher.launch1D(n);
	}
}

void b3FillCL::execute(b3OpenCLArray<i32>& src, i32k value, i32 n, i32 offset)
{
	drx3DAssert(n > 0);

	{
		b3LauncherCL launcher(m_commandQueue, m_fillIntKernel, "m_fillIntKernel");
		launcher.setBuffer(src.getBufferCL());
		launcher.setConst(n);
		launcher.setConst(value);
		launcher.setConst(offset);
		launcher.launch1D(n);
	}
}

void b3FillCL::execute(b3OpenCLArray<u32>& src, u32k value, i32 n, i32 offset)
{
	drx3DAssert(n > 0);

	{
		b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src.getBufferCL())};

		b3LauncherCL launcher(m_commandQueue, m_fillUnsignedIntKernel, "m_fillUnsignedIntKernel");
		launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
		launcher.setConst(n);
		launcher.setConst(value);
		launcher.setConst(offset);

		launcher.launch1D(n);
	}
}

void b3FillCL::executeHost(b3AlignedObjectArray<b3Int2>& src, const b3Int2& value, i32 n, i32 offset)
{
	for (i32 i = 0; i < n; i++)
	{
		src[i + offset] = value;
	}
}

void b3FillCL::executeHost(b3AlignedObjectArray<i32>& src, i32k value, i32 n, i32 offset)
{
	for (i32 i = 0; i < n; i++)
	{
		src[i + offset] = value;
	}
}

void b3FillCL::execute(b3OpenCLArray<b3Int2>& src, const b3Int2& value, i32 n, i32 offset)
{
	drx3DAssert(n > 0);

	{
		b3BufferInfoCL bInfo[] = {b3BufferInfoCL(src.getBufferCL())};

		b3LauncherCL launcher(m_commandQueue, m_fillKernelInt2, "m_fillKernelInt2");
		launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
		launcher.setConst(n);
		launcher.setConst(value);
		launcher.setConst(offset);

		//( constBuffer );
		launcher.launch1D(n);
	}
}
