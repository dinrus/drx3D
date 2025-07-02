#include <drx3D/Physics/Dynamics/ConstraintSolver/HingeConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <new>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SolverBody.h>

//#define HINGE_USE_OBSOLETE_SOLVER false
#define HINGE_USE_OBSOLETE_SOLVER false

#define HINGE_USE_FRAME_OFFSET true

#ifndef __SPU__

HingeConstraint::HingeConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& pivotInA, const Vec3& pivotInB,
									 const Vec3& axisInA, const Vec3& axisInB, bool useReferenceFrameA)
	: TypedConstraint(HINGE_CONSTRAINT_TYPE, rbA, rbB),
#ifdef _DRX3D_USE_CENTER_LIMIT_
	  m_limit(),
#endif
	  m_angularOnly(false),
	  m_enableAngularMotor(false),
	  m_useSolveConstraintObsolete(HINGE_USE_OBSOLETE_SOLVER),
	  m_useOffsetForConstraintFrame(HINGE_USE_FRAME_OFFSET),
	  m_useReferenceFrameA(useReferenceFrameA),
	  m_flags(0),
	  m_normalCFM(0),
	  m_normalERP(0),
	  m_stopCFM(0),
	  m_stopERP(0)
{
	m_rbAFrame.getOrigin() = pivotInA;

	// since no frame is given, assume this to be zero angle and just pick rb transform axis
	Vec3 rbAxisA1 = rbA.getCenterOfMassTransform().getBasis().getColumn(0);

	Vec3 rbAxisA2;
	Scalar projection = axisInA.dot(rbAxisA1);
	if (projection >= 1.0f - SIMD_EPSILON)
	{
		rbAxisA1 = -rbA.getCenterOfMassTransform().getBasis().getColumn(2);
		rbAxisA2 = rbA.getCenterOfMassTransform().getBasis().getColumn(1);
	}
	else if (projection <= -1.0f + SIMD_EPSILON)
	{
		rbAxisA1 = rbA.getCenterOfMassTransform().getBasis().getColumn(2);
		rbAxisA2 = rbA.getCenterOfMassTransform().getBasis().getColumn(1);
	}
	else
	{
		rbAxisA2 = axisInA.cross(rbAxisA1);
		rbAxisA1 = rbAxisA2.cross(axisInA);
	}

	m_rbAFrame.getBasis().setVal(rbAxisA1.getX(), rbAxisA2.getX(), axisInA.getX(),
								   rbAxisA1.getY(), rbAxisA2.getY(), axisInA.getY(),
								   rbAxisA1.getZ(), rbAxisA2.getZ(), axisInA.getZ());

	Quat rotationArc = shortestArcQuat(axisInA, axisInB);
	Vec3 rbAxisB1 = quatRotate(rotationArc, rbAxisA1);
	Vec3 rbAxisB2 = axisInB.cross(rbAxisB1);

	m_rbBFrame.getOrigin() = pivotInB;
	m_rbBFrame.getBasis().setVal(rbAxisB1.getX(), rbAxisB2.getX(), axisInB.getX(),
								   rbAxisB1.getY(), rbAxisB2.getY(), axisInB.getY(),
								   rbAxisB1.getZ(), rbAxisB2.getZ(), axisInB.getZ());

#ifndef _DRX3D_USE_CENTER_LIMIT_
	//start with free
	m_lowerLimit = Scalar(1.0f);
	m_upperLimit = Scalar(-1.0f);
	m_biasFactor = 0.3f;
	m_relaxationFactor = 1.0f;
	m_limitSoftness = 0.9f;
	m_solveLimit = false;
#endif
	m_referenceSign = m_useReferenceFrameA ? Scalar(-1.f) : Scalar(1.f);
}

HingeConstraint::HingeConstraint(RigidBody& rbA, const Vec3& pivotInA, const Vec3& axisInA, bool useReferenceFrameA)
	: TypedConstraint(HINGE_CONSTRAINT_TYPE, rbA),
#ifdef _DRX3D_USE_CENTER_LIMIT_
	  m_limit(),
#endif
	  m_angularOnly(false),
	  m_enableAngularMotor(false),
	  m_useSolveConstraintObsolete(HINGE_USE_OBSOLETE_SOLVER),
	  m_useOffsetForConstraintFrame(HINGE_USE_FRAME_OFFSET),
	  m_useReferenceFrameA(useReferenceFrameA),
	  m_flags(0),
	  m_normalCFM(0),
	  m_normalERP(0),
	  m_stopCFM(0),
	  m_stopERP(0)
{
	// since no frame is given, assume this to be zero angle and just pick rb transform axis
	// fixed axis in worldspace
	Vec3 rbAxisA1, rbAxisA2;
	PlaneSpace1(axisInA, rbAxisA1, rbAxisA2);

	m_rbAFrame.getOrigin() = pivotInA;
	m_rbAFrame.getBasis().setVal(rbAxisA1.getX(), rbAxisA2.getX(), axisInA.getX(),
								   rbAxisA1.getY(), rbAxisA2.getY(), axisInA.getY(),
								   rbAxisA1.getZ(), rbAxisA2.getZ(), axisInA.getZ());

	Vec3 axisInB = rbA.getCenterOfMassTransform().getBasis() * axisInA;

	Quat rotationArc = shortestArcQuat(axisInA, axisInB);
	Vec3 rbAxisB1 = quatRotate(rotationArc, rbAxisA1);
	Vec3 rbAxisB2 = axisInB.cross(rbAxisB1);

	m_rbBFrame.getOrigin() = rbA.getCenterOfMassTransform()(pivotInA);
	m_rbBFrame.getBasis().setVal(rbAxisB1.getX(), rbAxisB2.getX(), axisInB.getX(),
								   rbAxisB1.getY(), rbAxisB2.getY(), axisInB.getY(),
								   rbAxisB1.getZ(), rbAxisB2.getZ(), axisInB.getZ());

#ifndef _DRX3D_USE_CENTER_LIMIT_
	//start with free
	m_lowerLimit = Scalar(1.0f);
	m_upperLimit = Scalar(-1.0f);
	m_biasFactor = 0.3f;
	m_relaxationFactor = 1.0f;
	m_limitSoftness = 0.9f;
	m_solveLimit = false;
#endif
	m_referenceSign = m_useReferenceFrameA ? Scalar(-1.f) : Scalar(1.f);
}

