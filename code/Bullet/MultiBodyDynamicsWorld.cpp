#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/Serializer.h>

void MultiBodyDynamicsWorld::addMultiBody(MultiBody* body, i32 group, i32 mask)
{
	m_multiBodies.push_back(body);
}

void MultiBodyDynamicsWorld::removeMultiBody(MultiBody* body)
{
	m_multiBodies.remove(body);
}

void MultiBodyDynamicsWorld::predictUnconstraintMotion(Scalar timeStep)
{
    DiscreteDynamicsWorld::predictUnconstraintMotion(timeStep);
    predictMultiBodyTransforms(timeStep);
    
}
void MultiBodyDynamicsWorld::calculateSimulationIslands()
{
	DRX3D_PROFILE("calculateSimulationIslands");

	getSimulationIslandManager()->updateActivationState(getCollisionWorld(), getCollisionWorld()->getDispatcher());

	{
		//merge islands based on speculative contact manifolds too
		for (i32 i = 0; i < this->m_predictiveManifolds.size(); i++)
		{
			PersistentManifold* manifold = m_predictiveManifolds[i];

			const CollisionObject2* colObj0 = manifold->getBody0();
			const CollisionObject2* colObj1 = manifold->getBody1();

			if (((colObj0) && (!(colObj0)->isStaticOrKinematicObject())) &&
				((colObj1) && (!(colObj1)->isStaticOrKinematicObject())))
			{
				getSimulationIslandManager()->getUnionFind().unite((colObj0)->getIslandTag(), (colObj1)->getIslandTag());
			}
		}
	}

	{
		i32 i;
		i32 numConstraints = i32(m_constraints.size());
		for (i = 0; i < numConstraints; i++)
		{
			TypedConstraint* constraint = m_constraints[i];
			if (constraint->isEnabled())
			{
				const RigidBody* colObj0 = &constraint->getRigidBodyA();
				const RigidBody* colObj1 = &constraint->getRigidBodyB();

				if (((colObj0) && (!(colObj0)->isStaticOrKinematicObject())) &&
					((colObj1) && (!(colObj1)->isStaticOrKinematicObject())))
				{
					getSimulationIslandManager()->getUnionFind().unite((colObj0)->getIslandTag(), (colObj1)->getIslandTag());
				}
			}
		}
	}

	//merge islands linked by MultiBody link colliders
	for (i32 i = 0; i < m_multiBodies.size(); i++)
	{
		MultiBody* body = m_multiBodies[i];
		{
			MultiBodyLinkCollider* prev = body->getBaseCollider();

			for (i32 b = 0; b < body->getNumLinks(); b++)
			{
				MultiBodyLinkCollider* cur = body->getLink(b).m_collider;

				if (((cur) && (!(cur)->isStaticOrKinematicObject())) &&
					((prev) && (!(prev)->isStaticOrKinematicObject())))
				{
					i32 tagPrev = prev->getIslandTag();
					i32 tagCur = cur->getIslandTag();
					getSimulationIslandManager()->getUnionFind().unite(tagPrev, tagCur);
				}
				if (cur && !cur->isStaticOrKinematicObject())
					prev = cur;
			}
		}
	}

	//merge islands linked by multibody constraints
	{
		for (i32 i = 0; i < this->m_multiBodyConstraints.size(); i++)
		{
			MultiBodyConstraint* c = m_multiBodyConstraints[i];
			i32 tagA = c->getIslandIdA();
			i32 tagB = c->getIslandIdB();
			if (tagA >= 0 && tagB >= 0)
				getSimulationIslandManager()->getUnionFind().unite(tagA, tagB);
		}
	}

	//Store the island id in each body
	getSimulationIslandManager()->storeIslandActivationState(getCollisionWorld());
}

