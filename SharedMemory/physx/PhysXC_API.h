#ifndef PHYSX_C_API_H
#define PHYSX_C_API_H

#ifdef DRX3D_ENABLE_PHYSX

#include "../PhysicsClientC_API.h"

#ifdef __cplusplus
extern "C"
{
#endif

	DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysX(i32 argc, tuk argv[]);

#ifdef __cplusplus
}
#endif

#endif  //DRX3D_ENABLE_PHYSX
#endif  //PHYSX_C_API_H
