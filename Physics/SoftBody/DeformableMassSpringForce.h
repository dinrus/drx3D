#ifndef DRX3D_MASS_SPRING_H
#define DRX3D_MASS_SPRING_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>

class DeformableMassSpringForce : public DeformableLagrangianForce
{
	// If true, the damping force will be in the direction of the spring
	// If false, the damping force will be in the direction of the velocity
	bool m_momentum_conserving;
	Scalar m_elasticStiffness, m_dampingStiffness, m_bendingStiffness;

public:
	typedef AlignedObjectArray<Vec3> TVStack;
	DeformableMassSpringForce() : m_momentum_conserving(false), m_elasticStiffness(1), m_dampingStiffness(0.05)
	{
	}
	DeformableMassSpringForce(Scalar k, Scalar d, bool conserve_angular = true, double bending_k = -1) : m_momentum_conserving(conserve_angular), m_elasticStiffness(k), m_dampingStiffness(d), m_bendingStiffness(bending_k)
	{
		if (m_bendingStiffness < Scalar(0))
		{
			m_bendingStiffness = m_elasticStiffness;
		}
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

	virtual void addScaledDampingForce(Scalar scale, TVStack& force)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			const SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				size_t id1 = node1->index;
				size_t id2 = node2->index;

				// damping force
				Vec3 v_diff = (node2->m_v - node1->m_v);
				Vec3 scaled_force = scale * m_dampingStiffness * v_diff;
				if (m_momentum_conserving)
				{
					if ((node2->m_x - node1->m_x).norm() > SIMD_EPSILON)
					{
						Vec3 dir = (node2->m_x - node1->m_x).normalized();
						scaled_force = scale * m_dampingStiffness * v_diff.dot(dir) * dir;
					}
				}
				force[id1] += scaled_force;
				force[id2] -= scaled_force;
			}
		}
	}

	virtual void addScaledElasticForce(Scalar scale, TVStack& force)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			const SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				Scalar r = link.m_rl;
				size_t id1 = node1->index;
				size_t id2 = node2->index;

				// elastic force
				Vec3 dir = (node2->m_q - node1->m_q);
				Vec3 dir_normalized = (dir.norm() > SIMD_EPSILON) ? dir.normalized() : Vec3(0, 0, 0);
				Scalar scaled_stiffness = scale * (link.m_bbending ? m_bendingStiffness : m_elasticStiffness);
				Vec3 scaled_force = scaled_stiffness * (dir - dir_normalized * r);
				force[id1] += scaled_force;
				force[id2] -= scaled_force;
			}
		}
	}

	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
		// implicit damping force differential
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			Scalar scaled_k_damp = m_dampingStiffness * scale;
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				size_t id1 = node1->index;
				size_t id2 = node2->index;

				Vec3 local_scaled_df = scaled_k_damp * (dv[id2] - dv[id1]);
				if (m_momentum_conserving)
				{
					if ((node2->m_x - node1->m_x).norm() > SIMD_EPSILON)
					{
						Vec3 dir = (node2->m_x - node1->m_x).normalized();
						local_scaled_df = scaled_k_damp * (dv[id2] - dv[id1]).dot(dir) * dir;
					}
				}
				df[id1] += local_scaled_df;
				df[id2] -= local_scaled_df;
			}
		}
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA)
	{
		// implicit damping force differential
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			Scalar scaled_k_damp = m_dampingStiffness * scale;
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				if (m_momentum_conserving)
				{
					if ((node2->m_x - node1->m_x).norm() > SIMD_EPSILON)
					{
						Vec3 dir = (node2->m_x - node1->m_x).normalized();
						for (i32 d = 0; d < 3; ++d)
						{
							if (node1->m_im > 0)
								diagA[id1][d] -= scaled_k_damp * dir[d] * dir[d];
							if (node2->m_im > 0)
								diagA[id2][d] -= scaled_k_damp * dir[d] * dir[d];
						}
					}
				}
				else
				{
					for (i32 d = 0; d < 3; ++d)
					{
						if (node1->m_im > 0)
							diagA[id1][d] -= scaled_k_damp;
						if (node2->m_im > 0)
							diagA[id2][d] -= scaled_k_damp;
					}
				}
			}
		}
	}

	virtual double totalElasticEnergy(Scalar dt)
	{
		double energy = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			const SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				Scalar r = link.m_rl;

				// elastic force
				Vec3 dir = (node2->m_q - node1->m_q);
				energy += 0.5 * m_elasticStiffness * (dir.norm() - r) * (dir.norm() - r);
			}
		}
		return energy;
	}

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

	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df)
	{
		// implicit damping force differential
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			const SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_links.size(); ++j)
			{
				const SoftBody::Link& link = psb->m_links[j];
				SoftBody::Node* node1 = link.m_n[0];
				SoftBody::Node* node2 = link.m_n[1];
				size_t id1 = node1->index;
				size_t id2 = node2->index;
				Scalar r = link.m_rl;

				Vec3 dir = (node1->m_q - node2->m_q);
				Scalar dir_norm = dir.norm();
				Vec3 dir_normalized = (dir_norm > SIMD_EPSILON) ? dir.normalized() : Vec3(0, 0, 0);
				Vec3 dx_diff = dx[id1] - dx[id2];
				Vec3 scaled_df = Vec3(0, 0, 0);
				Scalar scaled_k = scale * (link.m_bbending ? m_bendingStiffness : m_elasticStiffness);
				if (dir_norm > SIMD_EPSILON)
				{
					scaled_df -= scaled_k * dir_normalized.dot(dx_diff) * dir_normalized;
					scaled_df += scaled_k * dir_normalized.dot(dx_diff) * ((dir_norm - r) / dir_norm) * dir_normalized;
					scaled_df -= scaled_k * ((dir_norm - r) / dir_norm) * dx_diff;
				}

				df[id1] += scaled_df;
				df[id2] -= scaled_df;
			}
		}
	}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_MASSSPRING_FORCE;
	}
};

#endif /* MassSpring_h */