HingeConstraint::HingeConstraint(RigidBody& rbA, RigidBody& rbB,
									 const Transform2& rbAFrame, const Transform2& rbBFrame, bool useReferenceFrameA)
	: TypedConstraint(HINGE_CONSTRAINT_TYPE, rbA, rbB), m_rbAFrame(rbAFrame), m_rbBFrame(rbBFrame),
#ifdef _DRX3D_USE_CENTER_LIMIT_
	  m_limit(),
#endif
	  m_angularOnly(false),
	  m_enableAngularMotor(false),
	  m_useSolveConstraintObsolete(HINGE_USE_OBSOLETE_SOLVER),
	  m_useOffsetForConstraintFrame(HINGE_USE_FRAME_OFFSET),
	  m_useReferenceFrameA(useReferenceFrameA),
	  m_flags(0),
	  m_normalCFM(0),
	  m_normalERP(0),
	  m_stopCFM(0),
	  m_stopERP(0)
{
#ifndef _DRX3D_USE_CENTER_LIMIT_
	//start with free
	m_lowerLimit = Scalar(1.0f);
	m_upperLimit = Scalar(-1.0f);
	m_biasFactor = 0.3f;
	m_relaxationFactor = 1.0f;
	m_limitSoftness = 0.9f;
	m_solveLimit = false;
#endif
	m_referenceSign = m_useReferenceFrameA ? Scalar(-1.f) : Scalar(1.f);
}

HingeConstraint::HingeConstraint(RigidBody& rbA, const Transform2& rbAFrame, bool useReferenceFrameA)
	: TypedConstraint(HINGE_CONSTRAINT_TYPE, rbA), m_rbAFrame(rbAFrame), m_rbBFrame(rbAFrame),
#ifdef _DRX3D_USE_CENTER_LIMIT_
	  m_limit(),
#endif
	  m_angularOnly(false),
	  m_enableAngularMotor(false),
	  m_useSolveConstraintObsolete(HINGE_USE_OBSOLETE_SOLVER),
	  m_useOffsetForConstraintFrame(HINGE_USE_FRAME_OFFSET),
	  m_useReferenceFrameA(useReferenceFrameA),
	  m_flags(0),
	  m_normalCFM(0),
	  m_normalERP(0),
	  m_stopCFM(0),
	  m_stopERP(0)
{
	///not providing rigidbody B means implicitly using worldspace for body B

	m_rbBFrame.getOrigin() = m_rbA.getCenterOfMassTransform()(m_rbAFrame.getOrigin());
#ifndef _DRX3D_USE_CENTER_LIMIT_
	//start with free
	m_lowerLimit = Scalar(1.0f);
	m_upperLimit = Scalar(-1.0f);
	m_biasFactor = 0.3f;
	m_relaxationFactor = 1.0f;
	m_limitSoftness = 0.9f;
	m_solveLimit = false;
#endif
	m_referenceSign = m_useReferenceFrameA ? Scalar(-1.f) : Scalar(1.f);
}

