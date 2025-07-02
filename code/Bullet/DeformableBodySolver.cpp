#include <stdio.h>
#include <limits>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>
#include <drx3D/Maths/Linear/Quickprof.h>
static i32k kMaxConjugateGradientIterations = 300;
DeformableBodySolver::DeformableBodySolver()
	: m_numNodes(0), m_cg(kMaxConjugateGradientIterations), m_cr(kMaxConjugateGradientIterations), m_maxNewtonIterations(1), m_newtonTolerance(1e-4), m_lineSearch(false), m_useProjection(false)
{
	m_objective = new DeformableBackwardEulerObjective(m_softBodies, m_backupVelocity);
	m_reducedSolver = false;
}

DeformableBodySolver::~DeformableBodySolver()
{
	delete m_objective;
}

void DeformableBodySolver::solveDeformableConstraints(Scalar solverdt)
{
	DRX3D_PROFILE("solveDeformableConstraints");
	if (!m_implicit)
	{
		m_objective->computeResidual(solverdt, m_residual);
		m_objective->applyDynamicFriction(m_residual);
		if (m_useProjection)
		{
			computeStep(m_dv, m_residual);
		}
		else
		{
			TVStack rhs, x;
			m_objective->addLagrangeMultiplierRHS(m_residual, m_dv, rhs);
			m_objective->addLagrangeMultiplier(m_dv, x);
			m_objective->m_preconditioner->reinitialize(true);
			computeStep(x, rhs);
			for (i32 i = 0; i < m_dv.size(); ++i)
			{
				m_dv[i] = x[i];
			}
		}
		updateVelocity();
	}
	else
	{
		for (i32 i = 0; i < m_maxNewtonIterations; ++i)
		{
			updateState();
			// add the inertia term in the residual
			i32 counter = 0;
			for (i32 k = 0; k < m_softBodies.size(); ++k)
			{
				SoftBody* psb = m_softBodies[k];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					if (psb->m_nodes[j].m_im > 0)
					{
						m_residual[counter] = (-1. / psb->m_nodes[j].m_im) * m_dv[counter];
					}
					++counter;
				}
			}

			m_objective->computeResidual(solverdt, m_residual);
			if (m_objective->computeNorm(m_residual) < m_newtonTolerance && i > 0)
			{
				break;
			}
			// todo xuchenhan@: this really only needs to be calculated once
			m_objective->applyDynamicFriction(m_residual);
			if (m_lineSearch)
			{
				Scalar inner_product = computeDescentStep(m_ddv, m_residual);
				Scalar alpha = 0.01, beta = 0.5;  // Boyd & Vandenberghe suggested alpha between 0.01 and 0.3, beta between 0.1 to 0.8
				Scalar scale = 2;
				Scalar f0 = m_objective->totalEnergy(solverdt) + kineticEnergy(), f1, f2;
				backupDv();
				do
				{
					scale *= beta;
					if (scale < 1e-8)
					{
						return;
					}
					updateEnergy(scale);
					f1 = m_objective->totalEnergy(solverdt) + kineticEnergy();
					f2 = f0 - alpha * scale * inner_product;
				} while (!(f1 < f2 + SIMD_EPSILON));  // if anything here is nan then the search continues
				revertDv();
				updateDv(scale);
			}
			else
			{
				computeStep(m_ddv, m_residual);
				updateDv();
			}
			for (i32 j = 0; j < m_numNodes; ++j)
			{
				m_ddv[j].setZero();
				m_residual[j].setZero();
			}
		}
		updateVelocity();
	}
}

Scalar DeformableBodySolver::kineticEnergy()
{
	Scalar ke = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			SoftBody::Node& node = psb->m_nodes[j];
			if (node.m_im > 0)
			{
				ke += m_dv[node.index].length2() * 0.5 / node.m_im;
			}
		}
	}
	return ke;
}

void DeformableBodySolver::backupDv()
{
	m_backup_dv.resize(m_dv.size());
	for (i32 i = 0; i < m_backup_dv.size(); ++i)
	{
		m_backup_dv[i] = m_dv[i];
	}
}

void DeformableBodySolver::revertDv()
{
	for (i32 i = 0; i < m_backup_dv.size(); ++i)
	{
		m_dv[i] = m_backup_dv[i];
	}
}

