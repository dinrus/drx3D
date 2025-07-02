#ifndef DRX3D_GENERIC_6DOF_CONSTRAINT_H
#define DRX3D_GENERIC_6DOF_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

class RigidBody;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define Generic6DofConstraintData2 Generic6DofConstraintDoubleData2
#define Generic6DofConstraintDataName "Generic6DofConstraintDoubleData2"
#else
#define Generic6DofConstraintData2 Generic6DofConstraintData
#define Generic6DofConstraintDataName "Generic6DofConstraintData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

//! Rotation Limit structure for generic joints
class RotationalLimitMotor
{
public:
	//! limit_parameters
	//!@{
	Scalar m_loLimit;         //!< joint limit
	Scalar m_hiLimit;         //!< joint limit
	Scalar m_targetVelocity;  //!< target motor velocity
	Scalar m_maxMotorForce;   //!< max force on motor
	Scalar m_maxLimitForce;   //!< max force on limit
	Scalar m_damping;         //!< Damping.
	Scalar m_limitSoftness;   //! Relaxation factor
	Scalar m_normalCFM;       //!< Constraint force mixing factor
	Scalar m_stopERP;         //!< Error tolerance factor when joint is at limit
	Scalar m_stopCFM;         //!< Constraint force mixing factor when joint is at limit
	Scalar m_bounce;          //!< restitution factor
	bool m_enableMotor;

	//!@}

	//! temp_variables
	//!@{
	Scalar m_currentLimitError;  //!  How much is violated this limit
	Scalar m_currentPosition;    //!  current value of angle
	i32 m_currentLimit;            //!< 0=free, 1=at lo limit, 2=at hi limit
	Scalar m_accumulatedImpulse;
	//!@}

	RotationalLimitMotor()
	{
		m_accumulatedImpulse = 0.f;
		m_targetVelocity = 0;
		m_maxMotorForce = 6.0f;
		m_maxLimitForce = 300.0f;
		m_loLimit = 1.0f;
		m_hiLimit = -1.0f;
		m_normalCFM = 0.f;
		m_stopERP = 0.2f;
		m_stopCFM = 0.f;
		m_bounce = 0.0f;
		m_damping = 1.0f;
		m_limitSoftness = 0.5f;
		m_currentLimit = 0;
		m_currentLimitError = 0;
		m_enableMotor = false;
	}

	RotationalLimitMotor(const RotationalLimitMotor& limot)
	{
		m_targetVelocity = limot.m_targetVelocity;
		m_maxMotorForce = limot.m_maxMotorForce;
		m_limitSoftness = limot.m_limitSoftness;
		m_loLimit = limot.m_loLimit;
		m_hiLimit = limot.m_hiLimit;
		m_normalCFM = limot.m_normalCFM;
		m_stopERP = limot.m_stopERP;
		m_stopCFM = limot.m_stopCFM;
		m_bounce = limot.m_bounce;
		m_currentLimit = limot.m_currentLimit;
		m_currentLimitError = limot.m_currentLimitError;
		m_enableMotor = limot.m_enableMotor;
	}

	//! Is limited
	bool isLimited() const
	{
		if (m_loLimit > m_hiLimit) return false;
		return true;
	}

	//! Need apply correction
	bool needApplyTorques() const
	{
		if (m_currentLimit == 0 && m_enableMotor == false) return false;
		return true;
	}

	//! calculates  error
	/*!
	calculates m_currentLimit and m_currentLimitError.
	*/
	i32 testLimitValue(Scalar test_value);

	//! apply the correction impulses for two bodies
	Scalar solveAngularLimits(Scalar timeStep, Vec3& axis, Scalar jacDiagABInv, RigidBody* body0, RigidBody* body1);
};

