#ifndef D3_JACOBIAN_ENTRY_H
#define D3_JACOBIAN_ENTRY_H

#include <drx3D/Common/b3Matrix3x3.h>

//notes:
// Another memory optimization would be to store m_1MinvJt in the remaining 3 w components
// which makes the b3JacobianEntry memory layout 16 bytes
// if you only are interested in angular part, just feed massInvA and massInvB zero

/// Jacobian entry is an abstraction that allows to describe constraints
/// it can be used in combination with a constraint solver
/// Can be used to relate the effect of an impulse to the constraint error
D3_ATTRIBUTE_ALIGNED16(class)
b3JacobianEntry
{
public:
	b3JacobianEntry(){};
	//constraint between two different rigidbodies
	b3JacobianEntry(
		const b3Matrix3x3& world2A,
		const b3Matrix3x3& world2B,
		const b3Vec3& rel_pos1, const b3Vec3& rel_pos2,
		const b3Vec3& jointAxis,
		const b3Vec3& inertiaInvA,
		const b3Scalar massInvA,
		const b3Vec3& inertiaInvB,
		const b3Scalar massInvB)
		: m_linearJointAxis(jointAxis)
	{
		m_aJ = world2A * (rel_pos1.cross(m_linearJointAxis));
		m_bJ = world2B * (rel_pos2.cross(-m_linearJointAxis));
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = massInvA + m_0MinvJt.dot(m_aJ) + massInvB + m_1MinvJt.dot(m_bJ);

		drx3DAssert(m_Adiag > b3Scalar(0.0));
	}

	//angular constraint between two different rigidbodies
	b3JacobianEntry(const b3Vec3& jointAxis,
					const b3Matrix3x3& world2A,
					const b3Matrix3x3& world2B,
					const b3Vec3& inertiaInvA,
					const b3Vec3& inertiaInvB)
		: m_linearJointAxis(b3MakeVector3(b3Scalar(0.), b3Scalar(0.), b3Scalar(0.)))
	{
		m_aJ = world2A * jointAxis;
		m_bJ = world2B * -jointAxis;
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = m_0MinvJt.dot(m_aJ) + m_1MinvJt.dot(m_bJ);

		drx3DAssert(m_Adiag > b3Scalar(0.0));
	}

	//angular constraint between two different rigidbodies
	b3JacobianEntry(const b3Vec3& axisInA,
					const b3Vec3& axisInB,
					const b3Vec3& inertiaInvA,
					const b3Vec3& inertiaInvB)
		: m_linearJointAxis(b3MakeVector3(b3Scalar(0.), b3Scalar(0.), b3Scalar(0.))), m_aJ(axisInA), m_bJ(-axisInB)
	{
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = m_0MinvJt.dot(m_aJ) + m_1MinvJt.dot(m_bJ);

		drx3DAssert(m_Adiag > b3Scalar(0.0));
	}

	//constraint on one rigidbody
	b3JacobianEntry(
		const b3Matrix3x3& world2A,
		const b3Vec3& rel_pos1, const b3Vec3& rel_pos2,
		const b3Vec3& jointAxis,
		const b3Vec3& inertiaInvA,
		const b3Scalar massInvA)
		: m_linearJointAxis(jointAxis)
	{
		m_aJ = world2A * (rel_pos1.cross(jointAxis));
		m_bJ = world2A * (rel_pos2.cross(-jointAxis));
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = b3MakeVector3(b3Scalar(0.), b3Scalar(0.), b3Scalar(0.));
		m_Adiag = massInvA + m_0MinvJt.dot(m_aJ);

		drx3DAssert(m_Adiag > b3Scalar(0.0));
	}

	b3Scalar getDiagonal() const { return m_Adiag; }

	// for two constraints on the same rigidbody (for example vehicle friction)
	b3Scalar getNonDiagonal(const b3JacobianEntry& jacB, const b3Scalar massInvA) const
	{
		const b3JacobianEntry& jacA = *this;
		b3Scalar lin = massInvA * jacA.m_linearJointAxis.dot(jacB.m_linearJointAxis);
		b3Scalar ang = jacA.m_0MinvJt.dot(jacB.m_aJ);
		return lin + ang;
	}

	// for two constraints on sharing two same rigidbodies (for example two contact points between two rigidbodies)
	b3Scalar getNonDiagonal(const b3JacobianEntry& jacB, const b3Scalar massInvA, const b3Scalar massInvB) const
	{
		const b3JacobianEntry& jacA = *this;
		b3Vec3 lin = jacA.m_linearJointAxis * jacB.m_linearJointAxis;
		b3Vec3 ang0 = jacA.m_0MinvJt * jacB.m_aJ;
		b3Vec3 ang1 = jacA.m_1MinvJt * jacB.m_bJ;
		b3Vec3 lin0 = massInvA * lin;
		b3Vec3 lin1 = massInvB * lin;
		b3Vec3 sum = ang0 + ang1 + lin0 + lin1;
		return sum[0] + sum[1] + sum[2];
	}

	b3Scalar getRelativeVelocity(const b3Vec3& linvelA, const b3Vec3& angvelA, const b3Vec3& linvelB, const b3Vec3& angvelB)
	{
		b3Vec3 linrel = linvelA - linvelB;
		b3Vec3 angvela = angvelA * m_aJ;
		b3Vec3 angvelb = angvelB * m_bJ;
		linrel *= m_linearJointAxis;
		angvela += angvelb;
		angvela += linrel;
		b3Scalar rel_vel2 = angvela[0] + angvela[1] + angvela[2];
		return rel_vel2 + D3_EPSILON;
	}
	//private:

	b3Vec3 m_linearJointAxis;
	b3Vec3 m_aJ;
	b3Vec3 m_bJ;
	b3Vec3 m_0MinvJt;
	b3Vec3 m_1MinvJt;
	//Optimization: can be stored in the w/last component of one of the vectors
	b3Scalar m_Adiag;
};

#endif  //D3_JACOBIAN_ENTRY_H
