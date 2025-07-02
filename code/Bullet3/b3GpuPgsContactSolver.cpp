
bool gUseLargeBatches = false;
bool gCpuBatchContacts = false;
bool gCpuSolveConstraint = false;
bool gCpuRadixSort = false;
bool gCpuSetSortData = false;
bool gCpuSortContactsDeterminism = false;
bool gUseCpuCopyConstraints = false;
bool gUseScanHost = false;
bool gReorderContactsOnCpu = false;

bool optionalSortContactsDeterminism = true;

#include <drx3D/OpenCL/RigidBody/b3GpuPgsContactSolver.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3RadixSort32CL.h>

#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3BoundSearchCL.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanCL.h>
#include <string.h>
#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>
#include <drx3D/Physics/Collision/NarrowPhase/b3Config.h>
#include<drx3D/OpenCL/RigidBody/b3Solver.h>

#define D3_SOLVER_SETUP_KERNEL_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/solverSetup.cl"
#define D3_SOLVER_SETUP2_KERNEL_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/solverSetup2.cl"
#define D3_SOLVER_CONTACT_KERNEL_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/solveContact.cl"
#define D3_SOLVER_FRICTION_KERNEL_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/solveFriction.cl"
#define D3_BATCHING_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/batchingKernels.cl"
#define D3_BATCHING_NEW_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/batchingKernelsNew.cl"

#include <drx3D/OpenCL/RigidBody/kernels/solverSetup.h>
#include <drx3D/OpenCL/RigidBody/kernels/solverSetup2.h>
#include <drx3D/OpenCL/RigidBody/kernels/solveContact.h>
#include <drx3D/OpenCL/RigidBody/kernels/solveFriction.h>
#include <drx3D/OpenCL/RigidBody/kernels/batchingKernels.h>
#include <drx3D/OpenCL/RigidBody/kernels/batchingKernelsNew.h>

struct b3GpuBatchingPgsSolverInternalData
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;
	i32 m_pairCapacity;
	i32 m_nIterations;

	b3OpenCLArray<b3GpuConstraint4>* m_contactCGPU;
	b3OpenCLArray<u32>* m_numConstraints;
	b3OpenCLArray<u32>* m_offsets;

	b3Solver* m_solverGPU;

	cl_kernel m_batchingKernel;
	cl_kernel m_batchingKernelNew;
	cl_kernel m_solveContactKernel;
	cl_kernel m_solveSingleContactKernel;
	cl_kernel m_solveSingleFrictionKernel;
	cl_kernel m_solveFrictionKernel;
	cl_kernel m_contactToConstraintKernel;
	cl_kernel m_setSortDataKernel;
	cl_kernel m_reorderContactKernel;
	cl_kernel m_copyConstraintKernel;

	cl_kernel m_setDeterminismSortDataBodyAKernel;
	cl_kernel m_setDeterminismSortDataBodyBKernel;
	cl_kernel m_setDeterminismSortDataChildShapeAKernel;
	cl_kernel m_setDeterminismSortDataChildShapeBKernel;

	class b3RadixSort32CL* m_sort32;
	class b3BoundSearchCL* m_search;
	class b3PrefixScanCL* m_scan;

	b3OpenCLArray<b3SortData>* m_sortDataBuffer;
	b3OpenCLArray<b3Contact4>* m_contactBuffer;

	b3OpenCLArray<b3RigidBodyData>* m_bodyBufferGPU;
	b3OpenCLArray<b3InertiaData>* m_inertiaBufferGPU;
	b3OpenCLArray<b3Contact4>* m_pBufContactOutGPU;

	b3OpenCLArray<b3Contact4>* m_pBufContactOutGPUCopy;
	b3OpenCLArray<b3SortData>* m_contactKeyValues;

	b3AlignedObjectArray<u32> m_idxBuffer;
	b3AlignedObjectArray<b3SortData> m_sortData;
	b3AlignedObjectArray<b3Contact4> m_old;

	b3AlignedObjectArray<i32> m_batchSizes;
	b3OpenCLArray<i32>* m_batchSizesGpu;
};

b3GpuPgsContactSolver::b3GpuPgsContactSolver(cl_context ctx, cl_device_id device, cl_command_queue q, i32 pairCapacity)
{
	m_debugOutput = 0;
	m_data = new b3GpuBatchingPgsSolverInternalData;
	m_data->m_context = ctx;
	m_data->m_device = device;
	m_data->m_queue = q;
	m_data->m_pairCapacity = pairCapacity;
	m_data->m_nIterations = 4;
	m_data->m_batchSizesGpu = new b3OpenCLArray<i32>(ctx, q);
	m_data->m_bodyBufferGPU = new b3OpenCLArray<b3RigidBodyData>(ctx, q);
	m_data->m_inertiaBufferGPU = new b3OpenCLArray<b3InertiaData>(ctx, q);
	m_data->m_pBufContactOutGPU = new b3OpenCLArray<b3Contact4>(ctx, q);

	m_data->m_pBufContactOutGPUCopy = new b3OpenCLArray<b3Contact4>(ctx, q);
	m_data->m_contactKeyValues = new b3OpenCLArray<b3SortData>(ctx, q);

	m_data->m_solverGPU = new b3Solver(ctx, device, q, 512 * 1024);

	m_data->m_sort32 = new b3RadixSort32CL(ctx, device, m_data->m_queue);
	m_data->m_scan = new b3PrefixScanCL(ctx, device, m_data->m_queue, D3_SOLVER_N_CELLS);
	m_data->m_search = new b3BoundSearchCL(ctx, device, m_data->m_queue, D3_SOLVER_N_CELLS);

	i32k sortSize = B3NEXTMULTIPLEOF(pairCapacity, 512);

	m_data->m_sortDataBuffer = new b3OpenCLArray<b3SortData>(ctx, m_data->m_queue, sortSize);
	m_data->m_contactBuffer = new b3OpenCLArray<b3Contact4>(ctx, m_data->m_queue);

	m_data->m_numConstraints = new b3OpenCLArray<u32>(ctx, m_data->m_queue, D3_SOLVER_N_CELLS);
	m_data->m_numConstraints->resize(D3_SOLVER_N_CELLS);

	m_data->m_contactCGPU = new b3OpenCLArray<b3GpuConstraint4>(ctx, q, pairCapacity);

	m_data->m_offsets = new b3OpenCLArray<u32>(ctx, m_data->m_queue, D3_SOLVER_N_CELLS);
	m_data->m_offsets->resize(D3_SOLVER_N_CELLS);
	tukk additionalMacros = "";
	//tukk srcFileNameForCaching="";

	cl_int pErrNum;
	tukk batchKernelSource = batchingKernelsCL;
	tukk batchKernelNewSource = batchingKernelsNewCL;
	tukk solverSetupSource = solverSetupCL;
	tukk solverSetup2Source = solverSetup2CL;
	tukk solveContactSource = solveContactCL;
	tukk solveFrictionSource = solveFrictionCL;

	{
		cl_program solveContactProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, solveContactSource, &pErrNum, additionalMacros, D3_SOLVER_CONTACT_KERNEL_PATH);
		drx3DAssert(solveContactProg);

		cl_program solveFrictionProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, solveFrictionSource, &pErrNum, additionalMacros, D3_SOLVER_FRICTION_KERNEL_PATH);
		drx3DAssert(solveFrictionProg);

		cl_program solverSetup2Prog = b3OpenCLUtils::compileCLProgramFromString(ctx, device, solverSetup2Source, &pErrNum, additionalMacros, D3_SOLVER_SETUP2_KERNEL_PATH);

		drx3DAssert(solverSetup2Prog);

		cl_program solverSetupProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, solverSetupSource, &pErrNum, additionalMacros, D3_SOLVER_SETUP_KERNEL_PATH);
		drx3DAssert(solverSetupProg);

		m_data->m_solveFrictionKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveFrictionSource, "BatchSolveKernelFriction", &pErrNum, solveFrictionProg, additionalMacros);
		drx3DAssert(m_data->m_solveFrictionKernel);

		m_data->m_solveContactKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveContactSource, "BatchSolveKernelContact", &pErrNum, solveContactProg, additionalMacros);
		drx3DAssert(m_data->m_solveContactKernel);

		m_data->m_solveSingleContactKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveContactSource, "solveSingleContactKernel", &pErrNum, solveContactProg, additionalMacros);
		drx3DAssert(m_data->m_solveSingleContactKernel);

		m_data->m_solveSingleFrictionKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveFrictionSource, "solveSingleFrictionKernel", &pErrNum, solveFrictionProg, additionalMacros);
		drx3DAssert(m_data->m_solveSingleFrictionKernel);

		m_data->m_contactToConstraintKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetupSource, "ContactToConstraintKernel", &pErrNum, solverSetupProg, additionalMacros);
		drx3DAssert(m_data->m_contactToConstraintKernel);

		m_data->m_setSortDataKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetSortDataKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_setSortDataKernel);

		m_data->m_setDeterminismSortDataBodyAKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetDeterminismSortDataBodyA", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_setDeterminismSortDataBodyAKernel);

		m_data->m_setDeterminismSortDataBodyBKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetDeterminismSortDataBodyB", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_setDeterminismSortDataBodyBKernel);

		m_data->m_setDeterminismSortDataChildShapeAKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetDeterminismSortDataChildShapeA", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_setDeterminismSortDataChildShapeAKernel);

		m_data->m_setDeterminismSortDataChildShapeBKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetDeterminismSortDataChildShapeB", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_setDeterminismSortDataChildShapeBKernel);

		m_data->m_reorderContactKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "ReorderContactKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_reorderContactKernel);

		m_data->m_copyConstraintKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "CopyConstraintKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_data->m_copyConstraintKernel);
	}

	{
		cl_program batchingProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, batchKernelSource, &pErrNum, additionalMacros, D3_BATCHING_PATH);
		drx3DAssert(batchingProg);

		m_data->m_batchingKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, batchKernelSource, "CreateBatches", &pErrNum, batchingProg, additionalMacros);
		drx3DAssert(m_data->m_batchingKernel);
	}

	{
		cl_program batchingNewProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, batchKernelNewSource, &pErrNum, additionalMacros, D3_BATCHING_NEW_PATH);
		drx3DAssert(batchingNewProg);

		m_data->m_batchingKernelNew = b3OpenCLUtils::compileCLKernelFromString(ctx, device, batchKernelNewSource, "CreateBatchesNew", &pErrNum, batchingNewProg, additionalMacros);
		drx3DAssert(m_data->m_batchingKernelNew);
	}
}

