#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <cmath>
#include <new>

Generic6DofSpring2Constraint::Generic6DofSpring2Constraint ( RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, RotateOrder rotOrder )
		: TypedConstraint ( D6_SPRING_2_CONSTRAINT_TYPE, rbA, rbB ), m_frameInA ( frameInA ), m_frameInB ( frameInB ), m_rotateOrder ( rotOrder ), m_flags ( 0 )
{
	calculateTransforms();
}

Generic6DofSpring2Constraint::Generic6DofSpring2Constraint ( RigidBody& rbB, const Transform2& frameInB, RotateOrder rotOrder )
		: TypedConstraint ( D6_SPRING_2_CONSTRAINT_TYPE, getFixedBody(), rbB ), m_frameInB ( frameInB ), m_rotateOrder ( rotOrder ), m_flags ( 0 )
{
	///not providing rigidbody A means implicitly using worldspace for body A
	m_frameInA = rbB.getCenterOfMassTransform() * m_frameInB;
	calculateTransforms();
}

Scalar Generic6DofSpring2Constraint::GetMatrixElem ( const Matrix3x3& mat, i32 index )
{
	i32 i = index % 3;
	i32 j = index / 3;
	return mat[i][j];
}

// MatrixToEulerXYZ from http://www.geometrictools.com/LibFoundation/Mathematics/Wm4Matrix3.inl.html

bool Generic6DofSpring2Constraint::matrixToEulerXYZ ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	Scalar fi = GetMatrixElem ( mat, 2 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Atan2 ( -GetMatrixElem ( mat, 5 ), GetMatrixElem ( mat, 8 ) );
			xyz[1] = Asin ( GetMatrixElem ( mat, 2 ) );
			xyz[2] = Atan2 ( -GetMatrixElem ( mat, 1 ), GetMatrixElem ( mat, 0 ) );
			return true;
		}

		else
		{
			// WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
			xyz[0] = -Atan2 ( GetMatrixElem ( mat, 3 ), GetMatrixElem ( mat, 4 ) );
			xyz[1] = -SIMD_HALF_PI;
			xyz[2] = Scalar ( 0.0 );
			return false;
		}
	}

	else
	{
		// WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
		xyz[0] = Atan2 ( GetMatrixElem ( mat, 3 ), GetMatrixElem ( mat, 4 ) );
		xyz[1] = SIMD_HALF_PI;
		xyz[2] = 0.0;
	}

	return false;
}

bool Generic6DofSpring2Constraint::matrixToEulerXZY ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cy*cz          -sz           sy*cz
	//        cy*cx*sz+sx*sy  cx*cz        sy*cx*sz-cy*sx
	//        cy*sx*sz-cx*sy  sx*cz        sy*sx*sz+cx*cy

	Scalar fi = GetMatrixElem ( mat, 1 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Atan2 ( GetMatrixElem ( mat, 7 ), GetMatrixElem ( mat, 4 ) );
			xyz[1] = Atan2 ( GetMatrixElem ( mat, 2 ), GetMatrixElem ( mat, 0 ) );
			xyz[2] = Asin ( -GetMatrixElem ( mat, 1 ) );
			return true;
		}

		else
		{
			xyz[0] = -Atan2 ( -GetMatrixElem ( mat, 6 ), GetMatrixElem ( mat, 8 ) );
			xyz[1] = Scalar ( 0.0 );
			xyz[2] = SIMD_HALF_PI;
			return false;
		}
	}

	else
	{
		xyz[0] = Atan2 ( -GetMatrixElem ( mat, 6 ), GetMatrixElem ( mat, 8 ) );
		xyz[1] = 0.0;
		xyz[2] = -SIMD_HALF_PI;
	}

	return false;
}

bool Generic6DofSpring2Constraint::matrixToEulerYXZ ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cy*cz+sy*sx*sz  cz*sy*sx-cy*sz  cx*sy
	//        cx*sz           cx*cz           -sx
	//        cy*sx*sz-cz*sy  sy*sz+cy*cz*sx  cy*cx

	Scalar fi = GetMatrixElem ( mat, 5 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Asin ( -GetMatrixElem ( mat, 5 ) );
			xyz[1] = Atan2 ( GetMatrixElem ( mat, 2 ), GetMatrixElem ( mat, 8 ) );
			xyz[2] = Atan2 ( GetMatrixElem ( mat, 3 ), GetMatrixElem ( mat, 4 ) );
			return true;
		}

		else
		{
			xyz[0] = SIMD_HALF_PI;
			xyz[1] = -Atan2 ( -GetMatrixElem ( mat, 1 ), GetMatrixElem ( mat, 0 ) );
			xyz[2] = Scalar ( 0.0 );
			return false;
		}
	}

	else
	{
		xyz[0] = -SIMD_HALF_PI;
		xyz[1] = Atan2 ( -GetMatrixElem ( mat, 1 ), GetMatrixElem ( mat, 0 ) );
		xyz[2] = 0.0;
	}

	return false;
}

bool Generic6DofSpring2Constraint::matrixToEulerYZX ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cy*cz   sy*sx-cy*cx*sz   cx*sy+cy*sz*sx
	//        sz           cz*cx           -cz*sx
	//        -cz*sy  cy*sx+cx*sy*sz   cy*cx-sy*sz*sx

	Scalar fi = GetMatrixElem ( mat, 3 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Atan2 ( -GetMatrixElem ( mat, 5 ), GetMatrixElem ( mat, 4 ) );
			xyz[1] = Atan2 ( -GetMatrixElem ( mat, 6 ), GetMatrixElem ( mat, 0 ) );
			xyz[2] = Asin ( GetMatrixElem ( mat, 3 ) );
			return true;
		}

		else
		{
			xyz[0] = Scalar ( 0.0 );
			xyz[1] = -Atan2 ( GetMatrixElem ( mat, 7 ), GetMatrixElem ( mat, 8 ) );
			xyz[2] = -SIMD_HALF_PI;
			return false;
		}
	}

	else
	{
		xyz[0] = Scalar ( 0.0 );
		xyz[1] = Atan2 ( GetMatrixElem ( mat, 7 ), GetMatrixElem ( mat, 8 ) );
		xyz[2] = SIMD_HALF_PI;
	}

	return false;
}

