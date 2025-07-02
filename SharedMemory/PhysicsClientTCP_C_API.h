#ifndef PHYSICS_CLIENT_TCP_C_API_H
#define PHYSICS_CLIENT_TCP_C_API_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	///send physics commands using TCP networking
	DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsTCP(tukk hostName, i32 port);

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_CLIENT_TCP_C_API_H
