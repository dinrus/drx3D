#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <new>

#define D6_USE_OBSOLETE_METHOD false
#define D6_USE_FRAME_OFFSET true

Generic6DofConstraint::Generic6DofConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA)
	: TypedConstraint(D6_CONSTRAINT_TYPE, rbA, rbB), m_frameInA(frameInA), m_frameInB(frameInB), m_useLinearReferenceFrameA(useLinearReferenceFrameA), m_useOffsetForConstraintFrame(D6_USE_FRAME_OFFSET), m_flags(0), m_useSolveConstraintObsolete(D6_USE_OBSOLETE_METHOD)
{
	calculateTransforms();
}

Generic6DofConstraint::Generic6DofConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameB)
	: TypedConstraint(D6_CONSTRAINT_TYPE, getFixedBody(), rbB),
	  m_frameInB(frameInB),
	  m_useLinearReferenceFrameA(useLinearReferenceFrameB),
	  m_useOffsetForConstraintFrame(D6_USE_FRAME_OFFSET),
	  m_flags(0),
	  m_useSolveConstraintObsolete(false)
{
	///not providing rigidbody A means implicitly using worldspace for body A
	m_frameInA = rbB.getCenterOfMassTransform() * m_frameInB;
	calculateTransforms();
}

#define GENERIC_D6_DISABLE_WARMSTARTING 1

Scalar GetMatrixElem(const Matrix3x3& mat, i32 index);
Scalar GetMatrixElem(const Matrix3x3& mat, i32 index)
{
	i32 i = index % 3;
	i32 j = index / 3;
	return mat[i][j];
}

///MatrixToEulerXYZ from http://www.geometrictools.com/LibFoundation/Mathematics/Wm4Matrix3.inl.html
bool matrixToEulerXYZ(const Matrix3x3& mat, Vec3& xyz);
bool matrixToEulerXYZ(const Matrix3x3& mat, Vec3& xyz)
{
	//	// rot =  cy*cz          -cy*sz           sy
	//	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy
	//

	Scalar fi = GetMatrixElem(mat, 2);
	if (fi < Scalar(1.0f))
	{
		if (fi > Scalar(-1.0f))
		{
			xyz[0] = Atan2(-GetMatrixElem(mat, 5), GetMatrixElem(mat, 8));
			xyz[1] = Asin(GetMatrixElem(mat, 2));
			xyz[2] = Atan2(-GetMatrixElem(mat, 1), GetMatrixElem(mat, 0));
			return true;
		}
		else
		{
			// WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
			xyz[0] = -Atan2(GetMatrixElem(mat, 3), GetMatrixElem(mat, 4));
			xyz[1] = -SIMD_HALF_PI;
			xyz[2] = Scalar(0.0);
			return false;
		}
	}
	else
	{
		// WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
		xyz[0] = Atan2(GetMatrixElem(mat, 3), GetMatrixElem(mat, 4));
		xyz[1] = SIMD_HALF_PI;
		xyz[2] = 0.0;
	}
	return false;
}

//////////////////////////// RotationalLimitMotor ////////////////////////////////////

i32 RotationalLimitMotor::testLimitValue(Scalar test_value)
{
	if (m_loLimit > m_hiLimit)
	{
		m_currentLimit = 0;  //Free from violation
		return 0;
	}
	if (test_value < m_loLimit)
	{
		m_currentLimit = 1;  //low limit violation
		m_currentLimitError = test_value - m_loLimit;
		if (m_currentLimitError > SIMD_PI)
			m_currentLimitError -= SIMD_2_PI;
		else if (m_currentLimitError < -SIMD_PI)
			m_currentLimitError += SIMD_2_PI;
		return 1;
	}
	else if (test_value > m_hiLimit)
	{
		m_currentLimit = 2;  //High limit violation
		m_currentLimitError = test_value - m_hiLimit;
		if (m_currentLimitError > SIMD_PI)
			m_currentLimitError -= SIMD_2_PI;
		else if (m_currentLimitError < -SIMD_PI)
			m_currentLimitError += SIMD_2_PI;
		return 2;
	};

	m_currentLimit = 0;  //Free from violation
	return 0;
}

