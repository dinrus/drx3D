
#include <drx3D/SharedMemory/PhysicsClientTCP_C_API.h>
#include <drx3D/SharedMemory/PhysicsClientTCP.h>
#include <drx3D/SharedMemory/PhysicsDirect.h>
#include <stdio.h>

DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsTCP(tukk hostName, i32 port)
{
	TcpNetworkedPhysicsProcessor* tcp = new TcpNetworkedPhysicsProcessor(hostName, port);

	PhysicsDirect* direct = new PhysicsDirect(tcp, true);

	bool connected;
	connected = direct->connect();
	if (connected)
	{
		printf("b3ConnectPhysicsTCP connected successfully.\n");
	}
	else
	{
		printf("b3ConnectPhysicsTCP connection failed.\n");
	}
	return (b3PhysicsClientHandle)direct;
}
