#ifndef DRX3D_KINEMATIC_CHARACTER_CONTROLLER_H
#define DRX3D_KINEMATIC_CHARACTER_CONTROLLER_H

#include <drx3D/Maths/Linear/Vec3.h>

#include <drx3D/Physics/Dynamics/Character/CharacterControllerInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/CollisionAlgorithm.h>

class CollisionShape;
class ConvexShape;
class RigidBody;
class CollisionWorld;
class Dispatcher;
class PairCachingGhostObject;

//KinematicCharacterController is an object that supports a sliding motion in a world.
///It uses a ghost object and convex sweep test to test for upcoming collisions. This is combined with discrete collision detection to recover from penetrations.
///Interaction between KinematicCharacterController and dynamic rigid bodies needs to be explicity implemented by the user.
ATTRIBUTE_ALIGNED16(class)
KinematicCharacterController : public CharacterControllerInterface
{
protected:
	Scalar m_halfHeight;

	PairCachingGhostObject* m_ghostObject;
	ConvexShape* m_convexShape;  //is also in m_ghostObject, but it needs to be convex, so we store it here to avoid upcast

	Scalar m_maxPenetrationDepth;
	Scalar m_verticalVelocity;
	Scalar m_verticalOffset;
	Scalar m_fallSpeed;
	Scalar m_jumpSpeed;
	Scalar m_SetjumpSpeed;
	Scalar m_maxJumpHeight;
	Scalar m_maxSlopeRadians;  // Slope angle that is set (used for returning the exact value)
	Scalar m_maxSlopeCosine;   // Cosine equivalent of m_maxSlopeRadians (calculated once when set, for optimization)
	Scalar m_gravity;

	Scalar m_turnAngle;

	Scalar m_stepHeight;

	Scalar m_addedMargin;  //@todo: remove this and fix the code

	///this is the desired walk direction, set by the user
	Vec3 m_walkDirection;
	Vec3 m_normalizedDirection;
	Vec3 m_AngVel;

	Vec3 m_jumpPosition;

	//some internal variables
	Vec3 m_currentPosition;
	Scalar m_currentStepOffset;
	Vec3 m_targetPosition;

	Quat m_currentOrientation;
	Quat m_targetOrientation;

	///keep track of the contact manifolds
	ManifoldArray m_manifoldArray;

	bool m_touchingContact;
	Vec3 m_touchingNormal;

	Scalar m_linearDamping;
	Scalar m_angularDamping;

	bool m_wasOnGround;
	bool m_wasJumping;
	bool m_useGhostObjectSweepTest;
	bool m_useWalkDirection;
	Scalar m_velocityTimeInterval;
	Vec3 m_up;
	Vec3 m_jumpAxis;

	static Vec3* getUpAxisDirections();
	bool m_interpolateUp;
	bool full_drop;
	bool bounce_fix;

	Vec3 computeReflectionDirection(const Vec3& direction, const Vec3& normal);
	Vec3 parallelComponent(const Vec3& direction, const Vec3& normal);
	Vec3 perpindicularComponent(const Vec3& direction, const Vec3& normal);

	bool recoverFromPenetration(CollisionWorld * collisionWorld);
	void stepUp(CollisionWorld * collisionWorld);
	void updateTargetPositionBasedOnCollision(const Vec3& hit_normal, Scalar tangentMag = Scalar(0.0), Scalar normalMag = Scalar(1.0));
	void stepForwardAndStrafe(CollisionWorld * collisionWorld, const Vec3& walkMove);
	void stepDown(CollisionWorld * collisionWorld, Scalar dt);

	virtual bool needsCollision(const CollisionObject2* body0, const CollisionObject2* body1);

	void setUpVector(const Vec3& up);

	Quat getRotation(Vec3 & v0, Vec3 & v1) const;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	KinematicCharacterController(PairCachingGhostObject * ghostObject, ConvexShape * convexShape, Scalar stepHeight, const Vec3& up = Vec3(1.0, 0.0, 0.0));
	~KinematicCharacterController();

	//ActionInterface interface
	virtual void updateAction(CollisionWorld * collisionWorld, Scalar deltaTime)
	{
		preStep(collisionWorld);
		playerStep(collisionWorld, deltaTime);
	}

	//ActionInterface interface
	void debugDraw(IDebugDraw * debugDrawer);

	void setUp(const Vec3& up);

	const Vec3& getUp() { return m_up; }

	/// This should probably be called setPositionIncrementPerSimulatorStep.
	/// This is neither a direction nor a velocity, but the amount to
	///	increment the position each simulation iteration, regardless
	///	of dt.
	/// This call will reset any velocity set by setVelocityForTimeInterval().
	virtual void setWalkDirection(const Vec3& walkDirection);

	/// Caller provides a velocity with which the character should move for
	///	the given time period.  After the time period, velocity is reset
	///	to zero.
	/// This call will reset any walk direction set by setWalkDirection().
	/// Negative time intervals will result in no motion.
	virtual void setVelocityForTimeInterval(const Vec3& velocity,
											Scalar timeInterval);

	virtual void setAngularVelocity(const Vec3& velocity);
	virtual const Vec3& getAngularVelocity() const;

	virtual void setLinearVelocity(const Vec3& velocity);
	virtual Vec3 getLinearVelocity() const;

	void setLinearDamping(Scalar d) { m_linearDamping = Clamped(d, (Scalar)Scalar(0.0), (Scalar)Scalar(1.0)); }
	Scalar getLinearDamping() const { return m_linearDamping; }
	void setAngularDamping(Scalar d) { m_angularDamping = Clamped(d, (Scalar)Scalar(0.0), (Scalar)Scalar(1.0)); }
	Scalar getAngularDamping() const { return m_angularDamping; }

	void reset(CollisionWorld * collisionWorld);
	void warp(const Vec3& origin);

	void preStep(CollisionWorld * collisionWorld);
	void playerStep(CollisionWorld * collisionWorld, Scalar dt);

	void setStepHeight(Scalar h);
	Scalar getStepHeight() const { return m_stepHeight; }
	void setFallSpeed(Scalar fallSpeed);
	Scalar getFallSpeed() const { return m_fallSpeed; }
	void setJumpSpeed(Scalar jumpSpeed);
	Scalar getJumpSpeed() const { return m_jumpSpeed; }
	void setMaxJumpHeight(Scalar maxJumpHeight);
	bool canJump() const;

	void jump(const Vec3& v = Vec3(0, 0, 0));

	void applyImpulse(const Vec3& v) { jump(v); }

	void setGravity(const Vec3& gravity);
	Vec3 getGravity() const;

	/// The max slope determines the maximum angle that the controller can walk up.
	/// The slope angle is measured in radians.
	void setMaxSlope(Scalar slopeRadians);
	Scalar getMaxSlope() const;

	void setMaxPenetrationDepth(Scalar d);
	Scalar getMaxPenetrationDepth() const;

	PairCachingGhostObject* getGhostObject();
	void setUseGhostSweepTest(bool useGhostObjectSweepTest)
	{
		m_useGhostObjectSweepTest = useGhostObjectSweepTest;
	}

	bool onGround() const;
	void setUpInterpolate(bool value);
};

#endif  // DRX3D_KINEMATIC_CHARACTER_CONTROLLER_H