Scalar RotationalLimitMotor::solveAngularLimits(
	Scalar timeStep, Vec3& axis, Scalar jacDiagABInv,
	RigidBody* body0, RigidBody* body1)
{
	if (needApplyTorques() == false) return 0.0f;

	Scalar target_velocity = m_targetVelocity;
	Scalar maxMotorForce = m_maxMotorForce;

	//current error correction
	if (m_currentLimit != 0)
	{
		target_velocity = -m_stopERP * m_currentLimitError / (timeStep);
		maxMotorForce = m_maxLimitForce;
	}

	maxMotorForce *= timeStep;

	// current velocity difference

	Vec3 angVelA = body0->getAngularVelocity();
	Vec3 angVelB = body1->getAngularVelocity();

	Vec3 vel_diff;
	vel_diff = angVelA - angVelB;

	Scalar rel_vel = axis.dot(vel_diff);

	// correction velocity
	Scalar motor_relvel = m_limitSoftness * (target_velocity - m_damping * rel_vel);

	if (motor_relvel < SIMD_EPSILON && motor_relvel > -SIMD_EPSILON)
	{
		return 0.0f;  //no need for applying force
	}

	// correction impulse
	Scalar unclippedMotorImpulse = (1 + m_bounce) * motor_relvel * jacDiagABInv;

	// clip correction impulse
	Scalar clippedMotorImpulse;

	///@todo: should clip against accumulated impulse
	if (unclippedMotorImpulse > 0.0f)
	{
		clippedMotorImpulse = unclippedMotorImpulse > maxMotorForce ? maxMotorForce : unclippedMotorImpulse;
	}
	else
	{
		clippedMotorImpulse = unclippedMotorImpulse < -maxMotorForce ? -maxMotorForce : unclippedMotorImpulse;
	}

	// sort with accumulated impulses
	Scalar lo = Scalar(-DRX3D_LARGE_FLOAT);
	Scalar hi = Scalar(DRX3D_LARGE_FLOAT);

	Scalar oldaccumImpulse = m_accumulatedImpulse;
	Scalar sum = oldaccumImpulse + clippedMotorImpulse;
	m_accumulatedImpulse = sum > hi ? Scalar(0.) : sum < lo ? Scalar(0.) : sum;

	clippedMotorImpulse = m_accumulatedImpulse - oldaccumImpulse;

	Vec3 motorImp = clippedMotorImpulse * axis;

	body0->applyTorqueImpulse(motorImp);
	body1->applyTorqueImpulse(-motorImp);

	return clippedMotorImpulse;
}

//////////////////////////// End RotationalLimitMotor ////////////////////////////////////

//////////////////////////// TranslationalLimitMotor ////////////////////////////////////

i32 TranslationalLimitMotor::testLimitValue(i32 limitIndex, Scalar test_value)
{
	Scalar loLimit = m_lowerLimit[limitIndex];
	Scalar hiLimit = m_upperLimit[limitIndex];
	if (loLimit > hiLimit)
	{
		m_currentLimit[limitIndex] = 0;  //Free from violation
		m_currentLimitError[limitIndex] = Scalar(0.f);
		return 0;
	}

	if (test_value < loLimit)
	{
		m_currentLimit[limitIndex] = 2;  //low limit violation
		m_currentLimitError[limitIndex] = test_value - loLimit;
		return 2;
	}
	else if (test_value > hiLimit)
	{
		m_currentLimit[limitIndex] = 1;  //High limit violation
		m_currentLimitError[limitIndex] = test_value - hiLimit;
		return 1;
	};

	m_currentLimit[limitIndex] = 0;  //Free from violation
	m_currentLimitError[limitIndex] = Scalar(0.f);
	return 0;
}

