#include <drx3D/Physics/SoftBody/DeformableContactConstraint.h>
/* ================   Deformable Node Anchor   =================== */
DeformableNodeAnchorConstraint::DeformableNodeAnchorConstraint(const SoftBody::DeformableNodeRigidAnchor& a, const ContactSolverInfo& infoGlobal)
	: m_anchor(&a), DeformableContactConstraint(a.m_cti.m_normal, infoGlobal)
{
}

DeformableNodeAnchorConstraint::DeformableNodeAnchorConstraint(const DeformableNodeAnchorConstraint& other)
	: m_anchor(other.m_anchor), DeformableContactConstraint(other)
{
}

Vec3 DeformableNodeAnchorConstraint::getVa() const
{
	const SoftBody::sCti& cti = m_anchor->m_cti;
	Vec3 va(0, 0, 0);
	if (cti.m_colObj->hasContactResponse())
	{
		RigidBody* rigidCol = 0;
		MultiBodyLinkCollider* multibodyLinkCol = 0;

		// grab the velocity of the rigid body
		if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
		{
			rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
			va = rigidCol ? (rigidCol->getVelocityInLocalPoint(m_anchor->m_c1)) : Vec3(0, 0, 0);
		}
		else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
		{
			multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
			if (multibodyLinkCol)
			{
				i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
				const Scalar* J_n = &m_anchor->jacobianData_normal.m_jacobians[0];
				const Scalar* J_t1 = &m_anchor->jacobianData_t1.m_jacobians[0];
				const Scalar* J_t2 = &m_anchor->jacobianData_t2.m_jacobians[0];
				const Scalar* local_v = multibodyLinkCol->m_multiBody->getVelocityVector();
				const Scalar* local_dv = multibodyLinkCol->m_multiBody->getDeltaVelocityVector();
				// add in the normal component of the va
				Scalar vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_n[k];
				}
				va = cti.m_normal * vel;
				// add in the tangential components of the va
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_t1[k];
				}
				va += m_anchor->t1 * vel;
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_t2[k];
				}
				va += m_anchor->t2 * vel;
			}
		}
	}
	return va;
}

Scalar DeformableNodeAnchorConstraint::solveConstraint(const ContactSolverInfo& infoGlobal)
{
	const SoftBody::sCti& cti = m_anchor->m_cti;
	Vec3 va = getVa();
	Vec3 vb = getVb();
	Vec3 vr = (vb - va);
	// + (m_anchor->m_node->m_x - cti.m_colObj->getWorldTransform() * m_anchor->m_local) * 10.0
	const Scalar dn = Dot(vr, vr);
	// dn is the normal component of velocity diffrerence. Approximates the residual. // todo xuchenhan@: this prob needs to be scaled by dt
	Scalar residualSquare = dn * dn;
	Vec3 impulse = m_anchor->m_c0 * vr;
	// apply impulse to deformable nodes involved and change their velocities
	applyImpulse(impulse);

	// apply impulse to the rigid/multibodies involved and change their velocities
	if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
	{
		RigidBody* rigidCol = 0;
		rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
		if (rigidCol)
		{
			rigidCol->applyImpulse(impulse, m_anchor->m_c1);
		}
	}
	else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
	{
		MultiBodyLinkCollider* multibodyLinkCol = 0;
		multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
		if (multibodyLinkCol)
		{
			const Scalar* deltaV_normal = &m_anchor->jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
			// apply normal component of the impulse
			multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_normal, impulse.dot(cti.m_normal));
			// apply tangential component of the impulse
			const Scalar* deltaV_t1 = &m_anchor->jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
			multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_t1, impulse.dot(m_anchor->t1));
			const Scalar* deltaV_t2 = &m_anchor->jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];
			multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_t2, impulse.dot(m_anchor->t2));
		}
	}
	return residualSquare;
}

Vec3 DeformableNodeAnchorConstraint::getVb() const
{
	return m_anchor->m_node->m_v;
}

