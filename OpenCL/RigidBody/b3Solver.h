#ifndef __ADL_SOLVER_H
#define __ADL_SOLVER_H

#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/RigidBody/b3GpuConstraint4.h>

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Contact4.h>

#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3BoundSearchCL.h>

#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>

#define B3NEXTMULTIPLEOF(num, alignment) (((num) / (alignment) + (((num) % (alignment) == 0) ? 0 : 1)) * (alignment))

enum
{
	D3_SOLVER_N_SPLIT_X = 8,  //16,//4,
	D3_SOLVER_N_SPLIT_Y = 4,  //16,//4,
	D3_SOLVER_N_SPLIT_Z = 8,  //,
	D3_SOLVER_N_CELLS = D3_SOLVER_N_SPLIT_X * D3_SOLVER_N_SPLIT_Y * D3_SOLVER_N_SPLIT_Z,
	D3_SOLVER_N_BATCHES = 8,  //4,//8,//4,
	D3_MAX_NUM_BATCHES = 128,
};

class b3SolverBase
{
public:
	struct ConstraintCfg
	{
		ConstraintCfg(float dt = 0.f) : m_positionDrift(0.005f), m_positionConstraintCoeff(0.2f), m_dt(dt), m_staticIdx(-1) {}

		float m_positionDrift;
		float m_positionConstraintCoeff;
		float m_dt;
		bool m_enableParallelSolve;
		float m_batchCellSize;
		i32 m_staticIdx;
	};
};

class b3Solver : public b3SolverBase
{
public:
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

	b3OpenCLArray<u32>* m_numConstraints;
	b3OpenCLArray<u32>* m_offsets;
	b3OpenCLArray<i32> m_batchSizes;

	i32 m_nIterations;
	cl_kernel m_batchingKernel;
	cl_kernel m_batchingKernelNew;
	cl_kernel m_solveContactKernel;
	cl_kernel m_solveFrictionKernel;
	cl_kernel m_contactToConstraintKernel;
	cl_kernel m_setSortDataKernel;
	cl_kernel m_reorderContactKernel;
	cl_kernel m_copyConstraintKernel;

	class b3RadixSort32CL* m_sort32;
	class b3BoundSearchCL* m_search;
	class b3PrefixScanCL* m_scan;

	b3OpenCLArray<b3SortData>* m_sortDataBuffer;
	b3OpenCLArray<b3Contact4>* m_contactBuffer2;

	enum
	{
		DYNAMIC_CONTACT_ALLOCATION_THRESHOLD = 2000000,
	};

	b3Solver(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 pairCapacity);

	virtual ~b3Solver();

	void solveContactConstraint(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* inertiaBuf,
								b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches);

	void solveContactConstraintHost(b3OpenCLArray<b3RigidBodyData>* bodyBuf, b3OpenCLArray<b3InertiaData>* shapeBuf,
									b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, b3AlignedObjectArray<i32>* batchSizes);

	void convertToConstraints(const b3OpenCLArray<b3RigidBodyData>* bodyBuf,
							  const b3OpenCLArray<b3InertiaData>* shapeBuf,
							  b3OpenCLArray<b3Contact4>* contactsIn, b3OpenCLArray<b3GpuConstraint4>* contactCOut, uk additionalData,
							  i32 nContacts, const ConstraintCfg& cfg);

	void batchContacts(b3OpenCLArray<b3Contact4>* contacts, i32 nContacts, b3OpenCLArray<u32>* n, b3OpenCLArray<u32>* offsets, i32 staticIdx);
};

#endif  //__ADL_SOLVER_H