bool Generic6DofSpring2Constraint::matrixToEulerZXY ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cz*cy-sz*sx*sy    -cx*sz   cz*sy+cy*sz*sx
	//        cy*sz+cz*sx*sy     cz*cx   sz*sy-cz*xy*sx
	//        -cx*sy              sx     cx*cy

	Scalar fi = GetMatrixElem ( mat, 7 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Asin ( GetMatrixElem ( mat, 7 ) );
			xyz[1] = Atan2 ( -GetMatrixElem ( mat, 6 ), GetMatrixElem ( mat, 8 ) );
			xyz[2] = Atan2 ( -GetMatrixElem ( mat, 1 ), GetMatrixElem ( mat, 4 ) );
			return true;
		}

		else
		{
			xyz[0] = -SIMD_HALF_PI;
			xyz[1] = Scalar ( 0.0 );
			xyz[2] = -Atan2 ( GetMatrixElem ( mat, 2 ), GetMatrixElem ( mat, 0 ) );
			return false;
		}
	}

	else
	{
		xyz[0] = SIMD_HALF_PI;
		xyz[1] = Scalar ( 0.0 );
		xyz[2] = Atan2 ( GetMatrixElem ( mat, 2 ), GetMatrixElem ( mat, 0 ) );
	}

	return false;
}

bool Generic6DofSpring2Constraint::matrixToEulerZYX ( const Matrix3x3& mat, Vec3& xyz )
{
	// rot =  cz*cy   cz*sy*sx-cx*sz   sz*sx+cz*cx*sy
	//        cy*sz   cz*cx+sz*sy*sx   cx*sz*sy-cz*sx
	//        -sy          cy*sx         cy*cx

	Scalar fi = GetMatrixElem ( mat, 6 );

	if ( fi < Scalar ( 1.0f ) )
	{
		if ( fi > Scalar ( -1.0f ) )
		{
			xyz[0] = Atan2 ( GetMatrixElem ( mat, 7 ), GetMatrixElem ( mat, 8 ) );
			xyz[1] = Asin ( -GetMatrixElem ( mat, 6 ) );
			xyz[2] = Atan2 ( GetMatrixElem ( mat, 3 ), GetMatrixElem ( mat, 0 ) );
			return true;
		}

		else
		{
			xyz[0] = Scalar ( 0.0 );
			xyz[1] = SIMD_HALF_PI;
			xyz[2] = -Atan2 ( GetMatrixElem ( mat, 1 ), GetMatrixElem ( mat, 2 ) );
			return false;
		}
	}

	else
	{
		xyz[0] = Scalar ( 0.0 );
		xyz[1] = -SIMD_HALF_PI;
		xyz[2] = Atan2 ( -GetMatrixElem ( mat, 1 ), -GetMatrixElem ( mat, 2 ) );
	}

	return false;
}

void Generic6DofSpring2Constraint::calculateAngleInfo()
{
	Matrix3x3 relative_frame = m_calculatedTransformA.getBasis().inverse() * m_calculatedTransformB.getBasis();

	switch ( m_rotateOrder )
	{

		case RO_XYZ:
			matrixToEulerXYZ ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		case RO_XZY:
			matrixToEulerXZY ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		case RO_YXZ:
			matrixToEulerYXZ ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		case RO_YZX:
			matrixToEulerYZX ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		case RO_ZXY:
			matrixToEulerZXY ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		case RO_ZYX:
			matrixToEulerZYX ( relative_frame, m_calculatedAxisAngleDiff );
			break;

		default:
			Assert ( false );
	}

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

	switch ( m_rotateOrder )
	{

		case RO_XYZ:
			{
				//Is this the "line of nodes" calculation choosing planes YZ (B coordinate system) and xy (A coordinate system)? (http://en.wikipedia.org/wiki/Euler_angles)
				//The two planes are non-homologous, so this is a Tait Bryan angle formalism and not a proper Euler
				//Extrinsic rotations are equal to the reversed order intrinsic rotations so the above xyz extrinsic rotations (axes are fixed) are the same as the zy'x" intrinsic rotations (axes are refreshed after each rotation)
				//that is why xy and YZ planes are chosen (this will describe a zy'x" intrinsic rotation) (see the figure on the left at http://en.wikipedia.org/wiki/Euler_angles under Tait Bryan angles)
				// x' = Nperp = N.cross(axis2)
				// y' = N = axis2.cross(axis0)
				// z' = z
				//
				// x" = X
				// y" = y'
				// z" = ??
				//in other words:
				//first rotate around z
				//second rotate around y'= z.cross(X)
				//third rotate around x" = X
				//Original XYZ extrinsic rotation order.
				//Planes: xy and YZ normals: z, X.  Plane intersection (N) is z.cross(X)
				Vec3 axis0 = m_calculatedTransformB.getBasis().getColumn ( 0 );
				Vec3 axis2 = m_calculatedTransformA.getBasis().getColumn ( 2 );
				m_calculatedAxis[1] = axis2.cross ( axis0 );
				m_calculatedAxis[0] = m_calculatedAxis[1].cross ( axis2 );
				m_calculatedAxis[2] = axis0.cross ( m_calculatedAxis[1] );
				break;
			}

		case RO_XZY:
			{
				//planes: xz,ZY normals: y, X
				//first rotate around y
				//second rotate around z'= y.cross(X)
				//third rotate around x" = X
				Vec3 axis0 = m_calculatedTransformB.getBasis().getColumn ( 0 );
				Vec3 axis1 = m_calculatedTransformA.getBasis().getColumn ( 1 );
				m_calculatedAxis[2] = axis0.cross ( axis1 );
				m_calculatedAxis[0] = axis1.cross ( m_calculatedAxis[2] );
				m_calculatedAxis[1] = m_calculatedAxis[2].cross ( axis0 );
				break;
			}

		case RO_YXZ:
			{
				//planes: yx,XZ normals: z, Y
				//first rotate around z
				//second rotate around x'= z.cross(Y)
				//third rotate around y" = Y
				Vec3 axis1 = m_calculatedTransformB.getBasis().getColumn ( 1 );
				Vec3 axis2 = m_calculatedTransformA.getBasis().getColumn ( 2 );
				m_calculatedAxis[0] = axis1.cross ( axis2 );
				m_calculatedAxis[1] = axis2.cross ( m_calculatedAxis[0] );
				m_calculatedAxis[2] = m_calculatedAxis[0].cross ( axis1 );
				break;
			}

		case RO_YZX:
			{
				//planes: yz,ZX normals: x, Y
				//first rotate around x
				//second rotate around z'= x.cross(Y)
				//third rotate around y" = Y
				Vec3 axis0 = m_calculatedTransformA.getBasis().getColumn ( 0 );
				Vec3 axis1 = m_calculatedTransformB.getBasis().getColumn ( 1 );
				m_calculatedAxis[2] = axis0.cross ( axis1 );
				m_calculatedAxis[0] = axis1.cross ( m_calculatedAxis[2] );
				m_calculatedAxis[1] = m_calculatedAxis[2].cross ( axis0 );
				break;
			}

		case RO_ZXY:
			{
				//planes: zx,XY normals: y, Z
				//first rotate around y
				//second rotate around x'= y.cross(Z)
				//third rotate around z" = Z
				Vec3 axis1 = m_calculatedTransformA.getBasis().getColumn ( 1 );
				Vec3 axis2 = m_calculatedTransformB.getBasis().getColumn ( 2 );
				m_calculatedAxis[0] = axis1.cross ( axis2 );
				m_calculatedAxis[1] = axis2.cross ( m_calculatedAxis[0] );
				m_calculatedAxis[2] = m_calculatedAxis[0].cross ( axis1 );
				break;
			}

		case RO_ZYX:
			{
				//planes: zy,YX normals: x, Z
				//first rotate around x
				//second rotate around y' = x.cross(Z)
				//third rotate around z" = Z
				Vec3 axis0 = m_calculatedTransformA.getBasis().getColumn ( 0 );
				Vec3 axis2 = m_calculatedTransformB.getBasis().getColumn ( 2 );
				m_calculatedAxis[1] = axis2.cross ( axis0 );
				m_calculatedAxis[0] = m_calculatedAxis[1].cross ( axis2 );
				m_calculatedAxis[2] = axis0.cross ( m_calculatedAxis[1] );
				break;
			}

		default:
			Assert ( false );
	}

	m_calculatedAxis[0].normalize();

	m_calculatedAxis[1].normalize();
	m_calculatedAxis[2].normalize();
}

