#ifndef PHYSICS_COMMAND_PROCESSOR_INTERFACE_H
#define PHYSICS_COMMAND_PROCESSOR_INTERFACE_H

#include <drxtypes.h>

enum PhysicsCommandRenderFlags
{
	COV_DISABLE_SYNC_RENDERING = 1
};

class PhysicsCommandProcessorInterface
{
public:
	virtual ~PhysicsCommandProcessorInterface() {}

	virtual bool connect() = 0;

	virtual void disconnect() = 0;

	virtual bool isConnected() const = 0;

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes) = 0;

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, char* bufferServerToClient, i32 bufferSizeInBytes) = 0;

	virtual void renderScene(i32 renderFlags) = 0;
	virtual void physicsDebugDraw(i32 debugDrawFlags) = 0;
	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper) = 0;
	virtual void setTimeOut(double timeOutInSeconds) = 0;

	virtual void reportNotifications() = 0;
};

class Vec3;
class Quat;

class CommandProcessorInterface : public PhysicsCommandProcessorInterface
{
public:
	virtual ~CommandProcessorInterface() {}

	virtual void syncPhysicsToGraphics() = 0;
	virtual void stepSimulationRealTime(double dtInSec, const struct b3VRControllerEvent* vrControllerEvents, i32 numVRControllerEvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents) = 0;
	virtual void enableRealTimeSimulation(bool enableRealTimeSim) = 0;
	virtual bool isRealTimeSimulationEnabled() const = 0;

	virtual void enableCommandLogging(bool enable, tukk fileName) = 0;
	virtual void replayFromLogFile(tukk fileName) = 0;
	virtual void replayLogCommand(char* bufferServerToClient, i32 bufferSizeInBytes) = 0;

	virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) = 0;
	virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) = 0;
	virtual void removePickingConstraint() = 0;

	virtual const Vec3& getVRTeleportPosition() const = 0;
	virtual void setVRTeleportPosition(const Vec3& vrReleportPos) = 0;

	virtual const Quat& getVRTeleportOrientation() const = 0;
	virtual void setVRTeleportOrientation(const Quat& vrReleportOrn) = 0;

	virtual void processClientCommands() = 0;
};

#endif  //PHYSICS_COMMAND_PROCESSOR_INTERFACE_H
