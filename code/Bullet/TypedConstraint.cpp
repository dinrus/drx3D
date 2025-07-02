#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Serializer.h>

#define DEFAULT_DEBUGDRAW_SIZE Scalar(0.05f)

TypedConstraint::TypedConstraint(TypedConstraintType type, RigidBody& rbA)
	: TypedObject(type),
	  m_userConstraintType(-1),
	  m_userConstraintPtr((uk )-1),
	  m_breakingImpulseThreshold(SIMD_INFINITY),
	  m_isEnabled(true),
	  m_needsFeedback(false),
	  m_overrideNumSolverIterations(-1),
	  m_rbA(rbA),
	  m_rbB(getFixedBody()),
	  m_appliedImpulse(Scalar(0.)),
	  m_dbgDrawSize(DEFAULT_DEBUGDRAW_SIZE),
	  m_jointFeedback(0)
{
}

TypedConstraint::TypedConstraint(TypedConstraintType type, RigidBody& rbA, RigidBody& rbB)
	: TypedObject(type),
	  m_userConstraintType(-1),
	  m_userConstraintPtr((uk )-1),
	  m_breakingImpulseThreshold(SIMD_INFINITY),
	  m_isEnabled(true),
	  m_needsFeedback(false),
	  m_overrideNumSolverIterations(-1),
	  m_rbA(rbA),
	  m_rbB(rbB),
	  m_appliedImpulse(Scalar(0.)),
	  m_dbgDrawSize(DEFAULT_DEBUGDRAW_SIZE),
	  m_jointFeedback(0)
{
}

Scalar TypedConstraint::getMotorFactor(Scalar pos, Scalar lowLim, Scalar uppLim, Scalar vel, Scalar timeFact)
{
	if (lowLim > uppLim)
	{
		return Scalar(1.0f);
	}
	else if (lowLim == uppLim)
	{
		return Scalar(0.0f);
	}
	Scalar lim_fact = Scalar(1.0f);
	Scalar delta_max = vel / timeFact;
	if (delta_max < Scalar(0.0f))
	{
		if ((pos >= lowLim) && (pos < (lowLim - delta_max)))
		{
			lim_fact = (lowLim - pos) / delta_max;
		}
		else if (pos < lowLim)
		{
			lim_fact = Scalar(0.0f);
		}
		else
		{
			lim_fact = Scalar(1.0f);
		}
	}
	else if (delta_max > Scalar(0.0f))
	{
		if ((pos <= uppLim) && (pos > (uppLim - delta_max)))
		{
			lim_fact = (uppLim - pos) / delta_max;
		}
		else if (pos > uppLim)
		{
			lim_fact = Scalar(0.0f);
		}
		else
		{
			lim_fact = Scalar(1.0f);
		}
	}
	else
	{
		lim_fact = Scalar(0.0f);
	}
	return lim_fact;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk TypedConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	TypedConstraintData2* tcd = (TypedConstraintData2*)dataBuffer;

	tcd->m_rbA = (RigidBodyData*)serializer->getUniquePointer(&m_rbA);
	tcd->m_rbB = (RigidBodyData*)serializer->getUniquePointer(&m_rbB);
	tuk name = (tuk)serializer->findNameForPointer(this);
	tcd->m_name = (tuk)serializer->getUniquePointer(name);
	if (tcd->m_name)
	{
		serializer->serializeName(name);
	}

	tcd->m_objectType = m_objectType;
	tcd->m_needsFeedback = m_needsFeedback;
	tcd->m_overrideNumSolverIterations = m_overrideNumSolverIterations;
	tcd->m_breakingImpulseThreshold = m_breakingImpulseThreshold;
	tcd->m_isEnabled = m_isEnabled ? 1 : 0;

	tcd->m_userConstraintId = m_userConstraintId;
	tcd->m_userConstraintType = m_userConstraintType;

	tcd->m_appliedImpulse = m_appliedImpulse;
	tcd->m_dbgDrawSize = m_dbgDrawSize;

	tcd->m_disableCollisionsBetweenLinkedBodies = false;

	i32 i;
	for (i = 0; i < m_rbA.getNumConstraintRefs(); i++)
		if (m_rbA.getConstraintRef(i) == this)
			tcd->m_disableCollisionsBetweenLinkedBodies = true;
	for (i = 0; i < m_rbB.getNumConstraintRefs(); i++)
		if (m_rbB.getConstraintRef(i) == this)
			tcd->m_disableCollisionsBetweenLinkedBodies = true;

	return TypedConstraintDataName;
}

RigidBody& TypedConstraint::getFixedBody()
{
	static RigidBody s_fixed(0, 0, 0);
	s_fixed.setMassProps(Scalar(0.), Vec3(Scalar(0.), Scalar(0.), Scalar(0.)));
	return s_fixed;
}

void AngularLimit::set(Scalar low, Scalar high, Scalar _softness, Scalar _biasFactor, Scalar _relaxationFactor)
{
	m_halfRange = (high - low) / 2.0f;
	m_center = NormalizeAngle(low + m_halfRange);
	m_softness = _softness;
	m_biasFactor = _biasFactor;
	m_relaxationFactor = _relaxationFactor;
}

void AngularLimit::test(const Scalar angle)
{
	m_correction = 0.0f;
	m_sign = 0.0f;
	m_solveLimit = false;

	if (m_halfRange >= 0.0f)
	{
		Scalar deviation = NormalizeAngle(angle - m_center);
		if (deviation < -m_halfRange)
		{
			m_solveLimit = true;
			m_correction = -(deviation + m_halfRange);
			m_sign = +1.0f;
		}
		else if (deviation > m_halfRange)
		{
			m_solveLimit = true;
			m_correction = m_halfRange - deviation;
			m_sign = -1.0f;
		}
	}
}

Scalar AngularLimit::getError() const
{
	return m_correction * m_sign;
}

void AngularLimit::fit(Scalar& angle) const
{
	if (m_halfRange > 0.0f)
	{
		Scalar relativeAngle = NormalizeAngle(angle - m_center);
		if (!Equal(relativeAngle, m_halfRange))
		{
			if (relativeAngle > 0.0f)
			{
				angle = getHigh();
			}
			else
			{
				angle = getLow();
			}
		}
	}
}

Scalar AngularLimit::getLow() const
{
	return NormalizeAngle(m_center - m_halfRange);
}

Scalar AngularLimit::getHigh() const
{
	return NormalizeAngle(m_center + m_halfRange);
}
