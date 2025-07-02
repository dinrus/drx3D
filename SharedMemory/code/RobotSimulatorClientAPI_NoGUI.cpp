#include <drx3D/SharedMemory/RobotSimulatorClientAPI_NoGUI.h>

#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/RobotSimulatorClientAPI_InternalData.h>

//#ifdef DRX3D_ENABLE_ENET
#include <drx3D/SharedMemory/PhysicsClientUDP_C_API.h>
//#endif  //PHYSICS_UDP

//#ifdef DRX3D_ENABLE_CLSOCKET
#include <drx3D/SharedMemory/PhysicsClientTCP_C_API.h>
//#endif  //PHYSICS_TCP

#ifndef DRX3D_DISABLE_PHYSICS_DIRECT
#include <drx3D/SharedMemory/PhysicsDirectC_API.h>
#endif  //DRX3D_DISABLE_PHYSICS_DIRECT

#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Common/b3Logging.h>

RobotSimulatorClientAPI_NoGUI::RobotSimulatorClientAPI_NoGUI()
{
}

RobotSimulatorClientAPI_NoGUI::~RobotSimulatorClientAPI_NoGUI()
{
}

bool RobotSimulatorClientAPI_NoGUI::connect(i32 mode, const STxt& hostName, i32 portOrKey)
{
	if (m_data->m_physicsClientHandle)
	{
		drx3DWarning("Already connected, disconnect first.");
		return false;
	}
	b3PhysicsClientHandle sm = 0;
	i32 udpPort = 1234;
	i32 tcpPort = 6667;
	i32 key = SHARED_MEMORY_KEY;

	switch (mode)
	{
		case eCONNECT_DIRECT:
		{
#ifndef DRX3D_DISABLE_PHYSICS_DIRECT
			sm = b3ConnectPhysicsDirect();
#endif  //DRX3D_DISABLE_PHYSICS_DIRECT

			break;
		}
		case eCONNECT_SHARED_MEMORY:
		{
			if (portOrKey >= 0)
			{
				key = portOrKey;
			}
			sm = b3ConnectSharedMemory(key);
			break;
		}
		case eCONNECT_UDP:
		{
			if (portOrKey >= 0)
			{
				udpPort = portOrKey;
			}
//#ifdef DRX3D_ENABLE_ENET

			sm = b3ConnectPhysicsUDP(hostName.c_str(), udpPort);
//#else
	//		drx3DWarning("UDP is not enabled in this build");
//#endif  //DRX3D_ENABLE_ENET

			break;
		}
		case eCONNECT_TCP:
		{
			if (portOrKey >= 0)
			{
				tcpPort = portOrKey;
			}
//#ifdef DRX3D_ENABLE_CLSOCKET

			sm = b3ConnectPhysicsTCP(hostName.c_str(), tcpPort);
//#else
	//		drx3DWarning("TCP is not enabled in this pybullet build");
//#endif  //DRX3D_ENABLE_CLSOCKET
			break;
		}

		default:
		{
			drx3DWarning("connectPhysicsServer unexpected argument");
		}
	};

	if (sm)
	{
		m_data->m_physicsClientHandle = sm;
		if (!b3CanSubmitCommand(m_data->m_physicsClientHandle))
		{
			disconnect();
			return false;
		}
		return true;
	}
	return false;
}
