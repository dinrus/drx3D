#include <drx3D/Physics/SoftBody/DeformableBackwardEulerObjective.h>
#include <drx3D/Physics/SoftBody/Preconditioner.h>
#include <drx3D/Maths/Linear/Quickprof.h>

DeformableBackwardEulerObjective::DeformableBackwardEulerObjective(AlignedObjectArray<SoftBody*>& softBodies, const TVStack& backup_v)
	: m_softBodies(softBodies), m_projection(softBodies), m_backupVelocity(backup_v), m_implicit(false)
{
	m_massPreconditioner = new MassPreconditioner(m_softBodies);
	m_KKTPreconditioner = new KKTPreconditioner(m_softBodies, m_projection, m_lf, m_dt, m_implicit);
	m_preconditioner = m_KKTPreconditioner;
}

DeformableBackwardEulerObjective::~DeformableBackwardEulerObjective()
{
	delete m_KKTPreconditioner;
	delete m_massPreconditioner;
}

void DeformableBackwardEulerObjective::reinitialize(bool nodeUpdated, Scalar dt)
{
	DRX3D_PROFILE("reinitialize");
	if (dt > 0)
	{
		setDt(dt);
	}
	if (nodeUpdated)
	{
		updateId();
	}
	for (i32 i = 0; i < m_lf.size(); ++i)
	{
		m_lf[i]->reinitialize(nodeUpdated);
	}
	Matrix3x3 I;
	I.setIdentity();
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			if (psb->m_nodes[j].m_im > 0)
				psb->m_nodes[j].m_effectiveMass = I * (1.0 / psb->m_nodes[j].m_im);
		}
	}
	m_projection.reinitialize(nodeUpdated);
	//    m_preconditioner->reinitialize(nodeUpdated);
}

void DeformableBackwardEulerObjective::setDt(Scalar dt)
{
	m_dt = dt;
}

void DeformableBackwardEulerObjective::multiply(const TVStack& x, TVStack& b) const
{
	DRX3D_PROFILE("multiply");
	// add in the mass term
	size_t counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			const SoftBody::Node& node = psb->m_nodes[j];
			b[counter] = (node.m_im == 0) ? Vec3(0, 0, 0) : x[counter] / node.m_im;
			++counter;
		}
	}

	for (i32 i = 0; i < m_lf.size(); ++i)
	{
		// add damping matrix
		m_lf[i]->addScaledDampingForceDifferential(-m_dt, x, b);
        // Always integrate picking force implicitly for stability.
        if (m_implicit || m_lf[i]->getForceType() == DRX3D_MOUSE_PICKING_FORCE)
		{
			m_lf[i]->addScaledElasticForceDifferential(-m_dt * m_dt, x, b);
		}
	}
	i32 offset = m_nodes.size();
	for (i32 i = offset; i < b.size(); ++i)
	{
		b[i].setZero();
	}
	// add in the lagrange multiplier terms

	for (i32 c = 0; c < m_projection.m_lagrangeMultipliers.size(); ++c)
	{
		// C^T * lambda
		const LagrangeMultiplier& lm = m_projection.m_lagrangeMultipliers[c];
		for (i32 i = 0; i < lm.m_num_nodes; ++i)
		{
			for (i32 j = 0; j < lm.m_num_constraints; ++j)
			{
				b[lm.m_indices[i]] += x[offset + c][j] * lm.m_weights[i] * lm.m_dirs[j];
			}
		}
		// C * x
		for (i32 d = 0; d < lm.m_num_constraints; ++d)
		{
			for (i32 i = 0; i < lm.m_num_nodes; ++i)
			{
				b[offset + c][d] += lm.m_weights[i] * x[lm.m_indices[i]].dot(lm.m_dirs[d]);
			}
		}
	}
}

void DeformableBackwardEulerObjective::updateVelocity(const TVStack& dv)
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			SoftBody::Node& node = psb->m_nodes[j];
			node.m_v = m_backupVelocity[node.index] + dv[node.index];
		}
	}
}