b3GpuPgsContactSolver::~b3GpuPgsContactSolver()
{
	delete m_data->m_batchSizesGpu;
	delete m_data->m_bodyBufferGPU;
	delete m_data->m_inertiaBufferGPU;
	delete m_data->m_pBufContactOutGPU;
	delete m_data->m_pBufContactOutGPUCopy;
	delete m_data->m_contactKeyValues;

	delete m_data->m_contactCGPU;
	delete m_data->m_numConstraints;
	delete m_data->m_offsets;
	delete m_data->m_sortDataBuffer;
	delete m_data->m_contactBuffer;

	delete m_data->m_sort32;
	delete m_data->m_scan;
	delete m_data->m_search;
	delete m_data->m_solverGPU;

	clReleaseKernel(m_data->m_batchingKernel);
	clReleaseKernel(m_data->m_batchingKernelNew);
	clReleaseKernel(m_data->m_solveSingleContactKernel);
	clReleaseKernel(m_data->m_solveSingleFrictionKernel);
	clReleaseKernel(m_data->m_solveContactKernel);
	clReleaseKernel(m_data->m_solveFrictionKernel);

	clReleaseKernel(m_data->m_contactToConstraintKernel);
	clReleaseKernel(m_data->m_setSortDataKernel);
	clReleaseKernel(m_data->m_reorderContactKernel);
	clReleaseKernel(m_data->m_copyConstraintKernel);

	clReleaseKernel(m_data->m_setDeterminismSortDataBodyAKernel);
	clReleaseKernel(m_data->m_setDeterminismSortDataBodyBKernel);
	clReleaseKernel(m_data->m_setDeterminismSortDataChildShapeAKernel);
	clReleaseKernel(m_data->m_setDeterminismSortDataChildShapeBKernel);

	delete m_data;
}

struct b3ConstraintCfg
{
	b3ConstraintCfg(float dt = 0.f) : m_positionDrift(0.005f), m_positionConstraintCoeff(0.2f), m_dt(dt), m_staticIdx(0) {}

	float m_positionDrift;
	float m_positionConstraintCoeff;
	float m_dt;
	bool m_enableParallelSolve;
	float m_batchCellSize;
	i32 m_staticIdx;
};

void b3GpuPgsContactSolver::solveContactConstraintBatchSizes(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* shapeBuf,
															 b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, i32 numIterations, const b3AlignedObjectArray<i32>* batchSizes)  //const b3OpenCLArray<i32>* gpuBatchSizes)
{
	D3_PROFILE("solveContactConstraintBatchSizes");
	i32 numBatches = batchSizes->size() / D3_MAX_NUM_BATCHES;
	for (i32 iter = 0; iter < numIterations; iter++)
	{
		for (i32 cellId = 0; cellId < numBatches; cellId++)
		{
			i32 offset = 0;
			for (i32 ii = 0; ii < D3_MAX_NUM_BATCHES; ii++)
			{
				i32 numInBatch = batchSizes->at(cellId * D3_MAX_NUM_BATCHES + ii);
				if (!numInBatch)
					break;

				{
					b3LauncherCL launcher(m_data->m_queue, m_data->m_solveSingleContactKernel, "m_solveSingleContactKernel");
					launcher.setBuffer(bodyBuf->getBufferCL());
					launcher.setBuffer(shapeBuf->getBufferCL());
					launcher.setBuffer(constraint->getBufferCL());
					launcher.setConst(cellId);
					launcher.setConst(offset);
					launcher.setConst(numInBatch);
					launcher.launch1D(numInBatch);
					offset += numInBatch;
				}
			}
		}
	}

	for (i32 iter = 0; iter < numIterations; iter++)
	{
		for (i32 cellId = 0; cellId < numBatches; cellId++)
		{
			i32 offset = 0;
			for (i32 ii = 0; ii < D3_MAX_NUM_BATCHES; ii++)
			{
				i32 numInBatch = batchSizes->at(cellId * D3_MAX_NUM_BATCHES + ii);
				if (!numInBatch)
					break;

				{
					b3LauncherCL launcher(m_data->m_queue, m_data->m_solveSingleFrictionKernel, "m_solveSingleFrictionKernel");
					launcher.setBuffer(bodyBuf->getBufferCL());
					launcher.setBuffer(shapeBuf->getBufferCL());
					launcher.setBuffer(constraint->getBufferCL());
					launcher.setConst(cellId);
					launcher.setConst(offset);
					launcher.setConst(numInBatch);
					launcher.launch1D(numInBatch);
					offset += numInBatch;
				}
			}
		}
	}
}

