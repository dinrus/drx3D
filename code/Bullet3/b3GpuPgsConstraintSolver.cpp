bool useGpuInitSolverBodies = true;
bool useGpuInfo1 = true;
bool useGpuInfo2 = true;
bool useGpuSolveJointConstraintRows = true;
bool useGpuWriteBackVelocities = true;
bool gpuBreakConstraints = true;

#include <drx3D/OpenCL/RigidBody/b3GpuPgsConstraintSolver.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>
#include <new>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <string.h>  //for memset
#include <drx3D/Physics/Collision/NarrowPhase/b3Contact4.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3OpenCLArray.h>
#include <drx3D/OpenCL/ParallelPrimitives/b3LauncherCL.h>

#include <drx3D/OpenCL/ParallelPrimitives/b3PrefixScanCL.h>

#include <drx3D/OpenCL/RigidBody/kernels/jointSolver.h>  //solveConstraintRowsCL
#include <drx3D/OpenCL/Initialize/b3OpenCLUtils.h>

#define D3_JOINT_SOLVER_PATH "drx3D/Bullet3/OpenCL/RigidBody/kernels/jointSolver.cl"

struct b3GpuPgsJacobiSolverInternalData
{
	cl_context m_context;
	cl_device_id m_device;
	cl_command_queue m_queue;

	b3PrefixScanCL* m_prefixScan;

	cl_kernel m_solveJointConstraintRowsKernels;
	cl_kernel m_initSolverBodiesKernel;
	cl_kernel m_getInfo1Kernel;
	cl_kernel m_initBatchConstraintsKernel;
	cl_kernel m_getInfo2Kernel;
	cl_kernel m_writeBackVelocitiesKernel;
	cl_kernel m_breakViolatedConstraintsKernel;

	b3OpenCLArray<u32>* m_gpuConstraintRowOffsets;

	b3OpenCLArray<b3GpuSolverBody>* m_gpuSolverBodies;
	b3OpenCLArray<b3BatchConstraint>* m_gpuBatchConstraints;
	b3OpenCLArray<b3GpuSolverConstraint>* m_gpuConstraintRows;
	b3OpenCLArray<u32>* m_gpuConstraintInfo1;

	//	b3AlignedObjectArray<b3GpuSolverBody>		m_cpuSolverBodies;
	b3AlignedObjectArray<b3BatchConstraint> m_cpuBatchConstraints;
	b3AlignedObjectArray<b3GpuSolverConstraint> m_cpuConstraintRows;
	b3AlignedObjectArray<u32> m_cpuConstraintInfo1;
	b3AlignedObjectArray<u32> m_cpuConstraintRowOffsets;

	b3AlignedObjectArray<b3RigidBodyData> m_cpuBodies;
	b3AlignedObjectArray<b3InertiaData> m_cpuInertias;

	b3AlignedObjectArray<b3GpuGenericConstraint> m_cpuConstraints;

	b3AlignedObjectArray<i32> m_batchSizes;
};

/*
static b3Transform	getWorldTransform(b3RigidBodyData* rb)
{
	b3Transform newTrans;
	newTrans.setOrigin(rb->m_pos);
	newTrans.setRotation(rb->m_quat);
	return newTrans;
}

static const b3Matrix3x3&	getInvInertiaTensorWorld(b3InertiaData* inertia)
{
	return inertia->m_invInertiaWorld;
}

*/

static const b3Vec3& getLinearVelocity(b3RigidBodyData* rb)
{
	return rb->m_linVel;
}

static const b3Vec3& getAngularVelocity(b3RigidBodyData* rb)
{
	return rb->m_angVel;
}

b3Vec3 getVelocityInLocalPoint(b3RigidBodyData* rb, const b3Vec3& rel_pos)
{
	//we also calculate lin/ang velocity for kinematic objects
	return getLinearVelocity(rb) + getAngularVelocity(rb).cross(rel_pos);
}

b3GpuPgsConstraintSolver::b3GpuPgsConstraintSolver(cl_context ctx, cl_device_id device, cl_command_queue queue, bool usePgs)
{
	m_usePgs = usePgs;
	m_gpuData = new b3GpuPgsJacobiSolverInternalData();
	m_gpuData->m_context = ctx;
	m_gpuData->m_device = device;
	m_gpuData->m_queue = queue;

	m_gpuData->m_prefixScan = new b3PrefixScanCL(ctx, device, queue);

	m_gpuData->m_gpuConstraintRowOffsets = new b3OpenCLArray<u32>(m_gpuData->m_context, m_gpuData->m_queue);

	m_gpuData->m_gpuSolverBodies = new b3OpenCLArray<b3GpuSolverBody>(m_gpuData->m_context, m_gpuData->m_queue);
	m_gpuData->m_gpuBatchConstraints = new b3OpenCLArray<b3BatchConstraint>(m_gpuData->m_context, m_gpuData->m_queue);
	m_gpuData->m_gpuConstraintRows = new b3OpenCLArray<b3GpuSolverConstraint>(m_gpuData->m_context, m_gpuData->m_queue);
	m_gpuData->m_gpuConstraintInfo1 = new b3OpenCLArray<u32>(m_gpuData->m_context, m_gpuData->m_queue);
	cl_int errNum = 0;

	{
		cl_program prog = b3OpenCLUtils::compileCLProgramFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, &errNum, "", D3_JOINT_SOLVER_PATH);
		//cl_program prog = b3OpenCLUtils::compileCLProgramFromString(m_gpuData->m_context,m_gpuData->m_device,0,&errNum,"",D3_JOINT_SOLVER_PATH,true);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_solveJointConstraintRowsKernels = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "solveJointConstraintRows", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_initSolverBodiesKernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "initSolverBodies", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_getInfo1Kernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "getInfo1Kernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_initBatchConstraintsKernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "initBatchConstraintsKernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_getInfo2Kernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "getInfo2Kernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_writeBackVelocitiesKernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "writeBackVelocitiesKernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);
		m_gpuData->m_breakViolatedConstraintsKernel = b3OpenCLUtils::compileCLKernelFromString(m_gpuData->m_context, m_gpuData->m_device, solveConstraintRowsCL, "breakViolatedConstraintsKernel", &errNum, prog);
		drx3DAssert(errNum == CL_SUCCESS);

		clReleaseProgram(prog);
	}
}

