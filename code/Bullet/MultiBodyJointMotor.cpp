#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

MultiBodyJointMotor::MultiBodyJointMotor(MultiBody* body, i32 link, Scalar desiredVelocity, Scalar maxMotorImpulse)
	: MultiBodyConstraint(body, body, link, body->getLink(link).m_parent, 1, true, MULTIBODY_CONSTRAINT_1DOF_JOINT_MOTOR),
	  m_desiredVelocity(desiredVelocity),
	  m_desiredPosition(0),
	  m_kd(1.),
	  m_kp(0),
	  m_erp(1),
	  m_rhsClamp(SIMD_INFINITY)
{
	m_maxAppliedImpulse = maxMotorImpulse;
	// the data.m_jacobians never change, so may as well
	// initialize them here
}

void MultiBodyJointMotor::finalizeMultiDof()
{
	allocateJacobiansMultiDof();
	// note: we rely on the fact that data.m_jacobians are
	// always initialized to zero by the Constraint ctor
	i32 linkDoF = 0;
	u32 offset = 6 + (m_bodyA->getLink(m_linkA).m_dofOffset + linkDoF);

	// row 0: the lower bound
	// row 0: the lower bound
	jacobianA(0)[offset] = 1;

	m_numDofsFinalized = m_jacSizeBoth;
}

MultiBodyJointMotor::MultiBodyJointMotor(MultiBody* body, i32 link, i32 linkDoF, Scalar desiredVelocity, Scalar maxMotorImpulse)
	//:MultiBodyConstraint(body,0,link,-1,1,true),
	: MultiBodyConstraint(body, body, link, body->getLink(link).m_parent, 1, true, MULTIBODY_CONSTRAINT_1DOF_JOINT_MOTOR),
	  m_desiredVelocity(desiredVelocity),
	  m_desiredPosition(0),
	  m_kd(1.),
	  m_kp(0),
	  m_erp(1),
	  m_rhsClamp(SIMD_INFINITY)
{
	Assert(linkDoF < body->getLink(link).m_dofCount);

	m_maxAppliedImpulse = maxMotorImpulse;
}
MultiBodyJointMotor::~MultiBodyJointMotor()
{
}

i32 MultiBodyJointMotor::getIslandIdA() const
{
	if (this->m_linkA < 0)
	{
		MultiBodyLinkCollider* col = m_bodyA->getBaseCollider();
		if (col)
			return col->getIslandTag();
	}
	else
	{
		if (m_bodyA->getLink(m_linkA).m_collider)
		{
			return m_bodyA->getLink(m_linkA).m_collider->getIslandTag();
		}
	}
	return -1;
}

i32 MultiBodyJointMotor::getIslandIdB() const
{
	if (m_linkB < 0)
	{
		MultiBodyLinkCollider* col = m_bodyB->getBaseCollider();
		if (col)
			return col->getIslandTag();
	}
	else
	{
		if (m_bodyB->getLink(m_linkB).m_collider)
		{
			return m_bodyB->getLink(m_linkB).m_collider->getIslandTag();
		}
	}
	return -1;
}

void MultiBodyJointMotor::createConstraintRows(MultiBodyConstraintArray& constraintRows,
												 MultiBodyJacobianData& data,
												 const ContactSolverInfo& infoGlobal)
{
	// only positions need to be updated -- data.m_jacobians and force
	// directions were set in the ctor and never change.

	if (m_numDofsFinalized != m_jacSizeBoth)
	{
		finalizeMultiDof();
	}

	//don't crash
	if (m_numDofsFinalized != m_jacSizeBoth)
		return;

	if (m_maxAppliedImpulse == 0.f)
		return;

	const Scalar posError = 0;
	const Vec3 dummy(0, 0, 0);

	for (i32 row = 0; row < getNumRows(); row++)
	{
		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();

		i32 dof = 0;
		Scalar currentPosition = m_bodyA->getJointPosMultiDof(m_linkA)[dof];
		Scalar currentVelocity = m_bodyA->getJointVelMultiDof(m_linkA)[dof];
		Scalar positionStabiliationTerm = m_erp * (m_desiredPosition - currentPosition) / infoGlobal.m_timeStep;

		Scalar velocityError = (m_desiredVelocity - currentVelocity);
		Scalar rhs = m_kp * positionStabiliationTerm + currentVelocity + m_kd * velocityError;
		if (rhs > m_rhsClamp)
		{
			rhs = m_rhsClamp;
		}
		if (rhs < -m_rhsClamp)
		{
			rhs = -m_rhsClamp;
		}

		fillMultiBodyConstraint(constraintRow, data, jacobianA(row), jacobianB(row), dummy, dummy, dummy, dummy, posError, infoGlobal, -m_maxAppliedImpulse, m_maxAppliedImpulse, false, 1, false, rhs);
		constraintRow.m_orgConstraint = this;
		constraintRow.m_orgDofIndex = row;
		{
			//expect either prismatic or revolute joint type for now
			Assert((m_bodyA->getLink(m_linkA).m_jointType == MultibodyLink::eRevolute) || (m_bodyA->getLink(m_linkA).m_jointType == MultibodyLink::ePrismatic));
			switch (m_bodyA->getLink(m_linkA).m_jointType)
			{
				case MultibodyLink::eRevolute:
				{
					constraintRow.m_contactNormal1.setZero();
					constraintRow.m_contactNormal2.setZero();
					Vec3 revoluteAxisInWorld = quatRotate(m_bodyA->getLink(m_linkA).m_cachedWorldTransform.getRotation(), m_bodyA->getLink(m_linkA).m_axes[0].m_topVec);
					constraintRow.m_relpos1CrossNormal = revoluteAxisInWorld;
					constraintRow.m_relpos2CrossNormal = -revoluteAxisInWorld;

					break;
				}
				case MultibodyLink::ePrismatic:
				{
					Vec3 prismaticAxisInWorld = quatRotate(m_bodyA->getLink(m_linkA).m_cachedWorldTransform.getRotation(), m_bodyA->getLink(m_linkA).m_axes[0].m_bottomVec);
					constraintRow.m_contactNormal1 = prismaticAxisInWorld;
					constraintRow.m_contactNormal2 = -prismaticAxisInWorld;
					constraintRow.m_relpos1CrossNormal.setZero();
					constraintRow.m_relpos2CrossNormal.setZero();

					break;
				}
				default:
				{
					Assert(0);
				}
			};
		}
	}
}
