
#ifndef DRX3D_HINGECONSTRAINT_H
#define DRX3D_HINGECONSTRAINT_H

#define _DRX3D_USE_CENTER_LIMIT_ 1

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

class RigidBody;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define HingeConstraintData HingeConstraintDoubleData2  //rename to 2 for backwards compatibility, so we can still load the 'HingeConstraintDoubleData' version
#define HingeConstraintDataName "HingeConstraintDoubleData2"
#else
#define HingeConstraintData HingeConstraintFloatData
#define HingeConstraintDataName "HingeConstraintFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

enum HingeFlags
{
	DRX3D_HINGE_FLAGS_CFM_STOP = 1,
	DRX3D_HINGE_FLAGS_ERP_STOP = 2,
	DRX3D_HINGE_FLAGS_CFM_NORM = 4,
	DRX3D_HINGE_FLAGS_ERP_NORM = 8
};

/// hinge constraint between two rigidbodies each with a pivotpoint that descibes the axis location in local space
/// axis defines the orientation of the hinge axis
ATTRIBUTE_ALIGNED16(class)
HingeConstraint : public TypedConstraint
{
#ifdef IN_PARALLELL_SOLVER
public:
#endif
	JacobianEntry m_jac[3];     //3 orthogonal linear constraints
	JacobianEntry m_jacAng[3];  //2 orthogonal angular constraints+ 1 for limit/motor

	Transform2 m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2 m_rbBFrame;

	Scalar m_motorTargetVelocity;
	Scalar m_maxMotorImpulse;

#ifdef _DRX3D_USE_CENTER_LIMIT_
	AngularLimit m_limit;
#else
	Scalar m_lowerLimit;
	Scalar m_upperLimit;
	Scalar m_limitSign;
	Scalar m_correction;

	Scalar m_limitSoftness;
	Scalar m_biasFactor;
	Scalar m_relaxationFactor;

	bool m_solveLimit;
#endif

	Scalar m_kHinge;

	Scalar m_accLimitImpulse;
	Scalar m_hingeAngle;
	Scalar m_referenceSign;

	bool m_angularOnly;
	bool m_enableAngularMotor;
	bool m_useSolveConstraintObsolete;
	bool m_useOffsetForConstraintFrame;
	bool m_useReferenceFrameA;

	Scalar m_accMotorImpulse;

	i32 m_flags;
	Scalar m_normalCFM;
	Scalar m_normalERP;
	Scalar m_stopCFM;
	Scalar m_stopERP;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	HingeConstraint(RigidBody & rbA, RigidBody & rbB, const Vec3& pivotInA, const Vec3& pivotInB, const Vec3& axisInA, const Vec3& axisInB, bool useReferenceFrameA = false);

	HingeConstraint(RigidBody & rbA, const Vec3& pivotInA, const Vec3& axisInA, bool useReferenceFrameA = false);

	HingeConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& rbAFrame, const Transform2& rbBFrame, bool useReferenceFrameA = false);

	HingeConstraint(RigidBody & rbA, const Transform2& rbAFrame, bool useReferenceFrameA = false);

	virtual void buildJacobian();

	virtual void getInfo1(ConstraintInfo1 * info);

	void getInfo1NonVirtual(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	void getInfo2NonVirtual(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB);

	void getInfo2Internal(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB);
	void getInfo2InternalUsingFrameOffset(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB);

	void updateRHS(Scalar timeStep);

	const RigidBody& getRigidBodyA() const
	{
		return m_rbA;
	}
	const RigidBody& getRigidBodyB() const
	{
		return m_rbB;
	}

	RigidBody& getRigidBodyA()
	{
		return m_rbA;
	}

	RigidBody& getRigidBodyB()
	{
		return m_rbB;
	}

	Transform2& getFrameOffsetA()
	{
		return m_rbAFrame;
	}

	Transform2& getFrameOffsetB()
	{
		return m_rbBFrame;
	}

	void setFrames(const Transform2& frameA, const Transform2& frameB);

	void setAngularOnly(bool angularOnly)
	{
		m_angularOnly = angularOnly;
	}

	void enableAngularMotor(bool enableMotor, Scalar targetVelocity, Scalar maxMotorImpulse)
	{
		m_enableAngularMotor = enableMotor;
		m_motorTargetVelocity = targetVelocity;
		m_maxMotorImpulse = maxMotorImpulse;
	}

	// extra motor API, including ability to set a target rotation (as opposed to angular velocity)
	// note: setMotorTarget sets angular velocity under the hood, so you must call it every tick to
	//       maintain a given angular target.
	void enableMotor(bool enableMotor) { m_enableAngularMotor = enableMotor; }
	void setMaxMotorImpulse(Scalar maxMotorImpulse) { m_maxMotorImpulse = maxMotorImpulse; }
	void setMotorTargetVelocity(Scalar motorTargetVelocity) { m_motorTargetVelocity = motorTargetVelocity; }
	void setMotorTarget(const Quat& qAinB, Scalar dt);  // qAinB is rotation of body A wrt body B.
	void setMotorTarget(Scalar targetAngle, Scalar dt);

	void setLimit(Scalar low, Scalar high, Scalar _softness = 0.9f, Scalar _biasFactor = 0.3f, Scalar _relaxationFactor = 1.0f)
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		m_limit.set(low, high, _softness, _biasFactor, _relaxationFactor);
#else
		m_lowerLimit = NormalizeAngle(low);
		m_upperLimit = NormalizeAngle(high);
		m_limitSoftness = _softness;
		m_biasFactor = _biasFactor;
		m_relaxationFactor = _relaxationFactor;
#endif
	}

	Scalar getLimitSoftness() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getSoftness();
