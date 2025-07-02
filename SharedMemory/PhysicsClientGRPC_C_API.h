#ifndef PHYSICS_CLIENT_GRPC_C_API_H
#define PHYSICS_CLIENT_GRPC_C_API_H

#include <drx3D/SharedMemory/PhysicsClientC_API.h>

#ifdef __cplusplus
extern "C"
{
#endif

	///send physics commands using GRPC connection
	DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsGRPC(tukk hostName, i32 port);

#ifdef __cplusplus
}
#endif

#endif  //PHYSICS_CLIENT_GRPC_C_API_H
