#ifndef PHYSICS_SERVER_SHARED_MEMORY_H
#define PHYSICS_SERVER_SHARED_MEMORY_H

#include <drx3D/SharedMemory/PhysicsServer.h>
#include <drx3D/Maths/Linear/Quat.h>

class PhysicsServerSharedMemory : public PhysicsServer
{
	struct PhysicsServerSharedMemoryInternalData* m_data;

protected:
	void releaseSharedMemory();

public:
	PhysicsServerSharedMemory(struct CommandProcessorCreationInterface* commandProcessorCreator, class SharedMemoryInterface* sharedMem, i32 bla);
	virtual ~PhysicsServerSharedMemory();

	virtual void setSharedMemoryKey(i32 key);

	//todo: implement option to allocated shared memory from client
	virtual bool connectSharedMemory(struct GUIHelperInterface* guiHelper);

	virtual void disconnectSharedMemory(bool deInitializeSharedMemory);

	virtual void processClientCommands();

	virtual void stepSimulationRealTime(double dtInSec, const struct b3VRControllerEvent* vrEvents, i32 numVREvents, const struct b3KeyboardEvent* keyEvents, i32 numKeyEvents, const struct b3MouseEvent* mouseEvents, i32 numMouseEvents);

	virtual void enableRealTimeSimulation(bool enableRealTimeSim);
	virtual bool isRealTimeSimulationEnabled() const;

	virtual void reportNotifications();

	//bool	supportsJointMotor(class MultiBody* body, i32 linkIndex);

	///The pickBody method will try to pick the first body along a ray, return true if succeeds, false otherwise
	virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld);
	virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld);
	virtual void removePickingConstraint();

	virtual const Vec3& getVRTeleportPosition() const;
	virtual void setVRTeleportPosition(const Vec3& vrTeleportPos);

	virtual const Quat& getVRTeleportOrientation() const;
	virtual void setVRTeleportOrientation(const Quat& vrTeleportOrn);

	//for physicsDebugDraw and renderScene are mainly for debugging purposes
	//and for physics visualization. The idea is that physicsDebugDraw can also send wireframe
	//to a physics client, over shared memory
	void physicsDebugDraw(i32 debugDrawFlags);
	void renderScene(i32 renderFlags);
	void syncPhysicsToGraphics();

	void enableCommandLogging(bool enable, tukk fileName);
	void replayFromLogFile(tukk fileName);
};

#endif  //PHYSICS_SERVER_EXAMPLESHARED_MEMORY_H