void MultiBodyDynamicsWorld::updateActivationState(Scalar timeStep)
{
	DRX3D_PROFILE("MultiBodyDynamicsWorld::updateActivationState");

	for (i32 i = 0; i < m_multiBodies.size(); i++)
	{
		MultiBody* body = m_multiBodies[i];
		if (body)
		{
			body->checkMotionAndSleepIfRequired(timeStep);
			if (!body->isAwake())
			{
				MultiBodyLinkCollider* col = body->getBaseCollider();
				if (col && col->getActivationState() == ACTIVE_TAG)
				{
                    if (body->hasFixedBase())
					{
                        col->setActivationState(FIXED_BASE_MULTI_BODY);
                    } else
					{
                        col->setActivationState(WANTS_DEACTIVATION);
                    }
					
					col->setDeactivationTime(0.f);
				}
				for (i32 b = 0; b < body->getNumLinks(); b++)
				{
					MultiBodyLinkCollider* col = body->getLink(b).m_collider;
					if (col && col->getActivationState() == ACTIVE_TAG)
					{
						col->setActivationState(WANTS_DEACTIVATION);
						col->setDeactivationTime(0.f);
					}
				}
			}
			else
			{
				MultiBodyLinkCollider* col = body->getBaseCollider();
				if (col && col->getActivationState() != DISABLE_DEACTIVATION)
					col->setActivationState(ACTIVE_TAG);

				for (i32 b = 0; b < body->getNumLinks(); b++)
				{
					MultiBodyLinkCollider* col = body->getLink(b).m_collider;
					if (col && col->getActivationState() != DISABLE_DEACTIVATION)
						col->setActivationState(ACTIVE_TAG);
				}
			}
		}
	}

	DiscreteDynamicsWorld::updateActivationState(timeStep);
}

void MultiBodyDynamicsWorld::getAnalyticsData(AlignedObjectArray<SolverAnalyticsData>& islandAnalyticsData) const
{
	islandAnalyticsData = m_solverMultiBodyIslandCallback->m_islandAnalyticsData;
}

MultiBodyDynamicsWorld::MultiBodyDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, MultiBodyConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration)
	: DiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration),
	  m_multiBodyConstraintSolver(constraintSolver)
{
	//split impulse is not yet supported for MultiBody hierarchies
	//	getSolverInfo().m_splitImpulse = false;
	getSolverInfo().m_solverMode |= SOLVER_USE_2_FRICTION_DIRECTIONS;
	m_solverMultiBodyIslandCallback = new MultiBodyInplaceSolverIslandCallback(constraintSolver, dispatcher);
}

MultiBodyDynamicsWorld::~MultiBodyDynamicsWorld()
{
	delete m_solverMultiBodyIslandCallback;
}

void MultiBodyDynamicsWorld::setMultiBodyConstraintSolver(MultiBodyConstraintSolver* solver)
{
	m_multiBodyConstraintSolver = solver;
	m_solverMultiBodyIslandCallback->setMultiBodyConstraintSolver(solver);
	DiscreteDynamicsWorld::setConstraintSolver(solver);
}

void MultiBodyDynamicsWorld::setConstraintSolver(ConstraintSolver* solver)
{
	if (solver->getSolverType() == DRX3D_MULTIBODY_SOLVER)
	{
		m_multiBodyConstraintSolver = (MultiBodyConstraintSolver*)solver;
	}
	DiscreteDynamicsWorld::setConstraintSolver(solver);
}

void MultiBodyDynamicsWorld::forwardKinematics()
{
	for (i32 b = 0; b < m_multiBodies.size(); b++)
	{
		MultiBody* bod = m_multiBodies[b];
		bod->forwardKinematics(m_scratch_world_to_local, m_scratch_local_origin);
	}
}
void MultiBodyDynamicsWorld::solveConstraints(ContactSolverInfo& solverInfo)
{
    solveExternalForces(solverInfo);
    buildIslands();
    solveInternalConstraints(solverInfo);
}

void MultiBodyDynamicsWorld::buildIslands()
{
    m_islandManager->buildAndProcessIslands(getCollisionWorld()->getDispatcher(), getCollisionWorld(), m_solverMultiBodyIslandCallback);
}