void DeformableNodeAnchorConstraint::applyImpulse(const Vec3& impulse)
{
	Vec3 dv = impulse * m_anchor->m_c2;
	m_anchor->m_node->m_v -= dv;
}

/* ================   Deformable vs. Rigid   =================== */
DeformableRigidContactConstraint::DeformableRigidContactConstraint(const SoftBody::DeformableRigidContact& c, const ContactSolverInfo& infoGlobal)
	: m_contact(&c), DeformableContactConstraint(c.m_cti.m_normal, infoGlobal)
{
	m_total_normal_dv.setZero();
	m_total_tangent_dv.setZero();
	// The magnitude of penetration is the depth of penetration.
	m_penetration = c.m_cti.m_offset;
	m_total_split_impulse = 0;
	m_binding = false;
}

DeformableRigidContactConstraint::DeformableRigidContactConstraint(const DeformableRigidContactConstraint& other)
	: m_contact(other.m_contact), DeformableContactConstraint(other), m_penetration(other.m_penetration), m_total_split_impulse(other.m_total_split_impulse), m_binding(other.m_binding)
{
	m_total_normal_dv = other.m_total_normal_dv;
	m_total_tangent_dv = other.m_total_tangent_dv;
}

Vec3 DeformableRigidContactConstraint::getVa() const
{
	const SoftBody::sCti& cti = m_contact->m_cti;
	Vec3 va(0, 0, 0);
	if (cti.m_colObj->hasContactResponse())
	{
		RigidBody* rigidCol = 0;
		MultiBodyLinkCollider* multibodyLinkCol = 0;

		// grab the velocity of the rigid body
		if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
		{
			rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
			va = rigidCol ? (rigidCol->getVelocityInLocalPoint(m_contact->m_c1)) : Vec3(0, 0, 0);
		}
		else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
		{
			multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
			if (multibodyLinkCol)
			{
				i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
				const Scalar* J_n = &m_contact->jacobianData_normal.m_jacobians[0];
				const Scalar* J_t1 = &m_contact->jacobianData_t1.m_jacobians[0];
				const Scalar* J_t2 = &m_contact->jacobianData_t2.m_jacobians[0];
				const Scalar* local_v = multibodyLinkCol->m_multiBody->getVelocityVector();
				const Scalar* local_dv = multibodyLinkCol->m_multiBody->getDeltaVelocityVector();
				// add in the normal component of the va
				Scalar vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_n[k];
				}
				va = cti.m_normal * vel;
				// add in the tangential components of the va
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_t1[k];
				}
				va += m_contact->t1 * vel;
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += (local_v[k] + local_dv[k]) * J_t2[k];
				}
				va += m_contact->t2 * vel;
			}
		}
	}
	return va;
}

Vec3 DeformableRigidContactConstraint::getSplitVa() const
{
	const SoftBody::sCti& cti = m_contact->m_cti;
	Vec3 va(0, 0, 0);
	if (cti.m_colObj->hasContactResponse())
	{
		RigidBody* rigidCol = 0;
		MultiBodyLinkCollider* multibodyLinkCol = 0;

		// grab the velocity of the rigid body
		if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
		{
			rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
			va = rigidCol ? (rigidCol->getPushVelocityInLocalPoint(m_contact->m_c1)) : Vec3(0, 0, 0);
		}
		else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
		{
			multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
			if (multibodyLinkCol)
			{
				i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
				const Scalar* J_n = &m_contact->jacobianData_normal.m_jacobians[0];
				const Scalar* J_t1 = &m_contact->jacobianData_t1.m_jacobians[0];
				const Scalar* J_t2 = &m_contact->jacobianData_t2.m_jacobians[0];
				const Scalar* local_split_v = multibodyLinkCol->m_multiBody->getSplitVelocityVector();
				// add in the normal component of the va
				Scalar vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += local_split_v[k] * J_n[k];
				}
				va = cti.m_normal * vel;
				// add in the tangential components of the va
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += local_split_v[k] * J_t1[k];
				}
				va += m_contact->t1 * vel;
				vel = 0.0;
				for (i32 k = 0; k < ndof; ++k)
				{
					vel += local_split_v[k] * J_t2[k];
				}
				va += m_contact->t2 * vel;
			}
		}
	}
	return va;
}

