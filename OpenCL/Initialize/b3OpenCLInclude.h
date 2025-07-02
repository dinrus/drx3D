#ifndef D3_OPENCL_INCLUDE_H
#define D3_OPENCL_INCLUDE_H

//#define D3_USE_CLEW

#ifdef D3_USE_CLEW
#include <drx3D/clew.h>
#else

#ifdef __APPLE__
#ifdef USE_MINICL
#include <MiniCL/cl.h>
#else
#include <OpenCL/cl.h>
#include <OpenCL/cl_ext.h>  //clLogMessagesToStderrAPPLE
#endif
#else
#ifdef USE_MINICL
#include <MiniCL/cl.h>
#else
#include <CL/cl.h>
#ifdef _WIN32
#include <CL/cl_gl.h>
#endif  //_WIN32
#endif
#endif  //__APPLE__
#endif  //D3_USE_CLEW

#include <assert.h>
#include <stdio.h>
#define oclCHECKERROR(a, b)              \
	if ((a) != (b))                      \
	{                                    \
		printf("Ошибка OpenCL : %d\n", (a)); \
		assert((a) == (b));              \
	}

#endif  //D3_OPENCL_INCLUDE_H