#else
		return m_limitSoftness;
#endif
	}

	Scalar getLimitBiasFactor() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getBiasFactor();
#else
		return m_biasFactor;
#endif
	}

	Scalar getLimitRelaxationFactor() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getRelaxationFactor();
#else
		return m_relaxationFactor;
#endif
	}

	void setAxis(Vec3 & axisInA)
	{
		Vec3 rbAxisA1, rbAxisA2;
		PlaneSpace1(axisInA, rbAxisA1, rbAxisA2);
		Vec3 pivotInA = m_rbAFrame.getOrigin();
		//		m_rbAFrame.getOrigin() = pivotInA;
		m_rbAFrame.getBasis().setVal(rbAxisA1.getX(), rbAxisA2.getX(), axisInA.getX(),
									   rbAxisA1.getY(), rbAxisA2.getY(), axisInA.getY(),
									   rbAxisA1.getZ(), rbAxisA2.getZ(), axisInA.getZ());

		Vec3 axisInB = m_rbA.getCenterOfMassTransform().getBasis() * axisInA;

		Quat rotationArc = shortestArcQuat(axisInA, axisInB);
		Vec3 rbAxisB1 = quatRotate(rotationArc, rbAxisA1);
		Vec3 rbAxisB2 = axisInB.cross(rbAxisB1);

		m_rbBFrame.getOrigin() = m_rbB.getCenterOfMassTransform().inverse()(m_rbA.getCenterOfMassTransform()(pivotInA));

		m_rbBFrame.getBasis().setVal(rbAxisB1.getX(), rbAxisB2.getX(), axisInB.getX(),
									   rbAxisB1.getY(), rbAxisB2.getY(), axisInB.getY(),
									   rbAxisB1.getZ(), rbAxisB2.getZ(), axisInB.getZ());
		m_rbBFrame.getBasis() = m_rbB.getCenterOfMassTransform().getBasis().inverse() * m_rbBFrame.getBasis();
	}

	bool hasLimit() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getHalfRange() > 0;
#else
		return m_lowerLimit <= m_upperLimit;
#endif
	}

	Scalar getLowerLimit() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getLow();
#else
		return m_lowerLimit;
#endif
	}

	Scalar getUpperLimit() const
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getHigh();
#else
		return m_upperLimit;
#endif
	}

	///The getHingeAngle gives the hinge angle in range [-PI,PI]
	Scalar getHingeAngle();

	Scalar getHingeAngle(const Transform2& transA, const Transform2& transB);

	void testLimit(const Transform2& transA, const Transform2& transB);

	const Transform2& getAFrame() const { return m_rbAFrame; };
	const Transform2& getBFrame() const { return m_rbBFrame; };

	Transform2& getAFrame() { return m_rbAFrame; };
	Transform2& getBFrame() { return m_rbBFrame; };

	inline i32 getSolveLimit()
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.isLimit();
#else
		return m_solveLimit;
#endif
	}

	inline Scalar getLimitSign()
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		return m_limit.getSign();
#else
		return m_limitSign;
