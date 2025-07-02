/*
TODO:
 - add clamping od accumulated impulse to improve stability
 - add conversion for ODE constraint solver
*/

#ifndef DRX3D_SLIDER_CONSTRAINT_H
#define DRX3D_SLIDER_CONSTRAINT_H

#include <drx3D/Maths/Linear/Scalar.h>  //for DRX3D_USE_DOUBLE_PRECISION

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define SliderConstraintData2 SliderConstraintDoubleData
#define SliderConstraintDataName "SliderConstraintDoubleData"
#else
#define SliderConstraintData2 SliderConstraintData
#define SliderConstraintDataName "SliderConstraintData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>

class RigidBody;

#define SLIDER_CONSTRAINT_DEF_SOFTNESS (Scalar(1.0))
#define SLIDER_CONSTRAINT_DEF_DAMPING (Scalar(1.0))
#define SLIDER_CONSTRAINT_DEF_RESTITUTION (Scalar(0.7))
#define SLIDER_CONSTRAINT_DEF_CFM (Scalar(0.f))

enum SliderFlags
{
	DRX3D_SLIDER_FLAGS_CFM_DIRLIN = (1 << 0),
	DRX3D_SLIDER_FLAGS_ERP_DIRLIN = (1 << 1),
	DRX3D_SLIDER_FLAGS_CFM_DIRANG = (1 << 2),
	DRX3D_SLIDER_FLAGS_ERP_DIRANG = (1 << 3),
	DRX3D_SLIDER_FLAGS_CFM_ORTLIN = (1 << 4),
	DRX3D_SLIDER_FLAGS_ERP_ORTLIN = (1 << 5),
	DRX3D_SLIDER_FLAGS_CFM_ORTANG = (1 << 6),
	DRX3D_SLIDER_FLAGS_ERP_ORTANG = (1 << 7),
	DRX3D_SLIDER_FLAGS_CFM_LIMLIN = (1 << 8),
	DRX3D_SLIDER_FLAGS_ERP_LIMLIN = (1 << 9),
	DRX3D_SLIDER_FLAGS_CFM_LIMANG = (1 << 10),
	DRX3D_SLIDER_FLAGS_ERP_LIMANG = (1 << 11)
};

