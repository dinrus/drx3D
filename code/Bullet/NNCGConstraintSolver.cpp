
#include <drx3D/Physics/Dynamics/ConstraintSolver/NNCGConstraintSolver.h>

Scalar NNCGConstraintSolver::solveGroupCacheFriendlySetup(CollisionObject2** bodies, i32 numBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	Scalar val = SequentialImpulseConstraintSolver::solveGroupCacheFriendlySetup(bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);

	m_pNC.resizeNoInitialize(m_tmpSolverNonContactConstraintPool.size());
	m_pC.resizeNoInitialize(m_tmpSolverContactConstraintPool.size());
	m_pCF.resizeNoInitialize(m_tmpSolverContactFrictionConstraintPool.size());
	m_pCRF.resizeNoInitialize(m_tmpSolverContactRollingFrictionConstraintPool.size());

	m_deltafNC.resizeNoInitialize(m_tmpSolverNonContactConstraintPool.size());
	m_deltafC.resizeNoInitialize(m_tmpSolverContactConstraintPool.size());
	m_deltafCF.resizeNoInitialize(m_tmpSolverContactFrictionConstraintPool.size());
	m_deltafCRF.resizeNoInitialize(m_tmpSolverContactRollingFrictionConstraintPool.size());

	return val;
}

Scalar NNCGConstraintSolver::solveSingleIteration(i32 iteration, CollisionObject2** /*bodies */, i32 /*numBodies*/, PersistentManifold** /*manifoldPtr*/, i32 /*numManifolds*/, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* /*debugDrawer*/)
{
	i32 numNonContactPool = m_tmpSolverNonContactConstraintPool.size();
	i32 numConstraintPool = m_tmpSolverContactConstraintPool.size();
	i32 numFrictionPool = m_tmpSolverContactFrictionConstraintPool.size();

	if (infoGlobal.m_solverMode & SOLVER_RANDMIZE_ORDER)
	{
		if (1)  // uncomment this for a bit less random ((iteration & 7) == 0)
		{
			for (i32 j = 0; j < numNonContactPool; ++j)
			{
				i32 tmp = m_orderNonContactConstraintPool[j];
				i32 swapi = RandInt2(j + 1);
				m_orderNonContactConstraintPool[j] = m_orderNonContactConstraintPool[swapi];
				m_orderNonContactConstraintPool[swapi] = tmp;
			}

			//contact/friction constraints are not solved more than
			if (iteration < infoGlobal.m_numIterations)
			{
				for (i32 j = 0; j < numConstraintPool; ++j)
				{
					i32 tmp = m_orderTmpConstraintPool[j];
					i32 swapi = RandInt2(j + 1);
					m_orderTmpConstraintPool[j] = m_orderTmpConstraintPool[swapi];
					m_orderTmpConstraintPool[swapi] = tmp;
				}

				for (i32 j = 0; j < numFrictionPool; ++j)
				{
					i32 tmp = m_orderFrictionConstraintPool[j];
					i32 swapi = RandInt2(j + 1);
					m_orderFrictionConstraintPool[j] = m_orderFrictionConstraintPool[swapi];
					m_orderFrictionConstraintPool[swapi] = tmp;
				}
			}
		}
	}

	Scalar deltaflengthsqr = 0;
	{
		for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++)
		{
			SolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[m_orderNonContactConstraintPool[j]];
			if (iteration < constraint.m_overrideNumSolverIterations)
			{
				Scalar deltaf = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[constraint.m_solverBodyIdA], m_tmpSolverBodyPool[constraint.m_solverBodyIdB], constraint);
				m_deltafNC[j] = deltaf;
				deltaflengthsqr += deltaf * deltaf;
			}
		}
	}

	if (m_onlyForNoneContact)
	{
		if (iteration == 0)
		{
			for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++) m_pNC[j] = m_deltafNC[j];
		}
		else
		{
			// deltaflengthsqrprev can be 0 only if the solver solved the problem exactly in the previous iteration. In this case we should have quit, but mainly for debug reason with this 'hack' it is now allowed to continue the calculation
			Scalar beta = m_deltafLengthSqrPrev > 0 ? deltaflengthsqr / m_deltafLengthSqrPrev : 2;
			if (beta > 1)
			{
				for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++) m_pNC[j] = 0;
			}
			else
			{
				for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++)
				{
					SolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[m_orderNonContactConstraintPool[j]];
					if (iteration < constraint.m_overrideNumSolverIterations)
					{
						Scalar additionaldeltaimpulse = beta * m_pNC[j];
						constraint.m_appliedImpulse = Scalar(constraint.m_appliedImpulse) + additionaldeltaimpulse;
						m_pNC[j] = beta * m_pNC[j] + m_deltafNC[j];
						SolverBody& body1 = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
						SolverBody& body2 = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
						const SolverConstraint& c = constraint;
						body1.internalApplyImpulse(c.m_contactNormal1 * body1.internalGetInvMass(), c.m_angularComponentA, additionaldeltaimpulse);
						body2.internalApplyImpulse(c.m_contactNormal2 * body2.internalGetInvMass(), c.m_angularComponentB, additionaldeltaimpulse);
					}
				}
			}
		}
		m_deltafLengthSqrPrev = deltaflengthsqr;
	}

	{
		if (iteration < infoGlobal.m_numIterations)
		{
			for (i32 j = 0; j < numConstraints; j++)
			{
				if (constraints[j]->isEnabled())
				{
					i32 bodyAid = getOrInitSolverBody(constraints[j]->getRigidBodyA(), infoGlobal.m_timeStep);
					i32 bodyBid = getOrInitSolverBody(constraints[j]->getRigidBodyB(), infoGlobal.m_timeStep);
					SolverBody& bodyA = m_tmpSolverBodyPool[bodyAid];
					SolverBody& bodyB = m_tmpSolverBodyPool[bodyBid];
					constraints[j]->solveConstraintObsolete(bodyA, bodyB, infoGlobal.m_timeStep);
				}
			}

			///solve all contact constraints
			if (infoGlobal.m_solverMode & SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS)
			{
				i32 numPoolConstraints = m_tmpSolverContactConstraintPool.size();
				i32 multiplier = (infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS) ? 2 : 1;

				for (i32 c = 0; c < numPoolConstraints; c++)
				{
					Scalar totalImpulse = 0;

					{
						const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[c]];
						Scalar deltaf = resolveSingleConstraintRowLowerLimit(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
						m_deltafC[c] = deltaf;
						deltaflengthsqr += deltaf * deltaf;
						totalImpulse = solveManifold.m_appliedImpulse;
					}
					bool applyFriction = true;
					if (applyFriction)
					{
						{
							SolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[c * multiplier]];

							if (totalImpulse > Scalar(0))
							{
								solveManifold.m_lowerLimit = -(solveManifold.m_friction * totalImpulse);
								solveManifold.m_upperLimit = solveManifold.m_friction * totalImpulse;
								Scalar deltaf = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
								m_deltafCF[c * multiplier] = deltaf;
								deltaflengthsqr += deltaf * deltaf;
							}
							else
							{
								m_deltafCF[c * multiplier] = 0;
							}
						}

						if (infoGlobal.m_solverMode & SOLVER_USE_2_FRICTION_DIRECTIONS)
						{
							SolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[c * multiplier + 1]];

							if (totalImpulse > Scalar(0))
							{
								solveManifold.m_lowerLimit = -(solveManifold.m_friction * totalImpulse);
								solveManifold.m_upperLimit = solveManifold.m_friction * totalImpulse;
								Scalar deltaf = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
								m_deltafCF[c * multiplier + 1] = deltaf;
								deltaflengthsqr += deltaf * deltaf;
							}
							else
							{
								m_deltafCF[c * multiplier + 1] = 0;
							}
						}
					}
				}
			}
			else  //SOLVER_INTERLEAVE_CONTACT_AND_FRICTION_CONSTRAINTS
			{
				//solve the friction constraints after all contact constraints, don't interleave them
				i32 numPoolConstraints = m_tmpSolverContactConstraintPool.size();
				i32 j;

				for (j = 0; j < numPoolConstraints; j++)
				{
					const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];
					Scalar deltaf = resolveSingleConstraintRowLowerLimit(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
					m_deltafC[j] = deltaf;
					deltaflengthsqr += deltaf * deltaf;
				}

				///solve all friction constraints

				i32 numFrictionPoolConstraints = m_tmpSolverContactFrictionConstraintPool.size();
				for (j = 0; j < numFrictionPoolConstraints; j++)
				{
					SolverConstraint& solveManifold = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[j]];
					Scalar totalImpulse = m_tmpSolverContactConstraintPool[solveManifold.m_frictionIndex].m_appliedImpulse;

					if (totalImpulse > Scalar(0))
					{
						solveManifold.m_lowerLimit = -(solveManifold.m_friction * totalImpulse);
						solveManifold.m_upperLimit = solveManifold.m_friction * totalImpulse;

						Scalar deltaf = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
						m_deltafCF[j] = deltaf;
						deltaflengthsqr += deltaf * deltaf;
					}
					else
					{
						m_deltafCF[j] = 0;
					}
				}
			}

			{
				i32 numRollingFrictionPoolConstraints = m_tmpSolverContactRollingFrictionConstraintPool.size();
				for (i32 j = 0; j < numRollingFrictionPoolConstraints; j++)
				{
					SolverConstraint& rollingFrictionConstraint = m_tmpSolverContactRollingFrictionConstraintPool[j];
					Scalar totalImpulse = m_tmpSolverContactConstraintPool[rollingFrictionConstraint.m_frictionIndex].m_appliedImpulse;
					if (totalImpulse > Scalar(0))
					{
						Scalar rollingFrictionMagnitude = rollingFrictionConstraint.m_friction * totalImpulse;
						if (rollingFrictionMagnitude > rollingFrictionConstraint.m_friction)
							rollingFrictionMagnitude = rollingFrictionConstraint.m_friction;

						rollingFrictionConstraint.m_lowerLimit = -rollingFrictionMagnitude;
						rollingFrictionConstraint.m_upperLimit = rollingFrictionMagnitude;

						Scalar deltaf = resolveSingleConstraintRowGeneric(m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdA], m_tmpSolverBodyPool[rollingFrictionConstraint.m_solverBodyIdB], rollingFrictionConstraint);
						m_deltafCRF[j] = deltaf;
						deltaflengthsqr += deltaf * deltaf;
					}
					else
					{
						m_deltafCRF[j] = 0;
					}
				}
			}
		}
	}

	if (!m_onlyForNoneContact)
	{
		if (iteration == 0)
		{
			for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++) m_pNC[j] = m_deltafNC[j];
			for (i32 j = 0; j < m_tmpSolverContactConstraintPool.size(); j++) m_pC[j] = m_deltafC[j];
			for (i32 j = 0; j < m_tmpSolverContactFrictionConstraintPool.size(); j++) m_pCF[j] = m_deltafCF[j];
			for (i32 j = 0; j < m_tmpSolverContactRollingFrictionConstraintPool.size(); j++) m_pCRF[j] = m_deltafCRF[j];
		}
		else
		{
			// deltaflengthsqrprev can be 0 only if the solver solved the problem exactly in the previous iteration. In this case we should have quit, but mainly for debug reason with this 'hack' it is now allowed to continue the calculation
			Scalar beta = m_deltafLengthSqrPrev > 0 ? deltaflengthsqr / m_deltafLengthSqrPrev : 2;
			if (beta > 1)
			{
				for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++) m_pNC[j] = 0;
				for (i32 j = 0; j < m_tmpSolverContactConstraintPool.size(); j++) m_pC[j] = 0;
				for (i32 j = 0; j < m_tmpSolverContactFrictionConstraintPool.size(); j++) m_pCF[j] = 0;
				for (i32 j = 0; j < m_tmpSolverContactRollingFrictionConstraintPool.size(); j++) m_pCRF[j] = 0;
			}
			else
			{
				for (i32 j = 0; j < m_tmpSolverNonContactConstraintPool.size(); j++)
				{
					SolverConstraint& constraint = m_tmpSolverNonContactConstraintPool[m_orderNonContactConstraintPool[j]];
					if (iteration < constraint.m_overrideNumSolverIterations)
					{
						Scalar additionaldeltaimpulse = beta * m_pNC[j];
						constraint.m_appliedImpulse = Scalar(constraint.m_appliedImpulse) + additionaldeltaimpulse;
						m_pNC[j] = beta * m_pNC[j] + m_deltafNC[j];
						SolverBody& body1 = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
						SolverBody& body2 = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
						const SolverConstraint& c = constraint;
						body1.internalApplyImpulse(c.m_contactNormal1 * body1.internalGetInvMass(), c.m_angularComponentA, additionaldeltaimpulse);
						body2.internalApplyImpulse(c.m_contactNormal2 * body2.internalGetInvMass(), c.m_angularComponentB, additionaldeltaimpulse);
					}
				}
				for (i32 j = 0; j < m_tmpSolverContactConstraintPool.size(); j++)
				{
					SolverConstraint& constraint = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];
					if (iteration < infoGlobal.m_numIterations)
					{
						Scalar additionaldeltaimpulse = beta * m_pC[j];
						constraint.m_appliedImpulse = Scalar(constraint.m_appliedImpulse) + additionaldeltaimpulse;
						m_pC[j] = beta * m_pC[j] + m_deltafC[j];
						SolverBody& body1 = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
						SolverBody& body2 = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
						const SolverConstraint& c = constraint;
						body1.internalApplyImpulse(c.m_contactNormal1 * body1.internalGetInvMass(), c.m_angularComponentA, additionaldeltaimpulse);
						body2.internalApplyImpulse(c.m_contactNormal2 * body2.internalGetInvMass(), c.m_angularComponentB, additionaldeltaimpulse);
					}
				}
				for (i32 j = 0; j < m_tmpSolverContactFrictionConstraintPool.size(); j++)
				{
					SolverConstraint& constraint = m_tmpSolverContactFrictionConstraintPool[m_orderFrictionConstraintPool[j]];
					if (iteration < infoGlobal.m_numIterations)
					{
						Scalar additionaldeltaimpulse = beta * m_pCF[j];
						constraint.m_appliedImpulse = Scalar(constraint.m_appliedImpulse) + additionaldeltaimpulse;
						m_pCF[j] = beta * m_pCF[j] + m_deltafCF[j];
						SolverBody& body1 = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
						SolverBody& body2 = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
						const SolverConstraint& c = constraint;
						body1.internalApplyImpulse(c.m_contactNormal1 * body1.internalGetInvMass(), c.m_angularComponentA, additionaldeltaimpulse);
						body2.internalApplyImpulse(c.m_contactNormal2 * body2.internalGetInvMass(), c.m_angularComponentB, additionaldeltaimpulse);
					}
				}
				{
					for (i32 j = 0; j < m_tmpSolverContactRollingFrictionConstraintPool.size(); j++)
					{
						SolverConstraint& constraint = m_tmpSolverContactRollingFrictionConstraintPool[j];
						if (iteration < infoGlobal.m_numIterations)
						{
							Scalar additionaldeltaimpulse = beta * m_pCRF[j];
							constraint.m_appliedImpulse = Scalar(constraint.m_appliedImpulse) + additionaldeltaimpulse;
							m_pCRF[j] = beta * m_pCRF[j] + m_deltafCRF[j];
							SolverBody& body1 = m_tmpSolverBodyPool[constraint.m_solverBodyIdA];
							SolverBody& body2 = m_tmpSolverBodyPool[constraint.m_solverBodyIdB];
							const SolverConstraint& c = constraint;
							body1.internalApplyImpulse(c.m_contactNormal1 * body1.internalGetInvMass(), c.m_angularComponentA, additionaldeltaimpulse);
							body2.internalApplyImpulse(c.m_contactNormal2 * body2.internalGetInvMass(), c.m_angularComponentB, additionaldeltaimpulse);
						}
					}
				}
			}
		}
		m_deltafLengthSqrPrev = deltaflengthsqr;
	}

	return deltaflengthsqr;
}

Scalar NNCGConstraintSolver::solveGroupCacheFriendlyFinish(CollisionObject2** bodies, i32 numBodies, const ContactSolverInfo& infoGlobal)
{
	m_pNC.resizeNoInitialize(0);
	m_pC.resizeNoInitialize(0);
	m_pCF.resizeNoInitialize(0);
	m_pCRF.resizeNoInitialize(0);

	m_deltafNC.resizeNoInitialize(0);
	m_deltafC.resizeNoInitialize(0);
	m_deltafCF.resizeNoInitialize(0);
	m_deltafCRF.resizeNoInitialize(0);

	return SequentialImpulseConstraintSolver::solveGroupCacheFriendlyFinish(bodies, numBodies, infoGlobal);
}
