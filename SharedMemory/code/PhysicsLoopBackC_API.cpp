#include <drx3D/SharedMemory/PhysicsLoopBackC_API.h>

#include <drx3D/SharedMemory/PhysicsLoopBack.h>

//think more about naming. The b3ConnectPhysicsLoopback
b3PhysicsClientHandle b3ConnectPhysicsLoopback(i32 key)
{
	PhysicsLoopBack* loopBack = new PhysicsLoopBack();
	loopBack->setSharedMemoryKey(key);
	bool connected;
	connected = loopBack->connect();
	return (b3PhysicsClientHandle)loopBack;
}
