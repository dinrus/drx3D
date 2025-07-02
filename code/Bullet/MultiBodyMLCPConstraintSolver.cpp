#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyMLCPConstraintSolver.h>

#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolverInterface.h>

#define DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS

static bool interleaveContactAndFriction1 = false;

struct JointNode1
{
	i32 jointIndex;          // pointer to enclosing dxJoint object
	i32 otherBodyIndex;      // *other* body this joint is connected to
	i32 nextJointNodeIndex;  //-1 for null
	i32 constraintRowIndex;
};

// Helper function to compute a delta velocity in the constraint space.
static Scalar computeDeltaVelocityInConstraintSpace(
	const Vec3& angularDeltaVelocity,
	const Vec3& contactNormal,
	Scalar invMass,
	const Vec3& angularJacobian,
	const Vec3& linearJacobian)
{
	return angularDeltaVelocity.dot(angularJacobian) + contactNormal.dot(linearJacobian) * invMass;
}

// Faster version of computeDeltaVelocityInConstraintSpace that can be used when contactNormal and linearJacobian are
// identical.
static Scalar computeDeltaVelocityInConstraintSpace(
	const Vec3& angularDeltaVelocity,
	Scalar invMass,
	const Vec3& angularJacobian)
{
	return angularDeltaVelocity.dot(angularJacobian) + invMass;
}

// Helper function to compute a delta velocity in the constraint space.
static Scalar computeDeltaVelocityInConstraintSpace(const Scalar* deltaVelocity, const Scalar* jacobian, i32 size)
{
	Scalar result = 0;
	for (i32 i = 0; i < size; ++i)
		result += deltaVelocity[i] * jacobian[i];

	return result;
}

static Scalar computeConstraintMatrixDiagElementMultiBody(
	const AlignedObjectArray<SolverBody>& solverBodyPool,
	const MultiBodyJacobianData& data,
	const MultiBodySolverConstraint& constraint)
{
	Scalar ret = 0;

	const MultiBody* multiBodyA = constraint.m_multiBodyA;
	const MultiBody* multiBodyB = constraint.m_multiBodyB;

	if (multiBodyA)
	{
		const Scalar* jacA = &data.m_jacobians[constraint.m_jacAindex];
		const Scalar* deltaA = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacAindex];
		i32k ndofA = multiBodyA->getNumDofs() + 6;
		ret += computeDeltaVelocityInConstraintSpace(deltaA, jacA, ndofA);
	}
	else
	{
		i32k solverBodyIdA = constraint.m_solverBodyIdA;
		Assert(solverBodyIdA != -1);
		const SolverBody* solverBodyA = &solverBodyPool[solverBodyIdA];
		const Scalar invMassA = solverBodyA->m_originalBody ? solverBodyA->m_originalBody->getInvMass() : 0.0;
		ret += computeDeltaVelocityInConstraintSpace(
			constraint.m_relpos1CrossNormal,
			invMassA,
			constraint.m_angularComponentA);
	}

	if (multiBodyB)
	{
		const Scalar* jacB = &data.m_jacobians[constraint.m_jacBindex];
		const Scalar* deltaB = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacBindex];
		i32k ndofB = multiBodyB->getNumDofs() + 6;
		ret += computeDeltaVelocityInConstraintSpace(deltaB, jacB, ndofB);
	}
	else
	{
		i32k solverBodyIdB = constraint.m_solverBodyIdB;
		Assert(solverBodyIdB != -1);
		const SolverBody* solverBodyB = &solverBodyPool[solverBodyIdB];
		const Scalar invMassB = solverBodyB->m_originalBody ? solverBodyB->m_originalBody->getInvMass() : 0.0;
		ret += computeDeltaVelocityInConstraintSpace(
			constraint.m_relpos2CrossNormal,
			invMassB,
			constraint.m_angularComponentB);
	}

	return ret;
}

