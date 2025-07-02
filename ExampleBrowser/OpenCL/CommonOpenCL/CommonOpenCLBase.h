#ifndef COMMON_MULTI_BODY_SETUP_H
#define COMMON_MULTI_BODY_SETUP_H

#include <drx3D/Common/Interfaces/CommonExampleInterface.h"
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h"
#include <drx3D/Common/Interfaces/CommonRenderInterface.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h"
#include <drx3D/Common/Interfaces/CommonWindowInterface.h"
#include <drx3D/Common/Interfaces/CommonCameraInterface.h"

#include "GpuDemoInternalData.h"
#include <drx3D/Common/b3Scalar.h"
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"

struct CommonOpenCLBase : public CommonExampleInterface
{
	struct GUIHelperInterface* m_guiHelper;
	struct GpuDemoInternalData* m_clData;

	CommonOpenCLBase(GUIHelperInterface* helper)
		: m_guiHelper(helper),
		  m_clData(0)
	{
		m_clData = new GpuDemoInternalData();
	}

	virtual ~CommonOpenCLBase()
	{
		delete m_clData;
		m_clData = 0;
	}

	virtual void stepSimulation(float deltaTime)
	{
	}

	virtual void initCL(i32 preferredDeviceIndex, i32 preferredPlatformIndex)
	{
		//	uk glCtx=0;
		//	uk glDC = 0;

		i32 ciErrNum = 0;

		cl_device_type deviceType = CL_DEVICE_TYPE_GPU;
		//if (gAllowCpuOpenCL)
		//	deviceType = CL_DEVICE_TYPE_ALL;

		//	if (useInterop)
		//	{
		//		m_data->m_clContext = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, glCtx, glDC);
		//	} else
		{
			m_clData->m_clContext = b3OpenCLUtils::createContextFromType(deviceType, &ciErrNum, 0, 0, preferredDeviceIndex, preferredPlatformIndex, &m_clData->m_platformId);
		}

		oclCHECKERROR(ciErrNum, CL_SUCCESS);

		i32 numDev = b3OpenCLUtils::getNumDevices(m_clData->m_clContext);

		if (numDev > 0)
		{
			m_clData->m_clDevice = b3OpenCLUtils::getDevice(m_clData->m_clContext, 0);
			m_clData->m_clQueue = clCreateCommandQueue(m_clData->m_clContext, m_clData->m_clDevice, 0, &ciErrNum);
			oclCHECKERROR(ciErrNum, CL_SUCCESS);

			b3OpenCLDeviceInfo info;
			b3OpenCLUtils::getDeviceInfo(m_clData->m_clDevice, &info);
			m_clData->m_clDeviceName = info.m_deviceName;
			m_clData->m_clInitialized = true;
		}
	}

	virtual void exitCL()
	{
		if (m_clData && m_clData->m_clInitialized)
		{
			clReleaseCommandQueue(m_clData->m_clQueue);
			clReleaseContext(m_clData->m_clContext);
			m_clData->m_clInitialized = false;
		}
	}

	virtual void renderScene()
	{
		if (m_guiHelper->getRenderInterface())
		{
			m_guiHelper->getRenderInterface()->renderScene();
		}
	}

	virtual void physicsDebugDraw(i32 debugDrawFlags)
	{
	}

	virtual bool keyboardCallback(i32 key, i32 state)
	{
		return false;  //don't handle this key
	}

	virtual bool mouseMoveCallback(float x, float y)
	{
		return false;
	}

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		CommonRenderInterface* renderer = m_guiHelper->getRenderInterface();

		if (!renderer)
		{
			drx3DAssert(0);
			return false;
		}

		CommonWindowInterface* window = m_guiHelper->getAppInterface()->m_window;

		if (state == 1)
		{
			if (button == 0 && (!window->isModifierKeyPressed(B3G_ALT) && !window->isModifierKeyPressed(B3G_CONTROL)))
			{
				/*Vec3 camPos;
				renderer->getActiveCamera()->getCameraPosition(camPos);

				Vec3 rayFrom = camPos;
				Vec3 rayTo = getRayTo(i32(x),i32(y));

				pickBody(rayFrom, rayTo);
				*/
			}
		}
		else
		{
			if (button == 0)
			{
				//				removePickingConstraint();
				//remove p2p
			}
		}

		//printf("button=%d, state=%d\n",button,state);
		return false;
	}
};

#endif  //COMMON_MULTI_BODY_SETUP_H
