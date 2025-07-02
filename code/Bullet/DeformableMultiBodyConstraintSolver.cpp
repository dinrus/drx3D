#include <drx3D/Physics/SoftBody/DeformableMultiBodyConstraintSolver.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodySolver.h>
#include <iostream>

// override the iterations method to include deformable/multibody contact
Scalar DeformableMultiBodyConstraintSolver::solveDeformableGroupIterations(CollisionObject2** bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	{
		// pair deformable body with solver body
		pairDeformableAndSolverBody(bodies, numBodies, numDeformableBodies, infoGlobal);

		///this is a special step to resolve penetrations (just for contacts)
		solveGroupCacheFriendlySplitImpulseIterations(bodies, numBodies, deformableBodies, numDeformableBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);

		i32 maxIterations = m_maxOverrideNumSolverIterations > infoGlobal.m_numIterations ? m_maxOverrideNumSolverIterations : infoGlobal.m_numIterations;
		for (i32 iteration = 0; iteration < maxIterations; iteration++)
		{
			// rigid bodies are solved using solver body velocity, but rigid/deformable contact directly uses the velocity of the actual rigid body. So we have to do the following: Solve one iteration of the rigid/rigid contact, get the updated velocity in the solver body and update the velocity of the underlying rigid body. Then solve the rigid/deformable contact. Finally, grab the (once again) updated rigid velocity and update the velocity of the wrapping solver body

			// solve rigid/rigid in solver body
			m_leastSquaresResidual = solveSingleIteration(iteration, bodies, numBodies, manifoldPtr, numManifolds, constraints, numConstraints, infoGlobal, debugDrawer);
			// solver body velocity -> rigid body velocity
			solverBodyWriteBack(infoGlobal);
			Scalar deformableResidual = m_deformableSolver->solveContactConstraints(deformableBodies, numDeformableBodies, infoGlobal);
			// update rigid body velocity in rigid/deformable contact
			m_leastSquaresResidual = d3Max(m_leastSquaresResidual, deformableResidual);
			// solver body velocity <- rigid body velocity
			writeToSolverBody(bodies, numBodies, infoGlobal);


			// std::cout << "------------Iteration " << iteration << "------------\n";
			// std::cout << "m_leastSquaresResidual: " << m_leastSquaresResidual << "\n";

			if (m_leastSquaresResidual <= infoGlobal.m_leastSquaresResidualThreshold || (iteration >= (maxIterations - 1)))
			{
#ifdef VERBOSE_RESIDUAL_PRINTF
				if (iteration >= (maxIterations - 1))
					printf("residual = %f at iteration #%d\n", m_leastSquaresResidual, iteration);
#endif
				m_analyticsData.m_numSolverCalls++;
				m_analyticsData.m_numIterationsUsed = iteration + 1;
				m_analyticsData.m_islandId = -2;
				if (numBodies > 0)
					m_analyticsData.m_islandId = bodies[0]->getCompanionId();
				m_analyticsData.m_numBodies = numBodies;
				m_analyticsData.m_numContactManifolds = numManifolds;
				m_analyticsData.m_remainingLeastSquaresResidual = m_leastSquaresResidual;
				
				m_deformableSolver->deformableBodyInternalWriteBack();
				// std::cout << "[===================Next Step===================]\n";
				break;
			}
		}
	}
	return 0.f;
}

void DeformableMultiBodyConstraintSolver::solveDeformableBodyGroup(CollisionObject2** bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifold, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, MultiBodyConstraint** multiBodyConstraints, i32 numMultiBodyConstraints, const ContactSolverInfo& info, IDebugDraw* debugDrawer, Dispatcher* dispatcher)
{
	m_tmpMultiBodyConstraints = multiBodyConstraints;
	m_tmpNumMultiBodyConstraints = numMultiBodyConstraints;

	// inherited from MultiBodyConstraintSolver
	solveGroupCacheFriendlySetup(bodies, numBodies, manifold, numManifolds, constraints, numConstraints, info, debugDrawer);

	// overriden
	solveDeformableGroupIterations(bodies, numBodies, deformableBodies, numDeformableBodies, manifold, numManifolds, constraints, numConstraints, info, debugDrawer);

	// inherited from MultiBodyConstraintSolver
	solveGroupCacheFriendlyFinish(bodies, numBodies, info);

	m_tmpMultiBodyConstraints = 0;
	m_tmpNumMultiBodyConstraints = 0;
}

void DeformableMultiBodyConstraintSolver::writeToSolverBody(CollisionObject2** bodies, i32 numBodies, const ContactSolverInfo& infoGlobal)
{
	// reduced soft body solver directly modifies the solver body
	if (m_deformableSolver->isReducedSolver())
	{
		return;
	}

	for (i32 i = 0; i < numBodies; i++)
	{
		i32 bodyId = getOrInitSolverBody(*bodies[i], infoGlobal.m_timeStep);

		RigidBody* body = RigidBody::upcast(bodies[i]);
		if (body && body->getInvMass())
		{
			SolverBody& solverBody = m_tmpSolverBodyPool[bodyId];
			solverBody.m_linearVelocity = body->getLinearVelocity() - solverBody.m_deltaLinearVelocity;
			solverBody.m_angularVelocity = body->getAngularVelocity() - solverBody.m_deltaAngularVelocity;
		}
	}
}