void MultiBodyDynamicsWorld::solveInternalConstraints(ContactSolverInfo& solverInfo)
{
	/// solve all the constraints for this island
	m_solverMultiBodyIslandCallback->processConstraints();
	m_constraintSolver->allSolved(solverInfo, m_debugDrawer);
    {
        DRX3D_PROFILE("MultiBody stepVelocities");
        for (i32 i = 0; i < this->m_multiBodies.size(); i++)
        {
            MultiBody* bod = m_multiBodies[i];
            
            bool isSleeping = false;
            
            if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
            {
                isSleeping = true;
            }
            for (i32 b = 0; b < bod->getNumLinks(); b++)
            {
                if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                    isSleeping = true;
            }
            
            if (!isSleeping)
            {
                //useless? they get resized in stepVelocities once again (AND DIFFERENTLY)
                m_scratch_r.resize(bod->getNumLinks() + 1);  //multidof? ("Y"s use it and it is used to store qdd)
                m_scratch_v.resize(bod->getNumLinks() + 1);
                m_scratch_m.resize(bod->getNumLinks() + 1);
                
                if (bod->internalNeedsJointFeedback())
                {
                    if (!bod->isUsingRK4Integration())
                    {
                        if (bod->internalNeedsJointFeedback())
                        {
                            bool isConstraintPass = true;
                            bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(solverInfo.m_timeStep, m_scratch_r, m_scratch_v, m_scratch_m, isConstraintPass,
                                                                                      getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                      getSolverInfo().m_jointFeedbackInJointFrame);
                        }
                    }
                }
            }
        }
    }
    for (i32 i = 0; i < this->m_multiBodies.size(); i++)
    {
        MultiBody* bod = m_multiBodies[i];
        bod->processDeltaVeeMultiDof2();
    }
}

