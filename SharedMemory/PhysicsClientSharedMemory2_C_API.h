#ifndef PHYSICS_CLIENT_SHARED_MEMORY2_H
#define PHYSICS_CLIENT_SHARED_MEMORY2_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	b3PhysicsClientHandle b3ConnectSharedMemory2(i32 key);

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_CLIENT_SHARED_MEMORY2_H