b3GpuPgsConstraintSolver::~b3GpuPgsConstraintSolver()
{
	clReleaseKernel(m_gpuData->m_solveJointConstraintRowsKernels);
	clReleaseKernel(m_gpuData->m_initSolverBodiesKernel);
	clReleaseKernel(m_gpuData->m_getInfo1Kernel);
	clReleaseKernel(m_gpuData->m_initBatchConstraintsKernel);
	clReleaseKernel(m_gpuData->m_getInfo2Kernel);
	clReleaseKernel(m_gpuData->m_writeBackVelocitiesKernel);
	clReleaseKernel(m_gpuData->m_breakViolatedConstraintsKernel);

	delete m_gpuData->m_prefixScan;
	delete m_gpuData->m_gpuConstraintRowOffsets;
	delete m_gpuData->m_gpuSolverBodies;
	delete m_gpuData->m_gpuBatchConstraints;
	delete m_gpuData->m_gpuConstraintRows;
	delete m_gpuData->m_gpuConstraintInfo1;

	delete m_gpuData;
}

struct b3BatchConstraint
{
	i32 m_bodyAPtrAndSignBit;
	i32 m_bodyBPtrAndSignBit;
	i32 m_originalConstraintIndex;
	i32 m_batchId;
};

static b3AlignedObjectArray<b3BatchConstraint> batchConstraints;

void b3GpuPgsConstraintSolver::recomputeBatches()
{
	m_gpuData->m_batchSizes.clear();
}

