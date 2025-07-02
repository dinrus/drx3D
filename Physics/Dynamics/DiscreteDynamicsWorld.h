#ifndef DRX3D_DISCRETE_DYNAMICS_WORLD_H
#define DRX3D_DISCRETE_DYNAMICS_WORLD_H

#include <drx3D/Physics/Dynamics/DynamicsWorld.h>
class Dispatcher;
class OverlappingPairCache;
class ConstraintSolver;
class SimulationIslandManager;
class TypedConstraint;
class ActionInterface;
class PersistentManifold;
class IDebugDraw;

struct InplaceSolverIslandCallback;

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Threads.h>

//DiscreteDynamicsWorld provides discrete rigid body simulation
///those classes replace the obsolete CcdPhysicsEnvironment/CcdPhysicsController
ATTRIBUTE_ALIGNED16(class)
DiscreteDynamicsWorld : public DynamicsWorld
{
protected:
	AlignedObjectArray<TypedConstraint*> m_sortedConstraints;
	InplaceSolverIslandCallback* m_solverIslandCallback;

	ConstraintSolver* m_constraintSolver;

	SimulationIslandManager* m_islandManager;

	AlignedObjectArray<TypedConstraint*> m_constraints;

	AlignedObjectArray<RigidBody*> m_nonStaticRigidBodies;

	Vec3 m_gravity;

	//for variable timesteps
	Scalar m_localTime;
	Scalar m_fixedTimeStep;
	//for variable timesteps

	bool m_ownsIslandManager;
	bool m_ownsConstraintSolver;
	bool m_synchronizeAllMotionStates;
	bool m_applySpeculativeContactRestitution;

	AlignedObjectArray<ActionInterface*> m_actions;

	i32 m_profileTimings;

	bool m_latencyMotionStateInterpolation;

	AlignedObjectArray<PersistentManifold*> m_predictiveManifolds;
	SpinMutex m_predictiveManifoldsMutex;  // used to synchronize threads creating predictive contacts

	virtual void predictUnconstraintMotion(Scalar timeStep);

	void integrateTransformsInternal(RigidBody * *bodies, i32 numBodies, Scalar timeStep);  // can be called in parallel
	virtual void integrateTransforms(Scalar timeStep);

	virtual void calculateSimulationIslands();

	

	virtual void updateActivationState(Scalar timeStep);

	void updateActions(Scalar timeStep);

	void startProfiling(Scalar timeStep);

	virtual void internalSingleStepSimulation(Scalar timeStep);

	void releasePredictiveContacts();
	void createPredictiveContactsInternal(RigidBody * *bodies, i32 numBodies, Scalar timeStep);  // can be called in parallel
	virtual void createPredictiveContacts(Scalar timeStep);

	virtual void saveKinematicState(Scalar timeStep);

	void serializeRigidBodies(Serializer * serializer);

	void serializeDynamicsWorldInfo(Serializer * serializer);
    
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	///this DiscreteDynamicsWorld constructor gets created objects from the user, and will not delete those
	DiscreteDynamicsWorld(Dispatcher * dispatcher, BroadphaseInterface * pairCache, ConstraintSolver * constraintSolver, CollisionConfiguration * collisionConfiguration);

	virtual ~DiscreteDynamicsWorld();

	///if maxSubSteps > 0, it will interpolate motion between fixedTimeStep's
	virtual i32 stepSimulation(Scalar timeStep, i32 maxSubSteps = 1, Scalar fixedTimeStep = Scalar(1.) / Scalar(60.));

    virtual void solveConstraints(ContactSolverInfo & solverInfo);
    
	virtual void synchronizeMotionStates();

	///this can be useful to synchronize a single rigid body -> graphics object
	void synchronizeSingleMotionState(RigidBody * body);

	virtual void addConstraint(TypedConstraint * constraint, bool disableCollisionsBetweenLinkedBodies = false);

	virtual void removeConstraint(TypedConstraint * constraint);

	virtual void addAction(ActionInterface*);

	virtual void removeAction(ActionInterface*);

	SimulationIslandManager* getSimulationIslandManager()
	{
		return m_islandManager;
	}

	const SimulationIslandManager* getSimulationIslandManager() const
	{
		return m_islandManager;
	}

	CollisionWorld* getCollisionWorld()
	{
		return this;
	}

	virtual void setGravity(const Vec3& gravity);

	virtual Vec3 getGravity() const;

	virtual void addCollisionObject(CollisionObject2* collisionObject, i32 collisionFilterGroup = BroadphaseProxy::StaticFilter, i32 collisionFilterMask = BroadphaseProxy::AllFilter ^ BroadphaseProxy::StaticFilter);

	virtual void addRigidBody(RigidBody * body);

	virtual void addRigidBody(RigidBody * body, i32 group, i32 mask);

	virtual void removeRigidBody(RigidBody * body);

	///removeCollisionObject will first check if it is a rigid body, if so call removeRigidBody otherwise call CollisionWorld::removeCollisionObject
	virtual void removeCollisionObject(CollisionObject2* collisionObject);

	virtual void debugDrawConstraint(TypedConstraint * constraint);

	virtual void debugDrawWorld();

	virtual void setConstraintSolver(ConstraintSolver * solver);

	virtual ConstraintSolver* getConstraintSolver();

	virtual i32 getNumConstraints() const;

	virtual TypedConstraint* getConstraint(i32 index);

	virtual const TypedConstraint* getConstraint(i32 index) const;

	virtual DynamicsWorldType getWorldType() const
	{
		return DRX3D_DISCRETE_DYNAMICS_WORLD;
	}

	///the forces on each rigidbody is accumulating together with gravity. clear this after each timestep.
	virtual void clearForces();

	///apply gravity, call this once per timestep
	virtual void applyGravity();

	virtual void setNumTasks(i32 numTasks)
	{
		(void)numTasks;
	}

	///obsolete, use updateActions instead
	virtual void updateVehicles(Scalar timeStep)
	{
		updateActions(timeStep);
	}

	///obsolete, use addAction instead
	virtual void addVehicle(ActionInterface * vehicle);
	///obsolete, use removeAction instead
	virtual void removeVehicle(ActionInterface * vehicle);
	///obsolete, use addAction instead
	virtual void addCharacter(ActionInterface * character);
	///obsolete, use removeAction instead
	virtual void removeCharacter(ActionInterface * character);

	void setSynchronizeAllMotionStates(bool synchronizeAll)
	{
		m_synchronizeAllMotionStates = synchronizeAll;
	}
	bool getSynchronizeAllMotionStates() const
	{
		return m_synchronizeAllMotionStates;
	}

	void setApplySpeculativeContactRestitution(bool enable)
	{
		m_applySpeculativeContactRestitution = enable;
	}

	bool getApplySpeculativeContactRestitution() const
	{
		return m_applySpeculativeContactRestitution;
	}

	///Preliminary serialization test for drx3D 2.76. Loading those files requires a separate parser (see drx3D/Demos/SerializeDemo)
	virtual void serialize(Serializer * serializer);

	///Interpolate motion state between previous and current transform, instead of current and next transform.
	///This can relieve discontinuities in the rendering, due to penetrations
	void setLatencyMotionStateInterpolation(bool latencyInterpolation)
	{
		m_latencyMotionStateInterpolation = latencyInterpolation;
	}
	bool getLatencyMotionStateInterpolation() const
	{
		return m_latencyMotionStateInterpolation;
	}
    
    AlignedObjectArray<RigidBody*>& getNonStaticRigidBodies()
    {
        return m_nonStaticRigidBodies;
    }
    
    const AlignedObjectArray<RigidBody*>& getNonStaticRigidBodies() const
    {
        return m_nonStaticRigidBodies;
    }
};

#endif  //DRX3D_DISCRETE_DYNAMICS_WORLD_H