static Scalar computeConstraintMatrixOffDiagElementMultiBody(
	const AlignedObjectArray<SolverBody>& solverBodyPool,
	const MultiBodyJacobianData& data,
	const MultiBodySolverConstraint& constraint,
	const MultiBodySolverConstraint& offDiagConstraint)
{
	Scalar offDiagA = Scalar(0);

	const MultiBody* multiBodyA = constraint.m_multiBodyA;
	const MultiBody* multiBodyB = constraint.m_multiBodyB;
	const MultiBody* offDiagMultiBodyA = offDiagConstraint.m_multiBodyA;
	const MultiBody* offDiagMultiBodyB = offDiagConstraint.m_multiBodyB;

	// Assumed at least one system is multibody
	Assert(multiBodyA || multiBodyB);
	Assert(offDiagMultiBodyA || offDiagMultiBodyB);

	if (offDiagMultiBodyA)
	{
		const Scalar* offDiagJacA = &data.m_jacobians[offDiagConstraint.m_jacAindex];

		if (offDiagMultiBodyA == multiBodyA)
		{
			i32k ndofA = multiBodyA->getNumDofs() + 6;
			const Scalar* deltaA = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacAindex];
			offDiagA += computeDeltaVelocityInConstraintSpace(deltaA, offDiagJacA, ndofA);
		}
		else if (offDiagMultiBodyA == multiBodyB)
		{
			i32k ndofB = multiBodyB->getNumDofs() + 6;
			const Scalar* deltaB = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacBindex];
			offDiagA += computeDeltaVelocityInConstraintSpace(deltaB, offDiagJacA, ndofB);
		}
	}
	else
	{
		i32k solverBodyIdA = constraint.m_solverBodyIdA;
		i32k solverBodyIdB = constraint.m_solverBodyIdB;

		i32k offDiagSolverBodyIdA = offDiagConstraint.m_solverBodyIdA;
		Assert(offDiagSolverBodyIdA != -1);

		if (offDiagSolverBodyIdA == solverBodyIdA)
		{
			Assert(solverBodyIdA != -1);
			const SolverBody* solverBodyA = &solverBodyPool[solverBodyIdA];
			const Scalar invMassA = solverBodyA->m_originalBody ? solverBodyA->m_originalBody->getInvMass() : 0.0;
			offDiagA += computeDeltaVelocityInConstraintSpace(
				offDiagConstraint.m_relpos1CrossNormal,
				offDiagConstraint.m_contactNormal1,
				invMassA, constraint.m_angularComponentA,
				constraint.m_contactNormal1);
		}
		else if (offDiagSolverBodyIdA == solverBodyIdB)
		{
			Assert(solverBodyIdB != -1);
			const SolverBody* solverBodyB = &solverBodyPool[solverBodyIdB];
			const Scalar invMassB = solverBodyB->m_originalBody ? solverBodyB->m_originalBody->getInvMass() : 0.0;
			offDiagA += computeDeltaVelocityInConstraintSpace(
				offDiagConstraint.m_relpos1CrossNormal,
				offDiagConstraint.m_contactNormal1,
				invMassB,
				constraint.m_angularComponentB,
				constraint.m_contactNormal2);
		}
	}

	if (offDiagMultiBodyB)
	{
		const Scalar* offDiagJacB = &data.m_jacobians[offDiagConstraint.m_jacBindex];

		if (offDiagMultiBodyB == multiBodyA)
		{
			i32k ndofA = multiBodyA->getNumDofs() + 6;
			const Scalar* deltaA = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacAindex];
			offDiagA += computeDeltaVelocityInConstraintSpace(deltaA, offDiagJacB, ndofA);
		}
		else if (offDiagMultiBodyB == multiBodyB)
		{
			i32k ndofB = multiBodyB->getNumDofs() + 6;
			const Scalar* deltaB = &data.m_deltaVelocitiesUnitImpulse[constraint.m_jacBindex];
			offDiagA += computeDeltaVelocityInConstraintSpace(deltaB, offDiagJacB, ndofB);
		}
	}
	else
	{
		i32k solverBodyIdA = constraint.m_solverBodyIdA;
		i32k solverBodyIdB = constraint.m_solverBodyIdB;

		i32k offDiagSolverBodyIdB = offDiagConstraint.m_solverBodyIdB;
		Assert(offDiagSolverBodyIdB != -1);

		if (offDiagSolverBodyIdB == solverBodyIdA)
		{
			Assert(solverBodyIdA != -1);
			const SolverBody* solverBodyA = &solverBodyPool[solverBodyIdA];
			const Scalar invMassA = solverBodyA->m_originalBody ? solverBodyA->m_originalBody->getInvMass() : 0.0;
			offDiagA += computeDeltaVelocityInConstraintSpace(
				offDiagConstraint.m_relpos2CrossNormal,
				offDiagConstraint.m_contactNormal2,
				invMassA, constraint.m_angularComponentA,
				constraint.m_contactNormal1);
		}
		else if (offDiagSolverBodyIdB == solverBodyIdB)
		{
			Assert(solverBodyIdB != -1);
			const SolverBody* solverBodyB = &solverBodyPool[solverBodyIdB];
			const Scalar invMassB = solverBodyB->m_originalBody ? solverBodyB->m_originalBody->getInvMass() : 0.0;
			offDiagA += computeDeltaVelocityInConstraintSpace(
				offDiagConstraint.m_relpos2CrossNormal,
				offDiagConstraint.m_contactNormal2,
				invMassB, constraint.m_angularComponentB,
				constraint.m_contactNormal2);
		}
	}

	return offDiagA;
}

void MultiBodyMLCPConstraintSolver::createMLCPFast(const ContactSolverInfo& infoGlobal)
{
	createMLCPFastRigidBody(infoGlobal);
	createMLCPFastMultiBody(infoGlobal);
}

