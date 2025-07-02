#ifndef D3_GPU_PGS_CONSTRAINT_SOLVER_H
#define D3_GPU_PGS_CONSTRAINT_SOLVER_H

struct b3Contact4;
struct b3ContactPoint;

class b3Dispatcher;

#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3ContactSolverInfo.h>
#include "b3GpuSolverBody.h"
#include "b3GpuSolverConstraint.h"
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
struct b3RigidBodyData;
struct b3InertiaData;

#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include "b3GpuGenericConstraint.h"

class b3GpuPgsConstraintSolver
{
protected:
	i32 m_staticIdx;
	struct b3GpuPgsJacobiSolverInternalData* m_gpuData;

protected:
	b3AlignedObjectArray<b3GpuSolverBody> m_tmpSolverBodyPool;
	b3GpuConstraintArray m_tmpSolverContactConstraintPool;
	b3GpuConstraintArray m_tmpSolverNonContactConstraintPool;
	b3GpuConstraintArray m_tmpSolverContactFrictionConstraintPool;
	b3GpuConstraintArray m_tmpSolverContactRollingFrictionConstraintPool;

	b3AlignedObjectArray<u32> m_tmpConstraintSizesPool;

	bool m_usePgs;
	void averageVelocities();

	i32 m_maxOverrideNumSolverIterations;

	i32 m_numSplitImpulseRecoveries;

	//	i32	getOrInitSolverBody(i32 bodyIndex, b3RigidBodyData* bodies,b3InertiaData* inertias);
	void initSolverBody(i32 bodyIndex, b3GpuSolverBody* solverBody, b3RigidBodyData* rb);

public:
	b3GpuPgsConstraintSolver(cl_context ctx, cl_device_id device, cl_command_queue queue, bool usePgs);
	virtual ~b3GpuPgsConstraintSolver();

	virtual b3Scalar solveGroupCacheFriendlyIterations(b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints1, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);
	virtual b3Scalar solveGroupCacheFriendlySetup(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias, i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);
	b3Scalar solveGroupCacheFriendlyFinish(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias, i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);

	b3Scalar solveGroup(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias, i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal);
	void solveJoints(i32 numBodies, b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias,
					 i32 numConstraints, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints);

	i32 sortConstraintByBatch3(struct b3BatchConstraint* cs, i32 numConstraints, i32 simdWidth, i32 staticIdx, i32 numBodies);
	void recomputeBatches();
};

#endif  //D3_GPU_PGS_CONSTRAINT_SOLVER_H
