#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>

bool gDebugLauncherCL = false;

b3LauncherCL::b3LauncherCL(cl_command_queue queue, cl_kernel kernel, tukk name)
	: m_commandQueue(queue),
	  m_kernel(kernel),
	  m_idx(0),
	  m_enableSerialization(false),
	  m_name(name)
{
	if (gDebugLauncherCL)
	{
		static i32 counter = 0;
		printf("[%d] Prepare to launch OpenCL kernel %s\n", counter++, name);
	}

	m_serializationSizeInBytes = sizeof(i32);
}

b3LauncherCL::~b3LauncherCL()
{
	for (i32 i = 0; i < m_arrays.size(); i++)
	{
		delete (m_arrays[i]);
	}

	m_arrays.clear();
	if (gDebugLauncherCL)
	{
		static i32 counter = 0;
		printf("[%d] Finished launching OpenCL kernel %s\n", counter++, m_name);
	}
}

void b3LauncherCL::setBuffer(cl_mem clBuffer)
{
	if (m_enableSerialization)
	{
		b3KernelArgData kernelArg;
		kernelArg.m_argIndex = m_idx;
		kernelArg.m_isBuffer = 1;
		kernelArg.m_clBuffer = clBuffer;

		cl_mem_info param_name = CL_MEM_SIZE;
		size_t param_value;
		size_t sizeInBytes = sizeof(size_t);
		size_t actualSizeInBytes;
		cl_int err;
		err = clGetMemObjectInfo(kernelArg.m_clBuffer,
								 param_name,
								 sizeInBytes,
								 &param_value,
								 &actualSizeInBytes);

		drx3DAssert(err == CL_SUCCESS);
		kernelArg.m_argSizeInBytes = param_value;

		m_kernelArguments.push_back(kernelArg);
		m_serializationSizeInBytes += sizeof(b3KernelArgData);
		m_serializationSizeInBytes += param_value;
	}
	cl_int status = clSetKernelArg(m_kernel, m_idx++, sizeof(cl_mem), &clBuffer);
	drx3DAssert(status == CL_SUCCESS);
}

void b3LauncherCL::setBuffers(b3BufferInfoCL* buffInfo, i32 n)
{
	for (i32 i = 0; i < n; i++)
	{
		if (m_enableSerialization)
		{
			b3KernelArgData kernelArg;
			kernelArg.m_argIndex = m_idx;
			kernelArg.m_isBuffer = 1;
			kernelArg.m_clBuffer = buffInfo[i].m_clBuffer;

			cl_mem_info param_name = CL_MEM_SIZE;
			size_t param_value;
			size_t sizeInBytes = sizeof(size_t);
			size_t actualSizeInBytes;
			cl_int err;
			err = clGetMemObjectInfo(kernelArg.m_clBuffer,
									 param_name,
									 sizeInBytes,
									 &param_value,
									 &actualSizeInBytes);

			drx3DAssert(err == CL_SUCCESS);
			kernelArg.m_argSizeInBytes = param_value;

			m_kernelArguments.push_back(kernelArg);
			m_serializationSizeInBytes += sizeof(b3KernelArgData);
			m_serializationSizeInBytes += param_value;
		}
		cl_int status = clSetKernelArg(m_kernel, m_idx++, sizeof(cl_mem), &buffInfo[i].m_clBuffer);
		drx3DAssert(status == CL_SUCCESS);
	}
}

struct b3KernelArgDataUnaligned
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
#include <string.h>

i32 b3LauncherCL::deserializeArgs(u8* buf, i32 bufSize, cl_context ctx)
{
	i32 index = 0;

	i32 numArguments = *(i32*)&buf[index];
	index += sizeof(i32);

	for (i32 i = 0; i < numArguments; i++)
	{
		b3KernelArgDataUnaligned* arg = (b3KernelArgDataUnaligned*)&buf[index];

		index += sizeof(b3KernelArgData);
		if (arg->m_isBuffer)
		{
			b3OpenCLArray<u8>* clData = new b3OpenCLArray<u8>(ctx, m_commandQueue, arg->m_argSizeInBytes);
			clData->resize(arg->m_argSizeInBytes);

			clData->copyFromHostPointer(&buf[index], arg->m_argSizeInBytes);

			arg->m_clBuffer = clData->getBufferCL();

			m_arrays.push_back(clData);

			cl_int status = clSetKernelArg(m_kernel, m_idx++, sizeof(cl_mem), &arg->m_clBuffer);
			drx3DAssert(status == CL_SUCCESS);
			index += arg->m_argSizeInBytes;
		}
		else
		{
			cl_int status = clSetKernelArg(m_kernel, m_idx++, arg->m_argSizeInBytes, &arg->m_argData);
			drx3DAssert(status == CL_SUCCESS);
		}
		b3KernelArgData b;
		memcpy(&b, arg, sizeof(b3KernelArgDataUnaligned));
		m_kernelArguments.push_back(b);
	}
	m_serializationSizeInBytes = index;
	return index;
}