void MultiBodyMLCPConstraintSolver::createMLCPFastRigidBody(const ContactSolverInfo& infoGlobal)
{
	i32 numContactRows = interleaveContactAndFriction1 ? 3 : 1;

	i32 numConstraintRows = m_allConstraintPtrArray.size();

	if (numConstraintRows == 0)
		return;

	i32 n = numConstraintRows;
	{
		DRX3D_PROFILE("init b (rhs)");
		m_b.resize(numConstraintRows);
		m_bSplit.resize(numConstraintRows);
		m_b.setZero();
		m_bSplit.setZero();
		for (i32 i = 0; i < numConstraintRows; i++)
		{
			Scalar jacDiag = m_allConstraintPtrArray[i]->m_jacDiagABInv;
			if (!FuzzyZero(jacDiag))
			{
				Scalar rhs = m_allConstraintPtrArray[i]->m_rhs;
				Scalar rhsPenetration = m_allConstraintPtrArray[i]->m_rhsPenetration;
				m_b[i] = rhs / jacDiag;
				m_bSplit[i] = rhsPenetration / jacDiag;
			}
		}
	}

	//	Scalar* w = 0;
	//	i32 nub = 0;

	m_lo.resize(numConstraintRows);
	m_hi.resize(numConstraintRows);

	{
		DRX3D_PROFILE("init lo/ho");

		for (i32 i = 0; i < numConstraintRows; i++)
		{
			if (0)  //m_limitDependencies[i]>=0)
			{
				m_lo[i] = -DRX3D_INFINITY;
				m_hi[i] = DRX3D_INFINITY;
			}
			else
			{
				m_lo[i] = m_allConstraintPtrArray[i]->m_lowerLimit;
				m_hi[i] = m_allConstraintPtrArray[i]->m_upperLimit;
			}
		}
	}

	//
	i32 m = m_allConstraintPtrArray.size();

	i32 numBodies = m_tmpSolverBodyPool.size();
	AlignedObjectArray<i32> bodyJointNodeArray;
	{
		DRX3D_PROFILE("bodyJointNodeArray.resize");
		bodyJointNodeArray.resize(numBodies, -1);
	}
	AlignedObjectArray<JointNode1> jointNodeArray;
	{
		DRX3D_PROFILE("jointNodeArray.reserve");
		jointNodeArray.reserve(2 * m_allConstraintPtrArray.size());
	}

	MatrixXu& J3 = m_scratchJ3;
	{
		DRX3D_PROFILE("J3.resize");
		J3.resize(2 * m, 8);
	}
	MatrixXu& JinvM3 = m_scratchJInvM3;
	{
		DRX3D_PROFILE("JinvM3.resize/setZero");

		JinvM3.resize(2 * m, 8);
		JinvM3.setZero();
		J3.setZero();
	}
	i32 cur = 0;
	i32 rowOffset = 0;
	AlignedObjectArray<i32>& ofs = m_scratchOfs;
	{
		DRX3D_PROFILE("ofs resize");
		ofs.resize(0);
		ofs.resizeNoInitialize(m_allConstraintPtrArray.size());
	}
	{
		DRX3D_PROFILE("Compute J and JinvM");
		i32 c = 0;

		i32 numRows = 0;

		for (i32 i = 0; i < m_allConstraintPtrArray.size(); i += numRows, c++)
		{
			ofs[c] = rowOffset;
			i32 sbA = m_allConstraintPtrArray[i]->m_solverBodyIdA;
			i32 sbB = m_allConstraintPtrArray[i]->m_solverBodyIdB;
			RigidBody* orgBodyA = m_tmpSolverBodyPool[sbA].m_originalBody;
			RigidBody* orgBodyB = m_tmpSolverBodyPool[sbB].m_originalBody;

			numRows = i < m_tmpSolverNonContactConstraintPool.size() ? m_tmpConstraintSizesPool[c].m_numConstraintRows : numContactRows;
			if (orgBodyA)
			{
				{
					i32 slotA = -1;
					//find free jointNode slot for sbA
					slotA = jointNodeArray.size();
					jointNodeArray.expand();  //NonInitializing();
					i32 prevSlot = bodyJointNodeArray[sbA];
					bodyJointNodeArray[sbA] = slotA;
					jointNodeArray[slotA].nextJointNodeIndex = prevSlot;
					jointNodeArray[slotA].jointIndex = c;
					jointNodeArray[slotA].constraintRowIndex = i;
					jointNodeArray[slotA].otherBodyIndex = orgBodyB ? sbB : -1;
				}
				for (i32 row = 0; row < numRows; row++, cur++)
				{
					Vec3 normalInvMass = m_allConstraintPtrArray[i + row]->m_contactNormal1 * orgBodyA->getInvMass();
					Vec3 relPosCrossNormalInvInertia = m_allConstraintPtrArray[i + row]->m_relpos1CrossNormal * orgBodyA->getInvInertiaTensorWorld();

					for (i32 r = 0; r < 3; r++)
					{
						J3.setElem(cur, r, m_allConstraintPtrArray[i + row]->m_contactNormal1[r]);
						J3.setElem(cur, r + 4, m_allConstraintPtrArray[i + row]->m_relpos1CrossNormal[r]);
						JinvM3.setElem(cur, r, normalInvMass[r]);
						JinvM3.setElem(cur, r + 4, relPosCrossNormalInvInertia[r]);
					}
					J3.setElem(cur, 3, 0);
					JinvM3.setElem(cur, 3, 0);
					J3.setElem(cur, 7, 0);
					JinvM3.setElem(cur, 7, 0);
				}
			}
			else
			{
				cur += numRows;
			}
			if (orgBodyB)
			{
				{
					i32 slotB = -1;
					//find free jointNode slot for sbA
					slotB = jointNodeArray.size();
					jointNodeArray.expand();  //NonInitializing();
					i32 prevSlot = bodyJointNodeArray[sbB];
					bodyJointNodeArray[sbB] = slotB;
					jointNodeArray[slotB].nextJointNodeIndex = prevSlot;
					jointNodeArray[slotB].jointIndex = c;
					jointNodeArray[slotB].otherBodyIndex = orgBodyA ? sbA : -1;
					jointNodeArray[slotB].constraintRowIndex = i;
				}

				for (i32 row = 0; row < numRows; row++, cur++)
				{
					Vec3 normalInvMassB = m_allConstraintPtrArray[i + row]->m_contactNormal2 * orgBodyB->getInvMass();
					Vec3 relPosInvInertiaB = m_allConstraintPtrArray[i + row]->m_relpos2CrossNormal * orgBodyB->getInvInertiaTensorWorld();

					for (i32 r = 0; r < 3; r++)
					{
						J3.setElem(cur, r, m_allConstraintPtrArray[i + row]->m_contactNormal2[r]);
						J3.setElem(cur, r + 4, m_allConstraintPtrArray[i + row]->m_relpos2CrossNormal[r]);
						JinvM3.setElem(cur, r, normalInvMassB[r]);
						JinvM3.setElem(cur, r + 4, relPosInvInertiaB[r]);
					}
					J3.setElem(cur, 3, 0);
					JinvM3.setElem(cur, 3, 0);
					J3.setElem(cur, 7, 0);
					JinvM3.setElem(cur, 7, 0);
				}
			}
			else
			{
				cur += numRows;
			}
			rowOffset += numRows;
		}
	}

	//compute JinvM = J*invM.
	const Scalar* JinvM = JinvM3.getBufferPointer();

	const Scalar* Jptr = J3.getBufferPointer();
	{
		DRX3D_PROFILE("m_A.resize");
		m_A.resize(n, n);
	}

	{
		DRX3D_PROFILE("m_A.setZero");
		m_A.setZero();
	}
	i32 c = 0;
	{
		i32 numRows = 0;
		DRX3D_PROFILE("Compute A");
		for (i32 i = 0; i < m_allConstraintPtrArray.size(); i += numRows, c++)
		{
			i32 row__ = ofs[c];
			i32 sbA = m_allConstraintPtrArray[i]->m_solverBodyIdA;
			i32 sbB = m_allConstraintPtrArray[i]->m_solverBodyIdB;
			//	RigidBody* orgBodyA = m_tmpSolverBodyPool[sbA].m_originalBody;
			//	RigidBody* orgBodyB = m_tmpSolverBodyPool[sbB].m_originalBody;

			numRows = i < m_tmpSolverNonContactConstraintPool.size() ? m_tmpConstraintSizesPool[c].m_numConstraintRows : numContactRows;

			const Scalar* JinvMrow = JinvM + 2 * 8 * (size_t)row__;

			{
				i32 startJointNodeA = bodyJointNodeArray[sbA];
				while (startJointNodeA >= 0)
				{
					i32 j0 = jointNodeArray[startJointNodeA].jointIndex;
					i32 cr0 = jointNodeArray[startJointNodeA].constraintRowIndex;
					if (j0 < c)
					{
						i32 numRowsOther = cr0 < m_tmpSolverNonContactConstraintPool.size() ? m_tmpConstraintSizesPool[j0].m_numConstraintRows : numContactRows;
						size_t ofsother = (m_allConstraintPtrArray[cr0]->m_solverBodyIdB == sbA) ? 8 * numRowsOther : 0;
						//printf("%d joint i %d and j0: %d: ",count++,i,j0);
						m_A.multiplyAdd2_p8r(JinvMrow,
											 Jptr + 2 * 8 * (size_t)ofs[j0] + ofsother, numRows, numRowsOther, row__, ofs[j0]);
					}
					startJointNodeA = jointNodeArray[startJointNodeA].nextJointNodeIndex;
				}
			}

			{
				i32 startJointNodeB = bodyJointNodeArray[sbB];
				while (startJointNodeB >= 0)
				{
					i32 j1 = jointNodeArray[startJointNodeB].jointIndex;
					i32 cj1 = jointNodeArray[startJointNodeB].constraintRowIndex;

					if (j1 < c)
					{
						i32 numRowsOther = cj1 < m_tmpSolverNonContactConstraintPool.size() ? m_tmpConstraintSizesPool[j1].m_numConstraintRows : numContactRows;
						size_t ofsother = (m_allConstraintPtrArray[cj1]->m_solverBodyIdB == sbB) ? 8 * numRowsOther : 0;
						m_A.multiplyAdd2_p8r(JinvMrow + 8 * (size_t)numRows,
											 Jptr + 2 * 8 * (size_t)ofs[j1] + ofsother, numRows, numRowsOther, row__, ofs[j1]);
					}
					startJointNodeB = jointNodeArray[startJointNodeB].nextJointNodeIndex;
				}
			}
		}

		{
			DRX3D_PROFILE("compute diagonal");
			// compute diagonal blocks of m_A

			i32 row__ = 0;
			i32 numJointRows = m_allConstraintPtrArray.size();

			i32 jj = 0;
			for (; row__ < numJointRows;)
			{
				//i32 sbA = m_allConstraintPtrArray[row__]->m_solverBodyIdA;
				i32 sbB = m_allConstraintPtrArray[row__]->m_solverBodyIdB;
				//	RigidBody* orgBodyA = m_tmpSolverBodyPool[sbA].m_originalBody;
				RigidBody* orgBodyB = m_tmpSolverBodyPool[sbB].m_originalBody;

				u32k infom = row__ < m_tmpSolverNonContactConstraintPool.size() ? m_tmpConstraintSizesPool[jj].m_numConstraintRows : numContactRows;

				const Scalar* JinvMrow = JinvM + 2 * 8 * (size_t)row__;
				const Scalar* Jrow = Jptr + 2 * 8 * (size_t)row__;
				m_A.multiply2_p8r(JinvMrow, Jrow, infom, infom, row__, row__);
				if (orgBodyB)
				{
					m_A.multiplyAdd2_p8r(JinvMrow + 8 * (size_t)infom, Jrow + 8 * (size_t)infom, infom, infom, row__, row__);
				}
				row__ += infom;
				jj++;
			}
		}
	}

	if (1)
	{
		// add cfm to the diagonal of m_A
		for (i32 i = 0; i < m_A.rows(); ++i)
		{
			m_A.setElem(i, i, m_A(i, i) + infoGlobal.m_globalCfm / infoGlobal.m_timeStep);
		}
	}

	///fill the upper triangle of the matrix, to make it symmetric
	{
		DRX3D_PROFILE("fill the upper triangle ");
		m_A.copyLowerToUpperTriangle();
	}

	{
		DRX3D_PROFILE("resize/init x");
		m_x.resize(numConstraintRows);
		m_xSplit.resize(numConstraintRows);

		if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
		{
			for (i32 i = 0; i < m_allConstraintPtrArray.size(); i++)
			{
				const SolverConstraint& c = *m_allConstraintPtrArray[i];
				m_x[i] = c.m_appliedImpulse;
				m_xSplit[i] = c.m_appliedPushImpulse;
			}
		}
		else
		{
			m_x.setZero();
			m_xSplit.setZero();
		}
	}
}

