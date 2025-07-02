#ifndef PHYSICS_CLIENT_GRPC_H
#define PHYSICS_CLIENT_GRPC_H

#include <drx3D/SharedMemory/PhysicsDirect.h>
#include <drx3D/SharedMemory/PhysicsCommandProcessorInterface.h>

class GRPCNetworkedPhysicsProcessor : public PhysicsCommandProcessorInterface
{
	struct GRPCNetworkedInternalData* m_data;

public:
	GRPCNetworkedPhysicsProcessor(tukk hostName, i32 port);

	virtual ~GRPCNetworkedPhysicsProcessor();

	virtual bool connect();

	virtual void disconnect();

	virtual bool isConnected() const;

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual void renderScene(i32 renderFlags);

	virtual void physicsDebugDraw(i32 debugDrawFlags);

	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper);

	virtual void setTimeOut(double timeOutInSeconds);

	virtual void reportNotifications() {}
};

#endif  //PHYSICS_CLIENT_GRPC_H