Scalar TranslationalLimitMotor::solveLinearAxis(
	Scalar timeStep,
	Scalar jacDiagABInv,
	RigidBody& body1, const Vec3& pointInA,
	RigidBody& body2, const Vec3& pointInB,
	i32 limit_index,
	const Vec3& axis_normal_on_a,
	const Vec3& anchorPos)
{
	///find relative velocity
	//    Vec3 rel_pos1 = pointInA - body1.getCenterOfMassPosition();
	//    Vec3 rel_pos2 = pointInB - body2.getCenterOfMassPosition();
	Vec3 rel_pos1 = anchorPos - body1.getCenterOfMassPosition();
	Vec3 rel_pos2 = anchorPos - body2.getCenterOfMassPosition();

	Vec3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
	Vec3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
	Vec3 vel = vel1 - vel2;

	Scalar rel_vel = axis_normal_on_a.dot(vel);

	/// apply displacement correction

	//positional error (zeroth order error)
	Scalar depth = -(pointInA - pointInB).dot(axis_normal_on_a);
	Scalar lo = Scalar(-DRX3D_LARGE_FLOAT);
	Scalar hi = Scalar(DRX3D_LARGE_FLOAT);

	Scalar minLimit = m_lowerLimit[limit_index];
	Scalar maxLimit = m_upperLimit[limit_index];

	//handle the limits
	if (minLimit < maxLimit)
	{
		{
			if (depth > maxLimit)
			{
				depth -= maxLimit;
				lo = Scalar(0.);
			}
			else
			{
				if (depth < minLimit)
				{
					depth -= minLimit;
					hi = Scalar(0.);
				}
				else
				{
					return 0.0f;
				}
			}
		}
	}

	Scalar normalImpulse = m_limitSoftness * (m_restitution * depth / timeStep - m_damping * rel_vel) * jacDiagABInv;

	Scalar oldNormalImpulse = m_accumulatedImpulse[limit_index];
	Scalar sum = oldNormalImpulse + normalImpulse;
	m_accumulatedImpulse[limit_index] = sum > hi ? Scalar(0.) : sum < lo ? Scalar(0.) : sum;
	normalImpulse = m_accumulatedImpulse[limit_index] - oldNormalImpulse;

	Vec3 impulse_vector = axis_normal_on_a * normalImpulse;
	body1.applyImpulse(impulse_vector, rel_pos1);
	body2.applyImpulse(-impulse_vector, rel_pos2);

	return normalImpulse;
}

//////////////////////////// TranslationalLimitMotor ////////////////////////////////////

void Generic6DofConstraint::calculateAngleInfo()
{
	Matrix3x3 relative_frame = m_calculatedTransformA.getBasis().inverse() * m_calculatedTransformB.getBasis();
	matrixToEulerXYZ(relative_frame, m_calculatedAxisAngleDiff);
	// in euler angle mode we do not actually constrain the angular velocity
	// along the axes axis[0] and axis[2] (although we do use axis[1]) :
	//
	//    to get			constrain w2-w1 along		...not
	//    ------			---------------------		------
	//    d(angle[0])/dt = 0	ax[1] x ax[2]			ax[0]
	//    d(angle[1])/dt = 0	ax[1]
	//    d(angle[2])/dt = 0	ax[0] x ax[1]			ax[2]
	//
	// constraining w2-w1 along an axis 'a' means that a'*(w2-w1)=0.
	// to prove the result for angle[0], write the expression for angle[0] from
	// GetInfo1 then take the derivative. to prove this for angle[2] it is
	// easier to take the euler rate expression for d(angle[2])/dt with respect
	// to the components of w and set that to 0.
	Vec3 axis0 = m_calculatedTransformB.getBasis().getColumn(0);
	Vec3 axis2 = m_calculatedTransformA.getBasis().getColumn(2);

	m_calculatedAxis[1] = axis2.cross(axis0);
	m_calculatedAxis[0] = m_calculatedAxis[1].cross(axis2);
	m_calculatedAxis[2] = axis0.cross(m_calculatedAxis[1]);

	m_calculatedAxis[0].normalize();
	m_calculatedAxis[1].normalize();
	m_calculatedAxis[2].normalize();
}

void Generic6DofConstraint::calculateTransforms()
{
	calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
}

