#ifdef DRX3D_ENABLE_PHYSX
#include "PhysXC_API.h"
#include "../PhysicsDirect.h"
#include "PhysXServerCommandProcessor.h"


DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysX(i32 argc, tuk argv[])
{
	PhysXServerCommandProcessor* sdk = new PhysXServerCommandProcessor(argc,argv);

	PhysicsDirect* direct = new PhysicsDirect(sdk, true);
	bool connected;
	connected = direct->connect();
	return (b3PhysicsClientHandle)direct;
}
#endif  //DRX3D_ENABLE_PHYSX