void Generic6DofSpring2Constraint::calculateTransforms()
{
	calculateTransforms ( m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform() );
}

void Generic6DofSpring2Constraint::calculateTransforms ( const Transform2& transA, const Transform2& transB )
{
	m_calculatedTransformA = transA * m_frameInA;
	m_calculatedTransformB = transB * m_frameInB;
	calculateLinearInfo();
	calculateAngleInfo();

	Scalar miA = getRigidBodyA().getInvMass();
	Scalar miB = getRigidBodyB().getInvMass();
	m_hasStaticBody = ( miA < SIMD_EPSILON ) || ( miB < SIMD_EPSILON );
	Scalar miS = miA + miB;

	if ( miS > Scalar ( 0.f ) )
	{
		m_factA = miB / miS;
	}

	else
	{
		m_factA = Scalar ( 0.5f );
	}

	m_factB = Scalar ( 1.0f ) - m_factA;
}

void Generic6DofSpring2Constraint::testAngularLimitMotor ( i32 axis_index )
{
	Scalar angle = m_calculatedAxisAngleDiff[axis_index];
	angle = AdjustAngleToLimits ( angle, m_angularLimits[axis_index].m_loLimit, m_angularLimits[axis_index].m_hiLimit );
	m_angularLimits[axis_index].m_currentPosition = angle;
	m_angularLimits[axis_index].testLimitValue ( angle );
}

void Generic6DofSpring2Constraint::getInfo1 ( ConstraintInfo1* info )
{
	//prepare constraint
	calculateTransforms ( m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform() );
	info->m_numConstraintRows = 0;
	info->nub = 0;
	i32 i;
	//test linear limits

	for ( i = 0; i < 3; i++ )
	{
		if ( m_linearLimits.m_currentLimit[i] == 4 )
			info->m_numConstraintRows += 2;
		else
			if ( m_linearLimits.m_currentLimit[i] != 0 )
				info->m_numConstraintRows += 1;

		if ( m_linearLimits.m_enableMotor[i] )
			info->m_numConstraintRows += 1;

		if ( m_linearLimits.m_enableSpring[i] )
			info->m_numConstraintRows += 1;
	}

	//test angular limits

	for ( i = 0; i < 3; i++ )
	{
		testAngularLimitMotor ( i );

		if ( m_angularLimits[i].m_currentLimit == 4 )
			info->m_numConstraintRows += 2;
		else
			if ( m_angularLimits[i].m_currentLimit != 0 )
				info->m_numConstraintRows += 1;

		if ( m_angularLimits[i].m_enableMotor )
			info->m_numConstraintRows += 1;

		if ( m_angularLimits[i].m_enableSpring )
			info->m_numConstraintRows += 1;
	}
}

void Generic6DofSpring2Constraint::getInfo2 ( ConstraintInfo2* info )
{
	const Transform2& transA = m_rbA.getCenterOfMassTransform();
	const Transform2& transB = m_rbB.getCenterOfMassTransform();
	const Vec3& linVelA = m_rbA.getLinearVelocity();
	const Vec3& linVelB = m_rbB.getLinearVelocity();
	const Vec3& angVelA = m_rbA.getAngularVelocity();
	const Vec3& angVelB = m_rbB.getAngularVelocity();

	// for stability better to solve angular limits first
	i32 row = setAngularLimits ( info, 0, transA, transB, linVelA, linVelB, angVelA, angVelB );
	setLinearLimits ( info, row, transA, transB, linVelA, linVelB, angVelA, angVelB );
}