void b3GpuPgsContactSolver::solveContactConstraint(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* shapeBuf,
												   b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, i32 numIterations, const b3AlignedObjectArray<i32>* batchSizes)  //,const b3OpenCLArray<i32>* gpuBatchSizes)
{
	//sort the contacts

	b3Int4 cdata = b3MakeInt4(n, 0, 0, 0);
	{
		i32k nn = D3_SOLVER_N_CELLS;

		cdata.x = 0;
		cdata.y = maxNumBatches;  //250;

		i32 numWorkItems = 64 * nn / D3_SOLVER_N_BATCHES;
#ifdef DEBUG_ME
		SolverDebugInfo* debugInfo = new SolverDebugInfo[numWorkItems];
		adl::b3OpenCLArray<SolverDebugInfo> gpuDebugInfo(data->m_device, numWorkItems);
#endif

		{
			D3_PROFILE("m_batchSolveKernel iterations");
			for (i32 iter = 0; iter < numIterations; iter++)
			{
				for (i32 ib = 0; ib < D3_SOLVER_N_BATCHES; ib++)
				{
#ifdef DEBUG_ME
					memset(debugInfo, 0, sizeof(SolverDebugInfo) * numWorkItems);
					gpuDebugInfo.write(debugInfo, numWorkItems);
#endif

					cdata.z = ib;

					b3LauncherCL launcher(m_data->m_queue, m_data->m_solveContactKernel, "m_solveContactKernel");
#if 1

					b3BufferInfoCL bInfo[] = {

						b3BufferInfoCL(bodyBuf->getBufferCL()),
						b3BufferInfoCL(shapeBuf->getBufferCL()),
						b3BufferInfoCL(constraint->getBufferCL()),
						b3BufferInfoCL(m_data->m_solverGPU->m_numConstraints->getBufferCL()),
						b3BufferInfoCL(m_data->m_solverGPU->m_offsets->getBufferCL())
#ifdef DEBUG_ME
							,
						b3BufferInfoCL(&gpuDebugInfo)
#endif
					};

					launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
					launcher.setBuffer(m_data->m_solverGPU->m_batchSizes.getBufferCL());
					//launcher.setConst(  cdata.x );
					launcher.setConst(cdata.y);
					launcher.setConst(cdata.z);
					b3Int4 nSplit;
					nSplit.x = D3_SOLVER_N_SPLIT_X;
					nSplit.y = D3_SOLVER_N_SPLIT_Y;
					nSplit.z = D3_SOLVER_N_SPLIT_Z;

					launcher.setConst(nSplit);
					launcher.launch1D(numWorkItems, 64);

#else
					tukk fileName = "m_batchSolveKernel.bin";
					FILE* f = fopen(fileName, "rb");
					if (f)
					{
						i32 sizeInBytes = 0;
						if (fseek(f, 0, SEEK_END) || (sizeInBytes = ftell(f)) == EOF || fseek(f, 0, SEEK_SET))
						{
							printf("error, cannot get file size\n");
							exit(0);
						}

						u8* buf = (u8*)malloc(sizeInBytes);
						fread(buf, sizeInBytes, 1, f);
						i32 serializedBytes = launcher.deserializeArgs(buf, sizeInBytes, m_context);
						i32 num = *(i32*)&buf[serializedBytes];

						launcher.launch1D(num);

						//this clFinish is for testing on errors
						clFinish(m_queue);
					}

#endif

#ifdef DEBUG_ME
					clFinish(m_queue);
					gpuDebugInfo.read(debugInfo, numWorkItems);
					clFinish(m_queue);
					for (i32 i = 0; i < numWorkItems; i++)
					{
						if (debugInfo[i].m_valInt2 > 0)
						{
							printf("debugInfo[i].m_valInt2 = %d\n", i, debugInfo[i].m_valInt2);
						}

						if (debugInfo[i].m_valInt3 > 0)
						{
							printf("debugInfo[i].m_valInt3 = %d\n", i, debugInfo[i].m_valInt3);
						}
					}
#endif  //DEBUG_ME
				}
			}

			clFinish(m_data->m_queue);
		}

		cdata.x = 1;
		bool applyFriction = true;
		if (applyFriction)
		{
			D3_PROFILE("m_batchSolveKernel iterations2");
			for (i32 iter = 0; iter < numIterations; iter++)
			{
				for (i32 ib = 0; ib < D3_SOLVER_N_BATCHES; ib++)
				{
					cdata.z = ib;

					b3BufferInfoCL bInfo[] = {
						b3BufferInfoCL(bodyBuf->getBufferCL()),
						b3BufferInfoCL(shapeBuf->getBufferCL()),
						b3BufferInfoCL(constraint->getBufferCL()),
						b3BufferInfoCL(m_data->m_solverGPU->m_numConstraints->getBufferCL()),
						b3BufferInfoCL(m_data->m_solverGPU->m_offsets->getBufferCL())
#ifdef DEBUG_ME
							,
						b3BufferInfoCL(&gpuDebugInfo)
#endif  //DEBUG_ME
					};
					b3LauncherCL launcher(m_data->m_queue, m_data->m_solveFrictionKernel, "m_solveFrictionKernel");
					launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
					launcher.setBuffer(m_data->m_solverGPU->m_batchSizes.getBufferCL());
					//launcher.setConst(  cdata.x );
					launcher.setConst(cdata.y);
					launcher.setConst(cdata.z);

					b3Int4 nSplit;
					nSplit.x = D3_SOLVER_N_SPLIT_X;
					nSplit.y = D3_SOLVER_N_SPLIT_Y;
					nSplit.z = D3_SOLVER_N_SPLIT_Z;

					launcher.setConst(nSplit);

					launcher.launch1D(64 * nn / D3_SOLVER_N_BATCHES, 64);
				}
			}
			clFinish(m_data->m_queue);
		}
#ifdef DEBUG_ME
		delete[] debugInfo;
#endif  //DEBUG_ME
	}
}

static bool sortfnc(const b3SortData& a, const b3SortData& b)
{
	return (a.m_key < b.m_key);
}

static bool b3ContactCmp(const b3Contact4& p, const b3Contact4& q)
{
	return ((p.m_bodyAPtrAndSignBit < q.m_bodyAPtrAndSignBit) ||
			((p.m_bodyAPtrAndSignBit == q.m_bodyAPtrAndSignBit) && (p.m_bodyBPtrAndSignBit < q.m_bodyBPtrAndSignBit)) ||
			((p.m_bodyAPtrAndSignBit == q.m_bodyAPtrAndSignBit) && (p.m_bodyBPtrAndSignBit == q.m_bodyBPtrAndSignBit) && p.m_childIndexA < q.m_childIndexA) ||
			((p.m_bodyAPtrAndSignBit == q.m_bodyAPtrAndSignBit) && (p.m_bodyBPtrAndSignBit == q.m_bodyBPtrAndSignBit) && p.m_childIndexA < q.m_childIndexA) ||
			((p.m_bodyAPtrAndSignBit == q.m_bodyAPtrAndSignBit) && (p.m_bodyBPtrAndSignBit == q.m_bodyBPtrAndSignBit) && p.m_childIndexA == q.m_childIndexA && p.m_childIndexB < q.m_childIndexB));
}

#define USE_SPATIAL_BATCHING 1
#define USE_4x4_GRID 1

#ifndef USE_SPATIAL_BATCHING
static i32k gridTable4x4[] =
	{
		0, 1, 17, 16,
		1, 2, 18, 19,
		17, 18, 32, 3,
		16, 19, 3, 34};
static i32k gridTable8x8[] =
	{
		0, 2, 3, 16, 17, 18, 19, 1,
		66, 64, 80, 67, 82, 81, 65, 83,
		131, 144, 128, 130, 147, 129, 145, 146,
		208, 195, 194, 192, 193, 211, 210, 209,
		21, 22, 23, 5, 4, 6, 7, 20,
		86, 85, 69, 87, 70, 68, 84, 71,
		151, 133, 149, 150, 135, 148, 132, 134,
		197, 27, 214, 213, 212, 199, 198, 196

};

#endif