void Generic6DofConstraint::calculateTransforms(const Transform2& transA, const Transform2& transB)
{
	m_calculatedTransformA = transA * m_frameInA;
	m_calculatedTransformB = transB * m_frameInB;
	calculateLinearInfo();
	calculateAngleInfo();
	if (m_useOffsetForConstraintFrame)
	{  //  get weight factors depending on masses
		Scalar miA = getRigidBodyA().getInvMass();
		Scalar miB = getRigidBodyB().getInvMass();
		m_hasStaticBody = (miA < SIMD_EPSILON) || (miB < SIMD_EPSILON);
		Scalar miS = miA + miB;
		if (miS > Scalar(0.f))
		{
			m_factA = miB / miS;
		}
		else
		{
			m_factA = Scalar(0.5f);
		}
		m_factB = Scalar(1.0f) - m_factA;
	}
}

void Generic6DofConstraint::buildLinearJacobian(
	JacobianEntry& jacLinear, const Vec3& normalWorld,
	const Vec3& pivotAInW, const Vec3& pivotBInW)
{
	new (&jacLinear) JacobianEntry(
		m_rbA.getCenterOfMassTransform().getBasis().transpose(),
		m_rbB.getCenterOfMassTransform().getBasis().transpose(),
		pivotAInW - m_rbA.getCenterOfMassPosition(),
		pivotBInW - m_rbB.getCenterOfMassPosition(),
		normalWorld,
		m_rbA.getInvInertiaDiagLocal(),
		m_rbA.getInvMass(),
		m_rbB.getInvInertiaDiagLocal(),
		m_rbB.getInvMass());
}

void Generic6DofConstraint::buildAngularJacobian(
	JacobianEntry& jacAngular, const Vec3& jointAxisW)
{
	new (&jacAngular) JacobianEntry(jointAxisW,
									  m_rbA.getCenterOfMassTransform().getBasis().transpose(),
									  m_rbB.getCenterOfMassTransform().getBasis().transpose(),
									  m_rbA.getInvInertiaDiagLocal(),
									  m_rbB.getInvInertiaDiagLocal());
}

bool Generic6DofConstraint::testAngularLimitMotor(i32 axis_index)
{
	Scalar angle = m_calculatedAxisAngleDiff[axis_index];
	angle = AdjustAngleToLimits(angle, m_angularLimits[axis_index].m_loLimit, m_angularLimits[axis_index].m_hiLimit);
	m_angularLimits[axis_index].m_currentPosition = angle;
	//test limits
	m_angularLimits[axis_index].testLimitValue(angle);
	return m_angularLimits[axis_index].needApplyTorques();
}

void Generic6DofConstraint::buildJacobian()
{
#ifndef __SPU__
	if (m_useSolveConstraintObsolete)
	{
		// Clear accumulated impulses for the next simulation step
		m_linearLimits.m_accumulatedImpulse.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
		i32 i;
		for (i = 0; i < 3; i++)
		{
			m_angularLimits[i].m_accumulatedImpulse = Scalar(0.);
		}
		//calculates transform
		calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());

		//  const Vec3& pivotAInW = m_calculatedTransformA.getOrigin();
		//  const Vec3& pivotBInW = m_calculatedTransformB.getOrigin();
		calcAnchorPos();
		Vec3 pivotAInW = m_AnchorPos;
		Vec3 pivotBInW = m_AnchorPos;

		// not used here
		//    Vec3 rel_pos1 = pivotAInW - m_rbA.getCenterOfMassPosition();
		//    Vec3 rel_pos2 = pivotBInW - m_rbB.getCenterOfMassPosition();

		Vec3 normalWorld;
		//linear part
		for (i = 0; i < 3; i++)
		{
			if (m_linearLimits.isLimited(i))
			{
				if (m_useLinearReferenceFrameA)
					normalWorld = m_calculatedTransformA.getBasis().getColumn(i);
				else
					normalWorld = m_calculatedTransformB.getBasis().getColumn(i);

				buildLinearJacobian(
					m_jacLinear[i], normalWorld,
					pivotAInW, pivotBInW);
			}
		}

		// angular part
		for (i = 0; i < 3; i++)
		{
			//calculates error angle
			if (testAngularLimitMotor(i))
			{
				normalWorld = this->getAxis(i);
				// Create angular atom
				buildAngularJacobian(m_jacAng[i], normalWorld);
			}
		}
	}
#endif  //__SPU__
}