i32 Generic6DofSpring2Constraint::setLinearLimits ( ConstraintInfo2* info, i32 row, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB )
{
	//solve linear limits
	RotationalLimitMotor2 limot;

	for ( i32 i = 0; i < 3; i++ )
	{
		if ( m_linearLimits.m_currentLimit[i] || m_linearLimits.m_enableMotor[i] || m_linearLimits.m_enableSpring[i] )
		{
			// re-use rotational motor code
			limot.m_bounce = m_linearLimits.m_bounce[i];
			limot.m_currentLimit = m_linearLimits.m_currentLimit[i];
			limot.m_currentPosition = m_linearLimits.m_currentLinearDiff[i];
			limot.m_currentLimitError = m_linearLimits.m_currentLimitError[i];
			limot.m_currentLimitErrorHi = m_linearLimits.m_currentLimitErrorHi[i];
			limot.m_enableMotor = m_linearLimits.m_enableMotor[i];
			limot.m_servoMotor = m_linearLimits.m_servoMotor[i];
			limot.m_servoTarget = m_linearLimits.m_servoTarget[i];
			limot.m_enableSpring = m_linearLimits.m_enableSpring[i];
			limot.m_springStiffness = m_linearLimits.m_springStiffness[i];
			limot.m_springStiffnessLimited = m_linearLimits.m_springStiffnessLimited[i];
			limot.m_springDamping = m_linearLimits.m_springDamping[i];
			limot.m_springDampingLimited = m_linearLimits.m_springDampingLimited[i];
			limot.m_equilibriumPoint = m_linearLimits.m_equilibriumPoint[i];
			limot.m_hiLimit = m_linearLimits.m_upperLimit[i];
			limot.m_loLimit = m_linearLimits.m_lowerLimit[i];
			limot.m_maxMotorForce = m_linearLimits.m_maxMotorForce[i];
			limot.m_targetVelocity = m_linearLimits.m_targetVelocity[i];
			Vec3 axis = m_calculatedTransformA.getBasis().getColumn ( i );
			i32 flags = m_flags >> ( i * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
			limot.m_stopCFM = ( flags & DRX3D_6DOF_FLAGS_CFM_STOP2 ) ? m_linearLimits.m_stopCFM[i] : info->cfm[0];
			limot.m_stopERP = ( flags & DRX3D_6DOF_FLAGS_ERP_STOP2 ) ? m_linearLimits.m_stopERP[i] : info->erp;
			limot.m_motorCFM = ( flags & DRX3D_6DOF_FLAGS_CFM_MOTO2 ) ? m_linearLimits.m_motorCFM[i] : info->cfm[0];
			limot.m_motorERP = ( flags & DRX3D_6DOF_FLAGS_ERP_MOTO2 ) ? m_linearLimits.m_motorERP[i] : info->erp;

			//rotAllowed is a bit of a magic from the original 6dof. The calculation of it here is something that imitates the original behavior as much as possible.
			i32 indx1 = ( i + 1 ) % 3;
			i32 indx2 = ( i + 2 ) % 3;
			i32 rotAllowed = 1;  // rotations around orthos to current axis (it is used only when one of the body is static)
#define D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION 1.0e-3
			bool indx1Violated = m_angularLimits[indx1].m_currentLimit == 1 ||
					m_angularLimits[indx1].m_currentLimit == 2 ||
					( m_angularLimits[indx1].m_currentLimit == 3 && ( m_angularLimits[indx1].m_currentLimitError < -D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION || m_angularLimits[indx1].m_currentLimitError > D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION ) ) ||
					( m_angularLimits[indx1].m_currentLimit == 4 && ( m_angularLimits[indx1].m_currentLimitError < -D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION || m_angularLimits[indx1].m_currentLimitErrorHi > D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION ) );
			bool indx2Violated = m_angularLimits[indx2].m_currentLimit == 1 ||
					m_angularLimits[indx2].m_currentLimit == 2 ||
					( m_angularLimits[indx2].m_currentLimit == 3 && ( m_angularLimits[indx2].m_currentLimitError < -D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION || m_angularLimits[indx2].m_currentLimitError > D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION ) ) ||
					( m_angularLimits[indx2].m_currentLimit == 4 && ( m_angularLimits[indx2].m_currentLimitError < -D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION || m_angularLimits[indx2].m_currentLimitErrorHi > D6_LIMIT_ERROR_THRESHOLD_FOR_ROTATION ) );

			if ( indx1Violated && indx2Violated )
			{
				rotAllowed = 0;
			}

			row += get_limit_motor_info2 ( &limot, transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 0, rotAllowed );
		}
	}

	return row;
}

i32 Generic6DofSpring2Constraint::setAngularLimits ( ConstraintInfo2* info, i32 row_offset, const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB )
{
	i32 row = row_offset;

	//order of rotational constraint rows
	i32 cIdx[] = {0, 1, 2};

	switch ( m_rotateOrder )
	{

		case RO_XYZ:
			cIdx[0] = 0;
			cIdx[1] = 1;
			cIdx[2] = 2;
			break;

		case RO_XZY:
			cIdx[0] = 0;
			cIdx[1] = 2;
			cIdx[2] = 1;
			break;

		case RO_YXZ:
			cIdx[0] = 1;
			cIdx[1] = 0;
			cIdx[2] = 2;
			break;

		case RO_YZX:
			cIdx[0] = 1;
			cIdx[1] = 2;
			cIdx[2] = 0;
			break;

		case RO_ZXY:
			cIdx[0] = 2;
			cIdx[1] = 0;
			cIdx[2] = 1;
			break;

		case RO_ZYX:
			cIdx[0] = 2;
			cIdx[1] = 1;
			cIdx[2] = 0;
			break;

		default:
			Assert ( false );
	}

	for ( i32 ii = 0; ii < 3; ii++ )
	{
		i32 i = cIdx[ii];

		if ( m_angularLimits[i].m_currentLimit || m_angularLimits[i].m_enableMotor || m_angularLimits[i].m_enableSpring )
		{
			Vec3 axis = getAxis ( i );
			i32 flags = m_flags >> ( ( i + 3 ) * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );

			if ( ! ( flags & DRX3D_6DOF_FLAGS_CFM_STOP2 ) )
			{
				m_angularLimits[i].m_stopCFM = info->cfm[0];
			}

			if ( ! ( flags & DRX3D_6DOF_FLAGS_ERP_STOP2 ) )
			{
				m_angularLimits[i].m_stopERP = info->erp;
			}

			if ( ! ( flags & DRX3D_6DOF_FLAGS_CFM_MOTO2 ) )
			{
				m_angularLimits[i].m_motorCFM = info->cfm[0];
			}

			if ( ! ( flags & DRX3D_6DOF_FLAGS_ERP_MOTO2 ) )
			{
				m_angularLimits[i].m_motorERP = info->erp;
			}

			row += get_limit_motor_info2 ( &m_angularLimits[i], transA, transB, linVelA, linVelB, angVelA, angVelB, info, row, axis, 1 );
		}
	}

	return row;
}

void Generic6DofSpring2Constraint::setFrames ( const Transform2& frameA, const Transform2& frameB )
{
	m_frameInA = frameA;
	m_frameInB = frameB;
	buildJacobian();
	calculateTransforms();
}

void Generic6DofSpring2Constraint::calculateLinearInfo()
{
	m_calculatedLinearDiff = m_calculatedTransformB.getOrigin() - m_calculatedTransformA.getOrigin();
	m_calculatedLinearDiff = m_calculatedTransformA.getBasis().inverse() * m_calculatedLinearDiff;

	for ( i32 i = 0; i < 3; i++ )
	{
		m_linearLimits.m_currentLinearDiff[i] = m_calculatedLinearDiff[i];
		m_linearLimits.testLimitValue ( i, m_calculatedLinearDiff[i] );
	}
}

void Generic6DofSpring2Constraint::calculateJacobi ( RotationalLimitMotor2* limot, const Transform2& transA, const Transform2& transB, ConstraintInfo2* info, i32 srow, Vec3& ax1, i32 rotational, i32 rotAllowed )
{
	Scalar* J1 = rotational ? info->m_J1angularAxis : info->m_J1linearAxis;
	Scalar* J2 = rotational ? info->m_J2angularAxis : info->m_J2linearAxis;

	J1[srow + 0] = ax1[0];
	J1[srow + 1] = ax1[1];
	J1[srow + 2] = ax1[2];

	J2[srow + 0] = -ax1[0];
	J2[srow + 1] = -ax1[1];
	J2[srow + 2] = -ax1[2];

	if ( !rotational )
	{
		Vec3 tmpA, tmpB, relA, relB;
		// get vector from bodyB to frameB in WCS
		relB = m_calculatedTransformB.getOrigin() - transB.getOrigin();
		// same for bodyA
		relA = m_calculatedTransformA.getOrigin() - transA.getOrigin();
		tmpA = relA.cross ( ax1 );
		tmpB = relB.cross ( ax1 );

		if ( m_hasStaticBody && ( !rotAllowed ) )
		{
			tmpA *= m_factA;
			tmpB *= m_factB;
		}

		i32 i;

		for ( i = 0; i < 3; i++ )
			info->m_J1angularAxis[srow + i] = tmpA[i];

		for ( i = 0; i < 3; i++ )
			info->m_J2angularAxis[srow + i] = -tmpB[i];
	}
}

i32 Generic6DofSpring2Constraint::get_limit_motor_info2 (
	RotationalLimitMotor2* limot,
	const Transform2& transA, const Transform2& transB, const Vec3& linVelA, const Vec3& linVelB, const Vec3& angVelA, const Vec3& angVelB,
	ConstraintInfo2* info, i32 row, Vec3& ax1, i32 rotational, i32 rotAllowed )
{
	i32 count = 0;
	i32 srow = row * info->rowskip;

	if ( limot->m_currentLimit == 4 )
	{
		Scalar vel = rotational ? angVelA.dot ( ax1 ) - angVelB.dot ( ax1 ) : linVelA.dot ( ax1 ) - linVelB.dot ( ax1 );

		calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );
		info->m_constraintError[srow] = info->fps * limot->m_stopERP * limot->m_currentLimitError * ( rotational ? -1 : 1 );

		if ( rotational )
		{
			if ( info->m_constraintError[srow] - vel * limot->m_stopERP > 0 )
			{
				Scalar bounceerror = -limot->m_bounce * vel;

				if ( bounceerror > info->m_constraintError[srow] )
					info->m_constraintError[srow] = bounceerror;
			}
		}

		else
		{
			if ( info->m_constraintError[srow] - vel * limot->m_stopERP < 0 )
			{
				Scalar bounceerror = -limot->m_bounce * vel;

				if ( bounceerror < info->m_constraintError[srow] )
					info->m_constraintError[srow] = bounceerror;
			}
		}

		info->m_lowerLimit[srow] = rotational ? 0 : -SIMD_INFINITY;

		info->m_upperLimit[srow] = rotational ? SIMD_INFINITY : 0;
		info->cfm[srow] = limot->m_stopCFM;
		srow += info->rowskip;
		++count;

		calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );
		info->m_constraintError[srow] = info->fps * limot->m_stopERP * limot->m_currentLimitErrorHi * ( rotational ? -1 : 1 );

		if ( rotational )
		{
			if ( info->m_constraintError[srow] - vel * limot->m_stopERP < 0 )
			{
				Scalar bounceerror = -limot->m_bounce * vel;

				if ( bounceerror < info->m_constraintError[srow] )
					info->m_constraintError[srow] = bounceerror;
			}
		}

		else
		{
			if ( info->m_constraintError[srow] - vel * limot->m_stopERP > 0 )
			{
				Scalar bounceerror = -limot->m_bounce * vel;

				if ( bounceerror > info->m_constraintError[srow] )
					info->m_constraintError[srow] = bounceerror;
			}
		}

		info->m_lowerLimit[srow] = rotational ? -SIMD_INFINITY : 0;

		info->m_upperLimit[srow] = rotational ? 0 : SIMD_INFINITY;
		info->cfm[srow] = limot->m_stopCFM;
		srow += info->rowskip;
		++count;
	}

	else
		if ( limot->m_currentLimit == 3 )
		{
			calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );
			info->m_constraintError[srow] = info->fps * limot->m_stopERP * limot->m_currentLimitError * ( rotational ? -1 : 1 );
			info->m_lowerLimit[srow] = -SIMD_INFINITY;
			info->m_upperLimit[srow] = SIMD_INFINITY;
			info->cfm[srow] = limot->m_stopCFM;
			srow += info->rowskip;
			++count;
		}

	if ( limot->m_enableMotor && !limot->m_servoMotor )
	{
		calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );
		Scalar tag_vel = rotational ? limot->m_targetVelocity : -limot->m_targetVelocity;
		Scalar mot_fact = getMotorFactor ( limot->m_currentPosition,
						  limot->m_loLimit,
						  limot->m_hiLimit,
						  tag_vel,
						  info->fps * limot->m_motorERP );
		info->m_constraintError[srow] = mot_fact * limot->m_targetVelocity;
		info->m_lowerLimit[srow] = -limot->m_maxMotorForce / info->fps;
		info->m_upperLimit[srow] = limot->m_maxMotorForce / info->fps;
		info->cfm[srow] = limot->m_motorCFM;
		srow += info->rowskip;
		++count;
	}

	if ( limot->m_enableMotor && limot->m_servoMotor )
	{
		Scalar error = limot->m_currentPosition - limot->m_servoTarget;
		Scalar curServoTarget = limot->m_servoTarget;

		if ( rotational )
		{
			if ( error > SIMD_PI )
			{
				error -= SIMD_2_PI;
				curServoTarget += SIMD_2_PI;
			}

			if ( error < -SIMD_PI )
			{
				error += SIMD_2_PI;
				curServoTarget -= SIMD_2_PI;
			}
		}

		calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );

		Scalar targetvelocity = error < 0 ? -limot->m_targetVelocity : limot->m_targetVelocity;
		Scalar tag_vel = -targetvelocity;
		Scalar mot_fact;

		if ( error != 0 )
		{
			Scalar lowLimit;
			Scalar hiLimit;

			if ( limot->m_loLimit > limot->m_hiLimit )
			{
				lowLimit = error > 0 ? curServoTarget : -SIMD_INFINITY;
				hiLimit = error < 0 ? curServoTarget : SIMD_INFINITY;
			}

			else
			{
				lowLimit = error > 0 && curServoTarget > limot->m_loLimit ? curServoTarget : limot->m_loLimit;
				hiLimit = error < 0 && curServoTarget < limot->m_hiLimit ? curServoTarget : limot->m_hiLimit;
			}

			mot_fact = getMotorFactor ( limot->m_currentPosition, lowLimit, hiLimit, tag_vel, info->fps * limot->m_motorERP );
		}

		else
		{
			mot_fact = 0;
		}

		info->m_constraintError[srow] = mot_fact * targetvelocity * ( rotational ? -1 : 1 );

		info->m_lowerLimit[srow] = -limot->m_maxMotorForce / info->fps;
		info->m_upperLimit[srow] = limot->m_maxMotorForce / info->fps;
		info->cfm[srow] = limot->m_motorCFM;
		srow += info->rowskip;
		++count;
	}

	if ( limot->m_enableSpring )
	{
		Scalar error = limot->m_currentPosition - limot->m_equilibriumPoint;
		calculateJacobi ( limot, transA, transB, info, srow, ax1, rotational, rotAllowed );

		//Scalar cfm = 1.0 / ((1.0/info->fps)*limot->m_springStiffness+ limot->m_springDamping);
		//if(cfm > 0.99999)
		//	cfm = 0.99999;
		//Scalar erp = (1.0/info->fps)*limot->m_springStiffness / ((1.0/info->fps)*limot->m_springStiffness + limot->m_springDamping);
		//info->m_constraintError[srow] = info->fps * erp * error * (rotational ? -1.0 : 1.0);
		//info->m_lowerLimit[srow] = -SIMD_INFINITY;
		//info->m_upperLimit[srow] = SIMD_INFINITY;

		Scalar dt = DRX3D_ONE / info->fps;
		Scalar kd = limot->m_springDamping;
		Scalar ks = limot->m_springStiffness;
		Scalar vel;

		if ( rotational )
		{
			vel = angVelA.dot ( ax1 ) - angVelB.dot ( ax1 );
		}

		else
		{
			Vec3 tanVelA = angVelA.cross ( m_calculatedTransformA.getOrigin() - transA.getOrigin() );
			Vec3 tanVelB = angVelB.cross ( m_calculatedTransformB.getOrigin() - transB.getOrigin() );
			vel = ( linVelA + tanVelA ).dot ( ax1 ) - ( linVelB + tanVelB ).dot ( ax1 );
		}

		Scalar cfm = DRX3D_ZERO;

		Scalar mA = DRX3D_ONE / m_rbA.getInvMass();
		Scalar mB = DRX3D_ONE / m_rbB.getInvMass();

		if ( rotational )
		{
			Scalar rrA = ( m_calculatedTransformA.getOrigin() - transA.getOrigin() ).length2();
			Scalar rrB = ( m_calculatedTransformB.getOrigin() - transB.getOrigin() ).length2();

			if ( m_rbA.getInvMass() )
				mA = mA * rrA + 1 / ( m_rbA.getInvInertiaTensorWorld() * ax1 ).length();

			if ( m_rbB.getInvMass() )
				mB = mB * rrB + 1 / ( m_rbB.getInvInertiaTensorWorld() * ax1 ).length();
		}

		Scalar m;

		if ( m_rbA.getInvMass() == 0 )
			m = mB;
		else
			if ( m_rbB.getInvMass() == 0 )
				m = mA;
			else
				m = mA * mB / ( mA + mB );

		Scalar angularfreq = Sqrt ( ks / m );

		//limit stiffness (the spring should not be sampled faster that the quarter of its angular frequency)
		if ( limot->m_springStiffnessLimited && 0.25 < angularfreq * dt )
		{
			ks = DRX3D_ONE / dt / dt / Scalar ( 16.0 ) * m;
		}

		//avoid damping that would blow up the spring

		if ( limot->m_springDampingLimited && kd * dt > m )
		{
			kd = m / dt;
		}

		Scalar fs = ks * error * dt;

		Scalar fd = -kd * ( vel ) * ( rotational ? -1 : 1 ) * dt;
		Scalar f = ( fs + fd );

		// after the spring force affecting the body(es) the new velocity will be
		// vel + f / m * (rotational ? -1 : 1)
		// so in theory this should be set here for m_constraintError
		// (with m_constraintError we set a desired velocity for the affected body(es))
		// however in practice any value is fine as long as it is greater than the "proper" velocity,
		// because the m_lowerLimit and the m_upperLimit will determinate the strength of the final pulling force
		// so it is much simpler (and more robust) just to simply use inf (with the proper sign)
		// (Even with our best intent the "new" velocity is only an estimation. If we underestimate
		// the "proper" velocity that will weaken the spring, however if we overestimate it, it doesn't
		// matter, because the solver will limit it according the force limit)
		// you may also wonder what if the current velocity (vel) so high that the pulling force will not change its direction (in this iteration)
		// will we not request a velocity with the wrong direction ?
		// and the answer is not, because in practice during the solving the current velocity is subtracted from the m_constraintError
		// so the sign of the force that is really matters

		if ( m_flags & DRX3D_6DOF_FLAGS_USE_INFINITE_ERROR )
			info->m_constraintError[srow] = ( rotational ? -1 : 1 ) * ( f < 0 ? -SIMD_INFINITY : SIMD_INFINITY );
		else
			info->m_constraintError[srow] = vel + f / m * ( rotational ? -1 : 1 );

		Scalar minf = f < fd ? f : fd;

		Scalar maxf = f < fd ? fd : f;

		if ( !rotational )
		{
			info->m_lowerLimit[srow] = minf > 0 ? 0 : minf;
			info->m_upperLimit[srow] = maxf < 0 ? 0 : maxf;
		}

		else
		{
			info->m_lowerLimit[srow] = -maxf > 0 ? 0 : -maxf;
			info->m_upperLimit[srow] = -minf < 0 ? 0 : -minf;
		}

		info->cfm[srow] = cfm;

		srow += info->rowskip;
		++count;
	}

	return count;
}

