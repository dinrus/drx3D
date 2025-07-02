#include <drx3D/Physics/Dynamics/MultiBody/MultiBodySphericalJointLimit.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>

MultiBodySphericalJointLimit::MultiBodySphericalJointLimit(MultiBody* body, i32 link, 
	Scalar swingxRange,
	Scalar swingyRange,
	Scalar twistRange,
	Scalar maxAppliedImpulse)
	: MultiBodyConstraint(body, body, link, body->getLink(link).m_parent, 3, true, MULTIBODY_CONSTRAINT_SPHERICAL_LIMIT),
	m_desiredVelocity(0, 0, 0),
	m_desiredPosition(0,0,0,1),
	m_use_multi_dof_params(false),
	m_kd(1., 1., 1.),
	m_kp(0.2, 0.2, 0.2),
	m_erp(1),
	m_rhsClamp(SIMD_INFINITY),
	m_maxAppliedImpulseMultiDof(maxAppliedImpulse, maxAppliedImpulse, maxAppliedImpulse),
	m_pivotA(m_bodyA->getLink(link).m_eVector),
	m_pivotB(m_bodyB->getLink(link).m_eVector),
	m_swingxRange(swingxRange),
	m_swingyRange(swingyRange),
	m_twistRange(twistRange)

{

	m_maxAppliedImpulse = maxAppliedImpulse;
}


void MultiBodySphericalJointLimit::finalizeMultiDof()
{
	allocateJacobiansMultiDof();
	// note: we rely on the fact that data.m_jacobians are
	// always initialized to zero by the Constraint ctor
	i32 linkDoF = 0;
	u32 offset = 6 + (m_bodyA->getLink(m_linkA).m_dofOffset + linkDoF);

	// row 0: the lower bound
	// row 0: the lower bound
	jacobianA(0)[offset] = 1;

	jacobianB(1)[offset] = -1;

	m_numDofsFinalized = m_jacSizeBoth;
}


MultiBodySphericalJointLimit::~MultiBodySphericalJointLimit()
{
}

i32 MultiBodySphericalJointLimit::getIslandIdA() const
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

i32 MultiBodySphericalJointLimit::getIslandIdB() const
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

void MultiBodySphericalJointLimit::createConstraintRows(MultiBodyConstraintArray& constraintRows,
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
	const Vec3 zero(0, 0, 0);

	
	Vec3 axis[3] = { Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1) };
	
	Quat currentQuat(m_bodyA->getJointPosMultiDof(m_linkA)[0],
		m_bodyA->getJointPosMultiDof(m_linkA)[1],
		m_bodyA->getJointPosMultiDof(m_linkA)[2],
		m_bodyA->getJointPosMultiDof(m_linkA)[3]);

	Quat refQuat = m_desiredPosition;
	Vec3 vTwist(0,0,1);
	
	Vec3 vConeNoTwist = quatRotate(currentQuat, vTwist);
	vConeNoTwist.normalize();
	Quat qABCone = shortestArcQuat(vTwist, vConeNoTwist);
	qABCone.normalize();
	Quat qABTwist = qABCone.inverse() * currentQuat;
	qABTwist.normalize();
	Quat desiredQuat = qABTwist;


	Quat relRot = currentQuat.inverse() * desiredQuat;
	Vec3 angleDiff;
	Generic6DofSpring2Constraint::matrixToEulerXYZ(Matrix3x3(relRot), angleDiff);
	
	Scalar limitRanges[3] = {m_swingxRange, m_swingyRange, m_twistRange};
	
	/// twist axis/angle
	Quat qMinTwist = qABTwist;
	Scalar twistAngle = qABTwist.getAngle();

	if (twistAngle > SIMD_PI)  // long way around. flip quat and recalculate.
	{
		qMinTwist = -(qABTwist);
		twistAngle = qMinTwist.getAngle();
	}
	Vec3 vTwistAxis = Vec3(qMinTwist.x(), qMinTwist.y(), qMinTwist.z());
	if (twistAngle > SIMD_EPSILON)
		vTwistAxis.normalize();
	
	if (vTwistAxis.dot(vTwist)<0)
		twistAngle*=-1.;

	angleDiff[2] = twistAngle;


	for (i32 row = 0; row < getNumRows(); row++)
	{
		Scalar allowed = limitRanges[row];
		Scalar damp = 1;
		if((angleDiff[row]>-allowed)&&(angleDiff[row]<allowed))
		{
			angleDiff[row]=0;
			damp=0;

		} else
		{
			if (angleDiff[row]>allowed)
			{
				angleDiff[row]-=allowed;
				
			}
			if (angleDiff[row]<-allowed)
			{
				angleDiff[row]+=allowed;
				
			} 
		}
		

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
					//should multiply by time step
					//max_applied_impulse *= infoGlobal.m_timeStep

					double min_applied_impulse = -max_applied_impulse;
					

					if (posError>0)
						max_applied_impulse=0;
					else
						min_applied_impulse=0;

					if (Fabs(posError)>SIMD_EPSILON)
					{
						MultiBodySolverConstraint& constraintRow = constraintRows.expandNonInitializing();
						fillMultiBodyConstraint(constraintRow, data, 0, 0, constraintNormalAng,
							zero, zero, zero,//pure angular, so zero out linear parts
							posError,
							infoGlobal,
							min_applied_impulse, max_applied_impulse, true,
							1.0, false, 0, 0,
							damp);
						constraintRow.m_orgConstraint = this;
						constraintRow.m_orgDofIndex = row;
					}
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


void MultiBodySphericalJointLimit::debugDraw(class IDebugDraw* drawer)
{
	Transform2 tr;
	tr.setIdentity();
	if (m_bodyB)
	{
		Vec3 pivotBworld = m_bodyB->localPosToWorld(m_linkB, m_pivotB);
		tr.setOrigin(pivotBworld);
		drawer->drawTransform2(tr, 0.1);
	}
}
