/*
Overview:

ConeTwistConstraint can be used to simulate ragdoll joints (upper arm, leg etc).
It is a fixed translation, 3 degree-of-freedom (DOF) rotational "joint".
It divides the 3 rotational DOFs into swing (movement within a cone) and twist.
Swing is divided into swing1 and swing2 which can have different limits, giving an elliptical shape.
(Note: the cone's base isn't flat, so this ellipse is "embedded" on the surface of a sphere.)

In the contraint's frame of reference:
twist is along the x-axis,
and swing 1 and 2 are along the z and y axes respectively.
*/

#ifndef DRX3D_CONETWISTCONSTRAINT_H
#define DRX3D_CONETWISTCONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define ConeTwistConstraintData2 ConeTwistConstraintDoubleData
#define ConeTwistConstraintDataName "ConeTwistConstraintDoubleData"
#else
#define ConeTwistConstraintData2 ConeTwistConstraintData
#define ConeTwistConstraintDataName "ConeTwistConstraintData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

class RigidBody;

enum ConeTwistFlags
{
	DRX3D_CONETWIST_FLAGS_LIN_CFM = 1,
	DRX3D_CONETWIST_FLAGS_LIN_ERP = 2,
	DRX3D_CONETWIST_FLAGS_ANG_CFM = 4
};