//override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
//If no axis is provided, it uses the default axis for this constraint.
void Generic6DofSpring2Constraint::setParam ( i32 num, Scalar value, i32 axis )
{
	if ( ( axis >= 0 ) && ( axis < 3 ) )
	{
		switch ( num )
		{

			case DRX3D_CONSTRAINT_STOP_ERP:
				m_linearLimits.m_stopERP[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_ERP_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
				break;

			case DRX3D_CONSTRAINT_STOP_CFM:
				m_linearLimits.m_stopCFM[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
				break;

			case DRX3D_CONSTRAINT_ERP:
				m_linearLimits.m_motorERP[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_ERP_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
				break;

			case DRX3D_CONSTRAINT_CFM:
				m_linearLimits.m_motorCFM[axis] = value;
				m_flags |= DRX3D_6DOF_FLAGS_CFM_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
				break;

			default:
				AssertConstrParams ( 0 );
		}
	}

	else
		if ( ( axis >= 3 ) && ( axis < 6 ) )
		{
			switch ( num )
			{

				case DRX3D_CONSTRAINT_STOP_ERP:
					m_angularLimits[axis - 3].m_stopERP = value;
					m_flags |= DRX3D_6DOF_FLAGS_ERP_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
					break;

				case DRX3D_CONSTRAINT_STOP_CFM:
					m_angularLimits[axis - 3].m_stopCFM = value;
					m_flags |= DRX3D_6DOF_FLAGS_CFM_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
					break;

				case DRX3D_CONSTRAINT_ERP:
					m_angularLimits[axis - 3].m_motorERP = value;
					m_flags |= DRX3D_6DOF_FLAGS_ERP_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
					break;

				case DRX3D_CONSTRAINT_CFM:
					m_angularLimits[axis - 3].m_motorCFM = value;
					m_flags |= DRX3D_6DOF_FLAGS_CFM_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 );
					break;

				default:
					AssertConstrParams ( 0 );
			}
		}

		else
		{
			AssertConstrParams ( 0 );
		}
}

//return the local value of parameter
Scalar Generic6DofSpring2Constraint::getParam ( i32 num, i32 axis ) const
{
	Scalar retVal = 0;

	if ( ( axis >= 0 ) && ( axis < 3 ) )
	{
		switch ( num )
		{

			case DRX3D_CONSTRAINT_STOP_ERP:
				AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_ERP_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
				retVal = m_linearLimits.m_stopERP[axis];
				break;

			case DRX3D_CONSTRAINT_STOP_CFM:
				AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_CFM_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
				retVal = m_linearLimits.m_stopCFM[axis];
				break;

			case DRX3D_CONSTRAINT_ERP:
				AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_ERP_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
				retVal = m_linearLimits.m_motorERP[axis];
				break;

			case DRX3D_CONSTRAINT_CFM:
				AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_CFM_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
				retVal = m_linearLimits.m_motorCFM[axis];
				break;

			default:
				AssertConstrParams ( 0 );
		}
	}

	else
		if ( ( axis >= 3 ) && ( axis < 6 ) )
		{
			switch ( num )
			{

				case DRX3D_CONSTRAINT_STOP_ERP:
					AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_ERP_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
					retVal = m_angularLimits[axis - 3].m_stopERP;
					break;

				case DRX3D_CONSTRAINT_STOP_CFM:
					AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_CFM_STOP2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
					retVal = m_angularLimits[axis - 3].m_stopCFM;
					break;

				case DRX3D_CONSTRAINT_ERP:
					AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_ERP_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
					retVal = m_angularLimits[axis - 3].m_motorERP;
					break;

				case DRX3D_CONSTRAINT_CFM:
					AssertConstrParams ( m_flags & ( DRX3D_6DOF_FLAGS_CFM_MOTO2 << ( axis * DRX3D_6DOF_FLAGS_AXIS_SHIFT2 ) ) );
					retVal = m_angularLimits[axis - 3].m_motorCFM;
					break;

				default:
					AssertConstrParams ( 0 );
			}
		}

		else
		{
			AssertConstrParams ( 0 );
		}

	return retVal;
}

void Generic6DofSpring2Constraint::setAxis ( const Vec3& axis1, const Vec3& axis2 )
{
	Vec3 zAxis = axis1.normalized();
	Vec3 yAxis = axis2.normalized();
	Vec3 xAxis = yAxis.cross ( zAxis );  // we want right coordinate system

	Transform2 frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setVal ( xAxis[0], yAxis[0], zAxis[0],
			xAxis[1], yAxis[1], zAxis[1],
			xAxis[2], yAxis[2], zAxis[2] );

	// now get constraint frame in local coordinate systems
	m_frameInA = m_rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = m_rbB.getCenterOfMassTransform().inverse() * frameInW;

	calculateTransforms();
}

void Generic6DofSpring2Constraint::setBounce ( i32 index, Scalar bounce )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_bounce[index] = bounce;
	else
		m_angularLimits[index - 3].m_bounce = bounce;
}

void Generic6DofSpring2Constraint::enableMotor ( i32 index, bool onOff )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_enableMotor[index] = onOff;
	else
		m_angularLimits[index - 3].m_enableMotor = onOff;
}