void DeformableMultiBodyConstraintSolver::solverBodyWriteBack(const ContactSolverInfo& infoGlobal)
{
	// reduced soft body solver directly modifies the solver body
	if (m_deformableSolver->isReducedSolver())
	{
		return;
	}

	for (i32 i = 0; i < m_tmpSolverBodyPool.size(); i++)
	{
		RigidBody* body = m_tmpSolverBodyPool[i].m_originalBody;
		if (body)
		{
			m_tmpSolverBodyPool[i].m_originalBody->setLinearVelocity(m_tmpSolverBodyPool[i].m_linearVelocity + m_tmpSolverBodyPool[i].m_deltaLinearVelocity);
			m_tmpSolverBodyPool[i].m_originalBody->setAngularVelocity(m_tmpSolverBodyPool[i].m_angularVelocity + m_tmpSolverBodyPool[i].m_deltaAngularVelocity);
		}
	}
}


void DeformableMultiBodyConstraintSolver::pairDeformableAndSolverBody(CollisionObject2** bodies, i32 numBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal)
{
	if (!m_deformableSolver->isReducedSolver())
	{
		return;
	}

	ReducedDeformableBodySolver* solver = static_cast<ReducedDeformableBodySolver*>(m_deformableSolver);
	
	for (i32 i = 0; i < numDeformableBodies; ++i)
	{
		for (i32 k = 0; k < solver->m_nodeRigidConstraints[i].size(); ++k)
    {
      ReducedDeformableNodeRigidContactConstraint& constraint = solver->m_nodeRigidConstraints[i][k];

			if (!constraint.m_contact->m_cti.m_colObj->isStaticObject())
			{
				CollisionObject2& col_obj = const_cast<CollisionObject2&>(*constraint.m_contact->m_cti.m_colObj);

				// object index in the solver body pool
				i32 bodyId = getOrInitSolverBody(col_obj, infoGlobal.m_timeStep);
				
				const RigidBody* body = RigidBody::upcast(bodies[bodyId]);
				if (body && body->getInvMass())
				{
						// std::cout << "Node: " << constraint.m_node->index << ", body: " << bodyId << "\n";
					SolverBody& solverBody = m_tmpSolverBodyPool[bodyId];
					constraint.setSolverBody(bodyId, solverBody);
				}
			}
    }

		// for (i32 j = 0; j < numBodies; j++)
		// {
		// 	i32 bodyId = getOrInitSolverBody(*bodies[j], infoGlobal.m_timeStep);

		// 	RigidBody* body = RigidBody::upcast(bodies[j]);
		// 	if (body && body->getInvMass())
		// 	{
		// 		SolverBody& solverBody = m_tmpSolverBodyPool[bodyId];
		// 		m_deformableSolver->pairConstraintWithSolverBody(i, bodyId, solverBody);
		// 	}
		// }	
	}
}

void DeformableMultiBodyConstraintSolver::solveGroupCacheFriendlySplitImpulseIterations(CollisionObject2** bodies, i32 numBodies, CollisionObject2** deformableBodies, i32 numDeformableBodies, PersistentManifold** manifoldPtr, i32 numManifolds, TypedConstraint** constraints, i32 numConstraints, const ContactSolverInfo& infoGlobal, IDebugDraw* debugDrawer)
{
	DRX3D_PROFILE("solveGroupCacheFriendlySplitImpulseIterations");
	i32 iteration;
	if (infoGlobal.m_splitImpulse)
	{
		{
			for (iteration = 0; iteration < infoGlobal.m_numIterations; iteration++)
			{
				Scalar leastSquaresResidual = 0.f;
				{
					i32 numPoolConstraints = m_tmpSolverContactConstraintPool.size();
					i32 j;
					for (j = 0; j < numPoolConstraints; j++)
					{
						const SolverConstraint& solveManifold = m_tmpSolverContactConstraintPool[m_orderTmpConstraintPool[j]];

						Scalar residual = resolveSplitPenetrationImpulse(m_tmpSolverBodyPool[solveManifold.m_solverBodyIdA], m_tmpSolverBodyPool[solveManifold.m_solverBodyIdB], solveManifold);
						leastSquaresResidual = d3Max(leastSquaresResidual, residual * residual);
					}
					// solve the position correction between deformable and rigid/multibody
					//                    Scalar residual = m_deformableSolver->solveSplitImpulse(infoGlobal);
					Scalar residual = m_deformableSolver->m_objective->m_projection.solveSplitImpulse(deformableBodies, numDeformableBodies, infoGlobal);
					leastSquaresResidual = d3Max(leastSquaresResidual, residual * residual);
				}
				if (leastSquaresResidual <= infoGlobal.m_leastSquaresResidualThreshold || iteration >= (infoGlobal.m_numIterations - 1))
				{
#ifdef VERBOSE_RESIDUAL_PRINTF
					if (iteration >= (infoGlobal.m_numIterations - 1))
						printf("split impulse residual = %f at iteration #%d\n", leastSquaresResidual, iteration);
#endif
					break;
				}
			}
		}
	}
}
