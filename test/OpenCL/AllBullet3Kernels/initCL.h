
#ifndef INIT_CL_H
#define INIT_CL_H

void initCL()
{
	i32 preferredDeviceIndex = -1;
	i32 preferredPlatformIndex = -1;
	bool allowCpuOpenCL = false;

	b3CommandLineArgs args(gArgc, gArgv);
	args.GetCmdLineArgument("cl_device", preferredDeviceIndex);
	args.GetCmdLineArgument("cl_platform", preferredPlatformIndex);
	allowCpuOpenCL = args.CheckCmdLineFlag("allow_opencl_cpu");

	uk glCtx = 0;
	uk glDC = 0;

	i32 ciErrNum = 0;

	cl_device_type deviceType = CL_DEVICE_TYPE_GPU;
	if (allowCpuOpenCL)
		deviceType = CL_DEVICE_TYPE_ALL;

	//	if (useInterop)
	//	{
	//		m_data->m_clContext = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, glCtx, glDC);
	//	} else
	{
		m_clContext = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, 0, 0, preferredDeviceIndex, preferredPlatformIndex, &m_platformId);
		ASSERT_FALSE(m_clContext == 0);
	}

	b3OpenCLPlatformInfo platformInfo;
	b3OpenCLUtils::getPlatformInfo(m_platformId, &platformInfo);
	b3Printf("OpenCL Platform Name %s\n", platformInfo.m_platformName);
	b3Printf("OpenCL Platform Vendor %s\n", platformInfo.m_platformVendor);
	b3Printf("OpenCL Platform Version %s\n", platformInfo.m_platformVersion);

	ASSERT_EQ(ciErrNum, CL_SUCCESS);

	i32 numDev = b3OpenCLUtils::getNumDevices(m_clContext);
	EXPECT_GT(numDev, 0);

	if (numDev > 0)
	{
		m_clDevice = b3OpenCLUtils::getDevice(m_clContext, 0);
		ASSERT_FALSE(m_clDevice == 0);

		m_clQueue = clCreateCommandQueue(m_clContext, m_clDevice, 0, &ciErrNum);
		ASSERT_FALSE(m_clQueue == 0);

		ASSERT_EQ(ciErrNum, CL_SUCCESS);

		b3OpenCLDeviceInfo info;
		b3OpenCLUtils::getDeviceInfo(m_clDevice, &info);
		b3OpenCLUtils::printDeviceInfo(m_clDevice);
		m_clDeviceName = info.m_deviceName;
	}
}

void exitCL()
{
	clReleaseCommandQueue(m_clQueue);
	clReleaseContext(m_clContext);
}

#endif  //INIT_CL_H
