#ifndef DRX3D_LINEAR_ELASTICITY_H
#define DRX3D_LINEAR_ELASTICITY_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>
#define TETRA_FLAT_THRESHOLD 0.01
class DeformableLinearElasticityForce : public DeformableLagrangianForce
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	Scalar m_mu, m_lambda;
	Scalar m_E, m_nu;  // Young's modulus and Poisson ratio
	Scalar m_damping_alpha, m_damping_beta;
	DeformableLinearElasticityForce() : m_mu(1), m_lambda(1), m_damping_alpha(0.01), m_damping_beta(0.01)
	{
		updateYoungsModulusAndPoissonRatio();
	}

	DeformableLinearElasticityForce(Scalar mu, Scalar lambda, Scalar damping_alpha = 0.01, Scalar damping_beta = 0.01) : m_mu(mu), m_lambda(lambda), m_damping_alpha(damping_alpha), m_damping_beta(damping_beta)
	{
		updateYoungsModulusAndPoissonRatio();
	}

	void updateYoungsModulusAndPoissonRatio()
	{
		// conversion from Lame Parameters to Young's modulus and Poisson ratio
		// https://en.wikipedia.org/wiki/Lam%C3%A9_parameters
		m_E = m_mu * (3 * m_lambda + 2 * m_mu) / (m_lambda + m_mu);
		m_nu = m_lambda * 0.5 / (m_mu + m_lambda);
	}

	void updateLameParameters()
	{
		// conversion from Young's modulus and Poisson ratio to Lame Parameters
		// https://en.wikipedia.org/wiki/Lam%C3%A9_parameters
		m_mu = m_E * 0.5 / (1 + m_nu);
		m_lambda = m_E * m_nu / ((1 + m_nu) * (1 - 2 * m_nu));
	}

	void setYoungsModulus(Scalar E)
	{
		m_E = E;
		updateLameParameters();
	}

	void setPoissonRatio(Scalar nu)
	{
		m_nu = nu;
		updateLameParameters();
	}

	void setDamping(Scalar damping_alpha, Scalar damping_beta)
	{
		m_damping_alpha = damping_alpha;
		m_damping_beta = damping_beta;
	}

	void setLameParameters(Scalar mu, Scalar lambda)
	{
		m_mu = mu;
		m_lambda = lambda;
		updateYoungsModulusAndPoissonRatio();
	}

	virtual void addScaledForces(Scalar scale, TVStack& force)
	{
		addScaledDampingForce(scale, force);
		addScaledElasticForce(scale, force);
	}

	virtual void addScaledExplicitForce(Scalar scale, TVStack& force)
	{
		addScaledElasticForce(scale, force);
	}

	// The damping matrix is calculated using the time n state as described in https://www.math.ucla.edu/~jteran/papers/GSSJT15.pdf to allow line search
	virtual void addScaledDampingForce(Scalar scale, TVStack& force)
	{
		if (m_damping_alpha == 0 && m_damping_beta == 0)
			return;
		Scalar mu_damp = m_damping_beta * m_mu;
		Scalar lambda_damp = m_damping_beta * m_lambda;
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				bool close_to_flat = (psb->m_tetraScratches[j].m_J < TETRA_FLAT_THRESHOLD);
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				size_t id0 = node0->index;
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				size_t id3 = node3->index;
				Matrix3x3 dF = DsFromVelocity(node0, node1, node2, node3) * tetra.m_Dm_inverse;
				if (!close_to_flat)
				{
					dF = psb->m_tetraScratches[j].m_corotation.transpose() * dF;
				}
				Matrix3x3 I;
				I.setIdentity();
				Matrix3x3 dP = (dF + dF.transpose()) * mu_damp + I * ((dF[0][0] + dF[1][1] + dF[2][2]) * lambda_damp);
				Matrix3x3 df_on_node123 = dP * tetra.m_Dm_inverse.transpose();
				if (!close_to_flat)
				{
					df_on_node123 = psb->m_tetraScratches[j].m_corotation * df_on_node123;
				}
				Vec3 df_on_node0 = df_on_node123 * grad_N_hat_1st_col;
				// damping force differential
				Scalar scale1 = scale * tetra.m_element_measure;
				force[id0] -= scale1 * df_on_node0;
				force[id1] -= scale1 * df_on_node123.getColumn(0);
				force[id2] -= scale1 * df_on_node123.getColumn(1);
				force[id3] -= scale1 * df_on_node123.getColumn(2);
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				size_t id = node.index;
				if (node.m_im > 0)
				{
					force[id] -= scale * node.m_v / node.m_im * m_damping_alpha;
				}
			}
		}
	}

	virtual double totalElasticEnergy(Scalar dt)
	{
		double energy = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_tetraScratches.size(); ++j)
			{
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				SoftBody::TetraScratch& s = psb->m_tetraScratches[j];
				energy += tetra.m_element_measure * elasticEnergyDensity(s);
			}
		}
		return energy;
	}

	// The damping energy is formulated as in https://www.math.ucla.edu/~jteran/papers/GSSJT15.pdf to allow line search
	virtual double totalDampingEnergy(Scalar dt)
	{
		double energy = 0;
		i32 sz = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				sz = d3Max(sz, psb->m_nodes[j].index);
			}
		}
		TVStack dampingForce;
		dampingForce.resize(sz + 1);
		for (i32 i = 0; i < dampingForce.size(); ++i)
			dampingForce[i].setZero();
		addScaledDampingForce(0.5, dampingForce);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				energy -= dampingForce[node.index].dot(node.m_v) / dt;
			}
		}
		return energy;
	}

	double elasticEnergyDensity(const SoftBody::TetraScratch& s)
	{
		double density = 0;
		Matrix3x3 epsilon = (s.m_F + s.m_F.transpose()) * 0.5 - Matrix3x3::getIdentity();
		Scalar trace = epsilon[0][0] + epsilon[1][1] + epsilon[2][2];
		density += m_mu * (epsilon[0].length2() + epsilon[1].length2() + epsilon[2].length2());
		density += m_lambda * trace * trace * 0.5;
		return density;
	}

	virtual void addScaledElasticForce(Scalar scale, TVStack& force)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			Scalar max_p = psb->m_cfg.m_maxStress;
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				Matrix3x3 P;
				firstPiola(psb->m_tetraScratches[j], P);
