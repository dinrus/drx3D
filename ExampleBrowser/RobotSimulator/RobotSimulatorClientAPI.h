#ifndef D3_ROBOT_SIMULATOR_CLIENT_API_GUI_H
#define D3_ROBOT_SIMULATOR_CLIENT_API_GUI_H

#include <drx3D/SharedMemory/RobotSimulatorClientAPI_NoGUI.h>

///The RobotSimulatorClientAPI_GUI is pretty much the C++ version of pybullet
///as documented in the pybullet Quickstart Guide
///https://docs.google.com/document/d/10sXEhzFRSnvFcl3XxNGhnD4N2SedqwdAvK3dsihxVUA
class RobotSimulatorClientAPI : public RobotSimulatorClientAPI_NoGUI
{
public:
	RobotSimulatorClientAPI();

	virtual ~RobotSimulatorClientAPI();

	virtual bool connect(i32 mode, const STxt& hostName = "localhost", i32 portOrKey = -1);

	virtual void renderScene();

	virtual void debugDraw(i32 debugDrawMode);

	virtual bool mouseMoveCallback(float x, float y);

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y);
};

#endif  //D3_ROBOT_SIMULATOR_CLIENT_API_H
