#ifndef DRX3D_RIGIDBODY_H
#define DRX3D_RIGIDBODY_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

class CollisionShape;
class MotionState;
class TypedConstraint;

extern Scalar gDeactivationTime;
extern bool gDisableDeactivation;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define RigidBodyData RigidBodyDoubleData
#define RigidBodyDataName "RigidBodyDoubleData"
#else
#define RigidBodyData RigidBodyFloatData
#define RigidBodyDataName "RigidBodyFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

enum RigidBodyFlags
{
	DRX3D_DISABLE_WORLD_GRAVITY = 1,
	///DRX3D_ENABLE_GYROPSCOPIC_FORCE flags is enabled by default in drx3D 2.83 and onwards.
	///and it DRX3D_ENABLE_GYROPSCOPIC_FORCE becomes equivalent to DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY
	///See Demos/GyroscopicDemo and computeGyroscopicImpulseImplicit
	DRX3D_ENABLE_GYROSCOPIC_FORCE_EXPLICIT = 2,
	DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_WORLD = 4,
	DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY = 8,
	DRX3D_ENABLE_GYROPSCOPIC_FORCE = DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY,
};

///The RigidBody is the main class for rigid body objects. It is derived from CollisionObject2, so it keeps a pointer to a CollisionShape.
///It is recommended for performance and memory use to share CollisionShape objects whenever possible.
///There are 3 types of rigid bodies:
///- A) Dynamic rigid bodies, with positive mass. Motion is controlled by rigid body dynamics.
///- B) Fixed objects with zero mass. They are not moving (basically collision objects)
///- C) Kinematic objects, which are objects without mass, but the user can move them. There is one-way interaction, and drx3D calculates a velocity based on the timestep and previous and current world transform.
///drx3D automatically deactivates dynamic rigid bodies, when the velocity is below a threshold for a given time.
///Deactivated (sleeping) rigid bodies don't take any processing time, except a minor broadphase collision detection impact (to allow active objects to activate/wake up sleeping objects)
class RigidBody : public CollisionObject2
{
	Matrix3x3 m_invInertiaTensorWorld;
	Vec3 m_linearVelocity;
	Vec3 m_angularVelocity;
	Scalar m_inverseMass;
	Vec3 m_linearFactor;

	Vec3 m_gravity;
	Vec3 m_gravity_acceleration;
	Vec3 m_invInertiaLocal;
	Vec3 m_totalForce;
	Vec3 m_totalTorque;

	Scalar m_linearDamping;
	Scalar m_angularDamping;

	bool m_additionalDamping;
	Scalar m_additionalDampingFactor;
	Scalar m_additionalLinearDampingThresholdSqr;
	Scalar m_additionalAngularDampingThresholdSqr;
	Scalar m_additionalAngularDampingFactor;

	Scalar m_linearSleepingThreshold;
	Scalar m_angularSleepingThreshold;

	//m_optionalMotionState allows to automatic synchronize the world transform for active objects
	MotionState* m_optionalMotionState;

	//keep track of typed constraints referencing this rigid body, to disable collision between linked bodies
	AlignedObjectArray<TypedConstraint*> m_constraintRefs;

	i32 m_rigidbodyFlags;

	i32 m_debugBodyId;

protected:
	ATTRIBUTE_ALIGNED16(Vec3 m_deltaLinearVelocity);
	Vec3 m_deltaAngularVelocity;
	Vec3 m_angularFactor;
	Vec3 m_invMass;
	Vec3 m_pushVelocity;
	Vec3 m_turnVelocity;

public:
	///The RigidBodyConstructionInfo structure provides information to create a rigid body. Setting mass to zero creates a fixed (non-dynamic) rigid body.
	///For dynamic objects, you can use the collision shape to approximate the local inertia tensor, otherwise use the zero vector (default argument)
	///You can use the motion state to synchronize the world transform between physics and graphics objects.
	///And if the motion state is provided, the rigid body will initialize its initial world transform from the motion state,
	///m_startWorldTransform is only used when you don't provide a motion state.
	struct RigidBodyConstructionInfo
	{
		Scalar m_mass;

		///When a motionState is provided, the rigid body will initialize its world transform from the motion state
		///In this case, m_startWorldTransform is ignored.
		MotionState* m_motionState;
		Transform2 m_startWorldTransform;

		CollisionShape* m_collisionShape;
		Vec3 m_localInertia;
		Scalar m_linearDamping;
		Scalar m_angularDamping;

