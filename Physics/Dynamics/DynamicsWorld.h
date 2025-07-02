#ifndef DRX3D_DYNAMICS_WORLD_H
#define DRX3D_DYNAMICS_WORLD_H

#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>

class TypedConstraint;
class ActionInterface;
class ConstraintSolver;
class DynamicsWorld;

/// Type for the callback for each tick
typedef void (*InternalTickCallback)(DynamicsWorld* world, Scalar timeStep);

enum DynamicsWorldType
{
	DRX3D_SIMPLE_DYNAMICS_WORLD = 1,
	DRX3D_DISCRETE_DYNAMICS_WORLD = 2,
	DRX3D_CONTINUOUS_DYNAMICS_WORLD = 3,
	DRX3D_SOFT_RIGID_DYNAMICS_WORLD = 4,
	DRX3D_GPU_DYNAMICS_WORLD = 5,
	DRX3D_SOFT_MULTIBODY_DYNAMICS_WORLD = 6,
    DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD = 7
};

///The DynamicsWorld is the interface class for several dynamics implementation, basic, discrete, parallel, and continuous etc.
class DynamicsWorld : public CollisionWorld
{
protected:
	InternalTickCallback m_internalTickCallback;
	InternalTickCallback m_internalPreTickCallback;
	uk m_worldUserInfo;

	ContactSolverInfo m_solverInfo;

public:
	DynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* broadphase, CollisionConfiguration* collisionConfiguration)
		: CollisionWorld(dispatcher, broadphase, collisionConfiguration), m_internalTickCallback(0), m_internalPreTickCallback(0), m_worldUserInfo(0)
	{
	}

	virtual ~DynamicsWorld()
	{
	}

	///stepSimulation proceeds the simulation over 'timeStep', units in preferably in seconds.
	///By default, drx3D will subdivide the timestep in constant substeps of each 'fixedTimeStep'.
	///in order to keep the simulation real-time, the maximum number of substeps can be clamped to 'maxSubSteps'.
	///You can disable subdividing the timestep/substepping by passing maxSubSteps=0 as second argument to stepSimulation, but in that case you have to keep the timeStep constant.
	virtual i32 stepSimulation(Scalar timeStep, i32 maxSubSteps = 1, Scalar fixedTimeStep = Scalar(1.) / Scalar(60.)) = 0;

	virtual void debugDrawWorld() = 0;

	virtual void addConstraint(TypedConstraint* constraint, bool disableCollisionsBetweenLinkedBodies = false)
	{
		(void)constraint;
		(void)disableCollisionsBetweenLinkedBodies;
	}

	virtual void removeConstraint(TypedConstraint* constraint) { (void)constraint; }

	virtual void addAction(ActionInterface* action) = 0;

	virtual void removeAction(ActionInterface* action) = 0;

	//once a rigidbody is added to the dynamics world, it will get this gravity assigned
	//existing rigidbodies in the world get gravity assigned too, during this method
	virtual void setGravity(const Vec3& gravity) = 0;
	virtual Vec3 getGravity() const = 0;

	virtual void synchronizeMotionStates() = 0;

	virtual void addRigidBody(RigidBody* body) = 0;

	virtual void addRigidBody(RigidBody* body, i32 group, i32 mask) = 0;

	virtual void removeRigidBody(RigidBody* body) = 0;

	virtual void setConstraintSolver(ConstraintSolver* solver) = 0;

	virtual ConstraintSolver* getConstraintSolver() = 0;

	virtual i32 getNumConstraints() const { return 0; }

	virtual TypedConstraint* getConstraint(i32 index)
	{
		(void)index;
		return 0;
	}

	virtual const TypedConstraint* getConstraint(i32 index) const
	{
		(void)index;
		return 0;
	}

	virtual DynamicsWorldType getWorldType() const = 0;

	virtual void clearForces() = 0;

	/// Set the callback for when an internal tick (simulation substep) happens, optional user info
	void setInternalTickCallback(InternalTickCallback cb, uk worldUserInfo = 0, bool isPreTick = false)
	{
		if (isPreTick)
		{
			m_internalPreTickCallback = cb;
		}
		else
		{
			m_internalTickCallback = cb;
		}
		m_worldUserInfo = worldUserInfo;
	}

	void setWorldUserInfo(uk worldUserInfo)
	{
		m_worldUserInfo = worldUserInfo;
	}

	uk getWorldUserInfo() const
	{
		return m_worldUserInfo;
	}

	ContactSolverInfo& getSolverInfo()
	{
		return m_solverInfo;
	}

	const ContactSolverInfo& getSolverInfo() const
	{
		return m_solverInfo;
	}

	///obsolete, use addAction instead.
	virtual void addVehicle(ActionInterface* vehicle) { (void)vehicle; }
	///obsolete, use removeAction instead
	virtual void removeVehicle(ActionInterface* vehicle) { (void)vehicle; }
	///obsolete, use addAction instead.
	virtual void addCharacter(ActionInterface* character) { (void)character; }
	///obsolete, use removeAction instead
	virtual void removeCharacter(ActionInterface* character) { (void)character; }
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct DynamicsWorldDoubleData
{
	ContactSolverInfoDoubleData m_solverInfo;
	Vec3DoubleData m_gravity;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct DynamicsWorldFloatData
{
	ContactSolverInfoFloatData m_solverInfo;
	Vec3FloatData m_gravity;
};

#endif  //DRX3D_DYNAMICS_WORLD_H
