#ifndef DRX3D_MULTIBODY_DYNAMICS_WORLD_H
#define DRX3D_MULTIBODY_DYNAMICS_WORLD_H

#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyInplaceSolverIslandCallback.h>

#define DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY

class MultiBody;
class MultiBodyConstraint;
class MultiBodyConstraintSolver;
struct MultiBodyInplaceSolverIslandCallback;

///The MultiBodyDynamicsWorld adds MultiBody multi body dynamics to drx3D
///This implementation is still preliminary/experimental.
class MultiBodyDynamicsWorld : public DiscreteDynamicsWorld
{
protected:
	AlignedObjectArray<MultiBody*> m_multiBodies;
	AlignedObjectArray<MultiBodyConstraint*> m_multiBodyConstraints;
	AlignedObjectArray<MultiBodyConstraint*> m_sortedMultiBodyConstraints;
	MultiBodyConstraintSolver* m_multiBodyConstraintSolver;
	MultiBodyInplaceSolverIslandCallback* m_solverMultiBodyIslandCallback;

	//cached data to avoid memory allocations
	AlignedObjectArray<Quat> m_scratch_world_to_local;
	AlignedObjectArray<Vec3> m_scratch_local_origin;
	AlignedObjectArray<Quat> m_scratch_world_to_local1;
	AlignedObjectArray<Vec3> m_scratch_local_origin1;
	AlignedObjectArray<Scalar> m_scratch_r;
	AlignedObjectArray<Vec3> m_scratch_v;
	AlignedObjectArray<Matrix3x3> m_scratch_m;

	virtual void calculateSimulationIslands();
	virtual void updateActivationState(Scalar timeStep);
	

	virtual void serializeMultiBodies(Serializer* serializer);

public:
	MultiBodyDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, MultiBodyConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration);

	virtual ~MultiBodyDynamicsWorld();
    
    virtual void solveConstraints(ContactSolverInfo& solverInfo);
    
	virtual void addMultiBody(MultiBody* body, i32 group = BroadphaseProxy::DefaultFilter, i32 mask = BroadphaseProxy::AllFilter);

	virtual void removeMultiBody(MultiBody* body);

	virtual i32 getNumMultibodies() const
	{
		return m_multiBodies.size();
	}

	MultiBody* getMultiBody(i32 mbIndex)
	{
		return m_multiBodies[mbIndex];
	}

	const MultiBody* getMultiBody(i32 mbIndex) const
	{
		return m_multiBodies[mbIndex];
	}

	virtual void addMultiBodyConstraint(MultiBodyConstraint* constraint);

	virtual i32 getNumMultiBodyConstraints() const
	{
		return m_multiBodyConstraints.size();
	}

	virtual MultiBodyConstraint* getMultiBodyConstraint(i32 constraintIndex)
	{
		return m_multiBodyConstraints[constraintIndex];
	}

	virtual const MultiBodyConstraint* getMultiBodyConstraint(i32 constraintIndex) const
	{
		return m_multiBodyConstraints[constraintIndex];
	}

	virtual void removeMultiBodyConstraint(MultiBodyConstraint* constraint);

	virtual void integrateTransforms(Scalar timeStep);
    void integrateMultiBodyTransforms(Scalar timeStep);
    void predictMultiBodyTransforms(Scalar timeStep);
    
    virtual void predictUnconstraintMotion(Scalar timeStep);
	virtual void debugDrawWorld();

	virtual void debugDrawMultiBodyConstraint(MultiBodyConstraint* constraint);

	void forwardKinematics();
	virtual void clearForces();
	virtual void clearMultiBodyConstraintForces();
	virtual void clearMultiBodyForces();
	virtual void applyGravity();

	virtual void serialize(Serializer* serializer);
	virtual void setMultiBodyConstraintSolver(MultiBodyConstraintSolver* solver);
	virtual void setConstraintSolver(ConstraintSolver* solver);
	virtual void getAnalyticsData(AlignedObjectArray<struct SolverAnalyticsData>& m_islandAnalyticsData) const;
    
    virtual void solveExternalForces(ContactSolverInfo& solverInfo);
    virtual void solveInternalConstraints(ContactSolverInfo& solverInfo);
    void buildIslands();

	virtual void saveKinematicState(Scalar timeStep);
};
#endif  //DRX3D_MULTIBODY_DYNAMICS_WORLD_H