void SetSortDataCPU(b3Contact4* gContact, b3RigidBodyData* gBodies, b3SortData* gSortDataOut, i32 nContacts, float scale, const b3Int4& nSplit, i32 staticIdx)
{
	for (i32 gIdx = 0; gIdx < nContacts; gIdx++)
	{
		if (gIdx < nContacts)
		{
			i32 aPtrAndSignBit = gContact[gIdx].m_bodyAPtrAndSignBit;
			i32 bPtrAndSignBit = gContact[gIdx].m_bodyBPtrAndSignBit;

			i32 aIdx = abs(aPtrAndSignBit);
			i32 bIdx = abs(bPtrAndSignBit);

			bool aStatic = (aPtrAndSignBit < 0) || (aPtrAndSignBit == staticIdx);

#if USE_SPATIAL_BATCHING
			i32 idx = (aStatic) ? bIdx : aIdx;
			b3Vec3 p = gBodies[idx].m_pos;
			i32 xIdx = (i32)((p.x - ((p.x < 0.f) ? 1.f : 0.f)) * scale) & (nSplit.x - 1);
			i32 yIdx = (i32)((p.y - ((p.y < 0.f) ? 1.f : 0.f)) * scale) & (nSplit.y - 1);
			i32 zIdx = (i32)((p.z - ((p.z < 0.f) ? 1.f : 0.f)) * scale) & (nSplit.z - 1);

			i32 newIndex = (xIdx + yIdx * nSplit.x + zIdx * nSplit.x * nSplit.y);

#else  //USE_SPATIAL_BATCHING
			bool bStatic = (bPtrAndSignBit < 0) || (bPtrAndSignBit == staticIdx);

#if USE_4x4_GRID
			i32 aa = aIdx & 3;
			i32 bb = bIdx & 3;
			if (aStatic)
				aa = bb;
			if (bStatic)
				bb = aa;

			i32 gridIndex = aa + bb * 4;
			i32 newIndex = gridTable4x4[gridIndex];
#else   //USE_4x4_GRID
			i32 aa = aIdx & 7;
			i32 bb = bIdx & 7;
			if (aStatic)
				aa = bb;
			if (bStatic)
				bb = aa;

			i32 gridIndex = aa + bb * 8;
			i32 newIndex = gridTable8x8[gridIndex];
#endif  //USE_4x4_GRID
#endif  //USE_SPATIAL_BATCHING

			gSortDataOut[gIdx].x = newIndex;
			gSortDataOut[gIdx].y = gIdx;
		}
		else
		{
			gSortDataOut[gIdx].x = 0xffffffff;
		}
	}
}