void MultiBodyDynamicsWorld::solveExternalForces(ContactSolverInfo& solverInfo)
{
    forwardKinematics();
    
    DRX3D_PROFILE("solveConstraints");
    
    clearMultiBodyConstraintForces();
    
    m_sortedConstraints.resize(m_constraints.size());
    i32 i;
    for (i = 0; i < getNumConstraints(); i++)
    {
        m_sortedConstraints[i] = m_constraints[i];
    }
    m_sortedConstraints.quickSort(SortConstraintOnIslandPredicate2());
    TypedConstraint** constraintsPtr = getNumConstraints() ? &m_sortedConstraints[0] : 0;
    
    m_sortedMultiBodyConstraints.resize(m_multiBodyConstraints.size());
    for (i = 0; i < m_multiBodyConstraints.size(); i++)
    {
        m_sortedMultiBodyConstraints[i] = m_multiBodyConstraints[i];
    }
    m_sortedMultiBodyConstraints.quickSort(SortMultiBodyConstraintOnIslandPredicate());
    
    MultiBodyConstraint** sortedMultiBodyConstraints = m_sortedMultiBodyConstraints.size() ? &m_sortedMultiBodyConstraints[0] : 0;
    
    m_solverMultiBodyIslandCallback->setup(&solverInfo, constraintsPtr, m_sortedConstraints.size(), sortedMultiBodyConstraints, m_sortedMultiBodyConstraints.size(), getDebugDrawer());
    m_constraintSolver->prepareSolve(getCollisionWorld()->getNumCollisionObjects(), getCollisionWorld()->getDispatcher()->getNumManifolds());
    
#ifndef DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
    {
        DRX3D_PROFILE("MultiBody addForce");
        for (i32 i = 0; i < this->m_multiBodies.size(); i++)
        {
            MultiBody* bod = m_multiBodies[i];
            
            bool isSleeping = false;
            
            if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
            {
                isSleeping = true;
            }
            for (i32 b = 0; b < bod->getNumLinks(); b++)
            {
                if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                    isSleeping = true;
            }
            
            if (!isSleeping)
            {
                //useless? they get resized in stepVelocities once again (AND DIFFERENTLY)
                m_scratch_r.resize(bod->getNumLinks() + 1);  //multidof? ("Y"s use it and it is used to store qdd)
                m_scratch_v.resize(bod->getNumLinks() + 1);
                m_scratch_m.resize(bod->getNumLinks() + 1);
                
                bod->addBaseForce(m_gravity * bod->getBaseMass());
                
                for (i32 j = 0; j < bod->getNumLinks(); ++j)
                {
                    bod->addLinkForce(j, m_gravity * bod->getLinkMass(j));
                }
            }  //if (!isSleeping)
        }
    }
#endif  //DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
    
    {
        DRX3D_PROFILE("MultiBody stepVelocities");
        for (i32 i = 0; i < this->m_multiBodies.size(); i++)
        {
            MultiBody* bod = m_multiBodies[i];
            
            bool isSleeping = false;
            
            if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
            {
                isSleeping = true;
            }
            for (i32 b = 0; b < bod->getNumLinks(); b++)
            {
                if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                    isSleeping = true;
            }
            
            if (!isSleeping)
            {
                //useless? they get resized in stepVelocities once again (AND DIFFERENTLY)
                m_scratch_r.resize(bod->getNumLinks() + 1);  //multidof? ("Y"s use it and it is used to store qdd)
                m_scratch_v.resize(bod->getNumLinks() + 1);
                m_scratch_m.resize(bod->getNumLinks() + 1);
                bool doNotUpdatePos = false;
                bool isConstraintPass = false;
                {
                    if (!bod->isUsingRK4Integration())
                    {
                        bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(solverInfo.m_timeStep,
                                                                                  m_scratch_r, m_scratch_v, m_scratch_m,isConstraintPass,
                                                                                  getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                  getSolverInfo().m_jointFeedbackInJointFrame);
                    }
                    else
                    {
                        //
                        i32 numDofs = bod->getNumDofs() + 6;
                        i32 numPosVars = bod->getNumPosVars() + 7;
                        AlignedObjectArray<Scalar> scratch_r2;
                        scratch_r2.resize(2 * numPosVars + 8 * numDofs);
                        //convenience
                        Scalar* pMem = &scratch_r2[0];
                        Scalar* scratch_q0 = pMem;
                        pMem += numPosVars;
                        Scalar* scratch_qx = pMem;
                        pMem += numPosVars;
                        Scalar* scratch_qd0 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qd1 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qd2 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qd3 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qdd0 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qdd1 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qdd2 = pMem;
                        pMem += numDofs;
                        Scalar* scratch_qdd3 = pMem;
                        pMem += numDofs;
                        Assert((pMem - (2 * numPosVars + 8 * numDofs)) == &scratch_r2[0]);
                        
                        /////
                        //copy q0 to scratch_q0 and qd0 to scratch_qd0
                        scratch_q0[0] = bod->getWorldToBaseRot().x();
                        scratch_q0[1] = bod->getWorldToBaseRot().y();
                        scratch_q0[2] = bod->getWorldToBaseRot().z();
                        scratch_q0[3] = bod->getWorldToBaseRot().w();
                        scratch_q0[4] = bod->getBasePos().x();
                        scratch_q0[5] = bod->getBasePos().y();
                        scratch_q0[6] = bod->getBasePos().z();
                        //
                        for (i32 link = 0; link < bod->getNumLinks(); ++link)
                        {
                            for (i32 dof = 0; dof < bod->getLink(link).m_posVarCount; ++dof)
                                scratch_q0[7 + bod->getLink(link).m_cfgOffset + dof] = bod->getLink(link).m_jointPos[dof];
                        }
                        //
                        for (i32 dof = 0; dof < numDofs; ++dof)
                            scratch_qd0[dof] = bod->getVelocityVector()[dof];
                        ////
                        struct
                        {
                            MultiBody* bod;
                            Scalar *scratch_qx, *scratch_q0;
                            
                            void operator()()
                            {
                                for (i32 dof = 0; dof < bod->getNumPosVars() + 7; ++dof)
                                    scratch_qx[dof] = scratch_q0[dof];
                            }
                        } pResetQx = {bod, scratch_qx, scratch_q0};
                        //
                        struct
                        {
                            void operator()(Scalar dt, const Scalar* pDer, const Scalar* pCurVal, Scalar* pVal, i32 size)
                            {
                                for (i32 i = 0; i < size; ++i)
                                    pVal[i] = pCurVal[i] + dt * pDer[i];
                            }
                            
                        } pEulerIntegrate;
                        //
                        struct
                        {
                            void operator()(MultiBody* pBody, const Scalar* pData)
                            {
                                Scalar* pVel = const_cast<Scalar*>(pBody->getVelocityVector());
                                
                                for (i32 i = 0; i < pBody->getNumDofs() + 6; ++i)
                                    pVel[i] = pData[i];
                            }
                        } pCopyToVelocityVector;
                        //
                        struct
                        {
                            void operator()(const Scalar* pSrc, Scalar* pDst, i32 start, i32 size)
                            {
                                for (i32 i = 0; i < size; ++i)
                                    pDst[i] = pSrc[start + i];
                            }
                        } pCopy;
                        //
                        
                        Scalar h = solverInfo.m_timeStep;
#define output &m_scratch_r[bod->getNumDofs()]
                        //calc qdd0 from: q0 & qd0
                        bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(0., m_scratch_r, m_scratch_v, m_scratch_m,
                                                                                  isConstraintPass,getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                  getSolverInfo().m_jointFeedbackInJointFrame);
                        pCopy(output, scratch_qdd0, 0, numDofs);
                        //calc q1 = q0 + h/2 * qd0
                        pResetQx();
                        bod->stepPositionsMultiDof(Scalar(.5) * h, scratch_qx, scratch_qd0);
                        //calc qd1 = qd0 + h/2 * qdd0
                        pEulerIntegrate(Scalar(.5) * h, scratch_qdd0, scratch_qd0, scratch_qd1, numDofs);
                        //
                        //calc qdd1 from: q1 & qd1
                        pCopyToVelocityVector(bod, scratch_qd1);
                        bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(0., m_scratch_r, m_scratch_v, m_scratch_m,
                                                                                  isConstraintPass,getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                  getSolverInfo().m_jointFeedbackInJointFrame);
                        pCopy(output, scratch_qdd1, 0, numDofs);
                        //calc q2 = q0 + h/2 * qd1
                        pResetQx();
                        bod->stepPositionsMultiDof(Scalar(.5) * h, scratch_qx, scratch_qd1);
                        //calc qd2 = qd0 + h/2 * qdd1
                        pEulerIntegrate(Scalar(.5) * h, scratch_qdd1, scratch_qd0, scratch_qd2, numDofs);
                        //
                        //calc qdd2 from: q2 & qd2
                        pCopyToVelocityVector(bod, scratch_qd2);
                        bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(0., m_scratch_r, m_scratch_v, m_scratch_m,
                                                                                  isConstraintPass,getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                  getSolverInfo().m_jointFeedbackInJointFrame);
                        pCopy(output, scratch_qdd2, 0, numDofs);
                        //calc q3 = q0 + h * qd2
                        pResetQx();
                        bod->stepPositionsMultiDof(h, scratch_qx, scratch_qd2);
                        //calc qd3 = qd0 + h * qdd2
                        pEulerIntegrate(h, scratch_qdd2, scratch_qd0, scratch_qd3, numDofs);
                        //
                        //calc qdd3 from: q3 & qd3
                        pCopyToVelocityVector(bod, scratch_qd3);
                        bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(0., m_scratch_r, m_scratch_v, m_scratch_m,
                                                                                  isConstraintPass,getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                  getSolverInfo().m_jointFeedbackInJointFrame);
                        pCopy(output, scratch_qdd3, 0, numDofs);
                        
                        //
                        //calc q = q0 + h/6(qd0 + 2*(qd1 + qd2) + qd3)
                        //calc qd = qd0 + h/6(qdd0 + 2*(qdd1 + qdd2) + qdd3)
                        AlignedObjectArray<Scalar> delta_q;
                        delta_q.resize(numDofs);
                        AlignedObjectArray<Scalar> delta_qd;
                        delta_qd.resize(numDofs);
                        for (i32 i = 0; i < numDofs; ++i)
                        {
                            delta_q[i] = h / Scalar(6.) * (scratch_qd0[i] + 2 * scratch_qd1[i] + 2 * scratch_qd2[i] + scratch_qd3[i]);
                            delta_qd[i] = h / Scalar(6.) * (scratch_qdd0[i] + 2 * scratch_qdd1[i] + 2 * scratch_qdd2[i] + scratch_qdd3[i]);
                            //delta_q[i] = h*scratch_qd0[i];
                            //delta_qd[i] = h*scratch_qdd0[i];
                        }
                        //
                        pCopyToVelocityVector(bod, scratch_qd0);
                        bod->applyDeltaVeeMultiDof(&delta_qd[0], 1);
                        //
                        if (!doNotUpdatePos)
                        {
                            Scalar* pRealBuf = const_cast<Scalar*>(bod->getVelocityVector());
                            pRealBuf += 6 + bod->getNumDofs() + bod->getNumDofs() * bod->getNumDofs();
                            
                            for (i32 i = 0; i < numDofs; ++i)
                                pRealBuf[i] = delta_q[i];
                            
                            //bod->stepPositionsMultiDof(1, 0, &delta_q[0]);
                            bod->setPosUpdated(true);
                        }
                        
                        //ugly hack which resets the cached data to t0 (needed for constraint solver)
                        {
                            for (i32 link = 0; link < bod->getNumLinks(); ++link)
                                bod->getLink(link).updateCacheMultiDof();
                            bod->computeAccelerationsArticulatedBodyAlgorithmMultiDof(0, m_scratch_r, m_scratch_v, m_scratch_m,
                                                                                      isConstraintPass,getSolverInfo().m_jointFeedbackInWorldSpace,
                                                                                      getSolverInfo().m_jointFeedbackInJointFrame);
                        }
                    }
                }
                
#ifndef DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
                bod->clearForcesAndTorques();
#endif         //DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
            }  //if (!isSleeping)
        }
    }
}


