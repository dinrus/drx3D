#ifndef DRX3D_DEFORMABLE_GRAVITY_FORCE_H
#define DRX3D_DEFORMABLE_GRAVITY_FORCE_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>

class DeformableGravityForce : public DeformableLagrangianForce
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	Vec3 m_gravity;

	DeformableGravityForce(const Vec3& g) : m_gravity(g)
	{
	}

	virtual void addScaledForces(Scalar scale, TVStack& force)
	{
		addScaledGravityForce(scale, force);
	}

	virtual void addScaledExplicitForce(Scalar scale, TVStack& force)
	{
		addScaledGravityForce(scale, force);
	}

	virtual void addScaledDampingForce(Scalar scale, TVStack& force)
	{
	}

	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df)
	{
	}

	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) {}

	virtual void addScaledGravityForce(Scalar scale, TVStack& force)
	{
		i32 numNodes = getNumNodes();
		Assert(numNodes <= force.size());
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				SoftBody::Node& n = psb->m_nodes[j];
				size_t id = n.index;
				Scalar mass = (n.m_im == 0) ? 0 : 1. / n.m_im;
				Vec3 scaled_force = scale * m_gravity * mass * m_softBodies[i]->m_gravityFactor;
				force[id] += scaled_force;
			}
		}
	}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_GRAVITY_FORCE;
	}

	// the gravitational potential energy
	virtual double totalEnergy(Scalar dt)
	{
		double e = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			if (!psb->isActive())
			{
				continue;
			}
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				if (node.m_im > 0)
				{
					e -= m_gravity.dot(node.m_q) / node.m_im;
				}
			}
		}
		return e;
	}
};
#endif /* DRX3D_DEFORMABLE_GRAVITY_FORCE_H */