#endif
	}

	inline bool getAngularOnly()
	{
		return m_angularOnly;
	}
	inline bool getEnableAngularMotor()
	{
		return m_enableAngularMotor;
	}
	inline Scalar getMotorTargetVelocity()
	{
		return m_motorTargetVelocity;
	}
	inline Scalar getMaxMotorImpulse()
	{
		return m_maxMotorImpulse;
	}
	// access for UseFrameOffset
	bool getUseFrameOffset() { return m_useOffsetForConstraintFrame; }
	void setUseFrameOffset(bool frameOffsetOnOff) { m_useOffsetForConstraintFrame = frameOffsetOnOff; }
	// access for UseReferenceFrameA
	bool getUseReferenceFrameA() const { return m_useReferenceFrameA; }
	void setUseReferenceFrameA(bool useReferenceFrameA) { m_useReferenceFrameA = useReferenceFrameA; }

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1);
	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const;

	virtual i32 getFlags() const
	{
		return m_flags;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

//only for backward compatibility
#ifdef DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
///this structure is not used, except for loading pre-2.82 .bullet files
struct HingeConstraintDoubleData
{
	TypedConstraintData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2DoubleData m_rbBFrame;
	i32 m_useReferenceFrameA;
	i32 m_angularOnly;
	i32 m_enableAngularMotor;
	float m_motorTargetVelocity;
	float m_maxMotorImpulse;

	float m_lowerLimit;
	float m_upperLimit;
	float m_limitSoftness;
	float m_biasFactor;
	float m_relaxationFactor;
};
#endif  //DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION

///The getAccumulatedHingeAngle returns the accumulated hinge angle, taking rotation across the -PI/PI boundary into account
ATTRIBUTE_ALIGNED16(class)
HingeAccumulatedAngleConstraint : public HingeConstraint
{
protected:
	Scalar m_accumulatedAngle;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	HingeAccumulatedAngleConstraint(RigidBody & rbA, RigidBody & rbB, const Vec3& pivotInA, const Vec3& pivotInB, const Vec3& axisInA, const Vec3& axisInB, bool useReferenceFrameA = false)
		: HingeConstraint(rbA, rbB, pivotInA, pivotInB, axisInA, axisInB, useReferenceFrameA)
	{
		m_accumulatedAngle = getHingeAngle();
	}

	HingeAccumulatedAngleConstraint(RigidBody & rbA, const Vec3& pivotInA, const Vec3& axisInA, bool useReferenceFrameA = false)
		: HingeConstraint(rbA, pivotInA, axisInA, useReferenceFrameA)
	{
		m_accumulatedAngle = getHingeAngle();
	}

	HingeAccumulatedAngleConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& rbAFrame, const Transform2& rbBFrame, bool useReferenceFrameA = false)
		: HingeConstraint(rbA, rbB, rbAFrame, rbBFrame, useReferenceFrameA)
	{
		m_accumulatedAngle = getHingeAngle();
	}

	HingeAccumulatedAngleConstraint(RigidBody & rbA, const Transform2& rbAFrame, bool useReferenceFrameA = false)
		: HingeConstraint(rbA, rbAFrame, useReferenceFrameA)
	{
		m_accumulatedAngle = getHingeAngle();
	}
	Scalar getAccumulatedHingeAngle();
	void setAccumulatedHingeAngle(Scalar accAngle);
	virtual void getInfo1(ConstraintInfo1 * info);
};

struct HingeConstraintFloatData
{
	TypedConstraintData m_typeConstraintData;
	Transform2FloatData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2FloatData m_rbBFrame;
	i32 m_useReferenceFrameA;
	i32 m_angularOnly;

	i32 m_enableAngularMotor;
	float m_motorTargetVelocity;
	float m_maxMotorImpulse;

	float m_lowerLimit;
	float m_upperLimit;
	float m_limitSoftness;
	float m_biasFactor;
	float m_relaxationFactor;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct HingeConstraintDoubleData2
{
	TypedConstraintDoubleData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2DoubleData m_rbBFrame;
	i32 m_useReferenceFrameA;
	i32 m_angularOnly;
	i32 m_enableAngularMotor;
	double m_motorTargetVelocity;
	double m_maxMotorImpulse;

	double m_lowerLimit;
	double m_upperLimit;
	double m_limitSoftness;
	double m_biasFactor;
	double m_relaxationFactor;
	char m_padding1[4];
};

SIMD_FORCE_INLINE i32 HingeConstraint::calculateSerializeBufferSize() const
{
	return sizeof(HingeConstraintData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk HingeConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	HingeConstraintData* hingeData = (HingeConstraintData*)dataBuffer;
	TypedConstraint::serialize(&hingeData->m_typeConstraintData, serializer);

	m_rbAFrame.serialize(hingeData->m_rbAFrame);
	m_rbBFrame.serialize(hingeData->m_rbBFrame);

	hingeData->m_angularOnly = m_angularOnly;
	hingeData->m_enableAngularMotor = m_enableAngularMotor;
	hingeData->m_maxMotorImpulse = float(m_maxMotorImpulse);
	hingeData->m_motorTargetVelocity = float(m_motorTargetVelocity);
	hingeData->m_useReferenceFrameA = m_useReferenceFrameA;
#ifdef _DRX3D_USE_CENTER_LIMIT_
	hingeData->m_lowerLimit = float(m_limit.getLow());
	hingeData->m_upperLimit = float(m_limit.getHigh());
	hingeData->m_limitSoftness = float(m_limit.getSoftness());
	hingeData->m_biasFactor = float(m_limit.getBiasFactor());
	hingeData->m_relaxationFactor = float(m_limit.getRelaxationFactor());
#else
	hingeData->m_lowerLimit = float(m_lowerLimit);
	hingeData->m_upperLimit = float(m_upperLimit);
	hingeData->m_limitSoftness = float(m_limitSoftness);
	hingeData->m_biasFactor = float(m_biasFactor);
	hingeData->m_relaxationFactor = float(m_relaxationFactor);
#endif

	// Fill padding with zeros to appease msan.
#ifdef DRX3D_USE_DOUBLE_PRECISION
	hingeData->m_padding1[0] = 0;
	hingeData->m_padding1[1] = 0;
	hingeData->m_padding1[2] = 0;
	hingeData->m_padding1[3] = 0;
#endif

	return HingeConstraintDataName;
}

#endif  //DRX3D_HINGECONSTRAINT_H
