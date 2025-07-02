
#ifndef DRX3D_GENERIC_6DOF_SPRING_CONSTRAINT_H
#define DRX3D_GENERIC_6DOF_SPRING_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofConstraint.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Generic6DofSpringConstraintData2 Generic6DofSpringConstraintDoubleData2
#define Generic6DofSpringConstraintDataName "Generic6DofSpringConstraintDoubleData2"
#else
#define Generic6DofSpringConstraintData2 Generic6DofSpringConstraintData
#define Generic6DofSpringConstraintDataName "Generic6DofSpringConstraintData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

/// Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF

/// DOF index used in enableSpring() and setStiffness() means:
/// 0 : translation X
/// 1 : translation Y
/// 2 : translation Z
/// 3 : rotation X (3rd Euler rotational around new position of X axis, range [-PI+epsilon, PI-epsilon] )
/// 4 : rotation Y (2nd Euler rotational around new position of Y axis, range [-PI/2+epsilon, PI/2-epsilon] )
/// 5 : rotation Z (1st Euler rotational around Z axis, range [-PI+epsilon, PI-epsilon] )

ATTRIBUTE_ALIGNED16(class)
Generic6DofSpringConstraint : public Generic6DofConstraint
{
protected:
	bool m_springEnabled[6];
	Scalar m_equilibriumPoint[6];
	Scalar m_springStiffness[6];
	Scalar m_springDamping[6];  // between 0 and 1 (1 == no damping)
	void init();
	void internalUpdateSprings(ConstraintInfo2 * info);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Generic6DofSpringConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	Generic6DofSpringConstraint(RigidBody & rbB, const Transform2& frameInB, bool useLinearReferenceFrameB);
	void enableSpring(i32 index, bool onOff);
	void setStiffness(i32 index, Scalar stiffness);
	void setDamping(i32 index, Scalar damping);
	void setEquilibriumPoint();           // set the current constraint position/orientation as an equilibrium point for all DOF
	void setEquilibriumPoint(i32 index);  // set the current constraint position/orientation as an equilibrium point for given DOF
	void setEquilibriumPoint(i32 index, Scalar val);

	bool isSpringEnabled(i32 index) const
	{
		return m_springEnabled[index];
	}

	Scalar getStiffness(i32 index) const
	{
		return m_springStiffness[index];
	}

	Scalar getDamping(i32 index) const
	{
		return m_springDamping[index];
	}

	Scalar getEquilibriumPoint(i32 index) const
	{
		return m_equilibriumPoint[index];
	}

	virtual void setAxis(const Vec3& axis1, const Vec3& axis2);

	virtual void getInfo2(ConstraintInfo2 * info);

	virtual i32 calculateSerializeBufferSize() const;
	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

struct Generic6DofSpringConstraintData
{
	Generic6DofConstraintData m_6dofData;

	i32 m_springEnabled[6];
	float m_equilibriumPoint[6];
	float m_springStiffness[6];
	float m_springDamping[6];
};

struct Generic6DofSpringConstraintDoubleData2
{
	Generic6DofConstraintDoubleData2 m_6dofData;

	i32 m_springEnabled[6];
	double m_equilibriumPoint[6];
	double m_springStiffness[6];
	double m_springDamping[6];
};

SIMD_FORCE_INLINE i32 Generic6DofSpringConstraint::calculateSerializeBufferSize() const
{
	return sizeof(Generic6DofSpringConstraintData2);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk Generic6DofSpringConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	Generic6DofSpringConstraintData2* dof = (Generic6DofSpringConstraintData2*)dataBuffer;
	Generic6DofConstraint::serialize(&dof->m_6dofData, serializer);

	i32 i;
	for (i = 0; i < 6; i++)
	{
		dof->m_equilibriumPoint[i] = m_equilibriumPoint[i];
		dof->m_springDamping[i] = m_springDamping[i];
		dof->m_springEnabled[i] = m_springEnabled[i] ? 1 : 0;
		dof->m_springStiffness[i] = m_springStiffness[i];
	}
	return Generic6DofSpringConstraintDataName;
}

#endif  // DRX3D_GENERIC_6DOF_SPRING_CONSTRAINT_H