void MultiBodyDynamicsWorld::integrateTransforms(Scalar timeStep)
{
	DiscreteDynamicsWorld::integrateTransforms(timeStep);
    integrateMultiBodyTransforms(timeStep);
}

void MultiBodyDynamicsWorld::integrateMultiBodyTransforms(Scalar timeStep)
{
		DRX3D_PROFILE("MultiBody stepPositions");
		//integrate and update the MultiBody hierarchies

		for (i32 b = 0; b < m_multiBodies.size(); b++)
		{
			MultiBody* bod = m_multiBodies[b];
			bool isSleeping = false;
			if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
			{
				isSleeping = true;
			}
			for (i32 b = 0; b < bod->getNumLinks(); b++)
			{
				if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
					isSleeping = true;
			}

			if (!isSleeping)
			{
				bod->addSplitV();
				i32 nLinks = bod->getNumLinks();

				///base + num m_links
                if (!bod->isPosUpdated())
                    bod->stepPositionsMultiDof(timeStep);
                else
                {
                    Scalar* pRealBuf = const_cast<Scalar*>(bod->getVelocityVector());
                    pRealBuf += 6 + bod->getNumDofs() + bod->getNumDofs() * bod->getNumDofs();

                    bod->stepPositionsMultiDof(1, 0, pRealBuf);
                    bod->setPosUpdated(false);
                }


				m_scratch_world_to_local.resize(nLinks + 1);
				m_scratch_local_origin.resize(nLinks + 1);
                bod->updateCollisionObjectWorldTransforms(m_scratch_world_to_local, m_scratch_local_origin);
				bod->substractSplitV();
			}
			else
			{
				bod->clearVelocities();
			}
		}
}

