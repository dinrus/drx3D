#ifndef DRX3D_PRECONDITIONER_H
#define DRX3D_PRECONDITIONER_H

class Preconditioner
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	virtual void operator()(const TVStack& x, TVStack& b) = 0;
	virtual void reinitialize(bool nodeUpdated) = 0;
	virtual ~Preconditioner() {}
};

class DefaultPreconditioner : public Preconditioner
{
public:
	virtual void operator()(const TVStack& x, TVStack& b)
	{
		Assert(b.size() == x.size());
		for (i32 i = 0; i < b.size(); ++i)
			b[i] = x[i];
	}
	virtual void reinitialize(bool nodeUpdated)
	{
	}

	virtual ~DefaultPreconditioner() {}
};

class MassPreconditioner : public Preconditioner
{
	AlignedObjectArray<Scalar> m_inv_mass;
	const AlignedObjectArray<SoftBody*>& m_softBodies;

public:
	MassPreconditioner(const AlignedObjectArray<SoftBody*>& softBodies)
		: m_softBodies(softBodies)
	{
	}

	virtual void reinitialize(bool nodeUpdated)
	{
		if (nodeUpdated)
		{
			m_inv_mass.clear();
			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
					m_inv_mass.push_back(psb->m_nodes[j].m_im);
			}
		}
	}

	virtual void operator()(const TVStack& x, TVStack& b)
	{
		Assert(b.size() == x.size());
		Assert(m_inv_mass.size() <= x.size());
		for (i32 i = 0; i < m_inv_mass.size(); ++i)
		{
			b[i] = x[i] * m_inv_mass[i];
		}
		for (i32 i = m_inv_mass.size(); i < b.size(); ++i)
		{
			b[i] = x[i];
		}
	}
};

class KKTPreconditioner : public Preconditioner
{
	const AlignedObjectArray<SoftBody*>& m_softBodies;
	const DeformableContactProjection& m_projections;
	const AlignedObjectArray<DeformableLagrangianForce*>& m_lf;
	TVStack m_inv_A, m_inv_S;
	const Scalar& m_dt;
	const bool& m_implicit;

public:
	KKTPreconditioner(const AlignedObjectArray<SoftBody*>& softBodies, const DeformableContactProjection& projections, const AlignedObjectArray<DeformableLagrangianForce*>& lf, const Scalar& dt, const bool& implicit)
		: m_softBodies(softBodies), m_projections(projections), m_lf(lf), m_dt(dt), m_implicit(implicit)
	{
	}

	virtual void reinitialize(bool nodeUpdated)
	{
		if (nodeUpdated)
		{
			i32 num_nodes = 0;
			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				num_nodes += psb->m_nodes.size();
			}
			m_inv_A.resize(num_nodes);
		}
		buildDiagonalA(m_inv_A);
		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			//            printf("A[%d] = %f, %f, %f \n", i, m_inv_A[i][0], m_inv_A[i][1], m_inv_A[i][2]);
			for (i32 d = 0; d < 3; ++d)
			{
				m_inv_A[i][d] = (m_inv_A[i][d] == 0) ? 0.0 : 1.0 / m_inv_A[i][d];
			}
		}
		m_inv_S.resize(m_projections.m_lagrangeMultipliers.size());
		//        printf("S.size() = %d \n", m_inv_S.size());
		buildDiagonalS(m_inv_A, m_inv_S);
		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			//            printf("S[%d] = %f, %f, %f \n", i, m_inv_S[i][0], m_inv_S[i][1], m_inv_S[i][2]);
			for (i32 d = 0; d < 3; ++d)
			{
				m_inv_S[i][d] = (m_inv_S[i][d] == 0) ? 0.0 : 1.0 / m_inv_S[i][d];
			}
		}
	}

	void buildDiagonalA(TVStack& diagA) const
	{
		size_t counter = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				diagA[counter] = (node.m_im == 0) ? Vec3(0, 0, 0) : Vec3(1.0 / node.m_im, 1.0 / node.m_im, 1.0 / node.m_im);
				++counter;
			}
		}
		if (m_implicit)
		{
			printf("implicit not implemented\n");
			Assert(false);
		}
		for (i32 i = 0; i < m_lf.size(); ++i)
		{
			// add damping matrix
			m_lf[i]->buildDampingForceDifferentialDiagonal(-m_dt, diagA);
		}
	}

	void buildDiagonalS(const TVStack& inv_A, TVStack& diagS)
	{
		for (i32 c = 0; c < m_projections.m_lagrangeMultipliers.size(); ++c)
		{
			// S[k,k] = e_k^T * C A_d^-1 C^T * e_k
			const LagrangeMultiplier& lm = m_projections.m_lagrangeMultipliers[c];
			Vec3& t = diagS[c];
			t.setZero();
			for (i32 j = 0; j < lm.m_num_constraints; ++j)
			{
				for (i32 i = 0; i < lm.m_num_nodes; ++i)
				{
					for (i32 d = 0; d < 3; ++d)
					{
						t[j] += inv_A[lm.m_indices[i]][d] * lm.m_dirs[j][d] * lm.m_dirs[j][d] * lm.m_weights[i] * lm.m_weights[i];
					}
				}
			}
		}
	}