void Generic6DofSpring2Constraint::setServo ( i32 index, bool onOff )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_servoMotor[index] = onOff;
	else
		m_angularLimits[index - 3].m_servoMotor = onOff;
}

void Generic6DofSpring2Constraint::setTargetVelocity ( i32 index, Scalar velocity )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_targetVelocity[index] = velocity;
	else
		m_angularLimits[index - 3].m_targetVelocity = velocity;
}

void Generic6DofSpring2Constraint::setServoTarget ( i32 index, Scalar targetOrg )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
	{
		m_linearLimits.m_servoTarget[index] = targetOrg;
	}

	else
	{
		//wrap between -PI and PI, see also
		//https://stackoverflow.com/questions/4633177/c-how-to-wrap-a-float-to-the-interval-pi-pi

		Scalar target = targetOrg + SIMD_PI;

		if ( 1 )
		{
			Scalar m = target - SIMD_2_PI * std::floor ( target / SIMD_2_PI );
			// handle boundary cases resulted from floating-point cut off:
			{
				if ( m >= SIMD_2_PI )
				{
					target = 0;
				}

				else
				{
					if ( m < 0 )
					{
						if ( SIMD_2_PI + m == SIMD_2_PI )
							target = 0;
						else
							target = SIMD_2_PI + m;
					}

					else
					{
						target = m;
					}
				}
			}

			target -= SIMD_PI;
		}

		m_angularLimits[index - 3].m_servoTarget = target;
	}
}

