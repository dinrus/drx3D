#include "../RobotSimulatorClientAPI.h"
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/RobotSimulatorClientAPI_InternalData.h>
//#ifdef DRX3D_ENABLE_ENET
#include <drx3D/SharedMemory/PhysicsClientUDP_C_API.h>
//#endif  //PHYSICS_UDP
//#ifdef DRX3D_ENABLE_CLSOCKET
#include <drx3D/SharedMemory/PhysicsClientTCP_C_API.h>
//#endif  //PHYSICS_TCP
#include <drx3D/SharedMemory/PhysicsDirectC_API.h>
#include <drx3D/SharedMemory/SharedMemoryInProcessPhysicsC_API.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Common/b3Logging.h>
#ifdef DRX3D_ENABLE_GRPC
#include <drx3D/SharedMemory/PhysicsClientGRPC_C_API.h>
#endif

RobotSimulatorClientAPI::RobotSimulatorClientAPI()
{
}

RobotSimulatorClientAPI::~RobotSimulatorClientAPI()
{
}

void RobotSimulatorClientAPI::renderScene()
{
	if (!isConnected())
	{
		drx3DWarning("Не подключено");
		return;
	}
	if (m_data->m_guiHelper)
	{
		b3InProcessRenderSceneInternal(m_data->m_physicsClientHandle);
	}
}

void RobotSimulatorClientAPI::debugDraw(i32 debugDrawMode)
{
	if (!isConnected())
	{
		drx3DWarning("Не подключено");
		return;
	}
	if (m_data->m_guiHelper)
	{
		b3InProcessDebugDrawInternal(m_data->m_physicsClientHandle, debugDrawMode);
	}
}

bool RobotSimulatorClientAPI::mouseMoveCallback(float x, float y)
{
	if (!isConnected())
	{
		drx3DWarning("Не подключено");
		return false;
	}
	if (m_data->m_guiHelper)
	{
		return b3InProcessMouseMoveCallback(m_data->m_physicsClientHandle, x, y) != 0;
	}
	return false;
}
bool RobotSimulatorClientAPI::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (!isConnected())
	{
		drx3DWarning("Не подключено");
		return false;
	}
	if (m_data->m_guiHelper)
	{
		return b3InProcessMouseButtonCallback(m_data->m_physicsClientHandle, button, state, x, y) != 0;
	}
	return false;
}

bool RobotSimulatorClientAPI::connect(i32 mode, const STxt& hostName, i32 portOrKey)
{
	if (m_data->m_physicsClientHandle)
	{
		drx3DWarning("Уже подключено, вначале нужно отключить.");
		return false;
	}
	b3PhysicsClientHandle sm = 0;

	i32 udpPort = 1234;
	i32 tcpPort = 6667;
	i32 key = SHARED_MEMORY_KEY;

	switch (mode)
	{
		case eCONNECT_EXISTING_EXAMPLE_BROWSER:
		{
			sm = b3CreateInProcessPhysicsServerFromExistingExampleBrowserAndConnect(m_data->m_guiHelper);
			break;
		}

		case eCONNECT_GUI:
		{
			i32 argc = 0;
			tuk argv[1] = {0};
#ifdef __APPLE__
			sm = b3CreateInProcessPhysicsServerAndConnectMainThread(argc, argv);
#else
			sm = b3CreateInProcessPhysicsServerAndConnect(argc, argv);
#endif
			break;
		}
		case eCONNECT_GUI_SERVER:
		{
			i32 argc = 0;
			tuk argv[1] = {0};
#ifdef __APPLE__
			sm = b3CreateInProcessPhysicsServerAndConnectMainThread(argc, argv);
#else
			sm = b3CreateInProcessPhysicsServerAndConnect(argc, argv);
#endif
			break;
		}
		case eCONNECT_DIRECT:
		{
			sm = b3ConnectPhysicsDirect();
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
//			drx3DWarning("UDP is not enabled in this build");
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
//			drx3DWarning("TCP is not enabled in this pybullet build");
//#endif  //DRX3D_ENABLE_CLSOCKET
			break;
		}
		case eCONNECT_GRPC:
		{
#ifdef DRX3D_ENABLE_GRPC
			sm = b3ConnectPhysicsGRPC(hostName.c_str(), tcpPort);
#else
			drx3DWarning("GRPC is not enabled in this pybullet build");
#endif
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
