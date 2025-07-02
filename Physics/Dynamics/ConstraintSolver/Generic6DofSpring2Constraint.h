#ifndef DRX3D_GENERIC_6DOF_CONSTRAINT2_H
#define DRX3D_GENERIC_6DOF_CONSTRAINT2_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

class RigidBody;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Generic6DofSpring2ConstraintData2 Generic6DofSpring2ConstraintDoubleData2
#define Generic6DofSpring2ConstraintDataName "Generic6DofSpring2ConstraintDoubleData2"
#else
#define Generic6DofSpring2ConstraintData2 Generic6DofSpring2ConstraintData
#define Generic6DofSpring2ConstraintDataName "Generic6DofSpring2ConstraintData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

enum RotateOrder
{
	RO_XYZ = 0,
	RO_XZY,
	RO_YXZ,
	RO_YZX,
	RO_ZXY,
	RO_ZYX
};

class RotationalLimitMotor2
{
public:
	// upper < lower means free
	// upper == lower means locked
	// upper > lower means limited
	Scalar m_loLimit;
	Scalar m_hiLimit;
	Scalar m_bounce;
	Scalar m_stopERP;
	Scalar m_stopCFM;
	Scalar m_motorERP;
	Scalar m_motorCFM;
	bool m_enableMotor;
	Scalar m_targetVelocity;
	Scalar m_maxMotorForce;
	bool m_servoMotor;
	Scalar m_servoTarget;
	bool m_enableSpring;
	Scalar m_springStiffness;
	bool m_springStiffnessLimited;
	Scalar m_springDamping;
	bool m_springDampingLimited;
	Scalar m_equilibriumPoint;

	Scalar m_currentLimitError;
	Scalar m_currentLimitErrorHi;
	Scalar m_currentPosition;
	i32 m_currentLimit;

	RotationalLimitMotor2()
	{
		m_loLimit = 1.0f;
		m_hiLimit = -1.0f;
		m_bounce = 0.0f;
		m_stopERP = 0.2f;
		m_stopCFM = 0.f;
		m_motorERP = 0.9f;
		m_motorCFM = 0.f;
		m_enableMotor = false;
		m_targetVelocity = 0;
		m_maxMotorForce = 6.0f;
		m_servoMotor = false;
		m_servoTarget = 0;
		m_enableSpring = false;
		m_springStiffness = 0;
		m_springStiffnessLimited = false;
		m_springDamping = 0;
		m_springDampingLimited = false;
		m_equilibriumPoint = 0;

		m_currentLimitError = 0;
		m_currentLimitErrorHi = 0;
		m_currentPosition = 0;
		m_currentLimit = 0;
	}

	RotationalLimitMotor2(const RotationalLimitMotor2& limot)
	{
		m_loLimit = limot.m_loLimit;
		m_hiLimit = limot.m_hiLimit;
		m_bounce = limot.m_bounce;
		m_stopERP = limot.m_stopERP;
		m_stopCFM = limot.m_stopCFM;
		m_motorERP = limot.m_motorERP;
		m_motorCFM = limot.m_motorCFM;
		m_enableMotor = limot.m_enableMotor;
		m_targetVelocity = limot.m_targetVelocity;
		m_maxMotorForce = limot.m_maxMotorForce;
		m_servoMotor = limot.m_servoMotor;
		m_servoTarget = limot.m_servoTarget;
		m_enableSpring = limot.m_enableSpring;
		m_springStiffness = limot.m_springStiffness;
		m_springStiffnessLimited = limot.m_springStiffnessLimited;
		m_springDamping = limot.m_springDamping;
		m_springDampingLimited = limot.m_springDampingLimited;
		m_equilibriumPoint = limot.m_equilibriumPoint;

		m_currentLimitError = limot.m_currentLimitError;
		m_currentLimitErrorHi = limot.m_currentLimitErrorHi;
		m_currentPosition = limot.m_currentPosition;
		m_currentLimit = limot.m_currentLimit;
	}

	bool isLimited()
	{
		if (m_loLimit > m_hiLimit) return false;
		return true;
	}

