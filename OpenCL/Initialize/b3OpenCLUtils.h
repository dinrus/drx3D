#ifndef D3_OPENCL_UTILS_H
#define D3_OPENCL_UTILS_H

#include "b3OpenCLInclude.h"

#ifdef __cplusplus
extern "C"
{
#endif

	///C API for OpenCL utilities: convenience functions, see below for C++ API

	/// CL Context optionally takes a GL context. This is a generic type because we don't really want this code
	/// to have to understand GL types. It is a HGLRC in _WIN32 or a GLXContext otherwise.
	cl_context b3OpenCLUtils_createContextFromType(cl_device_type deviceType, cl_int* pErrNum, uk pGLCtx, uk pGLDC, i32 preferredDeviceIndex, i32 preferredPlatformIndex, cl_platform_id* platformId);

	i32 b3OpenCLUtils_getNumDevices(cl_context cxMainContext);

	cl_device_id b3OpenCLUtils_getDevice(cl_context cxMainContext, i32 nr);

	void b3OpenCLUtils_printDeviceInfo(cl_device_id device);

	cl_kernel b3OpenCLUtils_compileCLKernelFromString(cl_context clContext, cl_device_id device, tukk kernelSource, tukk kernelName, cl_int* pErrNum, cl_program prog, tukk additionalMacros);

	//optional
	cl_program b3OpenCLUtils_compileCLProgramFromString(cl_context clContext, cl_device_id device, tukk kernelSource, cl_int* pErrNum, tukk additionalMacros, tukk srcFileNameForCaching, bool disableBinaryCaching);

	//the following optional APIs provide access using specific platform information
	i32 b3OpenCLUtils_getNumPlatforms(cl_int* pErrNum);

	///get the nr'th platform, where nr is in the range [0..getNumPlatforms)
	cl_platform_id b3OpenCLUtils_getPlatform(i32 nr, cl_int* pErrNum);

	void b3OpenCLUtils_printPlatformInfo(cl_platform_id platform);

	tukk b3OpenCLUtils_getSdkVendorName();

	///set the path (directory/folder) where the compiled OpenCL kernel are stored
	void b3OpenCLUtils_setCachePath(tukk path);

	cl_context b3OpenCLUtils_createContextFromPlatform(cl_platform_id platform, cl_device_type deviceType, cl_int* pErrNum, uk pGLCtx, uk pGLDC, i32 preferredDeviceIndex, i32 preferredPlatformIndex);

#ifdef __cplusplus
}

#define D3_MAX_STRING_LENGTH 1024

typedef struct
{
	char m_deviceName[D3_MAX_STRING_LENGTH];
	char m_deviceVendor[D3_MAX_STRING_LENGTH];
	char m_driverVersion[D3_MAX_STRING_LENGTH];
	char m_deviceExtensions[D3_MAX_STRING_LENGTH];

	cl_device_type m_deviceType;
	cl_uint m_computeUnits;
	size_t m_workitemDims;
	size_t m_workItemSize[3];
	size_t m_image2dMaxWidth;
	size_t m_image2dMaxHeight;
	size_t m_image3dMaxWidth;
	size_t m_image3dMaxHeight;
	size_t m_image3dMaxDepth;
	size_t m_workgroupSize;
	cl_uint m_clockFrequency;
	cl_ulong m_constantBufferSize;
	cl_ulong m_localMemSize;
	cl_ulong m_globalMemSize;
	cl_bool m_errorCorrectionSupport;
	cl_device_local_mem_type m_localMemType;
	cl_uint m_maxReadImageArgs;
	cl_uint m_maxWriteImageArgs;

	cl_uint m_addressBits;
	cl_ulong m_maxMemAllocSize;
	cl_command_queue_properties m_queueProperties;
	cl_bool m_imageSupport;
	cl_uint m_vecWidthChar;
	cl_uint m_vecWidthShort;
	cl_uint m_vecWidthInt;
	cl_uint m_vecWidthLong;
	cl_uint m_vecWidthFloat;
	cl_uint m_vecWidthDouble;

} b3OpenCLDeviceInfo;

