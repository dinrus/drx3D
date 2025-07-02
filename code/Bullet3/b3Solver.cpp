#include <drx3D/OpenCL/RigidBody/b3Solver.h>

///useNewBatchingKernel  is a rewritten kernel using just a single thread of the warp, for experiments
bool useNewBatchingKernel = true;
bool gConvertConstraintOnCpu = false;

#define D3_SOLVER_SETUP_KERNEL_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/solverSetup.cl"
#define D3_SOLVER_SETUP2_KERNEL_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/solverSetup2.cl"
#define D3_SOLVER_CONTACT_KERNEL_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/solveContact.cl"
#define D3_SOLVER_FRICTION_KERNEL_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/solveFriction.cl"
#define D3_BATCHING_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/batchingKernels.cl"
#define D3_BATCHING_NEW_PATH "drx3D/Bullet3/OpenCL//RigidBody/kernels/batchingKernelsNew.cl"

#include <drx3D/Physics/Dynamics/shared/b3ConvertConstraint4.h>

#include <drx3D/OpenCL/RigidBody/kernels/solverSetup.h>
#include <drx3D/OpenCL/RigidBody/kernels/solverSetup2.h>

#include <drx3D/OpenCL/RigidBody/kernels/solveContact.h>
#include <drx3D/OpenCL/RigidBody/kernels/solveFriction.h>

#include <drx3D/OpenCL/RigidBody/kernels/batchingKernels.h>
#include <drx3D/OpenCL/RigidBody/kernels/batchingKernelsNew.h>

#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>
#include <drx3D/Common/b3Vec3.h>

struct SolverDebugInfo
{
	i32 m_valInt0;
	i32 m_valInt1;
	i32 m_valInt2;
	i32 m_valInt3;

	i32 m_valInt4;
	i32 m_valInt5;
	i32 m_valInt6;
	i32 m_valInt7;

	i32 m_valInt8;
	i32 m_valInt9;
	i32 m_valInt10;
	i32 m_valInt11;

	i32 m_valInt12;
	i32 m_valInt13;
	i32 m_valInt14;
	i32 m_valInt15;

	float m_val0;
	float m_val1;
	float m_val2;
	float m_val3;
};

class SolverDeviceInl
{
public:
	struct ParallelSolveData
	{
		b3OpenCLArray<u32>* m_numConstraints;
		b3OpenCLArray<u32>* m_offsets;
	};
};

b3Solver::b3Solver(cl_context ctx, cl_device_id device, cl_command_queue queue, i32 pairCapacity)
	: m_context(ctx),
	  m_device(device),
	  m_queue(queue),
	  m_batchSizes(ctx, queue),
	  m_nIterations(4)
{
	m_sort32 = new b3RadixSort32CL(ctx, device, queue);
	m_scan = new b3PrefixScanCL(ctx, device, queue, D3_SOLVER_N_CELLS);
	m_search = new b3BoundSearchCL(ctx, device, queue, D3_SOLVER_N_CELLS);

	i32k sortSize = B3NEXTMULTIPLEOF(pairCapacity, 512);

	m_sortDataBuffer = new b3OpenCLArray<b3SortData>(ctx, queue, sortSize);
	m_contactBuffer2 = new b3OpenCLArray<b3Contact4>(ctx, queue);

	m_numConstraints = new b3OpenCLArray<u32>(ctx, queue, D3_SOLVER_N_CELLS);
	m_numConstraints->resize(D3_SOLVER_N_CELLS);

	m_offsets = new b3OpenCLArray<u32>(ctx, queue, D3_SOLVER_N_CELLS);
	m_offsets->resize(D3_SOLVER_N_CELLS);
	tukk additionalMacros = "";
	//	tukk srcFileNameForCaching="";

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

		m_solveFrictionKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveFrictionSource, "BatchSolveKernelFriction", &pErrNum, solveFrictionProg, additionalMacros);
		drx3DAssert(m_solveFrictionKernel);

		m_solveContactKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solveContactSource, "BatchSolveKernelContact", &pErrNum, solveContactProg, additionalMacros);
		drx3DAssert(m_solveContactKernel);

		m_contactToConstraintKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetupSource, "ContactToConstraintKernel", &pErrNum, solverSetupProg, additionalMacros);
		drx3DAssert(m_contactToConstraintKernel);

		m_setSortDataKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "SetSortDataKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_setSortDataKernel);

		m_reorderContactKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "ReorderContactKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_reorderContactKernel);

		m_copyConstraintKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, solverSetup2Source, "CopyConstraintKernel", &pErrNum, solverSetup2Prog, additionalMacros);
		drx3DAssert(m_copyConstraintKernel);
	}

	{
		cl_program batchingProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, batchKernelSource, &pErrNum, additionalMacros, D3_BATCHING_PATH);
		//cl_program batchingProg = b3OpenCLUtils::compileCLProgramFromString( ctx, device, 0, &pErrNum,additionalMacros, D3_BATCHING_PATH,true);
		drx3DAssert(batchingProg);

		m_batchingKernel = b3OpenCLUtils::compileCLKernelFromString(ctx, device, batchKernelSource, "CreateBatches", &pErrNum, batchingProg, additionalMacros);
		drx3DAssert(m_batchingKernel);
	}
	{
		cl_program batchingNewProg = b3OpenCLUtils::compileCLProgramFromString(ctx, device, batchKernelNewSource, &pErrNum, additionalMacros, D3_BATCHING_NEW_PATH);
		drx3DAssert(batchingNewProg);

		m_batchingKernelNew = b3OpenCLUtils::compileCLKernelFromString(ctx, device, batchKernelNewSource, "CreateBatchesNew", &pErrNum, batchingNewProg, additionalMacros);
		//m_batchingKernelNew = b3OpenCLUtils::compileCLKernelFromString( ctx, device, batchKernelNewSource, "CreateBatchesBruteForce", &pErrNum, batchingNewProg,additionalMacros );
		drx3DAssert(m_batchingKernelNew);
	}
}