void DeformableBodySolver::updateEnergy(Scalar scale)
{
	for (i32 i = 0; i < m_dv.size(); ++i)
	{
		m_dv[i] = m_backup_dv[i] + scale * m_ddv[i];
	}
	updateState();
}

Scalar DeformableBodySolver::computeDescentStep(TVStack& ddv, const TVStack& residual, bool verbose)
{
	m_cg.solve(*m_objective, ddv, residual, false);
	Scalar inner_product = m_cg.dot(residual, m_ddv);
	Scalar res_norm = m_objective->computeNorm(residual);
	Scalar tol = 1e-5 * res_norm * m_objective->computeNorm(m_ddv);
	if (inner_product < -tol)
	{
		if (verbose)
		{
			std::cout << "Looking backwards!" << std::endl;
		}
		for (i32 i = 0; i < m_ddv.size(); ++i)
		{
			m_ddv[i] = -m_ddv[i];
		}
		inner_product = -inner_product;
	}
	else if (std::abs(inner_product) < tol)
	{
		if (verbose)
		{
			std::cout << "Gradient Descent!" << std::endl;
		}
		Scalar scale = m_objective->computeNorm(m_ddv) / res_norm;
		for (i32 i = 0; i < m_ddv.size(); ++i)
		{
			m_ddv[i] = scale * residual[i];
		}
		inner_product = scale * res_norm * res_norm;
	}
	return inner_product;
}

void DeformableBodySolver::updateState()
{
	updateVelocity();
	updateTempPosition();
}

void DeformableBodySolver::updateDv(Scalar scale)
{
	for (i32 i = 0; i < m_numNodes; ++i)
	{
		m_dv[i] += scale * m_ddv[i];
	}
}

void DeformableBodySolver::computeStep(TVStack& ddv, const TVStack& residual)
{
	if (m_useProjection)
		m_cg.solve(*m_objective, ddv, residual, false);
	else
		m_cr.solve(*m_objective, ddv, residual, false);
}

void DeformableBodySolver::reinitialize(const AlignedObjectArray<SoftBody*>& softBodies, Scalar dt)
{
	m_softBodies.copyFromArray(softBodies);
	bool nodeUpdated = updateNodes();

	if (nodeUpdated)
	{
		m_dv.resize(m_numNodes, Vec3(0, 0, 0));
		m_ddv.resize(m_numNodes, Vec3(0, 0, 0));
		m_residual.resize(m_numNodes, Vec3(0, 0, 0));
		m_backupVelocity.resize(m_numNodes, Vec3(0, 0, 0));
	}

	// need to setZero here as resize only set value for newly allocated items
	for (i32 i = 0; i < m_numNodes; ++i)
	{
		m_dv[i].setZero();
		m_ddv[i].setZero();
		m_residual[i].setZero();
	}

	if (dt > 0)
	{
		m_dt = dt;
	}
	m_objective->reinitialize(nodeUpdated, dt);
	updateSoftBodies();
}

void DeformableBodySolver::setConstraints(const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("setConstraint");
	m_objective->setConstraints(infoGlobal);
}

Scalar DeformableBodySolver::solveContactConstraints(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("solveContactConstraints");
	Scalar maxSquaredResidual = m_objective->m_projection.update(deformableBodies, numDeformableBodies, infoGlobal);
	return maxSquaredResidual;
}

void DeformableBodySolver::updateVelocity()
{
	i32 counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		psb->m_maxSpeedSquared = 0;
		if (!psb->isActive())
		{
			counter += psb->m_nodes.size();
			continue;
		}
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			// set NaN to zero;
			if (m_dv[counter] != m_dv[counter])
			{
				m_dv[counter].setZero();
			}
			if (m_implicit)
			{
				psb->m_nodes[j].m_v = m_backupVelocity[counter] + m_dv[counter];
			}
			else
			{
				psb->m_nodes[j].m_v = m_backupVelocity[counter] + m_dv[counter] - psb->m_nodes[j].m_splitv;
			}
			psb->m_maxSpeedSquared = d3Max(psb->m_maxSpeedSquared, psb->m_nodes[j].m_v.length2());
			++counter;
		}
	}
}

void DeformableBodySolver::updateTempPosition()
{
	i32 counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			counter += psb->m_nodes.size();
			continue;
		}
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			psb->m_nodes[j].m_q = psb->m_nodes[j].m_x + m_dt * (psb->m_nodes[j].m_v + psb->m_nodes[j].m_splitv);
			++counter;
		}
		psb->updateDeformation();
	}
}