void b3GpuPgsContactSolver::solveContacts(i32 numBodies, cl_mem bodyBuf, cl_mem inertiaBuf, i32 numContacts, cl_mem contactBuf, const b3Config& config, i32 static0Index)
{
	D3_PROFILE("solveContacts");
	m_data->m_bodyBufferGPU->setFromOpenCLBuffer(bodyBuf, numBodies);
	m_data->m_inertiaBufferGPU->setFromOpenCLBuffer(inertiaBuf, numBodies);
	m_data->m_pBufContactOutGPU->setFromOpenCLBuffer(contactBuf, numContacts);

	if (optionalSortContactsDeterminism)
	{
		if (!gCpuSortContactsDeterminism)
		{
			D3_PROFILE("GPU Sort contact constraints (determinism)");

			m_data->m_pBufContactOutGPUCopy->resize(numContacts);
			m_data->m_contactKeyValues->resize(numContacts);

			m_data->m_pBufContactOutGPU->copyToCL(m_data->m_pBufContactOutGPUCopy->getBufferCL(), numContacts, 0, 0);

			{
				b3LauncherCL launcher(m_data->m_queue, m_data->m_setDeterminismSortDataChildShapeBKernel, "m_setDeterminismSortDataChildShapeBKernel");
				launcher.setBuffer(m_data->m_pBufContactOutGPUCopy->getBufferCL());
				launcher.setBuffer(m_data->m_contactKeyValues->getBufferCL());
				launcher.setConst(numContacts);
				launcher.launch1D(numContacts, 64);
			}
			m_data->m_solverGPU->m_sort32->execute(*m_data->m_contactKeyValues);
			{
				b3LauncherCL launcher(m_data->m_queue, m_data->m_setDeterminismSortDataChildShapeAKernel, "m_setDeterminismSortDataChildShapeAKernel");
				launcher.setBuffer(m_data->m_pBufContactOutGPUCopy->getBufferCL());
				launcher.setBuffer(m_data->m_contactKeyValues->getBufferCL());
				launcher.setConst(numContacts);
				launcher.launch1D(numContacts, 64);
			}
			m_data->m_solverGPU->m_sort32->execute(*m_data->m_contactKeyValues);
			{
				b3LauncherCL launcher(m_data->m_queue, m_data->m_setDeterminismSortDataBodyBKernel, "m_setDeterminismSortDataBodyBKernel");
				launcher.setBuffer(m_data->m_pBufContactOutGPUCopy->getBufferCL());
				launcher.setBuffer(m_data->m_contactKeyValues->getBufferCL());
				launcher.setConst(numContacts);
				launcher.launch1D(numContacts, 64);
			}

			m_data->m_solverGPU->m_sort32->execute(*m_data->m_contactKeyValues);

			{
				b3LauncherCL launcher(m_data->m_queue, m_data->m_setDeterminismSortDataBodyAKernel, "m_setDeterminismSortDataBodyAKernel");
				launcher.setBuffer(m_data->m_pBufContactOutGPUCopy->getBufferCL());
				launcher.setBuffer(m_data->m_contactKeyValues->getBufferCL());
				launcher.setConst(numContacts);
				launcher.launch1D(numContacts, 64);
			}

			m_data->m_solverGPU->m_sort32->execute(*m_data->m_contactKeyValues);

			{
				D3_PROFILE("gpu reorderContactKernel (determinism)");

				b3Int4 cdata;
				cdata.x = numContacts;

				//b3BufferInfoCL bInfo[] = { b3BufferInfoCL( m_data->m_pBufContactOutGPU->getBufferCL() ), b3BufferInfoCL( m_data->m_solverGPU->m_contactBuffer2->getBufferCL())
				//	, b3BufferInfoCL( m_data->m_solverGPU->m_sortDataBuffer->getBufferCL()) };
				b3LauncherCL launcher(m_data->m_queue, m_data->m_solverGPU->m_reorderContactKernel, "m_reorderContactKernel");
				launcher.setBuffer(m_data->m_pBufContactOutGPUCopy->getBufferCL());
				launcher.setBuffer(m_data->m_pBufContactOutGPU->getBufferCL());
				launcher.setBuffer(m_data->m_contactKeyValues->getBufferCL());
				launcher.setConst(cdata);
				launcher.launch1D(numContacts, 64);
			}
		}
		else
		{
			D3_PROFILE("CPU Sort contact constraints (determinism)");
			b3AlignedObjectArray<b3Contact4> cpuConstraints;
			m_data->m_pBufContactOutGPU->copyToHost(cpuConstraints);
			bool sort = true;
			if (sort)
			{
				cpuConstraints.quickSort(b3ContactCmp);

				for (i32 i = 0; i < cpuConstraints.size(); i++)
				{
					cpuConstraints[i].m_batchIdx = i;
				}
			}
			m_data->m_pBufContactOutGPU->copyFromHost(cpuConstraints);
			if (m_debugOutput == 100)
			{
				for (i32 i = 0; i < cpuConstraints.size(); i++)
				{
					printf("c[%d].m_bodyA = %d, m_bodyB = %d, batchId = %d\n", i, cpuConstraints[i].m_bodyAPtrAndSignBit, cpuConstraints[i].m_bodyBPtrAndSignBit, cpuConstraints[i].m_batchIdx);
				}
			}

			m_debugOutput++;
		}
	}

	i32 nContactOut = m_data->m_pBufContactOutGPU->size();

	bool useSolver = true;

	if (useSolver)
	{
		float dt = 1. / 60.;
		b3ConstraintCfg csCfg(dt);
		csCfg.m_enableParallelSolve = true;
		csCfg.m_batchCellSize = 6;
		csCfg.m_staticIdx = static0Index;

		b3OpenCLArray<b3RigidBodyData>* bodyBuf = m_data->m_bodyBufferGPU;

		uk additionalData = 0;  //m_data->m_frictionCGPU;
		const b3OpenCLArray<b3InertiaData>* shapeBuf = m_data->m_inertiaBufferGPU;
		b3OpenCLArray<b3GpuConstraint4>* contactConstraintOut = m_data->m_contactCGPU;
		i32 nContacts = nContactOut;

		i32 maxNumBatches = 0;

		if (!gUseLargeBatches)
		{
			if (m_data->m_solverGPU->m_contactBuffer2)
			{
				m_data->m_solverGPU->m_contactBuffer2->resize(nContacts);
			}

			if (m_data->m_solverGPU->m_contactBuffer2 == 0)
			{
				m_data->m_solverGPU->m_contactBuffer2 = new b3OpenCLArray<b3Contact4>(m_data->m_context, m_data->m_queue, nContacts);
				m_data->m_solverGPU->m_contactBuffer2->resize(nContacts);
			}

			//clFinish(m_data->m_queue);

			{
				D3_PROFILE("batching");
				//@todo: just reserve it, without copy of original contact (unless we use warmstarting)

				//const b3OpenCLArray<b3RigidBodyData>* bodyNative = bodyBuf;

				{
					//b3OpenCLArray<b3RigidBodyData>* bodyNative = b3OpenCLArrayUtils::map<adl::TYPE_CL, true>( data->m_device, bodyBuf );
					//b3OpenCLArray<b3Contact4>* contactNative = b3OpenCLArrayUtils::map<adl::TYPE_CL, true>( data->m_device, contactsIn );

					i32k sortAlignment = 512;  // todo. get this out of sort
					if (csCfg.m_enableParallelSolve)
					{
						i32 sortSize = B3NEXTMULTIPLEOF(nContacts, sortAlignment);

						b3OpenCLArray<u32>* countsNative = m_data->m_solverGPU->m_numConstraints;
						b3OpenCLArray<u32>* offsetsNative = m_data->m_solverGPU->m_offsets;

						if (!gCpuSetSortData)
						{  //	2. set cell idx
							D3_PROFILE("GPU set cell idx");
							struct CB
							{
								i32 m_nContacts;
								i32 m_staticIdx;
								float m_scale;
								b3Int4 m_nSplit;
							};

							drx3DAssert(sortSize % 64 == 0);
							CB cdata;
							cdata.m_nContacts = nContacts;
							cdata.m_staticIdx = csCfg.m_staticIdx;
							cdata.m_scale = 1.f / csCfg.m_batchCellSize;
							cdata.m_nSplit.x = D3_SOLVER_N_SPLIT_X;
							cdata.m_nSplit.y = D3_SOLVER_N_SPLIT_Y;
							cdata.m_nSplit.z = D3_SOLVER_N_SPLIT_Z;

							m_data->m_solverGPU->m_sortDataBuffer->resize(nContacts);

							b3BufferInfoCL bInfo[] = {b3BufferInfoCL(m_data->m_pBufContactOutGPU->getBufferCL()), b3BufferInfoCL(bodyBuf->getBufferCL()), b3BufferInfoCL(m_data->m_solverGPU->m_sortDataBuffer->getBufferCL())};
							b3LauncherCL launcher(m_data->m_queue, m_data->m_solverGPU->m_setSortDataKernel, "m_setSortDataKernel");
							launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
							launcher.setConst(cdata.m_nContacts);
							launcher.setConst(cdata.m_scale);
							launcher.setConst(cdata.m_nSplit);
							launcher.setConst(cdata.m_staticIdx);

							launcher.launch1D(sortSize, 64);
						}
						else
						{
							m_data->m_solverGPU->m_sortDataBuffer->resize(nContacts);
							b3AlignedObjectArray<b3SortData> sortDataCPU;
							m_data->m_solverGPU->m_sortDataBuffer->copyToHost(sortDataCPU);

							b3AlignedObjectArray<b3Contact4> contactCPU;
							m_data->m_pBufContactOutGPU->copyToHost(contactCPU);
							b3AlignedObjectArray<b3RigidBodyData> bodiesCPU;
							bodyBuf->copyToHost(bodiesCPU);
							float scale = 1.f / csCfg.m_batchCellSize;
							b3Int4 nSplit;
							nSplit.x = D3_SOLVER_N_SPLIT_X;
							nSplit.y = D3_SOLVER_N_SPLIT_Y;
							nSplit.z = D3_SOLVER_N_SPLIT_Z;

							SetSortDataCPU(&contactCPU[0], &bodiesCPU[0], &sortDataCPU[0], nContacts, scale, nSplit, csCfg.m_staticIdx);

							m_data->m_solverGPU->m_sortDataBuffer->copyFromHost(sortDataCPU);
						}

						if (!gCpuRadixSort)
						{  //	3. sort by cell idx
							D3_PROFILE("gpuRadixSort");
							//i32 n = D3_SOLVER_N_SPLIT*D3_SOLVER_N_SPLIT;
							//i32 sortBit = 32;
							//if( n <= 0xffff ) sortBit = 16;
							//if( n <= 0xff ) sortBit = 8;
							//adl::RadixSort<adl::TYPE_CL>::execute( data->m_sort, *data->m_sortDataBuffer, sortSize );
							//adl::RadixSort32<adl::TYPE_CL>::execute( data->m_sort32, *data->m_sortDataBuffer, sortSize );
							b3OpenCLArray<b3SortData>& keyValuesInOut = *(m_data->m_solverGPU->m_sortDataBuffer);
							this->m_data->m_solverGPU->m_sort32->execute(keyValuesInOut);
						}
						else
						{
							b3OpenCLArray<b3SortData>& keyValuesInOut = *(m_data->m_solverGPU->m_sortDataBuffer);
							b3AlignedObjectArray<b3SortData> hostValues;
							keyValuesInOut.copyToHost(hostValues);
							hostValues.quickSort(sortfnc);
							keyValuesInOut.copyFromHost(hostValues);
						}

						if (gUseScanHost)
						{
							//	4. find entries
							D3_PROFILE("cpuBoundSearch");
							b3AlignedObjectArray<u32> countsHost;
							countsNative->copyToHost(countsHost);

							b3AlignedObjectArray<b3SortData> sortDataHost;
							m_data->m_solverGPU->m_sortDataBuffer->copyToHost(sortDataHost);

							//m_data->m_solverGPU->m_search->executeHost(*m_data->m_solverGPU->m_sortDataBuffer,nContacts,*countsNative,D3_SOLVER_N_CELLS,b3BoundSearchCL::COUNT);
							m_data->m_solverGPU->m_search->executeHost(sortDataHost, nContacts, countsHost, D3_SOLVER_N_CELLS, b3BoundSearchCL::COUNT);

							countsNative->copyFromHost(countsHost);

							//adl::BoundSearch<adl::TYPE_CL>::execute( data->m_search, *data->m_sortDataBuffer, nContacts, *countsNative,
							//	D3_SOLVER_N_SPLIT*D3_SOLVER_N_SPLIT, adl::BoundSearchBase::COUNT );

							//u32 sum;
							//m_data->m_solverGPU->m_scan->execute(*countsNative,*offsetsNative, D3_SOLVER_N_CELLS);//,&sum );
							b3AlignedObjectArray<u32> offsetsHost;
							offsetsHost.resize(offsetsNative->size());

							m_data->m_solverGPU->m_scan->executeHost(countsHost, offsetsHost, D3_SOLVER_N_CELLS);  //,&sum );
							offsetsNative->copyFromHost(offsetsHost);

							//printf("sum = %d\n",sum);
						}
						else
						{
							//	4. find entries
							D3_PROFILE("gpuBoundSearch");
							m_data->m_solverGPU->m_search->execute(*m_data->m_solverGPU->m_sortDataBuffer, nContacts, *countsNative, D3_SOLVER_N_CELLS, b3BoundSearchCL::COUNT);
							m_data->m_solverGPU->m_scan->execute(*countsNative, *offsetsNative, D3_SOLVER_N_CELLS);  //,&sum );
						}

						if (nContacts)
						{  //	5. sort constraints by cellIdx
							if (gReorderContactsOnCpu)
							{
								D3_PROFILE("cpu m_reorderContactKernel");
								b3AlignedObjectArray<b3SortData> sortDataHost;
								m_data->m_solverGPU->m_sortDataBuffer->copyToHost(sortDataHost);
								b3AlignedObjectArray<b3Contact4> inContacts;
								b3AlignedObjectArray<b3Contact4> outContacts;
								m_data->m_pBufContactOutGPU->copyToHost(inContacts);
								outContacts.resize(inContacts.size());
								for (i32 i = 0; i < nContacts; i++)
								{
									i32 srcIdx = sortDataHost[i].y;
									outContacts[i] = inContacts[srcIdx];
								}
								m_data->m_solverGPU->m_contactBuffer2->copyFromHost(outContacts);

								/*								"void ReorderContactKernel(__global struct b3Contact4Data* in, __global struct b3Contact4Data* out, __global int2* sortData, int4 cb )\n"
								"{\n"
								"	i32 nContacts = cb.x;\n"
								"	i32 gIdx = GET_GLOBAL_IDX;\n"
								"	if( gIdx < nContacts )\n"
								"	{\n"
								"		i32 srcIdx = sortData[gIdx].y;\n"
								"		out[gIdx] = in[srcIdx];\n"
								"	}\n"
								"}\n"
								*/
							}
							else
							{
								D3_PROFILE("gpu m_reorderContactKernel");

								b3Int4 cdata;
								cdata.x = nContacts;

								b3BufferInfoCL bInfo[] = {
									b3BufferInfoCL(m_data->m_pBufContactOutGPU->getBufferCL()),
									b3BufferInfoCL(m_data->m_solverGPU->m_contactBuffer2->getBufferCL()), b3BufferInfoCL(m_data->m_solverGPU->m_sortDataBuffer->getBufferCL())};

								b3LauncherCL launcher(m_data->m_queue, m_data->m_solverGPU->m_reorderContactKernel, "m_reorderContactKernel");
								launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
								launcher.setConst(cdata);
								launcher.launch1D(nContacts, 64);
							}
						}
					}
				}

				//clFinish(m_data->m_queue);

				//				{
				//				b3AlignedObjectArray<u32> histogram;
				//				m_data->m_solverGPU->m_numConstraints->copyToHost(histogram);
				//				printf(",,,\n");
				//				}

				if (nContacts)
				{
					if (gUseCpuCopyConstraints)
					{
						for (i32 i = 0; i < nContacts; i++)
						{
							m_data->m_pBufContactOutGPU->copyFromOpenCLArray(*m_data->m_solverGPU->m_contactBuffer2);
							//							m_data->m_solverGPU->m_contactBuffer2->getBufferCL();
							//						m_data->m_pBufContactOutGPU->getBufferCL()
						}
					}
					else
					{
						D3_PROFILE("gpu m_copyConstraintKernel");
						b3Int4 cdata;
						cdata.x = nContacts;
						b3BufferInfoCL bInfo[] = {
							b3BufferInfoCL(m_data->m_solverGPU->m_contactBuffer2->getBufferCL()),
							b3BufferInfoCL(m_data->m_pBufContactOutGPU->getBufferCL())};

						b3LauncherCL launcher(m_data->m_queue, m_data->m_solverGPU->m_copyConstraintKernel, "m_copyConstraintKernel");
						launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
						launcher.setConst(cdata);
						launcher.launch1D(nContacts, 64);
						//we use the clFinish for proper benchmark/profile
						clFinish(m_data->m_queue);
					}
				}

				//				bool compareGPU = false;
				if (nContacts)
				{
					if (!gCpuBatchContacts)
					{
						D3_PROFILE("gpu batchContacts");
						maxNumBatches = 250;  //250;
						m_data->m_solverGPU->batchContacts(m_data->m_pBufContactOutGPU, nContacts, m_data->m_solverGPU->m_numConstraints, m_data->m_solverGPU->m_offsets, csCfg.m_staticIdx);
						clFinish(m_data->m_queue);
					}
					else
					{
						D3_PROFILE("cpu batchContacts");
						static b3AlignedObjectArray<b3Contact4> cpuContacts;
						b3OpenCLArray<b3Contact4>* contactsIn = m_data->m_solverGPU->m_contactBuffer2;
						{
							D3_PROFILE("copyToHost");
							contactsIn->copyToHost(cpuContacts);
						}
						b3OpenCLArray<u32>* countsNative = m_data->m_solverGPU->m_numConstraints;
						b3OpenCLArray<u32>* offsetsNative = m_data->m_solverGPU->m_offsets;

						b3AlignedObjectArray<u32> nNativeHost;
						b3AlignedObjectArray<u32> offsetsNativeHost;

						{
							D3_PROFILE("countsNative/offsetsNative copyToHost");
							countsNative->copyToHost(nNativeHost);
							offsetsNative->copyToHost(offsetsNativeHost);
						}

						i32 numNonzeroGrid = 0;

						if (gUseLargeBatches)
						{
							m_data->m_batchSizes.resize(D3_MAX_NUM_BATCHES);
							i32 totalNumConstraints = cpuContacts.size();
							//i32 simdWidth =numBodies+1;//-1;//64;//-1;//32;
							i32 numBatches = sortConstraintByBatch3(&cpuContacts[0], totalNumConstraints, totalNumConstraints + 1, csCfg.m_staticIdx, numBodies, &m_data->m_batchSizes[0]);  //	on GPU
							maxNumBatches = d3Max(numBatches, maxNumBatches);
							static i32 globalMaxBatch = 0;
							if (maxNumBatches > globalMaxBatch)
							{
								globalMaxBatch = maxNumBatches;
								drx3DPrintf("maxNumBatches = %d\n", maxNumBatches);
							}
						}
						else
						{
							m_data->m_batchSizes.resize(D3_SOLVER_N_CELLS * D3_MAX_NUM_BATCHES);
							D3_PROFILE("cpu batch grid");
							for (i32 i = 0; i < D3_SOLVER_N_CELLS; i++)
							{
								i32 n = (nNativeHost)[i];
								i32 offset = (offsetsNativeHost)[i];
								if (n)
								{
									numNonzeroGrid++;
									i32 simdWidth = numBodies + 1;                                                                                                                                 //-1;//64;//-1;//32;
									i32 numBatches = sortConstraintByBatch3(&cpuContacts[0] + offset, n, simdWidth, csCfg.m_staticIdx, numBodies, &m_data->m_batchSizes[i * D3_MAX_NUM_BATCHES]);  //	on GPU
									maxNumBatches = d3Max(numBatches, maxNumBatches);
									static i32 globalMaxBatch = 0;
									if (maxNumBatches > globalMaxBatch)
									{
										globalMaxBatch = maxNumBatches;
										drx3DPrintf("maxNumBatches = %d\n", maxNumBatches);
									}
									//we use the clFinish for proper benchmark/profile
								}
							}
							//clFinish(m_data->m_queue);
						}
						{
							D3_PROFILE("m_contactBuffer->copyFromHost");
							m_data->m_solverGPU->m_contactBuffer2->copyFromHost((b3AlignedObjectArray<b3Contact4>&)cpuContacts);
						}
					}
				}
			}
		}

		//printf("maxNumBatches = %d\n", maxNumBatches);

		if (gUseLargeBatches)
		{
			if (nContacts)
			{
				D3_PROFILE("cpu batchContacts");
				static b3AlignedObjectArray<b3Contact4> cpuContacts;
				//				b3OpenCLArray<b3Contact4>* contactsIn = m_data->m_solverGPU->m_contactBuffer2;
				{
					D3_PROFILE("copyToHost");
					m_data->m_pBufContactOutGPU->copyToHost(cpuContacts);
				}
				//				b3OpenCLArray<u32>* countsNative = m_data->m_solverGPU->m_numConstraints;
				//				b3OpenCLArray<u32>* offsetsNative = m_data->m_solverGPU->m_offsets;

				//				i32 numNonzeroGrid=0;

				{
					m_data->m_batchSizes.resize(D3_MAX_NUM_BATCHES);
					i32 totalNumConstraints = cpuContacts.size();
					//				i32 simdWidth =numBodies+1;//-1;//64;//-1;//32;
					i32 numBatches = sortConstraintByBatch3(&cpuContacts[0], totalNumConstraints, totalNumConstraints + 1, csCfg.m_staticIdx, numBodies, &m_data->m_batchSizes[0]);  //	on GPU
					maxNumBatches = d3Max(numBatches, maxNumBatches);
					static i32 globalMaxBatch = 0;
					if (maxNumBatches > globalMaxBatch)
					{
						globalMaxBatch = maxNumBatches;
						drx3DPrintf("maxNumBatches = %d\n", maxNumBatches);
					}
				}
				{
					D3_PROFILE("m_contactBuffer->copyFromHost");
					m_data->m_solverGPU->m_contactBuffer2->copyFromHost((b3AlignedObjectArray<b3Contact4>&)cpuContacts);
				}
			}
		}

		if (nContacts)
		{
			D3_PROFILE("gpu convertToConstraints");
			m_data->m_solverGPU->convertToConstraints(bodyBuf,
													  shapeBuf, m_data->m_solverGPU->m_contactBuffer2,
													  contactConstraintOut,
													  additionalData, nContacts,
													  (b3SolverBase::ConstraintCfg&)csCfg);
			clFinish(m_data->m_queue);
		}

		if (1)
		{
			i32 numIter = 4;

			m_data->m_solverGPU->m_nIterations = numIter;  //10
			if (!gCpuSolveConstraint)
			{
				D3_PROFILE("GPU solveContactConstraint");

				/*m_data->m_solverGPU->solveContactConstraint(
				m_data->m_bodyBufferGPU, 
				m_data->m_inertiaBufferGPU,
				m_data->m_contactCGPU,0,
				nContactOut ,
				maxNumBatches);
				*/

				//m_data->m_batchSizesGpu->copyFromHost(m_data->m_batchSizes);

				if (gUseLargeBatches)
				{
					solveContactConstraintBatchSizes(m_data->m_bodyBufferGPU,
													 m_data->m_inertiaBufferGPU,
													 m_data->m_contactCGPU, 0,
													 nContactOut,
													 maxNumBatches, numIter, &m_data->m_batchSizes);
				}
				else
				{
					solveContactConstraint(
						m_data->m_bodyBufferGPU,
						m_data->m_inertiaBufferGPU,
						m_data->m_contactCGPU, 0,
						nContactOut,
						maxNumBatches, numIter, &m_data->m_batchSizes);  //m_data->m_batchSizesGpu);
				}
			}
			else
			{
				D3_PROFILE("Host solveContactConstraint");

				m_data->m_solverGPU->solveContactConstraintHost(m_data->m_bodyBufferGPU, m_data->m_inertiaBufferGPU, m_data->m_contactCGPU, 0, nContactOut, maxNumBatches, &m_data->m_batchSizes);
			}
		}

#if 0
        if (0)
        {
            D3_PROFILE("read body velocities back to CPU");
            //read body updated linear/angular velocities back to CPU
            m_data->m_bodyBufferGPU->read(
                                                  m_data->m_bodyBufferCPU->m_ptr,numOfConvexRBodies);
            adl::DeviceUtils::waitForCompletion( m_data->m_deviceCL );
        }
#endif
	}
}

