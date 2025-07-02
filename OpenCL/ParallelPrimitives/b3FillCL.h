#ifndef D3_FILL_CL_H
#define D3_FILL_CL_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/Common/b3Scalar.h>

#include <drx3D/Common/shared/b3Int2.h>
#include <drx3D/Common/shared/b3Int4.h>

class b3FillCL
{
	cl_command_queue m_commandQueue;

	cl_kernel m_fillKernelInt2;
	cl_kernel m_fillIntKernel;
	cl_kernel m_fillUnsignedIntKernel;
	cl_kernel m_fillFloatKernel;

public:
	struct b3ConstData
	{
		union {
			b3Int4 m_data;
			b3UnsignedInt4 m_UnsignedData;
		};
		i32 m_offset;
		i32 m_n;
		i32 m_padding[2];
	};

protected:
public:
	b3FillCL(cl_context ctx, cl_device_id device, cl_command_queue queue);

	virtual ~b3FillCL();

	void execute(b3OpenCLArray<u32>& src, u32k value, i32 n, i32 offset = 0);

	void execute(b3OpenCLArray<i32>& src, i32k value, i32 n, i32 offset = 0);

	void execute(b3OpenCLArray<float>& src, const float value, i32 n, i32 offset = 0);

	void execute(b3OpenCLArray<b3Int2>& src, const b3Int2& value, i32 n, i32 offset = 0);

	void executeHost(b3AlignedObjectArray<b3Int2>& src, const b3Int2& value, i32 n, i32 offset);

	void executeHost(b3AlignedObjectArray<i32>& src, i32k value, i32 n, i32 offset);

	//	void execute(b3OpenCLArray<b3Int4>& src, const b3Int4& value, i32 n, i32 offset = 0);
};

#endif  //D3_FILL_CL_H