void Generic6DofConstraint::getInfo1(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		//prepare constraint
		calculateTransforms(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform());
		info->m_numConstraintRows = 0;
		info->nub = 6;
		i32 i;
		//test linear limits
		for (i = 0; i < 3; i++)
		{
			if (m_linearLimits.needApplyForce(i))
			{
				info->m_numConstraintRows++;
				info->nub--;
			}
		}
		//test angular limits
		for (i = 0; i < 3; i++)
		{
			if (testAngularLimitMotor(i))
			{
				info->m_numConstraintRows++;
				info->nub--;
			}
		}
	}
}

void Generic6DofConstraint::getInfo1NonVirtual(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		//pre-allocate all 6
		info->m_numConstraintRows = 6;
		info->nub = 0;
	}
}

void Generic6DofConstraint::getInfo2(ConstraintInfo2* info)
{
	Assert(!m_useSolveConstraintObsolete);

	const Transform2& transA = m_rbA.getCenterOfMassTransform();
	const Transform2& transB = m_rbB.getCenterOfMassTransform();
	const Vec3& linVelA = m_rbA.getLinearVelocity();
	const Vec3& linVelB = m_rbB.getLinearVelocity();
	const Vec3& angVelA = m_rbA.getAngularVelocity();
	const Vec3& angVelB = m_rbB.getAngularVelocity();

	if (m_useOffsetForConstraintFrame)
	{  // for stability better to solve angular limits first
		i32 row = setAngularLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
		setLinearLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	}
	else
	{  // leave old version for compatibility
		i32 row = setLinearLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
		setAngularLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	}
}

void Generic6DofConstraint::getInfo2NonVirtual(ConstraintInfo2* info, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB)
{
	Assert(!m_useSolveConstraintObsolete);
	//prepare constraint
	calculateTransforms(transA, transB);

	i32 i;
	for (i = 0; i < 3; i++)
	{
		testAngularLimitMotor(i);
	}

	if (m_useOffsetForConstraintFrame)
	{  // for stability better to solve angular limits first
		i32 row = setAngularLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
		setLinearLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	}
	else
	{  // leave old version for compatibility
		i32 row = setLinearLimits(info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB);
		setAngularLimits(info, row, transA, transB, linVelA, linVelB, angVelA, angVelB);
	}
}

