#ifndef MUJOCO_PHYSICS_C_API_H
#define MUJOCO_PHYSICS_C_API_H

#ifdef DRX3D_ENABLE_MUJOCO

#include "../PhysicsClientC_API.h"

#ifdef __cplusplus
extern "C"
{
#endif

	//think more about naming. The b3ConnectPhysicsLoopback
	DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsMuJoCo();

#ifdef __cplusplus
}
#endif

#endif  //DRX3D_ENABLE_MUJOCO
#endif  //MUJOCO_PHYSICS_C_API_H
