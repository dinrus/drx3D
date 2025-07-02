#ifndef DRX3D_SOFT_RIGID_DYNAMICS_WORLD_H
#define DRX3D_SOFT_RIGID_DYNAMICS_WORLD_H

#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>

typedef AlignedObjectArray<SoftBody*> SoftBodyArray;

class SoftBodySolver;

class SoftRigidDynamicsWorld : public DiscreteDynamicsWorld
{
	SoftBodyArray m_softBodies;
	i32 m_drawFlags;
	bool m_drawNodeTree;
	bool m_drawFaceTree;
	bool m_drawClusterTree;
	SoftBodyWorldInfo m_sbi;
	///Solver classes that encapsulate multiple soft bodies for solving
	SoftBodySolver* m_softBodySolver;
	bool m_ownsSolver;

protected:
	virtual void predictUnconstraintMotion(Scalar timeStep);

	virtual void internalSingleStepSimulation(Scalar timeStep);

	void solveSoftBodiesConstraints(Scalar timeStep);

	void serializeSoftBodies(Serializer* serializer);

public:
	SoftRigidDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, ConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration, SoftBodySolver* softBodySolver = 0);

	virtual ~SoftRigidDynamicsWorld();

	virtual void debugDrawWorld();

	void addSoftBody(SoftBody* body, i32 collisionFilterGroup = BroadphaseProxy::DefaultFilter, i32 collisionFilterMask = BroadphaseProxy::AllFilter);

	void removeSoftBody(SoftBody* body);

	///removeCollisionObject will first check if it is a rigid body, if so call removeRigidBody otherwise call DiscreteDynamicsWorld::removeCollisionObject
	virtual void removeCollisionObject(CollisionObject2* collisionObject);

	i32 getDrawFlags() const { return (m_drawFlags); }
	void setDrawFlags(i32 f) { m_drawFlags = f; }

	SoftBodyWorldInfo& getWorldInfo()
	{
		return m_sbi;
	}
	const SoftBodyWorldInfo& getWorldInfo() const
	{
		return m_sbi;
	}

	virtual DynamicsWorldType getWorldType() const
	{
		return DRX3D_SOFT_RIGID_DYNAMICS_WORLD;
	}

	SoftBodyArray& getSoftBodyArray()
	{
		return m_softBodies;
	}

	const SoftBodyArray& getSoftBodyArray() const
	{
		return m_softBodies;
	}

	virtual void rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, RayResultCallback& resultCallback) const;

	/// rayTestSingle performs a raycast call and calls the resultCallback. It is used internally by rayTest.
	/// In a future implementation, we consider moving the ray test as a virtual method in CollisionShape.
	/// This allows more customization.
	static void rayTestSingle(const Transform2& rayFromTrans, const Transform2& rayToTrans,
							  CollisionObject2* collisionObject,
							  const CollisionShape* collisionShape,
							  const Transform2& colObjWorldTransform,
							  RayResultCallback& resultCallback);

	virtual void serialize(Serializer* serializer);
};

#endif  //DRX3D_SOFT_RIGID_DYNAMICS_WORLD_H