		///best simulation results when friction is non-zero
		Scalar m_friction;
		///the m_rollingFriction prevents rounded shapes, such as spheres, cylinders and capsules from rolling forever.
		///See drx3D/Demos/RollingFrictionDemo for usage
		Scalar m_rollingFriction;
		Scalar m_spinningFriction;  //torsional friction around contact normal

		///best simulation results using zero restitution.
		Scalar m_restitution;

		Scalar m_linearSleepingThreshold;
		Scalar m_angularSleepingThreshold;

		//Additional damping can help avoiding lowpass jitter motion, help stability for ragdolls etc.
		//Such damping is undesirable, so once the overall simulation quality of the rigid body dynamics system has improved, this should become obsolete
		bool m_additionalDamping;
		Scalar m_additionalDampingFactor;
		Scalar m_additionalLinearDampingThresholdSqr;
		Scalar m_additionalAngularDampingThresholdSqr;
		Scalar m_additionalAngularDampingFactor;

		RigidBodyConstructionInfo(Scalar mass, MotionState* motionState, CollisionShape* collisionShape, const Vec3& localInertia = Vec3(0, 0, 0)) : m_mass(mass),
																																									   m_motionState(motionState),
																																									   m_collisionShape(collisionShape),
																																									   m_localInertia(localInertia),
																																									   m_linearDamping(Scalar(0.)),
																																									   m_angularDamping(Scalar(0.)),
																																									   m_friction(Scalar(0.5)),
																																									   m_rollingFriction(Scalar(0)),
																																									   m_spinningFriction(Scalar(0)),
																																									   m_restitution(Scalar(0.)),
																																									   m_linearSleepingThreshold(Scalar(0.8)),
																																									   m_angularSleepingThreshold(Scalar(1.f)),
																																									   m_additionalDamping(false),
																																									   m_additionalDampingFactor(Scalar(0.005)),
																																									   m_additionalLinearDampingThresholdSqr(Scalar(0.01)),
																																									   m_additionalAngularDampingThresholdSqr(Scalar(0.01)),
																																									   m_additionalAngularDampingFactor(Scalar(0.01))
		{
			m_startWorldTransform.setIdentity();
		}
	};

	//RigidBody constructor using construction info
	RigidBody(const RigidBodyConstructionInfo& constructionInfo);

	//RigidBody constructor for backwards compatibility.
	///To specify friction (etc) during rigid body construction, please use the other constructor (using RigidBodyConstructionInfo)
	RigidBody(Scalar mass, MotionState* motionState, CollisionShape* collisionShape, const Vec3& localInertia = Vec3(0, 0, 0));

	virtual ~RigidBody()
	{
		//No constraints should point to this rigidbody
		//Remove constraints from the dynamics world before you delete the related rigidbodies.
		Assert(m_constraintRefs.size() == 0);
	}

protected:
	///setupRigidBody is only used internally by the constructor
	void setupRigidBody(const RigidBodyConstructionInfo& constructionInfo);

public:
	void proceedToTransform2(const Transform2& newTrans);

	///to keep collision detection and dynamics separate we don't store a rigidbody pointer
	///but a rigidbody is derived from CollisionObject2, so we can safely perform an upcast
	static const RigidBody* upcast(const CollisionObject2* colObj)
	{
		if (colObj->getInternalType() & CollisionObject2::CO_RIGID_BODY)
			return (const RigidBody*)colObj;
		return 0;
	}
	static RigidBody* upcast(CollisionObject2* colObj)
	{
		if (colObj->getInternalType() & CollisionObject2::CO_RIGID_BODY)
			return (RigidBody*)colObj;
		return 0;
	}

	/// continuous collision detection needs prediction
	void predictIntegratedTransform2(Scalar step, Transform2& predictedTransform2);

	void saveKinematicState(Scalar step);

	void applyGravity();
    
    void clearGravity();

	void setGravity(const Vec3& acceleration);

	const Vec3& getGravity() const
	{
		return m_gravity_acceleration;
	}

	void setDamping(Scalar lin_damping, Scalar ang_damping);

	Scalar getLinearDamping() const
	{
		return m_linearDamping;
	}

	Scalar getAngularDamping() const
	{
		return m_angularDamping;
	}

	Scalar getLinearSleepingThreshold() const
	{
		return m_linearSleepingThreshold;
	}