void MultiBodyMLCPConstraintSolver::createMLCPFastMultiBody(const ContactSolverInfo& infoGlobal)
{
	i32k multiBodyNumConstraints = m_multiBodyAllConstraintPtrArray.size();

	if (multiBodyNumConstraints == 0)
		return;

	// 1. Compute b
	{
		DRX3D_PROFILE("init b (rhs)");

		m_multiBodyB.resize(multiBodyNumConstraints);
		m_multiBodyB.setZero();

		for (i32 i = 0; i < multiBodyNumConstraints; ++i)
		{
			const MultiBodySolverConstraint& constraint = *m_multiBodyAllConstraintPtrArray[i];
			const Scalar jacDiag = constraint.m_jacDiagABInv;

			if (!FuzzyZero(jacDiag))
			{
				// Note that rhsPenetration is currently always zero because the split impulse hasn't been implemented for multibody yet.
				const Scalar rhs = constraint.m_rhs;
				m_multiBodyB[i] = rhs / jacDiag;
			}
		}
	}

	// 2. Compute lo and hi
	{
		DRX3D_PROFILE("init lo/ho");

		m_multiBodyLo.resize(multiBodyNumConstraints);
		m_multiBodyHi.resize(multiBodyNumConstraints);

		for (i32 i = 0; i < multiBodyNumConstraints; ++i)
		{
			const MultiBodySolverConstraint& constraint = *m_multiBodyAllConstraintPtrArray[i];
			m_multiBodyLo[i] = constraint.m_lowerLimit;
			m_multiBodyHi[i] = constraint.m_upperLimit;
		}
	}

	// 3. Construct A matrix by using the impulse testing
	{
		DRX3D_PROFILE("Compute A");

		{
			DRX3D_PROFILE("m_A.resize");
			m_multiBodyA.resize(multiBodyNumConstraints, multiBodyNumConstraints);
		}

		for (i32 i = 0; i < multiBodyNumConstraints; ++i)
		{
			// Compute the diagonal of A, which is A(i, i)
			const MultiBodySolverConstraint& constraint = *m_multiBodyAllConstraintPtrArray[i];
			const Scalar diagA = computeConstraintMatrixDiagElementMultiBody(m_tmpSolverBodyPool, m_data, constraint);
			m_multiBodyA.setElem(i, i, diagA);

			// Computes the off-diagonals of A:
			//   a. The rest of i-th row of A, from A(i, i+1) to A(i, n)
			//   b. The rest of i-th column of A, from A(i+1, i) to A(n, i)
			for (i32 j = i + 1; j < multiBodyNumConstraints; ++j)
			{
				const MultiBodySolverConstraint& offDiagConstraint = *m_multiBodyAllConstraintPtrArray[j];
				const Scalar offDiagA = computeConstraintMatrixOffDiagElementMultiBody(m_tmpSolverBodyPool, m_data, constraint, offDiagConstraint);

				// Set the off-diagonal values of A. Note that A is symmetric.
				m_multiBodyA.setElem(i, j, offDiagA);
				m_multiBodyA.setElem(j, i, offDiagA);
			}
		}
	}

	// Add CFM to the diagonal of m_A
	for (i32 i = 0; i < m_multiBodyA.rows(); ++i)
	{
		m_multiBodyA.setElem(i, i, m_multiBodyA(i, i) + infoGlobal.m_globalCfm / infoGlobal.m_timeStep);
	}

	// 4. Initialize x
	{
		DRX3D_PROFILE("resize/init x");

		m_multiBodyX.resize(multiBodyNumConstraints);

		if (infoGlobal.m_solverMode & SOLVER_USE_WARMSTARTING)
		{
			for (i32 i = 0; i < multiBodyNumConstraints; ++i)
			{
				const MultiBodySolverConstraint& constraint = *m_multiBodyAllConstraintPtrArray[i];
				m_multiBodyX[i] = constraint.m_appliedImpulse;
			}
		}
		else
		{
			m_multiBodyX.setZero();
		}
	}
}

