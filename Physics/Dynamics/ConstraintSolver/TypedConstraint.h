#ifndef DRX3D_TYPED_CONSTRAINT_H
#define DRX3D_TYPED_CONSTRAINT_H

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define TypedConstraintData2 TypedConstraintDoubleData
#define TypedConstraintDataName "TypedConstraintDoubleData"
#else
#define TypedConstraintData2 TypedConstraintFloatData
#define TypedConstraintDataName "TypedConstraintFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

class Serializer;

//Don't change any of the existing enum values, so add enum types at the end for serialization compatibility
enum TypedConstraintType
{
	POINT2POINT_CONSTRAINT_TYPE = 3,
	HINGE_CONSTRAINT_TYPE,
	CONETWIST_CONSTRAINT_TYPE,
	D6_CONSTRAINT_TYPE,
	SLIDER_CONSTRAINT_TYPE,
	CONTACT_CONSTRAINT_TYPE,
	D6_SPRING_CONSTRAINT_TYPE,
	GEAR_CONSTRAINT_TYPE,
	FIXED_CONSTRAINT_TYPE,
	D6_SPRING_2_CONSTRAINT_TYPE,
	MAX_CONSTRAINT_TYPE
};

enum ConstraintParams
{
	DRX3D_CONSTRAINT_ERP = 1,
	DRX3D_CONSTRAINT_STOP_ERP,
	DRX3D_CONSTRAINT_CFM,
	DRX3D_CONSTRAINT_STOP_CFM
};

#if 1
#define AssertConstrParams(_par) Assert(_par)
#else
#define AssertConstrParams(_par)
#endif

ATTRIBUTE_ALIGNED16(struct)
JointFeedback
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();
	Vec3 m_appliedForceBodyA;
	Vec3 m_appliedTorqueBodyA;
	Vec3 m_appliedForceBodyB;
	Vec3 m_appliedTorqueBodyB;
};