ATTRIBUTE_ALIGNED16(class)
SliderConstraint : public TypedConstraint
{
protected:
	///for backwards compatibility during the transition to 'getInfo/getInfo2'
	bool m_useSolveConstraintObsolete;
	bool m_useOffsetForConstraintFrame;
	Transform2 m_frameInA;
	Transform2 m_frameInB;
	// use frameA fo define limits, if true
	bool m_useLinearReferenceFrameA;
	// linear limits
	Scalar m_lowerLinLimit;
	Scalar m_upperLinLimit;
	// angular limits
	Scalar m_lowerAngLimit;
	Scalar m_upperAngLimit;
	// softness, restitution and damping for different cases
	// DirLin - moving inside linear limits
	// LimLin - hitting linear limit
	// DirAng - moving inside angular limits
	// LimAng - hitting angular limit
	// OrthoLin, OrthoAng - against constraint axis
	Scalar m_softnessDirLin;
	Scalar m_restitutionDirLin;
	Scalar m_dampingDirLin;
	Scalar m_cfmDirLin;

	Scalar m_softnessDirAng;
	Scalar m_restitutionDirAng;
	Scalar m_dampingDirAng;
	Scalar m_cfmDirAng;

	Scalar m_softnessLimLin;
	Scalar m_restitutionLimLin;
	Scalar m_dampingLimLin;
	Scalar m_cfmLimLin;

	Scalar m_softnessLimAng;
	Scalar m_restitutionLimAng;
	Scalar m_dampingLimAng;
	Scalar m_cfmLimAng;

	Scalar m_softnessOrthoLin;
	Scalar m_restitutionOrthoLin;
	Scalar m_dampingOrthoLin;
	Scalar m_cfmOrthoLin;

	Scalar m_softnessOrthoAng;
	Scalar m_restitutionOrthoAng;
	Scalar m_dampingOrthoAng;
	Scalar m_cfmOrthoAng;

	// for interlal use
	bool m_solveLinLim;
	bool m_solveAngLim;

	i32 m_flags;

	JacobianEntry m_jacLin[3];
	Scalar m_jacLinDiagABInv[3];

	JacobianEntry m_jacAng[3];

	Scalar m_timeStep;
	Transform2 m_calculatedTransformA;
	Transform2 m_calculatedTransformB;

	Vec3 m_sliderAxis;
	Vec3 m_realPivotAInW;
	Vec3 m_realPivotBInW;
	Vec3 m_projPivotInW;
	Vec3 m_delta;
	Vec3 m_depth;
	Vec3 m_relPosA;
	Vec3 m_relPosB;

	Scalar m_linPos;
	Scalar m_angPos;

	Scalar m_angDepth;
	Scalar m_kAngle;

	bool m_poweredLinMotor;
	Scalar m_targetLinMotorVelocity;
	Scalar m_maxLinMotorForce;
	Scalar m_accumulatedLinMotorImpulse;

	bool m_poweredAngMotor;
	Scalar m_targetAngMotorVelocity;
	Scalar m_maxAngMotorForce;
	Scalar m_accumulatedAngMotorImpulse;

	//------------------------
	void initParams();

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	// constructors
	SliderConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	SliderConstraint(RigidBody & rbB, const Transform2& frameInB, bool useLinearReferenceFrameA);

	// overrides

	virtual void getInfo1(ConstraintInfo1 * info);

	void getInfo1NonVirtual(ConstraintInfo1 * info);

	virtual void getInfo2(ConstraintInfo2 * info);

	void getInfo2NonVirtual(ConstraintInfo2 * info, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, Scalar rbAinvMass, Scalar rbBinvMass);

	// access
	const RigidBody& getRigidBodyA() const { return m_rbA; }
	const RigidBody& getRigidBodyB() const { return m_rbB; }
	const Transform2& getCalculatedTransform2A() const { return m_calculatedTransformA; }
	const Transform2& getCalculatedTransform2B() const { return m_calculatedTransformB; }
	const Transform2& getFrameOffsetA() const { return m_frameInA; }
	const Transform2& getFrameOffsetB() const { return m_frameInB; }
	Transform2& getFrameOffsetA() { return m_frameInA; }
	Transform2& getFrameOffsetB() { return m_frameInB; }
	Scalar getLowerLinLimit() { return m_lowerLinLimit; }
	void setLowerLinLimit(Scalar lowerLimit) { m_lowerLinLimit = lowerLimit; }
	Scalar getUpperLinLimit() { return m_upperLinLimit; }
	void setUpperLinLimit(Scalar upperLimit) { m_upperLinLimit = upperLimit; }
	Scalar getLowerAngLimit() { return m_lowerAngLimit; }
	void setLowerAngLimit(Scalar lowerLimit) { m_lowerAngLimit = NormalizeAngle(lowerLimit); }
	Scalar getUpperAngLimit() { return m_upperAngLimit; }
	void setUpperAngLimit(Scalar upperLimit) { m_upperAngLimit = NormalizeAngle(upperLimit); }
	bool getUseLinearReferenceFrameA() { return m_useLinearReferenceFrameA; }
	Scalar getSoftnessDirLin() { return m_softnessDirLin; }
	Scalar getRestitutionDirLin() { return m_restitutionDirLin; }
	Scalar getDampingDirLin() { return m_dampingDirLin; }
	Scalar getSoftnessDirAng() { return m_softnessDirAng; }
	Scalar getRestitutionDirAng() { return m_restitutionDirAng; }
	Scalar getDampingDirAng() { return m_dampingDirAng; }
	Scalar getSoftnessLimLin() { return m_softnessLimLin; }
	Scalar getRestitutionLimLin() { return m_restitutionLimLin; }
	Scalar getDampingLimLin() { return m_dampingLimLin; }
	Scalar getSoftnessLimAng() { return m_softnessLimAng; }
	Scalar getRestitutionLimAng() { return m_restitutionLimAng; }
	Scalar getDampingLimAng() { return m_dampingLimAng; }
	Scalar getSoftnessOrthoLin() { return m_softnessOrthoLin; }
	Scalar getRestitutionOrthoLin() { return m_restitutionOrthoLin; }
	Scalar getDampingOrthoLin() { return m_dampingOrthoLin; }
	Scalar getSoftnessOrthoAng() { return m_softnessOrthoAng; }
	Scalar getRestitutionOrthoAng() { return m_restitutionOrthoAng; }
	Scalar getDampingOrthoAng() { return m_dampingOrthoAng; }
	void setSoftnessDirLin(Scalar softnessDirLin) { m_softnessDirLin = softnessDirLin; }
	void setRestitutionDirLin(Scalar restitutionDirLin) { m_restitutionDirLin = restitutionDirLin; }
	void setDampingDirLin(Scalar dampingDirLin) { m_dampingDirLin = dampingDirLin; }
	void setSoftnessDirAng(Scalar softnessDirAng) { m_softnessDirAng = softnessDirAng; }
	void setRestitutionDirAng(Scalar restitutionDirAng) { m_restitutionDirAng = restitutionDirAng; }
	void setDampingDirAng(Scalar dampingDirAng) { m_dampingDirAng = dampingDirAng; }
	void setSoftnessLimLin(Scalar softnessLimLin) { m_softnessLimLin = softnessLimLin; }
	void setRestitutionLimLin(Scalar restitutionLimLin) { m_restitutionLimLin = restitutionLimLin; }
	void setDampingLimLin(Scalar dampingLimLin) { m_dampingLimLin = dampingLimLin; }
	void setSoftnessLimAng(Scalar softnessLimAng) { m_softnessLimAng = softnessLimAng; }
	void setRestitutionLimAng(Scalar restitutionLimAng) { m_restitutionLimAng = restitutionLimAng; }
	void setDampingLimAng(Scalar dampingLimAng) { m_dampingLimAng = dampingLimAng; }
	void setSoftnessOrthoLin(Scalar softnessOrthoLin) { m_softnessOrthoLin = softnessOrthoLin; }
	void setRestitutionOrthoLin(Scalar restitutionOrthoLin) { m_restitutionOrthoLin = restitutionOrthoLin; }
	void setDampingOrthoLin(Scalar dampingOrthoLin) { m_dampingOrthoLin = dampingOrthoLin; }
	void setSoftnessOrthoAng(Scalar softnessOrthoAng) { m_softnessOrthoAng = softnessOrthoAng; }
	void setRestitutionOrthoAng(Scalar restitutionOrthoAng) { m_restitutionOrthoAng = restitutionOrthoAng; }
	void setDampingOrthoAng(Scalar dampingOrthoAng) { m_dampingOrthoAng = dampingOrthoAng; }
	void setPoweredLinMotor(bool onOff) { m_poweredLinMotor = onOff; }
	bool getPoweredLinMotor() { return m_poweredLinMotor; }
	void setTargetLinMotorVelocity(Scalar targetLinMotorVelocity) { m_targetLinMotorVelocity = targetLinMotorVelocity; }
	Scalar getTargetLinMotorVelocity() { return m_targetLinMotorVelocity; }
	void setMaxLinMotorForce(Scalar maxLinMotorForce) { m_maxLinMotorForce = maxLinMotorForce; }
	Scalar getMaxLinMotorForce() { return m_maxLinMotorForce; }
	void setPoweredAngMotor(bool onOff) { m_poweredAngMotor = onOff; }
	bool getPoweredAngMotor() { return m_poweredAngMotor; }
	void setTargetAngMotorVelocity(Scalar targetAngMotorVelocity) { m_targetAngMotorVelocity = targetAngMotorVelocity; }
	Scalar getTargetAngMotorVelocity() { return m_targetAngMotorVelocity; }
	void setMaxAngMotorForce(Scalar maxAngMotorForce) { m_maxAngMotorForce = maxAngMotorForce; }
	Scalar getMaxAngMotorForce() { return m_maxAngMotorForce; }

	Scalar getLinearPos() const { return m_linPos; }
	Scalar getAngularPos() const { return m_angPos; }

	// access for ODE solver
	bool getSolveLinLimit() { return m_solveLinLim; }
	Scalar getLinDepth() { return m_depth[0]; }
	bool getSolveAngLimit() { return m_solveAngLim; }
	Scalar getAngDepth() { return m_angDepth; }
	// shared code used by ODE solver
	void calculateTransforms(const Transform2& transA, const Transform2& transB);
	void testLinLimits();
	void testAngLimits();
	// access for PE Solver
	Vec3 getAncorInA();
	Vec3 getAncorInB();
	// access for UseFrameOffset
	bool getUseFrameOffset() { return m_useOffsetForConstraintFrame; }
	void setUseFrameOffset(bool frameOffsetOnOff) { m_useOffsetForConstraintFrame = frameOffsetOnOff; }

	void setFrames(const Transform2& frameA, const Transform2& frameB)
	{
		m_frameInA = frameA;
		m_frameInB = frameB;
		calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
		buildJacobian();
	}

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

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64

struct SliderConstraintData
{
	TypedConstraintData m_typeConstraintData;
	Transform2FloatData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2FloatData m_rbBFrame;

	float m_linearUpperLimit;
	float m_linearLowerLimit;

	float m_angularUpperLimit;
	float m_angularLowerLimit;

	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

struct SliderConstraintDoubleData
{
	TypedConstraintDoubleData m_typeConstraintData;
	Transform2DoubleData m_rbAFrame;  // constraint axii. Assumes z is hinge axis.
	Transform2DoubleData m_rbBFrame;

	double m_linearUpperLimit;
	double m_linearLowerLimit;

	double m_angularUpperLimit;
	double m_angularLowerLimit;

	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

SIMD_FORCE_INLINE i32 SliderConstraint::calculateSerializeBufferSize() const
{
	return sizeof(SliderConstraintData2);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk SliderConstraint::serialize(uk dataBuffer, Serializer* serializer) const
{
	SliderConstraintData2* sliderData = (SliderConstraintData2*)dataBuffer;
	TypedConstraint::serialize(&sliderData->m_typeConstraintData, serializer);

	m_frameInA.serialize(sliderData->m_rbAFrame);
	m_frameInB.serialize(sliderData->m_rbBFrame);

	sliderData->m_linearUpperLimit = m_upperLinLimit;
	sliderData->m_linearLowerLimit = m_lowerLinLimit;

	sliderData->m_angularUpperLimit = m_upperAngLimit;
	sliderData->m_angularLowerLimit = m_lowerAngLimit;

	sliderData->m_useLinearReferenceFrameA = m_useLinearReferenceFrameA;
	sliderData->m_useOffsetForConstraintFrame = m_useOffsetForConstraintFrame;

	return SliderConstraintDataName;
}

#endif  //DRX3D_SLIDER_CONSTRAINT_H
