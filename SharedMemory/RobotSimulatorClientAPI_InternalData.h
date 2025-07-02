#ifndef D3_ROBOT_SIMULATOR_CLIENT_API_INTERNAL_DATA_H
#define D3_ROBOT_SIMULATOR_CLIENT_API_INTERNAL_DATA_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

struct RobotSimulatorClientAPI_InternalData
{
	b3PhysicsClientHandle m_physicsClientHandle;
	struct GUIHelperInterface* m_guiHelper;

	RobotSimulatorClientAPI_InternalData()
		: m_physicsClientHandle(0),
		  m_guiHelper(0)
	{
	}
};

#endif  //D3_ROBOT_SIMULATOR_CLIENT_API_INTERNAL_DATA_H