bool MultiBodyMLCPConstraintSolver::solveMLCP(const ContactSolverInfo& infoGlobal)
{
	bool result = true;

	if (m_A.rows() != 0)
	{
		// If using split impulse, we solve 2 separate (M)LCPs
		if (infoGlobal.m_splitImpulse)
		{
			const MatrixXu Acopy = m_A;
			const AlignedObjectArray<i32> limitDependenciesCopy = m_limitDependencies;
			// TODO(JS): Do we really need these copies when solveMLCP takes them as const?

			result = m_solver->solveMLCP(m_A, m_b, m_x, m_lo, m_hi, m_limitDependencies, infoGlobal.m_numIterations);
			if (result)
				result = m_solver->solveMLCP(Acopy, m_bSplit, m_xSplit, m_lo, m_hi, limitDependenciesCopy, infoGlobal.m_numIterations);
		}
		else
		{
			result = m_solver->solveMLCP(m_A, m_b, m_x, m_lo, m_hi, m_limitDependencies, infoGlobal.m_numIterations);
		}
	}

	if (!result)
		return false;

	if (m_multiBodyA.rows() != 0)
	{
		result = m_solver->solveMLCP(m_multiBodyA, m_multiBodyB, m_multiBodyX, m_multiBodyLo, m_multiBodyHi, m_multiBodyLimitDependencies, infoGlobal.m_numIterations);
	}

	return result;
}