	Scalar getAngularSleepingThreshold() const
	{
		return m_angularSleepingThreshold;
	}

	void applyDamping(Scalar timeStep);

	SIMD_FORCE_INLINE const CollisionShape* getCollisionShape() const
	{
		return m_collisionShape;
	}

	SIMD_FORCE_INLINE CollisionShape* getCollisionShape()
	{
		return m_collisionShape;
	}

	void setMassProps(Scalar mass, const Vec3& inertia);

	const Vec3& getLinearFactor() const
	{
		return m_linearFactor;
	}
	void setLinearFactor(const Vec3& linearFactor)
	{
		m_linearFactor = linearFactor;
		m_invMass = m_linearFactor * m_inverseMass;
	}
	Scalar getInvMass() const { return m_inverseMass; }
	Scalar getMass() const { return m_inverseMass == Scalar(0.) ? Scalar(0.) : Scalar(1.0) / m_inverseMass; }
	const Matrix3x3& getInvInertiaTensorWorld() const
	{
		return m_invInertiaTensorWorld;
	}

	void integrateVelocities(Scalar step);

	void setCenterOfMassTransform(const Transform2& xform);

	void applyCentralForce(const Vec3& force)
	{
		m_totalForce += force * m_linearFactor;
	}

	const Vec3& getTotalForce() const
	{
		return m_totalForce;
	};

	const Vec3& getTotalTorque() const
	{
		return m_totalTorque;
	};

	const Vec3& getInvInertiaDiagLocal() const
	{
		return m_invInertiaLocal;
	};

	void setInvInertiaDiagLocal(const Vec3& diagInvInertia)
	{
		m_invInertiaLocal = diagInvInertia;
	}

	void setSleepingThresholds(Scalar linear, Scalar angular)
	{
		m_linearSleepingThreshold = linear;
		m_angularSleepingThreshold = angular;
	}

	void applyTorque(const Vec3& torque)
	{
		m_totalTorque += torque * m_angularFactor;
		#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
		clampVelocity(m_totalTorque);
		#endif
	}

	void applyForce(const Vec3& force, const Vec3& rel_pos)
	{
		applyCentralForce(force);
		applyTorque(rel_pos.cross(force * m_linearFactor));
	}

	void applyCentralImpulse(const Vec3& impulse)
	{
		m_linearVelocity += impulse * m_linearFactor * m_inverseMass;
		#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
		clampVelocity(m_linearVelocity);
		#endif
	}

	void applyTorqueImpulse(const Vec3& torque)
	{
		m_angularVelocity += m_invInertiaTensorWorld * torque * m_angularFactor;
		#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
		clampVelocity(m_angularVelocity);
		#endif
	}

	void applyImpulse(const Vec3& impulse, const Vec3& rel_pos)
	{
		if (m_inverseMass != Scalar(0.))
		{
			applyCentralImpulse(impulse);
			if (m_angularFactor)
			{
				applyTorqueImpulse(rel_pos.cross(impulse * m_linearFactor));
			}
		}
	}
    
    void applyPushImpulse(const Vec3& impulse, const Vec3& rel_pos)
    {
        if (m_inverseMass != Scalar(0.))
        {
            applyCentralPushImpulse(impulse);
            if (m_angularFactor)
            {
                applyTorqueTurnImpulse(rel_pos.cross(impulse * m_linearFactor));
            }
        }
    }
    
    Vec3 getPushVelocity() const
    {
        return m_pushVelocity;
    }
    
    Vec3 getTurnVelocity() const
    {
        return m_turnVelocity;
    }
    
    void setPushVelocity(const Vec3& v)
    {
        m_pushVelocity = v;
    }

    #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
    void clampVelocity(Vec3& v) const {
        v.setX(
            fmax(-DRX3D_CLAMP_VELOCITY_TO,
                 fmin(DRX3D_CLAMP_VELOCITY_TO, v.getX()))
        );
        v.setY(
            fmax(-DRX3D_CLAMP_VELOCITY_TO,
                 fmin(DRX3D_CLAMP_VELOCITY_TO, v.getY()))
        );
        v.setZ(
            fmax(-DRX3D_CLAMP_VELOCITY_TO,
                 fmin(DRX3D_CLAMP_VELOCITY_TO, v.getZ()))
        );
    }
    #endif

    void setTurnVelocity(const Vec3& v)
    {
        m_turnVelocity = v;
        #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
        clampVelocity(m_turnVelocity);
        #endif
    }
    