void b3GpuPgsContactSolver::batchContacts(b3OpenCLArray<b3Contact4>* contacts, i32 nContacts, b3OpenCLArray<u32>* n, b3OpenCLArray<u32>* offsets, i32 staticIdx)
{
}

b3AlignedObjectArray<u32> idxBuffer;
b3AlignedObjectArray<b3SortData> sortData;
b3AlignedObjectArray<b3Contact4> old;

inline i32 b3GpuPgsContactSolver::sortConstraintByBatch(b3Contact4* cs, i32 n, i32 simdWidth, i32 staticIdx, i32 numBodies)
{
	D3_PROFILE("sortConstraintByBatch");
	i32 numIter = 0;

	sortData.resize(n);
	idxBuffer.resize(n);
	old.resize(n);

	u32* idxSrc = &idxBuffer[0];
	u32* idxDst = &idxBuffer[0];
	i32 nIdxSrc, nIdxDst;

	i32k N_FLG = 256;
	i32k FLG_MASK = N_FLG - 1;
	u32 flg[N_FLG / 32];
#if defined(_DEBUG)
	for (i32 i = 0; i < n; i++)
		cs[i].getBatchIdx() = -1;
#endif
	for (i32 i = 0; i < n; i++)
		idxSrc[i] = i;
	nIdxSrc = n;

	i32 batchIdx = 0;

	{
		D3_PROFILE("cpu batch innerloop");
		while (nIdxSrc)
		{
			numIter++;
			nIdxDst = 0;
			i32 nCurrentBatch = 0;

			//	clear flag
			for (i32 i = 0; i < N_FLG / 32; i++) flg[i] = 0;

			for (i32 i = 0; i < nIdxSrc; i++)
			{
				i32 idx = idxSrc[i];

				drx3DAssert(idx < n);
				//	check if it can go
				i32 bodyAS = cs[idx].m_bodyAPtrAndSignBit;
				i32 bodyBS = cs[idx].m_bodyBPtrAndSignBit;

				i32 bodyA = abs(bodyAS);
				i32 bodyB = abs(bodyBS);

				i32 aIdx = bodyA & FLG_MASK;
				i32 bIdx = bodyB & FLG_MASK;

				u32 aUnavailable = flg[aIdx / 32] & (1 << (aIdx & 31));
				u32 bUnavailable = flg[bIdx / 32] & (1 << (bIdx & 31));

				bool aIsStatic = (bodyAS < 0) || bodyAS == staticIdx;
				bool bIsStatic = (bodyBS < 0) || bodyBS == staticIdx;

				//use inv_mass!
				aUnavailable = !aIsStatic ? aUnavailable : 0;  //
				bUnavailable = !bIsStatic ? bUnavailable : 0;

				if (aUnavailable == 0 && bUnavailable == 0)  // ok
				{
					if (!aIsStatic)
						flg[aIdx / 32] |= (1 << (aIdx & 31));
					if (!bIsStatic)
						flg[bIdx / 32] |= (1 << (bIdx & 31));

					cs[idx].getBatchIdx() = batchIdx;
					sortData[idx].m_key = batchIdx;
					sortData[idx].m_value = idx;

					{
						nCurrentBatch++;
						if (nCurrentBatch == simdWidth)
						{
							nCurrentBatch = 0;
							for (i32 i = 0; i < N_FLG / 32; i++) flg[i] = 0;
						}
					}
				}
				else
				{
					idxDst[nIdxDst++] = idx;
				}
			}
			b3Swap(idxSrc, idxDst);
			b3Swap(nIdxSrc, nIdxDst);
			batchIdx++;
		}
	}
	{
		D3_PROFILE("quickSort");
		sortData.quickSort(sortfnc);
	}

	{
		D3_PROFILE("reorder");
		//	reorder

		memcpy(&old[0], cs, sizeof(b3Contact4) * n);
		for (i32 i = 0; i < n; i++)
		{
			i32 idx = sortData[i].m_value;
			cs[i] = old[idx];
		}
	}

#if defined(_DEBUG)
	//		debugPrintf( "nBatches: %d\n", batchIdx );
	for (i32 i = 0; i < n; i++)
	{
		drx3DAssert(cs[i].getBatchIdx() != -1);
	}
#endif
	return batchIdx;
}