void DeformableBackwardEulerObjective::applyForce(TVStack& force, bool setZero)
{
	size_t counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			counter += psb->m_nodes.size();
			continue;
		}
		if (m_implicit)
		{
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				if (psb->m_nodes[j].m_im != 0)
				{
					psb->m_nodes[j].m_v += psb->m_nodes[j].m_effectiveMass_inv * force[counter++];
				}
			}
		}
		else
		{
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				Scalar one_over_mass = (psb->m_nodes[j].m_im == 0) ? 0 : psb->m_nodes[j].m_im;
				psb->m_nodes[j].m_v += one_over_mass * force[counter++];
			}
		}
	}
	if (setZero)
	{
		for (i32 i = 0; i < force.size(); ++i)
			force[i].setZero();
	}
}

void DeformableBackwardEulerObjective::computeResidual(Scalar dt, TVStack& residual)
{
	DRX3D_PROFILE("computeResidual");
	// add implicit force
	for (i32 i = 0; i < m_lf.size(); ++i)
	{
        // Always integrate picking force implicitly for stability.
		if (m_implicit || m_lf[i]->getForceType() == DRX3D_MOUSE_PICKING_FORCE)
		{
			m_lf[i]->addScaledForces(dt, residual);
		}
		else
		{
			m_lf[i]->addScaledDampingForce(dt, residual);
		}
	}
	//    m_projection.project(residual);
}

Scalar DeformableBackwardEulerObjective::computeNorm(const TVStack& residual) const
{
	Scalar mag = 0;
	for (i32 i = 0; i < residual.size(); ++i)
	{
		mag += residual[i].length2();
	}
	return std::sqrt(mag);
}

Scalar DeformableBackwardEulerObjective::totalEnergy(Scalar dt)
{
	Scalar e = 0;
	for (i32 i = 0; i < m_lf.size(); ++i)
	{
		e += m_lf[i]->totalEnergy(dt);
	}
	return e;
}

void DeformableBackwardEulerObjective::applyExplicitForce(TVStack& force)
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		m_softBodies[i]->advanceDeformation();
	}
	if (m_implicit)
	{
		// apply forces except gravity force
		Vec3 gravity;
		for (i32 i = 0; i < m_lf.size(); ++i)
		{
			if (m_lf[i]->getForceType() == DRX3D_GRAVITY_FORCE)
			{
				gravity = static_cast<DeformableGravityForce*>(m_lf[i])->m_gravity;
			}
			else
			{
				m_lf[i]->addScaledForces(m_dt, force);
			}
		}
		for (i32 i = 0; i < m_lf.size(); ++i)
		{
			m_lf[i]->addScaledHessian(m_dt);
		}
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (psb->isActive())
			{
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					// add gravity explicitly
					psb->m_nodes[j].m_v += m_dt * psb->m_gravityFactor * gravity;
				}
			}
		}
	}
	else
	{
		for (i32 i = 0; i < m_lf.size(); ++i)
		{
			m_lf[i]->addScaledExplicitForce(m_dt, force);
		}
	}
	// calculate inverse mass matrix for all nodes
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (psb->isActive())
		{
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				if (psb->m_nodes[j].m_im > 0)
				{
					psb->m_nodes[j].m_effectiveMass_inv = psb->m_nodes[j].m_effectiveMass.inverse();
				}
			}
		}
	}
	applyForce(force, true);
}

void DeformableBackwardEulerObjective::initialGuess(TVStack& dv, const TVStack& residual)
{
	size_t counter = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			dv[counter] = psb->m_nodes[j].m_im * residual[counter];
			++counter;
		}
	}
}

//set constraints as projections
void DeformableBackwardEulerObjective::setConstraints(const ContactSolverInfo& infoGlobal)
{
	m_projection.setConstraints(infoGlobal);
}

void DeformableBackwardEulerObjective::applyDynamicFriction(TVStack& r)
{
	m_projection.applyDynamicFriction(r);
}
