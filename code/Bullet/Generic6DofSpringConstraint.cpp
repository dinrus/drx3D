#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpringConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

Generic6DofSpringConstraint::Generic6DofSpringConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA)
	: Generic6DofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
{
	init();
}

Generic6DofSpringConstraint::Generic6DofSpringConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameB)
	: Generic6DofConstraint(rbB, frameInB, useLinearReferenceFrameB)
{
	init();
}

void Generic6DofSpringConstraint::init()
{
	m_objectType = D6_SPRING_CONSTRAINT_TYPE;

	for (i32 i = 0; i < 6; i++)
	{
		m_springEnabled[i] = false;
		m_equilibriumPoint[i] = Scalar(0.f);
		m_springStiffness[i] = Scalar(0.f);
		m_springDamping[i] = Scalar(1.f);
	}
}

void Generic6DofSpringConstraint::enableSpring(i32 index, bool onOff)
{
	Assert((index >= 0) && (index < 6));
	m_springEnabled[index] = onOff;
	if (index < 3)
	{
		m_linearLimits.m_enableMotor[index] = onOff;
	}
	else
	{
		m_angularLimits[index - 3].m_enableMotor = onOff;
	}
}

void Generic6DofSpringConstraint::setStiffness(i32 index, Scalar stiffness)
{
	Assert((index >= 0) && (index < 6));
	m_springStiffness[index] = stiffness;
}

void Generic6DofSpringConstraint::setDamping(i32 index, Scalar damping)
{
	Assert((index >= 0) && (index < 6));
	m_springDamping[index] = damping;
}

void Generic6DofSpringConstraint::setEquilibriumPoint()
{
	calculateTransforms();
	i32 i;

	for (i = 0; i < 3; i++)
	{
		m_equilibriumPoint[i] = m_calculatedLinearDiff[i];
	}
	for (i = 0; i < 3; i++)
	{
		m_equilibriumPoint[i + 3] = m_calculatedAxisAngleDiff[i];
	}
}

void Generic6DofSpringConstraint::setEquilibriumPoint(i32 index)
{
	Assert((index >= 0) && (index < 6));
	calculateTransforms();
	if (index < 3)
	{
		m_equilibriumPoint[index] = m_calculatedLinearDiff[index];
	}
	else
	{
		m_equilibriumPoint[index] = m_calculatedAxisAngleDiff[index - 3];
	}
}

void Generic6DofSpringConstraint::setEquilibriumPoint(i32 index, Scalar val)
{
	Assert((index >= 0) && (index < 6));
	m_equilibriumPoint[index] = val;
}

void Generic6DofSpringConstraint::internalUpdateSprings(ConstraintInfo2* info)
{
	// it is assumed that calculateTransforms() have been called before this call
	i32 i;
	//Vec3 relVel = m_rbB.getLinearVelocity() - m_rbA.getLinearVelocity();
	for (i = 0; i < 3; i++)
	{
		if (m_springEnabled[i])
		{
			// get current position of constraint
			Scalar currPos = m_calculatedLinearDiff[i];
			// calculate difference
			Scalar delta = currPos - m_equilibriumPoint[i];
			// spring force is (delta * m_stiffness) according to Hooke's Law
			Scalar force = delta * m_springStiffness[i];
			Scalar velFactor = info->fps * m_springDamping[i] / Scalar(info->m_numIterations);
			m_linearLimits.m_targetVelocity[i] = velFactor * force;
			m_linearLimits.m_maxMotorForce[i] = Fabs(force);
		}
	}
	for (i = 0; i < 3; i++)
	{
		if (m_springEnabled[i + 3])
		{
			// get current position of constraint
			Scalar currPos = m_calculatedAxisAngleDiff[i];
			// calculate difference
			Scalar delta = currPos - m_equilibriumPoint[i + 3];
			// spring force is (-delta * m_stiffness) according to Hooke's Law
			Scalar force = -delta * m_springStiffness[i + 3];
			Scalar velFactor = info->fps * m_springDamping[i + 3] / Scalar(info->m_numIterations);
			m_angularLimits[i].m_targetVelocity = velFactor * force;
			m_angularLimits[i].m_maxMotorForce = Fabs(force);
		}
	}
}

void Generic6DofSpringConstraint::getInfo2(ConstraintInfo2* info)
{
	// this will be called by constraint solver at the constraint setup stage
	// set current motor parameters
	internalUpdateSprings(info);
	// do the rest of job for constraint setup
	Generic6DofConstraint::getInfo2(info);
}

void Generic6DofSpringConstraint::setAxis(const Vec3& axis1, const Vec3& axis2)
{
	Vec3 zAxis = axis1.normalized();
	Vec3 yAxis = axis2.normalized();
	Vec3 xAxis = yAxis.cross(zAxis);  // we want right coordinate system

	Transform2 frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setVal(xAxis[0], yAxis[0], zAxis[0],
								 xAxis[1], yAxis[1], zAxis[1],
								 xAxis[2], yAxis[2], zAxis[2]);

	// now get constraint frame in local coordinate systems
	m_frameInA = m_rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = m_rbB.getCenterOfMassTransform().inverse() * frameInW;

	calculateTransforms();
}