#if USE_SVD
				if (max_p > 0)
				{
					// since we want to clamp the principal stress to max_p, we only need to
					// calculate SVD when sigma_0^2 + sigma_1^2 + sigma_2^2 > max_p * max_p
					Scalar trPTP = (P[0].length2() + P[1].length2() + P[2].length2());
					if (trPTP > max_p * max_p)
					{
						Matrix3x3 U, V;
						Vec3 sigma;
						singularValueDecomposition(P, U, sigma, V);
						sigma[0] = d3Min(sigma[0], max_p);
						sigma[1] = d3Min(sigma[1], max_p);
						sigma[2] = d3Min(sigma[2], max_p);
						sigma[0] = d3Max(sigma[0], -max_p);
						sigma[1] = d3Max(sigma[1], -max_p);
						sigma[2] = d3Max(sigma[2], -max_p);
						Matrix3x3 Sigma;
						Sigma.setIdentity();
						Sigma[0][0] = sigma[0];
						Sigma[1][1] = sigma[1];
						Sigma[2][2] = sigma[2];
						P = U * Sigma * V.transpose();
					}
				}
#endif
				//                Vec3 force_on_node0 = P * (tetra.m_Dm_inverse.transpose()*grad_N_hat_1st_col);
				Matrix3x3 force_on_node123 = psb->m_tetraScratches[j].m_corotation * P * tetra.m_Dm_inverse.transpose();
				Vec3 force_on_node0 = force_on_node123 * grad_N_hat_1st_col;

				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				size_t id0 = node0->index;
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				size_t id3 = node3->index;

				// elastic force
				Scalar scale1 = scale * tetra.m_element_measure;
				force[id0] -= scale1 * force_on_node0;
				force[id1] -= scale1 * force_on_node123.getColumn(0);
				force[id2] -= scale1 * force_on_node123.getColumn(1);
				force[id3] -= scale1 * force_on_node123.getColumn(2);
			}
		}
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) {}

	// The damping matrix is calculated using the time n state as described in https://www.math.ucla.edu/~jteran/papers/GSSJT15.pdf to allow line search
	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
		if (m_damping_alpha == 0 && m_damping_beta == 0)
			return;
		Scalar mu_damp = m_damping_beta * m_mu;
		Scalar lambda_damp = m_damping_beta * m_lambda;
		i32 numNodes = getNumNodes();
		Assert(numNodes <= df.size());
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				bool close_to_flat = (psb->m_tetraScratches[j].m_J < TETRA_FLAT_THRESHOLD);
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				size_t id0 = node0->index;
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				size_t id3 = node3->index;
				Matrix3x3 dF = Ds(id0, id1, id2, id3, dv) * tetra.m_Dm_inverse;
				if (!close_to_flat)
				{
					dF = psb->m_tetraScratches[j].m_corotation.transpose() * dF;
				}
				Matrix3x3 I;
				I.setIdentity();
				Matrix3x3 dP = (dF + dF.transpose()) * mu_damp + I * ((dF[0][0] + dF[1][1] + dF[2][2]) * lambda_damp);
				Matrix3x3 df_on_node123 = dP * tetra.m_Dm_inverse.transpose();
				if (!close_to_flat)
				{
					df_on_node123 = psb->m_tetraScratches[j].m_corotation * df_on_node123;
				}
				Vec3 df_on_node0 = df_on_node123 * grad_N_hat_1st_col;

				// damping force differential
				Scalar scale1 = scale * tetra.m_element_measure;
				df[id0] -= scale1 * df_on_node0;
				df[id1] -= scale1 * df_on_node123.getColumn(0);
				df[id2] -= scale1 * df_on_node123.getColumn(1);
				df[id3] -= scale1 * df_on_node123.getColumn(2);
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				size_t id = node.index;
				if (node.m_im > 0)
				{
					df[id] -= scale * dv[id] / node.m_im * m_damping_alpha;
				}
			}
		}
	}

	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= df.size());
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				size_t id0 = node0->index;
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				size_t id3 = node3->index;
				Matrix3x3 dF = psb->m_tetraScratches[j].m_corotation.transpose() * Ds(id0, id1, id2, id3, dx) * tetra.m_Dm_inverse;
				Matrix3x3 dP;
				firstPiolaDifferential(psb->m_tetraScratches[j], dF, dP);
				//                Vec3 df_on_node0 = dP * (tetra.m_Dm_inverse.transpose()*grad_N_hat_1st_col);
				Matrix3x3 df_on_node123 = psb->m_tetraScratches[j].m_corotation * dP * tetra.m_Dm_inverse.transpose();
				Vec3 df_on_node0 = df_on_node123 * grad_N_hat_1st_col;

				// elastic force differential
				Scalar scale1 = scale * tetra.m_element_measure;
				df[id0] -= scale1 * df_on_node0;
				df[id1] -= scale1 * df_on_node123.getColumn(0);
				df[id2] -= scale1 * df_on_node123.getColumn(1);
				df[id3] -= scale1 * df_on_node123.getColumn(2);
			}
		}
	}

	void firstPiola(const SoftBody::TetraScratch& s, Matrix3x3& P)
	{
		Matrix3x3 corotated_F = s.m_corotation.transpose() * s.m_F;

		Matrix3x3 epsilon = (corotated_F + corotated_F.transpose()) * 0.5 - Matrix3x3::getIdentity();
		Scalar trace = epsilon[0][0] + epsilon[1][1] + epsilon[2][2];
		P = epsilon * Scalar(2) * m_mu + Matrix3x3::getIdentity() * m_lambda * trace;
	}

	// Let P be the first piola stress.
	// This function calculates the dP = dP/dF * dF
	void firstPiolaDifferential(const SoftBody::TetraScratch& s, const Matrix3x3& dF, Matrix3x3& dP)
	{
		Scalar trace = (dF[0][0] + dF[1][1] + dF[2][2]);
		dP = (dF + dF.transpose()) * m_mu + Matrix3x3::getIdentity() * m_lambda * trace;
	}

	// Let Q be the damping stress.
	// This function calculates the dP = dQ/dF * dF
	void firstPiolaDampingDifferential(const SoftBody::TetraScratch& s, const Matrix3x3& dF, Matrix3x3& dP)
	{
		Scalar mu_damp = m_damping_beta * m_mu;
		Scalar lambda_damp = m_damping_beta * m_lambda;
		Scalar trace = (dF[0][0] + dF[1][1] + dF[2][2]);
		dP = (dF + dF.transpose()) * mu_damp + Matrix3x3::getIdentity() * lambda_damp * trace;
	}

	virtual void addScaledHessian(Scalar scale)
	{
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				Matrix3x3 P;
				firstPiola(psb->m_tetraScratches[j], P);  // make sure scratch is evaluated at x_n + dt * vn
				Matrix3x3 force_on_node123 = psb->m_tetraScratches[j].m_corotation * P * tetra.m_Dm_inverse.transpose();
				Vec3 force_on_node0 = force_on_node123 * grad_N_hat_1st_col;
				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				Scalar scale1 = scale * (scale + m_damping_beta) * tetra.m_element_measure;  // stiff and stiffness-damping terms;
				node0->m_effectiveMass += OuterProduct(force_on_node0, force_on_node0) * scale1;
				node1->m_effectiveMass += OuterProduct(force_on_node123.getColumn(0), force_on_node123.getColumn(0)) * scale1;
				node2->m_effectiveMass += OuterProduct(force_on_node123.getColumn(1), force_on_node123.getColumn(1)) * scale1;
				node3->m_effectiveMass += OuterProduct(force_on_node123.getColumn(2), force_on_node123.getColumn(2)) * scale1;
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				SoftBody::Node& node = psb->m_nodes[j];
				if (node.m_im > 0)
				{
					Matrix3x3 I;
					I.setIdentity();
					node.m_effectiveMass += I * (scale * (1.0 / node.m_im) * m_damping_alpha);
				}
			}
		}
	}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_LINEAR_ELASTICITY_FORCE;
	}
};
#endif /* DRX3D_LINEAR_ELASTICITY_H */
