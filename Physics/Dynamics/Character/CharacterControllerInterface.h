#ifndef DRX3D_CHARACTER_CONTROLLER_INTERFACE_H
#define DRX3D_CHARACTER_CONTROLLER_INTERFACE_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ActionInterface.h>

class CollisionShape;
class RigidBody;
class CollisionWorld;

class CharacterControllerInterface : public ActionInterface
{
public:
	CharacterControllerInterface(){};
	virtual ~CharacterControllerInterface(){};

	virtual void setWalkDirection(const Vec3& walkDirection) = 0;
	virtual void setVelocityForTimeInterval(const Vec3& velocity, Scalar timeInterval) = 0;
	virtual void reset(CollisionWorld* collisionWorld) = 0;
	virtual void warp(const Vec3& origin) = 0;

	virtual void preStep(CollisionWorld* collisionWorld) = 0;
	virtual void playerStep(CollisionWorld* collisionWorld, Scalar dt) = 0;
	virtual bool canJump() const = 0;
	virtual void jump(const Vec3& dir = Vec3(0, 0, 0)) = 0;

	virtual bool onGround() const = 0;
	virtual void setUpInterpolate(bool value) = 0;
};

#endif  //DRX3D_CHARACTER_CONTROLLER_INTERFACE_H