	void testLimitValue(Scalar test_value);
};

class TranslationalLimitMotor2
{
public:
	// upper < lower means free
	// upper == lower means locked
	// upper > lower means limited
	Vec3 m_lowerLimit;
	Vec3 m_upperLimit;
	Vec3 m_bounce;
	Vec3 m_stopERP;
	Vec3 m_stopCFM;
	Vec3 m_motorERP;
	Vec3 m_motorCFM;
	bool m_enableMotor[3];
	bool m_servoMotor[3];
	bool m_enableSpring[3];
	Vec3 m_servoTarget;
	Vec3 m_springStiffness;
	bool m_springStiffnessLimited[3];
	Vec3 m_springDamping;
	bool m_springDampingLimited[3];
	Vec3 m_equilibriumPoint;
	Vec3 m_targetVelocity;
	Vec3 m_maxMotorForce;

	Vec3 m_currentLimitError;
	Vec3 m_currentLimitErrorHi;
	Vec3 m_currentLinearDiff;
	i32 m_currentLimit[3];

	TranslationalLimitMotor2()
	{
		m_lowerLimit.setVal(0.f, 0.f, 0.f);
		m_upperLimit.setVal(0.f, 0.f, 0.f);
		m_bounce.setVal(0.f, 0.f, 0.f);
		m_stopERP.setVal(0.2f, 0.2f, 0.2f);
		m_stopCFM.setVal(0.f, 0.f, 0.f);
		m_motorERP.setVal(0.9f, 0.9f, 0.9f);
		m_motorCFM.setVal(0.f, 0.f, 0.f);

		m_currentLimitError.setVal(0.f, 0.f, 0.f);
		m_currentLimitErrorHi.setVal(0.f, 0.f, 0.f);
		m_currentLinearDiff.setVal(0.f, 0.f, 0.f);

		for (i32 i = 0; i < 3; i++)
		{
			m_enableMotor[i] = false;
			m_servoMotor[i] = false;
			m_enableSpring[i] = false;
			m_servoTarget[i] = Scalar(0.f);
			m_springStiffness[i] = Scalar(0.f);
			m_springStiffnessLimited[i] = false;
			m_springDamping[i] = Scalar(0.f);
			m_springDampingLimited[i] = false;
			m_equilibriumPoint[i] = Scalar(0.f);
			m_targetVelocity[i] = Scalar(0.f);
			m_maxMotorForce[i] = Scalar(0.f);

			m_currentLimit[i] = 0;
		}
	}

	TranslationalLimitMotor2(const TranslationalLimitMotor2& other)
	{
		m_lowerLimit = other.m_lowerLimit;
		m_upperLimit = other.m_upperLimit;
		m_bounce = other.m_bounce;
		m_stopERP = other.m_stopERP;
		m_stopCFM = other.m_stopCFM;
		m_motorERP = other.m_motorERP;
		m_motorCFM = other.m_motorCFM;

		m_currentLimitError = other.m_currentLimitError;
		m_currentLimitErrorHi = other.m_currentLimitErrorHi;
		m_currentLinearDiff = other.m_currentLinearDiff;

		for (i32 i = 0; i < 3; i++)
		{
			m_enableMotor[i] = other.m_enableMotor[i];
			m_servoMotor[i] = other.m_servoMotor[i];
			m_enableSpring[i] = other.m_enableSpring[i];
			m_servoTarget[i] = other.m_servoTarget[i];
			m_springStiffness[i] = other.m_springStiffness[i];
			m_springStiffnessLimited[i] = other.m_springStiffnessLimited[i];
			m_springDamping[i] = other.m_springDamping[i];
			m_springDampingLimited[i] = other.m_springDampingLimited[i];
			m_equilibriumPoint[i] = other.m_equilibriumPoint[i];
			m_targetVelocity[i] = other.m_targetVelocity[i];
			m_maxMotorForce[i] = other.m_maxMotorForce[i];

			m_currentLimit[i] = other.m_currentLimit[i];
		}
	}

