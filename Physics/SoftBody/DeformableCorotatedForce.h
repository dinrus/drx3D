#ifndef DRX3D_COROTATED_H
#define DRX3D_COROTATED_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>
#include <drx3D/Maths/Linear/PolarDecomposition.h>

static inline i32 PolarDecomposition(const Matrix3x3& m, Matrix3x3& q, Matrix3x3& s)
{
	static const class PolarDecomposition polar;
	return polar.decompose(m, q, s);
}

class DeformableCorotatedForce : public DeformableLagrangianForce
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	Scalar m_mu, m_lambda;
	DeformableCorotatedForce() : m_mu(1), m_lambda(1)
	{
	}

	DeformableCorotatedForce(Scalar mu, Scalar lambda) : m_mu(mu), m_lambda(lambda)
	{
	}

	virtual void addScaledForces(Scalar scale, TVStack& force)
	{
		addScaledElasticForce(scale, force);
	}

	virtual void addScaledExplicitForce(Scalar scale, TVStack& force)
	{
		addScaledElasticForce(scale, force);
	}

	virtual void addScaledDampingForce(Scalar scale, TVStack& force)
	{
	}

	virtual void addScaledElasticForce(Scalar scale, TVStack& force)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		Vec3 grad_N_hat_1st_col = Vec3(-1, -1, -1);
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_tetras.size(); ++j)
			{
				SoftBody::Tetra& tetra = psb->m_tetras[j];
				Matrix3x3 P;
				firstPiola(tetra.m_F, P);
				Vec3 force_on_node0 = P * (tetra.m_Dm_inverse.transpose() * grad_N_hat_1st_col);
				Matrix3x3 force_on_node123 = P * tetra.m_Dm_inverse.transpose();

				SoftBody::Node* node0 = tetra.m_n[0];
				SoftBody::Node* node1 = tetra.m_n[1];
				SoftBody::Node* node2 = tetra.m_n[2];
				SoftBody::Node* node3 = tetra.m_n[3];
				size_t id0 = node0->index;
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				size_t id3 = node3->index;

				// elastic force
				// explicit elastic force
				Scalar scale1 = scale * tetra.m_element_measure;
				force[id0] -= scale1 * force_on_node0;
				force[id1] -= scale1 * force_on_node123.getColumn(0);
				force[id2] -= scale1 * force_on_node123.getColumn(1);
				force[id3] -= scale1 * force_on_node123.getColumn(2);
			}
		}
	}

	void firstPiola(const Matrix3x3& F, Matrix3x3& P)
	{
		// Matrix3x3 JFinvT = F.adjoint();
		Scalar J = F.determinant();
		P = F.adjoint().transpose() * (m_lambda * (J - 1));
		if (m_mu > SIMD_EPSILON)
		{
			Matrix3x3 R, S;
			if (J < 1024 * SIMD_EPSILON)
				R.setIdentity();
			else
				PolarDecomposition(F, R, S);  // this QR is not robust, consider using implicit shift svd
			/*https://fuchuyuan.github.io/research/svd/paper.pdf*/
			P += (F - R) * 2 * m_mu;
		}
	}

	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df)
	{
	}

	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) {}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_COROTATED_FORCE;
	}
};

#endif /* Corotated_h */
