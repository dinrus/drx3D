#ifndef D3_ROBOT_SIMULATOR_CLIENT_API_H
#define D3_ROBOT_SIMULATOR_CLIENT_API_H

#include <drx3D/SharedMemory/RobotSimulatorClientAPI_NoDirect.h>

///The RobotSimulatorClientAPI is pretty much the C++ version of pybullet
///as documented in the pybullet Quickstart Guide
///https://docs.google.com/document/d/10sXEhzFRSnvFcl3XxNGhnD4N2SedqwdAvK3dsihxVUA
class RobotSimulatorClientAPI_NoGUI : public RobotSimulatorClientAPI_NoDirect
{
public:
	RobotSimulatorClientAPI_NoGUI();
	virtual ~RobotSimulatorClientAPI_NoGUI();

	bool connect(i32 mode, const STxt& hostName = "localhost", i32 portOrKey = -1);
};

#endif  //D3_ROBOT_SIMULATOR_CLIENT_API_H