	inline bool isLimited(i32 limitIndex)
	{
		return (m_upperLimit[limitIndex] >= m_lowerLimit[limitIndex]);
	}

	void testLimitValue(i32 limitIndex, Scalar test_value);
};

enum bt6DofFlags2
{
	DRX3D_6DOF_FLAGS_CFM_STOP2 = 1,
	DRX3D_6DOF_FLAGS_ERP_STOP2 = 2,
	DRX3D_6DOF_FLAGS_CFM_MOTO2 = 4,
	DRX3D_6DOF_FLAGS_ERP_MOTO2 = 8,
	DRX3D_6DOF_FLAGS_USE_INFINITE_ERROR = (1<<16)
};
#define DRX3D_6DOF_FLAGS_AXIS_SHIFT2 4  // bits per axis

ATTRIBUTE_ALIGNED16(class)
Generic6DofSpring2Constraint : public TypedConstraint
{
protected:
	Transform2 m_frameInA;
	Transform2 m_frameInB;

	JacobianEntry m_jacLinear[3];
	JacobianEntry m_jacAng[3];

	TranslationalLimitMotor2 m_linearLimits;
	RotationalLimitMotor2 m_angularLimits[3];

	RotateOrder m_rotateOrder;

protected:
	Transform2 m_calculatedTransformA;
	Transform2 m_calculatedTransformB;
	Vec3 m_calculatedAxisAngleDiff;
	Vec3 m_calculatedAxis[3];
	Vec3 m_calculatedLinearDiff;
	Scalar m_factA;
	Scalar m_factB;
	bool m_hasStaticBody;
	i32 m_flags;

	Generic6DofSpring2Constraint& operator=(const Generic6DofSpring2Constraint&)
	{
		Assert(0);
		return *this;
	}

	i32 setAngularLimits(ConstraintInfo2 * info, i32 row_offset, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB);
	i32 setLinearLimits(ConstraintInfo2 * info, i32 row, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB);

	void calculateLinearInfo();
	void calculateAngleInfo();
	void testAngularLimitMotor(i32 axis_index);

	void calculateJacobi(RotationalLimitMotor2 * limot, const Transform2& transA, const Transform2& transB, ConstraintInfo2* info, i32 srow, Vec3& ax1, i32 rotational, i32 rotAllowed);
	i32 get_limit_motor_info2(RotationalLimitMotor2 * limot,
							  const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB,
							  ConstraintInfo2* info, i32 row, Vec3& ax1, i32 rotational, i32 rotAllowed = false);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Generic6DofSpring2Constraint(RigidBody & rbA, RigidBody & rbB, const Transform2& frameInA, const Transform2& frameInB, RotateOrder rotOrder = RO_XYZ);
	Generic6DofSpring2Constraint(RigidBody & rbB, const Transform2& frameInB, RotateOrder rotOrder = RO_XYZ);

	virtual void buildJacobian() {}
	virtual void getInfo1(ConstraintInfo1 * info);
	virtual void getInfo2(ConstraintInfo2 * info);
	virtual i32 calculateSerializeBufferSize() const;
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	RotationalLimitMotor2* getRotationalLimitMotor(i32 index) { return &m_angularLimits[index]; }
	TranslationalLimitMotor2* getTranslationalLimitMotor() { return &m_linearLimits; }

	// Calculates the global transform for the joint offset for body A an B, and also calculates the angle differences between the bodies.
	void calculateTransforms(const Transform2& transA, const Transform2& transB);
	void calculateTransforms();

	// Gets the global transform of the offset for body A
	const Transform2& getCalculatedTransform2A() const { return m_calculatedTransformA; }
	// Gets the global transform of the offset for body B
	const Transform2& getCalculatedTransform2B() const { return m_calculatedTransformB; }

	const Transform2& getFrameOffsetA() const { return m_frameInA; }
	const Transform2& getFrameOffsetB() const { return m_frameInB; }

	Transform2& getFrameOffsetA() { return m_frameInA; }
	Transform2& getFrameOffsetB() { return m_frameInB; }

	// Get the rotation axis in global coordinates ( Generic6DofSpring2Constraint::calculateTransforms() must be called previously )
	Vec3 getAxis(i32 axis_index) const { return m_calculatedAxis[axis_index]; }

	// Get the relative Euler angle ( Generic6DofSpring2Constraint::calculateTransforms() must be called previously )
	Scalar getAngle(i32 axis_index) const { return m_calculatedAxisAngleDiff[axis_index]; }

	// Get the relative position of the constraint pivot ( Generic6DofSpring2Constraint::calculateTransforms() must be called previously )
	Scalar getRelativePivotPosition(i32 axis_index) const { return m_calculatedLinearDiff[axis_index]; }

	void setFrames(const Transform2& frameA, const Transform2& frameB);

	void setLinearLowerLimit(const Vec3& linearLower) { m_linearLimits.m_lowerLimit = linearLower; }
	void getLinearLowerLimit(Vec3 & linearLower) { linearLower = m_linearLimits.m_lowerLimit; }
	void setLinearUpperLimit(const Vec3& linearUpper) { m_linearLimits.m_upperLimit = linearUpper; }
	void getLinearUpperLimit(Vec3 & linearUpper) { linearUpper = m_linearLimits.m_upperLimit; }

	void setAngularLowerLimit(const Vec3& angularLower)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_loLimit = NormalizeAngle(angularLower[i]);
	}

	void setAngularLowerLimitReversed(const Vec3& angularLower)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_hiLimit = NormalizeAngle(-angularLower[i]);
	}

	void getAngularLowerLimit(Vec3 & angularLower)
	{
		for (i32 i = 0; i < 3; i++)
			angularLower[i] = m_angularLimits[i].m_loLimit;
	}

	void getAngularLowerLimitReversed(Vec3 & angularLower)
	{
		for (i32 i = 0; i < 3; i++)
			angularLower[i] = -m_angularLimits[i].m_hiLimit;
	}

	void setAngularUpperLimit(const Vec3& angularUpper)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_hiLimit = NormalizeAngle(angularUpper[i]);
	}

	void setAngularUpperLimitReversed(const Vec3& angularUpper)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_loLimit = NormalizeAngle(-angularUpper[i]);
	}

	void getAngularUpperLimit(Vec3 & angularUpper)
	{
		for (i32 i = 0; i < 3; i++)
			angularUpper[i] = m_angularLimits[i].m_hiLimit;
	}

	void getAngularUpperLimitReversed(Vec3 & angularUpper)
	{
		for (i32 i = 0; i < 3; i++)
			angularUpper[i] = -m_angularLimits[i].m_loLimit;
	}

	//first 3 are linear, next 3 are angular

	void setLimit(i32 axis, Scalar lo, Scalar hi)
	{
		if (axis < 3)
		{
			m_linearLimits.m_lowerLimit[axis] = lo;
			m_linearLimits.m_upperLimit[axis] = hi;
		}
		else
		{
			lo = NormalizeAngle(lo);
			hi = NormalizeAngle(hi);
			m_angularLimits[axis - 3].m_loLimit = lo;
			m_angularLimits[axis - 3].m_hiLimit = hi;
		}
	}

	void setLimitReversed(i32 axis, Scalar lo, Scalar hi)
	{
		if (axis < 3)
		{
			m_linearLimits.m_lowerLimit[axis] = lo;
			m_linearLimits.m_upperLimit[axis] = hi;
		}
		else
		{
			lo = NormalizeAngle(lo);
			hi = NormalizeAngle(hi);
			m_angularLimits[axis - 3].m_hiLimit = -lo;
			m_angularLimits[axis - 3].m_loLimit = -hi;
		}
	}

	bool isLimited(i32 limitIndex)
	{
		if (limitIndex < 3)
		{
			return m_linearLimits.isLimited(limitIndex);
		}
		return m_angularLimits[limitIndex - 3].isLimited();
	}

	void setRotationOrder(RotateOrder order) { m_rotateOrder = order; }
	RotateOrder getRotationOrder() { return m_rotateOrder; }

	void setAxis(const Vec3& axis1, const Vec3& axis2);

	void setBounce(i32 index, Scalar bounce);

	void enableMotor(i32 index, bool onOff);
	void setServo(i32 index, bool onOff);  // set the type of the motor (servo or not) (the motor has to be turned on for servo also)
	void setTargetVelocity(i32 index, Scalar velocity);
	void setServoTarget(i32 index, Scalar target);
	void setMaxMotorForce(i32 index, Scalar force);

	void enableSpring(i32 index, bool onOff);
	void setStiffness(i32 index, Scalar stiffness, bool limitIfNeeded = true);  // if limitIfNeeded is true the system will automatically limit the stiffness in necessary situations where otherwise the spring would move unrealistically too widely
	void setDamping(i32 index, Scalar damping, bool limitIfNeeded = true);      // if limitIfNeeded is true the system will automatically limit the damping in necessary situations where otherwise the spring would blow up
	void setEquilibriumPoint();                                                   // set the current constraint position/orientation as an equilibrium point for all DOF
	void setEquilibriumPoint(i32 index);                                          // set the current constraint position/orientation as an equilibrium point for given DOF
	void setEquilibriumPoint(i32 index, Scalar val);

	//override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	//If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1);
	virtual Scalar getParam(i32 num, i32 axis = -1) const;

	static Scalar GetMatrixElem(const Matrix3x3& mat, i32 index);
	static bool matrixToEulerXYZ(const Matrix3x3& mat, Vec3& xyz);
	static bool matrixToEulerXZY(const Matrix3x3& mat, Vec3& xyz);
	static bool matrixToEulerYXZ(const Matrix3x3& mat, Vec3& xyz);
	static bool matrixToEulerYZX(const Matrix3x3& mat, Vec3& xyz);
	static bool matrixToEulerZXY(const Matrix3x3& mat, Vec3& xyz);
	static bool matrixToEulerZYX(const Matrix3x3& mat, Vec3& xyz);
};

