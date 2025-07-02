#ifndef D3_GPU_RIGIDBODY_PIPELINE_INTERNAL_DATA_H
#define D3_GPU_RIGIDBODY_PIPELINE_INTERNAL_DATA_H

#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Collidable.h>

#include <drx3D/Physics/Collision/BroadPhase/b3SapAabb.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Config.h>

#include <drx3D/Physics/Collision/BroadPhase/b3OverlappingPair.h>
#include <drx3D/OpenCL/RigidBody/b3GpuGenericConstraint.h>

struct b3GpuRigidBodyPipelineInternalData
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

	cl_kernel m_integrateTransformsKernel;
	cl_kernel m_updateAabbsKernel;
	cl_kernel m_clearOverlappingPairsKernel;

	class b3PgsJacobiSolver* m_solver;

	class b3GpuPgsConstraintSolver* m_gpuSolver;

	class b3GpuPgsContactSolver* m_solver2;
	class b3GpuJacobiContactSolver* m_solver3;
	class b3GpuRaycast* m_raycaster;

	class b3GpuBroadphaseInterface* m_broadphaseSap;

	struct b3DynamicBvhBroadphase* m_broadphaseDbvt;
	b3OpenCLArray<b3SapAabb>* m_allAabbsGPU;
	b3AlignedObjectArray<b3SapAabb> m_allAabbsCPU;
	b3OpenCLArray<b3BroadphasePair>* m_overlappingPairsGPU;

	b3OpenCLArray<b3GpuGenericConstraint>* m_gpuConstraints;
	b3AlignedObjectArray<b3GpuGenericConstraint> m_cpuConstraints;

	b3AlignedObjectArray<b3TypedConstraint*> m_joints;
	i32 m_constraintUid;
	class b3GpuNarrowPhase* m_narrowphase;
	b3Vec3 m_gravity;

	b3Config m_config;
};

#endif  //D3_GPU_RIGIDBODY_PIPELINE_INTERNAL_DATA_H
