
#include <drx3D/SharedMemory/PhysicsClientUDP_C_API.h>
#include <drx3D/SharedMemory/PhysicsClientUDP.h>
#include <drx3D/SharedMemory/PhysicsDirect.h>
#include <stdio.h>

//think more about naming. The b3ConnectPhysicsLoopback
DRX3D_SHARED_API b3PhysicsClientHandle b3ConnectPhysicsUDP(tukk hostName, i32 port)
{
	UdpNetworkedPhysicsProcessor* udp = new UdpNetworkedPhysicsProcessor(hostName, port);

	PhysicsDirect* direct = new PhysicsDirect(udp, true);

	bool connected;
	connected = direct->connect();
	if (connected)
	{
		printf("b3ConnectPhysicsUDP connected successfully.\n");
	}
	else
	{
		printf("b3ConnectPhysicsUDP connection failed.\n");
	}
	return (b3PhysicsClientHandle)direct;
}