b3Scalar b3GpuPgsConstraintSolver::solveGroupCacheFriendlySetup(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias, i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal)
{
	D3_PROFILE("GPU solveGroupCacheFriendlySetup");
	batchConstraints.resize(numConstraints);
	m_gpuData->m_gpuBatchConstraints->resize(numConstraints);
	m_staticIdx = -1;
	m_maxOverrideNumSolverIterations = 0;

	/*	m_gpuData->m_gpuBodies->resize(numBodies);
	m_gpuData->m_gpuBodies->copyFromHostPointer(bodies,numBodies);

	b3OpenCLArray<b3InertiaData> gpuInertias(m_gpuData->m_context,m_gpuData->m_queue);
	gpuInertias.resize(numBodies);
	gpuInertias.copyFromHostPointer(inertias,numBodies);
	*/

	m_gpuData->m_gpuSolverBodies->resize(numBodies);

	m_tmpSolverBodyPool.resize(numBodies);
	{
		if (useGpuInitSolverBodies)
		{
			D3_PROFILE("m_initSolverBodiesKernel");

			b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_initSolverBodiesKernel, "m_initSolverBodiesKernel");
			launcher.setBuffer(m_gpuData->m_gpuSolverBodies->getBufferCL());
			launcher.setBuffer(gpuBodies->getBufferCL());
			launcher.setConst(numBodies);
			launcher.launch1D(numBodies);
			clFinish(m_gpuData->m_queue);

			//			m_gpuData->m_gpuSolverBodies->copyToHost(m_tmpSolverBodyPool);
		}
		else
		{
			gpuBodies->copyToHost(m_gpuData->m_cpuBodies);
			for (i32 i = 0; i < numBodies; i++)
			{
				b3RigidBodyData& body = m_gpuData->m_cpuBodies[i];
				b3GpuSolverBody& solverBody = m_tmpSolverBodyPool[i];
				initSolverBody(i, &solverBody, &body);
				solverBody.m_originalBodyIndex = i;
			}
			m_gpuData->m_gpuSolverBodies->copyFromHost(m_tmpSolverBodyPool);
		}
	}

	//	i32 totalBodies = 0;
	i32 totalNumRows = 0;
	//b3RigidBody* rb0=0,*rb1=0;
	//if (1)
	{
		{
			//			i32 i;

			m_tmpConstraintSizesPool.resizeNoInitialize(numConstraints);

			//			b3OpenCLArray<b3GpuGenericConstraint> gpuConstraints(m_gpuData->m_context,m_gpuData->m_queue);

			if (useGpuInfo1)
			{
				D3_PROFILE("info1 and init batchConstraint");

				m_gpuData->m_gpuConstraintInfo1->resize(numConstraints);

				if (1)
				{
					D3_PROFILE("getInfo1Kernel");

					b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_getInfo1Kernel, "m_getInfo1Kernel");
					launcher.setBuffer(m_gpuData->m_gpuConstraintInfo1->getBufferCL());
					launcher.setBuffer(gpuConstraints->getBufferCL());
					launcher.setConst(numConstraints);
					launcher.launch1D(numConstraints);
					clFinish(m_gpuData->m_queue);
				}

				if (m_gpuData->m_batchSizes.size() == 0)
				{
					D3_PROFILE("initBatchConstraintsKernel");

					m_gpuData->m_gpuConstraintRowOffsets->resize(numConstraints);
					u32 total = 0;
					m_gpuData->m_prefixScan->execute(*m_gpuData->m_gpuConstraintInfo1, *m_gpuData->m_gpuConstraintRowOffsets, numConstraints, &total);
					u32 lastElem = m_gpuData->m_gpuConstraintInfo1->at(numConstraints - 1);
					totalNumRows = total + lastElem;

					{
						D3_PROFILE("init batch constraints");
						b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_initBatchConstraintsKernel, "m_initBatchConstraintsKernel");
						launcher.setBuffer(m_gpuData->m_gpuConstraintInfo1->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuConstraintRowOffsets->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuBatchConstraints->getBufferCL());
						launcher.setBuffer(gpuConstraints->getBufferCL());
						launcher.setBuffer(gpuBodies->getBufferCL());
						launcher.setConst(numConstraints);
						launcher.launch1D(numConstraints);
						clFinish(m_gpuData->m_queue);
					}
					//assume the batching happens on CPU, so copy the data
					m_gpuData->m_gpuBatchConstraints->copyToHost(batchConstraints);
				}
			}
			else
			{
				totalNumRows = 0;
				gpuConstraints->copyToHost(m_gpuData->m_cpuConstraints);
				//calculate the total number of contraint rows
				for (i32 i = 0; i < numConstraints; i++)
				{
					u32& info1 = m_tmpConstraintSizesPool[i];
					//					u32 info1;
					if (m_gpuData->m_cpuConstraints[i].isEnabled())
					{
						m_gpuData->m_cpuConstraints[i].getInfo1(&info1, &m_gpuData->m_cpuBodies[0]);
					}
					else
					{
						info1 = 0;
					}

					totalNumRows += info1;
				}

				m_gpuData->m_gpuBatchConstraints->copyFromHost(batchConstraints);
				m_gpuData->m_gpuConstraintInfo1->copyFromHost(m_tmpConstraintSizesPool);
			}
			m_tmpSolverNonContactConstraintPool.resizeNoInitialize(totalNumRows);
			m_gpuData->m_gpuConstraintRows->resize(totalNumRows);

			//			b3GpuConstraintArray		verify;

			if (useGpuInfo2)
			{
				{
					D3_PROFILE("getInfo2Kernel");
					b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_getInfo2Kernel, "m_getInfo2Kernel");
					launcher.setBuffer(m_gpuData->m_gpuConstraintRows->getBufferCL());
					launcher.setBuffer(m_gpuData->m_gpuConstraintInfo1->getBufferCL());
					launcher.setBuffer(m_gpuData->m_gpuConstraintRowOffsets->getBufferCL());
					launcher.setBuffer(gpuConstraints->getBufferCL());
					launcher.setBuffer(m_gpuData->m_gpuBatchConstraints->getBufferCL());
					launcher.setBuffer(gpuBodies->getBufferCL());
					launcher.setBuffer(gpuInertias->getBufferCL());
					launcher.setBuffer(m_gpuData->m_gpuSolverBodies->getBufferCL());
					launcher.setConst(infoGlobal.m_timeStep);
					launcher.setConst(infoGlobal.m_erp);
					launcher.setConst(infoGlobal.m_globalCfm);
					launcher.setConst(infoGlobal.m_damping);
					launcher.setConst(infoGlobal.m_numIterations);
					launcher.setConst(numConstraints);
					launcher.launch1D(numConstraints);
					clFinish(m_gpuData->m_queue);

					if (m_gpuData->m_batchSizes.size() == 0)
						m_gpuData->m_gpuBatchConstraints->copyToHost(batchConstraints);
					//m_gpuData->m_gpuConstraintRows->copyToHost(verify);
					//m_gpuData->m_gpuConstraintRows->copyToHost(m_tmpSolverNonContactConstraintPool);
				}
			}
			else
			{
				gpuInertias->copyToHost(m_gpuData->m_cpuInertias);

				///setup the b3SolverConstraints

				for (i32 i = 0; i < numConstraints; i++)
				{
					i32k& info1 = m_tmpConstraintSizesPool[i];

					if (info1)
					{
						i32 constraintIndex = batchConstraints[i].m_originalConstraintIndex;
						i32 constraintRowOffset = m_gpuData->m_cpuConstraintRowOffsets[constraintIndex];

						b3GpuSolverConstraint* currentConstraintRow = &m_tmpSolverNonContactConstraintPool[constraintRowOffset];
						b3GpuGenericConstraint& constraint = m_gpuData->m_cpuConstraints[i];

						b3RigidBodyData& rbA = m_gpuData->m_cpuBodies[constraint.getRigidBodyA()];
						//b3RigidBody& rbA = constraint.getRigidBodyA();
						//				b3RigidBody& rbB = constraint.getRigidBodyB();
						b3RigidBodyData& rbB = m_gpuData->m_cpuBodies[constraint.getRigidBodyB()];

						i32 solverBodyIdA = constraint.getRigidBodyA();  //getOrInitSolverBody(constraint.getRigidBodyA(),bodies,inertias);
						i32 solverBodyIdB = constraint.getRigidBodyB();  //getOrInitSolverBody(constraint.getRigidBodyB(),bodies,inertias);

						b3GpuSolverBody* bodyAPtr = &m_tmpSolverBodyPool[solverBodyIdA];
						b3GpuSolverBody* bodyBPtr = &m_tmpSolverBodyPool[solverBodyIdB];

						if (rbA.m_invMass)
						{
							batchConstraints[i].m_bodyAPtrAndSignBit = solverBodyIdA;
						}
						else
						{
							if (!solverBodyIdA)
								m_staticIdx = 0;
							batchConstraints[i].m_bodyAPtrAndSignBit = -solverBodyIdA;
						}

						if (rbB.m_invMass)
						{
							batchConstraints[i].m_bodyBPtrAndSignBit = solverBodyIdB;
						}
						else
						{
							if (!solverBodyIdB)
								m_staticIdx = 0;
							batchConstraints[i].m_bodyBPtrAndSignBit = -solverBodyIdB;
						}

						i32 overrideNumSolverIterations = 0;  //constraint->getOverrideNumSolverIterations() > 0 ? constraint->getOverrideNumSolverIterations() : infoGlobal.m_numIterations;
						if (overrideNumSolverIterations > m_maxOverrideNumSolverIterations)
							m_maxOverrideNumSolverIterations = overrideNumSolverIterations;

						i32 j;
						for (j = 0; j < info1; j++)
						{
							memset(&currentConstraintRow[j], 0, sizeof(b3GpuSolverConstraint));
							currentConstraintRow[j].m_angularComponentA.setVal(0, 0, 0);
							currentConstraintRow[j].m_angularComponentB.setVal(0, 0, 0);
							currentConstraintRow[j].m_appliedImpulse = 0.f;
							currentConstraintRow[j].m_appliedPushImpulse = 0.f;
							currentConstraintRow[j].m_cfm = 0.f;
							currentConstraintRow[j].m_contactNormal.setVal(0, 0, 0);
							currentConstraintRow[j].m_friction = 0.f;
							currentConstraintRow[j].m_frictionIndex = 0;
							currentConstraintRow[j].m_jacDiagABInv = 0.f;
							currentConstraintRow[j].m_lowerLimit = 0.f;
							currentConstraintRow[j].m_upperLimit = 0.f;

							currentConstraintRow[j].m_originalContactPoint = 0;
							currentConstraintRow[j].m_overrideNumSolverIterations = 0;
							currentConstraintRow[j].m_relpos1CrossNormal.setVal(0, 0, 0);
							currentConstraintRow[j].m_relpos2CrossNormal.setVal(0, 0, 0);
							currentConstraintRow[j].m_rhs = 0.f;
							currentConstraintRow[j].m_rhsPenetration = 0.f;
							currentConstraintRow[j].m_solverBodyIdA = 0;
							currentConstraintRow[j].m_solverBodyIdB = 0;

							currentConstraintRow[j].m_lowerLimit = -D3_INFINITY;
							currentConstraintRow[j].m_upperLimit = D3_INFINITY;
							currentConstraintRow[j].m_appliedImpulse = 0.f;
							currentConstraintRow[j].m_appliedPushImpulse = 0.f;
							currentConstraintRow[j].m_solverBodyIdA = solverBodyIdA;
							currentConstraintRow[j].m_solverBodyIdB = solverBodyIdB;
							currentConstraintRow[j].m_overrideNumSolverIterations = overrideNumSolverIterations;
						}

						bodyAPtr->internalGetDeltaLinearVelocity().setVal(0.f, 0.f, 0.f);
						bodyAPtr->internalGetDeltaAngularVelocity().setVal(0.f, 0.f, 0.f);
						bodyAPtr->internalGetPushVelocity().setVal(0.f, 0.f, 0.f);
						bodyAPtr->internalGetTurnVelocity().setVal(0.f, 0.f, 0.f);
						bodyBPtr->internalGetDeltaLinearVelocity().setVal(0.f, 0.f, 0.f);
						bodyBPtr->internalGetDeltaAngularVelocity().setVal(0.f, 0.f, 0.f);
						bodyBPtr->internalGetPushVelocity().setVal(0.f, 0.f, 0.f);
						bodyBPtr->internalGetTurnVelocity().setVal(0.f, 0.f, 0.f);

						b3GpuConstraintInfo2 info2;
						info2.fps = 1.f / infoGlobal.m_timeStep;
						info2.erp = infoGlobal.m_erp;
						info2.m_J1linearAxis = currentConstraintRow->m_contactNormal;
						info2.m_J1angularAxis = currentConstraintRow->m_relpos1CrossNormal;
						info2.m_J2linearAxis = 0;
						info2.m_J2angularAxis = currentConstraintRow->m_relpos2CrossNormal;
						info2.rowskip = sizeof(b3GpuSolverConstraint) / sizeof(b3Scalar);  //check this
						///the size of b3GpuSolverConstraint needs be a multiple of b3Scalar
						drx3DAssert(info2.rowskip * sizeof(b3Scalar) == sizeof(b3GpuSolverConstraint));
						info2.m_constraintError = &currentConstraintRow->m_rhs;
						currentConstraintRow->m_cfm = infoGlobal.m_globalCfm;
						info2.m_damping = infoGlobal.m_damping;
						info2.cfm = &currentConstraintRow->m_cfm;
						info2.m_lowerLimit = &currentConstraintRow->m_lowerLimit;
						info2.m_upperLimit = &currentConstraintRow->m_upperLimit;
						info2.m_numIterations = infoGlobal.m_numIterations;
						m_gpuData->m_cpuConstraints[i].getInfo2(&info2, &m_gpuData->m_cpuBodies[0]);

						///finalize the constraint setup
						for (j = 0; j < info1; j++)
						{
							b3GpuSolverConstraint& solverConstraint = currentConstraintRow[j];

							if (solverConstraint.m_upperLimit >= m_gpuData->m_cpuConstraints[i].getBreakingImpulseThreshold())
							{
								solverConstraint.m_upperLimit = m_gpuData->m_cpuConstraints[i].getBreakingImpulseThreshold();
							}

							if (solverConstraint.m_lowerLimit <= -m_gpuData->m_cpuConstraints[i].getBreakingImpulseThreshold())
							{
								solverConstraint.m_lowerLimit = -m_gpuData->m_cpuConstraints[i].getBreakingImpulseThreshold();
							}

							//						solverConstraint.m_originalContactPoint = constraint;

							b3Matrix3x3& invInertiaWorldA = m_gpuData->m_cpuInertias[constraint.getRigidBodyA()].m_invInertiaWorld;
							{
								//b3Vec3 angularFactorA(1,1,1);
								const b3Vec3& ftorqueAxis1 = solverConstraint.m_relpos1CrossNormal;
								solverConstraint.m_angularComponentA = invInertiaWorldA * ftorqueAxis1;  //*angularFactorA;
							}

							b3Matrix3x3& invInertiaWorldB = m_gpuData->m_cpuInertias[constraint.getRigidBodyB()].m_invInertiaWorld;
							{
								const b3Vec3& ftorqueAxis2 = solverConstraint.m_relpos2CrossNormal;
								solverConstraint.m_angularComponentB = invInertiaWorldB * ftorqueAxis2;  //*constraint.getRigidBodyB().getAngularFactor();
							}

							{
								//it is ok to use solverConstraint.m_contactNormal instead of -solverConstraint.m_contactNormal
								//because it gets multiplied iMJlB
								b3Vec3 iMJlA = solverConstraint.m_contactNormal * rbA.m_invMass;
								b3Vec3 iMJaA = invInertiaWorldA * solverConstraint.m_relpos1CrossNormal;
								b3Vec3 iMJlB = solverConstraint.m_contactNormal * rbB.m_invMass;  //sign of normal?
								b3Vec3 iMJaB = invInertiaWorldB * solverConstraint.m_relpos2CrossNormal;

								b3Scalar sum = iMJlA.dot(solverConstraint.m_contactNormal);
								sum += iMJaA.dot(solverConstraint.m_relpos1CrossNormal);
								sum += iMJlB.dot(solverConstraint.m_contactNormal);
								sum += iMJaB.dot(solverConstraint.m_relpos2CrossNormal);
								b3Scalar fsum = b3Fabs(sum);
								drx3DAssert(fsum > D3_EPSILON);
								solverConstraint.m_jacDiagABInv = fsum > D3_EPSILON ? b3Scalar(1.) / sum : 0.f;
							}

							///fix rhs
							///todo: add force/torque accelerators
							{
								b3Scalar rel_vel;
								b3Scalar vel1Dotn = solverConstraint.m_contactNormal.dot(rbA.m_linVel) + solverConstraint.m_relpos1CrossNormal.dot(rbA.m_angVel);
								b3Scalar vel2Dotn = -solverConstraint.m_contactNormal.dot(rbB.m_linVel) + solverConstraint.m_relpos2CrossNormal.dot(rbB.m_angVel);

								rel_vel = vel1Dotn + vel2Dotn;

								b3Scalar restitution = 0.f;
								b3Scalar positionalError = solverConstraint.m_rhs;  //already filled in by getConstraintInfo2
								b3Scalar velocityError = restitution - rel_vel * info2.m_damping;
								b3Scalar penetrationImpulse = positionalError * solverConstraint.m_jacDiagABInv;
								b3Scalar velocityImpulse = velocityError * solverConstraint.m_jacDiagABInv;
								solverConstraint.m_rhs = penetrationImpulse + velocityImpulse;
								solverConstraint.m_appliedImpulse = 0.f;
							}
						}
					}
				}

				m_gpuData->m_gpuConstraintRows->copyFromHost(m_tmpSolverNonContactConstraintPool);
				m_gpuData->m_gpuConstraintInfo1->copyFromHost(m_tmpConstraintSizesPool);

				if (m_gpuData->m_batchSizes.size() == 0)
					m_gpuData->m_gpuBatchConstraints->copyFromHost(batchConstraints);
				else
					m_gpuData->m_gpuBatchConstraints->copyToHost(batchConstraints);

				m_gpuData->m_gpuSolverBodies->copyFromHost(m_tmpSolverBodyPool);

			}  //end useGpuInfo2
		}