struct Generic6DofSpring2ConstraintData
{
	TypedConstraintData m_typeConstraintData;
	Transform2FloatData m_rbAFrame;
	Transform2FloatData m_rbBFrame;

	Vec3FloatData m_linearUpperLimit;
	Vec3FloatData m_linearLowerLimit;
	Vec3FloatData m_linearBounce;
	Vec3FloatData m_linearStopERP;
	Vec3FloatData m_linearStopCFM;
	Vec3FloatData m_linearMotorERP;
	Vec3FloatData m_linearMotorCFM;
	Vec3FloatData m_linearTargetVelocity;
	Vec3FloatData m_linearMaxMotorForce;
	Vec3FloatData m_linearServoTarget;
	Vec3FloatData m_linearSpringStiffness;
	Vec3FloatData m_linearSpringDamping;
	Vec3FloatData m_linearEquilibriumPoint;
	char m_linearEnableMotor[4];
	char m_linearServoMotor[4];
	char m_linearEnableSpring[4];
	char m_linearSpringStiffnessLimited[4];
	char m_linearSpringDampingLimited[4];
	char m_padding1[4];

	Vec3FloatData m_angularUpperLimit;
	Vec3FloatData m_angularLowerLimit;
	Vec3FloatData m_angularBounce;
	Vec3FloatData m_angularStopERP;
	Vec3FloatData m_angularStopCFM;
	Vec3FloatData m_angularMotorERP;
	Vec3FloatData m_angularMotorCFM;
	Vec3FloatData m_angularTargetVelocity;
	Vec3FloatData m_angularMaxMotorForce;
	Vec3FloatData m_angularServoTarget;
	Vec3FloatData m_angularSpringStiffness;
	Vec3FloatData m_angularSpringDamping;
	Vec3FloatData m_angularEquilibriumPoint;
	char m_angularEnableMotor[4];
	char m_angularServoMotor[4];
	char m_angularEnableSpring[4];
	char m_angularSpringStiffnessLimited[4];
	char m_angularSpringDampingLimited[4];