void MultiBodyDynamicsWorld::predictMultiBodyTransforms(Scalar timeStep)
{
    DRX3D_PROFILE("MultiBody stepPositions");
    //integrate and update the MultiBody hierarchies
    
    for (i32 b = 0; b < m_multiBodies.size(); b++)
    {
        MultiBody* bod = m_multiBodies[b];
        bool isSleeping = false;
        if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
        {
            isSleeping = true;
        }
        for (i32 b = 0; b < bod->getNumLinks(); b++)
        {
            if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
                isSleeping = true;
        }
        
        if (!isSleeping)
        {
            i32 nLinks = bod->getNumLinks();
            bod->predictPositionsMultiDof(timeStep);
            m_scratch_world_to_local.resize(nLinks + 1);
            m_scratch_local_origin.resize(nLinks + 1);
            bod->updateCollisionObject2InterpolationWorldTransforms(m_scratch_world_to_local, m_scratch_local_origin);
        }
        else
        {
            bod->clearVelocities();
        }
    }
}

void MultiBodyDynamicsWorld::addMultiBodyConstraint(MultiBodyConstraint* constraint)
{
	m_multiBodyConstraints.push_back(constraint);
}

void MultiBodyDynamicsWorld::removeMultiBodyConstraint(MultiBodyConstraint* constraint)
{
	m_multiBodyConstraints.remove(constraint);
}

