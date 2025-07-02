#ifndef DRX3D_NEOHOOKEAN_H
#define DRX3D_NEOHOOKEAN_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/ImplicitQRSVD.h>
// This energy is as described in https://graphics.pixar.com/library/StableElasticity/paper.pdf
class DeformableNeoHookeanForce : public DeformableLagrangianForce
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	Scalar m_mu, m_lambda;  // Lame Parameters
	Scalar m_E, m_nu;       // Young's modulus and Poisson ratio
	Scalar m_mu_damp, m_lambda_damp;
	DeformableNeoHookeanForce() : m_mu(1), m_lambda(1)
	{
		Scalar damping = 0.05;
		m_mu_damp = damping * m_mu;
		m_lambda_damp = damping * m_lambda;
		updateYoungsModulusAndPoissonRatio();
	}

	DeformableNeoHookeanForce(Scalar mu, Scalar lambda, Scalar damping = 0.05) : m_mu(mu), m_lambda(lambda)
	{
		m_mu_damp = damping * m_mu;
		m_lambda_damp = damping * m_lambda;
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

	void setDamping(Scalar damping)
	{
		m_mu_damp = damping * m_mu;
		m_lambda_damp = damping * m_lambda;
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
		if (m_mu_damp == 0 && m_lambda_damp == 0)
			return;
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
				Matrix3x3 I;
				I.setIdentity();
				Matrix3x3 dP = (dF + dF.transpose()) * m_mu_damp + I * (dF[0][0] + dF[1][1] + dF[2][2]) * m_lambda_damp;
				//                firstPiolaDampingDifferential(psb->m_tetraScratchesTn[j], dF, dP);
				Vec3 df_on_node0 = dP * (tetra.m_Dm_inverse.transpose() * grad_N_hat_1st_col);
				Matrix3x3 df_on_node123 = dP * tetra.m_Dm_inverse.transpose();

				// damping force differential
				Scalar scale1 = scale * tetra.m_element_measure;
				force[id0] -= scale1 * df_on_node0;
				force[id1] -= scale1 * df_on_node123.getColumn(0);
				force[id2] -= scale1 * df_on_node123.getColumn(1);
				force[id3] -= scale1 * df_on_node123.getColumn(2);
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
		density += m_mu * 0.5 * (s.m_trace - 3.);
		density += m_lambda * 0.5 * (s.m_J - 1. - 0.75 * m_mu / m_lambda) * (s.m_J - 1. - 0.75 * m_mu / m_lambda);
		density -= m_mu * 0.5 * log(s.m_trace + 1);
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
#ifdef USE_SVD
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
				Matrix3x3 force_on_node123 = P * tetra.m_Dm_inverse.transpose();
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

	// The damping matrix is calculated using the time n state as described in https://www.math.ucla.edu/~jteran/papers/GSSJT15.pdf to allow line search
	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
		if (m_mu_damp == 0 && m_lambda_damp == 0)
			return;
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
				Matrix3x3 dF = Ds(id0, id1, id2, id3, dv) * tetra.m_Dm_inverse;
				Matrix3x3 I;
				I.setIdentity();
				Matrix3x3 dP = (dF + dF.transpose()) * m_mu_damp + I * (dF[0][0] + dF[1][1] + dF[2][2]) * m_lambda_damp;
				//                firstPiolaDampingDifferential(psb->m_tetraScratchesTn[j], dF, dP);
				//                Vec3 df_on_node0 = dP * (tetra.m_Dm_inverse.transpose()*grad_N_hat_1st_col);
				Matrix3x3 df_on_node123 = dP * tetra.m_Dm_inverse.transpose();
				Vec3 df_on_node0 = df_on_node123 * grad_N_hat_1st_col;

				// damping force differential
				Scalar scale1 = scale * tetra.m_element_measure;
				df[id0] -= scale1 * df_on_node0;
				df[id1] -= scale1 * df_on_node123.getColumn(0);
				df[id2] -= scale1 * df_on_node123.getColumn(1);
				df[id3] -= scale1 * df_on_node123.getColumn(2);
			}
		}
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) {}

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
				Matrix3x3 dF = Ds(id0, id1, id2, id3, dx) * tetra.m_Dm_inverse;
				Matrix3x3 dP;
				firstPiolaDifferential(psb->m_tetraScratches[j], dF, dP);
				//                Vec3 df_on_node0 = dP * (tetra.m_Dm_inverse.transpose()*grad_N_hat_1st_col);
				Matrix3x3 df_on_node123 = dP * tetra.m_Dm_inverse.transpose();
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
		Scalar c1 = (m_mu * (1. - 1. / (s.m_trace + 1.)));
		Scalar c2 = (m_lambda * (s.m_J - 1.) - 0.75 * m_mu);
		P = s.m_F * c1 + s.m_cofF * c2;
	}

	// Let P be the first piola stress.
	// This function calculates the dP = dP/dF * dF
	void firstPiolaDifferential(const SoftBody::TetraScratch& s, const Matrix3x3& dF, Matrix3x3& dP)
	{
		Scalar c1 = m_mu * (1. - 1. / (s.m_trace + 1.));
		Scalar c2 = (2. * m_mu) * DotProduct(s.m_F, dF) * (1. / ((1. + s.m_trace) * (1. + s.m_trace)));
		Scalar c3 = (m_lambda * DotProduct(s.m_cofF, dF));
		dP = dF * c1 + s.m_F * c2;
		addScaledCofactorMatrixDifferential(s.m_F, dF, m_lambda * (s.m_J - 1.) - 0.75 * m_mu, dP);
		dP += s.m_cofF * c3;
	}

	// Let Q be the damping stress.
	// This function calculates the dP = dQ/dF * dF
	void firstPiolaDampingDifferential(const SoftBody::TetraScratch& s, const Matrix3x3& dF, Matrix3x3& dP)
	{
		Scalar c1 = (m_mu_damp * (1. - 1. / (s.m_trace + 1.)));
		Scalar c2 = ((2. * m_mu_damp) * DotProduct(s.m_F, dF) * (1. / ((1. + s.m_trace) * (1. + s.m_trace))));
		Scalar c3 = (m_lambda_damp * DotProduct(s.m_cofF, dF));
		dP = dF * c1 + s.m_F * c2;
		addScaledCofactorMatrixDifferential(s.m_F, dF, m_lambda_damp * (s.m_J - 1.) - 0.75 * m_mu_damp, dP);
		dP += s.m_cofF * c3;
	}

	Scalar DotProduct(const Matrix3x3& A, const Matrix3x3& B)
	{
		Scalar ans = 0;
		for (i32 i = 0; i < 3; ++i)
		{
			ans += A[i].dot(B[i]);
		}
		return ans;
	}

	// Let C(A) be the cofactor of the matrix A
	// Let H = the derivative of C(A) with respect to A evaluated at F = A
	// This function calculates H*dF
	void addScaledCofactorMatrixDifferential(const Matrix3x3& F, const Matrix3x3& dF, Scalar scale, Matrix3x3& M)
	{
		M[0][0] += scale * (dF[1][1] * F[2][2] + F[1][1] * dF[2][2] - dF[2][1] * F[1][2] - F[2][1] * dF[1][2]);
		M[1][0] += scale * (dF[2][1] * F[0][2] + F[2][1] * dF[0][2] - dF[0][1] * F[2][2] - F[0][1] * dF[2][2]);
		M[2][0] += scale * (dF[0][1] * F[1][2] + F[0][1] * dF[1][2] - dF[1][1] * F[0][2] - F[1][1] * dF[0][2]);
		M[0][1] += scale * (dF[2][0] * F[1][2] + F[2][0] * dF[1][2] - dF[1][0] * F[2][2] - F[1][0] * dF[2][2]);
		M[1][1] += scale * (dF[0][0] * F[2][2] + F[0][0] * dF[2][2] - dF[2][0] * F[0][2] - F[2][0] * dF[0][2]);
		M[2][1] += scale * (dF[1][0] * F[0][2] + F[1][0] * dF[0][2] - dF[0][0] * F[1][2] - F[0][0] * dF[1][2]);
		M[0][2] += scale * (dF[1][0] * F[2][1] + F[1][0] * dF[2][1] - dF[2][0] * F[1][1] - F[2][0] * dF[1][1]);
		M[1][2] += scale * (dF[2][0] * F[0][1] + F[2][0] * dF[0][1] - dF[0][0] * F[2][1] - F[0][0] * dF[2][1]);
		M[2][2] += scale * (dF[0][0] * F[1][1] + F[0][0] * dF[1][1] - dF[1][0] * F[0][1] - F[1][0] * dF[0][1]);
	}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_NEOHOOKEAN_FORCE;
	}
};
#endif /* DRX3D_NEOHOOKEAN_H */