b3Solver::~b3Solver()
{
	delete m_offsets;
	delete m_numConstraints;
	delete m_sortDataBuffer;
	delete m_contactBuffer2;

	delete m_sort32;
	delete m_scan;
	delete m_search;

	clReleaseKernel(m_batchingKernel);
	clReleaseKernel(m_batchingKernelNew);

	clReleaseKernel(m_solveContactKernel);
	clReleaseKernel(m_solveFrictionKernel);

	clReleaseKernel(m_contactToConstraintKernel);
	clReleaseKernel(m_setSortDataKernel);
	clReleaseKernel(m_reorderContactKernel);
	clReleaseKernel(m_copyConstraintKernel);
}

template <bool JACOBI>
static __inline void solveContact(b3GpuConstraint4& cs,
								  const b3Vec3& posA, b3Vec3& linVelA, b3Vec3& angVelA, float invMassA, const b3Matrix3x3& invInertiaA,
								  const b3Vec3& posB, b3Vec3& linVelB, b3Vec3& angVelB, float invMassB, const b3Matrix3x3& invInertiaB,
								  float maxRambdaDt[4], float minRambdaDt[4])
{
	b3Vec3 dLinVelA;
	dLinVelA.setZero();
	b3Vec3 dAngVelA;
	dAngVelA.setZero();
	b3Vec3 dLinVelB;
	dLinVelB.setZero();
	b3Vec3 dAngVelB;
	dAngVelB.setZero();

	for (i32 ic = 0; ic < 4; ic++)
	{
		//	dont necessary because this makes change to 0
		if (cs.m_jacCoeffInv[ic] == 0.f) continue;

		{
			b3Vec3 angular0, angular1, linear;
			b3Vec3 r0 = cs.m_worldPos[ic] - (b3Vec3&)posA;
			b3Vec3 r1 = cs.m_worldPos[ic] - (b3Vec3&)posB;
			setLinearAndAngular((const b3Vec3&)cs.m_linear, (const b3Vec3&)r0, (const b3Vec3&)r1, &linear, &angular0, &angular1);

			float rambdaDt = calcRelVel((const b3Vec3&)cs.m_linear, (const b3Vec3&)-cs.m_linear, angular0, angular1,
										linVelA, angVelA, linVelB, angVelB) +
							 cs.m_b[ic];
			rambdaDt *= cs.m_jacCoeffInv[ic];

			{
				float prevSum = cs.m_appliedRambdaDt[ic];
				float updated = prevSum;
				updated += rambdaDt;
				updated = d3Max(updated, minRambdaDt[ic]);
				updated = d3Min(updated, maxRambdaDt[ic]);
				rambdaDt = updated - prevSum;
				cs.m_appliedRambdaDt[ic] = updated;
			}

			b3Vec3 linImp0 = invMassA * linear * rambdaDt;
			b3Vec3 linImp1 = invMassB * (-linear) * rambdaDt;
			b3Vec3 angImp0 = (invInertiaA * angular0) * rambdaDt;
			b3Vec3 angImp1 = (invInertiaB * angular1) * rambdaDt;
#ifdef _WIN32
			drx3DAssert(_finite(linImp0.getX()));
			drx3DAssert(_finite(linImp1.getX()));
#endif
			if (JACOBI)
			{
				dLinVelA += linImp0;
				dAngVelA += angImp0;
				dLinVelB += linImp1;
				dAngVelB += angImp1;
			}
			else
			{
				linVelA += linImp0;
				angVelA += angImp0;
				linVelB += linImp1;
				angVelB += angImp1;
			}
		}
	}

	if (JACOBI)
	{
		linVelA += dLinVelA;
		angVelA += dAngVelA;
		linVelB += dLinVelB;
		angVelB += dAngVelB;
	}
}

