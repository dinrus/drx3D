#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableContactConstraint.h>
#include <iostream>

// ================= static constraints ===================
ReducedDeformableStaticConstraint::ReducedDeformableStaticConstraint (
	ReducedDeformableBody* rsb,
	SoftBody::Node* node,
	const Vec3& ri,
	const Vec3& x0,
	const Vec3& dir,
	const ContactSolverInfo& infoGlobal,
	Scalar dt )
		: m_rsb ( rsb ), m_ri ( ri ), m_targetPos ( x0 ), m_impulseDirection ( dir ), m_dt ( dt ), DeformableStaticConstraint ( node, infoGlobal )
{
	m_erp = 0.2;
	m_appliedImpulse = 0;

	// get impulse factor
	m_impulseFactorMatrix = rsb->getImpulseFactor ( m_node->index );
	m_impulseFactor = ( m_impulseFactorMatrix * m_impulseDirection ).dot ( m_impulseDirection );

	Scalar vel_error = Dot ( -m_node->m_v, m_impulseDirection );
	Scalar pos_error = Dot ( m_targetPos - m_node->m_x, m_impulseDirection );

	m_rhs = ( vel_error + m_erp * pos_error / m_dt ) / m_impulseFactor;
}

Scalar ReducedDeformableStaticConstraint::solveConstraint ( const ContactSolverInfo& infoGlobal )
{
	// target velocity of fixed constraint is 0
	Vec3 deltaVa = getDeltaVa();
	Scalar deltaV_rel = Dot ( deltaVa, m_impulseDirection );
	Scalar deltaImpulse = m_rhs - deltaV_rel / m_impulseFactor;
	m_appliedImpulse = m_appliedImpulse + deltaImpulse;

	Vec3 impulse = deltaImpulse * m_impulseDirection;
	applyImpulse ( impulse );

	// calculate residual
	Scalar residualSquare = m_impulseFactor * deltaImpulse;
	residualSquare *= residualSquare;

	return residualSquare;
}

// this calls reduced deformable body's internalApplyFullSpaceImpulse
void ReducedDeformableStaticConstraint::applyImpulse ( const Vec3& impulse )
{
	// apply full space impulse
	m_rsb->internalApplyFullSpaceImpulse ( impulse, m_ri, m_node->index, m_dt );
}

Vec3 ReducedDeformableStaticConstraint::getDeltaVa() const
{
	return m_rsb->internalComputeNodeDeltaVelocity ( m_rsb->getInterpolationWorldTransform(), m_node->index );
}

// ================= base contact constraints ===================
ReducedDeformableRigidContactConstraint::ReducedDeformableRigidContactConstraint (
	ReducedDeformableBody* rsb,
	const SoftBody::DeformableRigidContact& c,
	const ContactSolverInfo& infoGlobal,
	Scalar dt )
		: m_rsb ( rsb ), m_dt ( dt ), DeformableRigidContactConstraint ( c, infoGlobal )
{
	m_nodeQueryIndex = 0;
	m_appliedNormalImpulse = 0;
	m_appliedTangentImpulse = 0;
	m_rhs = 0;
	m_rhs_tangent = 0;
	m_cfm = infoGlobal.m_deformable_cfm;
	m_cfm_friction = 0;
	m_erp = infoGlobal.m_deformable_erp;
	m_erp_friction = infoGlobal.m_deformable_erp;
	m_friction = infoGlobal.m_friction;

	m_collideStatic = m_contact->m_cti.m_colObj->isStaticObject();
	m_collideMultibody = ( m_contact->m_cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK );
}

void ReducedDeformableRigidContactConstraint::setSolverBody ( i32k bodyId, SolverBody& solver_body )
{
	if ( !m_collideMultibody )
	{
		m_solverBodyId = bodyId;
		m_solverBody = &solver_body;
		m_linearComponentNormal = -m_contactNormalA * m_solverBody->internalGetInvMass();
		Vec3	torqueAxis = -m_relPosA.cross ( m_contactNormalA );
		m_angularComponentNormal = m_solverBody->m_originalBody->getInvInertiaTensorWorld() * torqueAxis;

		m_linearComponentTangent = m_contactTangent * m_solverBody->internalGetInvMass();
		Vec3 torqueAxisTangent = m_relPosA.cross ( m_contactTangent );
		m_angularComponentTangent = m_solverBody->m_originalBody->getInvInertiaTensorWorld() * torqueAxisTangent;
	}
}

