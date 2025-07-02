#include <drx3D/Physics/Dynamics/MLCPSolvers/MLCPSolver.h>
#include <drx3D/Maths/Linear/MatrixX.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Dynamics/MLCPSolvers/SolveProjectedGaussSeidel.h>

MLCPSolver::MLCPSolver(MLCPSolverInterface* solver)
	: m_solver(solver),
	  m_fallback(0)
{
}

MLCPSolver::~MLCPSolver()
{
}

bool gUseMatrixMultiply = false;
bool interleaveContactAndFriction = false;

Scalar MLCPSolver::solveGroupCacheFriendlySetup(CollisionObject2** bodies, i32 numBodiesUnUsed, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	SequentialImpulseConstraintSolver::solveGroupCacheFriendlySetup(bodies, numBodiesUnUsed, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);

	{
		DRX3D_PROFILE("gather constraint data");

		i32 numFrictionPerContact = m_tmpSolverContactConstraintPool.size() == m_tmpSolverContactFrictionConstraintPool.size() ? 1 : 2;

		//	i32 numBodies = m_tmpSolverBodyPool.size();
		m_allConstraintPtrArray.resize(0);
		m_limitDependencies.resize(m_tmpSolverNonContactConstraintPool.size() + m_tmpSolverContactConstraintPool.size() + m_tmpSolverContactFrictionConstraintPool.size());
		Assert(m_limitDependencies.size() == m_tmpSolverNonContactConstraintPool.size() + m_tmpSolverContactConstraintPool.size() + m_tmpSolverContactFrictionConstraintPool.size());
		//	printf("m_limitDependencies.size() = %d\n",m_limitDependencies.size());

		i32 dindex = 0;
		for (i32 i = 0; i < m_tmpSolverNonContactConstraintPool.size(); i++)
		{
			m_allConstraintPtrArray.push_back(&m_tmpSolverNonContactConstraintPool[i]);
			m_limitDependencies[dindex++] = -1;
		}

		///The SequentialImpulseConstraintSolver moves all friction constraints at the very end, we can also interleave them instead

		i32 firstContactConstraintOffset = dindex;

		if (interleaveContactAndFriction)
		{
			for (i32 i = 0; i < m_tmpSolverContactConstraintPool.size(); i++)
			{
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
			return 0.f;
		}
	}

	if (gUseMatrixMultiply)
	{
		DRX3D_PROFILE("createMLCP");
		createMLCP(infoGlobal);
	}
	else
	{
		DRX3D_PROFILE("createMLCPFast");
		createMLCPFast(infoGlobal);
	}

	return 0.f;
}

bool MLCPSolver::solveMLCP(const ContactSolverInfo& infoGlobal)
{
	bool result = true;

	if (m_A.rows() == 0)
		return true;

	//if using split impulse, we solve 2 separate (M)LCPs
	if (infoGlobal.m_splitImpulse)
	{
		MatrixXu Acopy = m_A;
		AlignedObjectArray<i32> limitDependenciesCopy = m_limitDependencies;
		//		printf("solve first LCP\n");
		result = m_solver->solveMLCP(m_A, m_b, m_x, m_lo, m_hi, m_limitDependencies, infoGlobal.m_numIterations);
		if (result)
			result = m_solver->solveMLCP(Acopy, m_bSplit, m_xSplit, m_lo, m_hi, limitDependenciesCopy, infoGlobal.m_numIterations);
	}
	else
	{
		result = m_solver->solveMLCP(m_A, m_b, m_x, m_lo, m_hi, m_limitDependencies, infoGlobal.m_numIterations);
	}
	return result;
}

struct JointNode
{
	i32 jointIndex;          // pointer to enclosing dxJoint object
	i32 otherBodyIndex;      // *other* body this joint is connected to
	i32 nextJointNodeIndex;  //-1 for null
	i32 constraintRowIndex;
};

void MLCPSolver::createMLCPFast(const ContactSolverInfo& infoGlobal)
{
	i32 numContactRows = interleaveContactAndFriction ? 3 : 1;

	i32 numConstraintRows = m_allConstraintPtrArray.size();
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
	AlignedObjectArray<JointNode> jointNodeArray;
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

void MLCPSolver::createMLCP(const ContactSolverInfo& infoGlobal)
{
	i32 numBodies = this->m_tmpSolverBodyPool.size();
	i32 numConstraintRows = m_allConstraintPtrArray.size();

	m_b.resize(numConstraintRows);
	if (infoGlobal.m_splitImpulse)
		m_bSplit.resize(numConstraintRows);

	m_bSplit.setZero();
	m_b.setZero();

	for (i32 i = 0; i < numConstraintRows; i++)
	{
		if (m_allConstraintPtrArray[i]->m_jacDiagABInv)
		{
			m_b[i] = m_allConstraintPtrArray[i]->m_rhs / m_allConstraintPtrArray[i]->m_jacDiagABInv;
			if (infoGlobal.m_splitImpulse)
				m_bSplit[i] = m_allConstraintPtrArray[i]->m_rhsPenetration / m_allConstraintPtrArray[i]->m_jacDiagABInv;
		}
	}

	MatrixXu& Minv = m_scratchMInv;
	Minv.resize(6 * numBodies, 6 * numBodies);
	Minv.setZero();
	for (i32 i = 0; i < numBodies; i++)
	{
		const SolverBody& rb = m_tmpSolverBodyPool[i];
		const Vec3& invMass = rb.m_invMass;
		setElem(Minv, i * 6 + 0, i * 6 + 0, invMass[0]);
		setElem(Minv, i * 6 + 1, i * 6 + 1, invMass[1]);
		setElem(Minv, i * 6 + 2, i * 6 + 2, invMass[2]);
		RigidBody* orgBody = m_tmpSolverBodyPool[i].m_originalBody;

		for (i32 r = 0; r < 3; r++)
			for (i32 c = 0; c < 3; c++)
				setElem(Minv, i * 6 + 3 + r, i * 6 + 3 + c, orgBody ? orgBody->getInvInertiaTensorWorld()[r][c] : 0);
	}

	MatrixXu& J = m_scratchJ;
	J.resize(numConstraintRows, 6 * numBodies);
	J.setZero();

	m_lo.resize(numConstraintRows);
	m_hi.resize(numConstraintRows);

	for (i32 i = 0; i < numConstraintRows; i++)
	{
		m_lo[i] = m_allConstraintPtrArray[i]->m_lowerLimit;
		m_hi[i] = m_allConstraintPtrArray[i]->m_upperLimit;

		i32 bodyIndex0 = m_allConstraintPtrArray[i]->m_solverBodyIdA;
		i32 bodyIndex1 = m_allConstraintPtrArray[i]->m_solverBodyIdB;
		if (m_tmpSolverBodyPool[bodyIndex0].m_originalBody)
		{
			setElem(J, i, 6 * bodyIndex0 + 0, m_allConstraintPtrArray[i]->m_contactNormal1[0]);
			setElem(J, i, 6 * bodyIndex0 + 1, m_allConstraintPtrArray[i]->m_contactNormal1[1]);
			setElem(J, i, 6 * bodyIndex0 + 2, m_allConstraintPtrArray[i]->m_contactNormal1[2]);
			setElem(J, i, 6 * bodyIndex0 + 3, m_allConstraintPtrArray[i]->m_relpos1CrossNormal[0]);
			setElem(J, i, 6 * bodyIndex0 + 4, m_allConstraintPtrArray[i]->m_relpos1CrossNormal[1]);
			setElem(J, i, 6 * bodyIndex0 + 5, m_allConstraintPtrArray[i]->m_relpos1CrossNormal[2]);
		}
		if (m_tmpSolverBodyPool[bodyIndex1].m_originalBody)
		{
			setElem(J, i, 6 * bodyIndex1 + 0, m_allConstraintPtrArray[i]->m_contactNormal2[0]);
			setElem(J, i, 6 * bodyIndex1 + 1, m_allConstraintPtrArray[i]->m_contactNormal2[1]);
			setElem(J, i, 6 * bodyIndex1 + 2, m_allConstraintPtrArray[i]->m_contactNormal2[2]);
			setElem(J, i, 6 * bodyIndex1 + 3, m_allConstraintPtrArray[i]->m_relpos2CrossNormal[0]);
			setElem(J, i, 6 * bodyIndex1 + 4, m_allConstraintPtrArray[i]->m_relpos2CrossNormal[1]);
			setElem(J, i, 6 * bodyIndex1 + 5, m_allConstraintPtrArray[i]->m_relpos2CrossNormal[2]);
		}
	}

	MatrixXu& J_transpose = m_scratchJTranspose;
	J_transpose = J.transpose();

	MatrixXu& tmp = m_scratchTmp;
	//Minv.printMatrix("Minv=");
	{
		{
			DRX3D_PROFILE("J*Minv");
			tmp = J * Minv;
		}
		{
			DRX3D_PROFILE("J*tmp");
			m_A = tmp * J_transpose;
		}
	}
	//J.printMatrix("J");
	if (1)
	{
		// add cfm to the diagonal of m_A
		for (i32 i = 0; i < m_A.rows(); ++i)
		{
			m_A.setElem(i, i, m_A(i, i) + infoGlobal.m_globalCfm / infoGlobal.m_timeStep);
		}
	}

	m_x.resize(numConstraintRows);
	if (infoGlobal.m_splitImpulse)
		m_xSplit.resize(numConstraintRows);
	//	m_x.setZero();

	for (i32 i = 0; i < m_allConstraintPtrArray.size(); i++)
	{
		const SolverConstraint& c = *m_allConstraintPtrArray[i];
		m_x[i] = c.m_appliedImpulse;
		if (infoGlobal.m_splitImpulse)
			m_xSplit[i] = c.m_appliedPushImpulse;
	}
}

Scalar MLCPSolver::solveGroupCacheFriendlyIterations(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	bool result = true;
	{
		DRX3D_PROFILE("solveMLCP");
		//		printf("m_A(%d,%d)\n", m_A.rows(),m_A.cols());
		result = solveMLCP(infoGlobal);
	}

	//check if solution is valid, and otherwise fallback to SequentialImpulseConstraintSolver::solveGroupCacheFriendlyIterations
	if (result)
	{
		DRX3D_PROFILE("process MLCP results");
		for (i32 i = 0; i < m_allConstraintPtrArray.size(); i++)
		{
			{
				SolverConstraint& c = *m_allConstraintPtrArray[i];
				i32 sbA = c.m_solverBodyIdA;
				i32 sbB = c.m_solverBodyIdB;
				//RigidBody* orgBodyA = m_tmpSolverBodyPool[sbA].m_originalBody;
				//	RigidBody* orgBodyB = m_tmpSolverBodyPool[sbB].m_originalBody;

				SolverBody& solverBodyA = m_tmpSolverBodyPool[sbA];
				SolverBody& solverBodyB = m_tmpSolverBodyPool[sbB];

				{
					Scalar deltaImpulse = m_x[i] - c.m_appliedImpulse;
					c.m_appliedImpulse = m_x[i];
					solverBodyA.internalApplyImpulse(c.m_contactNormal1 * solverBodyA.internalGetInvMass(), c.m_angularComponentA, deltaImpulse);
					solverBodyB.internalApplyImpulse(c.m_contactNormal2 * solverBodyB.internalGetInvMass(), c.m_angularComponentB, deltaImpulse);
				}

				if (infoGlobal.m_splitImpulse)
				{
					Scalar deltaImpulse = m_xSplit[i] - c.m_appliedPushImpulse;
					solverBodyA.internalApplyPushImpulse(c.m_contactNormal1 * solverBodyA.internalGetInvMass(), c.m_angularComponentA, deltaImpulse);
					solverBodyB.internalApplyPushImpulse(c.m_contactNormal2 * solverBodyB.internalGetInvMass(), c.m_angularComponentB, deltaImpulse);
					c.m_appliedPushImpulse = m_xSplit[i];
				}
			}
		}
	}
	else
	{
		//	printf("m_fallback = %d\n",m_fallback);
		m_fallback++;
		SequentialImpulseConstraintSolver::solveGroupCacheFriendlyIterations(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
	}

	return 0.f;
}