#ifdef D3_SUPPORT_CONTACT_CONSTRAINTS
		{
			i32 i;

			for (i = 0; i < numManifolds; i++)
			{
				b3Contact4& manifold = manifoldPtr[i];
				convertContact(bodies, inertias, &manifold, infoGlobal);
			}
		}
#endif  //D3_SUPPORT_CONTACT_CONSTRAINTS
	}

	//	b3ContactSolverInfo info = infoGlobal;

	//	i32 numNonContactPool = m_tmpSolverNonContactConstraintPool.size();
	//	i32 numConstraintPool = m_tmpSolverContactConstraintPool.size();
	//	i32 numFrictionPool = m_tmpSolverContactFrictionConstraintPool.size();

	return 0.f;
}

///a straight copy from GPU/OpenCL kernel, for debugging
__inline void internalApplyImpulse(b3GpuSolverBody* body, const b3Vec3& linearComponent, const b3Vec3& angularComponent, float impulseMagnitude)
{
	body->m_deltaLinearVelocity += linearComponent * impulseMagnitude * body->m_linearFactor;
	body->m_deltaAngularVelocity += angularComponent * (impulseMagnitude * body->m_angularFactor);
}

void resolveSingleConstraintRowGeneric2(b3GpuSolverBody* body1, b3GpuSolverBody* body2, b3GpuSolverConstraint* c)
{
	float deltaImpulse = c->m_rhs - b3Scalar(c->m_appliedImpulse) * c->m_cfm;
	float deltaVel1Dotn = b3Dot(c->m_contactNormal, body1->m_deltaLinearVelocity) + b3Dot(c->m_relpos1CrossNormal, body1->m_deltaAngularVelocity);
	float deltaVel2Dotn = -b3Dot(c->m_contactNormal, body2->m_deltaLinearVelocity) + b3Dot(c->m_relpos2CrossNormal, body2->m_deltaAngularVelocity);

	deltaImpulse -= deltaVel1Dotn * c->m_jacDiagABInv;
	deltaImpulse -= deltaVel2Dotn * c->m_jacDiagABInv;

	float sum = b3Scalar(c->m_appliedImpulse) + deltaImpulse;
	if (sum < c->m_lowerLimit)
	{
		deltaImpulse = c->m_lowerLimit - b3Scalar(c->m_appliedImpulse);
		c->m_appliedImpulse = c->m_lowerLimit;
	}
	else if (sum > c->m_upperLimit)
	{
		deltaImpulse = c->m_upperLimit - b3Scalar(c->m_appliedImpulse);
		c->m_appliedImpulse = c->m_upperLimit;
	}
	else
	{
		c->m_appliedImpulse = sum;
	}

	internalApplyImpulse(body1, c->m_contactNormal * body1->m_invMass, c->m_angularComponentA, deltaImpulse);
	internalApplyImpulse(body2, -c->m_contactNormal * body2->m_invMass, c->m_angularComponentB, deltaImpulse);
}

