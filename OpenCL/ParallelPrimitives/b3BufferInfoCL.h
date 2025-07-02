#ifndef D3_BUFFER_INFO_CL_H
#define D3_BUFFER_INFO_CL_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>

struct b3BufferInfoCL
{
	//b3BufferInfoCL(){}

	//	template<typename T>
	b3BufferInfoCL(cl_mem buff, bool isReadOnly = false) : m_clBuffer(buff), m_isReadOnly(isReadOnly) {}

	cl_mem m_clBuffer;
	bool m_isReadOnly;
};

#endif  //D3_BUFFER_INFO_CL_H
