
#ifndef _DRX3D_ACTION_INTERFACE_H
#define _DRX3D_ACTION_INTERFACE_H

class IDebugDraw;
class CollisionWorld;

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

///Basic interface to allow actions such as vehicles and characters to be updated inside a DynamicsWorld
class ActionInterface
{
protected:
	static RigidBody& getFixedBody();

public:
	virtual ~ActionInterface()
	{
	}

	virtual void updateAction(CollisionWorld* collisionWorld, Scalar deltaTimeStep) = 0;

	virtual void debugDraw(IDebugDraw* debugDrawer) = 0;
};

#endif  //_DRX3D_ACTION_INTERFACE_H