    void applyCentralPushImpulse(const Vec3& impulse)
    {
        m_pushVelocity += impulse * m_linearFactor * m_inverseMass;
        #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
        clampVelocity(m_pushVelocity);
        #endif
    }
    
    void applyTorqueTurnImpulse(const Vec3& torque)
    {
        m_turnVelocity += m_invInertiaTensorWorld * torque * m_angularFactor;
        #if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
        clampVelocity(m_turnVelocity);
        #endif
    }

	void clearForces()
	{
		m_totalForce.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
		m_totalTorque.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	}

	void updateInertiaTensor();

	const Vec3& getCenterOfMassPosition() const
	{
		return m_worldTransform.getOrigin();
	}
	Quat getOrientation() const;

	const Transform2& getCenterOfMassTransform() const
	{
		return m_worldTransform;
	}
	const Vec3& getLinearVelocity() const
	{
		return m_linearVelocity;
	}
	const Vec3& getAngularVelocity() const
	{
		return m_angularVelocity;
	}

	inline void setLinearVelocity(const Vec3& lin_vel)
	{
		m_updateRevision++;
		m_linearVelocity = lin_vel;
		#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
		clampVelocity(m_linearVelocity);
		#endif
	}

	inline void setAngularVelocity(const Vec3& ang_vel)
	{
		m_updateRevision++;
		m_angularVelocity = ang_vel;
		#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
		clampVelocity(m_angularVelocity);
		#endif
	}

	Vec3 getVelocityInLocalPoint(const Vec3& rel_pos) const
	{
		//we also calculate lin/ang velocity for kinematic objects
		return m_linearVelocity + m_angularVelocity.cross(rel_pos);

		//for kinematic objects, we could also use use:
		//		return 	(m_worldTransform(rel_pos) - m_interpolationWorldTransform(rel_pos)) / m_kinematicTimeStep;
	}
    
    Vec3 getPushVelocityInLocalPoint(const Vec3& rel_pos) const
    {
        //we also calculate lin/ang velocity for kinematic objects
        return m_pushVelocity + m_turnVelocity.cross(rel_pos);
    }

	void translate(const Vec3& v)
	{
		m_worldTransform.getOrigin() += v;
	}

	void getAabb(Vec3& aabbMin, Vec3& aabbMax) const;

	SIMD_FORCE_INLINE Scalar computeImpulseDenominator(const Vec3& pos, const Vec3& normal) const
	{
		Vec3 r0 = pos - getCenterOfMassPosition();

		Vec3 c0 = (r0).cross(normal);

		Vec3 vec = (c0 * getInvInertiaTensorWorld()).cross(r0);

		return m_inverseMass + normal.dot(vec);
	}

	SIMD_FORCE_INLINE Scalar computeAngularImpulseDenominator(const Vec3& axis) const
	{
		Vec3 vec = axis * getInvInertiaTensorWorld();
		return axis.dot(vec);
	}

	SIMD_FORCE_INLINE void updateDeactivation(Scalar timeStep)
	{
		if ((getActivationState() == ISLAND_SLEEPING) || (getActivationState() == DISABLE_DEACTIVATION))
			return;

		if ((getLinearVelocity().length2() < m_linearSleepingThreshold * m_linearSleepingThreshold) &&
			(getAngularVelocity().length2() < m_angularSleepingThreshold * m_angularSleepingThreshold))
		{
			m_deactivationTime += timeStep;
		}
		else
		{
			m_deactivationTime = Scalar(0.);
			setActivationState(0);
		}
	}

	SIMD_FORCE_INLINE bool wantsSleeping()
	{
		if (getActivationState() == DISABLE_DEACTIVATION)
			return false;

		//disable deactivation
		if (gDisableDeactivation || (gDeactivationTime == Scalar(0.)))
			return false;

		if ((getActivationState() == ISLAND_SLEEPING) || (getActivationState() == WANTS_DEACTIVATION))
			return true;

		if (m_deactivationTime > gDeactivationTime)
		{
			return true;
		}
		return false;
	}

	const BroadphaseProxy* getBroadphaseProxy() const
	{
		return m_broadphaseHandle;
	}
	BroadphaseProxy* getBroadphaseProxy()
	{
		return m_broadphaseHandle;
	}
	void setNewBroadphaseProxy(BroadphaseProxy* broadphaseProxy)
	{
		m_broadphaseHandle = broadphaseProxy;
	}