Scalar MultiBodyMLCPConstraintSolver::solveGroupCacheFriendlySetup(
	CollisionObject2** bodies,
	i32 numBodies,
	PersistentManifold** manifoldPtr,
	i32 numManifolds,
	TypedConstraint** constraints,
	i32 numConstraints,
	const ContactSolverInfo& infoGlobal,
	IDebugDraw* debugDrawer)
{
	// 1. Setup for rigid-bodies
	MultiBodyConstraintSolver::solveGroupCacheFriendlySetup(
		bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);

	// 2. Setup for multi-bodies
	//   a. Collect all different kinds of constraint as pointers into one array, m_allConstraintPtrArray
	//   b. Set the index array for frictional contact constraints, m_limitDependencies
	{
		DRX3D_PROFILE("gather constraint data");

		i32 dindex = 0;

		i32k numRigidBodyConstraints = m_tmpSolverNonContactConstraintPool.size() + m_tmpSolverContactConstraintPool.size() + m_tmpSolverContactFrictionConstraintPool.size();
		i32k numMultiBodyConstraints = m_multiBodyNonContactConstraints.size() + m_multiBodyNormalContactConstraints.size() + m_multiBodyFrictionContactConstraints.size();

		m_allConstraintPtrArray.resize(0);
		m_multiBodyAllConstraintPtrArray.resize(0);

		// i. Setup for rigid bodies

		m_limitDependencies.resize(numRigidBodyConstraints);

		for (i32 i = 0; i < m_tmpSolverNonContactConstraintPool.size(); ++i)
		{
			m_allConstraintPtrArray.push_back(&m_tmpSolverNonContactConstraintPool[i]);
			m_limitDependencies[dindex++] = -1;
		}

		i32 firstContactConstraintOffset = dindex;

		// The SequentialImpulseConstraintSolver moves all friction constraints at the very end, we can also interleave them instead
		if (interleaveContactAndFriction1)
		{
			for (i32 i = 0; i < m_tmpSolverContactConstraintPool.size(); i++)
			{
				i32k numFrictionPerContact = m_tmpSolverContactConstraintPool.size() == m_tmpSolverContactFrictionConstraintPool.size() ? 1 : 2;

				m_allConstraintPtrArray.push_back(&m_tmpSolverContactConstraintPool[i]);
				m_limitDependencies[dindex++] = -1;
				m_allConstraintPtrArray.push_back(&m_tmpSolverContactFrictionConstraintPool[i * numFrictionPerContact]);
				i32 findex = (m_tmpSolverContactFrictionConstraintPool[i * numFrictionPerContact].m_frictionIndex * (1 + numFrictionPerContact));
				m_limitDependencies[dindex++] = findex + firstContactConstraintOffset;
				if (numFrictionPerContact == 2)
				{
					m_allConstraintPtrArray.push_back(&m_tmpSolverContactFrictionConstraintPool[i * numFrictionPerContact + 1]);
					m_limitDependencies[dindex++] = findex + firstContactConstraintOffset;
				}
			}
		}
		else
		{
			for (i32 i = 0; i < m_tmpSolverContactConstraintPool.size(); i++)
			{
				m_allConstraintPtrArray.push_back(&m_tmpSolverContactConstraintPool[i]);
				m_limitDependencies[dindex++] = -1;
			}
			for (i32 i = 0; i < m_tmpSolverContactFrictionConstraintPool.size(); i++)
			{
				m_allConstraintPtrArray.push_back(&m_tmpSolverContactFrictionConstraintPool[i]);
				m_limitDependencies[dindex++] = m_tmpSolverContactFrictionConstraintPool[i].m_frictionIndex + firstContactConstraintOffset;
			}
		}

		if (!m_allConstraintPtrArray.size())
		{
			m_A.resize(0, 0);
			m_b.resize(0);
			m_x.resize(0);
			m_lo.resize(0);
			m_hi.resize(0);
		}

		// ii. Setup for multibodies

		dindex = 0;

		m_multiBodyLimitDependencies.resize(numMultiBodyConstraints);

		for (i32 i = 0; i < m_multiBodyNonContactConstraints.size(); ++i)
		{
			m_multiBodyAllConstraintPtrArray.push_back(&m_multiBodyNonContactConstraints[i]);
			m_multiBodyLimitDependencies[dindex++] = -1;
		}

		firstContactConstraintOffset = dindex;

		// The SequentialImpulseConstraintSolver moves all friction constraints at the very end, we can also interleave them instead
		if (interleaveContactAndFriction1)
		{
			for (i32 i = 0; i < m_multiBodyNormalContactConstraints.size(); ++i)
			{
				i32k numtiBodyNumFrictionPerContact = m_multiBodyNormalContactConstraints.size() == m_multiBodyFrictionContactConstraints.size() ? 1 : 2;

				m_multiBodyAllConstraintPtrArray.push_back(&m_multiBodyNormalContactConstraints[i]);
				m_multiBodyLimitDependencies[dindex++] = -1;

				MultiBodySolverConstraint& frictionContactConstraint1 = m_multiBodyFrictionContactConstraints[i * numtiBodyNumFrictionPerContact];
				m_multiBodyAllConstraintPtrArray.push_back(&frictionContactConstraint1);

				i32k findex = (frictionContactConstraint1.m_frictionIndex * (1 + numtiBodyNumFrictionPerContact)) + firstContactConstraintOffset;

				m_multiBodyLimitDependencies[dindex++] = findex;

				if (numtiBodyNumFrictionPerContact == 2)
				{
					MultiBodySolverConstraint& frictionContactConstraint2 = m_multiBodyFrictionContactConstraints[i * numtiBodyNumFrictionPerContact + 1];
					m_multiBodyAllConstraintPtrArray.push_back(&frictionContactConstraint2);

					m_multiBodyLimitDependencies[dindex++] = findex;
				}
			}
		}
		else
		{
			for (i32 i = 0; i < m_multiBodyNormalContactConstraints.size(); ++i)
			{
				m_multiBodyAllConstraintPtrArray.push_back(&m_multiBodyNormalContactConstraints[i]);
				m_multiBodyLimitDependencies[dindex++] = -1;
			}
			for (i32 i = 0; i < m_multiBodyFrictionContactConstraints.size(); ++i)
			{
				m_multiBodyAllConstraintPtrArray.push_back(&m_multiBodyFrictionContactConstraints[i]);
				m_multiBodyLimitDependencies[dindex++] = m_multiBodyFrictionContactConstraints[i].m_frictionIndex + firstContactConstraintOffset;
			}
		}

		if (!m_multiBodyAllConstraintPtrArray.size())
		{
			m_multiBodyA.resize(0, 0);
			m_multiBodyB.resize(0);
			m_multiBodyX.resize(0);
			m_multiBodyLo.resize(0);
			m_multiBodyHi.resize(0);
		}
	}

	// Construct MLCP terms
	{
		DRX3D_PROFILE("createMLCPFast");
		createMLCPFast(infoGlobal);
	}

	return Scalar(0);
}