void b3GpuPgsConstraintSolver::initSolverBody(i32 bodyIndex, b3GpuSolverBody* solverBody, b3RigidBodyData* rb)
{
	solverBody->m_deltaLinearVelocity.setVal(0.f, 0.f, 0.f);
	solverBody->m_deltaAngularVelocity.setVal(0.f, 0.f, 0.f);
	solverBody->internalGetPushVelocity().setVal(0.f, 0.f, 0.f);
	solverBody->internalGetTurnVelocity().setVal(0.f, 0.f, 0.f);

	drx3DAssert(rb);
	//	solverBody->m_worldTransform = getWorldTransform(rb);
	solverBody->internalSetInvMass(b3MakeVector3(rb->m_invMass, rb->m_invMass, rb->m_invMass));
	solverBody->m_originalBodyIndex = bodyIndex;
	solverBody->m_angularFactor = b3MakeVector3(1, 1, 1);
	solverBody->m_linearFactor = b3MakeVector3(1, 1, 1);
	solverBody->m_linearVelocity = getLinearVelocity(rb);
	solverBody->m_angularVelocity = getAngularVelocity(rb);
}

void b3GpuPgsConstraintSolver::averageVelocities()
{
}

b3Scalar b3GpuPgsConstraintSolver::solveGroupCacheFriendlyIterations(b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints1, i32 numConstraints, const b3ContactSolverInfo& infoGlobal)
{
	//only create the batches once.
	//@todo: incrementally update batches when constraints are added/activated and/or removed/deactivated
	D3_PROFILE("GpuSolveGroupCacheFriendlyIterations");

	bool createBatches = m_gpuData->m_batchSizes.size() == 0;
	{
		if (createBatches)
		{
			m_gpuData->m_batchSizes.resize(0);

			{
				m_gpuData->m_gpuBatchConstraints->copyToHost(batchConstraints);

				D3_PROFILE("batch joints");
				drx3DAssert(batchConstraints.size() == numConstraints);
				i32 simdWidth = numConstraints + 1;
				i32 numBodies = m_tmpSolverBodyPool.size();
				sortConstraintByBatch3(&batchConstraints[0], numConstraints, simdWidth, m_staticIdx, numBodies);

				m_gpuData->m_gpuBatchConstraints->copyFromHost(batchConstraints);
			}
		}
		else
		{
			/*b3AlignedObjectArray<b3BatchConstraint> cpuCheckBatches;
			m_gpuData->m_gpuBatchConstraints->copyToHost(cpuCheckBatches);
			drx3DAssert(cpuCheckBatches.size()==batchConstraints.size());
			printf(".\n");
			*/
			//>copyFromHost(batchConstraints);
		}
		i32 maxIterations = infoGlobal.m_numIterations;

		bool useBatching = true;

		if (useBatching)
		{
			if (!useGpuSolveJointConstraintRows)
			{
				D3_PROFILE("copy to host");
				m_gpuData->m_gpuSolverBodies->copyToHost(m_tmpSolverBodyPool);
				m_gpuData->m_gpuBatchConstraints->copyToHost(batchConstraints);
				m_gpuData->m_gpuConstraintRows->copyToHost(m_tmpSolverNonContactConstraintPool);
				m_gpuData->m_gpuConstraintInfo1->copyToHost(m_gpuData->m_cpuConstraintInfo1);
				m_gpuData->m_gpuConstraintRowOffsets->copyToHost(m_gpuData->m_cpuConstraintRowOffsets);
				gpuConstraints1->copyToHost(m_gpuData->m_cpuConstraints);
			}

			for (i32 iteration = 0; iteration < maxIterations; iteration++)
			{
				i32 batchOffset = 0;
				i32 constraintOffset = 0;
				i32 numBatches = m_gpuData->m_batchSizes.size();
				for (i32 bb = 0; bb < numBatches; bb++)
				{
					i32 numConstraintsInBatch = m_gpuData->m_batchSizes[bb];

					if (useGpuSolveJointConstraintRows)
					{
						D3_PROFILE("solveJointConstraintRowsKernels");

						/*
						__kernel void solveJointConstraintRows(__global b3GpuSolverBody* solverBodies,
					  __global b3BatchConstraint* batchConstraints,
					  	__global b3SolverConstraint* rows,
						__global u32* numConstraintRowsInfo1, 
						__global u32* rowOffsets,
						__global b3GpuGenericConstraint* constraints,
						i32 batchOffset,
						i32 numConstraintsInBatch*/

						b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_solveJointConstraintRowsKernels, "m_solveJointConstraintRowsKernels");
						launcher.setBuffer(m_gpuData->m_gpuSolverBodies->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuBatchConstraints->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuConstraintRows->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuConstraintInfo1->getBufferCL());
						launcher.setBuffer(m_gpuData->m_gpuConstraintRowOffsets->getBufferCL());
						launcher.setBuffer(gpuConstraints1->getBufferCL());  //to detect disabled constraints
						launcher.setConst(batchOffset);
						launcher.setConst(numConstraintsInBatch);

						launcher.launch1D(numConstraintsInBatch);
					}
					else  //useGpu
					{
						for (i32 b = 0; b < numConstraintsInBatch; b++)
						{
							const b3BatchConstraint& c = batchConstraints[batchOffset + b];
							/*printf("-----------\n");
							printf("bb=%d\n",bb);
							printf("c.batchId = %d\n", c.m_batchId);
							*/
							drx3DAssert(c.m_batchId == bb);
							b3GpuGenericConstraint* constraint = &m_gpuData->m_cpuConstraints[c.m_originalConstraintIndex];
							if (constraint->m_flags & D3_CONSTRAINT_FLAG_ENABLED)
							{
								i32 numConstraintRows = m_gpuData->m_cpuConstraintInfo1[c.m_originalConstraintIndex];
								i32 constraintOffset = m_gpuData->m_cpuConstraintRowOffsets[c.m_originalConstraintIndex];

								for (i32 jj = 0; jj < numConstraintRows; jj++)
								{
									//
									b3GpuSolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[constraintOffset + jj];
									//resolveSingleConstraintRowGenericSIMD(m_tmpSolverBodyPool[constraint.m_solverBodyIdA],m_tmpSolverBodyPool[constraint.m_solverBodyIdB],constraint);
									resolveSingleConstraintRowGeneric2(&m_tmpSolverBodyPool[constraint.m_solverBodyIdA], &m_tmpSolverBodyPool[constraint.m_solverBodyIdB], &constraint);
								}
							}
						}
					}  //useGpu
					batchOffset += numConstraintsInBatch;
					constraintOffset += numConstraintsInBatch;
				}
			}  //for (i32 iteration...

			if (!useGpuSolveJointConstraintRows)
			{
				{
					D3_PROFILE("copy from host");
					m_gpuData->m_gpuSolverBodies->copyFromHost(m_tmpSolverBodyPool);
					m_gpuData->m_gpuBatchConstraints->copyFromHost(batchConstraints);
					m_gpuData->m_gpuConstraintRows->copyFromHost(m_tmpSolverNonContactConstraintPool);
				}

				//D3_PROFILE("copy to host");
				//m_gpuData->m_gpuSolverBodies->copyToHost(m_tmpSolverBodyPool);
			}
			//i32 sz = sizeof(b3GpuSolverBody);
			//printf("cpu sizeof(b3GpuSolverBody)=%d\n",sz);
		}
		else
		{
			for (i32 iteration = 0; iteration < maxIterations; iteration++)
			{
				i32 numJoints = m_tmpSolverNonContactConstraintPool.size();
				for (i32 j = 0; j < numJoints; j++)
				{
					b3GpuSolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[j];
					resolveSingleConstraintRowGeneric2(&m_tmpSolverBodyPool[constraint.m_solverBodyIdA], &m_tmpSolverBodyPool[constraint.m_solverBodyIdB], &constraint);
				}

				if (!m_usePgs)
				{
					averageVelocities();
				}
			}
		}
	}
	clFinish(m_gpuData->m_queue);
	return 0.f;
}

