
#include <drx3D/Physics/Dynamics/Vehicle/WheelInfo.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>  // for pointvelocity

Scalar WheelInfo::getSuspensionRestLength() const
{
	return m_suspensionRestLength1;
}

void WheelInfo::updateWheel(const RigidBody& chassis, RaycastInfo& raycastInfo)
{
	(void)raycastInfo;

	if (m_raycastInfo.m_isInContact)

	{
		Scalar project = m_raycastInfo.m_contactNormalWS.dot(m_raycastInfo.m_wheelDirectionWS);
		Vec3 chassis_velocity_at_contactPoint;
		Vec3 relpos = m_raycastInfo.m_contactPointWS - chassis.getCenterOfMassPosition();
		chassis_velocity_at_contactPoint = chassis.getVelocityInLocalPoint(relpos);
		Scalar projVel = m_raycastInfo.m_contactNormalWS.dot(chassis_velocity_at_contactPoint);
		if (project >= Scalar(-0.1))
		{
			m_suspensionRelativeVelocity = Scalar(0.0);
			m_clippedInvContactDotSuspension = Scalar(1.0) / Scalar(0.1);
		}
		else
		{
			Scalar inv = Scalar(-1.) / project;
			m_suspensionRelativeVelocity = projVel * inv;
			m_clippedInvContactDotSuspension = inv;
		}
	}

	else  // Not in contact : position wheel in a nice (rest length) position
	{
		m_raycastInfo.m_suspensionLength = this->getSuspensionRestLength();
		m_suspensionRelativeVelocity = Scalar(0.0);
		m_raycastInfo.m_contactNormalWS = -m_raycastInfo.m_wheelDirectionWS;
		m_clippedInvContactDotSuspension = Scalar(1.0);
	}
}
