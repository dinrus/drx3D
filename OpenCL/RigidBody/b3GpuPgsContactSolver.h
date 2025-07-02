
#ifndef D3_GPU_BATCHING_PGS_SOLVER_H
#define D3_GPU_BATCHING_PGS_SOLVER_H

#include <drx3D/OpenCL/Initialize/b3OpenCLInclude.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Contact4.h>
#include "b3GpuConstraint4.h"

class b3GpuPgsContactSolver
{
protected:
	i32 m_debugOutput;

	struct b3GpuBatchingPgsSolverInternalData* m_data;

	void batchContacts(b3OpenCLArray<b3Contact4>* contacts, i32 nContacts, b3OpenCLArray<u32>* n, b3OpenCLArray<u32>* offsets, i32 staticIdx);

	inline i32 sortConstraintByBatch(b3Contact4* cs, i32 n, i32 simdWidth, i32 staticIdx, i32 numBodies);
	inline i32 sortConstraintByBatch2(b3Contact4* cs, i32 n, i32 simdWidth, i32 staticIdx, i32 numBodies);
	inline i32 sortConstraintByBatch3(b3Contact4* cs, i32 n, i32 simdWidth, i32 staticIdx, i32 numBodies, i32* batchSizes);

	void solveContactConstraintBatchSizes(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* shapeBuf,
										  b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, i32 numIterations, const b3AlignedObjectArray<i32>* batchSizes);  //const b3OpenCLArray<i32>* gpuBatchSizes);

	void solveContactConstraint(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* shapeBuf,
								b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, i32 numIterations, const b3AlignedObjectArray<i32>* batchSizes);  //const b3OpenCLArray<i32>* gpuBatchSizes);

public:
	b3GpuPgsContactSolver(cl_context ctx, cl_device_id device, cl_command_queue q, i32 pairCapacity);
	virtual ~b3GpuPgsContactSolver();

	void solveContacts(i32 numBodies, cl_mem bodyBuf, cl_mem inertiaBuf, i32 numContacts, cl_mem contactBuf, const struct b3Config& config, i32 static0Index);
};

#endif  //D3_GPU_BATCHING_PGS_SOLVER_H