b3AlignedObjectArray<i32> bodyUsed2;

inline i32 b3GpuPgsContactSolver::sortConstraintByBatch2(b3Contact4* cs, i32 numConstraints, i32 simdWidth, i32 staticIdx, i32 numBodies)
{
	D3_PROFILE("sortConstraintByBatch2");

	bodyUsed2.resize(2 * simdWidth);

	for (i32 q = 0; q < 2 * simdWidth; q++)
		bodyUsed2[q] = 0;

	i32 curBodyUsed = 0;

	i32 numIter = 0;

	m_data->m_sortData.resize(numConstraints);
	m_data->m_idxBuffer.resize(numConstraints);
	m_data->m_old.resize(numConstraints);

	u32* idxSrc = &m_data->m_idxBuffer[0];

#if defined(_DEBUG)
	for (i32 i = 0; i < numConstraints; i++)
		cs[i].getBatchIdx() = -1;
#endif
	for (i32 i = 0; i < numConstraints; i++)
		idxSrc[i] = i;

	i32 numValidConstraints = 0;
	//	i32 unprocessedConstraintIndex = 0;

	i32 batchIdx = 0;

	{
		D3_PROFILE("cpu batch innerloop");

		while (numValidConstraints < numConstraints)
		{
			numIter++;
			i32 nCurrentBatch = 0;
			//	clear flag
			for (i32 i = 0; i < curBodyUsed; i++)
				bodyUsed2[i] = 0;
			curBodyUsed = 0;

			for (i32 i = numValidConstraints; i < numConstraints; i++)
			{
				i32 idx = idxSrc[i];
				drx3DAssert(idx < numConstraints);
				//	check if it can go
				i32 bodyAS = cs[idx].m_bodyAPtrAndSignBit;
				i32 bodyBS = cs[idx].m_bodyBPtrAndSignBit;
				i32 bodyA = abs(bodyAS);
				i32 bodyB = abs(bodyBS);
				bool aIsStatic = (bodyAS < 0) || bodyAS == staticIdx;
				bool bIsStatic = (bodyBS < 0) || bodyBS == staticIdx;
				i32 aUnavailable = 0;
				i32 bUnavailable = 0;
				if (!aIsStatic)
				{
					for (i32 j = 0; j < curBodyUsed; j++)
					{
						if (bodyA == bodyUsed2[j])
						{
							aUnavailable = 1;
							break;
						}
					}
				}
				if (!aUnavailable)
					if (!bIsStatic)
					{
						for (i32 j = 0; j < curBodyUsed; j++)
						{
							if (bodyB == bodyUsed2[j])
							{
								bUnavailable = 1;
								break;
							}
						}
					}

				if (aUnavailable == 0 && bUnavailable == 0)  // ok
				{
					if (!aIsStatic)
					{
						bodyUsed2[curBodyUsed++] = bodyA;
					}
					if (!bIsStatic)
					{
						bodyUsed2[curBodyUsed++] = bodyB;
					}

					cs[idx].getBatchIdx() = batchIdx;
					m_data->m_sortData[idx].m_key = batchIdx;
					m_data->m_sortData[idx].m_value = idx;

					if (i != numValidConstraints)
					{
						b3Swap(idxSrc[i], idxSrc[numValidConstraints]);
					}

					numValidConstraints++;
					{
						nCurrentBatch++;
						if (nCurrentBatch == simdWidth)
						{
							nCurrentBatch = 0;
							for (i32 i = 0; i < curBodyUsed; i++)
								bodyUsed2[i] = 0;

							curBodyUsed = 0;
						}
					}
				}
			}

			batchIdx++;
		}
	}
	{
		D3_PROFILE("quickSort");
		//m_data->m_sortData.quickSort(sortfnc);
	}

	{
		D3_PROFILE("reorder");
		//	reorder

		memcpy(&m_data->m_old[0], cs, sizeof(b3Contact4) * numConstraints);

		for (i32 i = 0; i < numConstraints; i++)
		{
			drx3DAssert(m_data->m_sortData[idxSrc[i]].m_value == idxSrc[i]);
			i32 idx = m_data->m_sortData[idxSrc[i]].m_value;
			cs[i] = m_data->m_old[idx];
		}
	}

#if defined(_DEBUG)
	//		debugPrintf( "nBatches: %d\n", batchIdx );
	for (i32 i = 0; i < numConstraints; i++)
	{
		drx3DAssert(cs[i].getBatchIdx() != -1);
	}
#endif

	return batchIdx;
}