	i32 m_rotateOrder;
};

struct Generic6DofSpring2ConstraintDoubleData2
{
	TypedConstraintDoubleData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;
	Transform2DoubleData m_rbBFrame;

	Vec3DoubleData m_linearUpperLimit;
	Vec3DoubleData m_linearLowerLimit;
	Vec3DoubleData m_linearBounce;
	Vec3DoubleData m_linearStopERP;
	Vec3DoubleData m_linearStopCFM;
	Vec3DoubleData m_linearMotorERP;
	Vec3DoubleData m_linearMotorCFM;
	Vec3DoubleData m_linearTargetVelocity;
	Vec3DoubleData m_linearMaxMotorForce;
	Vec3DoubleData m_linearServoTarget;
	Vec3DoubleData m_linearSpringStiffness;
	Vec3DoubleData m_linearSpringDamping;
	Vec3DoubleData m_linearEquilibriumPoint;
	char m_linearEnableMotor[4];
	char m_linearServoMotor[4];
	char m_linearEnableSpring[4];
	char m_linearSpringStiffnessLimited[4];
	char m_linearSpringDampingLimited[4];
	char m_padding1[4];

	Vec3DoubleData m_angularUpperLimit;
	Vec3DoubleData m_angularLowerLimit;
	Vec3DoubleData m_angularBounce;
	Vec3DoubleData m_angularStopERP;
	Vec3DoubleData m_angularStopCFM;
	Vec3DoubleData m_angularMotorERP;
	Vec3DoubleData m_angularMotorCFM;
	Vec3DoubleData m_angularTargetVelocity;
	Vec3DoubleData m_angularMaxMotorForce;
	Vec3DoubleData m_angularServoTarget;
	Vec3DoubleData m_angularSpringStiffness;
	Vec3DoubleData m_angularSpringDamping;
	Vec3DoubleData m_angularEquilibriumPoint;
	char m_angularEnableMotor[4];
	char m_angularServoMotor[4];
	char m_angularEnableSpring[4];
	char m_angularSpringStiffnessLimited[4];
	char m_angularSpringDampingLimited[4];

