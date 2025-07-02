#ifndef DRX3D_SIMPLE_DYNAMICS_WORLD_H
#define DRX3D_SIMPLE_DYNAMICS_WORLD_H

#include <drx3D/Physics/Dynamics/DynamicsWorld.h>

class Dispatcher;
class OverlappingPairCache;
class ConstraintSolver;

///The SimpleDynamicsWorld serves as unit-test and to verify more complicated and optimized dynamics worlds.
///Please use DiscreteDynamicsWorld instead
class SimpleDynamicsWorld : public DynamicsWorld
{
protected:
	ConstraintSolver* m_constraintSolver;

	bool m_ownsConstraintSolver;

	void predictUnconstraintMotion(Scalar timeStep);

	void integrateTransforms(Scalar timeStep);

	Vec3 m_gravity;

public:
	///this SimpleDynamicsWorld constructor creates dispatcher, broadphase pairCache and constraintSolver
	SimpleDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration);

	virtual ~SimpleDynamicsWorld();

	///maxSubSteps/fixedTimeStep for interpolation is currently ignored for SimpleDynamicsWorld, use DiscreteDynamicsWorld instead
	virtual i32 stepSimulation(Scalar timeStep, i32 maxSubSteps = 1, Scalar fixedTimeStep = Scalar(1.) / Scalar(60.));

	virtual void setGravity(const Vec3& gravity);

	virtual Vec3 getGravity() const;

	virtual void addRigidBody(RigidBody* body);

	virtual void addRigidBody(RigidBody* body, i32 group, i32 mask);

	virtual void removeRigidBody(RigidBody* body);

	virtual void debugDrawWorld();

	virtual void addAction(ActionInterface* action);

	virtual void removeAction(ActionInterface* action);

	///removeCollisionObject will first check if it is a rigid body, if so call removeRigidBody otherwise call CollisionWorld::removeCollisionObject
	virtual void removeCollisionObject(CollisionObject2* collisionObject);

	virtual void updateAabbs();

	virtual void synchronizeMotionStates();

	virtual void setConstraintSolver(ConstraintSolver* solver);

	virtual ConstraintSolver* getConstraintSolver();

	virtual DynamicsWorldType getWorldType() const
	{
		return DRX3D_SIMPLE_DYNAMICS_WORLD;
	}

	virtual void clearForces();
};

#endif  //DRX3D_SIMPLE_DYNAMICS_WORLD_H