Vec3 ReducedDeformableRigidContactConstraint::getVa() const
{
	Vec3 Va ( 0, 0, 0 );

	if ( !m_collideStatic )
	{
		Va = DeformableRigidContactConstraint::getVa();
	}

	return Va;
}

Scalar ReducedDeformableRigidContactConstraint::solveConstraint ( const ContactSolverInfo& infoGlobal )
{
	// Vec3 Va = getVa();
	// Vec3 deltaVa = Va - m_bufferVelocityA;
	// if (!m_collideStatic)
	// {
	// std::cout << "moving collision!!!\n";
	// std::cout << "relPosA: " << m_relPosA[0] << "\t" << m_relPosA[1] << "\t" << m_relPosA[2] << "\n";
	// std::cout << "moving rigid linear_vel: " << m_solverBody->m_originalBody->getLinearVelocity()[0] << '\t'
	//  << m_solverBody->m_originalBody->getLinearVelocity()[1] << '\t'
	//   << m_solverBody->m_originalBody->getLinearVelocity()[2] << '\n';
	// }
	Vec3 deltaVa = getDeltaVa();
	Vec3 deltaVb = getDeltaVb();

	// if (!m_collideStatic)
	// {
	// 	std::cout << "deltaVa: " << deltaVa[0] << '\t' << deltaVa[1] << '\t' << deltaVa[2] << '\n';
	// 	std::cout << "deltaVb: " << deltaVb[0] << '\t' << deltaVb[1] << '\t' << deltaVb[2] << '\n';
	// }

	// get delta relative velocity and magnitude (i.e., how much impulse has been applied?)
	Vec3 deltaV_rel = deltaVa - deltaVb;
	Scalar deltaV_rel_normal = -Dot ( deltaV_rel, m_contactNormalA );

	// if (!m_collideStatic)
	// {
	// 	std::cout << "deltaV_rel: " << deltaV_rel[0] << '\t' << deltaV_rel[1] << '\t' << deltaV_rel[2] << "\n";
	// 	std::cout << "deltaV_rel_normal: " << deltaV_rel_normal << "\n";
	// 	std::cout << "normal_A: " << m_contactNormalA[0] << '\t' << m_contactNormalA[1] << '\t' << m_contactNormalA[2] << '\n';
	// }

	// get the normal impulse to be applied
	Scalar deltaImpulse = m_rhs - m_appliedNormalImpulse * m_cfm - deltaV_rel_normal / m_normalImpulseFactor;
	// if (!m_collideStatic)
	// {
	// 	std::cout << "m_rhs: " << m_rhs << '\t' << "m_appliedNormalImpulse: "  << m_appliedNormalImpulse << "\n";
	// 	std::cout << "m_normalImpulseFactor: " << m_normalImpulseFactor << '\n';
	// }

	{
		// cumulative impulse that has been applied
		Scalar sum = m_appliedNormalImpulse + deltaImpulse;
		// if the cumulative impulse is pushing the object into the rigid body, set it zero

		if ( sum < 0 )
		{
			deltaImpulse = -m_appliedNormalImpulse;
			m_appliedNormalImpulse = 0;
		}

		else
		{
			m_appliedNormalImpulse = sum;
		}
	}

	// if (!m_collideStatic)
	// {
	// 	std::cout << "m_appliedNormalImpulse: " << m_appliedNormalImpulse << '\n';
	// 	std::cout << "deltaImpulse: " << deltaImpulse << '\n';
	// }

	// residual is the nodal normal velocity change in current iteration
	Scalar residualSquare = deltaImpulse * m_normalImpulseFactor;	// get residual
	residualSquare *= residualSquare;


	// apply Coulomb friction (based on delta velocity, |dv_t| = |dv_n * friction|)
	Scalar deltaImpulse_tangent = 0;
	Scalar deltaImpulse_tangent2 = 0;
	{
		// calculate how much impulse is needed
		// Scalar deltaV_rel_tangent = Dot(deltaV_rel, m_contactTangent);
		// Scalar impulse_changed = deltaV_rel_tangent * m_tangentImpulseFactorInv;
		// deltaImpulse_tangent = m_rhs_tangent - impulse_changed;

		// Scalar sum = m_appliedTangentImpulse + deltaImpulse_tangent;
		Scalar lower_limit = - m_appliedNormalImpulse * m_friction;
		Scalar upper_limit = m_appliedNormalImpulse * m_friction;
		// if (sum > upper_limit)
		// {
		// 	deltaImpulse_tangent = upper_limit - m_appliedTangentImpulse;
		// 	m_appliedTangentImpulse = upper_limit;
		// }
		// else if (sum < lower_limit)
		// {
		// 	deltaImpulse_tangent = lower_limit - m_appliedTangentImpulse;
		// 	m_appliedTangentImpulse = lower_limit;
		// }
		// else
		// {
		// 	m_appliedTangentImpulse = sum;
		// }
		//
		calculateTangentialImpulse ( deltaImpulse_tangent, m_appliedTangentImpulse, m_rhs_tangent,
				m_tangentImpulseFactorInv, m_contactTangent, lower_limit, upper_limit, deltaV_rel );

		if ( m_collideMultibody )
		{
			calculateTangentialImpulse ( deltaImpulse_tangent2, m_appliedTangentImpulse2, m_rhs_tangent2,
					m_tangentImpulseFactorInv2, m_contactTangent2, lower_limit, upper_limit, deltaV_rel );
		}


		if ( !m_collideStatic )
		{
			// std::cout << "m_contactTangent: " << m_contactTangent[0] << "\t"  << m_contactTangent[1] << "\t"  << m_contactTangent[2] << "\n";
			// std::cout << "deltaV_rel_tangent: " << deltaV_rel_tangent << '\n';
			// std::cout << "deltaImpulseTangent: " << deltaImpulse_tangent << '\n';
			// std::cout << "m_appliedTangentImpulse: " << m_appliedTangentImpulse << '\n';
		}
	}

	// get the total impulse vector
	Vec3 impulse_normal = deltaImpulse * m_contactNormalA;
	Vec3 impulse_tangent = deltaImpulse_tangent * ( -m_contactTangent );
	Vec3 impulse_tangent2 = deltaImpulse_tangent2 * ( -m_contactTangent2 );
	Vec3 impulse = impulse_normal + impulse_tangent + impulse_tangent2;

	applyImpulse ( impulse );

	// apply impulse to the rigid/multibodies involved and change their velocities

	if ( !m_collideStatic )
	{
		// std::cout << "linear_component: " << m_linearComponentNormal[0] << '\t'
		// 																	<< m_linearComponentNormal[1] << '\t'
		// 																	<< m_linearComponentNormal[2] << '\n';
		// std::cout << "angular_component: " << m_angularComponentNormal[0] << '\t'
		// 																	<< m_angularComponentNormal[1] << '\t'
		// 																	<< m_angularComponentNormal[2] << '\n';

		if ( !m_collideMultibody )		// collision with rigid body
		{
			// std::cout << "rigid impulse applied!!\n";
			// std::cout << "delta Linear: " << m_solverBody->getDeltaLinearVelocity()[0] << '\t'
			// << m_solverBody->getDeltaLinearVelocity()[1] << '\t'
			// 	<< m_solverBody->getDeltaLinearVelocity()[2] << '\n';
			// std::cout << "delta Angular: " << m_solverBody->getDeltaAngularVelocity()[0] << '\t'
			// << m_solverBody->getDeltaAngularVelocity()[1] << '\t'
			// 	<< m_solverBody->getDeltaAngularVelocity()[2] << '\n';

			m_solverBody->internalApplyImpulse ( m_linearComponentNormal, m_angularComponentNormal, deltaImpulse );
			m_solverBody->internalApplyImpulse ( m_linearComponentTangent, m_angularComponentTangent, deltaImpulse_tangent );

			// std::cout << "after\n";
			// std::cout << "rigid impulse applied!!\n";
			// std::cout << "delta Linear: " << m_solverBody->getDeltaLinearVelocity()[0] << '\t'
			// << m_solverBody->getDeltaLinearVelocity()[1] << '\t'
			// 	<< m_solverBody->getDeltaLinearVelocity()[2] << '\n';
			// std::cout << "delta Angular: " << m_solverBody->getDeltaAngularVelocity()[0] << '\t'
			// << m_solverBody->getDeltaAngularVelocity()[1] << '\t'
			// 	<< m_solverBody->getDeltaAngularVelocity()[2] << '\n';
		}

		else		// collision with multibody
		{
			MultiBodyLinkCollider* multibodyLinkCol = 0;
			multibodyLinkCol = ( MultiBodyLinkCollider* ) MultiBodyLinkCollider::upcast ( m_contact->m_cti.m_colObj );

			if ( multibodyLinkCol )
			{
				const Scalar* deltaV_normal = &m_contact->jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
				// apply normal component of the impulse
				multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2 ( deltaV_normal, -deltaImpulse );

				// i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
				// std::cout << "deltaV_normal: ";
				// for (i32 i = 0; i < ndof; ++i)
				// {
				// 	std::cout << i << "\t" << deltaV_normal[i] << '\n';
				// }

				if ( impulse_tangent.norm() > SIMD_EPSILON )
				{
					// apply tangential component of the impulse
					const Scalar* deltaV_t1 = &m_contact->jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
					multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2 ( deltaV_t1, deltaImpulse_tangent );
					const Scalar* deltaV_t2 = &m_contact->jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];
					multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof2 ( deltaV_t2, deltaImpulse_tangent2 );
				}
			}
		}
	}

	return residualSquare;
}

