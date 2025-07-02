#ifndef PHYSX_SERVER_COMMAND_PROCESSOR_H
#define PHYSX_SERVER_COMMAND_PROCESSOR_H

#include "../PhysicsCommandProcessorInterface.h"

class PhysXServerCommandProcessor : public PhysicsCommandProcessorInterface
{
	struct PhysXServerCommandProcessorInternalData* m_data;

	bool processSyncBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestInternalDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSyncUserDataCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processLoadURDFCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestBodyInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processForwardDynamicsCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSendPhysicsParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestActualStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processResetSimulationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processInitPoseCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSendDesiredStateCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processChangeDynamicsInfoCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestPhysicsSimulationParametersCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestContactpointInformationCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processCreateCollisionShapeCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processSetAdditionalSearchPathCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processUserDebugDrawCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);
	bool processRequestCameraImageCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	bool processCustomCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	void resetSimulation();
	bool processStateLoggingCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

public:
	PhysXServerCommandProcessor(i32 argc, tuk argv[]);

	virtual ~PhysXServerCommandProcessor();

	virtual bool connect();

	virtual void disconnect();

	virtual bool isConnected() const;

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual void renderScene(i32 renderFlags) {}
	virtual void physicsDebugDraw(i32 debugDrawFlags) {}
	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper) {}
	virtual void setTimeOut(double timeOutInSeconds) {}

	virtual void reportNotifications() {}
};

#endif  //PHYSX_SERVER_COMMAND_PROCESSOR_H