static __inline void solveFriction(b3GpuConstraint4& cs,
								   const b3Vec3& posA, b3Vec3& linVelA, b3Vec3& angVelA, float invMassA, const b3Matrix3x3& invInertiaA,
								   const b3Vec3& posB, b3Vec3& linVelB, b3Vec3& angVelB, float invMassB, const b3Matrix3x3& invInertiaB,
								   float maxRambdaDt[4], float minRambdaDt[4])
{
	if (cs.m_fJacCoeffInv[0] == 0 && cs.m_fJacCoeffInv[0] == 0) return;
	const b3Vec3& center = (const b3Vec3&)cs.m_center;

	b3Vec3 n = -(const b3Vec3&)cs.m_linear;

	b3Vec3 tangent[2];
#if 1
	b3PlaneSpace1(n, tangent[0], tangent[1]);
#else
	b3Vec3 r = cs.m_worldPos[0] - center;
	tangent[0] = cross3(n, r);
	tangent[1] = cross3(tangent[0], n);
	tangent[0] = normalize3(tangent[0]);
	tangent[1] = normalize3(tangent[1]);
#endif

	b3Vec3 angular0, angular1, linear;
	b3Vec3 r0 = center - posA;
	b3Vec3 r1 = center - posB;
	for (i32 i = 0; i < 2; i++)
	{
		setLinearAndAngular(tangent[i], r0, r1, &linear, &angular0, &angular1);
		float rambdaDt = calcRelVel(linear, -linear, angular0, angular1,
									linVelA, angVelA, linVelB, angVelB);
		rambdaDt *= cs.m_fJacCoeffInv[i];

		{
			float prevSum = cs.m_fAppliedRambdaDt[i];
			float updated = prevSum;
			updated += rambdaDt;
			updated = d3Max(updated, minRambdaDt[i]);
			updated = d3Min(updated, maxRambdaDt[i]);
			rambdaDt = updated - prevSum;
			cs.m_fAppliedRambdaDt[i] = updated;
		}

		b3Vec3 linImp0 = invMassA * linear * rambdaDt;
		b3Vec3 linImp1 = invMassB * (-linear) * rambdaDt;
		b3Vec3 angImp0 = (invInertiaA * angular0) * rambdaDt;
		b3Vec3 angImp1 = (invInertiaB * angular1) * rambdaDt;
#ifdef _WIN32
		drx3DAssert(_finite(linImp0.getX()));
		drx3DAssert(_finite(linImp1.getX()));
#endif
		linVelA += linImp0;
		angVelA += angImp0;
		linVelB += linImp1;
		angVelB += angImp1;
	}

	{  //	angular damping for point constraint
		b3Vec3 ab = (posB - posA).normalized();
		b3Vec3 ac = (center - posA).normalized();
		if (b3Dot(ab, ac) > 0.95f || (invMassA == 0.f || invMassB == 0.f))
		{
			float angNA = b3Dot(n, angVelA);
			float angNB = b3Dot(n, angVelB);

			angVelA -= (angNA * 0.1f) * n;
			angVelB -= (angNB * 0.1f) * n;
		}
	}
}
/*
 b3AlignedObjectArray<b3RigidBodyData>& m_bodies;
	b3AlignedObjectArray<b3InertiaData>& m_shapes;
	b3AlignedObjectArray<b3GpuConstraint4>& m_constraints;
	b3AlignedObjectArray<i32>* m_batchSizes;
	i32 m_cellIndex;
	i32 m_curWgidx;
	i32 m_start;
	i32 m_nConstraints;
	bool m_solveFriction;
	i32 m_maxNumBatches;
 */

struct SolveTask  // : public ThreadPool::Task
{
	SolveTask(b3AlignedObjectArray<b3RigidBodyData>& bodies, b3AlignedObjectArray<b3InertiaData>& shapes, b3AlignedObjectArray<b3GpuConstraint4>& constraints,
			  i32 start, i32 nConstraints, i32 maxNumBatches, b3AlignedObjectArray<i32>* wgUsedBodies, i32 curWgidx, b3AlignedObjectArray<i32>* batchSizes, i32 cellIndex)
		: m_bodies(bodies), m_shapes(shapes), m_constraints(constraints), m_batchSizes(batchSizes), m_cellIndex(cellIndex), m_curWgidx(curWgidx), m_start(start), m_nConstraints(nConstraints), m_solveFriction(true), m_maxNumBatches(maxNumBatches)
	{
	}

	u16 getType() { return 0; }