void ReducedDeformableRigidContactConstraint::calculateTangentialImpulse ( Scalar& deltaImpulse_tangent,
		Scalar& appliedImpulse,
		const Scalar rhs_tangent,
		const Scalar tangentImpulseFactorInv,
		const Vec3& tangent,
		const Scalar lower_limit,
		const Scalar upper_limit,
		const Vec3& deltaV_rel )
{
	Scalar deltaV_rel_tangent = Dot ( deltaV_rel, tangent );
	Scalar impulse_changed = deltaV_rel_tangent * tangentImpulseFactorInv;
	deltaImpulse_tangent = rhs_tangent - m_cfm_friction * appliedImpulse - impulse_changed;

	Scalar sum = appliedImpulse + deltaImpulse_tangent;

	if ( sum > upper_limit )
	{
		deltaImpulse_tangent = upper_limit - appliedImpulse;
		appliedImpulse = upper_limit;
	}

	else
		if ( sum < lower_limit )
		{
			deltaImpulse_tangent = lower_limit - appliedImpulse;
			appliedImpulse = lower_limit;
		}

		else
		{
			appliedImpulse = sum;
		}
}

// ================= node vs rigid constraints ===================
ReducedDeformableNodeRigidContactConstraint::ReducedDeformableNodeRigidContactConstraint (
	ReducedDeformableBody* rsb,
	const SoftBody::DeformableNodeRigidContact& contact,
	const ContactSolverInfo& infoGlobal,
	Scalar dt )
		: m_node ( contact.m_node ), ReducedDeformableRigidContactConstraint ( rsb, contact, infoGlobal, dt )
{
	m_contactNormalA = contact.m_cti.m_normal;
	m_contactNormalB = -contact.m_cti.m_normal;

	if ( contact.m_node->index < rsb->m_nodes.size() )
	{
		m_nodeQueryIndex = contact.m_node->index;
	}

	else
	{
		m_nodeQueryIndex = m_node->index - rsb->m_nodeIndexOffset;
	}

	if ( m_contact->m_cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY )
	{
		m_relPosA = contact.m_c1;
	}

	else
	{
		m_relPosA = Vec3 ( 0, 0, 0 );
	}

	m_relPosB = m_node->m_x - m_rsb->getRigidTransform().getOrigin();

	if ( m_collideStatic )		// colliding with static object, only consider reduced deformable body's impulse factor
	{
		m_impulseFactor = m_rsb->getImpulseFactor ( m_nodeQueryIndex );
	}

	else		// colliding with dynamic object, consider both reduced deformable and rigid body's impulse factors
	{
		m_impulseFactor = m_rsb->getImpulseFactor ( m_nodeQueryIndex ) + contact.m_c0;
	}

	m_normalImpulseFactor = ( m_impulseFactor * m_contactNormalA ).dot ( m_contactNormalA );

	m_tangentImpulseFactor = 0;

	warmStarting();
}

