#include <drx3D/Physics/Dynamics/ConstraintSolver/GearConstraint.h>

GearConstraint::GearConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& axisInA, const Vec3& axisInB, Scalar ratio)
	: TypedConstraint(GEAR_CONSTRAINT_TYPE, rbA, rbB),
	  m_axisInA(axisInA),
	  m_axisInB(axisInB),
	  m_ratio(ratio)
{
}

GearConstraint::~GearConstraint()
{
}

void GearConstraint::getInfo1(ConstraintInfo1* info)
{
	info->m_numConstraintRows = 1;
	info->nub = 1;
}

void GearConstraint::getInfo2(ConstraintInfo2* info)
{
	Vec3 globalAxisA, globalAxisB;

	globalAxisA = m_rbA.getWorldTransform().getBasis() * this->m_axisInA;
	globalAxisB = m_rbB.getWorldTransform().getBasis() * this->m_axisInB;

	info->m_J1angularAxis[0] = globalAxisA[0];
	info->m_J1angularAxis[1] = globalAxisA[1];
	info->m_J1angularAxis[2] = globalAxisA[2];

	info->m_J2angularAxis[0] = m_ratio * globalAxisB[0];
	info->m_J2angularAxis[1] = m_ratio * globalAxisB[1];
	info->m_J2angularAxis[2] = m_ratio * globalAxisB[2];
}
