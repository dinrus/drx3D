#include <drx3D/SharedMemory/PhysicsClientSharedMemory_C_API.h>
#include <drx3D/SharedMemory/PhysicsClientSharedMemory.h>

DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectSharedMemory(i32 key)
{
	PhysicsClientSharedMemory* cl = new PhysicsClientSharedMemory();
	cl->setSharedMemoryKey(key);
	cl->connect();
	return (b3PhysicsClientHandle)cl;
}