i32 Generic6DofConstraint::setLinearLimits(ConstraintInfo2* info, i32 row, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB)
{
	//	i32 row = 0;
	//solve linear limits
	RotationalLimitMotor limot;
	for (i32 i = 0; i < 3; i++)
	{
		if (m_linearLimits.needApplyForce(i))
		{  // re-use rotational motor code
			limot.m_bounce = Scalar(0.f);
			limot.m_currentLimit = m_linearLimits.m_currentLimit[i];
			limot.m_currentPosition = m_linearLimits.m_currentLinearDiff[i];
			limot.m_currentLimitError = m_linearLimits.m_currentLimitError[i];
			limot.m_damping = m_linearLimits.m_damping;
			limot.m_enableMotor = m_linearLimits.m_enableMotor[i];
			limot.m_hiLimit = m_linearLimits.m_upperLimit[i];
			limot.m_limitSoftness = m_linearLimits.m_limitSoftness;
			limot.m_loLimit = m_linearLimits.m_lowerLimit[i];
			limot.m_maxLimitForce = Scalar(0.f);
			limot.m_maxMotorForce = m_linearLimits.m_maxMotorForce[i];
			limot.m_targetVelocity = m_linearLimits.m_targetVelocity[i];
			Vec3 axis = m_calculatedTransformA.getBasis().getColumn(i);
			i32 flags = m_flags >> (i * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
			limot.m_normalCFM = (flags & DRX3D_6DOF_FLAGS_CFM_NORM) ? m_linearLimits.m_normalCFM[i] : info->cfm[0];
			limot.m_stopCFM = (flags & DRX3D_6DOF_FLAGS_CFM_STOP) ? m_linearLimits.m_stopCFM[i] : info->cfm[0];
			limot.m_stopERP = (flags & DRX3D_6DOF_FLAGS_ERP_STOP) ? m_linearLimits.m_stopERP[i] : info->erp;
			if (m_useOffsetForConstraintFrame)
			{
				i32 indx1 = (i + 1) % 3;
				i32 indx2 = (i + 2) % 3;
				i32 rotAllowed = 1;  // rotations around orthos to current axis
				if (m_angularLimits[indx1].m_currentLimit && m_angularLimits[indx2].m_currentLimit)
				{
					rotAllowed = 0;
				}
				row += get_limit_motor_info2(&limot, transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 0, rotAllowed);
			}
			else
			{
				row += get_limit_motor_info2(&limot, transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 0);
			}
		}
	}
	return row;
}

i32 Generic6DofConstraint::setAngularLimits(ConstraintInfo2* info, i32 row_offset, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB)
{
	Generic6DofConstraint* d6constraint = this;
	i32 row = row_offset;
	//solve angular limits
	for (i32 i = 0; i < 3; i++)
	{
		if (d6constraint->getRotationalLimitMotor(i)->needApplyTorques())
		{
			Vec3 axis = d6constraint->getAxis(i);
			i32 flags = m_flags >> ((i + 3) * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
			if (!(flags & DRX3D_6DOF_FLAGS_CFM_NORM))
			{
				m_angularLimits[i].m_normalCFM = info->cfm[0];
			}
			if (!(flags & DRX3D_6DOF_FLAGS_CFM_STOP))
			{
				m_angularLimits[i].m_stopCFM = info->cfm[0];
			}
			if (!(flags & DRX3D_6DOF_FLAGS_ERP_STOP))
			{
				m_angularLimits[i].m_stopERP = info->erp;
			}
			row += get_limit_motor_info2(d6constraint->getRotationalLimitMotor(i),
										 transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 1);
		}
	}

	return row;
}

void Generic6DofConstraint::updateRHS(Scalar timeStep)
{
	(void)timeStep;
}

void Generic6DofConstraint::setFrames(const Transform2& frameA, const Transform2& frameB)
{
	m_frameInA = frameA;
	m_frameInB = frameB;
	buildJacobian();
	calculateTransforms();
}

Vec3 Generic6DofConstraint::getAxis(i32 axis_index) const
{
	return m_calculatedAxis[axis_index];
}

Scalar Generic6DofConstraint::getRelativePivotPosition(i32 axisIndex) const
{
	return m_calculatedLinearDiff[axisIndex];
}

Scalar Generic6DofConstraint::getAngle(i32 axisIndex) const
{
	return m_calculatedAxisAngleDiff[axisIndex];
}

void Generic6DofConstraint::calcAnchorPos(void)
{
	Scalar imA = m_rbA.getInvMass();
	Scalar imB = m_rbB.getInvMass();
	Scalar weight;
	if (imB == Scalar(0.0))
	{
		weight = Scalar(1.0);
	}
	else
	{
		weight = imA / (imA + imB);
	}
	const Vec3& pA = m_calculatedTransformA.getOrigin();
	const Vec3& pB = m_calculatedTransformB.getOrigin();
	m_AnchorPos = pA * weight + pB * (Scalar(1.0) - weight);
	return;
}

void Generic6DofConstraint::calculateLinearInfo()
{
	m_calculatedLinearDiff = m_calculatedTransformB.getOrigin() - m_calculatedTransformA.getOrigin();
	m_calculatedLinearDiff = m_calculatedTransformA.getBasis().inverse() * m_calculatedLinearDiff;
	for (i32 i = 0; i < 3; i++)
	{
		m_linearLimits.m_currentLinearDiff[i] = m_calculatedLinearDiff[i];
		m_linearLimits.testLimitValue(i, m_calculatedLinearDiff[i]);
	}
}

i32 Generic6DofConstraint::get_limit_motor_info2(
	RotationalLimitMotor* limot,
	const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB,
	ConstraintInfo2* info, i32 row, Vec3& ax1, i32 rotational, i32 rotAllowed)
{
	i32 srow = row * info->rowskip;
	bool powered = limot->m_enableMotor;
	i32 limit = limot->m_currentLimit;
	if (powered || limit)
	{  // if the joint is powered, or has joint limits, add in the extra row
		Scalar* J1 = rotational ? info->m_J1angularAxis : info->m_J1linearAxis;
		Scalar* J2 = rotational ? info->m_J2angularAxis : info->m_J2linearAxis;
		J1[srow + 0] = ax1[0];
		J1[srow + 1] = ax1[1];
		J1[srow + 2] = ax1[2];

		J2[srow + 0] = -ax1[0];
		J2[srow + 1] = -ax1[1];
		J2[srow + 2] = -ax1[2];

		if ((!rotational))
		{
			if (m_useOffsetForConstraintFrame)
			{
				Vec3 tmpA, tmpB, relA, relB;
				// get vector from bodyB to frameB in WCS
				relB = m_calculatedTransformB.getOrigin() - transB.getOrigin();
				// get its projection to constraint axis
				Vec3 projB = ax1 * relB.dot(ax1);
				// get vector directed from bodyB to constraint axis (and orthogonal to it)
				Vec3 orthoB = relB - projB;
				// same for bodyA
				relA = m_calculatedTransformA.getOrigin() - transA.getOrigin();
				Vec3 projA = ax1 * relA.dot(ax1);
				Vec3 orthoA = relA - projA;
				// get desired offset between frames A and B along constraint axis
				Scalar desiredOffs = limot->m_currentPosition - limot->m_currentLimitError;
				// desired vector from projection of center of bodyA to projection of center of bodyB to constraint axis
				Vec3 totalDist = projA + ax1 * desiredOffs - projB;
				// get offset vectors relA and relB
				relA = orthoA + totalDist * m_factA;
				relB = orthoB - totalDist * m_factB;
				tmpA = relA.cross(ax1);
				tmpB = relB.cross(ax1);
				if (m_hasStaticBody && (!rotAllowed))
				{
					tmpA *= m_factA;
					tmpB *= m_factB;
				}
				i32 i;
				for (i = 0; i < 3; i++) info->m_J1angularAxis[srow + i] = tmpA[i];
				for (i = 0; i < 3; i++) info->m_J2angularAxis[srow + i] = -tmpB[i];
			}
			else
			{
				Vec3 ltd;  // Linear Torque Decoupling vector
				Vec3 c = m_calculatedTransformB.getOrigin() - transA.getOrigin();
				ltd = c.cross(ax1);
				info->m_J1angularAxis[srow + 0] = ltd[0];
				info->m_J1angularAxis[srow + 1] = ltd[1];
				info->m_J1angularAxis[srow + 2] = ltd[2];

				c = m_calculatedTransformB.getOrigin() - transB.getOrigin();
				ltd = -c.cross(ax1);
				info->m_J2angularAxis[srow + 0] = ltd[0];
				info->m_J2angularAxis[srow + 1] = ltd[1];
				info->m_J2angularAxis[srow + 2] = ltd[2];
			}
		}
		// if we're limited low and high simultaneously, the joint motor is
		// ineffective
		if (limit && (limot->m_loLimit == limot->m_hiLimit)) powered = false;
		info->m_constraintError[srow] = Scalar(0.f);
		if (powered)
		{
			info->cfm[srow] = limot->m_normalCFM;
			if (!limit)
			{
				Scalar tag_vel = rotational ? limot->m_targetVelocity : -limot->m_targetVelocity;

				Scalar mot_fact = getMotorFactor(limot->m_currentPosition,
												   limot->m_loLimit,
												   limot->m_hiLimit,
												   tag_vel,
												   info->fps * limot->m_stopERP);
				info->m_constraintError[srow] += mot_fact * limot->m_targetVelocity;
				info->m_lowerLimit[srow] = -limot->m_maxMotorForce / info->fps;
				info->m_upperLimit[srow] = limot->m_maxMotorForce / info->fps;
			}
		}
		if (limit)
		{
			Scalar k = info->fps * limot->m_stopERP;
			if (!rotational)
			{
				info->m_constraintError[srow] += k * limot->m_currentLimitError;
			}
			else
			{
				info->m_constraintError[srow] += -k * limot->m_currentLimitError;
			}
			info->cfm[srow] = limot->m_stopCFM;
			if (limot->m_loLimit == limot->m_hiLimit)
			{  // limited low and high simultaneously
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else
			{
				if (limit == 1)
				{
					info->m_lowerLimit[srow] = 0;
					info->m_upperLimit[srow] = SIMD_INFINITY;
				}
				else
				{
					info->m_lowerLimit[srow] = -SIMD_INFINITY;
					info->m_upperLimit[srow] = 0;
				}
				// deal with bounce
				if (limot->m_bounce > 0)
				{
					// calculate joint velocity
					Scalar vel;
					if (rotational)
					{
						vel = angVelA.dot(ax1);
						//make sure that if no body -> angVelB == zero vec
						//                        if (body1)
						vel -= angVelB.dot(ax1);
					}
					else
					{
						vel = linVelA.dot(ax1);
						//make sure that if no body -> angVelB == zero vec
						//                        if (body1)
						vel -= linVelB.dot(ax1);
					}
					// only apply bounce if the velocity is incoming, and if the
					// resulting c[] exceeds what we already have.
					if (limit == 1)
					{
						if (vel < 0)
						{
							Scalar newc = -limot->m_bounce * vel;
							if (newc > info->m_constraintError[srow])
								info->m_constraintError[srow] = newc;
						}
					}
					else
					{
						if (vel > 0)
						{
							Scalar newc = -limot->m_bounce * vel;
							if (newc < info->m_constraintError[srow])
								info->m_constraintError[srow] = newc;
						}
					}
				}
			}
		}
		return 1;
	}
	else
		return 0;
}

///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
///If no axis is provided, it uses the default axis for this constraint.
void Generic6DofConstraint::setParam(i32 num, Scalar value, i32 axis)
{
	if ((axis >= 0) && (axis < 3))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				m_linearLimits.m_stopERP[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_ERP_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				m_linearLimits.m_stopCFM[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
				break;
			case DRX3D_CONSTRAINT_CFM:
				m_linearLimits.m_normalCFM[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_NORM << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
				break;
			default:
				AssertConstrParams(0);
		}
	}
	else if ((axis >= 3) && (axis < 6))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				m_angularLimits[axis - 3].m_stopERP = value;
				m_flags |= DRX3D_6DOF_FLAGS_ERP_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				m_angularLimits[axis - 3].m_stopCFM = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
				break;
			case DRX3D_CONSTRAINT_CFM:
				m_angularLimits[axis - 3].m_normalCFM = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_NORM << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT);
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
Scalar Generic6DofConstraint::getParam(i32 num, i32 axis) const
{
	Scalar retVal = 0;
	if ((axis >= 0) && (axis < 3))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_ERP_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_linearLimits.m_stopERP[axis];
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_CFM_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_linearLimits.m_stopCFM[axis];
				break;
			case DRX3D_CONSTRAINT_CFM:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_CFM_NORM << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_linearLimits.m_normalCFM[axis];
				break;
			default:
				AssertConstrParams(0);
		}
	}
	else if ((axis >= 3) && (axis < 6))
	{
		switch (num)
		{
			case DRX3D_CONSTRAINT_STOP_ERP:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_ERP_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_angularLimits[axis - 3].m_stopERP;
				break;
			case DRX3D_CONSTRAINT_STOP_CFM:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_CFM_STOP << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_angularLimits[axis - 3].m_stopCFM;
				break;
			case DRX3D_CONSTRAINT_CFM:
				AssertConstrParams(m_flags & (DRX3D_6DOF_FLAGS_CFM_NORM << (axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT)));
				retVal = m_angularLimits[axis - 3].m_normalCFM;
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

void Generic6DofConstraint::setAxis(const Vec3& axis1, const Vec3& axis2)
{
	Vec3 zAxis = axis1.normalized();
	Vec3 yAxis = axis2.normalized();
	Vec3 xAxis = yAxis.cross(zAxis);  // we want right coordinate system

	Transform2 frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setVal(xAxis[0], yAxis[0], zAxis[0],
								 xAxis[1], yAxis[1], zAxis[1],
								 xAxis[2], yAxis[2], zAxis[2]);

	// now get constraint frame in local coordinate systems
	m_frameInA = m_rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = m_rbB.getCenterOfMassTransform().inverse() * frameInW;

	calculateTransforms();
}