Scalar DeformableRigidContactConstraint::solveConstraint(const ContactSolverInfo& infoGlobal)
{
	const SoftBody::sCti& cti = m_contact->m_cti;
	Vec3 va = getVa();
	Vec3 vb = getVb();
	Vec3 vr = vb - va;
	Scalar dn = Dot(vr, cti.m_normal) + m_total_normal_dv.dot(cti.m_normal) * infoGlobal.m_deformable_cfm;
	if (m_penetration > 0)
	{
		dn += m_penetration / infoGlobal.m_timeStep;
	}
	if (!infoGlobal.m_splitImpulse)
	{
		dn += m_penetration * infoGlobal.m_deformable_erp / infoGlobal.m_timeStep;
	}
	// dn is the normal component of velocity diffrerence. Approximates the residual. // todo xuchenhan@: this prob needs to be scaled by dt
	Vec3 impulse = m_contact->m_c0 * (vr + m_total_normal_dv * infoGlobal.m_deformable_cfm + ((m_penetration > 0) ? m_penetration / infoGlobal.m_timeStep * cti.m_normal : Vec3(0, 0, 0)));
	if (!infoGlobal.m_splitImpulse)
	{
		impulse += m_contact->m_c0 * (m_penetration * infoGlobal.m_deformable_erp / infoGlobal.m_timeStep * cti.m_normal);
	}
	Vec3 impulse_normal = m_contact->m_c0 * (cti.m_normal * dn);
	Vec3 impulse_tangent = impulse - impulse_normal;
	if (dn > 0)
	{
		return 0;
	}
	m_binding = true;
	Scalar residualSquare = dn * dn;
	Vec3 old_total_tangent_dv = m_total_tangent_dv;
	// m_c5 is the inverse mass of the deformable node/face
	m_total_normal_dv -= m_contact->m_c5 * impulse_normal;
	m_total_tangent_dv -= m_contact->m_c5 * impulse_tangent;

	if (m_total_normal_dv.dot(cti.m_normal) < 0)
	{
		// separating in the normal direction
		m_binding = false;
		m_static = false;
		impulse_tangent.setZero();
	}
	else
	{
		if (m_total_normal_dv.norm() * m_contact->m_c3 < m_total_tangent_dv.norm())
		{
			// dynamic friction
			// with dynamic friction, the impulse are still applied to the two objects colliding, however, it does not pose a constraint in the cg solve, hence the change to dv merely serves to update velocity in the contact iterations.
			m_static = false;
			if (m_total_tangent_dv.safeNorm() < SIMD_EPSILON)
			{
				m_total_tangent_dv = Vec3(0, 0, 0);
			}
			else
			{
				m_total_tangent_dv = m_total_tangent_dv.normalized() * m_total_normal_dv.safeNorm() * m_contact->m_c3;
			}
			//            impulse_tangent = -Scalar(1)/m_contact->m_c2 * (m_total_tangent_dv - old_total_tangent_dv);
			impulse_tangent = m_contact->m_c5.inverse() * (old_total_tangent_dv - m_total_tangent_dv);
		}
		else
		{
			// static friction
			m_static = true;
		}
	}
	impulse = impulse_normal + impulse_tangent;
	// apply impulse to deformable nodes involved and change their velocities
	applyImpulse(impulse);
	// apply impulse to the rigid/multibodies involved and change their velocities
	if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
	{
		RigidBody* rigidCol = 0;
		rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
		if (rigidCol)
		{
			rigidCol->applyImpulse(impulse, m_contact->m_c1);
		}
	}
	else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
	{
		MultiBodyLinkCollider* multibodyLinkCol = 0;
		multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
		if (multibodyLinkCol)
		{
			const Scalar* deltaV_normal = &m_contact->jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
			// apply normal component of the impulse
			multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_normal, impulse.dot(cti.m_normal));
			if (impulse_tangent.norm() > SIMD_EPSILON)
			{
				// apply tangential component of the impulse
				const Scalar* deltaV_t1 = &m_contact->jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
				multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_t1, impulse.dot(m_contact->t1));
				const Scalar* deltaV_t2 = &m_contact->jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];
				multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2(deltaV_t2, impulse.dot(m_contact->t2));
			}
		}
	}
	return residualSquare;
}