void ReducedDeformableNodeRigidContactConstraint::warmStarting()
{
	Vec3 va = getVa();
	Vec3 vb = getVb();
	m_bufferVelocityA = va;
	m_bufferVelocityB = vb;

	// we define the (+) direction of errors to be the outward surface normal of the rigid object
	Vec3 v_rel = vb - va;
	// get tangent direction of the relative velocity
	Vec3 v_tangent = v_rel - v_rel.dot ( m_contactNormalA ) * m_contactNormalA;

	if ( v_tangent.norm() < SIMD_EPSILON )
	{
		m_contactTangent = Vec3 ( 0, 0, 0 );
		// tangent impulse factor
		m_tangentImpulseFactor = 0;
		m_tangentImpulseFactorInv = 0;
	}

	else
	{
		if ( !m_collideMultibody )
		{
			m_contactTangent = v_tangent.normalized();
			m_contactTangent2.setZero();
			// tangent impulse factor 1
			m_tangentImpulseFactor = ( m_impulseFactor * m_contactTangent ).dot ( m_contactTangent );
			m_tangentImpulseFactorInv = Scalar ( 1 ) / m_tangentImpulseFactor;
			// tangent impulse factor 2
			m_tangentImpulseFactor2 = 0;
			m_tangentImpulseFactorInv2 = 0;
		}

		else	// multibody requires 2 contact directions
		{
			m_contactTangent = m_contact->t1;
			m_contactTangent2 = m_contact->t2;

			// tangent impulse factor 1
			m_tangentImpulseFactor = ( m_impulseFactor * m_contactTangent ).dot ( m_contactTangent );
			m_tangentImpulseFactorInv = Scalar ( 1 ) / m_tangentImpulseFactor;
			// tangent impulse factor 2
			m_tangentImpulseFactor2 = ( m_impulseFactor * m_contactTangent2 ).dot ( m_contactTangent2 );
			m_tangentImpulseFactorInv2 = Scalar ( 1 ) / m_tangentImpulseFactor2;
		}
	}


	// initial guess for normal impulse
	{
		Scalar velocity_error = Dot ( v_rel, m_contactNormalA );	// magnitude of relative velocity
		Scalar position_error = 0;

		if ( m_penetration > 0 )
		{
			velocity_error += m_penetration / m_dt;
		}

		else
		{
			// add penetration correction vel
			position_error = m_penetration * m_erp / m_dt;
		}

		// get the initial estimate of impulse magnitude to be applied
		m_rhs = - ( velocity_error + position_error ) / m_normalImpulseFactor;
	}

	// initial guess for tangential impulse
	{
		Scalar velocity_error = Dot ( v_rel, m_contactTangent );
		m_rhs_tangent = velocity_error * m_tangentImpulseFactorInv;

		if ( m_collideMultibody )
		{
			Scalar velocity_error2 = Dot ( v_rel, m_contactTangent2 );
			m_rhs_tangent2 = velocity_error2 * m_tangentImpulseFactorInv2;
		}
	}

	// warm starting
	// applyImpulse(m_rhs * m_contactNormalA);
	// if (!m_collideStatic)
	// {
	// 	const SoftBody::sCti& cti = m_contact->m_cti;
	// 	if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
	// 	{
	// 		m_solverBody->internalApplyImpulse(m_linearComponentNormal, m_angularComponentNormal, -m_rhs);
	// 	}
	// }
}

