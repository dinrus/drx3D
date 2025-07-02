#ifdef DRX3D_ENABLE_MUJOCO
#include "MuJoCoPhysicsC_API.h"
#include "MuJoCoPhysicsServerCommandProcessor.h"
#include "MuJoCoPhysicsClient.h"

DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsMuJoCo()
{
	MuJoCoPhysicsServerCommandProcessor* sdk = new MuJoCoPhysicsServerCommandProcessor;

	MuJoCoPhysicsClient* direct = new MuJoCoPhysicsClient(sdk, true);
	bool connected;
	connected = direct->connect();
	return (b3PhysicsClientHandle)direct;
}
#endif  //DRX3D_ENABLE_MUJOCO