Scalar MultiBodyMLCPConstraintSolver::solveGroupCacheFriendlyIterations(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	bool result = true;
	{
		DRX3D_PROFILE("solveMLCP");
		result = solveMLCP(infoGlobal);
	}

	// Fallback to SequentialImpulseConstraintSolver::solveGroupCacheFriendlyIterations if the solution isn't valid.
	if (!result)
	{
		m_fallback++;
		return MultiBodyConstraintSolver::solveGroupCacheFriendlyIterations(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
	}

	{
		DRX3D_PROFILE("process MLCP results");

		for (i32 i = 0; i < m_allConstraintPtrArray.size(); ++i)
		{
			const SolverConstraint& c = *m_allConstraintPtrArray[i];

			const Scalar deltaImpulse = m_x[i] - c.m_appliedImpulse;
			c.m_appliedImpulse = m_x[i];

			i32 sbA = c.m_solverBodyIdA;
			i32 sbB = c.m_solverBodyIdB;

			SolverBody& solverBodyA = m_tmpSolverBodyPool[sbA];
			SolverBody& solverBodyB = m_tmpSolverBodyPool[sbB];

			solverBodyA.internalApplyImpulse(c.m_contactNormal1 * solverBodyA.internalGetInvMass(), c.m_angularComponentA, deltaImpulse);
			solverBodyB.internalApplyImpulse(c.m_contactNormal2 * solverBodyB.internalGetInvMass(), c.m_angularComponentB, deltaImpulse);

			if (infoGlobal.m_splitImpulse)
			{
				const Scalar deltaPushImpulse = m_xSplit[i] - c.m_appliedPushImpulse;
				solverBodyA.internalApplyPushImpulse(c.m_contactNormal1 * solverBodyA.internalGetInvMass(), c.m_angularComponentA, deltaPushImpulse);
				solverBodyB.internalApplyPushImpulse(c.m_contactNormal2 * solverBodyB.internalGetInvMass(), c.m_angularComponentB, deltaPushImpulse);
				c.m_appliedPushImpulse = m_xSplit[i];
			}
		}

		for (i32 i = 0; i < m_multiBodyAllConstraintPtrArray.size(); ++i)
		{
			MultiBodySolverConstraint& c = *m_multiBodyAllConstraintPtrArray[i];

			const Scalar deltaImpulse = m_multiBodyX[i] - c.m_appliedImpulse;
			c.m_appliedImpulse = m_multiBodyX[i];

			MultiBody* multiBodyA = c.m_multiBodyA;
			if (multiBodyA)
			{
				i32k ndofA = multiBodyA->getNumDofs() + 6;
				applyDeltaVee(&m_data.m_deltaVelocitiesUnitImpulse[c.m_jacAindex], deltaImpulse, c.m_deltaVelAindex, ndofA);
#ifdef DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS
				//note: update of the actual velocities (below) in the multibody does not have to happen now since m_deltaVelocities can be applied after all iterations
				//it would make the multibody solver more like the regular one with m_deltaVelocities being equivalent to SolverBody::m_deltaLinearVelocity/m_deltaAngularVelocity
				multiBodyA->applyDeltaVeeMultiDof2(&m_data.m_deltaVelocitiesUnitImpulse[c.m_jacAindex], deltaImpulse);
#endif  // DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS
			}
			else
			{
				i32k sbA = c.m_solverBodyIdA;
				SolverBody& solverBodyA = m_tmpSolverBodyPool[sbA];
				solverBodyA.internalApplyImpulse(c.m_contactNormal1 * solverBodyA.internalGetInvMass(), c.m_angularComponentA, deltaImpulse);
			}

			MultiBody* multiBodyB = c.m_multiBodyB;
			if (multiBodyB)
			{
				i32k ndofB = multiBodyB->getNumDofs() + 6;
				applyDeltaVee(&m_data.m_deltaVelocitiesUnitImpulse[c.m_jacBindex], deltaImpulse, c.m_deltaVelBindex, ndofB);
#ifdef DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS
				//note: update of the actual velocities (below) in the multibody does not have to happen now since m_deltaVelocities can be applied after all iterations
				//it would make the multibody solver more like the regular one with m_deltaVelocities being equivalent to SolverBody::m_deltaLinearVelocity/m_deltaAngularVelocity
				multiBodyB->applyDeltaVeeMultiDof2(&m_data.m_deltaVelocitiesUnitImpulse[c.m_jacBindex], deltaImpulse);
#endif  // DIRECTLY_UPDATE_VELOCITY_DURING_SOLVER_ITERATIONS
			}
			else
			{
				i32k sbB = c.m_solverBodyIdB;
				SolverBody& solverBodyB = m_tmpSolverBodyPool[sbB];
				solverBodyB.internalApplyImpulse(c.m_contactNormal2 * solverBodyB.internalGetInvMass(), c.m_angularComponentB, deltaImpulse);
			}
		}
	}

	return Scalar(0);
}

MultiBodyMLCPConstraintSolver::MultiBodyMLCPConstraintSolver(MLCPSolverInterface* solver)
	: m_solver(solver), m_fallback(0)
{
	// Do nothing
}

MultiBodyMLCPConstraintSolver::~MultiBodyMLCPConstraintSolver()
{
	// Do nothing
}

void MultiBodyMLCPConstraintSolver::setMLCPSolver(MLCPSolverInterface* solver)
{
	m_solver = solver;
}

i32 MultiBodyMLCPConstraintSolver::getNumFallbacks() const
{
	return m_fallback;
}

void MultiBodyMLCPConstraintSolver::setNumFallbacks(i32 num)
{
	m_fallback = num;
}

ConstraintSolverType MultiBodyMLCPConstraintSolver::getSolverType() const
{
	return DRX3D_MLCP_SOLVER;
}