void HingeConstraint::buildJacobian()
{
	if (m_useSolveConstraintObsolete)
	{
		m_appliedImpulse = Scalar(0.);
		m_accMotorImpulse = Scalar(0.);

		if (!m_angularOnly)
		{
			Vec3 pivotAInW = m_rbA.getCenterOfMassTransform() * m_rbAFrame.getOrigin();
			Vec3 pivotBInW = m_rbB.getCenterOfMassTransform() * m_rbBFrame.getOrigin();
			Vec3 relPos = pivotBInW - pivotAInW;

			Vec3 normal[3];
			if (relPos.length2() > SIMD_EPSILON)
			{
				normal[0] = relPos.normalized();
			}
			else
			{
				normal[0].setVal(Scalar(1.0), 0, 0);
			}

			PlaneSpace1(normal[0], normal[1], normal[2]);

			for (i32 i = 0; i < 3; i++)
			{
				new (&m_jac[i]) JacobianEntry(
					m_rbA.getCenterOfMassTransform().getBasis().transpose(),
					m_rbB.getCenterOfMassTransform().getBasis().transpose(),
					pivotAInW - m_rbA.getCenterOfMassPosition(),
					pivotBInW - m_rbB.getCenterOfMassPosition(),
					normal[i],
					m_rbA.getInvInertiaDiagLocal(),
					m_rbA.getInvMass(),
					m_rbB.getInvInertiaDiagLocal(),
					m_rbB.getInvMass());
			}
		}

		//calculate two perpendicular jointAxis, orthogonal to hingeAxis
		//these two jointAxis require equal angular velocities for both bodies

		//this is unused for now, it's a todo
		Vec3 jointAxis0local;
		Vec3 jointAxis1local;

		PlaneSpace1(m_rbAFrame.getBasis().getColumn(2), jointAxis0local, jointAxis1local);

		Vec3 jointAxis0 = getRigidBodyA().getCenterOfMassTransform().getBasis() * jointAxis0local;
		Vec3 jointAxis1 = getRigidBodyA().getCenterOfMassTransform().getBasis() * jointAxis1local;
		Vec3 hingeAxisWorld = getRigidBodyA().getCenterOfMassTransform().getBasis() * m_rbAFrame.getBasis().getColumn(2);

		new (&m_jacAng[0]) JacobianEntry(jointAxis0,
										   m_rbA.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbB.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbA.getInvInertiaDiagLocal(),
										   m_rbB.getInvInertiaDiagLocal());

		new (&m_jacAng[1]) JacobianEntry(jointAxis1,
										   m_rbA.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbB.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbA.getInvInertiaDiagLocal(),
										   m_rbB.getInvInertiaDiagLocal());

		new (&m_jacAng[2]) JacobianEntry(hingeAxisWorld,
										   m_rbA.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbB.getCenterOfMassTransform().getBasis().transpose(),
										   m_rbA.getInvInertiaDiagLocal(),
										   m_rbB.getInvInertiaDiagLocal());

		// clear accumulator
		m_accLimitImpulse = Scalar(0.);

		// test angular limit
		testLimit(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());

		//Compute K = J*W*J' for hinge axis
		Vec3 axisA = getRigidBodyA().getCenterOfMassTransform().getBasis() * m_rbAFrame.getBasis().getColumn(2);
		m_kHinge = 1.0f / (getRigidBodyA().computeAngularImpulseDenominator(axisA) +
						   getRigidBodyB().computeAngularImpulseDenominator(axisA));
	}
}

#endif  //__SPU__

static inline Scalar NormalizeAnglePositive(Scalar angle)
{
	return Fmod(Fmod(angle, Scalar(2.0 * SIMD_PI)) + Scalar(2.0 * SIMD_PI), Scalar(2.0 * SIMD_PI));
}

static Scalar ShortestAngularDistance(Scalar accAngle, Scalar curAngle)
{
	Scalar result = NormalizeAngle(NormalizeAnglePositive(NormalizeAnglePositive(curAngle) -
																NormalizeAnglePositive(accAngle)));
	return result;
}

static Scalar ShortestAngleUpdate(Scalar accAngle, Scalar curAngle)
{
	Scalar tol(0.3);
	Scalar result = ShortestAngularDistance(accAngle, curAngle);

	if (Fabs(result) > tol)
		return curAngle;
	else
		return accAngle + result;

	return curAngle;
}

Scalar HingeAccumulatedAngleConstraint::getAccumulatedHingeAngle()
{
	Scalar hingeAngle = getHingeAngle();
	m_accumulatedAngle = ShortestAngleUpdate(m_accumulatedAngle, hingeAngle);
	return m_accumulatedAngle;
}
void HingeAccumulatedAngleConstraint::setAccumulatedHingeAngle(Scalar accAngle)
{
	m_accumulatedAngle = accAngle;
}

void HingeAccumulatedAngleConstraint::getInfo1(ConstraintInfo1* info)
{
	//update m_accumulatedAngle
	Scalar curHingeAngle = getHingeAngle();
	m_accumulatedAngle = ShortestAngleUpdate(m_accumulatedAngle, curHingeAngle);

	HingeConstraint::getInfo1(info);
}

void HingeConstraint::getInfo1(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		info->m_numConstraintRows = 5;  // Fixed 3 linear + 2 angular
		info->nub = 1;
		//always add the row, to avoid computation (data is not available yet)
		//prepare constraint
		testLimit(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
		if (getSolveLimit() || getEnableAngularMotor())
		{
			info->m_numConstraintRows++;  // limit 3rd anguar as well
			info->nub--;
		}
	}
}

void HingeConstraint::getInfo1NonVirtual(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		//always add the 'limit' row, to avoid computation (data is not available yet)
		info->m_numConstraintRows = 6;  // Fixed 3 linear + 2 angular
		info->nub = 0;
	}
}

void HingeConstraint::getInfo2(ConstraintInfo2* info)
{
	if (m_useOffsetForConstraintFrame)
	{
		getInfo2InternalUsingFrameOffset(info, m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform(), m_rbA.getAngularVelocity(), m_rbB.getAngularVelocity());
	}
	else
	{
		getInfo2Internal(info, m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform(), m_rbA.getAngularVelocity(), m_rbB.getAngularVelocity());
	}
}

void HingeConstraint::getInfo2NonVirtual(ConstraintInfo2* info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB)
{
	///the regular (virtual) implementation getInfo2 already performs 'testLimit' during getInfo1, so we need to do it now
	testLimit(transA, transB);

	getInfo2Internal(info, transA, transB, angVelA, angVelB);
}