static b3AlignedObjectArray<i32> bodyUsed;
static b3AlignedObjectArray<i32> curUsed;

inline i32 b3GpuPgsConstraintSolver::sortConstraintByBatch3(b3BatchConstraint* cs, i32 numConstraints, i32 simdWidth, i32 staticIdx, i32 numBodies)
{
	//i32 sz = sizeof(b3BatchConstraint);

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

#if defined(_DEBUG)
	for (i32 i = 0; i < numConstraints; i++)
		cs[i].m_batchId = -1;
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

					cs[idx].m_batchId = batchIdx;

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
							nCurrentBatch = 0;
							for (i32 i = 0; i < curBodyUsed; i++)
								bodyUsed[curUsed[i] / 32] = 0;
							curBodyUsed = 0;
						}
					}
				}
			}
			m_gpuData->m_batchSizes.push_back(nCurrentBatch);
			batchIdx++;
		}
	}

#if defined(_DEBUG)
	//		debugPrintf( "nBatches: %d\n", batchIdx );
	for (i32 i = 0; i < numConstraints; i++)
	{
		drx3DAssert(cs[i].m_batchId != -1);
	}
#endif

	if (maxSwaps < numSwaps)
	{
		maxSwaps = numSwaps;
		//printf("maxSwaps = %d\n", maxSwaps);
	}

	return batchIdx;
}