void DeformableBodySolver::backupVelocity()
{
	i32 counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			m_backupVelocity[counter++] = psb->m_nodes[j].m_v;
		}
	}
}

void DeformableBodySolver::setupDeformableSolve(bool implicit)
{
	i32 counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			counter += psb->m_nodes.size();
			continue;
		}
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			if (implicit)
			{
				// setting the initial guess for newton, need m_dv = v_{n+1} - v_n for dofs that are in constraint.
				if (psb->m_nodes[j].m_v == m_backupVelocity[counter])
					m_dv[counter].setZero();
				else
					m_dv[counter] = psb->m_nodes[j].m_v - psb->m_nodes[j].m_vn;
				m_backupVelocity[counter] = psb->m_nodes[j].m_vn;
			}
			else
			{
				m_dv[counter] = psb->m_nodes[j].m_v + psb->m_nodes[j].m_splitv - m_backupVelocity[counter];
			}
			psb->m_nodes[j].m_v = m_backupVelocity[counter];
			++counter;
		}
	}
}

void DeformableBodySolver::revertVelocity()
{
	i32 counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			psb->m_nodes[j].m_v = m_backupVelocity[counter++];
		}
	}
}

bool DeformableBodySolver::updateNodes()
{
	i32 numNodes = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
		numNodes += m_softBodies[i]->m_nodes.size();
	if (numNodes != m_numNodes)
	{
		m_numNodes = numNodes;
		return true;
	}
	return false;
}

void DeformableBodySolver::predictMotion(Scalar solverdt)
{
	// apply explicit forces to velocity
	if (m_implicit)
	{
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (psb->isActive())
			{
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = psb->m_nodes[j].m_x + psb->m_nodes[j].m_v * solverdt;
				}
			}
		}
	}
	applyExplicitForce();
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (psb->isActive())
		{
			/* Clear contacts when softbody is active*/
			psb->m_nodeRigidContacts.resize(0);
			psb->m_faceRigidContacts.resize(0);
			psb->m_faceNodeContacts.resize(0);
			psb->m_faceNodeContactsCCD.resize(0);
			// predict motion for collision detection
			predictDeformableMotion(psb, solverdt);
		}
	}
}

void DeformableBodySolver::predictDeformableMotion(SoftBody* psb, Scalar dt)
{
	DRX3D_PROFILE("DeformableBodySolver::predictDeformableMotion");
	i32 i, ni;

	/* Update                */
	if (psb->m_bUpdateRtCst)
	{
		psb->m_bUpdateRtCst = false;
		psb->updateConstants();
		psb->m_fdbvt.clear();
		if (psb->m_cfg.collisions & SoftBody::fCollision::SDF_RD)
		{
			psb->initializeFaceTree();
		}
	}

	/* Prepare                */
	psb->m_sst.sdt = dt * psb->m_cfg.timescale;
	psb->m_sst.isdt = 1 / psb->m_sst.sdt;
	psb->m_sst.velmrg = psb->m_sst.sdt * 3;
	psb->m_sst.radmrg = psb->getCollisionShape()->getMargin();
	psb->m_sst.updmrg = psb->m_sst.radmrg * (Scalar)0.25;
	/* Bounds                */
	psb->updateBounds();

	/* Integrate            */
	// do not allow particles to move more than the bounding box size
	Scalar max_v = (psb->m_bounds[1] - psb->m_bounds[0]).norm() / dt;
	for (i = 0, ni = psb->m_nodes.size(); i < ni; ++i)
	{
		SoftBody::Node& n = psb->m_nodes[i];
		// apply drag
		n.m_v *= (1 - psb->m_cfg.drag);
		// scale velocity back
		if (m_implicit)
		{
			n.m_q = n.m_x;
		}
		else
		{
			if (n.m_v.norm() > max_v)
			{
				n.m_v.safeNormalize();
				n.m_v *= max_v;
			}
			n.m_q = n.m_x + n.m_v * dt;
		}
		n.m_splitv.setZero();
		n.m_constrained = false;
	}

	/* Nodes                */
	psb->updateNodeTree(true, true);
	if (!psb->m_fdbvt.empty())
	{
		psb->updateFaceTree(true, true);
	}
	/* Optimize dbvt's        */
	//    psb->m_ndbvt.optimizeIncremental(1);
	//    psb->m_fdbvt.optimizeIncremental(1);
}