	void run(i32 tIdx)
	{
		i32 offset = 0;
		for (i32 ii = 0; ii < D3_MAX_NUM_BATCHES; ii++)
		{
			i32 numInBatch = m_batchSizes->at(m_cellIndex * D3_MAX_NUM_BATCHES + ii);
			if (!numInBatch)
				break;

			for (i32 jj = 0; jj < numInBatch; jj++)
			{
				i32 i = m_start + offset + jj;
				i32 batchId = m_constraints[i].m_batchIdx;
				drx3DAssert(batchId == ii);
				float frictionCoeff = m_constraints[i].getFrictionCoeff();
				i32 aIdx = (i32)m_constraints[i].m_bodyA;
				i32 bIdx = (i32)m_constraints[i].m_bodyB;
				//				i32 localBatch = m_constraints[i].m_batchIdx;
				b3RigidBodyData& bodyA = m_bodies[aIdx];
				b3RigidBodyData& bodyB = m_bodies[bIdx];

				if (!m_solveFriction)
				{
					float maxRambdaDt[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
					float minRambdaDt[4] = {0.f, 0.f, 0.f, 0.f};

					solveContact<false>(m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass, (const b3Matrix3x3&)m_shapes[aIdx].m_invInertiaWorld,
										(b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass, (const b3Matrix3x3&)m_shapes[bIdx].m_invInertiaWorld,
										maxRambdaDt, minRambdaDt);
				}
				else
				{
					float maxRambdaDt[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
					float minRambdaDt[4] = {0.f, 0.f, 0.f, 0.f};
					float sum = 0;
					for (i32 j = 0; j < 4; j++)
					{
						sum += m_constraints[i].m_appliedRambdaDt[j];
					}
					frictionCoeff = 0.7f;
					for (i32 j = 0; j < 4; j++)
					{
						maxRambdaDt[j] = frictionCoeff * sum;
						minRambdaDt[j] = -maxRambdaDt[j];
					}
					solveFriction(m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass, (const b3Matrix3x3&)m_shapes[aIdx].m_invInertiaWorld,
								  (b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass, (const b3Matrix3x3&)m_shapes[bIdx].m_invInertiaWorld,
								  maxRambdaDt, minRambdaDt);
				}
			}
			offset += numInBatch;
		}
		/*		for (i32 bb=0;bb<m_maxNumBatches;bb++)
		{
			//for(i32 ic=m_nConstraints-1; ic>=0; ic--)
			for(i32 ic=0; ic<m_nConstraints; ic++)
			{
				
				i32 i = m_start + ic;
				if (m_constraints[i].m_batchIdx != bb)
					continue;

				float frictionCoeff = m_constraints[i].getFrictionCoeff();
				i32 aIdx = (i32)m_constraints[i].m_bodyA;
				i32 bIdx = (i32)m_constraints[i].m_bodyB;
				i32 localBatch = m_constraints[i].m_batchIdx;
				b3RigidBodyData& bodyA = m_bodies[aIdx];
				b3RigidBodyData& bodyB = m_bodies[bIdx];

				if( !m_solveFriction )
				{
					float maxRambdaDt[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};
					float minRambdaDt[4] = {0.f,0.f,0.f,0.f};

					solveContact<false>( m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass, (const b3Matrix3x3 &)m_shapes[aIdx].m_invInertiaWorld, 
							(b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass, (const b3Matrix3x3 &)m_shapes[bIdx].m_invInertiaWorld,
						maxRambdaDt, minRambdaDt );
				}
				else
				{
					float maxRambdaDt[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};
					float minRambdaDt[4] = {0.f,0.f,0.f,0.f};
					float sum = 0;
					for(i32 j=0; j<4; j++)
					{
						sum +=m_constraints[i].m_appliedRambdaDt[j];
					}
					frictionCoeff = 0.7f;
					for(i32 j=0; j<4; j++)
					{
						maxRambdaDt[j] = frictionCoeff*sum;
						minRambdaDt[j] = -maxRambdaDt[j];
					}
					solveFriction( m_constraints[i], (b3Vec3&)bodyA.m_pos, (b3Vec3&)bodyA.m_linVel, (b3Vec3&)bodyA.m_angVel, bodyA.m_invMass,(const b3Matrix3x3 &) m_shapes[aIdx].m_invInertiaWorld, 
						(b3Vec3&)bodyB.m_pos, (b3Vec3&)bodyB.m_linVel, (b3Vec3&)bodyB.m_angVel, bodyB.m_invMass,(const b3Matrix3x3 &) m_shapes[bIdx].m_invInertiaWorld,
						maxRambdaDt, minRambdaDt );
			
				}
			}
		}
		*/
	}

	b3AlignedObjectArray<b3RigidBodyData>& m_bodies;
	b3AlignedObjectArray<b3InertiaData>& m_shapes;
	b3AlignedObjectArray<b3GpuConstraint4>& m_constraints;
	b3AlignedObjectArray<i32>* m_batchSizes;
	i32 m_cellIndex;
	i32 m_curWgidx;
	i32 m_start;
	i32 m_nConstraints;
	bool m_solveFriction;
	i32 m_maxNumBatches;
};

void b3Solver::solveContactConstraintHost(b3OpenCLArray<b3RigidBodyData>* bodyBuf, b3OpenCLArray<b3InertiaData>* shapeBuf,
										  b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches, b3AlignedObjectArray<i32>* batchSizes)
{
#if 0
	{	
		i32 nSplitX = D3_SOLVER_N_SPLIT_X;
		i32 nSplitY = D3_SOLVER_N_SPLIT_Y;
		i32 numWorkgroups = D3_SOLVER_N_CELLS/D3_SOLVER_N_BATCHES;
		for (i32 z=0;z<4;z++)
		{
			for (i32 y=0;y<4;y++)
			{
				for (i32 x=0;x<4;x++)
				{
					i32 newIndex = (x+y*nSplitX+z*nSplitX*nSplitY);
				//	printf("newIndex=%d\n",newIndex);

					i32 zIdx = newIndex/(nSplitX*nSplitY);
					i32 remain = newIndex%(nSplitX*nSplitY);
					i32 yIdx = remain/nSplitX;
					i32 xIdx = remain%nSplitX;
				//	printf("newIndex=%d\n",newIndex);
				}
			}
		}

		//for (i32 wgIdx=numWorkgroups-1;wgIdx>=0;wgIdx--)
		for (i32 cellBatch=0;cellBatch<D3_SOLVER_N_BATCHES;cellBatch++)
		{
			for (i32 wgIdx=0;wgIdx<numWorkgroups;wgIdx++)
			{
				i32 zIdx = (wgIdx/((nSplitX*nSplitY)/4))*2+((cellBatch&4)>>2);
				i32 remain= (wgIdx%((nSplitX*nSplitY)/4));
				i32 yIdx = (remain/(nSplitX/2))*2 + ((cellBatch&2)>>1);
				i32 xIdx = (remain%(nSplitX/2))*2 + (cellBatch&1);
				
				/*i32 zIdx = newIndex/(nSplitX*nSplitY);
				i32 remain = newIndex%(nSplitX*nSplitY);
				i32 yIdx = remain/nSplitX;
				i32 xIdx = remain%nSplitX;
				*/
				i32 cellIdx = xIdx+yIdx*nSplitX+zIdx*(nSplitX*nSplitY);
			//	printf("wgIdx %d: xIdx=%d, yIdx=%d, zIdx=%d, cellIdx=%d, cell Batch %d\n",wgIdx,xIdx,yIdx,zIdx,cellIdx,cellBatch);
			}
		}
	}
#endif

	b3AlignedObjectArray<b3RigidBodyData> bodyNative;
	bodyBuf->copyToHost(bodyNative);
	b3AlignedObjectArray<b3InertiaData> shapeNative;
	shapeBuf->copyToHost(shapeNative);
	b3AlignedObjectArray<b3GpuConstraint4> constraintNative;
	constraint->copyToHost(constraintNative);

	b3AlignedObjectArray<u32> numConstraintsHost;
	m_numConstraints->copyToHost(numConstraintsHost);

	//printf("------------------------\n");
	b3AlignedObjectArray<u32> offsetsHost;
	m_offsets->copyToHost(offsetsHost);
	static i32 frame = 0;
	bool useBatches = true;
	if (useBatches)
	{
		for (i32 iter = 0; iter < m_nIterations; iter++)
		{
			for (i32 cellBatch = 0; cellBatch < D3_SOLVER_N_BATCHES; cellBatch++)
			{
				i32 nSplitX = D3_SOLVER_N_SPLIT_X;
				i32 nSplitY = D3_SOLVER_N_SPLIT_Y;
				i32 numWorkgroups = D3_SOLVER_N_CELLS / D3_SOLVER_N_BATCHES;
				//printf("cell Batch %d\n",cellBatch);
				b3AlignedObjectArray<i32> usedBodies[D3_SOLVER_N_CELLS];
				for (i32 i = 0; i < D3_SOLVER_N_CELLS; i++)
				{
					usedBodies[i].resize(0);
				}

				//for (i32 wgIdx=numWorkgroups-1;wgIdx>=0;wgIdx--)
				for (i32 wgIdx = 0; wgIdx < numWorkgroups; wgIdx++)
				{
					i32 zIdx = (wgIdx / ((nSplitX * nSplitY) / 4)) * 2 + ((cellBatch & 4) >> 2);
					i32 remain = (wgIdx % ((nSplitX * nSplitY) / 4));
					i32 yIdx = (remain / (nSplitX / 2)) * 2 + ((cellBatch & 2) >> 1);
					i32 xIdx = (remain % (nSplitX / 2)) * 2 + (cellBatch & 1);
					i32 cellIdx = xIdx + yIdx * nSplitX + zIdx * (nSplitX * nSplitY);

					if (numConstraintsHost[cellIdx] == 0)
						continue;

					//printf("wgIdx %d: xIdx=%d, yIdx=%d, zIdx=%d, cellIdx=%d, cell Batch %d\n",wgIdx,xIdx,yIdx,zIdx,cellIdx,cellBatch);
					//printf("cell %d has %d constraints\n", cellIdx,numConstraintsHost[cellIdx]);
					if (zIdx)
					{
						//printf("?\n");
					}

					if (iter == 0)
					{
						//printf("frame=%d, Cell xIdx=%x, yIdx=%d ",frame, xIdx,yIdx);
						//printf("cellBatch=%d, wgIdx=%d, #constraints in cell=%d\n",cellBatch,wgIdx,numConstraintsHost[cellIdx]);
					}
					i32k start = offsetsHost[cellIdx];
					i32 numConstraintsInCell = numConstraintsHost[cellIdx];
					//				i32k end = start + numConstraintsInCell;

					SolveTask task(bodyNative, shapeNative, constraintNative, start, numConstraintsInCell, maxNumBatches, usedBodies, wgIdx, batchSizes, cellIdx);
					task.m_solveFriction = false;
					task.run(0);
				}
			}
		}

		for (i32 iter = 0; iter < m_nIterations; iter++)
		{
			for (i32 cellBatch = 0; cellBatch < D3_SOLVER_N_BATCHES; cellBatch++)
			{
				i32 nSplitX = D3_SOLVER_N_SPLIT_X;
				i32 nSplitY = D3_SOLVER_N_SPLIT_Y;

				i32 numWorkgroups = D3_SOLVER_N_CELLS / D3_SOLVER_N_BATCHES;

				for (i32 wgIdx = 0; wgIdx < numWorkgroups; wgIdx++)
				{
					i32 zIdx = (wgIdx / ((nSplitX * nSplitY) / 4)) * 2 + ((cellBatch & 4) >> 2);
					i32 remain = (wgIdx % ((nSplitX * nSplitY) / 4));
					i32 yIdx = (remain / (nSplitX / 2)) * 2 + ((cellBatch & 2) >> 1);
					i32 xIdx = (remain % (nSplitX / 2)) * 2 + (cellBatch & 1);

					i32 cellIdx = xIdx + yIdx * nSplitX + zIdx * (nSplitX * nSplitY);

					if (numConstraintsHost[cellIdx] == 0)
						continue;

					//printf("yIdx=%d\n",yIdx);

					i32k start = offsetsHost[cellIdx];
					i32 numConstraintsInCell = numConstraintsHost[cellIdx];
					//				i32k end = start + numConstraintsInCell;

					SolveTask task(bodyNative, shapeNative, constraintNative, start, numConstraintsInCell, maxNumBatches, 0, 0, batchSizes, cellIdx);
					task.m_solveFriction = true;
					task.run(0);
				}
			}
		}
	}
	else
	{
		for (i32 iter = 0; iter < m_nIterations; iter++)
		{
			SolveTask task(bodyNative, shapeNative, constraintNative, 0, n, maxNumBatches, 0, 0, 0, 0);
			task.m_solveFriction = false;
			task.run(0);
		}

		for (i32 iter = 0; iter < m_nIterations; iter++)
		{
			SolveTask task(bodyNative, shapeNative, constraintNative, 0, n, maxNumBatches, 0, 0, 0, 0);
			task.m_solveFriction = true;
			task.run(0);
		}
	}

	bodyBuf->copyFromHost(bodyNative);
	shapeBuf->copyFromHost(shapeNative);
	constraint->copyFromHost(constraintNative);
	frame++;
}

void checkConstraintBatch(const b3OpenCLArray<b3RigidBodyData>* bodyBuf,
						  const b3OpenCLArray<b3InertiaData>* shapeBuf,
						  b3OpenCLArray<b3GpuConstraint4>* constraint,
						  b3OpenCLArray<u32>* m_numConstraints,
						  b3OpenCLArray<u32>* m_offsets,
						  i32 batchId)
{
	//						b3BufferInfoCL( m_numConstraints->getBufferCL() ),
	//						b3BufferInfoCL( m_offsets->getBufferCL() )

	i32 cellBatch = batchId;
	i32k nn = D3_SOLVER_N_CELLS;
	//	i32 numWorkItems = 64*nn/D3_SOLVER_N_BATCHES;

	b3AlignedObjectArray<u32> gN;
	m_numConstraints->copyToHost(gN);
	b3AlignedObjectArray<u32> gOffsets;
	m_offsets->copyToHost(gOffsets);
	i32 nSplitX = D3_SOLVER_N_SPLIT_X;
	i32 nSplitY = D3_SOLVER_N_SPLIT_Y;

	//	i32 bIdx = batchId;

	b3AlignedObjectArray<b3GpuConstraint4> cpuConstraints;
	constraint->copyToHost(cpuConstraints);

	printf("batch = %d\n", batchId);

	i32 numWorkgroups = nn / D3_SOLVER_N_BATCHES;
	b3AlignedObjectArray<i32> usedBodies;

	for (i32 wgIdx = 0; wgIdx < numWorkgroups; wgIdx++)
	{
		printf("wgIdx = %d           ", wgIdx);

		i32 zIdx = (wgIdx / ((nSplitX * nSplitY)) / 2) * 2 + ((cellBatch & 4) >> 2);
		i32 remain = wgIdx % ((nSplitX * nSplitY));
		i32 yIdx = (remain % (nSplitX / 2)) * 2 + ((cellBatch & 2) >> 1);
		i32 xIdx = (remain / (nSplitX / 2)) * 2 + (cellBatch & 1);

		i32 cellIdx = xIdx + yIdx * nSplitX + zIdx * (nSplitX * nSplitY);
		printf("cellIdx=%d\n", cellIdx);
		if (gN[cellIdx] == 0)
			continue;

		i32k start = gOffsets[cellIdx];
		i32k end = start + gN[cellIdx];

		for (i32 c = start; c < end; c++)
		{
			b3GpuConstraint4& constraint = cpuConstraints[c];
			//printf("constraint (%d,%d)\n", constraint.m_bodyA,constraint.m_bodyB);
			if (usedBodies.findLinearSearch(constraint.m_bodyA) < usedBodies.size())
			{
				printf("error?\n");
			}
			if (usedBodies.findLinearSearch(constraint.m_bodyB) < usedBodies.size())
			{
				printf("error?\n");
			}
		}

		for (i32 c = start; c < end; c++)
		{
			b3GpuConstraint4& constraint = cpuConstraints[c];
			usedBodies.push_back(constraint.m_bodyA);
			usedBodies.push_back(constraint.m_bodyB);
		}
	}
}

static bool verify = false;

void b3Solver::solveContactConstraint(const b3OpenCLArray<b3RigidBodyData>* bodyBuf, const b3OpenCLArray<b3InertiaData>* shapeBuf,
									  b3OpenCLArray<b3GpuConstraint4>* constraint, uk additionalData, i32 n, i32 maxNumBatches)
{
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
			for (i32 iter = 0; iter < m_nIterations; iter++)
			{
				for (i32 ib = 0; ib < D3_SOLVER_N_BATCHES; ib++)
				{
					if (verify)
					{
						checkConstraintBatch(bodyBuf, shapeBuf, constraint, m_numConstraints, m_offsets, ib);
					}

#ifdef DEBUG_ME
					memset(debugInfo, 0, sizeof(SolverDebugInfo) * numWorkItems);
					gpuDebugInfo.write(debugInfo, numWorkItems);
#endif

					cdata.z = ib;

					b3LauncherCL launcher(m_queue, m_solveContactKernel, "m_solveContactKernel");
#if 1

					b3BufferInfoCL bInfo[] = {

						b3BufferInfoCL(bodyBuf->getBufferCL()),
						b3BufferInfoCL(shapeBuf->getBufferCL()),
						b3BufferInfoCL(constraint->getBufferCL()),
						b3BufferInfoCL(m_numConstraints->getBufferCL()),
						b3BufferInfoCL(m_offsets->getBufferCL())
#ifdef DEBUG_ME
							,
						b3BufferInfoCL(&gpuDebugInfo)
#endif
					};

					launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
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

			clFinish(m_queue);
		}

		cdata.x = 1;
		bool applyFriction = true;
		if (applyFriction)
		{
			D3_PROFILE("m_batchSolveKernel iterations2");
			for (i32 iter = 0; iter < m_nIterations; iter++)
			{
				for (i32 ib = 0; ib < D3_SOLVER_N_BATCHES; ib++)
				{
					cdata.z = ib;

					b3BufferInfoCL bInfo[] = {
						b3BufferInfoCL(bodyBuf->getBufferCL()),
						b3BufferInfoCL(shapeBuf->getBufferCL()),
						b3BufferInfoCL(constraint->getBufferCL()),
						b3BufferInfoCL(m_numConstraints->getBufferCL()),
						b3BufferInfoCL(m_offsets->getBufferCL())
#ifdef DEBUG_ME
							,
						b3BufferInfoCL(&gpuDebugInfo)
#endif  //DEBUG_ME
					};
					b3LauncherCL launcher(m_queue, m_solveFrictionKernel, "m_solveFrictionKernel");
					launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
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
			clFinish(m_queue);
		}
#ifdef DEBUG_ME
		delete[] debugInfo;
#endif  //DEBUG_ME
	}
}

void b3Solver::convertToConstraints(const b3OpenCLArray<b3RigidBodyData>* bodyBuf,
									const b3OpenCLArray<b3InertiaData>* shapeBuf,
									b3OpenCLArray<b3Contact4>* contactsIn, b3OpenCLArray<b3GpuConstraint4>* contactCOut, uk additionalData,
									i32 nContacts, const ConstraintCfg& cfg)
{
	//	b3OpenCLArray<b3GpuConstraint4>* constraintNative =0;
	contactCOut->resize(nContacts);
	struct CB
	{
		i32 m_nContacts;
		float m_dt;
		float m_positionDrift;
		float m_positionConstraintCoeff;
	};

	{
		CB cdata;
		cdata.m_nContacts = nContacts;
		cdata.m_dt = cfg.m_dt;
		cdata.m_positionDrift = cfg.m_positionDrift;
		cdata.m_positionConstraintCoeff = cfg.m_positionConstraintCoeff;

		if (gConvertConstraintOnCpu)
		{
			b3AlignedObjectArray<b3RigidBodyData> gBodies;
			bodyBuf->copyToHost(gBodies);

			b3AlignedObjectArray<b3Contact4> gContact;
			contactsIn->copyToHost(gContact);

			b3AlignedObjectArray<b3InertiaData> gShapes;
			shapeBuf->copyToHost(gShapes);

			b3AlignedObjectArray<b3GpuConstraint4> gConstraintOut;
			gConstraintOut.resize(nContacts);

			D3_PROFILE("cpu contactToConstraintKernel");
			for (i32 gIdx = 0; gIdx < nContacts; gIdx++)
			{
				i32 aIdx = abs(gContact[gIdx].m_bodyAPtrAndSignBit);
				i32 bIdx = abs(gContact[gIdx].m_bodyBPtrAndSignBit);

				b3Float4 posA = gBodies[aIdx].m_pos;
				b3Float4 linVelA = gBodies[aIdx].m_linVel;
				b3Float4 angVelA = gBodies[aIdx].m_angVel;
				float invMassA = gBodies[aIdx].m_invMass;
				b3Mat3x3 invInertiaA = gShapes[aIdx].m_initInvInertia;

				b3Float4 posB = gBodies[bIdx].m_pos;
				b3Float4 linVelB = gBodies[bIdx].m_linVel;
				b3Float4 angVelB = gBodies[bIdx].m_angVel;
				float invMassB = gBodies[bIdx].m_invMass;
				b3Mat3x3 invInertiaB = gShapes[bIdx].m_initInvInertia;

				b3ContactConstraint4_t cs;

				setConstraint4(posA, linVelA, angVelA, invMassA, invInertiaA, posB, linVelB, angVelB, invMassB, invInertiaB,
							   &gContact[gIdx], cdata.m_dt, cdata.m_positionDrift, cdata.m_positionConstraintCoeff,
							   &cs);

				cs.m_batchIdx = gContact[gIdx].m_batchIdx;

				gConstraintOut[gIdx] = (b3GpuConstraint4&)cs;
			}

			contactCOut->copyFromHost(gConstraintOut);
		}
		else
		{
			D3_PROFILE("gpu m_contactToConstraintKernel");

			b3BufferInfoCL bInfo[] = {b3BufferInfoCL(contactsIn->getBufferCL()), b3BufferInfoCL(bodyBuf->getBufferCL()), b3BufferInfoCL(shapeBuf->getBufferCL()),
									  b3BufferInfoCL(contactCOut->getBufferCL())};
			b3LauncherCL launcher(m_queue, m_contactToConstraintKernel, "m_contactToConstraintKernel");
			launcher.setBuffers(bInfo, sizeof(bInfo) / sizeof(b3BufferInfoCL));
			//launcher.setConst(  cdata );

			launcher.setConst(cdata.m_nContacts);
			launcher.setConst(cdata.m_dt);
			launcher.setConst(cdata.m_positionDrift);
			launcher.setConst(cdata.m_positionConstraintCoeff);

			launcher.launch1D(nContacts, 64);
			clFinish(m_queue);
		}
	}
}

/*
void b3Solver::sortContacts(  const b3OpenCLArray<b3RigidBodyData>* bodyBuf, 
			b3OpenCLArray<b3Contact4>* contactsIn, uk additionalData, 
			i32 nContacts, const b3Solver::ConstraintCfg& cfg )
{
	
	

	i32k sortAlignment = 512; // todo. get this out of sort
	if( cfg.m_enableParallelSolve )
	{
		

		i32 sortSize = NEXTMULTIPLEOF( nContacts, sortAlignment );

		b3OpenCLArray<u32>* countsNative = m_numConstraints;//BufferUtils::map<TYPE_CL, false>( data->m_device, &countsHost );
		b3OpenCLArray<u32>* offsetsNative = m_offsets;//BufferUtils::map<TYPE_CL, false>( data->m_device, &offsetsHost );

		{	//	2. set cell idx
			struct CB
			{
				i32 m_nContacts;
				i32 m_staticIdx;
				float m_scale;
				i32 m_nSplit;
			};

			drx3DAssert( sortSize%64 == 0 );
			CB cdata;
			cdata.m_nContacts = nContacts;
			cdata.m_staticIdx = cfg.m_staticIdx;
			cdata.m_scale = 1.f/(N_OBJ_PER_SPLIT*cfg.m_averageExtent);
			cdata.m_nSplit = D3_SOLVER_N_SPLIT;

			
			b3BufferInfoCL bInfo[] = { b3BufferInfoCL( contactsIn->getBufferCL() ), b3BufferInfoCL( bodyBuf->getBufferCL() ), b3BufferInfoCL( m_sortDataBuffer->getBufferCL() ) };
			b3LauncherCL launcher( m_queue, m_setSortDataKernel );
			launcher.setBuffers( bInfo, sizeof(bInfo)/sizeof(b3BufferInfoCL) );
			launcher.setConst(  cdata );
			launcher.launch1D( sortSize, 64 );
		}

		{	//	3. sort by cell idx
			i32 n = D3_SOLVER_N_SPLIT*D3_SOLVER_N_SPLIT;
			i32 sortBit = 32;
			//if( n <= 0xffff ) sortBit = 16;
			//if( n <= 0xff ) sortBit = 8;
			m_sort32->execute(*m_sortDataBuffer,sortSize);
		}
		{	//	4. find entries
			m_search->execute( *m_sortDataBuffer, nContacts, *countsNative, D3_SOLVER_N_SPLIT*D3_SOLVER_N_SPLIT, b3BoundSearchCL::COUNT);

			m_scan->execute( *countsNative, *offsetsNative, D3_SOLVER_N_SPLIT*D3_SOLVER_N_SPLIT );
		}

		{	//	5. sort constraints by cellIdx
			//	todo. preallocate this
//			drx3DAssert( contactsIn->getType() == TYPE_HOST );
//			b3OpenCLArray<b3Contact4>* out = BufferUtils::map<TYPE_CL, false>( data->m_device, contactsIn );	//	copying contacts to this buffer

			{
				

				b3Int4 cdata; cdata.x = nContacts;
				b3BufferInfoCL bInfo[] = { b3BufferInfoCL( contactsIn->getBufferCL() ), b3BufferInfoCL( m_contactBuffer->getBufferCL() ), b3BufferInfoCL( m_sortDataBuffer->getBufferCL() ) };
				b3LauncherCL launcher( m_queue, m_reorderContactKernel );
				launcher.setBuffers( bInfo, sizeof(bInfo)/sizeof(b3BufferInfoCL) );
				launcher.setConst(  cdata );
				launcher.launch1D( nContacts, 64 );
			}
//			BufferUtils::unmap<true>( out, contactsIn, nContacts );
		}
	}

	
}

*/
void b3Solver::batchContacts(b3OpenCLArray<b3Contact4>* contacts, i32 nContacts, b3OpenCLArray<u32>* nNative, b3OpenCLArray<u32>* offsetsNative, i32 staticIdx)
{
	i32 numWorkItems = 64 * D3_SOLVER_N_CELLS;
	{
		D3_PROFILE("batch generation");

		b3Int4 cdata;
		cdata.x = nContacts;
		cdata.y = 0;
		cdata.z = staticIdx;

#ifdef BATCH_DEBUG
		SolverDebugInfo* debugInfo = new SolverDebugInfo[numWorkItems];
		adl::b3OpenCLArray<SolverDebugInfo> gpuDebugInfo(data->m_device, numWorkItems);
		memset(debugInfo, 0, sizeof(SolverDebugInfo) * numWorkItems);
		gpuDebugInfo.write(debugInfo, numWorkItems);
#endif

#if 0
		b3BufferInfoCL bInfo[] = { 
			b3BufferInfoCL( contacts->getBufferCL() ), 
			b3BufferInfoCL(  m_contactBuffer2->getBufferCL()),
			b3BufferInfoCL( nNative->getBufferCL() ), 
			b3BufferInfoCL( offsetsNative->getBufferCL() ),
#ifdef BATCH_DEBUG
			,	b3BufferInfoCL(&gpuDebugInfo)
#endif
		};
#endif

		{
			m_batchSizes.resize(nNative->size());
			D3_PROFILE("batchingKernel");
			//b3LauncherCL launcher( m_queue, m_batchingKernel);
			cl_kernel k = useNewBatchingKernel ? m_batchingKernelNew : m_batchingKernel;

			b3LauncherCL launcher(m_queue, k, "*batchingKernel");
			if (!useNewBatchingKernel)
			{
				launcher.setBuffer(contacts->getBufferCL());
			}
			launcher.setBuffer(m_contactBuffer2->getBufferCL());
			launcher.setBuffer(nNative->getBufferCL());
			launcher.setBuffer(offsetsNative->getBufferCL());

			launcher.setBuffer(m_batchSizes.getBufferCL());

			//launcher.setConst(  cdata );
			launcher.setConst(staticIdx);

			launcher.launch1D(numWorkItems, 64);
			//clFinish(m_queue);
			//b3AlignedObjectArray<i32> batchSizesCPU;
			//m_batchSizes.copyToHost(batchSizesCPU);
			//printf(".\n");
		}

#ifdef BATCH_DEBUG
		aaaa
			b3Contact4* hostContacts = new b3Contact4[nContacts];
		m_contactBuffer->read(hostContacts, nContacts);
		clFinish(m_queue);

		gpuDebugInfo.read(debugInfo, numWorkItems);
		clFinish(m_queue);

		for (i32 i = 0; i < numWorkItems; i++)
		{
			if (debugInfo[i].m_valInt1 > 0)
			{
				printf("catch\n");
			}
			if (debugInfo[i].m_valInt2 > 0)
			{
				printf("catch22\n");
			}

			if (debugInfo[i].m_valInt3 > 0)
			{
				printf("catch666\n");
			}

			if (debugInfo[i].m_valInt4 > 0)
			{
				printf("catch777\n");
			}
		}
		delete[] debugInfo;
#endif  //BATCH_DEBUG
	}

	//	copy buffer to buffer
	//drx3DAssert(m_contactBuffer->size()==nContacts);
	//contacts->copyFromOpenCLArray( *m_contactBuffer);
	//clFinish(m_queue);//needed?
}