Vec3 ReducedDeformableNodeRigidContactConstraint::getVb() const
{
	return m_node->m_v;
}

Vec3 ReducedDeformableNodeRigidContactConstraint::getDeltaVa() const
{
	Vec3 deltaVa ( 0, 0, 0 );

	if ( !m_collideStatic )
	{
		if ( !m_collideMultibody )		// for rigid body
		{
			deltaVa = m_solverBody->internalGetDeltaLinearVelocity() + m_solverBody->internalGetDeltaAngularVelocity().cross ( m_relPosA );
		}

		else		// for multibody
		{
			MultiBodyLinkCollider* multibodyLinkCol = 0;
			multibodyLinkCol = ( MultiBodyLinkCollider* ) MultiBodyLinkCollider::upcast ( m_contact->m_cti.m_colObj );

			if ( multibodyLinkCol )
			{
				i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
				const Scalar* J_n = &m_contact->jacobianData_normal.m_jacobians[0];
				const Scalar* J_t1 = &m_contact->jacobianData_t1.m_jacobians[0];
				const Scalar* J_t2 = &m_contact->jacobianData_t2.m_jacobians[0];
				const Scalar* local_dv = multibodyLinkCol->m_multiBody->getDeltaVelocityVector();
				// add in the normal component of the va
				Scalar vel = 0;

				for ( i32 k = 0; k < ndof; ++k )
				{
					vel += local_dv[k] * J_n[k];
				}

				deltaVa = m_contact->m_cti.m_normal * vel;

				// add in the tangential components of the va
				vel = 0;

				for ( i32 k = 0; k < ndof; ++k )
				{
					vel += local_dv[k] * J_t1[k];
				}

				deltaVa += m_contact->t1 * vel;

				vel = 0;

				for ( i32 k = 0; k < ndof; ++k )
				{
					vel += local_dv[k] * J_t2[k];
				}

				deltaVa += m_contact->t2 * vel;
			}
		}
	}

	return deltaVa;
}