Scalar DeformableRigidContactConstraint::solveSplitImpulse(const ContactSolverInfo& infoGlobal)
{
	Scalar MAX_PENETRATION_CORRECTION = infoGlobal.m_deformable_maxErrorReduction;
	const SoftBody::sCti& cti = m_contact->m_cti;
	Vec3 vb = getSplitVb();
	Vec3 va = getSplitVa();
	Scalar p = m_penetration;
	if (p > 0)
	{
		return 0;
	}
	Vec3 vr = vb - va;
	Scalar dn = Dot(vr, cti.m_normal) + p * infoGlobal.m_deformable_erp / infoGlobal.m_timeStep;
	if (dn > 0)
	{
		return 0;
	}
	if (m_total_split_impulse + dn > MAX_PENETRATION_CORRECTION)
	{
		dn = MAX_PENETRATION_CORRECTION - m_total_split_impulse;
	}
	if (m_total_split_impulse + dn < -MAX_PENETRATION_CORRECTION)
	{
		dn = -MAX_PENETRATION_CORRECTION - m_total_split_impulse;
	}
	m_total_split_impulse += dn;

	Scalar residualSquare = dn * dn;
	const Vec3 impulse = m_contact->m_c0 * (cti.m_normal * dn);
	applySplitImpulse(impulse);

	// apply split impulse to the rigid/multibodies involved and change their velocities
	if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
	{
		RigidBody* rigidCol = 0;
		rigidCol = (RigidBody*)RigidBody::upcast(cti.m_colObj);
		if (rigidCol)
		{
			rigidCol->applyPushImpulse(impulse, m_contact->m_c1);
		}
	}
	else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
	{
		MultiBodyLinkCollider* multibodyLinkCol = 0;
		multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
		if (multibodyLinkCol)
		{
			const Scalar* deltaV_normal = &m_contact->jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
			// apply normal component of the impulse
			multibodyLinkCol->m_multiBody->applyDeltaSplitVeeMultiDof(deltaV_normal, impulse.dot(cti.m_normal));
		}
	}
	return residualSquare;
}
/* ================   Node vs. Rigid   =================== */
DeformableNodeRigidContactConstraint::DeformableNodeRigidContactConstraint(const SoftBody::DeformableNodeRigidContact& contact, const ContactSolverInfo& infoGlobal)
	: m_node(contact.m_node), DeformableRigidContactConstraint(contact, infoGlobal)
{
}

DeformableNodeRigidContactConstraint::DeformableNodeRigidContactConstraint(const DeformableNodeRigidContactConstraint& other)
	: m_node(other.m_node), DeformableRigidContactConstraint(other)
{
}

Vec3 DeformableNodeRigidContactConstraint::getVb() const
{
	return m_node->m_v;
}

Vec3 DeformableNodeRigidContactConstraint::getSplitVb() const
{
	return m_node->m_splitv;
}

Vec3 DeformableNodeRigidContactConstraint::getDv(const SoftBody::Node* node) const
{
	return m_total_normal_dv + m_total_tangent_dv;
}

void DeformableNodeRigidContactConstraint::applyImpulse(const Vec3& impulse)
{
	const SoftBody::DeformableNodeRigidContact* contact = getContact();
	Vec3 dv = contact->m_c5 * impulse;
	contact->m_node->m_v -= dv;
}

void DeformableNodeRigidContactConstraint::applySplitImpulse(const Vec3& impulse)
{
	const SoftBody::DeformableNodeRigidContact* contact = getContact();
	Vec3 dv = contact->m_c5 * impulse;
	contact->m_node->m_splitv -= dv;
}