void DeformableBodySolver::updateSoftBodies()
{
	DRX3D_PROFILE("updateSoftBodies");
	for (i32 i = 0; i < m_softBodies.size(); i++)
	{
		SoftBody* psb = (SoftBody*)m_softBodies[i];
		if (psb->isActive())
		{
			psb->updateNormals();
		}
	}
}

void DeformableBodySolver::setImplicit(bool implicit)
{
	m_implicit = implicit;
	m_objective->setImplicit(implicit);
}

void DeformableBodySolver::setLineSearch(bool lineSearch)
{
	m_lineSearch = lineSearch;
}

void DeformableBodySolver::applyExplicitForce()
{
	m_objective->applyExplicitForce(m_residual);
}

void DeformableBodySolver::applyTransforms(Scalar timeStep)
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			SoftBody::Node& node = psb->m_nodes[j];
			Scalar maxDisplacement = psb->getWorldInfo()->m_maxDisplacement;
			Scalar clampDeltaV = maxDisplacement / timeStep;
			for (i32 c = 0; c < 3; c++)
			{
				if (node.m_v[c] > clampDeltaV)
				{
					node.m_v[c] = clampDeltaV;
				}
				if (node.m_v[c] < -clampDeltaV)
				{
					node.m_v[c] = -clampDeltaV;
				}
			}
			node.m_x = node.m_x + timeStep * (node.m_v + node.m_splitv);
			node.m_q = node.m_x;
			node.m_vn = node.m_v;
		}
		// enforce anchor constraints
		for (i32 j = 0; j < psb->m_deformableAnchors.size(); ++j)
		{
			SoftBody::DeformableNodeRigidAnchor& a = psb->m_deformableAnchors[j];
			SoftBody::Node* n = a.m_node;
			n->m_x = a.m_cti.m_colObj->getWorldTransform() * a.m_local;

			// update multibody anchor info
			if (a.m_cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
			{
				MultiBodyLinkCollider* multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(a.m_cti.m_colObj);
				if (multibodyLinkCol)
				{
					Vec3 nrm;
					const CollisionShape* shp = multibodyLinkCol->getCollisionShape();
					const Transform2& wtr = multibodyLinkCol->getWorldTransform();
					psb->m_worldInfo->m_sparsesdf.Evaluate(
						wtr.invXform(n->m_x),
						shp,
						nrm,
						0);
					a.m_cti.m_normal = wtr.getBasis() * nrm;
					Vec3 normal = a.m_cti.m_normal;
					Vec3 t1 = generateUnitOrthogonalVector(normal);
					Vec3 t2 = Cross(normal, t1);
					MultiBodyJacobianData jacobianData_normal, jacobianData_t1, jacobianData_t2;
					findJacobian(multibodyLinkCol, jacobianData_normal, a.m_node->m_x, normal);
					findJacobian(multibodyLinkCol, jacobianData_t1, a.m_node->m_x, t1);
					findJacobian(multibodyLinkCol, jacobianData_t2, a.m_node->m_x, t2);

					Scalar* J_n = &jacobianData_normal.m_jacobians[0];
					Scalar* J_t1 = &jacobianData_t1.m_jacobians[0];
					Scalar* J_t2 = &jacobianData_t2.m_jacobians[0];

					Scalar* u_n = &jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
					Scalar* u_t1 = &jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
					Scalar* u_t2 = &jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];

					Matrix3x3 rot(normal.getX(), normal.getY(), normal.getZ(),
									t1.getX(), t1.getY(), t1.getZ(),
									t2.getX(), t2.getY(), t2.getZ());  // world frame to local frame
					i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
					Matrix3x3 local_impulse_matrix = (Diagonal(n->m_im) + OuterProduct(J_n, J_t1, J_t2, u_n, u_t1, u_t2, ndof)).inverse();
					a.m_c0 = rot.transpose() * local_impulse_matrix * rot;
					a.jacobianData_normal = jacobianData_normal;
					a.jacobianData_t1 = jacobianData_t1;
					a.jacobianData_t2 = jacobianData_t2;
					a.t1 = t1;
					a.t2 = t2;
				}
			}
		}
		psb->interpolateRenderMesh();
	}
}