#ifndef D3_CPU_RIGIDBODY_PIPELINE_H
#define D3_CPU_RIGIDBODY_PIPELINE_H

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3RaycastInfo.h>

class b3CpuRigidBodyPipeline
{
protected:
	struct b3CpuRigidBodyPipelineInternalData* m_data;

	i32 allocateCollidable();

public:
	b3CpuRigidBodyPipeline(class b3CpuNarrowPhase* narrowphase, struct b3DynamicBvhBroadphase* broadphaseDbvt, const struct b3Config& config);
	virtual ~b3CpuRigidBodyPipeline();

	virtual void stepSimulation(float deltaTime);
	virtual void integrate(float timeStep);
	virtual void updateAabbWorldSpace();
	virtual void computeOverlappingPairs();
	virtual void computeContactPoints();
	virtual void solveContactConstraints();

	i32 registerConvexPolyhedron(class b3ConvexUtility* convex);

	i32 registerPhysicsInstance(float mass, const float* position, const float* orientation, i32 collisionShapeIndex, i32 userData);
	void writeAllInstancesToGpu();
	void copyConstraintsToHost();
	void setGravity(const float* grav);
	void reset();

	i32 createPoint2PointConstraint(i32 bodyA, i32 bodyB, const float* pivotInA, const float* pivotInB, float breakingThreshold);
	i32 createFixedConstraint(i32 bodyA, i32 bodyB, const float* pivotInA, const float* pivotInB, const float* relTargetAB, float breakingThreshold);
	void removeConstraintByUid(i32 uid);

	void addConstraint(class b3TypedConstraint* constraint);
	void removeConstraint(b3TypedConstraint* constraint);

	void castRays(const b3AlignedObjectArray<b3RayInfo>& rays, b3AlignedObjectArray<b3RayHit>& hitResults);

	const struct b3RigidBodyData* getBodyBuffer() const;

	i32 getNumBodies() const;
};

#endif  //D3_CPU_RIGIDBODY_PIPELINE_H