///TypedConstraint is the baseclass for drx3D constraints and vehicles
ATTRIBUTE_ALIGNED16(class)
TypedConstraint : public TypedObject
{
	i32 m_userConstraintType;

	union {
		i32 m_userConstraintId;
		uk m_userConstraintPtr;
	};

	Scalar m_breakingImpulseThreshold;
	bool m_isEnabled;
	bool m_needsFeedback;
	i32 m_overrideNumSolverIterations;

	TypedConstraint& operator=(TypedConstraint& other)
	{
		Assert(0);
		(void)other;
		return *this;
	}

protected:
	RigidBody& m_rbA;
	RigidBody& m_rbB;
	Scalar m_appliedImpulse;
	Scalar m_dbgDrawSize;
	JointFeedback* m_jointFeedback;

	///internal method used by the constraint solver, don't use them directly
	Scalar getMotorFactor(Scalar pos, Scalar lowLim, Scalar uppLim, Scalar vel, Scalar timeFact);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	virtual ~TypedConstraint(){};
	TypedConstraint(TypedConstraintType type, RigidBody & rbA);
	TypedConstraint(TypedConstraintType type, RigidBody & rbA, RigidBody & rbB);

	struct ConstraintInfo1
	{
		i32 m_numConstraintRows, nub;
	};

	static RigidBody& getFixedBody();

	struct ConstraintInfo2
	{
		// integrator parameters: frames per second (1/stepsize), default error
		// reduction parameter (0..1).
		Scalar fps, erp;

		// for the first and second body, pointers to two (linear and angular)
		// n*3 jacobian sub matrices, stored by rows. these matrices will have
		// been initialized to 0 on entry. if the second body is zero then the
		// J2xx pointers may be 0.
		Scalar *m_J1linearAxis, *m_J1angularAxis, *m_J2linearAxis, *m_J2angularAxis;

		// elements to jump from one row to the next in J's
		i32 rowskip;

		// right hand sides of the equation J*v = c + cfm * lambda. cfm is the
		// "constraint force mixing" vector. c is set to zero on entry, cfm is
		// set to a constant value (typically very small or zero) value on entry.
		Scalar *m_constraintError, *cfm;

		// lo and hi limits for variables (set to -/+ infinity on entry).
		Scalar *m_lowerLimit, *m_upperLimit;

		// number of solver iterations
		i32 m_numIterations;

		//damping of the velocity
		Scalar m_damping;
	};

	i32 getOverrideNumSolverIterations() const
	{
		return m_overrideNumSolverIterations;
	}

	///override the number of constraint solver iterations used to solve this constraint
	///-1 will use the default number of iterations, as specified in SolverInfo.m_numIterations
	void setOverrideNumSolverIterations(i32 overideNumIterations)
	{
		m_overrideNumSolverIterations = overideNumIterations;
	}

	///internal method used by the constraint solver, don't use them directly
	virtual void buildJacobian(){};

	///internal method used by the constraint solver, don't use them directly
	virtual void setupSolverConstraint(ConstraintArray & ca, i32 solverBodyA, i32 solverBodyB, Scalar timeStep)
	{
		(void)ca;
		(void)solverBodyA;
		(void)solverBodyB;
		(void)timeStep;
	}

	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo1(ConstraintInfo1 * info) = 0;

	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo2(ConstraintInfo2 * info) = 0;

	///internal method used by the constraint solver, don't use them directly
	void internalSetAppliedImpulse(Scalar appliedImpulse)
	{
		m_appliedImpulse = appliedImpulse;
	}
	///internal method used by the constraint solver, don't use them directly
	Scalar internalGetAppliedImpulse()
	{
		return m_appliedImpulse;
	}

	Scalar getBreakingImpulseThreshold() const
	{
		return m_breakingImpulseThreshold;
	}

	void setBreakingImpulseThreshold(Scalar threshold)
	{
		m_breakingImpulseThreshold = threshold;
	}

	bool isEnabled() const
	{
		return m_isEnabled;
	}

	void setEnabled(bool enabled)
	{
		m_isEnabled = enabled;
	}

	///internal method used by the constraint solver, don't use them directly
	virtual void solveConstraintObsolete(SolverBody& /*bodyA*/, SolverBody& /*bodyB*/, Scalar /*timeStep*/){};

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

	i32 getUserConstraintType() const
	{
		return m_userConstraintType;
	}

	void setUserConstraintType(i32 userConstraintType)
	{
		m_userConstraintType = userConstraintType;
	};

	void setUserConstraintId(i32 uid)
	{
		m_userConstraintId = uid;
	}

	i32 getUserConstraintId() const
	{
		return m_userConstraintId;
	}

	void setUserConstraintPtr(uk ptr)
	{
		m_userConstraintPtr = ptr;
	}

	uk getUserConstraintPtr()
	{
		return m_userConstraintPtr;
	}

	void setJointFeedback(JointFeedback * jointFeedback)
	{
		m_jointFeedback = jointFeedback;
	}

	const JointFeedback* getJointFeedback() const
	{
		return m_jointFeedback;
	}

	JointFeedback* getJointFeedback()
	{
		return m_jointFeedback;
	}

	i32 getUid() const
	{
		return m_userConstraintId;
	}

	bool needsFeedback() const
	{
		return m_needsFeedback;
	}

	///enableFeedback will allow to read the applied linear and angular impulse
	///use getAppliedImpulse, getAppliedLinearImpulse and getAppliedAngularImpulse to read feedback information
	void enableFeedback(bool needsFeedback)
	{
		m_needsFeedback = needsFeedback;
	}

	///getAppliedImpulse is an estimated total applied impulse.
	///This feedback could be used to determine breaking constraints or playing sounds.
	Scalar getAppliedImpulse() const
	{
		Assert(m_needsFeedback);
		return m_appliedImpulse;
	}

	TypedConstraintType getConstraintType() const
	{
		return TypedConstraintType(m_objectType);
	}

	void setDbgDrawSize(Scalar dbgDrawSize)
	{
		m_dbgDrawSize = dbgDrawSize;
	}
	Scalar getDbgDrawSize()
	{
		return m_dbgDrawSize;
	}

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
	///If no axis is provided, it uses the default axis for this constraint.
	virtual void setParam(i32 num, Scalar value, i32 axis = -1) = 0;

	///return the local value of parameter
	virtual Scalar getParam(i32 num, i32 axis = -1) const = 0;

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

// returns angle in range [-SIMD_2_PI, SIMD_2_PI], closest to one of the limits
// all arguments should be normalized angles (i.e. in range [-SIMD_PI, SIMD_PI])
SIMD_FORCE_INLINE Scalar AdjustAngleToLimits(Scalar angleInRadians, Scalar angleLowerLimitInRadians, Scalar angleUpperLimitInRadians)
{
	if (angleLowerLimitInRadians >= angleUpperLimitInRadians)
	{
		return angleInRadians;
	}
	else if (angleInRadians < angleLowerLimitInRadians)
	{
		Scalar diffLo = Fabs(NormalizeAngle(angleLowerLimitInRadians - angleInRadians));
		Scalar diffHi = Fabs(NormalizeAngle(angleUpperLimitInRadians - angleInRadians));
		return (diffLo < diffHi) ? angleInRadians : (angleInRadians + SIMD_2_PI);
	}
	else if (angleInRadians > angleUpperLimitInRadians)
	{
		Scalar diffHi = Fabs(NormalizeAngle(angleInRadians - angleUpperLimitInRadians));
		Scalar diffLo = Fabs(NormalizeAngle(angleInRadians - angleLowerLimitInRadians));
		return (diffLo < diffHi) ? (angleInRadians - SIMD_2_PI) : angleInRadians;
	}
	else
	{
		return angleInRadians;
	}
}

// clang-format off

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	TypedConstraintFloatData
{
	RigidBodyFloatData		*m_rbA;
	RigidBodyFloatData		*m_rbB;
	char	*m_name;

	i32	m_objectType;
	i32	m_userConstraintType;
	i32	m_userConstraintId;
	i32	m_needsFeedback;

	float	m_appliedImpulse;
	float	m_dbgDrawSize;

	i32	m_disableCollisionsBetweenLinkedBodies;
	i32	m_overrideNumSolverIterations;

	float	m_breakingImpulseThreshold;
	i32		m_isEnabled;
	
};



///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64

#define DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
#ifdef DRX3D_BACKWARDS_COMPATIBLE_SERIALIZATION
///this structure is not used, except for loading pre-2.82 .bullet files
struct	TypedConstraintData
{
	RigidBodyData		*m_rbA;
	RigidBodyData		*m_rbB;
	char	*m_name;

	i32	m_objectType;
	i32	m_userConstraintType;
	i32	m_userConstraintId;
	i32	m_needsFeedback;

	float	m_appliedImpulse;
	float	m_dbgDrawSize;

	i32	m_disableCollisionsBetweenLinkedBodies;
	i32	m_overrideNumSolverIterations;

	float	m_breakingImpulseThreshold;
	i32		m_isEnabled;
	
};
#endif //BACKWARDS_COMPATIBLE

struct	TypedConstraintDoubleData
{
	RigidBodyDoubleData		*m_rbA;
	RigidBodyDoubleData		*m_rbB;
	char	*m_name;

	i32	m_objectType;
	i32	m_userConstraintType;
	i32	m_userConstraintId;
	i32	m_needsFeedback;

	double	m_appliedImpulse;
	double	m_dbgDrawSize;

	i32	m_disableCollisionsBetweenLinkedBodies;
	i32	m_overrideNumSolverIterations;

	double	m_breakingImpulseThreshold;
	i32		m_isEnabled;
	char	padding[4];
	
};

// clang-format on

SIMD_FORCE_INLINE i32 TypedConstraint::calculateSerializeBufferSize() const
{
	return sizeof(TypedConstraintData2);
}

class AngularLimit
{
private:
	Scalar
		m_center,
		m_halfRange,
		m_softness,
		m_biasFactor,
		m_relaxationFactor,
		m_correction,
		m_sign;

	bool
		m_solveLimit;

public:
	/// Default constructor initializes limit as inactive, allowing free constraint movement
	AngularLimit()
		: m_center(0.0f),
		  m_halfRange(-1.0f),
		  m_softness(0.9f),
		  m_biasFactor(0.3f),
		  m_relaxationFactor(1.0f),
		  m_correction(0.0f),
		  m_sign(0.0f),
		  m_solveLimit(false)
	{
	}

	/// Sets all limit's parameters.
	/// When low > high limit becomes inactive.
	/// When high - low > 2PI limit is ineffective too becouse no angle can exceed the limit
	void set(Scalar low, Scalar high, Scalar _softness = 0.9f, Scalar _biasFactor = 0.3f, Scalar _relaxationFactor = 1.0f);

	/// Checks conastaint angle against limit. If limit is active and the angle violates the limit
	/// correction is calculated.
	void test(const Scalar angle);

	/// Returns limit's softness
	inline Scalar getSoftness() const
	{
		return m_softness;
	}

	/// Returns limit's bias factor
	inline Scalar getBiasFactor() const
	{
		return m_biasFactor;
	}

	/// Returns limit's relaxation factor
	inline Scalar getRelaxationFactor() const
	{
		return m_relaxationFactor;
	}

	/// Returns correction value evaluated when test() was invoked
	inline Scalar getCorrection() const
	{
		return m_correction;
	}

	/// Returns sign value evaluated when test() was invoked
	inline Scalar getSign() const
	{
		return m_sign;
	}

	/// Gives half of the distance between min and max limit angle
	inline Scalar getHalfRange() const
	{
		return m_halfRange;
	}

	/// Returns true when the last test() invocation recognized limit violation
	inline bool isLimit() const
	{
		return m_solveLimit;
	}

	/// Checks given angle against limit. If limit is active and angle doesn't fit it, the angle
	/// returned is modified so it equals to the limit closest to given angle.
	void fit(Scalar& angle) const;

	/// Returns correction value multiplied by sign value
	Scalar getError() const;

	Scalar getLow() const;

	Scalar getHigh() const;
};

#endif  //DRX3D_TYPED_CONSTRAINT_H