void MultiBodyDynamicsWorld::debugDrawMultiBodyConstraint(MultiBodyConstraint* constraint)
{
	constraint->debugDraw(getDebugDrawer());
}

void MultiBodyDynamicsWorld::debugDrawWorld()
{
	DRX3D_PROFILE("MultiBodyDynamicsWorld debugDrawWorld");

	DiscreteDynamicsWorld::debugDrawWorld();

	bool drawConstraints = false;
	if (getDebugDrawer())
	{
		i32 mode = getDebugDrawer()->getDebugMode();
		if (mode & (IDebugDraw::DBG_DrawConstraints | IDebugDraw::DBG_DrawConstraintLimits))
		{
			drawConstraints = true;
		}

		if (drawConstraints)
		{
			DRX3D_PROFILE("MultiBody debugDrawWorld");

			for (i32 c = 0; c < m_multiBodyConstraints.size(); c++)
			{
				MultiBodyConstraint* constraint = m_multiBodyConstraints[c];
				debugDrawMultiBodyConstraint(constraint);
			}

			for (i32 b = 0; b < m_multiBodies.size(); b++)
			{
				MultiBody* bod = m_multiBodies[b];
				bod->forwardKinematics(m_scratch_world_to_local1, m_scratch_local_origin1);

				if (mode & IDebugDraw::DBG_DrawFrames)
				{
					getDebugDrawer()->drawTransform2(bod->getBaseWorldTransform(), 0.1);
				}

				for (i32 m = 0; m < bod->getNumLinks(); m++)
				{
					const Transform2& tr = bod->getLink(m).m_cachedWorldTransform;
					if (mode & IDebugDraw::DBG_DrawFrames)
					{
						getDebugDrawer()->drawTransform2(tr, 0.1);
					}
					//draw the joint axis
					if (bod->getLink(m).m_jointType == MultibodyLink::eRevolute)
					{
						Vec3 vec = quatRotate(tr.getRotation(), bod->getLink(m).m_axes[0].m_topVec) * 0.1;

						Vec4 color(0, 0, 0, 1);  //1,1,1);
						Vec3 from = vec + tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						Vec3 to = tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						getDebugDrawer()->drawLine(from, to, color);
					}
					if (bod->getLink(m).m_jointType == MultibodyLink::eFixed)
					{
						Vec3 vec = quatRotate(tr.getRotation(), bod->getLink(m).m_axes[0].m_bottomVec) * 0.1;

						Vec4 color(0, 0, 0, 1);  //1,1,1);
						Vec3 from = vec + tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						Vec3 to = tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						getDebugDrawer()->drawLine(from, to, color);
					}
					if (bod->getLink(m).m_jointType == MultibodyLink::ePrismatic)
					{
						Vec3 vec = quatRotate(tr.getRotation(), bod->getLink(m).m_axes[0].m_bottomVec) * 0.1;

						Vec4 color(0, 0, 0, 1);  //1,1,1);
						Vec3 from = vec + tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						Vec3 to = tr.getOrigin() - quatRotate(tr.getRotation(), bod->getLink(m).m_dVector);
						getDebugDrawer()->drawLine(from, to, color);
					}
				}
			}
		}
	}
}