/// b3PgsJacobiSolver Sequentially applies impulses
b3Scalar b3GpuPgsConstraintSolver::solveGroup(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias,
											  i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal)
{
	D3_PROFILE("solveJoints");
	//you need to provide at least some bodies

	solveGroupCacheFriendlySetup(gpuBodies, gpuInertias, numBodies, gpuConstraints, numConstraints, infoGlobal);

	solveGroupCacheFriendlyIterations(gpuConstraints, numConstraints, infoGlobal);

	solveGroupCacheFriendlyFinish(gpuBodies, gpuInertias, numBodies, gpuConstraints, numConstraints, infoGlobal);

	return 0.f;
}

void b3GpuPgsConstraintSolver::solveJoints(i32 numBodies, b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias,
										   i32 numConstraints, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints)
{
	b3ContactSolverInfo infoGlobal;
	infoGlobal.m_splitImpulse = false;
	infoGlobal.m_timeStep = 1.f / 60.f;
	infoGlobal.m_numIterations = 4;  //4;
									 //	infoGlobal.m_solverMode|=D3_SOLVER_USE_2_FRICTION_DIRECTIONS|D3_SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS|D3_SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION;
	//infoGlobal.m_solverMode|=D3_SOLVER_USE_2_FRICTION_DIRECTIONS|D3_SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS;
	infoGlobal.m_solverMode |= D3_SOLVER_USE_2_FRICTION_DIRECTIONS;

	//if (infoGlobal.m_solverMode & D3_SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS)
	//if ((infoGlobal.m_solverMode & D3_SOLVER_USE_2_FRICTION_DIRECTIONS) && (infoGlobal.m_solverMode & D3_SOLVER_DISABLE_VELOCITY_DEPENDENT_FRICTION_DIRECTION))

	solveGroup(gpuBodies, gpuInertias, numBodies, gpuConstraints, numConstraints, infoGlobal);
}

//b3AlignedObjectArray<b3RigidBodyData> testBodies;

