#ifndef DRX3D_MOUSE_PICKING_FORCE_H
#define DRX3D_MOUSE_PICKING_FORCE_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>

class DeformableMousePickingForce : public DeformableLagrangianForce
{
	// If true, the damping force will be in the direction of the spring
	// If false, the damping force will be in the direction of the velocity
	Scalar m_elasticStiffness, m_dampingStiffness;
	const SoftBody::Face& m_face;
	Vec3 m_mouse_pos;
	Scalar m_maxForce;

public:
	typedef AlignedObjectArray<Vec3> TVStack;
	DeformableMousePickingForce(Scalar k, Scalar d, const SoftBody::Face& face, const Vec3& mouse_pos, Scalar maxForce = 0.3) : m_elasticStiffness(k), m_dampingStiffness(d), m_face(face), m_mouse_pos(mouse_pos), m_maxForce(maxForce)
	{
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
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 v_diff = m_face.m_n[i]->m_v;
			Vec3 scaled_force = scale * m_dampingStiffness * v_diff;
			if ((m_face.m_n[i]->m_x - m_mouse_pos).norm() > SIMD_EPSILON)
			{
				Vec3 dir = (m_face.m_n[i]->m_x - m_mouse_pos).normalized();
				scaled_force = scale * m_dampingStiffness * v_diff.dot(dir) * dir;
			}
			force[m_face.m_n[i]->index] -= scaled_force;
		}
	}

	virtual void addScaledElasticForce(Scalar scale, TVStack& force)
	{
		Scalar scaled_stiffness = scale * m_elasticStiffness;
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 dir = (m_face.m_n[i]->m_q - m_mouse_pos);
			Vec3 scaled_force = scaled_stiffness * dir;
			if (scaled_force.safeNorm() > m_maxForce)
			{
				scaled_force.safeNormalize();
				scaled_force *= m_maxForce;
			}
			force[m_face.m_n[i]->index] -= scaled_force;
		}
	}

	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df)
	{
		Scalar scaled_k_damp = m_dampingStiffness * scale;
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 local_scaled_df = scaled_k_damp * dv[m_face.m_n[i]->index];
			if ((m_face.m_n[i]->m_x - m_mouse_pos).norm() > SIMD_EPSILON)
			{
				Vec3 dir = (m_face.m_n[i]->m_x - m_mouse_pos).normalized();
				local_scaled_df = scaled_k_damp * dv[m_face.m_n[i]->index].dot(dir) * dir;
			}
			df[m_face.m_n[i]->index] -= local_scaled_df;
		}
	}

	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) {}

	virtual double totalElasticEnergy(Scalar dt)
	{
		double energy = 0;
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 dir = (m_face.m_n[i]->m_q - m_mouse_pos);
			Vec3 scaled_force = m_elasticStiffness * dir;
			if (scaled_force.safeNorm() > m_maxForce)
			{
				scaled_force.safeNormalize();
				scaled_force *= m_maxForce;
			}
			energy += 0.5 * scaled_force.dot(dir);
		}
		return energy;
	}

	virtual double totalDampingEnergy(Scalar dt)
	{
		double energy = 0;
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 v_diff = m_face.m_n[i]->m_v;
			Vec3 scaled_force = m_dampingStiffness * v_diff;
			if ((m_face.m_n[i]->m_x - m_mouse_pos).norm() > SIMD_EPSILON)
			{
				Vec3 dir = (m_face.m_n[i]->m_x - m_mouse_pos).normalized();
				scaled_force = m_dampingStiffness * v_diff.dot(dir) * dir;
			}
			energy -= scaled_force.dot(m_face.m_n[i]->m_v) / dt;
		}
		return energy;
	}

	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df)
	{
		Scalar scaled_stiffness = scale * m_elasticStiffness;
		for (i32 i = 0; i < 3; ++i)
		{
			Vec3 dir = (m_face.m_n[i]->m_q - m_mouse_pos);
			Scalar dir_norm = dir.norm();
			Vec3 dir_normalized = (dir_norm > SIMD_EPSILON) ? dir.normalized() : Vec3(0, 0, 0);
			i32 id = m_face.m_n[i]->index;
			Vec3 dx_diff = dx[id];
			Scalar r = 0;  // rest length is 0 for picking spring
			Vec3 scaled_df = Vec3(0, 0, 0);
			if (dir_norm > SIMD_EPSILON)
			{
				scaled_df -= scaled_stiffness * dir_normalized.dot(dx_diff) * dir_normalized;
				scaled_df += scaled_stiffness * dir_normalized.dot(dx_diff) * ((dir_norm - r) / dir_norm) * dir_normalized;
				scaled_df -= scaled_stiffness * ((dir_norm - r) / dir_norm) * dx_diff;
			}
			df[id] += scaled_df;
		}
	}

	void setMousePos(const Vec3& p)
	{
		m_mouse_pos = p;
	}

	virtual DeformableLagrangianForceType getForceType()
	{
		return DRX3D_MOUSE_PICKING_FORCE;
	}
};

#endif /* MassSpring_h */
