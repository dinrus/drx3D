#include <drx3D/SharedMemory/PhysicsDirectC_API.h>
#include <drx3D/SharedMemory/PhysicsDirect.h>
#include <drx3D/SharedMemory/PhysicsServerCommandProcessor.h>

//think more about naming. The b3ConnectPhysicsLoopback
DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsDirect()
{
	PhysicsServerCommandProcessor* sdk = new PhysicsServerCommandProcessor;

	PhysicsDirect* direct = new PhysicsDirect(sdk, true);
	bool connected;
	connected = direct->connect();
	return (b3PhysicsClientHandle)direct;
}

//
