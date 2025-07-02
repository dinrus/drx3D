#ifndef SHARED_MEMORY_COMMAND_PROCESSOR_H
#define SHARED_MEMORY_COMMAND_PROCESSOR_H

#include <drx3D/SharedMemory/PhysicsCommandProcessorInterface.h>

class SharedMemoryCommandProcessor : public PhysicsCommandProcessorInterface
{
	struct SharedMemoryCommandProcessorInternalData* m_data;

public:
	SharedMemoryCommandProcessor();

	virtual ~SharedMemoryCommandProcessor();

	virtual bool connect();

	virtual void disconnect();

	virtual bool isConnected() const;

	virtual bool processCommand(const struct SharedMemoryCommand& clientCmd, struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual bool receiveStatus(struct SharedMemoryStatus& serverStatusOut, tuk bufferServerToClient, i32 bufferSizeInBytes);

	virtual void renderScene(i32 renderFlags);
	virtual void physicsDebugDraw(i32 debugDrawFlags);
	virtual void setGuiHelper(struct GUIHelperInterface* guiHelper);

	void setSharedMemoryInterface(class SharedMemoryInterface* sharedMem);
	void setSharedMemoryKey(i32 key);
	virtual void setTimeOut(double timeOutInSeconds);

	virtual void reportNotifications() {}
};

#endif  //SHARED_MEMORY_COMMAND_PROCESSOR_H
