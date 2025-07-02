#ifdef DRX3D_ENABLE_DART
#include "DARTPhysicsC_API.h"
#include "DARTPhysicsServerCommandProcessor.h"
#include "DARTPhysicsClient.h"

//think more about naming. The b3ConnectPhysicsLoopback
DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsDART()
{
	DARTPhysicsServerCommandProcessor* sdk = new DARTPhysicsServerCommandProcessor;

	DARTPhysicsClient* direct = new DARTPhysicsClient(sdk, true);
	bool connected;
	connected = direct->connect();
	return (b3PhysicsClientHandle)direct;
}
#endif  //DRX3D_ENABLE_DART