Vec3 ReducedDeformableNodeRigidContactConstraint::getDeltaVb() const
{
	// std::cout << "node: " << m_node->index << '\n';
	return m_rsb->internalComputeNodeDeltaVelocity ( m_rsb->getInterpolationWorldTransform(), m_nodeQueryIndex );
}

Vec3 ReducedDeformableNodeRigidContactConstraint::getSplitVb() const
{
	return m_node->m_splitv;
}

Vec3 ReducedDeformableNodeRigidContactConstraint::getDv ( const SoftBody::Node* node ) const
{
	return m_total_normal_dv + m_total_tangent_dv;
}

void ReducedDeformableNodeRigidContactConstraint::applyImpulse ( const Vec3& impulse )
{
	m_rsb->internalApplyFullSpaceImpulse ( impulse, m_relPosB, m_nodeQueryIndex, m_dt );
	// m_rsb->applyFullSpaceImpulse(impulse, m_relPosB, m_node->index, m_dt);
	// m_rsb->mapToFullVelocity(m_rsb->getInterpolationWorldTransform());
	// if (!m_collideStatic)
	// {
	// 	// std::cout << "impulse applied: " << impulse[0] << '\t' << impulse[1] << '\t' << impulse[2] << '\n';
	// 	// std::cout << "node: " << m_node->index << " vel: " << m_node->m_v[0] << '\t' << m_node->m_v[1] << '\t' << m_node->m_v[2] << '\n';
	// 	Vec3 v_after = getDeltaVb() + m_node->m_v;
	// 	// std::cout << "vel after: " << v_after[0] << '\t' << v_after[1] << '\t' << v_after[2] << '\n';
	// }
	// std::cout << "node: " << m_node->index << " pos: " << m_node->m_x[0] << '\t' << m_node->m_x[1] << '\t' << m_node->m_x[2] << '\n';
}

// ================= face vs rigid constraints ===================
ReducedDeformableFaceRigidContactConstraint::ReducedDeformableFaceRigidContactConstraint (
	ReducedDeformableBody* rsb,
	const SoftBody::DeformableFaceRigidContact& contact,
	const ContactSolverInfo& infoGlobal,
	Scalar dt,
	bool useStrainLimiting )
		: m_face ( contact.m_face ), m_useStrainLimiting ( useStrainLimiting ), ReducedDeformableRigidContactConstraint ( rsb, contact, infoGlobal, dt )
{}

Vec3 ReducedDeformableFaceRigidContactConstraint::getVb() const
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 vb = m_face->m_n[0]->m_v * contact->m_bary[0] + m_face->m_n[1]->m_v * contact->m_bary[1] + m_face->m_n[2]->m_v * contact->m_bary[2];
	return vb;
}

Vec3 ReducedDeformableFaceRigidContactConstraint::getSplitVb() const
{
	const SoftBody::DeformableFaceRigidContact* contact = getContact();
	Vec3 vb = ( m_face->m_n[0]->m_splitv ) * contact->m_bary[0] + ( m_face->m_n[1]->m_splitv ) * contact->m_bary[1] + ( m_face->m_n[2]->m_splitv ) * contact->m_bary[2];
	return vb;
}

Vec3 ReducedDeformableFaceRigidContactConstraint::getDv ( const SoftBody::Node* node ) const
{
	Vec3 face_dv = m_total_normal_dv + m_total_tangent_dv;
	const SoftBody::DeformableFaceRigidContact* contact = getContact();

	if ( m_face->m_n[0] == node )
	{
		return face_dv * contact->m_weights[0];
	}

	if ( m_face->m_n[1] == node )
	{
		return face_dv * contact->m_weights[1];
	}

	Assert ( node == m_face->m_n[2] );

	return face_dv * contact->m_weights[2];
}

void ReducedDeformableFaceRigidContactConstraint::applyImpulse ( const Vec3& impulse )
{
	//
}