//#define USE_FULL_PRECONDITIONER
#ifndef USE_FULL_PRECONDITIONER
	virtual void operator()(const TVStack& x, TVStack& b)
	{
		Assert(b.size() == x.size());
		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			b[i] = x[i] * m_inv_A[i];
		}
		i32 offset = m_inv_A.size();
		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			b[i + offset] = x[i + offset] * m_inv_S[i];
		}
	}
#else
	virtual void operator()(const TVStack& x, TVStack& b)
	{
		Assert(b.size() == x.size());
		i32 offset = m_inv_A.size();

		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			b[i] = x[i] * m_inv_A[i];
		}

		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			b[i + offset].setZero();
		}

		for (i32 c = 0; c < m_projections.m_lagrangeMultipliers.size(); ++c)
		{
			const LagrangeMultiplier& lm = m_projections.m_lagrangeMultipliers[c];
			// C * x
			for (i32 d = 0; d < lm.m_num_constraints; ++d)
			{
				for (i32 i = 0; i < lm.m_num_nodes; ++i)
				{
					b[offset + c][d] += lm.m_weights[i] * b[lm.m_indices[i]].dot(lm.m_dirs[d]);
				}
			}
		}

		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			b[i + offset] = b[i + offset] * m_inv_S[i];
		}

		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			b[i].setZero();
		}

		for (i32 c = 0; c < m_projections.m_lagrangeMultipliers.size(); ++c)
		{
			// C^T * lambda
			const LagrangeMultiplier& lm = m_projections.m_lagrangeMultipliers[c];
			for (i32 i = 0; i < lm.m_num_nodes; ++i)
			{
				for (i32 j = 0; j < lm.m_num_constraints; ++j)
				{
					b[lm.m_indices[i]] += b[offset + c][j] * lm.m_weights[i] * lm.m_dirs[j];
				}
			}
		}

		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			b[i] = (x[i] - b[i]) * m_inv_A[i];
		}

		TVStack t;
		t.resize(b.size());
		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			t[i + offset] = x[i + offset] * m_inv_S[i];
		}
		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			t[i].setZero();
		}
		for (i32 c = 0; c < m_projections.m_lagrangeMultipliers.size(); ++c)
		{
			// C^T * lambda
			const LagrangeMultiplier& lm = m_projections.m_lagrangeMultipliers[c];
			for (i32 i = 0; i < lm.m_num_nodes; ++i)
			{
				for (i32 j = 0; j < lm.m_num_constraints; ++j)
				{
					t[lm.m_indices[i]] += t[offset + c][j] * lm.m_weights[i] * lm.m_dirs[j];
				}
			}
		}
		for (i32 i = 0; i < m_inv_A.size(); ++i)
		{
			b[i] += t[i] * m_inv_A[i];
		}

		for (i32 i = 0; i < m_inv_S.size(); ++i)
		{
			b[i + offset] -= x[i + offset] * m_inv_S[i];
		}
	}
#endif
};

#endif /* DRX3D_PRECONDITIONER_H */