class TranslationalLimitMotor
{
public:
	Vec3 m_lowerLimit;  //!< the constraint lower limits
	Vec3 m_upperLimit;  //!< the constraint upper limits
	Vec3 m_accumulatedImpulse;
	//! Linear_Limit_parameters
	//!@{
	Scalar m_limitSoftness;  //!< Softness for linear limit
	Scalar m_damping;        //!< Damping for linear limit
	Scalar m_restitution;    //! Bounce parameter for linear limit
	Vec3 m_normalCFM;     //!< Constraint force mixing factor
	Vec3 m_stopERP;       //!< Error tolerance factor when joint is at limit
	Vec3 m_stopCFM;       //!< Constraint force mixing factor when joint is at limit
							   //!@}
	bool m_enableMotor[3];
	Vec3 m_targetVelocity;     //!< target motor velocity
	Vec3 m_maxMotorForce;      //!< max force on motor
	Vec3 m_currentLimitError;  //!  How much is violated this limit
	Vec3 m_currentLinearDiff;  //!  Current relative offset of constraint frames
	i32 m_currentLimit[3];          //!< 0=free, 1=at lower limit, 2=at upper limit

	TranslationalLimitMotor()
	{
		m_lowerLimit.setVal(0.f, 0.f, 0.f);
		m_upperLimit.setVal(0.f, 0.f, 0.f);
		m_accumulatedImpulse.setVal(0.f, 0.f, 0.f);
		m_normalCFM.setVal(0.f, 0.f, 0.f);
		m_stopERP.setVal(0.2f, 0.2f, 0.2f);
		m_stopCFM.setVal(0.f, 0.f, 0.f);

		m_limitSoftness = 0.7f;
		m_damping = Scalar(1.0f);
		m_restitution = Scalar(0.5f);
		for (i32 i = 0; i < 3; i++)
		{
			m_enableMotor[i] = false;
			m_targetVelocity[i] = Scalar(0.f);
			m_maxMotorForce[i] = Scalar(0.f);
		}
	}

	TranslationalLimitMotor(const TranslationalLimitMotor& other)
	{
		m_lowerLimit = other.m_lowerLimit;
		m_upperLimit = other.m_upperLimit;
		m_accumulatedImpulse = other.m_accumulatedImpulse;

		m_limitSoftness = other.m_limitSoftness;
		m_damping = other.m_damping;
		m_restitution = other.m_restitution;
		m_normalCFM = other.m_normalCFM;
		m_stopERP = other.m_stopERP;
		m_stopCFM = other.m_stopCFM;

		for (i32 i = 0; i < 3; i++)
		{
			m_enableMotor[i] = other.m_enableMotor[i];
			m_targetVelocity[i] = other.m_targetVelocity[i];
			m_maxMotorForce[i] = other.m_maxMotorForce[i];
		}
	}

	//! Test limit
	/*!
    - free means upper < lower,
    - locked means upper == lower
    - limited means upper > lower
    - limitIndex: first 3 are linear, next 3 are angular
    */
	inline bool isLimited(i32 limitIndex) const
	{
		return (m_upperLimit[limitIndex] >= m_lowerLimit[limitIndex]);
	}
	inline bool needApplyForce(i32 limitIndex) const
	{
		if (m_currentLimit[limitIndex] == 0 && m_enableMotor[limitIndex] == false) return false;
		return true;
	}
	i32 testLimitValue(i32 limitIndex, Scalar test_value);

	Scalar solveLinearAxis(
		Scalar timeStep,
		Scalar jacDiagABInv,
		RigidBody& body1, const Vec3& pointInA,
		RigidBody& body2, const Vec3& pointInB,
		i32 limit_index,
		const Vec3& axis_normal_on_a,
		const Vec3& anchorPos);
};

enum bt6DofFlags
{
	DRX3D_6DOF_FLAGS_CFM_NORM = 1,
	DRX3D_6DOF_FLAGS_CFM_STOP = 2,
	DRX3D_6DOF_FLAGS_ERP_STOP = 4
};
#define DRX3D_6DOF_FLAGS_AXIS_SHIFT 3  // bits per axis

