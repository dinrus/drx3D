#ifndef PHYSICS_LOOPBACK_C_API_H
#define PHYSICS_LOOPBACK_C_API_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	///think more about naming. The b3ConnectPhysicsLoopback
	b3PhysicsClientHandle b3ConnectPhysicsLoopback(i32 key);

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_LOOPBACK_C_API_H