b3AlignedObjectArray<i32> bodyUsed;
b3AlignedObjectArray<i32> curUsed;

inline i32 b3GpuPgsContactSolver::sortConstraintByBatch3(b3Contact4* cs, i32 numConstraints, i32 simdWidth, i32 staticIdx, i32 numBodies, i32* batchSizes)
{
	D3_PROFILE("sortConstraintByBatch3");

	static i32 maxSwaps = 0;
	i32 numSwaps = 0;

	curUsed.resize(2 * simdWidth);

	static i32 maxNumConstraints = 0;
	if (maxNumConstraints < numConstraints)
	{
		maxNumConstraints = numConstraints;
		//printf("maxNumConstraints  = %d\n",maxNumConstraints );
	}

	i32 numUsedArray = numBodies / 32 + 1;
	bodyUsed.resize(numUsedArray);

	for (i32 q = 0; q < numUsedArray; q++)
		bodyUsed[q] = 0;

	i32 curBodyUsed = 0;

	i32 numIter = 0;

	m_data->m_sortData.resize(0);
	m_data->m_idxBuffer.resize(0);
	m_data->m_old.resize(0);

#if defined(_DEBUG)
	for (i32 i = 0; i < numConstraints; i++)
		cs[i].getBatchIdx() = -1;
#endif

	i32 numValidConstraints = 0;
	//	i32 unprocessedConstraintIndex = 0;

	i32 batchIdx = 0;

	{
		D3_PROFILE("cpu batch innerloop");

		while (numValidConstraints < numConstraints)
		{
			numIter++;
			i32 nCurrentBatch = 0;
			batchSizes[batchIdx] = 0;

			//	clear flag
			for (i32 i = 0; i < curBodyUsed; i++)
				bodyUsed[curUsed[i] / 32] = 0;

			curBodyUsed = 0;

			for (i32 i = numValidConstraints; i < numConstraints; i++)
			{
				i32 idx = i;
				drx3DAssert(idx < numConstraints);
				//	check if it can go
				i32 bodyAS = cs[idx].m_bodyAPtrAndSignBit;
				i32 bodyBS = cs[idx].m_bodyBPtrAndSignBit;
				i32 bodyA = abs(bodyAS);
				i32 bodyB = abs(bodyBS);
				bool aIsStatic = (bodyAS < 0) || bodyAS == staticIdx;
				bool bIsStatic = (bodyBS < 0) || bodyBS == staticIdx;
				i32 aUnavailable = 0;
				i32 bUnavailable = 0;
				if (!aIsStatic)
				{
					aUnavailable = bodyUsed[bodyA / 32] & (1 << (bodyA & 31));
				}
				if (!aUnavailable)
					if (!bIsStatic)
					{
						bUnavailable = bodyUsed[bodyB / 32] & (1 << (bodyB & 31));
					}

				if (aUnavailable == 0 && bUnavailable == 0)  // ok
				{
					if (!aIsStatic)
					{
						bodyUsed[bodyA / 32] |= (1 << (bodyA & 31));
						curUsed[curBodyUsed++] = bodyA;
					}
					if (!bIsStatic)
					{
						bodyUsed[bodyB / 32] |= (1 << (bodyB & 31));
						curUsed[curBodyUsed++] = bodyB;
					}

					cs[idx].getBatchIdx() = batchIdx;

					if (i != numValidConstraints)
					{
						b3Swap(cs[i], cs[numValidConstraints]);
						numSwaps++;
					}

					numValidConstraints++;
					{
						nCurrentBatch++;
						if (nCurrentBatch == simdWidth)
						{
							batchSizes[batchIdx] += simdWidth;
							nCurrentBatch = 0;
							for (i32 i = 0; i < curBodyUsed; i++)
								bodyUsed[curUsed[i] / 32] = 0;
							curBodyUsed = 0;
						}
					}
				}
			}

			if (batchIdx >= D3_MAX_NUM_BATCHES)
			{
				drx3DError("batchIdx>=D3_MAX_NUM_BATCHES");
				drx3DAssert(0);
				break;
			}

			batchSizes[batchIdx] += nCurrentBatch;

			batchIdx++;
		}
	}

#if defined(_DEBUG)
	//		debugPrintf( "nBatches: %d\n", batchIdx );
	for (i32 i = 0; i < numConstraints; i++)
	{
		drx3DAssert(cs[i].getBatchIdx() != -1);
	}
#endif

	batchSizes[batchIdx] = 0;

	if (maxSwaps < numSwaps)
	{
		maxSwaps = numSwaps;
		//printf("maxSwaps = %d\n", maxSwaps);
	}

	return batchIdx;
}
