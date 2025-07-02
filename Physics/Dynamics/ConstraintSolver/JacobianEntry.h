#ifndef DRX3D_JACOBIAN_ENTRY_H
#define DRX3D_JACOBIAN_ENTRY_H

#include <drx3D/Maths/Linear/Matrix3x3.h>

//notes:
// Another memory optimization would be to store m_1MinvJt in the remaining 3 w components
// which makes the JacobianEntry memory layout 16 bytes
// if you only are interested in angular part, just feed massInvA and massInvB zero

/// Jacobian entry is an abstraction that allows to describe constraints
/// it can be used in combination with a constraint solver
/// Can be used to relate the effect of an impulse to the constraint error
ATTRIBUTE_ALIGNED16(class)
JacobianEntry
{
public:
	JacobianEntry(){};
	//constraint between two different rigidbodies
	JacobianEntry(
		const Matrix3x3& world2A,
		const Matrix3x3& world2B,
		const Vec3& rel_pos1, const Vec3& rel_pos2,
		const Vec3& jointAxis,
		const Vec3& inertiaInvA,
		const Scalar massInvA,
		const Vec3& inertiaInvB,
		const Scalar massInvB)
		: m_linearJointAxis(jointAxis)
	{
		m_aJ = world2A * (rel_pos1.cross(m_linearJointAxis));
		m_bJ = world2B * (rel_pos2.cross(-m_linearJointAxis));
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = massInvA + m_0MinvJt.dot(m_aJ) + massInvB + m_1MinvJt.dot(m_bJ);

		Assert(m_Adiag > Scalar(0.0));
	}

	//angular constraint between two different rigidbodies
	JacobianEntry(const Vec3& jointAxis,
					const Matrix3x3& world2A,
					const Matrix3x3& world2B,
					const Vec3& inertiaInvA,
					const Vec3& inertiaInvB)
		: m_linearJointAxis(Vec3(Scalar(0.), Scalar(0.), Scalar(0.)))
	{
		m_aJ = world2A * jointAxis;
		m_bJ = world2B * -jointAxis;
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = m_0MinvJt.dot(m_aJ) + m_1MinvJt.dot(m_bJ);

		Assert(m_Adiag > Scalar(0.0));
	}

	//angular constraint between two different rigidbodies
	JacobianEntry(const Vec3& axisInA,
					const Vec3& axisInB,
					const Vec3& inertiaInvA,
					const Vec3& inertiaInvB)
		: m_linearJointAxis(Vec3(Scalar(0.), Scalar(0.), Scalar(0.))), m_aJ(axisInA), m_bJ(-axisInB)
	{
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = inertiaInvB * m_bJ;
		m_Adiag = m_0MinvJt.dot(m_aJ) + m_1MinvJt.dot(m_bJ);

		Assert(m_Adiag > Scalar(0.0));
	}

	//constraint on one rigidbody
	JacobianEntry(
		const Matrix3x3& world2A,
		const Vec3& rel_pos1, const Vec3& rel_pos2,
		const Vec3& jointAxis,
		const Vec3& inertiaInvA,
		const Scalar massInvA)
		: m_linearJointAxis(jointAxis)
	{
		m_aJ = world2A * (rel_pos1.cross(jointAxis));
		m_bJ = world2A * (rel_pos2.cross(-jointAxis));
		m_0MinvJt = inertiaInvA * m_aJ;
		m_1MinvJt = Vec3(Scalar(0.), Scalar(0.), Scalar(0.));
		m_Adiag = massInvA + m_0MinvJt.dot(m_aJ);

		Assert(m_Adiag > Scalar(0.0));
	}

	Scalar getDiagonal() const { return m_Adiag; }

	// for two constraints on the same rigidbody (for example vehicle friction)
	Scalar getNonDiagonal(const JacobianEntry& jacB, const Scalar massInvA) const
	{
		const JacobianEntry& jacA = *this;
		Scalar lin = massInvA * jacA.m_linearJointAxis.dot(jacB.m_linearJointAxis);
		Scalar ang = jacA.m_0MinvJt.dot(jacB.m_aJ);
		return lin + ang;
	}

	// for two constraints on sharing two same rigidbodies (for example two contact points between two rigidbodies)
	Scalar getNonDiagonal(const JacobianEntry& jacB, const Scalar massInvA, const Scalar massInvB) const
	{
		const JacobianEntry& jacA = *this;
		Vec3 lin = jacA.m_linearJointAxis * jacB.m_linearJointAxis;
		Vec3 ang0 = jacA.m_0MinvJt * jacB.m_aJ;
		Vec3 ang1 = jacA.m_1MinvJt * jacB.m_bJ;
		Vec3 lin0 = massInvA * lin;
		Vec3 lin1 = massInvB * lin;
		Vec3 sum = ang0 + ang1 + lin0 + lin1;
		return sum[0] + sum[1] + sum[2];
	}

	Scalar getRelativeVelocity(const Vec3& linvelA, const Vec3& angvelA, const Vec3& linvelB, const Vec3& angvelB)
	{
		Vec3 linrel = linvelA - linvelB;
		Vec3 angvela = angvelA * m_aJ;
		Vec3 angvelb = angvelB * m_bJ;
		linrel *= m_linearJointAxis;
		angvela += angvelb;
		angvela += linrel;
		Scalar rel_vel2 = angvela[0] + angvela[1] + angvela[2];
		return rel_vel2 + SIMD_EPSILON;
	}
	//private:

	Vec3 m_linearJointAxis;
	Vec3 m_aJ;
	Vec3 m_bJ;
	Vec3 m_0MinvJt;
	Vec3 m_1MinvJt;
	//Optimization: can be stored in the w/last component of one of the vectors
	Scalar m_Adiag;
};

#endif  //DRX3D_JACOBIAN_ENTRY_H
