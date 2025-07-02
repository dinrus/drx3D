#ifndef PHYSICS_CLIENT_SHARED_MEMORY_H
#define PHYSICS_CLIENT_SHARED_MEMORY_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectSharedMemory(i32 key);

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_CLIENT_SHARED_MEMORY_H