struct b3OpenCLPlatformInfo
{
	char m_platformVendor[D3_MAX_STRING_LENGTH];
	char m_platformName[D3_MAX_STRING_LENGTH];
	char m_platformVersion[D3_MAX_STRING_LENGTH];

	b3OpenCLPlatformInfo()
	{
		m_platformVendor[0] = 0;
		m_platformName[0] = 0;
		m_platformVersion[0] = 0;
	}
};

///C++ API for OpenCL utilities: convenience functions
struct b3OpenCLUtils
{
	/// CL Context optionally takes a GL context. This is a generic type because we don't really want this code
	/// to have to understand GL types. It is a HGLRC in _WIN32 or a GLXContext otherwise.
	static inline cl_context createContextFromType(cl_device_type deviceType, cl_int* pErrNum, uk pGLCtx = 0, uk pGLDC = 0, i32 preferredDeviceIndex = -1, i32 preferredPlatformIndex = -1, cl_platform_id* platformId = 0)
	{
		return b3OpenCLUtils_createContextFromType(deviceType, pErrNum, pGLCtx, pGLDC, preferredDeviceIndex, preferredPlatformIndex, platformId);
	}

	static inline i32 getNumDevices(cl_context cxMainContext)
	{
		return b3OpenCLUtils_getNumDevices(cxMainContext);
	}
	static inline cl_device_id getDevice(cl_context cxMainContext, i32 nr)
	{
		return b3OpenCLUtils_getDevice(cxMainContext, nr);
	}

	static void getDeviceInfo(cl_device_id device, b3OpenCLDeviceInfo* info);

	static inline void printDeviceInfo(cl_device_id device)
	{
		b3OpenCLUtils_printDeviceInfo(device);
	}

	static inline cl_kernel compileCLKernelFromString(cl_context clContext, cl_device_id device, tukk kernelSource, tukk kernelName, cl_int* pErrNum = 0, cl_program prog = 0, tukk additionalMacros = "")
	{
		return b3OpenCLUtils_compileCLKernelFromString(clContext, device, kernelSource, kernelName, pErrNum, prog, additionalMacros);
	}

	//optional
	static inline cl_program compileCLProgramFromString(cl_context clContext, cl_device_id device, tukk kernelSource, cl_int* pErrNum = 0, tukk additionalMacros = "", tukk srcFileNameForCaching = 0, bool disableBinaryCaching = false)
	{
		return b3OpenCLUtils_compileCLProgramFromString(clContext, device, kernelSource, pErrNum, additionalMacros, srcFileNameForCaching, disableBinaryCaching);
	}

	//the following optional APIs provide access using specific platform information
	static inline i32 getNumPlatforms(cl_int* pErrNum = 0)
	{
		return b3OpenCLUtils_getNumPlatforms(pErrNum);
	}
	///get the nr'th platform, where nr is in the range [0..getNumPlatforms)
	static inline cl_platform_id getPlatform(i32 nr, cl_int* pErrNum = 0)
	{
		return b3OpenCLUtils_getPlatform(nr, pErrNum);
	}

	static void getPlatformInfo(cl_platform_id platform, b3OpenCLPlatformInfo* platformInfo);

	static inline void printPlatformInfo(cl_platform_id platform)
	{
		b3OpenCLUtils_printPlatformInfo(platform);
	}

	static inline tukk getSdkVendorName()
	{
		return b3OpenCLUtils_getSdkVendorName();
	}
	static inline cl_context createContextFromPlatform(cl_platform_id platform, cl_device_type deviceType, cl_int* pErrNum, uk pGLCtx = 0, uk pGLDC = 0, i32 preferredDeviceIndex = -1, i32 preferredPlatformIndex = -1)
	{
		return b3OpenCLUtils_createContextFromPlatform(platform, deviceType, pErrNum, pGLCtx, pGLDC, preferredDeviceIndex, preferredPlatformIndex);
	}
	static void setCachePath(tukk path)
	{
		b3OpenCLUtils_setCachePath(path);
	}
};

#endif  //__cplusplus

#endif  // D3_OPENCL_UTILS_H