void Generic6DofSpring2Constraint::setMaxMotorForce ( i32 index, Scalar force )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_maxMotorForce[index] = force;
	else
		m_angularLimits[index - 3].m_maxMotorForce = force;
}

void Generic6DofSpring2Constraint::enableSpring ( i32 index, bool onOff )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_enableSpring[index] = onOff;
	else
		m_angularLimits[index - 3].m_enableSpring = onOff;
}

void Generic6DofSpring2Constraint::setStiffness ( i32 index, Scalar stiffness, bool limitIfNeeded )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
	{
		m_linearLimits.m_springStiffness[index] = stiffness;
		m_linearLimits.m_springStiffnessLimited[index] = limitIfNeeded;
	}

	else
	{
		m_angularLimits[index - 3].m_springStiffness = stiffness;
		m_angularLimits[index - 3].m_springStiffnessLimited = limitIfNeeded;
	}
}

void Generic6DofSpring2Constraint::setDamping ( i32 index, Scalar damping, bool limitIfNeeded )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
	{
		m_linearLimits.m_springDamping[index] = damping;
		m_linearLimits.m_springDampingLimited[index] = limitIfNeeded;
	}

	else
	{
		m_angularLimits[index - 3].m_springDamping = damping;
		m_angularLimits[index - 3].m_springDampingLimited = limitIfNeeded;
	}
}

