#ifdef DRX3D_ENABLE_GRPC

#include "PhysicsClientGRPC_C_API.h"
#include "PhysicsClientGRPC.h"
#include "PhysicsDirect.h"
#include <stdio.h>

DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsGRPC(tukk hostName, i32 port)
{
	GRPCNetworkedPhysicsProcessor* tcp = new GRPCNetworkedPhysicsProcessor(hostName, port);

	PhysicsDirect* direct = new PhysicsDirect(tcp, true);

	bool connected;
	connected = direct->connect();
	if (connected)
	{
		printf("b3ConnectPhysicsGRPC connected successfully.\n");
	}
	else
	{
		printf("b3ConnectPhysicsGRPC connection failed.\n");
	}
	return (b3PhysicsClientHandle)direct;
}

#endif  //DRX3D_ENABLE_GRPC