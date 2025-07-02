#include <drx3D/SharedMemory/PhysicsClientSharedMemory2_C_API.h>

#include  <drx3D/SharedMemory/PhysicsDirect.h>
#include  <drx3D/SharedMemory/SharedMemoryCommandProcessor.h>

b3PhysicsClientHandle b3ConnectSharedMemory2(i32 key)
{
	SharedMemoryCommandProcessor* cmdProc = new SharedMemoryCommandProcessor();
	cmdProc->setSharedMemoryKey(key);
	PhysicsDirect* cl = new PhysicsDirect(cmdProc, true);

	cl->setSharedMemoryKey(key);

	cl->connect();

	return (b3PhysicsClientHandle)cl;
}