	//MotionState allows to automatic synchronize the world transform for active objects
	MotionState* getMotionState()
	{
		return m_optionalMotionState;
	}
	const MotionState* getMotionState() const
	{
		return m_optionalMotionState;
	}
	void setMotionState(MotionState* motionState)
	{
		m_optionalMotionState = motionState;
		if (m_optionalMotionState)
			motionState->getWorldTransform(m_worldTransform);
	}

	//for experimental overriding of friction/contact solver func
	i32 m_contactSolverType;
	i32 m_frictionSolverType;

	void setAngularFactor(const Vec3& angFac)
	{
		m_updateRevision++;
		m_angularFactor = angFac;
	}

	void setAngularFactor(Scalar angFac)
	{
		m_updateRevision++;
		m_angularFactor.setVal(angFac, angFac, angFac);
	}
	const Vec3& getAngularFactor() const
	{
		return m_angularFactor;
	}

	//is this rigidbody added to a CollisionWorldDynamicsWorldBroadphase?
	bool isInWorld() const
	{
		return (getBroadphaseProxy() != 0);
	}

	void addConstraintRef(TypedConstraint* c);
	void removeConstraintRef(TypedConstraint* c);

	TypedConstraint* getConstraintRef(i32 index)
	{
		return m_constraintRefs[index];
	}

	i32 getNumConstraintRefs() const
	{
		return m_constraintRefs.size();
	}

	void setFlags(i32 flags)
	{
		m_rigidbodyFlags = flags;
	}

	i32 getFlags() const
	{
		return m_rigidbodyFlags;
	}

	///perform implicit force computation in world space
	Vec3 computeGyroscopicImpulseImplicit_World(Scalar dt) const;

	///perform implicit force computation in body space (inertial frame)
	Vec3 computeGyroscopicImpulseImplicit_Body(Scalar step) const;

	///explicit version is best avoided, it gains energy
	Vec3 computeGyroscopicForceExplicit(Scalar maxGyroscopicForce) const;
	Vec3 getLocalInertia() const;

	///////////////////////////////////////////////

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, class Serializer* serializer) const;

	virtual void serializeSingleObject(class Serializer* serializer) const;
};

//@todo add m_optionalMotionState and m_constraintRefs to RigidBodyData
///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct RigidBodyFloatData
{
	CollisionObject2FloatData m_collisionObjectData;
	Matrix3x3FloatData m_invInertiaTensorWorld;
	Vec3FloatData m_linearVelocity;
	Vec3FloatData m_angularVelocity;
	Vec3FloatData m_angularFactor;
	Vec3FloatData m_linearFactor;
	Vec3FloatData m_gravity;
	Vec3FloatData m_gravity_acceleration;
	Vec3FloatData m_invInertiaLocal;
	Vec3FloatData m_totalForce;
	Vec3FloatData m_totalTorque;
	float m_inverseMass;
	float m_linearDamping;
	float m_angularDamping;
	float m_additionalDampingFactor;
	float m_additionalLinearDampingThresholdSqr;
	float m_additionalAngularDampingThresholdSqr;
	float m_additionalAngularDampingFactor;
	float m_linearSleepingThreshold;
	float m_angularSleepingThreshold;
	i32 m_additionalDamping;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct RigidBodyDoubleData
{
	CollisionObject2DoubleData m_collisionObjectData;
	Matrix3x3DoubleData m_invInertiaTensorWorld;
	Vec3DoubleData m_linearVelocity;
	Vec3DoubleData m_angularVelocity;
	Vec3DoubleData m_angularFactor;
	Vec3DoubleData m_linearFactor;
	Vec3DoubleData m_gravity;
	Vec3DoubleData m_gravity_acceleration;
	Vec3DoubleData m_invInertiaLocal;
	Vec3DoubleData m_totalForce;
	Vec3DoubleData m_totalTorque;
	double m_inverseMass;
	double m_linearDamping;
	double m_angularDamping;
	double m_additionalDampingFactor;
	double m_additionalLinearDampingThresholdSqr;
	double m_additionalAngularDampingThresholdSqr;
	double m_additionalAngularDampingFactor;
	double m_linearSleepingThreshold;
	double m_angularSleepingThreshold;
	i32 m_additionalDamping;
	char m_padding[4];
};

#endif  //DRX3D_RIGIDBODY_H