/* ================   Face vs. Rigid   =================== */
DeformableFaceRigidContactConstraint::DeformableFaceRigidContactConstraint(const SoftBody::DeformableFaceRigidContact& contact, const ContactSolverInfo& infoGlobal, bool useStrainLimiting)
	: m_face(contact.m_face), m_useStrainLimiting(useStrainLimiting), DeformableRigidContactConstraint(contact, infoGlobal)
{
}

DeformableFaceRigidContactConstraint::DeformableFaceRigidContactConstraint(const DeformableFaceRigidContactConstraint& other)
	: m_face(other.m_face), m_useStrainLimiting(other.m_useStrainLimiting), DeformableRigidContactConstraint(other)
{
}

Vec3 DeformableFaceRigidContactConstraint::getVb() const
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 vb = m_face->m_n[0]->m_v * contact->m_bary[0] + m_face->m_n[1]->m_v * contact->m_bary[1] + m_face->m_n[2]->m_v * contact->m_bary[2];
	return vb;
}

Vec3 DeformableFaceRigidContactConstraint::getDv(const SoftBody::Node* node) const
{
	Vec3 face_dv = m_total_normal_dv + m_total_tangent_dv;
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	if (m_face->m_n[0] == node)
	{
		return face_dv * contact->m_weights[0];
	}
	if (m_face->m_n[1] == node)
	{
		return face_dv * contact->m_weights[1];
	}
	Assert(node == m_face->m_n[2]);
	return face_dv * contact->m_weights[2];
}

void DeformableFaceRigidContactConstraint::applyImpulse(const Vec3& impulse)
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 dv = impulse * contact->m_c2;
	SoftBody::Face* face = contact->m_face;

	Vec3& v0 = face->m_n[0]->m_v;
	Vec3& v1 = face->m_n[1]->m_v;
	Vec3& v2 = face->m_n[2]->m_v;
	const Scalar& im0 = face->m_n[0]->m_im;
	const Scalar& im1 = face->m_n[1]->m_im;
	const Scalar& im2 = face->m_n[2]->m_im;
	if (im0 > 0)
		v0 -= dv * contact->m_weights[0];
	if (im1 > 0)
		v1 -= dv * contact->m_weights[1];
	if (im2 > 0)
		v2 -= dv * contact->m_weights[2];
	if (m_useStrainLimiting)
	{
		Scalar relaxation = 1. / Scalar(m_infoGlobal->m_numIterations);
		Scalar m01 = (relaxation / (im0 + im1));
		Scalar m02 = (relaxation / (im0 + im2));
		Scalar m12 = (relaxation / (im1 + im2));
#ifdef USE_STRAIN_RATE_LIMITING
		// apply strain limiting to prevent the new velocity to change the current length of the edge by more than 1%.
		Scalar p = 0.01;
		Vec3& x0 = face->m_n[0]->m_x;
		Vec3& x1 = face->m_n[1]->m_x;
		Vec3& x2 = face->m_n[2]->m_x;
		const Vec3 x_diff[3] = {x1 - x0, x2 - x0, x2 - x1};
		const Vec3 v_diff[3] = {v1 - v0, v2 - v0, v2 - v1};
		Vec3 u[3];
		Scalar x_diff_dot_u, dn[3];
		Scalar dt = m_infoGlobal->m_timeStep;
		for (i32 i = 0; i < 3; ++i)
		{
			Scalar x_diff_norm = x_diff[i].safeNorm();
			Scalar x_diff_norm_new = (x_diff[i] + v_diff[i] * dt).safeNorm();
			Scalar strainRate = x_diff_norm_new / x_diff_norm;
			u[i] = v_diff[i];
			u[i].safeNormalize();
			if (x_diff_norm == 0 || (1 - p <= strainRate && strainRate <= 1 + p))
			{
				dn[i] = 0;
				continue;
			}
			x_diff_dot_u = Dot(x_diff[i], u[i]);
			Scalar s;
			if (1 - p > strainRate)
			{
				s = 1 / dt * (-x_diff_dot_u - Sqrt(x_diff_dot_u * x_diff_dot_u + (p * p - 2 * p) * x_diff_norm * x_diff_norm));
			}
			else
			{
				s = 1 / dt * (-x_diff_dot_u + Sqrt(x_diff_dot_u * x_diff_dot_u + (p * p + 2 * p) * x_diff_norm * x_diff_norm));
			}
			//		x_diff_norm_new = (x_diff[i] + s * u[i] * dt).safeNorm();
			//		strainRate = x_diff_norm_new/x_diff_norm;
			dn[i] = s - v_diff[i].safeNorm();
		}
		Vec3 dv0 = im0 * (m01 * u[0] * (-dn[0]) + m02 * u[1] * -(dn[1]));
		Vec3 dv1 = im1 * (m01 * u[0] * (dn[0]) + m12 * u[2] * (-dn[2]));
		Vec3 dv2 = im2 * (m12 * u[2] * (dn[2]) + m02 * u[1] * (dn[1]));
#else
		// apply strain limiting to prevent undamped modes
		Vec3 dv0 = im0 * (m01 * (v1 - v0) + m02 * (v2 - v0));
		Vec3 dv1 = im1 * (m01 * (v0 - v1) + m12 * (v2 - v1));
		Vec3 dv2 = im2 * (m12 * (v1 - v2) + m02 * (v0 - v2));
#endif
		v0 += dv0;
		v1 += dv1;
		v2 += dv2;
	}
}