	i32 m_rotateOrder;
};

SIMD_FORCE_INLINE i32 Generic6DofSpring2Constraint::calculateSerializeBufferSize() const
{
	return sizeof(Generic6DofSpring2ConstraintData2);
}

SIMD_FORCE_INLINE tukk Generic6DofSpring2Constraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	Generic6DofSpring2ConstraintData2* dof = (Generic6DofSpring2ConstraintData2*)dataBuffer;
	TypedConstraint::serialize(&dof->m_typeConstraintData, serializer);

	m_frameInA.serialize(dof->m_rbAFrame);
	m_frameInB.serialize(dof->m_rbBFrame);

	i32 i;
	for (i = 0; i < 3; i++)
	{
		dof->m_angularLowerLimit.m_floats[i] = m_angularLimits[i].m_loLimit;
		dof->m_angularUpperLimit.m_floats[i] = m_angularLimits[i].m_hiLimit;
		dof->m_angularBounce.m_floats[i] = m_angularLimits[i].m_bounce;
		dof->m_angularStopERP.m_floats[i] = m_angularLimits[i].m_stopERP;
		dof->m_angularStopCFM.m_floats[i] = m_angularLimits[i].m_stopCFM;
		dof->m_angularMotorERP.m_floats[i] = m_angularLimits[i].m_motorERP;
		dof->m_angularMotorCFM.m_floats[i] = m_angularLimits[i].m_motorCFM;
		dof->m_angularTargetVelocity.m_floats[i] = m_angularLimits[i].m_targetVelocity;
		dof->m_angularMaxMotorForce.m_floats[i] = m_angularLimits[i].m_maxMotorForce;
		dof->m_angularServoTarget.m_floats[i] = m_angularLimits[i].m_servoTarget;
		dof->m_angularSpringStiffness.m_floats[i] = m_angularLimits[i].m_springStiffness;
		dof->m_angularSpringDamping.m_floats[i] = m_angularLimits[i].m_springDamping;
		dof->m_angularEquilibriumPoint.m_floats[i] = m_angularLimits[i].m_equilibriumPoint;
	}
	dof->m_angularLowerLimit.m_floats[3] = 0;
	dof->m_angularUpperLimit.m_floats[3] = 0;
	dof->m_angularBounce.m_floats[3] = 0;
	dof->m_angularStopERP.m_floats[3] = 0;
	dof->m_angularStopCFM.m_floats[3] = 0;
	dof->m_angularMotorERP.m_floats[3] = 0;
	dof->m_angularMotorCFM.m_floats[3] = 0;
	dof->m_angularTargetVelocity.m_floats[3] = 0;
	dof->m_angularMaxMotorForce.m_floats[3] = 0;
	dof->m_angularServoTarget.m_floats[3] = 0;
	dof->m_angularSpringStiffness.m_floats[3] = 0;
	dof->m_angularSpringDamping.m_floats[3] = 0;
	dof->m_angularEquilibriumPoint.m_floats[3] = 0;
	for (i = 0; i < 4; i++)
	{
		dof->m_angularEnableMotor[i] = i < 3 ? (m_angularLimits[i].m_enableMotor ? 1 : 0) : 0;
		dof->m_angularServoMotor[i] = i < 3 ? (m_angularLimits[i].m_servoMotor ? 1 : 0) : 0;
		dof->m_angularEnableSpring[i] = i < 3 ? (m_angularLimits[i].m_enableSpring ? 1 : 0) : 0;
		dof->m_angularSpringStiffnessLimited[i] = i < 3 ? (m_angularLimits[i].m_springStiffnessLimited ? 1 : 0) : 0;
		dof->m_angularSpringDampingLimited[i] = i < 3 ? (m_angularLimits[i].m_springDampingLimited ? 1 : 0) : 0;
	}

	m_linearLimits.m_lowerLimit.serialize(dof->m_linearLowerLimit);
	m_linearLimits.m_upperLimit.serialize(dof->m_linearUpperLimit);
	m_linearLimits.m_bounce.serialize(dof->m_linearBounce);
	m_linearLimits.m_stopERP.serialize(dof->m_linearStopERP);
	m_linearLimits.m_stopCFM.serialize(dof->m_linearStopCFM);
	m_linearLimits.m_motorERP.serialize(dof->m_linearMotorERP);
	m_linearLimits.m_motorCFM.serialize(dof->m_linearMotorCFM);
	m_linearLimits.m_targetVelocity.serialize(dof->m_linearTargetVelocity);
	m_linearLimits.m_maxMotorForce.serialize(dof->m_linearMaxMotorForce);
	m_linearLimits.m_servoTarget.serialize(dof->m_linearServoTarget);
	m_linearLimits.m_springStiffness.serialize(dof->m_linearSpringStiffness);
	m_linearLimits.m_springDamping.serialize(dof->m_linearSpringDamping);
	m_linearLimits.m_equilibriumPoint.serialize(dof->m_linearEquilibriumPoint);
	for (i = 0; i < 4; i++)
	{
		dof->m_linearEnableMotor[i] = i < 3 ? (m_linearLimits.m_enableMotor[i] ? 1 : 0) : 0;
		dof->m_linearServoMotor[i] = i < 3 ? (m_linearLimits.m_servoMotor[i] ? 1 : 0) : 0;
		dof->m_linearEnableSpring[i] = i < 3 ? (m_linearLimits.m_enableSpring[i] ? 1 : 0) : 0;
		dof->m_linearSpringStiffnessLimited[i] = i < 3 ? (m_linearLimits.m_springStiffnessLimited[i] ? 1 : 0) : 0;
		dof->m_linearSpringDampingLimited[i] = i < 3 ? (m_linearLimits.m_springDampingLimited[i] ? 1 : 0) : 0;
	}

	dof->m_rotateOrder = m_rotateOrder;

	dof->m_padding1[0] = 0;
	dof->m_padding1[1] = 0;
	dof->m_padding1[2] = 0;
	dof->m_padding1[3] = 0;

	return Generic6DofSpring2ConstraintDataName;
}

#endif  //DRX3D_GENERIC_6DOF_CONSTRAINT_H