void MultiBodyDynamicsWorld::applyGravity()
{
	DiscreteDynamicsWorld::applyGravity();
#ifdef DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
	DRX3D_PROFILE("MultiBody addGravity");
	for (i32 i = 0; i < this->m_multiBodies.size(); i++)
	{
		MultiBody* bod = m_multiBodies[i];

		bool isSleeping = false;

		if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
		{
			isSleeping = true;
		}
		for (i32 b = 0; b < bod->getNumLinks(); b++)
		{
			if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
				isSleeping = true;
		}

		if (!isSleeping)
		{
			bod->addBaseForce(m_gravity * bod->getBaseMass());

			for (i32 j = 0; j < bod->getNumLinks(); ++j)
			{
				bod->addLinkForce(j, m_gravity * bod->getLinkMass(j));
			}
		}  //if (!isSleeping)
	}
#endif  //DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
}

void MultiBodyDynamicsWorld::clearMultiBodyConstraintForces()
{
	for (i32 i = 0; i < this->m_multiBodies.size(); i++)
	{
		MultiBody* bod = m_multiBodies[i];
		bod->clearConstraintForces();
	}
}
void MultiBodyDynamicsWorld::clearMultiBodyForces()
{
	{
		// DRX3D_PROFILE("clearMultiBodyForces");
		for (i32 i = 0; i < this->m_multiBodies.size(); i++)
		{
			MultiBody* bod = m_multiBodies[i];

			bool isSleeping = false;

			if (bod->getBaseCollider() && bod->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
			{
				isSleeping = true;
			}
			for (i32 b = 0; b < bod->getNumLinks(); b++)
			{
				if (bod->getLink(b).m_collider && bod->getLink(b).m_collider->getActivationState() == ISLAND_SLEEPING)
					isSleeping = true;
			}

			if (!isSleeping)
			{
				MultiBody* bod = m_multiBodies[i];
				bod->clearForcesAndTorques();
			}
		}
	}
}
void MultiBodyDynamicsWorld::clearForces()
{
	DiscreteDynamicsWorld::clearForces();

#ifdef DRX3D_USE_VIRTUAL_CLEARFORCES_AND_GRAVITY
	clearMultiBodyForces();
#endif
}

void MultiBodyDynamicsWorld::serialize(Serializer* serializer)
{
	serializer->startSerialization();

	serializeDynamicsWorldInfo(serializer);

	serializeMultiBodies(serializer);

	serializeRigidBodies(serializer);

	serializeCollisionObjects(serializer);

	serializeContactManifolds(serializer);

	serializer->finishSerialization();
}

void MultiBodyDynamicsWorld::serializeMultiBodies(Serializer* serializer)
{
	i32 i;
	//serialize all collision objects
	for (i = 0; i < m_multiBodies.size(); i++)
	{
		MultiBody* mb = m_multiBodies[i];
		{
			i32 len = mb->calculateSerializeBufferSize();
			Chunk* chunk = serializer->allocate(len, 1);
			tukk structType = mb->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_MULTIBODY_CODE, mb);
		}
	}

	//serialize all multibody links (collision objects)
	for (i = 0; i < m_collisionObjects.size(); i++)
	{
		CollisionObject2* colObj = m_collisionObjects[i];
		if (colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
		{
			i32 len = colObj->calculateSerializeBufferSize();
			Chunk* chunk = serializer->allocate(len, 1);
			tukk structType = colObj->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk, structType, DRX3D_MB_LINKCOLLIDER_CODE, colObj);
		}
	}
}

void MultiBodyDynamicsWorld::saveKinematicState(Scalar timeStep)
{
	DiscreteDynamicsWorld::saveKinematicState(timeStep);
	for(i32 i = 0; i < m_multiBodies.size(); i++)
	{
		MultiBody* body = m_multiBodies[i];
		if(body->isBaseKinematic())
			body->saveKinematicState(timeStep);
	}
}

//
//void MultiBodyDynamicsWorld::setSplitIslands(bool split)
//{
//    m_islandManager->setSplitIslands(split);
//}