void Generic6DofSpring2Constraint::setEquilibriumPoint()
{
	calculateTransforms();
	i32 i;

	for ( i = 0; i < 3; i++ )
		m_linearLimits.m_equilibriumPoint[i] = m_calculatedLinearDiff[i];

	for ( i = 0; i < 3; i++ )
		m_angularLimits[i].m_equilibriumPoint = m_calculatedAxisAngleDiff[i];
}

void Generic6DofSpring2Constraint::setEquilibriumPoint ( i32 index )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );
	calculateTransforms();

	if ( index < 3 )
		m_linearLimits.m_equilibriumPoint[index] = m_calculatedLinearDiff[index];
	else
		m_angularLimits[index - 3].m_equilibriumPoint = m_calculatedAxisAngleDiff[index - 3];
}

void Generic6DofSpring2Constraint::setEquilibriumPoint ( i32 index, Scalar val )
{
	Assert ( ( index >= 0 ) && ( index < 6 ) );

	if ( index < 3 )
		m_linearLimits.m_equilibriumPoint[index] = val;
	else
		m_angularLimits[index - 3].m_equilibriumPoint = val;
}

//////////////////////////// RotationalLimitMotor2 ////////////////////////////////////

void RotationalLimitMotor2::testLimitValue ( Scalar test_value )
{
	//we can't normalize the angles here because we would lost the sign that we use later, but it doesn't seem to be a problem
	if ( m_loLimit > m_hiLimit )
	{
		m_currentLimit = 0;
		m_currentLimitError = Scalar ( 0.f );
	}

	else
		if ( m_loLimit == m_hiLimit )
		{
			m_currentLimitError = test_value - m_loLimit;
			m_currentLimit = 3;
		}

		else
		{
			m_currentLimitError = test_value - m_loLimit;
			m_currentLimitErrorHi = test_value - m_hiLimit;
			m_currentLimit = 4;
		}
}

//////////////////////////// TranslationalLimitMotor2 ////////////////////////////////////

void TranslationalLimitMotor2::testLimitValue ( i32 limitIndex, Scalar test_value )
{
	Scalar loLimit = m_lowerLimit[limitIndex];
	Scalar hiLimit = m_upperLimit[limitIndex];

	if ( loLimit > hiLimit )
	{
		m_currentLimitError[limitIndex] = 0;
		m_currentLimit[limitIndex] = 0;
	}

	else
		if ( loLimit == hiLimit )
		{
			m_currentLimitError[limitIndex] = test_value - loLimit;
			m_currentLimit[limitIndex] = 3;
		}

		else
		{
			m_currentLimitError[limitIndex] = test_value - loLimit;
			m_currentLimitErrorHi[limitIndex] = test_value - hiLimit;
			m_currentLimit[limitIndex] = 4;
		}
}