void HingeConstraint::getInfo2Internal(ConstraintInfo2* info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB)
{
	Assert(!m_useSolveConstraintObsolete);
	i32 i, skip = info->rowskip;
	// transforms in world space
	Transform2 trA = transA * m_rbAFrame;
	Transform2 trB = transB * m_rbBFrame;
	// pivot point
	Vec3 pivotAInW = trA.getOrigin();
	Vec3 pivotBInW = trB.getOrigin();
#if 0
	if (0)
	{
		for (i=0;i<6;i++)
		{
			info->m_J1linearAxis[i*skip]=0;
			info->m_J1linearAxis[i*skip+1]=0;
			info->m_J1linearAxis[i*skip+2]=0;

			info->m_J1angularAxis[i*skip]=0;
			info->m_J1angularAxis[i*skip+1]=0;
			info->m_J1angularAxis[i*skip+2]=0;

			info->m_J2linearAxis[i*skip]=0;
			info->m_J2linearAxis[i*skip+1]=0;
			info->m_J2linearAxis[i*skip+2]=0;

			info->m_J2angularAxis[i*skip]=0;
			info->m_J2angularAxis[i*skip+1]=0;
			info->m_J2angularAxis[i*skip+2]=0;

			info->m_constraintError[i*skip]=0.f;
		}
	}
#endif  //#if 0
	// linear (all fixed)

	if (!m_angularOnly)
	{
		info->m_J1linearAxis[0] = 1;
		info->m_J1linearAxis[skip + 1] = 1;
		info->m_J1linearAxis[2 * skip + 2] = 1;

		info->m_J2linearAxis[0] = -1;
		info->m_J2linearAxis[skip + 1] = -1;
		info->m_J2linearAxis[2 * skip + 2] = -1;
	}

	Vec3 a1 = pivotAInW - transA.getOrigin();
	{
		Vec3* angular0 = (Vec3*)(info->m_J1angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J1angularAxis + skip);
		Vec3* angular2 = (Vec3*)(info->m_J1angularAxis + 2 * skip);
		Vec3 a1neg = -a1;
		a1neg.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}
	Vec3 a2 = pivotBInW - transB.getOrigin();
	{
		Vec3* angular0 = (Vec3*)(info->m_J2angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J2angularAxis + skip);
		Vec3* angular2 = (Vec3*)(info->m_J2angularAxis + 2 * skip);
		a2.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}
	// linear RHS
	Scalar normalErp = (m_flags & DRX3D_HINGE_FLAGS_ERP_NORM) ? m_normalERP : info->erp;

	Scalar k = info->fps * normalErp;
	if (!m_angularOnly)
	{
		for (i = 0; i < 3; i++)
		{
			info->m_constraintError[i * skip] = k * (pivotBInW[i] - pivotAInW[i]);
		}
	}
	// make rotations around X and Y equal
	// the hinge axis should be the only unconstrained
	// rotational axis, the angular velocity of the two bodies perpendicular to
	// the hinge axis should be equal. thus the constraint equations are
	//    p*w1 - p*w2 = 0
	//    q*w1 - q*w2 = 0
	// where p and q are unit vectors normal to the hinge axis, and w1 and w2
	// are the angular velocity vectors of the two bodies.
	// get hinge axis (Z)
	Vec3 ax1 = trA.getBasis().getColumn(2);
	// get 2 orthos to hinge axis (X, Y)
	Vec3 p = trA.getBasis().getColumn(0);
	Vec3 q = trA.getBasis().getColumn(1);
	// set the two hinge angular rows
	i32 s3 = 3 * info->rowskip;
	i32 s4 = 4 * info->rowskip;

	info->m_J1angularAxis[s3 + 0] = p[0];
	info->m_J1angularAxis[s3 + 1] = p[1];
	info->m_J1angularAxis[s3 + 2] = p[2];
	info->m_J1angularAxis[s4 + 0] = q[0];
	info->m_J1angularAxis[s4 + 1] = q[1];
	info->m_J1angularAxis[s4 + 2] = q[2];

	info->m_J2angularAxis[s3 + 0] = -p[0];
	info->m_J2angularAxis[s3 + 1] = -p[1];
	info->m_J2angularAxis[s3 + 2] = -p[2];
	info->m_J2angularAxis[s4 + 0] = -q[0];
	info->m_J2angularAxis[s4 + 1] = -q[1];
	info->m_J2angularAxis[s4 + 2] = -q[2];
	// compute the right hand side of the constraint equation. set relative
	// body velocities along p and q to bring the hinge back into alignment.
	// if ax1,ax2 are the unit length hinge axes as computed from body1 and
	// body2, we need to rotate both bodies along the axis u = (ax1 x ax2).
	// if `theta' is the angle between ax1 and ax2, we need an angular velocity
	// along u to cover angle erp*theta in one step :
	//   |angular_velocity| = angle/time = erp*theta / stepsize
	//                      = (erp*fps) * theta
	//    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
	//                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
	// ...as ax1 and ax2 are unit length. if theta is smallish,
	// theta ~= sin(theta), so
	//    angular_velocity  = (erp*fps) * (ax1 x ax2)
	// ax1 x ax2 is in the plane space of ax1, so we project the angular
	// velocity to p and q to find the right hand side.
	Vec3 ax2 = trB.getBasis().getColumn(2);
	Vec3 u = ax1.cross(ax2);
	info->m_constraintError[s3] = k * u.dot(p);
	info->m_constraintError[s4] = k * u.dot(q);
	// check angular limits
	i32 nrow = 4;  // last filled row
	i32 srow;
	Scalar limit_err = Scalar(0.0);
	i32 limit = 0;
	if (getSolveLimit())
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		limit_err = m_limit.getCorrection() * m_referenceSign;
#else
		limit_err = m_correction * m_referenceSign;
#endif
		limit = (limit_err > Scalar(0.0)) ? 1 : 2;
	}
	// if the hinge has joint limits or motor, add in the extra row
	bool powered = getEnableAngularMotor();
	if (limit || powered)
	{
		nrow++;
		srow = nrow * info->rowskip;
		info->m_J1angularAxis[srow + 0] = ax1[0];
		info->m_J1angularAxis[srow + 1] = ax1[1];
		info->m_J1angularAxis[srow + 2] = ax1[2];

		info->m_J2angularAxis[srow + 0] = -ax1[0];
		info->m_J2angularAxis[srow + 1] = -ax1[1];
		info->m_J2angularAxis[srow + 2] = -ax1[2];

		Scalar lostop = getLowerLimit();
		Scalar histop = getUpperLimit();
		if (limit && (lostop == histop))
		{  // the joint motor is ineffective
			powered = false;
		}
		info->m_constraintError[srow] = Scalar(0.0f);
		Scalar currERP = (m_flags & DRX3D_HINGE_FLAGS_ERP_STOP) ? m_stopERP : normalErp;
		if (powered)
		{
			if (m_flags & DRX3D_HINGE_FLAGS_CFM_NORM)
			{
				info->cfm[srow] = m_normalCFM;
			}
			Scalar mot_fact = getMotorFactor(m_hingeAngle, lostop, histop, m_motorTargetVelocity, info->fps * currERP);
			info->m_constraintError[srow] += mot_fact * m_motorTargetVelocity * m_referenceSign;
			info->m_lowerLimit[srow] = -m_maxMotorImpulse;
			info->m_upperLimit[srow] = m_maxMotorImpulse;
		}
		if (limit)
		{
			k = info->fps * currERP;
			info->m_constraintError[srow] += k * limit_err;
			if (m_flags & DRX3D_HINGE_FLAGS_CFM_STOP)
			{
				info->cfm[srow] = m_stopCFM;
			}
			if (lostop == histop)
			{
				// limited low and high simultaneously
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else if (limit == 1)
			{  // low limit
				info->m_lowerLimit[srow] = 0;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else
			{  // high limit
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = 0;
			}
			// bounce (we'll use slider parameter abs(1.0 - m_dampingLimAng) for that)
#ifdef _DRX3D_USE_CENTER_LIMIT_
			Scalar bounce = m_limit.getRelaxationFactor();
#else
			Scalar bounce = m_relaxationFactor;
#endif
			if (bounce > Scalar(0.0))
			{
				Scalar vel = angVelA.dot(ax1);
				vel -= angVelB.dot(ax1);
				// only apply bounce if the velocity is incoming, and if the
				// resulting c[] exceeds what we already have.
				if (limit == 1)
				{  // low limit
					if (vel < 0)
					{
						Scalar newc = -bounce * vel;
						if (newc > info->m_constraintError[srow])
						{
							info->m_constraintError[srow] = newc;
						}
					}
				}
				else
				{  // high limit - all those computations are reversed
					if (vel > 0)
					{
						Scalar newc = -bounce * vel;
						if (newc < info->m_constraintError[srow])
						{
							info->m_constraintError[srow] = newc;
						}
					}
				}
			}
#ifdef _DRX3D_USE_CENTER_LIMIT_
			info->m_constraintError[srow] *= m_limit.getBiasFactor();
#else
			info->m_constraintError[srow] *= m_biasFactor;
#endif
		}  // if(limit)
	}      // if angular limit or powered
}

void HingeConstraint::setFrames(const Transform2& frameA, const Transform2& frameB)
{
	m_rbAFrame = frameA;
	m_rbBFrame = frameB;
	buildJacobian();
}

void HingeConstraint::updateRHS(Scalar timeStep)
{
	(void)timeStep;
}

Scalar HingeConstraint::getHingeAngle()
{
	return getHingeAngle(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
}

Scalar HingeConstraint::getHingeAngle(const Transform2& transA, const Transform2& transB)
{
	const Vec3 refAxis0 = transA.getBasis() * m_rbAFrame.getBasis().getColumn(0);
	const Vec3 refAxis1 = transA.getBasis() * m_rbAFrame.getBasis().getColumn(1);
	const Vec3 swingAxis = transB.getBasis() * m_rbBFrame.getBasis().getColumn(1);
	//	Scalar angle = Atan2Fast(swingAxis.dot(refAxis0), swingAxis.dot(refAxis1));
	Scalar angle = Atan2(swingAxis.dot(refAxis0), swingAxis.dot(refAxis1));
	return m_referenceSign * angle;
}

void HingeConstraint::testLimit(const Transform2& transA, const Transform2& transB)
{
	// Compute limit information
	m_hingeAngle = getHingeAngle(transA, transB);
#ifdef _DRX3D_USE_CENTER_LIMIT_
	m_limit.test(m_hingeAngle);
#else
	m_correction = Scalar(0.);
	m_limitSign = Scalar(0.);
	m_solveLimit = false;
	if (m_lowerLimit <= m_upperLimit)
	{
		m_hingeAngle = AdjustAngleToLimits(m_hingeAngle, m_lowerLimit, m_upperLimit);
		if (m_hingeAngle <= m_lowerLimit)
		{
			m_correction = (m_lowerLimit - m_hingeAngle);
			m_limitSign = 1.0f;
			m_solveLimit = true;
		}
		else if (m_hingeAngle >= m_upperLimit)
		{
			m_correction = m_upperLimit - m_hingeAngle;
			m_limitSign = -1.0f;
			m_solveLimit = true;
		}
	}
#endif
	return;
}

static Vec3 vHinge(0, 0, Scalar(1));

void HingeConstraint::setMotorTarget(const Quat& qAinB, Scalar dt)
{
	// convert target from body to constraint space
	Quat qConstraint = m_rbBFrame.getRotation().inverse() * qAinB * m_rbAFrame.getRotation();
	qConstraint.normalize();

	// extract "pure" hinge component
	Vec3 vNoHinge = quatRotate(qConstraint, vHinge);
	vNoHinge.normalize();
	Quat qNoHinge = shortestArcQuat(vHinge, vNoHinge);
	Quat qHinge = qNoHinge.inverse() * qConstraint;
	qHinge.normalize();

	// compute angular target, clamped to limits
	Scalar targetAngle = qHinge.getAngle();
	if (targetAngle > SIMD_PI)  // long way around. flip quat and recalculate.
	{
		qHinge = -(qHinge);
		targetAngle = qHinge.getAngle();
	}
	if (qHinge.getZ() < 0)
		targetAngle = -targetAngle;

	setMotorTarget(targetAngle, dt);
}

void HingeConstraint::setMotorTarget(Scalar targetAngle, Scalar dt)
{
#ifdef _DRX3D_USE_CENTER_LIMIT_
	m_limit.fit(targetAngle);
#else
	if (m_lowerLimit < m_upperLimit)
	{
		if (targetAngle < m_lowerLimit)
			targetAngle = m_lowerLimit;
		else if (targetAngle > m_upperLimit)
			targetAngle = m_upperLimit;
	}
#endif
	// compute angular velocity
	Scalar curAngle = getHingeAngle(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
	Scalar dAngle = targetAngle - curAngle;
	m_motorTargetVelocity = dAngle / dt;
}

void HingeConstraint::getInfo2InternalUsingFrameOffset(ConstraintInfo2* info, const Transform2& transA, const Transform2& transB, const Vec3& angVelA, const Vec3& angVelB)
{
	Assert(!m_useSolveConstraintObsolete);
	i32 i, s = info->rowskip;
	// transforms in world space
	Transform2 trA = transA * m_rbAFrame;
	Transform2 trB = transB * m_rbBFrame;
	// pivot point
//	Vec3 pivotAInW = trA.getOrigin();
//	Vec3 pivotBInW = trB.getOrigin();
#if 1
	// difference between frames in WCS
	Vec3 ofs = trB.getOrigin() - trA.getOrigin();
	// now get weight factors depending on masses
	Scalar miA = getRigidBodyA().getInvMass();
	Scalar miB = getRigidBodyB().getInvMass();
	bool hasStaticBody = (miA < SIMD_EPSILON) || (miB < SIMD_EPSILON);
	Scalar miS = miA + miB;
	Scalar factA, factB;
	if (miS > Scalar(0.f))
	{
		factA = miB / miS;
	}
	else
	{
		factA = Scalar(0.5f);
	}
	factB = Scalar(1.0f) - factA;
	// get the desired direction of hinge axis
	// as weighted sum of Z-orthos of frameA and frameB in WCS
	Vec3 ax1A = trA.getBasis().getColumn(2);
	Vec3 ax1B = trB.getBasis().getColumn(2);
	Vec3 ax1 = ax1A * factA + ax1B * factB;
	if (ax1.length2()<SIMD_EPSILON)
	{
		factA=0.f;
		factB=1.f;
		ax1 = ax1A * factA + ax1B * factB;
	}
	ax1.normalize();
	// fill first 3 rows
	// we want: velA + wA x relA == velB + wB x relB
	Transform2 bodyA_trans = transA;
	Transform2 bodyB_trans = transB;
	i32 s0 = 0;
	i32 s1 = s;
	i32 s2 = s * 2;
	i32 nrow = 2;  // last filled row
	Vec3 tmpA, tmpB, relA, relB, p, q;
	// get vector from bodyB to frameB in WCS
	relB = trB.getOrigin() - bodyB_trans.getOrigin();
	// get its projection to hinge axis
	Vec3 projB = ax1 * relB.dot(ax1);
	// get vector directed from bodyB to hinge axis (and orthogonal to it)
	Vec3 orthoB = relB - projB;
	// same for bodyA
	relA = trA.getOrigin() - bodyA_trans.getOrigin();
	Vec3 projA = ax1 * relA.dot(ax1);
	Vec3 orthoA = relA - projA;
	Vec3 totalDist = projA - projB;
	// get offset vectors relA and relB
	relA = orthoA + totalDist * factA;
	relB = orthoB - totalDist * factB;
	// now choose average ortho to hinge axis
	p = orthoB * factA + orthoA * factB;
	Scalar len2 = p.length2();
	if (len2 > SIMD_EPSILON)
	{
		p /= Sqrt(len2);
	}
	else
	{
		p = trA.getBasis().getColumn(1);
	}
	// make one more ortho
	q = ax1.cross(p);
	// fill three rows
	tmpA = relA.cross(p);
	tmpB = relB.cross(p);
	for (i = 0; i < 3; i++) info->m_J1angularAxis[s0 + i] = tmpA[i];
	for (i = 0; i < 3; i++) info->m_J2angularAxis[s0 + i] = -tmpB[i];
	tmpA = relA.cross(q);
	tmpB = relB.cross(q);
	if (hasStaticBody && getSolveLimit())
	{  // to make constraint between static and dynamic objects more rigid
		// remove wA (or wB) from equation if angular limit is hit
		tmpB *= factB;
		tmpA *= factA;
	}
	for (i = 0; i < 3; i++) info->m_J1angularAxis[s1 + i] = tmpA[i];
	for (i = 0; i < 3; i++) info->m_J2angularAxis[s1 + i] = -tmpB[i];
	tmpA = relA.cross(ax1);
	tmpB = relB.cross(ax1);
	if (hasStaticBody)
	{  // to make constraint between static and dynamic objects more rigid
		// remove wA (or wB) from equation
		tmpB *= factB;
		tmpA *= factA;
	}
	for (i = 0; i < 3; i++) info->m_J1angularAxis[s2 + i] = tmpA[i];
	for (i = 0; i < 3; i++) info->m_J2angularAxis[s2 + i] = -tmpB[i];

	Scalar normalErp = (m_flags & DRX3D_HINGE_FLAGS_ERP_NORM) ? m_normalERP : info->erp;
	Scalar k = info->fps * normalErp;

	if (!m_angularOnly)
	{
		for (i = 0; i < 3; i++) info->m_J1linearAxis[s0 + i] = p[i];
		for (i = 0; i < 3; i++) info->m_J1linearAxis[s1 + i] = q[i];
		for (i = 0; i < 3; i++) info->m_J1linearAxis[s2 + i] = ax1[i];

		for (i = 0; i < 3; i++) info->m_J2linearAxis[s0 + i] = -p[i];
		for (i = 0; i < 3; i++) info->m_J2linearAxis[s1 + i] = -q[i];
		for (i = 0; i < 3; i++) info->m_J2linearAxis[s2 + i] = -ax1[i];

		// compute three elements of right hand side

		Scalar rhs = k * p.dot(ofs);
		info->m_constraintError[s0] = rhs;
		rhs = k * q.dot(ofs);
		info->m_constraintError[s1] = rhs;
		rhs = k * ax1.dot(ofs);
		info->m_constraintError[s2] = rhs;
	}
	// the hinge axis should be the only unconstrained
	// rotational axis, the angular velocity of the two bodies perpendicular to
	// the hinge axis should be equal. thus the constraint equations are
	//    p*w1 - p*w2 = 0
	//    q*w1 - q*w2 = 0
	// where p and q are unit vectors normal to the hinge axis, and w1 and w2
	// are the angular velocity vectors of the two bodies.
	i32 s3 = 3 * s;
	i32 s4 = 4 * s;
	info->m_J1angularAxis[s3 + 0] = p[0];
	info->m_J1angularAxis[s3 + 1] = p[1];
	info->m_J1angularAxis[s3 + 2] = p[2];
	info->m_J1angularAxis[s4 + 0] = q[0];
	info->m_J1angularAxis[s4 + 1] = q[1];
	info->m_J1angularAxis[s4 + 2] = q[2];

	info->m_J2angularAxis[s3 + 0] = -p[0];
	info->m_J2angularAxis[s3 + 1] = -p[1];
	info->m_J2angularAxis[s3 + 2] = -p[2];
	info->m_J2angularAxis[s4 + 0] = -q[0];
	info->m_J2angularAxis[s4 + 1] = -q[1];
	info->m_J2angularAxis[s4 + 2] = -q[2];
	// compute the right hand side of the constraint equation. set relative
	// body velocities along p and q to bring the hinge back into alignment.
	// if ax1A,ax1B are the unit length hinge axes as computed from bodyA and
	// bodyB, we need to rotate both bodies along the axis u = (ax1 x ax2).
	// if "theta" is the angle between ax1 and ax2, we need an angular velocity
	// along u to cover angle erp*theta in one step :
	//   |angular_velocity| = angle/time = erp*theta / stepsize
	//                      = (erp*fps) * theta
	//    angular_velocity  = |angular_velocity| * (ax1 x ax2) / |ax1 x ax2|
	//                      = (erp*fps) * theta * (ax1 x ax2) / sin(theta)
	// ...as ax1 and ax2 are unit length. if theta is smallish,
	// theta ~= sin(theta), so
	//    angular_velocity  = (erp*fps) * (ax1 x ax2)
	// ax1 x ax2 is in the plane space of ax1, so we project the angular
	// velocity to p and q to find the right hand side.
	k = info->fps * normalErp;  //??

	Vec3 u = ax1A.cross(ax1B);
	info->m_constraintError[s3] = k * u.dot(p);
	info->m_constraintError[s4] = k * u.dot(q);
#endif
	// check angular limits
	nrow = 4;  // last filled row
	i32 srow;
	Scalar limit_err = Scalar(0.0);
	i32 limit = 0;
	if (getSolveLimit())
	{
#ifdef _DRX3D_USE_CENTER_LIMIT_
		limit_err = m_limit.getCorrection() * m_referenceSign;
#else
		limit_err = m_correction * m_referenceSign;
#endif
		limit = (limit_err > Scalar(0.0)) ? 1 : 2;
	}
	// if the hinge has joint limits or motor, add in the extra row
	bool powered = getEnableAngularMotor();
	if (limit || powered)
	{
		nrow++;
		srow = nrow * info->rowskip;
		info->m_J1angularAxis[srow + 0] = ax1[0];
		info->m_J1angularAxis[srow + 1] = ax1[1];
		info->m_J1angularAxis[srow + 2] = ax1[2];

		info->m_J2angularAxis[srow + 0] = -ax1[0];
		info->m_J2angularAxis[srow + 1] = -ax1[1];
		info->m_J2angularAxis[srow + 2] = -ax1[2];

		Scalar lostop = getLowerLimit();
		Scalar histop = getUpperLimit();
		if (limit && (lostop == histop))
		{  // the joint motor is ineffective
			powered = false;
		}
		info->m_constraintError[srow] = Scalar(0.0f);
		Scalar currERP = (m_flags & DRX3D_HINGE_FLAGS_ERP_STOP) ? m_stopERP : normalErp;
		if (powered)
		{
			if (m_flags & DRX3D_HINGE_FLAGS_CFM_NORM)
			{
				info->cfm[srow] = m_normalCFM;
			}
			Scalar mot_fact = getMotorFactor(m_hingeAngle, lostop, histop, m_motorTargetVelocity, info->fps * currERP);
			info->m_constraintError[srow] += mot_fact * m_motorTargetVelocity * m_referenceSign;
			info->m_lowerLimit[srow] = -m_maxMotorImpulse;
			info->m_upperLimit[srow] = m_maxMotorImpulse;
		}
		if (limit)
		{
			k = info->fps * currERP;
			info->m_constraintError[srow] += k * limit_err;
			if (m_flags & DRX3D_HINGE_FLAGS_CFM_STOP)
			{
				info->cfm[srow] = m_stopCFM;
			}
			if (lostop == histop)
			{
				// limited low and high simultaneously
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else if (limit == 1)
			{  // low limit
				info->m_lowerLimit[srow] = 0;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else
			{  // high limit
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = 0;
			}
			// bounce (we'll use slider parameter abs(1.0 - m_dampingLimAng) for that)
#ifdef _DRX3D_USE_CENTER_LIMIT_
			Scalar bounce = m_limit.getRelaxationFactor();
#else
			Scalar bounce = m_relaxationFactor;
#endif
			if (bounce > Scalar(0.0))
			{
				Scalar vel = angVelA.dot(ax1);
				vel -= angVelB.dot(ax1);
				// only apply bounce if the velocity is incoming, and if the
				// resulting c[] exceeds what we already have.
				if (limit == 1)
				{  // low limit
					if (vel < 0)
					{
						Scalar newc = -bounce * vel;
						if (newc > info->m_constraintError[srow])
						{
							info->m_constraintError[srow] = newc;
						}
					}
				}
				else
				{  // high limit - all those computations are reversed
					if (vel > 0)
					{
						Scalar newc = -bounce * vel;
						if (newc < info->m_constraintError[srow])
						{
							info->m_constraintError[srow] = newc;
						}
					}
				}
			}
#ifdef _DRX3D_USE_CENTER_LIMIT_
			info->m_constraintError[srow] *= m_limit.getBiasFactor();
#else
			info->m_constraintError[srow] *= m_biasFactor;
#endif
		}  // if(limit)
	}      // if angular limit or powered
}

///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
///If no axis is provided, it uses the default axis for this constraint.
void HingeConstraint::setParam(i32 num, Scalar value, i32 axis)
{
	if ((axis == -1) || (axis == 5))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				m_stopERP = value;
				m_flags |= DRX3D_HINGE_FLAGS_ERP_STOP;
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				m_stopCFM = value;
				m_flags |= DRX3D_HINGE_FLAGS_CFM_STOP;
				break;
			case DRX3D_CONSTRAINT_CFM:
				m_normalCFM = value;
				m_flags |= DRX3D_HINGE_FLAGS_CFM_NORM;
				break;
			case DRX3D_CONSTRAINT_ERP:
				m_normalERP = value;
				m_flags |= DRX3D_HINGE_FLAGS_ERP_NORM;
				break;
			default:
				AssertConstrParams(0);
		}
	}
	else
	{
		AssertConstrParams(0);
	}
}

///return the local value of parameter
Scalar HingeConstraint::getParam(i32 num, i32 axis) const
{
	Scalar retVal = 0;
	if ((axis == -1) || (axis == 5))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				AssertConstrParams(m_flags & DRX3D_HINGE_FLAGS_ERP_STOP);
				retVal = m_stopERP;
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				AssertConstrParams(m_flags & DRX3D_HINGE_FLAGS_CFM_STOP);
				retVal = m_stopCFM;
				break;
			case DRX3D_CONSTRAINT_CFM:
				AssertConstrParams(m_flags & DRX3D_HINGE_FLAGS_CFM_NORM);
				retVal = m_normalCFM;
				break;
			case DRX3D_CONSTRAINT_ERP:
				AssertConstrParams(m_flags & DRX3D_HINGE_FLAGS_ERP_NORM);
				retVal = m_normalERP;
				break;
			default:
				AssertConstrParams(0);
		}
	}
	else
	{
		AssertConstrParams(0);
	}
	return retVal;
}
