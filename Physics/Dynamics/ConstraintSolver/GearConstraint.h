#ifndef DRX3D_GEAR_CONSTRAINT_H
#define DRX3D_GEAR_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define GearConstraintData GearConstraintDoubleData
#define GearConstraintDataName "GearConstraintDoubleData"
#else
#define GearConstraintData GearConstraintFloatData
#define GearConstraintDataName "GearConstraintFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

///The GeatConstraint will couple the angular velocity for two bodies around given local axis and ratio.
///See drx3D/Demos/ConstraintDemo for an example use.
class GearConstraint : public TypedConstraint
{
protected:
	Vec3 m_axisInA;
	Vec3 m_axisInB;
	bool m_useFrameA;
	Scalar m_ratio;

public:
	GearConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& axisInA, const Vec3& axisInB, Scalar ratio = 1.f);
	virtual ~GearConstraint();

	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo1(ConstraintInfo1* info);

	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo2(ConstraintInfo2* info);

	void setAxisA(Vec3& axisA)
	{
		m_axisInA = axisA;
	}
	void setAxisB(Vec3& axisB)
	{
		m_axisInB = axisB;
	}
	void setRatio(Scalar ratio)
	{
		m_ratio = ratio;
	}
	const Vec3& getAxisA() const
	{
		return m_axisInA;
	}
	const Vec3& getAxisB() const
	{
		return m_axisInB;
	}
	Scalar getRatio() const
	{
		return m_ratio;
	}

	virtual void setParam(i32 num, Scalar value, i32 axis = -1)
	{
		(void)num;
		(void)value;
		(void)axis;
		Assert(0);
	}

	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const
	{
		(void)num;
		(void)axis;
		Assert(0);
		return 0.f;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct GearConstraintFloatData
{
	TypedConstraintFloatData m_typeConstraintData;

	Vec3FloatData m_axisInA;
	Vec3FloatData m_axisInB;

	float m_ratio;
	char m_padding[4];
};

struct GearConstraintDoubleData
{
	TypedConstraintDoubleData m_typeConstraintData;

	Vec3DoubleData m_axisInA;
	Vec3DoubleData m_axisInB;

	double m_ratio;
};

SIMD_FORCE_INLINE i32 GearConstraint::calculateSerializeBufferSize() const
{
	return sizeof(GearConstraintData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk GearConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	GearConstraintData* gear = (GearConstraintData*)dataBuffer;
	TypedConstraint::serialize(&gear->m_typeConstraintData, serializer);

	m_axisInA.serialize(gear->m_axisInA);
	m_axisInB.serialize(gear->m_axisInB);

	gear->m_ratio = m_ratio;

	// Fill padding with zeros to appease msan.
#ifndef DRX3D_USE_DOUBLE_PRECISION
	gear->m_padding[0] = 0;
	gear->m_padding[1] = 0;
	gear->m_padding[2] = 0;
	gear->m_padding[3] = 0;
#endif

	return GearConstraintDataName;
}

#endif  //DRX3D_GEAR_CONSTRAINT_H