Vec3 DeformableFaceRigidContactConstraint::getSplitVb() const
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 vb = (m_face->m_n[0]->m_splitv) * contact->m_bary[0] + (m_face->m_n[1]->m_splitv) * contact->m_bary[1] + (m_face->m_n[2]->m_splitv) * contact->m_bary[2];
	return vb;
}

void DeformableFaceRigidContactConstraint::applySplitImpulse(const Vec3& impulse)
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 dv = impulse * contact->m_c2;
	SoftBody::Face* face = contact->m_face;
	Vec3& v0 = face->m_n[0]->m_splitv;
	Vec3& v1 = face->m_n[1]->m_splitv;
	Vec3& v2 = face->m_n[2]->m_splitv;
	const Scalar& im0 = face->m_n[0]->m_im;
	const Scalar& im1 = face->m_n[1]->m_im;
	const Scalar& im2 = face->m_n[2]->m_im;
	if (im0 > 0)
	{
		v0 -= dv * contact->m_weights[0];
	}
	if (im1 > 0)
	{
		v1 -= dv * contact->m_weights[1];
	}
	if (im2 > 0)
	{
		v2 -= dv * contact->m_weights[2];
	}
}

/* ================   Face vs. Node   =================== */
DeformableFaceNodeContactConstraint::DeformableFaceNodeContactConstraint(const SoftBody::DeformableFaceNodeContact& contact, const ContactSolverInfo& infoGlobal)
	: m_node(contact.m_node), m_face(contact.m_face), m_contact(&contact), DeformableContactConstraint(contact.m_normal, infoGlobal)
{
	m_total_normal_dv.setZero();
	m_total_tangent_dv.setZero();
}

Vec3 DeformableFaceNodeContactConstraint::getVa() const
{
	return m_node->m_v;
}

Vec3 DeformableFaceNodeContactConstraint::getVb() const
{
	const SoftBody::DeformableFaceNodeContact* contact = getContact();
	Vec3 vb = m_face->m_n[0]->m_v * contact->m_bary[0] + m_face->m_n[1]->m_v * contact->m_bary[1] + m_face->m_n[2]->m_v * contact->m_bary[2];
	return vb;
}

