
#ifndef DRX3D_CONVERT_GRPC_DRX3D_H
#define DRX3D_CONVERT_GRPC_DRX3D_H

#include "../PhysicsClientC_API.h"

namespace pybullet_grpc
{
class PyBulletCommand;
class PyBulletStatus;
};  // namespace pybullet_grpc

struct SharedMemoryCommand* convertGRPCToBulletCommand(const pybullet_grpc::PyBulletCommand& grpcCommand, struct SharedMemoryCommand& cmd);

pybullet_grpc::PyBulletCommand* convertBulletToGRPCCommand(const struct SharedMemoryCommand& clientCmd, pybullet_grpc::PyBulletCommand& grpcCommand);

bool convertGRPCToStatus(const pybullet_grpc::PyBulletStatus& grpcReply, struct SharedMemoryStatus& serverStatus, tuk bufferServerToClient, i32 bufferSizeInBytes);

bool convertStatusToGRPC(const struct SharedMemoryStatus& serverStatus, tuk bufferServerToClient, i32 bufferSizeInBytes, pybullet_grpc::PyBulletStatus& grpcReply);

#endif  //DRX3D_CONVERT_GRPC_DRX3D_H