#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySphericalJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>

MultiBodySphericalJointMotor::MultiBodySphericalJointMotor(MultiBody* body, i32 link, Scalar maxMotorImpulse)
	: MultiBodyConstraint(body, body, link, body->getLink(link).m_parent, 3, true, MULTIBODY_CONSTRAINT_SPHERICAL_MOTOR),
	m_desiredVelocity(0, 0, 0),
	m_desiredPosition(0,0,0,1),
	m_use_multi_dof_params(false),
	m_kd(1., 1., 1.),
	m_kp(0.2, 0.2, 0.2),
	m_erp(1),
	m_rhsClamp(SIMD_INFINITY),
	m_maxAppliedImpulseMultiDof(maxMotorImpulse, maxMotorImpulse, maxMotorImpulse),
	m_damping(1.0, 1.0, 1.0)
{

	m_maxAppliedImpulse = maxMotorImpulse;
}


void MultiBodySphericalJointMotor::finalizeMultiDof()
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


MultiBodySphericalJointMotor::~MultiBodySphericalJointMotor()
{
}

i32 MultiBodySphericalJointMotor::getIslandIdA() const
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

i32 MultiBodySphericalJointMotor::getIslandIdB() const
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

void MultiBodySphericalJointMotor::createConstraintRows(MultiBodyConstraintArray& constraintRows,
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

	
	Vec3 axis[3] = { Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1) };
	
	Quat desiredQuat = m_desiredPosition;
	Quat currentQuat(m_bodyA->getJointPosMultiDof(m_linkA)[0],
		m_bodyA->getJointPosMultiDof(m_linkA)[1],
		m_bodyA->getJointPosMultiDof(m_linkA)[2],
		m_bodyA->getJointPosMultiDof(m_linkA)[3]);

Quat relRot = currentQuat.inverse() * desiredQuat;
	Vec3 angleDiff;
	Generic6DofSpring2Constraint::matrixToEulerXYZ(Matrix3x3(relRot), angleDiff);



	for (i32 row = 0; row < getNumRows(); row++)
	{
		MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();

		i32 dof = row;
		
		Scalar currentVelocity = m_bodyA->getJointVelMultiDof(m_linkA)[dof];
		Scalar desiredVelocity = this->m_desiredVelocity[row];
		
		double kd = m_use_multi_dof_params ? m_kd[row % 3] : m_kd[0];
		Scalar velocityError = (desiredVelocity - currentVelocity) * kd;

		Matrix3x3 frameAworld;
		frameAworld.setIdentity();
		frameAworld = m_bodyA->localFrameToWorld(m_linkA, frameAworld);
		Scalar posError = 0;
		{
			Assert(m_bodyA->getLink(m_linkA).m_jointType == MultibodyLink::eSpherical);
			switch (m_bodyA->getLink(m_linkA).m_jointType)
			{
				case MultibodyLink::eSpherical:
				{
					Vec3 constraintNormalAng = frameAworld.getColumn(row % 3);
					double kp = m_use_multi_dof_params ? m_kp[row % 3] : m_kp[0];
					posError = kp*angleDiff[row % 3];
					double max_applied_impulse = m_use_multi_dof_params ? m_maxAppliedImpulseMultiDof[row % 3] : m_maxAppliedImpulse;
					fillMultiBodyConstraint(constraintRow, data, 0, 0, constraintNormalAng,
						Vec3(0,0,0), dummy, dummy,
						posError,
						infoGlobal,
						-max_applied_impulse, max_applied_impulse, true,
						1.0, false, 0, 0,
						m_damping[row % 3]);
					constraintRow.m_orgConstraint = this;
					constraintRow.m_orgDofIndex = row;
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
