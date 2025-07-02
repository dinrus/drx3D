#ifndef D3_BOUNDSEARCH_H
#define D3_BOUNDSEARCH_H

#pragma once
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3FillCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>  //for b3SortData (perhaps move it?)

class b3BoundSearchCL
{
public:
	enum Option
	{
		BOUND_LOWER,
		BOUND_UPPER,
		COUNT,
	};

	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

	cl_kernel m_lowerSortDataKernel;
	cl_kernel m_upperSortDataKernel;
	cl_kernel m_subtractKernel;

	b3OpenCLArray<b3Int4>* m_constOpenCLArray;
	b3OpenCLArray<u32>* m_lower;
	b3OpenCLArray<u32>* m_upper;

	b3FillCL* m_filler;

	b3BoundSearchCL(cl_context context, cl_device_id device, cl_command_queue queue, i32 size);

	virtual ~b3BoundSearchCL();

	//	src has to be src[i].m_key <= src[i+1].m_key
	void execute(b3OpenCLArray<b3SortData>& src, i32 nSrc, b3OpenCLArray<u32>& dst, i32 nDst, Option option = BOUND_LOWER);

	void executeHost(b3AlignedObjectArray<b3SortData>& src, i32 nSrc, b3AlignedObjectArray<u32>& dst, i32 nDst, Option option = BOUND_LOWER);
};

#endif  //D3_BOUNDSEARCH_H