b3Scalar b3GpuPgsConstraintSolver::solveGroupCacheFriendlyFinish(b3OpenCLArray<b3RigidBodyData>* gpuBodies, b3OpenCLArray<b3InertiaData>* gpuInertias, i32 numBodies, b3OpenCLArray<b3GpuGenericConstraint>* gpuConstraints, i32 numConstraints, const b3ContactSolverInfo& infoGlobal)
{
	D3_PROFILE("solveGroupCacheFriendlyFinish");
	//	i32 numPoolConstraints = m_tmpSolverContactConstraintPool.size();
	//	i32 i,j;

	{
		if (gpuBreakConstraints)
		{
			D3_PROFILE("breakViolatedConstraintsKernel");
			b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_breakViolatedConstraintsKernel, "m_breakViolatedConstraintsKernel");
			launcher.setBuffer(gpuConstraints->getBufferCL());
			launcher.setBuffer(m_gpuData->m_gpuConstraintInfo1->getBufferCL());
			launcher.setBuffer(m_gpuData->m_gpuConstraintRowOffsets->getBufferCL());
			launcher.setBuffer(m_gpuData->m_gpuConstraintRows->getBufferCL());
			launcher.setConst(numConstraints);
			launcher.launch1D(numConstraints);
		}
		else
		{
			gpuConstraints->copyToHost(m_gpuData->m_cpuConstraints);
			m_gpuData->m_gpuBatchConstraints->copyToHost(m_gpuData->m_cpuBatchConstraints);
			m_gpuData->m_gpuConstraintRows->copyToHost(m_gpuData->m_cpuConstraintRows);
			gpuConstraints->copyToHost(m_gpuData->m_cpuConstraints);
			m_gpuData->m_gpuConstraintInfo1->copyToHost(m_gpuData->m_cpuConstraintInfo1);
			m_gpuData->m_gpuConstraintRowOffsets->copyToHost(m_gpuData->m_cpuConstraintRowOffsets);

			for (i32 cid = 0; cid < numConstraints; cid++)
			{
				i32 originalConstraintIndex = batchConstraints[cid].m_originalConstraintIndex;
				i32 constraintRowOffset = m_gpuData->m_cpuConstraintRowOffsets[originalConstraintIndex];
				i32 numRows = m_gpuData->m_cpuConstraintInfo1[originalConstraintIndex];
				if (numRows)
				{
					//	printf("cid=%d, breakingThreshold =%f\n",cid,breakingThreshold);
					for (i32 i = 0; i < numRows; i++)
					{
						i32 rowIndex = constraintRowOffset + i;
						i32 orgConstraintIndex = m_gpuData->m_cpuConstraintRows[rowIndex].m_originalConstraintIndex;
						float breakingThreshold = m_gpuData->m_cpuConstraints[orgConstraintIndex].m_breakingImpulseThreshold;
						//	printf("rows[%d].m_appliedImpulse=%f\n",rowIndex,rows[rowIndex].m_appliedImpulse);
						if (b3Fabs(m_gpuData->m_cpuConstraintRows[rowIndex].m_appliedImpulse) >= breakingThreshold)
						{
							m_gpuData->m_cpuConstraints[orgConstraintIndex].m_flags = 0;  //&= ~D3_CONSTRAINT_FLAG_ENABLED;
						}
					}
				}
			}

			gpuConstraints->copyFromHost(m_gpuData->m_cpuConstraints);
		}
	}

	{
		if (useGpuWriteBackVelocities)
		{
			D3_PROFILE("GPU write back velocities and transforms");

			b3LauncherCL launcher(m_gpuData->m_queue, m_gpuData->m_writeBackVelocitiesKernel, "m_writeBackVelocitiesKernel");
			launcher.setBuffer(gpuBodies->getBufferCL());
			launcher.setBuffer(m_gpuData->m_gpuSolverBodies->getBufferCL());
			launcher.setConst(numBodies);
			launcher.launch1D(numBodies);
			clFinish(m_gpuData->m_queue);
			//			m_gpuData->m_gpuSolverBodies->copyToHost(m_tmpSolverBodyPool);
			//			m_gpuData->m_gpuBodies->copyToHostPointer(bodies,numBodies);
			//m_gpuData->m_gpuBodies->copyToHost(testBodies);
		}
		else
		{
			D3_PROFILE("CPU write back velocities and transforms");

			m_gpuData->m_gpuSolverBodies->copyToHost(m_tmpSolverBodyPool);
			gpuBodies->copyToHost(m_gpuData->m_cpuBodies);
			for (i32 i = 0; i < m_tmpSolverBodyPool.size(); i++)
			{
				i32 bodyIndex = m_tmpSolverBodyPool[i].m_originalBodyIndex;
				//printf("bodyIndex=%d\n",bodyIndex);
				drx3DAssert(i == bodyIndex);

				b3RigidBodyData* body = &m_gpuData->m_cpuBodies[bodyIndex];
				if (body->m_invMass)
				{
					if (infoGlobal.m_splitImpulse)
						m_tmpSolverBodyPool[i].writebackVelocityAndTransform(infoGlobal.m_timeStep, infoGlobal.m_splitImpulseTurnErp);
					else
						m_tmpSolverBodyPool[i].writebackVelocity();

					if (m_usePgs)
					{
						body->m_linVel = m_tmpSolverBodyPool[i].m_linearVelocity;
						body->m_angVel = m_tmpSolverBodyPool[i].m_angularVelocity;
					}
					else
					{
						drx3DAssert(0);
					}
					/*			
					if (infoGlobal.m_splitImpulse)
					{
						body->m_pos = m_tmpSolverBodyPool[i].m_worldTransform.getOrigin();
						b3Quat orn;
						orn = m_tmpSolverBodyPool[i].m_worldTransform.getRotation();
						body->m_quat = orn;
					}
					*/
				}
			}  //for

			gpuBodies->copyFromHost(m_gpuData->m_cpuBodies);
		}
	}

	clFinish(m_gpuData->m_queue);

	m_tmpSolverContactConstraintPool.resizeNoInitialize(0);
	m_tmpSolverNonContactConstraintPool.resizeNoInitialize(0);
	m_tmpSolverContactFrictionConstraintPool.resizeNoInitialize(0);
	m_tmpSolverContactRollingFrictionConstraintPool.resizeNoInitialize(0);

	m_tmpSolverBodyPool.resizeNoInitialize(0);
	return 0.f;
}