//ConeTwistConstraint can be used to simulate ragdoll joints (upper arm, leg etc)
ATTRIBUTE_ALIGNED16(class)
ConeTwistConstraint : public TypedConstraint
{
#ifdef IN_PARALLELL_SOLVER
public:
#endif
	JacobianEntry m_jac[3];  //3 orthogonal linear constraints

	Transform2 m_rbAFrame;
	Transform2 m_rbBFrame;

	Scalar m_limitSoftness;
	Scalar m_biasFactor;
	Scalar m_relaxationFactor;

	Scalar m_damping;

	Scalar m_swingSpan1;
	Scalar m_swingSpan2;
	Scalar m_twistSpan;

	Scalar m_fixThresh;

	Vec3 m_swingAxis;
	Vec3 m_twistAxis;

	Scalar m_kSwing;
	Scalar m_kTwist;

	Scalar m_twistLimitSign;
	Scalar m_swingCorrection;
	Scalar m_twistCorrection;

	Scalar m_twistAngle;

	Scalar m_accSwingLimitImpulse;
	Scalar m_accTwistLimitImpulse;

	bool m_angularOnly;
	bool m_solveTwistLimit;
	bool m_solveSwingLimit;

	bool m_useSolveConstraintObsolete;

	// not yet used...
	Scalar m_swingLimitRatio;
	Scalar m_twistLimitRatio;
	Vec3 m_twistAxisA;

	// motor
	bool m_bMotorEnabled;
	bool m_bNormalizedMotorStrength;
	Quat m_qTarget;
	Scalar m_maxMotorImpulse;
	Vec3 m_accMotorImpulse;

	// parameters
	i32 m_flags;
	Scalar m_linCFM;
	Scalar m_linERP;
	Scalar m_angCFM;

protected:
	void init();

	void computeConeLimitInfo(const Quat& qCone,                                           // in
							  Scalar& swingAngle, Vec3& vSwingAxis, Scalar& swingLimit);  // all outs

	void computeTwistLimitInfo(const Quat& qTwist,                    // in
							   Scalar& twistAngle, Vec3& vTwistAxis);  // all outs

	void adjustSwingAxisToUseEllipseNormal(Vec3 & vSwingAxis) const;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	ConeTwistConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& rbAFrame, const Transform2& rbBFrame);

	ConeTwistConstraint(RigidBody & rbA, const Transform2& rbAFrame);

	virtual void buildJacobian();

	virtual void getInfo1(ConstraintInfo1 * info);

	void getInfo1NonVirtual(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	void getInfo2NonVirtual(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Matrix3x3& invInertiaWorldA, const Matrix3x3& invInertiaWorldB);

	virtual void solveConstraintObsolete(SolverBody & bodyA, SolverBody & bodyB, Scalar timeStep);

	void updateRHS(Scalar timeStep);

	const RigidBody& getRigidBodyA() const
	{
		return m_rbA;
	}
	const RigidBody& getRigidBodyB() const
	{
		return m_rbB;
	}

	void setAngularOnly(bool angularOnly)
	{
		m_angularOnly = angularOnly;
	}

	bool getAngularOnly() const
	{
		return m_angularOnly;
	}

	void setLimit(i32 limitIndex, Scalar limitValue)
	{
		switch (limitIndex)
		{
			case 3:
			{
				m_twistSpan = limitValue;
				break;
			}
			case 4:
			{
				m_swingSpan2 = limitValue;
				break;
			}
			case 5:
			{
				m_swingSpan1 = limitValue;
				break;
			}
			default:
			{
			}
		};
	}

	Scalar getLimit(i32 limitIndex) const
	{
		switch (limitIndex)
		{
			case 3:
			{
				return m_twistSpan;
				break;
			}
			case 4:
			{
				return m_swingSpan2;
				break;
			}
			case 5:
			{
				return m_swingSpan1;
				break;
			}
			default:
			{
				Assert(0 && "Invalid limitIndex specified for ConeTwistConstraint");
				return 0.0;
			}
		};
	}

	// setLimit(), a few notes:
	// _softness:
	//		0->1, recommend ~0.8->1.
	//		describes % of limits where movement is free.
	//		beyond this softness %, the limit is gradually enforced until the "hard" (1.0) limit is reached.
	// _biasFactor:
	//		0->1?, recommend 0.3 +/-0.3 or so.
	//		strength with which constraint resists zeroth order (angular, not angular velocity) limit violation.
	// __relaxationFactor:
	//		0->1, recommend to stay near 1.
	//		the lower the value, the less the constraint will fight velocities which violate the angular limits.
	void setLimit(Scalar _swingSpan1, Scalar _swingSpan2, Scalar _twistSpan, Scalar _softness = 1.f, Scalar _biasFactor = 0.3f, Scalar _relaxationFactor = 1.0f)
	{
		m_swingSpan1 = _swingSpan1;
		m_swingSpan2 = _swingSpan2;
		m_twistSpan = _twistSpan;

		m_limitSoftness = _softness;
		m_biasFactor = _biasFactor;
		m_relaxationFactor = _relaxationFactor;
	}

	const Transform2& getAFrame() const { return m_rbAFrame; };
	const Transform2& getBFrame() const { return m_rbBFrame; };

	inline i32 getSolveTwistLimit()
	{
		return m_solveTwistLimit;
	}

	inline i32 getSolveSwingLimit()
	{
		return m_solveSwingLimit;
	}

	inline Scalar getTwistLimitSign()
	{
		return m_twistLimitSign;
	}

	void calcAngleInfo();
	void calcAngleInfo2(const Transform2& transA, const Transform2& transB, const Matrix3x3& invInertiaWorldA, const Matrix3x3& invInertiaWorldB);

	inline Scalar getSwingSpan1() const
	{
		return m_swingSpan1;
	}
	inline Scalar getSwingSpan2() const
	{
		return m_swingSpan2;
	}
	inline Scalar getTwistSpan() const
	{
		return m_twistSpan;
	}
	inline Scalar getLimitSoftness() const
	{
		return m_limitSoftness;
	}
	inline Scalar getBiasFactor() const
	{
		return m_biasFactor;
	}
	inline Scalar getRelaxationFactor() const
	{
		return m_relaxationFactor;
	}
	inline Scalar getTwistAngle() const
	{
		return m_twistAngle;
	}
	bool isPastSwingLimit() { return m_solveSwingLimit; }

	Scalar getDamping() const { return m_damping; }
	void setDamping(Scalar damping) { m_damping = damping; }

	void enableMotor(bool b) { m_bMotorEnabled = b; }
	bool isMotorEnabled() const { return m_bMotorEnabled; }
	Scalar getMaxMotorImpulse() const { return m_maxMotorImpulse; }
	bool isMaxMotorImpulseNormalized() const { return m_bNormalizedMotorStrength; }
	void setMaxMotorImpulse(Scalar maxMotorImpulse)
	{
		m_maxMotorImpulse = maxMotorImpulse;
		m_bNormalizedMotorStrength = false;
	}
	void setMaxMotorImpulseNormalized(Scalar maxMotorImpulse)
	{
		m_maxMotorImpulse = maxMotorImpulse;
		m_bNormalizedMotorStrength = true;
	}

	Scalar getFixThresh() { return m_fixThresh; }
	void setFixThresh(Scalar fixThresh) { m_fixThresh = fixThresh; }

	// setMotorTarget:
	// q: the desired rotation of bodyA wrt bodyB.
	// note: if q violates the joint limits, the internal target is clamped to avoid conflicting impulses (very bad for stability)
	// note: don't forget to enableMotor()
	void setMotorTarget(const Quat& q);
	const Quat& getMotorTarget() const { return m_qTarget; }

	// same as above, but q is the desired rotation of frameA wrt frameB in constraint space
	void setMotorTargetInConstraintSpace(const Quat& q);

	Vec3 GetPointForAngle(Scalar fAngleInRadians, Scalar fLength) const;

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1);

	virtual void setFrames(const Transform2& frameA, const Transform2& frameB);

	const Transform2& getFrameOffsetA() const
	{
		return m_rbAFrame;
	}

	const Transform2& getFrameOffsetB() const
	{
		return m_rbBFrame;
	}

	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const;

	i32 getFlags() const
	{
		return m_flags;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

struct ConeTwistConstraintDoubleData
{
	TypedConstraintDoubleData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;
	Transform2DoubleData m_rbBFrame;

	//limits
	double m_swingSpan1;
	double m_swingSpan2;
	double m_twistSpan;
	double m_limitSoftness;
	double m_biasFactor;
	double m_relaxationFactor;

	double m_damping;
};

#ifdef DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
///this structure is not used, except for loading pre-2.82 .bullet files
struct ConeTwistConstraintData
{
	TypedConstraintData m_typeConstraintData;
	Transform2FloatData m_rbAFrame;
	Transform2FloatData m_rbBFrame;

	//limits
	float m_swingSpan1;
	float m_swingSpan2;
	float m_twistSpan;
	float m_limitSoftness;
	float m_biasFactor;
	float m_relaxationFactor;

	float m_damping;

	char m_pad[4];
};
#endif  //DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
//

SIMD_FORCE_INLINE i32 ConeTwistConstraint::calculateSerializeBufferSize() const
{
	return sizeof(ConeTwistConstraintData2);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk ConeTwistConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	ConeTwistConstraintData2* cone = (ConeTwistConstraintData2*)dataBuffer;
	TypedConstraint::serialize(&cone->m_typeConstraintData, serializer);

	m_rbAFrame.serialize(cone->m_rbAFrame);
	m_rbBFrame.serialize(cone->m_rbBFrame);

	cone->m_swingSpan1 = m_swingSpan1;
	cone->m_swingSpan2 = m_swingSpan2;
	cone->m_twistSpan = m_twistSpan;
	cone->m_limitSoftness = m_limitSoftness;
	cone->m_biasFactor = m_biasFactor;
	cone->m_relaxationFactor = m_relaxationFactor;
	cone->m_damping = m_damping;

	return ConeTwistConstraintDataName;
}

#endif  //DRX3D_CONETWISTCONSTRAINT_H