/// Generic6DofConstraint between two rigidbodies each with a pivotpoint that descibes the axis location in local space
/*!
Generic6DofConstraint can leave any of the 6 degree of freedom 'free' or 'locked'.
currently this limit supports rotational motors<br>
<ul>
<li> For Linear limits, use Generic6DofConstraint.setLinearUpperLimit, Generic6DofConstraint.setLinearLowerLimit. You can set the parameters with the TranslationalLimitMotor structure accsesible through the Generic6DofConstraint.getTranslationalLimitMotor method.
At this moment translational motors are not supported. May be in the future. </li>

<li> For Angular limits, use the RotationalLimitMotor structure for configuring the limit.
This is accessible through Generic6DofConstraint.getLimitMotor method,
This brings support for limit parameters and motors. </li>

<li> Angulars limits have these possible ranges:
<table border=1 >
<tr>
	<td><b>AXIS</b></td>
	<td><b>MIN ANGLE</b></td>
	<td><b>MAX ANGLE</b></td>
</tr><tr>
	<td>X</td>
	<td>-PI</td>
	<td>PI</td>
</tr><tr>
	<td>Y</td>
	<td>-PI/2</td>
	<td>PI/2</td>
</tr><tr>
	<td>Z</td>
	<td>-PI</td>
	<td>PI</td>
</tr>
</table>
</li>
</ul>

*/
ATTRIBUTE_ALIGNED16(class)
Generic6DofConstraint : public TypedConstraint
{
protected:
	//! relative_frames
	//!@{
	Transform2 m_frameInA;  //!< the constraint space w.r.t body A
	Transform2 m_frameInB;  //!< the constraint space w.r.t body B
	//!@}

	//! Jacobians
	//!@{
	JacobianEntry m_jacLinear[3];  //!< 3 orthogonal linear constraints
	JacobianEntry m_jacAng[3];     //!< 3 orthogonal angular constraints
	//!@}

	//! Linear_Limit_parameters
	//!@{
	TranslationalLimitMotor m_linearLimits;
	//!@}

	//! hinge_parameters
	//!@{
	RotationalLimitMotor m_angularLimits[3];
	//!@}

protected:
	//! temporal variables
	//!@{
	Scalar m_timeStep;
	Transform2 m_calculatedTransformA;
	Transform2 m_calculatedTransformB;
	Vec3 m_calculatedAxisAngleDiff;
	Vec3 m_calculatedAxis[3];
	Vec3 m_calculatedLinearDiff;
	Scalar m_factA;
	Scalar m_factB;
	bool m_hasStaticBody;

	Vec3 m_AnchorPos;  // point betwen pivots of bodies A and B to solve linear axes

	bool m_useLinearReferenceFrameA;
	bool m_useOffsetForConstraintFrame;

	i32 m_flags;

	//!@}

	Generic6DofConstraint& operator=(Generic6DofConstraint& other)
	{
		Assert(0);
		(void)other;
		return *this;
	}

	i32 setAngularLimits(ConstraintInfo2 * info, i32 row_offset, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB);

	i32 setLinearLimits(ConstraintInfo2 * info, i32 row, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB);

	void buildLinearJacobian(
		JacobianEntry & jacLinear, const Vec3& normalWorld,
		const Vec3& pivotAInW, const Vec3& pivotBInW);

	void buildAngularJacobian(JacobianEntry & jacAngular, const Vec3& jointAxisW);

	// tests linear limits
	void calculateLinearInfo();

	//! calcs the euler angles between the two bodies.
	void calculateAngleInfo();

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	///for backwards compatibility during the transition to 'getInfo/getInfo2'
	bool m_useSolveConstraintObsolete;

	Generic6DofConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	Generic6DofConstraint(RigidBody & rbB, const Transform2& frameInB, bool useLinearReferenceFrameB);

	//! Calcs global transform of the offsets
	/*!
	Calcs the global transform for the joint offset for body A an B, and also calcs the agle differences between the bodies.
	\sa Generic6DofConstraint.getCalculatedTransform2A , Generic6DofConstraint.getCalculatedTransform2B, Generic6DofConstraint.calculateAngleInfo
	*/
	void calculateTransforms(const Transform2& transA, const Transform2& transB);

	void calculateTransforms();

	//! Gets the global transform of the offset for body A
	/*!
    \sa Generic6DofConstraint.getFrameOffsetA, Generic6DofConstraint.getFrameOffsetB, Generic6DofConstraint.calculateAngleInfo.
    */
	const Transform2& getCalculatedTransform2A() const
	{
		return m_calculatedTransformA;
	}

	//! Gets the global transform of the offset for body B
	/*!
    \sa Generic6DofConstraint.getFrameOffsetA, Generic6DofConstraint.getFrameOffsetB, Generic6DofConstraint.calculateAngleInfo.
    */
	const Transform2& getCalculatedTransform2B() const
	{
		return m_calculatedTransformB;
	}

	const Transform2& getFrameOffsetA() const
	{
		return m_frameInA;
	}

	const Transform2& getFrameOffsetB() const
	{
		return m_frameInB;
	}

	Transform2& getFrameOffsetA()
	{
		return m_frameInA;
	}

	Transform2& getFrameOffsetB()
	{
		return m_frameInB;
	}

	//! performs Jacobian calculation, and also calculates angle differences and axis
	virtual void buildJacobian();

	virtual void getInfo1(ConstraintInfo1 * info);

	void getInfo1NonVirtual(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	void getInfo2NonVirtual(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB);

	void updateRHS(Scalar timeStep);

	//! Get the rotation axis in global coordinates
	/*!
	\pre Generic6DofConstraint.buildJacobian must be called previously.
	*/
	Vec3 getAxis(i32 axis_index) const;

	//! Get the relative Euler angle
	/*!
	\pre Generic6DofConstraint::calculateTransforms() must be called previously.
	*/
	Scalar getAngle(i32 axis_index) const;

	//! Get the relative position of the constraint pivot
	/*!
	\pre Generic6DofConstraint::calculateTransforms() must be called previously.
	*/
	Scalar getRelativePivotPosition(i32 axis_index) const;

	void setFrames(const Transform2& frameA, const Transform2& frameB);

	//! Test angular limit.
	/*!
	Calculates angular correction and returns true if limit needs to be corrected.
	\pre Generic6DofConstraint::calculateTransforms() must be called previously.
	*/
	bool testAngularLimitMotor(i32 axis_index);

	void setLinearLowerLimit(const Vec3& linearLower)
	{
		m_linearLimits.m_lowerLimit = linearLower;
	}

	void getLinearLowerLimit(Vec3 & linearLower) const
	{
		linearLower = m_linearLimits.m_lowerLimit;
	}

	void setLinearUpperLimit(const Vec3& linearUpper)
	{
		m_linearLimits.m_upperLimit = linearUpper;
	}

	void getLinearUpperLimit(Vec3 & linearUpper) const
	{
		linearUpper = m_linearLimits.m_upperLimit;
	}

	void setAngularLowerLimit(const Vec3& angularLower)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_loLimit = NormalizeAngle(angularLower[i]);
	}

	void getAngularLowerLimit(Vec3 & angularLower) const
	{
		for (i32 i = 0; i < 3; i++)
			angularLower[i] = m_angularLimits[i].m_loLimit;
	}

	void setAngularUpperLimit(const Vec3& angularUpper)
	{
		for (i32 i = 0; i < 3; i++)
			m_angularLimits[i].m_hiLimit = NormalizeAngle(angularUpper[i]);
	}

	void getAngularUpperLimit(Vec3 & angularUpper) const
	{
		for (i32 i = 0; i < 3; i++)
			angularUpper[i] = m_angularLimits[i].m_hiLimit;
	}

	//! Retrieves the angular limit informacion
	RotationalLimitMotor* getRotationalLimitMotor(i32 index)
	{
		return &m_angularLimits[index];
	}

	//! Retrieves the  limit informacion
	TranslationalLimitMotor* getTranslationalLimitMotor()
	{
		return &m_linearLimits;
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

	//! Test limit
	/*!
    - free means upper < lower,
    - locked means upper == lower
    - limited means upper > lower
    - limitIndex: first 3 are linear, next 3 are angular
    */
	bool isLimited(i32 limitIndex) const
	{
		if (limitIndex < 3)
		{
			return m_linearLimits.isLimited(limitIndex);
		}
		return m_angularLimits[limitIndex - 3].isLimited();
	}

	virtual void calcAnchorPos(void);  // overridable

	i32 get_limit_motor_info2(RotationalLimitMotor * limot,
							  const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB,
							  ConstraintInfo2* info, i32 row, Vec3& ax1, i32 rotational, i32 rotAllowed = false);

	// access for UseFrameOffset
	bool getUseFrameOffset() const { return m_useOffsetForConstraintFrame; }
	void setUseFrameOffset(bool frameOffsetOnOff) { m_useOffsetForConstraintFrame = frameOffsetOnOff; }

	bool getUseLinearReferenceFrameA() const { return m_useLinearReferenceFrameA; }
	void setUseLinearReferenceFrameA(bool linearReferenceFrameA) { m_useLinearReferenceFrameA = linearReferenceFrameA; }

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1);
	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const;

	void setAxis(const Vec3& axis1, const Vec3& axis2);

	virtual i32 getFlags() const
	{
		return m_flags;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

struct Generic6DofConstraintData
{
	TypedConstraintData m_typeConstraintData;
	Transform2FloatData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2FloatData m_rbBFrame;

	Vec3FloatData m_linearUpperLimit;
	Vec3FloatData m_linearLowerLimit;

	Vec3FloatData m_angularUpperLimit;
	Vec3FloatData m_angularLowerLimit;

	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

struct Generic6DofConstraintDoubleData2
{
	TypedConstraintDoubleData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2DoubleData m_rbBFrame;

	Vec3DoubleData m_linearUpperLimit;
	Vec3DoubleData m_linearLowerLimit;

	Vec3DoubleData m_angularUpperLimit;
	Vec3DoubleData m_angularLowerLimit;

	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

SIMD_FORCE_INLINE i32 Generic6DofConstraint::calculateSerializeBufferSize() const
{
	return sizeof(Generic6DofConstraintData2);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk Generic6DofConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	Generic6DofConstraintData2* dof = (Generic6DofConstraintData2*)dataBuffer;
	TypedConstraint::serialize(&dof->m_typeConstraintData, serializer);

	m_frameInA.serialize(dof->m_rbAFrame);
	m_frameInB.serialize(dof->m_rbBFrame);

	i32 i;
	for (i = 0; i < 3; i++)
	{
		dof->m_angularLowerLimit.m_floats[i] = m_angularLimits[i].m_loLimit;
		dof->m_angularUpperLimit.m_floats[i] = m_angularLimits[i].m_hiLimit;
		dof->m_linearLowerLimit.m_floats[i] = m_linearLimits.m_lowerLimit[i];
		dof->m_linearUpperLimit.m_floats[i] = m_linearLimits.m_upperLimit[i];
	}

	dof->m_useLinearReferenceFrameA = m_useLinearReferenceFrameA ? 1 : 0;
	dof->m_useOffsetForConstraintFrame = m_useOffsetForConstraintFrame ? 1 : 0;

	return Generic6DofConstraintDataName;
}

#endif  //DRX3D_GENERIC_6DOF_CONSTRAINT_H