i32 b3LauncherCL::validateResults(u8* goldBuffer, i32 goldBufferCapacity, cl_context ctx)
{
	i32 index = 0;

	i32 numArguments = *(i32*)&goldBuffer[index];
	index += sizeof(i32);

	if (numArguments != m_kernelArguments.size())
	{
		printf("failed validation: expected %d arguments, found %d\n", numArguments, m_kernelArguments.size());
		return -1;
	}

	for (i32 ii = 0; ii < numArguments; ii++)
	{
		b3KernelArgData* argGold = (b3KernelArgData*)&goldBuffer[index];

		if (m_kernelArguments[ii].m_argSizeInBytes != argGold->m_argSizeInBytes)
		{
			printf("failed validation: argument %d sizeInBytes expected: %d, found %d\n", ii, argGold->m_argSizeInBytes, m_kernelArguments[ii].m_argSizeInBytes);
			return -2;
		}

		{
			i32 expected = argGold->m_isBuffer;
			i32 found = m_kernelArguments[ii].m_isBuffer;

			if (expected != found)
			{
				printf("failed validation: argument %d isBuffer expected: %d, found %d\n", ii, expected, found);
				return -3;
			}
		}
		index += sizeof(b3KernelArgData);

		if (argGold->m_isBuffer)
		{
			u8* memBuf = (u8*)malloc(m_kernelArguments[ii].m_argSizeInBytes);
			u8* goldBuf = &goldBuffer[index];
			for (i32 j = 0; j < m_kernelArguments[j].m_argSizeInBytes; j++)
			{
				memBuf[j] = 0xaa;
			}

			cl_int status = 0;
			status = clEnqueueReadBuffer(m_commandQueue, m_kernelArguments[ii].m_clBuffer, CL_TRUE, 0, m_kernelArguments[ii].m_argSizeInBytes,
										 memBuf, 0, 0, 0);
			drx3DAssert(status == CL_SUCCESS);
			clFinish(m_commandQueue);

			for (i32 b = 0; b < m_kernelArguments[ii].m_argSizeInBytes; b++)
			{
				i32 expected = goldBuf[b];
				i32 found = memBuf[b];
				if (expected != found)
				{
					printf("failed validation: argument %d OpenCL data at byte position %d expected: %d, found %d\n",
						   ii, b, expected, found);
					return -4;
				}
			}

			index += argGold->m_argSizeInBytes;
		}
		else
		{
			//compare content
			for (i32 b = 0; b < m_kernelArguments[ii].m_argSizeInBytes; b++)
			{
				i32 expected = argGold->m_argData[b];
				i32 found = m_kernelArguments[ii].m_argData[b];
				if (expected != found)
				{
					printf("failed validation: argument %d const data at byte position %d expected: %d, found %d\n",
						   ii, b, expected, found);
					return -5;
				}
			}
		}
	}
	return index;
}

i32 b3LauncherCL::serializeArguments(u8* destBuffer, i32 destBufferCapacity)
{
	//initialize to known values
	for (i32 i = 0; i < destBufferCapacity; i++)
		destBuffer[i] = 0xec;

	assert(destBufferCapacity >= m_serializationSizeInBytes);

	//todo: use the b3Serializer for this to allow for 32/64bit, endianness etc
	i32 numArguments = m_kernelArguments.size();
	i32 curBufferSize = 0;
	i32* dest = (i32*)&destBuffer[curBufferSize];
	*dest = numArguments;
	curBufferSize += sizeof(i32);

	for (i32 i = 0; i < this->m_kernelArguments.size(); i++)
	{
		b3KernelArgData* arg = (b3KernelArgData*)&destBuffer[curBufferSize];
		*arg = m_kernelArguments[i];
		curBufferSize += sizeof(b3KernelArgData);
		if (arg->m_isBuffer == 1)
		{
			//copy the OpenCL buffer content
			cl_int status = 0;
			status = clEnqueueReadBuffer(m_commandQueue, arg->m_clBuffer, 0, 0, arg->m_argSizeInBytes,
										 &destBuffer[curBufferSize], 0, 0, 0);
			drx3DAssert(status == CL_SUCCESS);
			clFinish(m_commandQueue);
			curBufferSize += arg->m_argSizeInBytes;
		}
	}
	return curBufferSize;
}

void b3LauncherCL::serializeToFile(tukk fileName, i32 numWorkItems)
{
	i32 num = numWorkItems;
	i32 buffSize = getSerializationBufferSize();
	u8* buf = new u8[buffSize + sizeof(i32)];
	for (i32 i = 0; i < buffSize + 1; i++)
	{
		u8* ptr = (u8*)&buf[i];
		*ptr = 0xff;
	}
	//	i32 actualWrite = serializeArguments(buf,buffSize);

	//	u8* cptr = (u8*)&buf[buffSize];
	//            printf("buf[buffSize] = %d\n",*cptr);

	assert(buf[buffSize] == 0xff);  //check for buffer overrun
	i32* ptr = (i32*)&buf[buffSize];

	*ptr = num;

	FILE* f = fopen(fileName, "wb");
	fwrite(buf, buffSize + sizeof(i32), 1, f);
	fclose(f);

	delete[] buf;
}
