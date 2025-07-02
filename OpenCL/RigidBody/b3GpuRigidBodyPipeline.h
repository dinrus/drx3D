#ifndef D3_GPU_RIGIDBODY_PIPELINE_H
#define D3_GPU_RIGIDBODY_PIPELINE_H

#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Config.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3RaycastInfo.h>

class b3GpuRigidBodyPipeline
{
protected:
	struct b3GpuRigidBodyPipelineInternalData* m_data;

	i32 allocateCollidable();

public:
	b3GpuRigidBodyPipeline(cl_context ctx, cl_device_id device, cl_command_queue q, class b3GpuNarrowPhase* narrowphase, class b3GpuBroadphaseInterface* broadphaseSap, struct b3DynamicBvhBroadphase* broadphaseDbvt, const b3Config& config);
	virtual ~b3GpuRigidBodyPipeline();

	void stepSimulation(float deltaTime);
	void integrate(float timeStep);
	void setupGpuAabbsFull();

	i32 registerConvexPolyhedron(class b3ConvexUtility* convex);

	//i32		registerConvexPolyhedron(const float* vertices, i32 strideInBytes, i32 numVertices, const float* scaling);
	//i32		registerSphereShape(float radius);
	//i32		registerPlaneShape(const b3Vec3& planeNormal, float planeConstant);

	//i32		registerConcaveMesh(b3AlignedObjectArray<b3Vec3>* vertices, b3AlignedObjectArray<i32>* indices, const float* scaling);
	//i32		registerCompoundShape(b3AlignedObjectArray<b3GpuChildShape>* childShapes);

	i32 registerPhysicsInstance(float mass, const float* position, const float* orientation, i32 collisionShapeIndex, i32 userData, bool writeInstanceToGpu);
	//if you passed "writeInstanceToGpu" false in the registerPhysicsInstance method (for performance) you need to call writeAllInstancesToGpu after all instances are registered
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

	cl_mem getBodyBuffer();

	i32 getNumBodies() const;
};

#endif  //D3_GPU_RIGIDBODY_PIPELINE_H