Vec3 DeformableFaceNodeContactConstraint::getDv(const SoftBody::Node* n) const
{
	Vec3 dv = m_total_normal_dv + m_total_tangent_dv;
	if (n == m_node)
		return dv;
	const SoftBody::DeformableFaceNodeContact* contact = getContact();
	if (m_face->m_n[0] == n)
	{
		return dv * contact->m_weights[0];
	}
	if (m_face->m_n[1] == n)
	{
		return dv * contact->m_weights[1];
	}
	Assert(n == m_face->m_n[2]);
	return dv * contact->m_weights[2];
}

Scalar DeformableFaceNodeContactConstraint::solveConstraint(const ContactSolverInfo& infoGlobal)
{
	Vec3 va = getVa();
	Vec3 vb = getVb();
	Vec3 vr = vb - va;
	const Scalar dn = Dot(vr, m_contact->m_normal);
	// dn is the normal component of velocity diffrerence. Approximates the residual. // todo xuchenhan@: this prob needs to be scaled by dt
	Scalar residualSquare = dn * dn;
	Vec3 impulse = m_contact->m_c0 * vr;
	const Vec3 impulse_normal = m_contact->m_c0 * (m_contact->m_normal * dn);
	Vec3 impulse_tangent = impulse - impulse_normal;

	Vec3 old_total_tangent_dv = m_total_tangent_dv;
	// m_c2 is the inverse mass of the deformable node/face
	if (m_node->m_im > 0)
	{
		m_total_normal_dv -= impulse_normal * m_node->m_im;
		m_total_tangent_dv -= impulse_tangent * m_node->m_im;
	}
	else
	{
		m_total_normal_dv -= impulse_normal * m_contact->m_imf;
		m_total_tangent_dv -= impulse_tangent * m_contact->m_imf;
	}

	if (m_total_normal_dv.dot(m_contact->m_normal) > 0)
	{
		// separating in the normal direction
		m_static = false;
		m_total_tangent_dv = Vec3(0, 0, 0);
		impulse_tangent.setZero();
	}
	else
	{
		if (m_total_normal_dv.norm() * m_contact->m_friction < m_total_tangent_dv.norm())
		{
			// dynamic friction
			// with dynamic friction, the impulse are still applied to the two objects colliding, however, it does not pose a constraint in the cg solve, hence the change to dv merely serves to update velocity in the contact iterations.
			m_static = false;
			if (m_total_tangent_dv.safeNorm() < SIMD_EPSILON)
			{
				m_total_tangent_dv = Vec3(0, 0, 0);
			}
			else
			{
				m_total_tangent_dv = m_total_tangent_dv.normalized() * m_total_normal_dv.safeNorm() * m_contact->m_friction;
			}
			impulse_tangent = -Scalar(1) / m_node->m_im * (m_total_tangent_dv - old_total_tangent_dv);
		}
		else
		{
			// static friction
			m_static = true;
		}
	}
	impulse = impulse_normal + impulse_tangent;
	// apply impulse to deformable nodes involved and change their velocities
	applyImpulse(impulse);
	return residualSquare;
}

void DeformableFaceNodeContactConstraint::applyImpulse(const Vec3& impulse)
{
	const SoftBody::DeformableFaceNodeContact* contact = getContact();
	Vec3 dva = impulse * contact->m_node->m_im;
	Vec3 dvb = impulse * contact->m_imf;
	if (contact->m_node->m_im > 0)
	{
		contact->m_node->m_v += dva;
	}

	SoftBody::Face* face = contact->m_face;
	Vec3& v0 = face->m_n[0]->m_v;
	Vec3& v1 = face->m_n[1]->m_v;
	Vec3& v2 = face->m_n[2]->m_v;
	const Scalar& im0 = face->m_n[0]->m_im;
	const Scalar& im1 = face->m_n[1]->m_im;
	const Scalar& im2 = face->m_n[2]->m_im;
	if (im0 > 0)
	{
		v0 -= dvb * contact->m_weights[0];
	}
	if (im1 > 0)
	{
		v1 -= dvb * contact->m_weights[1];
	}
	if (im2 > 0)
	{
		v2 -= dvb * contact->m_weights[2];
	}
}
