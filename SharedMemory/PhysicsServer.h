#ifndef PHYSICS_SERVER_H
#define PHYSICS_SERVER_H

#include <drx3D/Maths/Linear/Vec3.h>

class PhysicsServer
{
public:
	virtual ~PhysicsServer();

	virtual void setSharedMemoryKey(i32 key) = 0;

	virtual bool connectSharedMemory(struct GUIHelperInterface* guiHelper) = 0;

	virtual void disconnectSharedMemory(bool deInitializeSharedMemory) = 0;

	virtual void processClientCommands() = 0;

	//	virtual bool	supportsJointMotor(class MultiBody* body, i32 linkIndex)=0;

	//@todo(erwincoumans) Should we have shared memory commands for picking objects?
	///The pickBody method will try to pick the first body along a ray, return true if succeeds, false otherwise
	virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) { return false; }
	virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) { return false; }
	virtual void removePickingConstraint() {}

	//for physicsDebugDraw and renderScene are mainly for debugging purposes
	//and for physics visualization. The idea is that physicsDebugDraw can also send wireframe
	//to a physics client, over shared memory
	virtual void physicsDebugDraw(i32 debugDrawFlags) {}
	virtual void renderScene(i32 renderFlags) {}

	virtual void enableCommandLogging(bool enable, tukk fileName) {}
	virtual void replayFromLogFile(tukk fileName) {}
};

#endif  //PHYSICS_SERVER_H
