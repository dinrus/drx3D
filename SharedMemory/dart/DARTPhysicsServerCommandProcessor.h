#ifndef DART_PHYSICS_SERVER_COMMAND_PROCESSOR_H
#define DART_PHYSICS_SERVER_COMMAND_PROCESSOR_H

#include "../PhysicsCommandProcessorInterface.h"

class DARTPhysicsServerCommandProcessor : public PhysicsCommandProcessorInterface
{
public:
	DARTPhysicsServerCommandProcessor();

	virtual ~DARTPhysicsServerCommandProcessor();

	virtual bool connect();

	virtual void disconnect();

	virtual bool isConnected() const;

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual void renderScene(i32 renderFlags) {}
	virtual void physicsDebugDraw(i32 debugDrawFlags) {}
	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper) {}
	virtual void setTimeOut(double timeOutInSeconds) {}
};

#endif  //DART_PHYSICS_COMMAND_PROCESSOR_H
