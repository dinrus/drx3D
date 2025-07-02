#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointLimitConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

MultiBodyJointLimitConstraint::MultiBodyJointLimitConstraint(MultiBody* body, i32 link, Scalar lower, Scalar upper)
	//:MultiBodyConstraint(body,0,link,-1,2,true),
	: MultiBodyConstraint(body, body, link, body->getLink(link).m_parent, 2, true, MULTIBODY_CONSTRAINT_LIMIT),
	  m_lowerBound(lower),
	  m_upperBound(upper)
{
}

void MultiBodyJointLimitConstraint::finalizeMultiDof()
{
	// the data.m_jacobians never change, so may as well
	// initialize them here

	allocateJacobiansMultiDof();

	u32 offset = 6 + m_bodyA->getLink(m_linkA).m_dofOffset;

	// row 0: the lower bound
	jacobianA(0)[offset] = 1;
	// row 1: the upper bound
	//jacobianA(1)[offset] = -1;
	jacobianB(1)[offset] = -1;

	m_numDofsFinalized = m_jacSizeBoth;
}

MultiBodyJointLimitConstraint::~MultiBodyJointLimitConstraint()
{
}

i32 MultiBodyJointLimitConstraint::getIslandIdA() const
{
	if (m_bodyA)
	{
		if (m_linkA < 0)
		{
			MultiBodyLinkCollider* col = m_bodyA->getBaseCollider();
			if (col)
				return col->getIslandTag();
		}
		else
		{
			if (m_bodyA->getLink(m_linkA).m_collider)
				return m_bodyA->getLink(m_linkA).m_collider->getIslandTag();
		}
	}
	return -1;
}

i32 MultiBodyJointLimitConstraint::getIslandIdB() const
{
	if (m_bodyB)
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
				return m_bodyB->getLink(m_linkB).m_collider->getIslandTag();
		}
	}
	return -1;
}

void MultiBodyJointLimitConstraint::createConstraintRows(MultiBodyConstraintArray& constraintRows,
														   MultiBodyJacobianData& data,
														   const ContactSolverInfo& infoGlobal)
{
	// only positions need to be updated -- data.m_jacobians and force
	// directions were set in the ctor and never change.

	if (m_numDofsFinalized != m_jacSizeBoth)
	{
		finalizeMultiDof();
	}

	// row 0: the lower bound
	setPosition(0, m_bodyA->getJointPos(m_linkA) - m_lowerBound);  //multidof: this is joint-type dependent

	// row 1: the upper bound
	setPosition(1, m_upperBound - m_bodyA->getJointPos(m_linkA));

	for (i32 row = 0; row < getNumRows(); row++)
	{
		Scalar penetration = getPosition(row);

		//todo: consider adding some safety threshold here
		if (penetration > 0)
		{
			continue;
		}
		Scalar direction = row ? -1 : 1;

		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();
		constraintRow.m_orgConstraint = this;
		constraintRow.m_orgDofIndex = row;

		constraintRow.m_multiBodyA = m_bodyA;
		constraintRow.m_multiBodyB = m_bodyB;
		const Scalar posError = 0;  //why assume it's zero?
		const Vec3 dummy(0, 0, 0);

		Scalar rel_vel = fillMultiBodyConstraint(constraintRow, data, jacobianA(row), jacobianB(row), dummy, dummy, dummy, dummy, posError, infoGlobal, 0, m_maxAppliedImpulse);

		{
			//expect either prismatic or revolute joint type for now
			Assert((m_bodyA->getLink(m_linkA).m_jointType == MultibodyLink::eRevolute) || (m_bodyA->getLink(m_linkA).m_jointType == MultibodyLink::ePrismatic));
			switch (m_bodyA->getLink(m_linkA).m_jointType)
			{
				case MultibodyLink::eRevolute:
				{
					constraintRow.m_contactNormal1.setZero();
					constraintRow.m_contactNormal2.setZero();
					Vec3 revoluteAxisInWorld = direction * quatRotate(m_bodyA->getLink(m_linkA).m_cachedWorldTransform.getRotation(), m_bodyA->getLink(m_linkA).m_axes[0].m_topVec);
					constraintRow.m_relpos1CrossNormal = revoluteAxisInWorld;
					constraintRow.m_relpos2CrossNormal = -revoluteAxisInWorld;

					break;
				}
				case MultibodyLink::ePrismatic:
				{
					Vec3 prismaticAxisInWorld = direction * quatRotate(m_bodyA->getLink(m_linkA).m_cachedWorldTransform.getRotation(), m_bodyA->getLink(m_linkA).m_axes[0].m_bottomVec);
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

		{
			Scalar positionalError = 0.f;
			Scalar velocityError = -rel_vel;  // * damping;
			Scalar erp = infoGlobal.m_erp2;
			if (!infoGlobal.m_splitImpulse || (penetration > infoGlobal.m_splitImpulsePenetrationThreshold))
			{
				erp = infoGlobal.m_erp;
			}
			if (penetration > 0)
			{
				positionalError = 0;
				velocityError = -penetration / infoGlobal.m_timeStep;
			}
			else
			{
				positionalError = -penetration * erp / infoGlobal.m_timeStep;
			}

			Scalar penetrationImpulse = positionalError * constraintRow.m_jacDiagABInv;
			Scalar velocityImpulse = velocityError * constraintRow.m_jacDiagABInv;
			if (!infoGlobal.m_splitImpulse || (penetration > infoGlobal.m_splitImpulsePenetrationThreshold))
			{
				//combine position and velocity into rhs
				constraintRow.m_rhs = penetrationImpulse + velocityImpulse;
				constraintRow.m_rhsPenetration = 0.f;
			}
			else
			{
				//split position and velocity into rhs and m_rhsPenetration
				constraintRow.m_rhs = velocityImpulse;
				constraintRow.m_rhsPenetration = penetrationImpulse;
			}
		}
	}
}
