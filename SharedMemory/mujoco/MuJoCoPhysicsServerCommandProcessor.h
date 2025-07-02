#ifndef MUJOCO_PHYSICS_SERVER_COMMAND_PROCESSOR_H
#define MUJOCO_PHYSICS_SERVER_COMMAND_PROCESSOR_H

#include "../PhysicsCommandProcessorInterface.h"

class MuJoCoPhysicsServerCommandProcessor : public PhysicsCommandProcessorInterface
{
	struct MuJoCoPhysicsServerCommandProcessorInternalData* m_data;

	bool processSyncBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestInternalDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSyncUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadMJCFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processForwardDynamicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSendPhysicsParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestActualStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processResetSimulationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	void resetSimulation();

public:
	MuJoCoPhysicsServerCommandProcessor();

	virtual ~MuJoCoPhysicsServerCommandProcessor();

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

#endif  //MUJOCO_PHYSICS_COMMAND_PROCESSOR_H
