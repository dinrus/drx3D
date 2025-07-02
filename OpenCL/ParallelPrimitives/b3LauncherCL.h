
#ifndef D3_LAUNCHER_CL_H
#define D3_LAUNCHER_CL_H

#include <drx3D/OpenCL/ParallelPrimitives/b3BufferInfoCL.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <stdio.h>

#define D3_DEBUG_SERIALIZE_CL

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif
#define D3_CL_MAX_ARG_SIZE 16
D3_ATTRIBUTE_ALIGNED16(struct)
b3KernelArgData
{
	i32 m_isBuffer;
	i32 m_argIndex;
	i32 m_argSizeInBytes;
	i32 m_unusedPadding;
	union {
		cl_mem m_clBuffer;
		u8 m_argData[D3_CL_MAX_ARG_SIZE];
	};
};

class b3LauncherCL
{
	cl_command_queue m_commandQueue;
	cl_kernel m_kernel;
	i32 m_idx;

	b3AlignedObjectArray<b3KernelArgData> m_kernelArguments;
	i32 m_serializationSizeInBytes;
	bool m_enableSerialization;

	tukk m_name;

public:
	b3AlignedObjectArray<b3OpenCLArray<u8>*> m_arrays;

	b3LauncherCL(cl_command_queue queue, cl_kernel kernel, tukk name);

	virtual ~b3LauncherCL();

	void setBuffer(cl_mem clBuffer);

	void setBuffers(b3BufferInfoCL* buffInfo, i32 n);

	i32 getSerializationBufferSize() const
	{
		return m_serializationSizeInBytes;
	}

	i32 deserializeArgs(u8* buf, i32 bufSize, cl_context ctx);

	inline i32 validateResults(u8* goldBuffer, i32 goldBufferCapacity, cl_context ctx);

	i32 serializeArguments(u8* destBuffer, i32 destBufferCapacity);

	i32 getNumArguments() const
	{
		return m_kernelArguments.size();
	}

	b3KernelArgData getArgument(i32 index)
	{
		return m_kernelArguments[index];
	}

	void serializeToFile(tukk fileName, i32 numWorkItems);

	template <typename T>
	inline void setConst(const T& consts)
	{
		i32 sz = sizeof(T);
		drx3DAssert(sz <= D3_CL_MAX_ARG_SIZE);

		if (m_enableSerialization)
		{
			b3KernelArgData kernelArg;
			kernelArg.m_argIndex = m_idx;
			kernelArg.m_isBuffer = 0;
			T* destArg = (T*)kernelArg.m_argData;
			*destArg = consts;
			kernelArg.m_argSizeInBytes = sizeof(T);
			m_kernelArguments.push_back(kernelArg);
			m_serializationSizeInBytes += sizeof(b3KernelArgData);
		}

		cl_int status = clSetKernelArg(m_kernel, m_idx++, sz, &consts);
		drx3DAssert(status == CL_SUCCESS);
	}

	inline void launch1D(i32 numThreads, i32 localSize = 64)
	{
		launch2D(numThreads, 1, localSize, 1);
	}

	inline void launch2D(i32 numThreadsX, i32 numThreadsY, i32 localSizeX, i32 localSizeY)
	{
		size_t gRange[3] = {1, 1, 1};
		size_t lRange[3] = {1, 1, 1};
		lRange[0] = localSizeX;
		lRange[1] = localSizeY;
		gRange[0] = d3Max((size_t)1, (numThreadsX / lRange[0]) + (!(numThreadsX % lRange[0]) ? 0 : 1));
		gRange[0] *= lRange[0];
		gRange[1] = d3Max((size_t)1, (numThreadsY / lRange[1]) + (!(numThreadsY % lRange[1]) ? 0 : 1));
		gRange[1] *= lRange[1];

		cl_int status = clEnqueueNDRangeKernel(m_commandQueue,
											   m_kernel, 2, NULL, gRange, lRange, 0, 0, 0);
		if (status != CL_SUCCESS)
		{
			printf("Ошибка: OpenCL status = %d\n", status);
		}
		drx3DAssert(status == CL_SUCCESS);
	}

	void enableSerialization(bool serialize)
	{
		m_enableSerialization = serialize;
	}
};

#endif  //D3_LAUNCHER_CL_H
