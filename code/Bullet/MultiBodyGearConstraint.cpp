#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyGearConstraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

MultiBodyGearConstraint::MultiBodyGearConstraint(MultiBody* bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB)
	: MultiBodyConstraint(bodyA, bodyB, linkA, linkB, 1, false, MULTIBODY_CONSTRAINT_GEAR),
	  m_gearRatio(1),
	  m_gearAuxLink(-1),
	  m_erp(0),
	  m_relativePositionTarget(0)
{
}

void MultiBodyGearConstraint::finalizeMultiDof()
{
	allocateJacobiansMultiDof();

	m_numDofsFinalized = m_jacSizeBoth;
}

MultiBodyGearConstraint::~MultiBodyGearConstraint()
{
}

i32 MultiBodyGearConstraint::getIslandIdA() const
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

i32 MultiBodyGearConstraint::getIslandIdB() const
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

void MultiBodyGearConstraint::createConstraintRows(MultiBodyConstraintArray& constraintRows,
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

	// note: we rely on the fact that data.m_jacobians are
	// always initialized to zero by the Constraint ctor
	i32 linkDoF = 0;
	u32 offsetA = 6 + (m_bodyA->getLink(m_linkA).m_dofOffset + linkDoF);
	u32 offsetB = 6 + (m_bodyB->getLink(m_linkB).m_dofOffset + linkDoF);

	// row 0: the lower bound
	jacobianA(0)[offsetA] = 1;
	jacobianB(0)[offsetB] = m_gearRatio;

	Scalar posError = 0;
	const Vec3 dummy(0, 0, 0);

	Scalar kp = 1;
	Scalar kd = 1;
	i32 numRows = getNumRows();

	for (i32 row = 0; row < numRows; row++)
	{
		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();

		i32 dof = 0;
		Scalar currentPosition = m_bodyA->getJointPosMultiDof(m_linkA)[dof];
		Scalar currentVelocity = m_bodyA->getJointVelMultiDof(m_linkA)[dof];
		Scalar auxVel = 0;

		if (m_gearAuxLink >= 0)
		{
			auxVel = m_bodyA->getJointVelMultiDof(m_gearAuxLink)[dof];
		}
		currentVelocity += auxVel;
		if (m_erp != 0)
		{
			Scalar currentPositionA = m_bodyA->getJointPosMultiDof(m_linkA)[dof];
			if (m_gearAuxLink >= 0)
			{
				currentPositionA -= m_bodyA->getJointPosMultiDof(m_gearAuxLink)[dof];
			}
			Scalar currentPositionB = m_gearRatio * m_bodyA->getJointPosMultiDof(m_linkB)[dof];
			Scalar diff = currentPositionB + currentPositionA;
			Scalar desiredPositionDiff = this->m_relativePositionTarget;
			posError = -m_erp * (desiredPositionDiff - diff);
		}

		Scalar desiredRelativeVelocity = auxVel;

		fillMultiBodyConstraint(constraintRow, data, jacobianA(row), jacobianB(row), dummy, dummy, dummy, dummy, posError, infoGlobal, -m_maxAppliedImpulse, m_maxAppliedImpulse, false, 1, false, desiredRelativeVelocity);

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
