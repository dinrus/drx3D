#include <drx3D/Physics/Dynamics/ConstraintSolver/ConeTwistConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <cmath>
#include <new>

//#define CONETWIST_USE_OBSOLETE_SOLVER true
#define CONETWIST_USE_OBSOLETE_SOLVER false
#define CONETWIST_DEF_FIX_THRESH Scalar(.05f)

SIMD_FORCE_INLINE Scalar computeAngularImpulseDenominator(const Vec3& axis, const Matrix3x3& invInertiaWorld)
{
	Vec3 vec = axis * invInertiaWorld;
	return axis.dot(vec);
}

ConeTwistConstraint::ConeTwistConstraint(RigidBody& rbA, RigidBody& rbB,
											 const Transform2& rbAFrame, const Transform2& rbBFrame)
	: TypedConstraint(CONETWIST_CONSTRAINT_TYPE, rbA, rbB), m_rbAFrame(rbAFrame), m_rbBFrame(rbBFrame), m_angularOnly(false), m_useSolveConstraintObsolete(CONETWIST_USE_OBSOLETE_SOLVER)
{
	init();
}

ConeTwistConstraint::ConeTwistConstraint(RigidBody& rbA, const Transform2& rbAFrame)
	: TypedConstraint(CONETWIST_CONSTRAINT_TYPE, rbA), m_rbAFrame(rbAFrame), m_angularOnly(false), m_useSolveConstraintObsolete(CONETWIST_USE_OBSOLETE_SOLVER)
{
	m_rbBFrame = m_rbAFrame;
	m_rbBFrame.setOrigin(Vec3(0., 0., 0.));
	init();
}

void ConeTwistConstraint::init()
{
	m_angularOnly = false;
	m_solveTwistLimit = false;
	m_solveSwingLimit = false;
	m_bMotorEnabled = false;
	m_maxMotorImpulse = Scalar(-1);

	setLimit(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	m_damping = Scalar(0.01);
	m_fixThresh = CONETWIST_DEF_FIX_THRESH;
	m_flags = 0;
	m_linCFM = Scalar(0.f);
	m_linERP = Scalar(0.7f);
	m_angCFM = Scalar(0.f);
}

void ConeTwistConstraint::getInfo1(ConstraintInfo1* info)
{
	if (m_useSolveConstraintObsolete)
	{
		info->m_numConstraintRows = 0;
		info->nub = 0;
	}
	else
	{
		info->m_numConstraintRows = 3;
		info->nub = 3;
		calcAngleInfo2(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform(), m_rbA.getInvInertiaTensorWorld(), m_rbB.getInvInertiaTensorWorld());
		if (m_solveSwingLimit)
		{
			info->m_numConstraintRows++;
			info->nub--;
			if ((m_swingSpan1 < m_fixThresh) && (m_swingSpan2 < m_fixThresh))
			{
				info->m_numConstraintRows++;
				info->nub--;
			}
		}
		if (m_solveTwistLimit)
		{
			info->m_numConstraintRows++;
			info->nub--;
		}
	}
}

void ConeTwistConstraint::getInfo1NonVirtual(ConstraintInfo1* info)
{
	//always reserve 6 rows: object transform is not available on SPU
	info->m_numConstraintRows = 6;
	info->nub = 0;
}

void ConeTwistConstraint::getInfo2(ConstraintInfo2* info)
{
	getInfo2NonVirtual(info, m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform(), m_rbA.getInvInertiaTensorWorld(), m_rbB.getInvInertiaTensorWorld());
}

void ConeTwistConstraint::getInfo2NonVirtual(ConstraintInfo2* info, const Transform2& transA, const Transform2& transB, const Matrix3x3& invInertiaWorldA, const Matrix3x3& invInertiaWorldB)
{
	calcAngleInfo2(transA, transB, invInertiaWorldA, invInertiaWorldB);

	Assert(!m_useSolveConstraintObsolete);
	// set jacobian
	info->m_J1linearAxis[0] = 1;
	info->m_J1linearAxis[info->rowskip + 1] = 1;
	info->m_J1linearAxis[2 * info->rowskip + 2] = 1;
	Vec3 a1 = transA.getBasis() * m_rbAFrame.getOrigin();
	{
		Vec3* angular0 = (Vec3*)(info->m_J1angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J1angularAxis + info->rowskip);
		Vec3* angular2 = (Vec3*)(info->m_J1angularAxis + 2 * info->rowskip);
		Vec3 a1neg = -a1;
		a1neg.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}
	info->m_J2linearAxis[0] = -1;
	info->m_J2linearAxis[info->rowskip + 1] = -1;
	info->m_J2linearAxis[2 * info->rowskip + 2] = -1;
	Vec3 a2 = transB.getBasis() * m_rbBFrame.getOrigin();
	{
		Vec3* angular0 = (Vec3*)(info->m_J2angularAxis);
		Vec3* angular1 = (Vec3*)(info->m_J2angularAxis + info->rowskip);
		Vec3* angular2 = (Vec3*)(info->m_J2angularAxis + 2 * info->rowskip);
		a2.getSkewSymmetricMatrix(angular0, angular1, angular2);
	}
	// set right hand side
	Scalar linERP = (m_flags & DRX3D_CONETWIST_FLAGS_LIN_ERP) ? m_linERP : info->erp;
	Scalar k = info->fps * linERP;
	i32 j;
	for (j = 0; j < 3; j++)
	{
		info->m_constraintError[j * info->rowskip] = k * (a2[j] + transB.getOrigin()[j] - a1[j] - transA.getOrigin()[j]);
		info->m_lowerLimit[j * info->rowskip] = -SIMD_INFINITY;
		info->m_upperLimit[j * info->rowskip] = SIMD_INFINITY;
		if (m_flags & DRX3D_CONETWIST_FLAGS_LIN_CFM)
		{
			info->cfm[j * info->rowskip] = m_linCFM;
		}
	}
	i32 row = 3;
	i32 srow = row * info->rowskip;
	Vec3 ax1;
	// angular limits
	if (m_solveSwingLimit)
	{
		Scalar* J1 = info->m_J1angularAxis;
		Scalar* J2 = info->m_J2angularAxis;
		if ((m_swingSpan1 < m_fixThresh) && (m_swingSpan2 < m_fixThresh))
		{
			Transform2 trA = transA * m_rbAFrame;
			Vec3 p = trA.getBasis().getColumn(1);
			Vec3 q = trA.getBasis().getColumn(2);
			i32 srow1 = srow + info->rowskip;
			J1[srow + 0] = p[0];
			J1[srow + 1] = p[1];
			J1[srow + 2] = p[2];
			J1[srow1 + 0] = q[0];
			J1[srow1 + 1] = q[1];
			J1[srow1 + 2] = q[2];
			J2[srow + 0] = -p[0];
			J2[srow + 1] = -p[1];
			J2[srow + 2] = -p[2];
			J2[srow1 + 0] = -q[0];
			J2[srow1 + 1] = -q[1];
			J2[srow1 + 2] = -q[2];
			Scalar fact = info->fps * m_relaxationFactor;
			info->m_constraintError[srow] = fact * m_swingAxis.dot(p);
			info->m_constraintError[srow1] = fact * m_swingAxis.dot(q);
			info->m_lowerLimit[srow] = -SIMD_INFINITY;
			info->m_upperLimit[srow] = SIMD_INFINITY;
			info->m_lowerLimit[srow1] = -SIMD_INFINITY;
			info->m_upperLimit[srow1] = SIMD_INFINITY;
			srow = srow1 + info->rowskip;
		}
		else
		{
			ax1 = m_swingAxis * m_relaxationFactor * m_relaxationFactor;
			J1[srow + 0] = ax1[0];
			J1[srow + 1] = ax1[1];
			J1[srow + 2] = ax1[2];
			J2[srow + 0] = -ax1[0];
			J2[srow + 1] = -ax1[1];
			J2[srow + 2] = -ax1[2];
			Scalar k = info->fps * m_biasFactor;

			info->m_constraintError[srow] = k * m_swingCorrection;
			if (m_flags & DRX3D_CONETWIST_FLAGS_ANG_CFM)
			{
				info->cfm[srow] = m_angCFM;
			}
			// m_swingCorrection is always positive or 0
			info->m_lowerLimit[srow] = 0;
			info->m_upperLimit[srow] = (m_bMotorEnabled && m_maxMotorImpulse >= 0.0f) ? m_maxMotorImpulse : SIMD_INFINITY;
			srow += info->rowskip;
		}
	}
	if (m_solveTwistLimit)
	{
		ax1 = m_twistAxis * m_relaxationFactor * m_relaxationFactor;
		Scalar* J1 = info->m_J1angularAxis;
		Scalar* J2 = info->m_J2angularAxis;
		J1[srow + 0] = ax1[0];
		J1[srow + 1] = ax1[1];
		J1[srow + 2] = ax1[2];
		J2[srow + 0] = -ax1[0];
		J2[srow + 1] = -ax1[1];
		J2[srow + 2] = -ax1[2];
		Scalar k = info->fps * m_biasFactor;
		info->m_constraintError[srow] = k * m_twistCorrection;
		if (m_flags & DRX3D_CONETWIST_FLAGS_ANG_CFM)
		{
			info->cfm[srow] = m_angCFM;
		}
		if (m_twistSpan > 0.0f)
		{
			if (m_twistCorrection > 0.0f)
			{
				info->m_lowerLimit[srow] = 0;
				info->m_upperLimit[srow] = SIMD_INFINITY;
			}
			else
			{
				info->m_lowerLimit[srow] = -SIMD_INFINITY;
				info->m_upperLimit[srow] = 0;
			}
		}
		else
		{
			info->m_lowerLimit[srow] = -SIMD_INFINITY;
			info->m_upperLimit[srow] = SIMD_INFINITY;
		}
		srow += info->rowskip;
	}
}

void ConeTwistConstraint::buildJacobian()
{
	if (m_useSolveConstraintObsolete)
	{
		m_appliedImpulse = Scalar(0.);
		m_accTwistLimitImpulse = Scalar(0.);
		m_accSwingLimitImpulse = Scalar(0.);
		m_accMotorImpulse = Vec3(0., 0., 0.);

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

		calcAngleInfo2(m_rbA.getCenterOfMassTransform(), m_rbB.getCenterOfMassTransform(), m_rbA.getInvInertiaTensorWorld(), m_rbB.getInvInertiaTensorWorld());
	}
}

void ConeTwistConstraint::solveConstraintObsolete(SolverBody& bodyA, SolverBody& bodyB, Scalar timeStep)
{
#ifndef __SPU__
	if (m_useSolveConstraintObsolete)
	{
		Vec3 pivotAInW = m_rbA.getCenterOfMassTransform() * m_rbAFrame.getOrigin();
		Vec3 pivotBInW = m_rbB.getCenterOfMassTransform() * m_rbBFrame.getOrigin();

		Scalar tau = Scalar(0.3);

		//linear part
		if (!m_angularOnly)
		{
			Vec3 rel_pos1 = pivotAInW - m_rbA.getCenterOfMassPosition();
			Vec3 rel_pos2 = pivotBInW - m_rbB.getCenterOfMassPosition();

			Vec3 vel1;
			bodyA.internalGetVelocityInLocalPointObsolete(rel_pos1, vel1);
			Vec3 vel2;
			bodyB.internalGetVelocityInLocalPointObsolete(rel_pos2, vel2);
			Vec3 vel = vel1 - vel2;

			for (i32 i = 0; i < 3; i++)
			{
				const Vec3& normal = m_jac[i].m_linearJointAxis;
				Scalar jacDiagABInv = Scalar(1.) / m_jac[i].getDiagonal();

				Scalar rel_vel;
				rel_vel = normal.dot(vel);
				//positional error (zeroth order error)
				Scalar depth = -(pivotAInW - pivotBInW).dot(normal);  //this is the error projected on the normal
				Scalar impulse = depth * tau / timeStep * jacDiagABInv - rel_vel * jacDiagABInv;
				m_appliedImpulse += impulse;

				Vec3 ftorqueAxis1 = rel_pos1.cross(normal);
				Vec3 ftorqueAxis2 = rel_pos2.cross(normal);
				bodyA.internalApplyImpulse(normal * m_rbA.getInvMass(), m_rbA.getInvInertiaTensorWorld() * ftorqueAxis1, impulse);
				bodyB.internalApplyImpulse(normal * m_rbB.getInvMass(), m_rbB.getInvInertiaTensorWorld() * ftorqueAxis2, -impulse);
			}
		}

		// apply motor
		if (m_bMotorEnabled)
		{
			// compute current and predicted transforms
			Transform2 trACur = m_rbA.getCenterOfMassTransform();
			Transform2 trBCur = m_rbB.getCenterOfMassTransform();
			Vec3 omegaA;
			bodyA.internalGetAngularVelocity(omegaA);
			Vec3 omegaB;
			bodyB.internalGetAngularVelocity(omegaB);
			Transform2 trAPred;
			trAPred.setIdentity();
			Vec3 zerovec(0, 0, 0);
			Transform2Util::integrateTransform(
				trACur, zerovec, omegaA, timeStep, trAPred);
			Transform2 trBPred;
			trBPred.setIdentity();
			Transform2Util::integrateTransform(
				trBCur, zerovec, omegaB, timeStep, trBPred);

			// compute desired transforms in world
			Transform2 trPose(m_qTarget);
			Transform2 trABDes = m_rbBFrame * trPose * m_rbAFrame.inverse();
			Transform2 trADes = trBPred * trABDes;
			Transform2 trBDes = trAPred * trABDes.inverse();

			// compute desired omegas in world
			Vec3 omegaADes, omegaBDes;

			Transform2Util::calculateVelocity(trACur, trADes, timeStep, zerovec, omegaADes);
			Transform2Util::calculateVelocity(trBCur, trBDes, timeStep, zerovec, omegaBDes);

			// compute delta omegas
			Vec3 dOmegaA = omegaADes - omegaA;
			Vec3 dOmegaB = omegaBDes - omegaB;

			// compute weighted avg axis of dOmega (weighting based on inertias)
			Vec3 axisA, axisB;
			Scalar kAxisAInv = 0, kAxisBInv = 0;

			if (dOmegaA.length2() > SIMD_EPSILON)
			{
				axisA = dOmegaA.normalized();
				kAxisAInv = getRigidBodyA().computeAngularImpulseDenominator(axisA);
			}

			if (dOmegaB.length2() > SIMD_EPSILON)
			{
				axisB = dOmegaB.normalized();
				kAxisBInv = getRigidBodyB().computeAngularImpulseDenominator(axisB);
			}

			Vec3 avgAxis = kAxisAInv * axisA + kAxisBInv * axisB;

			static bool bDoTorque = true;
			if (bDoTorque && avgAxis.length2() > SIMD_EPSILON)
			{
				avgAxis.normalize();
				kAxisAInv = getRigidBodyA().computeAngularImpulseDenominator(avgAxis);
				kAxisBInv = getRigidBodyB().computeAngularImpulseDenominator(avgAxis);
				Scalar kInvCombined = kAxisAInv + kAxisBInv;

				Vec3 impulse = (kAxisAInv * dOmegaA - kAxisBInv * dOmegaB) /
									(kInvCombined * kInvCombined);

				if (m_maxMotorImpulse >= 0)
				{
					Scalar fMaxImpulse = m_maxMotorImpulse;
					if (m_bNormalizedMotorStrength)
						fMaxImpulse = fMaxImpulse / kAxisAInv;

					Vec3 newUnclampedAccImpulse = m_accMotorImpulse + impulse;
					Scalar newUnclampedMag = newUnclampedAccImpulse.length();
					if (newUnclampedMag > fMaxImpulse)
					{
						newUnclampedAccImpulse.normalize();
						newUnclampedAccImpulse *= fMaxImpulse;
						impulse = newUnclampedAccImpulse - m_accMotorImpulse;
					}
					m_accMotorImpulse += impulse;
				}

				Scalar impulseMag = impulse.length();
				Vec3 impulseAxis = impulse / impulseMag;

				bodyA.internalApplyImpulse(Vec3(0, 0, 0), m_rbA.getInvInertiaTensorWorld() * impulseAxis, impulseMag);
				bodyB.internalApplyImpulse(Vec3(0, 0, 0), m_rbB.getInvInertiaTensorWorld() * impulseAxis, -impulseMag);
			}
		}
		else if (m_damping > SIMD_EPSILON)  // no motor: do a little damping
		{
			Vec3 angVelA;
			bodyA.internalGetAngularVelocity(angVelA);
			Vec3 angVelB;
			bodyB.internalGetAngularVelocity(angVelB);
			Vec3 relVel = angVelB - angVelA;
			if (relVel.length2() > SIMD_EPSILON)
			{
				Vec3 relVelAxis = relVel.normalized();
				Scalar m_kDamping = Scalar(1.) /
									  (getRigidBodyA().computeAngularImpulseDenominator(relVelAxis) +
									   getRigidBodyB().computeAngularImpulseDenominator(relVelAxis));
				Vec3 impulse = m_damping * m_kDamping * relVel;

				Scalar impulseMag = impulse.length();
				Vec3 impulseAxis = impulse / impulseMag;
				bodyA.internalApplyImpulse(Vec3(0, 0, 0), m_rbA.getInvInertiaTensorWorld() * impulseAxis, impulseMag);
				bodyB.internalApplyImpulse(Vec3(0, 0, 0), m_rbB.getInvInertiaTensorWorld() * impulseAxis, -impulseMag);
			}
		}

		// joint limits
		{
			///solve angular part
			Vec3 angVelA;
			bodyA.internalGetAngularVelocity(angVelA);
			Vec3 angVelB;
			bodyB.internalGetAngularVelocity(angVelB);

			// solve swing limit
			if (m_solveSwingLimit)
			{
				Scalar amplitude = m_swingLimitRatio * m_swingCorrection * m_biasFactor / timeStep;
				Scalar relSwingVel = (angVelB - angVelA).dot(m_swingAxis);
				if (relSwingVel > 0)
					amplitude += m_swingLimitRatio * relSwingVel * m_relaxationFactor;
				Scalar impulseMag = amplitude * m_kSwing;

				// Clamp the accumulated impulse
				Scalar temp = m_accSwingLimitImpulse;
				m_accSwingLimitImpulse = d3Max(m_accSwingLimitImpulse + impulseMag, Scalar(0.0));
				impulseMag = m_accSwingLimitImpulse - temp;

				Vec3 impulse = m_swingAxis * impulseMag;

				// don't let cone response affect twist
				// (this can happen since body A's twist doesn't match body B's AND we use an elliptical cone limit)
				{
					Vec3 impulseTwistCouple = impulse.dot(m_twistAxisA) * m_twistAxisA;
					Vec3 impulseNoTwistCouple = impulse - impulseTwistCouple;
					impulse = impulseNoTwistCouple;
				}

				impulseMag = impulse.length();
				Vec3 noTwistSwingAxis = impulse / impulseMag;

				bodyA.internalApplyImpulse(Vec3(0, 0, 0), m_rbA.getInvInertiaTensorWorld() * noTwistSwingAxis, impulseMag);
				bodyB.internalApplyImpulse(Vec3(0, 0, 0), m_rbB.getInvInertiaTensorWorld() * noTwistSwingAxis, -impulseMag);
			}

			// solve twist limit
			if (m_solveTwistLimit)
			{
				Scalar amplitude = m_twistLimitRatio * m_twistCorrection * m_biasFactor / timeStep;
				Scalar relTwistVel = (angVelB - angVelA).dot(m_twistAxis);
				if (relTwistVel > 0)  // only damp when moving towards limit (m_twistAxis flipping is important)
					amplitude += m_twistLimitRatio * relTwistVel * m_relaxationFactor;
				Scalar impulseMag = amplitude * m_kTwist;

				// Clamp the accumulated impulse
				Scalar temp = m_accTwistLimitImpulse;
				m_accTwistLimitImpulse = d3Max(m_accTwistLimitImpulse + impulseMag, Scalar(0.0));
				impulseMag = m_accTwistLimitImpulse - temp;

				//		Vec3 impulse = m_twistAxis * impulseMag;

				bodyA.internalApplyImpulse(Vec3(0, 0, 0), m_rbA.getInvInertiaTensorWorld() * m_twistAxis, impulseMag);
				bodyB.internalApplyImpulse(Vec3(0, 0, 0), m_rbB.getInvInertiaTensorWorld() * m_twistAxis, -impulseMag);
			}
		}
	}
#else
	Assert(0);
#endif  //__SPU__
}

void ConeTwistConstraint::updateRHS(Scalar timeStep)
{
	(void)timeStep;
}

#ifndef __SPU__
void ConeTwistConstraint::calcAngleInfo()
{
	m_swingCorrection = Scalar(0.);
	m_twistLimitSign = Scalar(0.);
	m_solveTwistLimit = false;
	m_solveSwingLimit = false;

	Vec3 b1Axis1(0, 0, 0), b1Axis2(0, 0, 0), b1Axis3(0, 0, 0);
	Vec3 b2Axis1(0, 0, 0), b2Axis2(0, 0, 0);

	b1Axis1 = getRigidBodyA().getCenterOfMassTransform().getBasis() * this->m_rbAFrame.getBasis().getColumn(0);
	b2Axis1 = getRigidBodyB().getCenterOfMassTransform().getBasis() * this->m_rbBFrame.getBasis().getColumn(0);

	Scalar swing1 = Scalar(0.), swing2 = Scalar(0.);

	Scalar swx = Scalar(0.), swy = Scalar(0.);
	Scalar thresh = Scalar(10.);
	Scalar fact;

	// Get Frame into world space
	if (m_swingSpan1 >= Scalar(0.05f))
	{
		b1Axis2 = getRigidBodyA().getCenterOfMassTransform().getBasis() * this->m_rbAFrame.getBasis().getColumn(1);
		swx = b2Axis1.dot(b1Axis1);
		swy = b2Axis1.dot(b1Axis2);
		swing1 = Atan2Fast(swy, swx);
		fact = (swy * swy + swx * swx) * thresh * thresh;
		fact = fact / (fact + Scalar(1.0));
		swing1 *= fact;
	}

	if (m_swingSpan2 >= Scalar(0.05f))
	{
		b1Axis3 = getRigidBodyA().getCenterOfMassTransform().getBasis() * this->m_rbAFrame.getBasis().getColumn(2);
		swx = b2Axis1.dot(b1Axis1);
		swy = b2Axis1.dot(b1Axis3);
		swing2 = Atan2Fast(swy, swx);
		fact = (swy * swy + swx * swx) * thresh * thresh;
		fact = fact / (fact + Scalar(1.0));
		swing2 *= fact;
	}

	Scalar RMaxAngle1Sq = 1.0f / (m_swingSpan1 * m_swingSpan1);
	Scalar RMaxAngle2Sq = 1.0f / (m_swingSpan2 * m_swingSpan2);
	Scalar EllipseAngle = Fabs(swing1 * swing1) * RMaxAngle1Sq + Fabs(swing2 * swing2) * RMaxAngle2Sq;

	if (EllipseAngle > 1.0f)
	{
		m_swingCorrection = EllipseAngle - 1.0f;
		m_solveSwingLimit = true;
		// Calculate necessary axis & factors
		m_swingAxis = b2Axis1.cross(b1Axis2 * b2Axis1.dot(b1Axis2) + b1Axis3 * b2Axis1.dot(b1Axis3));
		m_swingAxis.normalize();
		Scalar swingAxisSign = (b2Axis1.dot(b1Axis1) >= 0.0f) ? 1.0f : -1.0f;
		m_swingAxis *= swingAxisSign;
	}

	// Twist limits
	if (m_twistSpan >= Scalar(0.))
	{
		Vec3 b2Axis2 = getRigidBodyB().getCenterOfMassTransform().getBasis() * this->m_rbBFrame.getBasis().getColumn(1);
		Quat rotationArc = shortestArcQuat(b2Axis1, b1Axis1);
		Vec3 TwistRef = quatRotate(rotationArc, b2Axis2);
		Scalar twist = Atan2Fast(TwistRef.dot(b1Axis3), TwistRef.dot(b1Axis2));
		m_twistAngle = twist;

		//		Scalar lockedFreeFactor = (m_twistSpan > Scalar(0.05f)) ? m_limitSoftness : Scalar(0.);
		Scalar lockedFreeFactor = (m_twistSpan > Scalar(0.05f)) ? Scalar(1.0f) : Scalar(0.);
		if (twist <= -m_twistSpan * lockedFreeFactor)
		{
			m_twistCorrection = -(twist + m_twistSpan);
			m_solveTwistLimit = true;
			m_twistAxis = (b2Axis1 + b1Axis1) * 0.5f;
			m_twistAxis.normalize();
			m_twistAxis *= -1.0f;
		}
		else if (twist > m_twistSpan * lockedFreeFactor)
		{
			m_twistCorrection = (twist - m_twistSpan);
			m_solveTwistLimit = true;
			m_twistAxis = (b2Axis1 + b1Axis1) * 0.5f;
			m_twistAxis.normalize();
		}
	}
}
#endif  //__SPU__

static Vec3 vTwist(1, 0, 0);  // twist axis in constraint's space

void ConeTwistConstraint::calcAngleInfo2(const Transform2& transA, const Transform2& transB, const Matrix3x3& invInertiaWorldA, const Matrix3x3& invInertiaWorldB)
{
	m_swingCorrection = Scalar(0.);
	m_twistLimitSign = Scalar(0.);
	m_solveTwistLimit = false;
	m_solveSwingLimit = false;
	// compute rotation of A wrt B (in constraint space)
	if (m_bMotorEnabled && (!m_useSolveConstraintObsolete))
	{  // it is assumed that setMotorTarget() was alredy called
		// and motor target m_qTarget is within constraint limits
		// TODO : split rotation to pure swing and pure twist
		// compute desired transforms in world
		Transform2 trPose(m_qTarget);
		Transform2 trA = transA * m_rbAFrame;
		Transform2 trB = transB * m_rbBFrame;
		Transform2 trDeltaAB = trB * trPose * trA.inverse();
		Quat qDeltaAB = trDeltaAB.getRotation();
		Vec3 swingAxis = Vec3(qDeltaAB.x(), qDeltaAB.y(), qDeltaAB.z());
		Scalar swingAxisLen2 = swingAxis.length2();
		if (FuzzyZero(swingAxisLen2))
		{
			return;
		}
		m_swingAxis = swingAxis;
		m_swingAxis.normalize();
		m_swingCorrection = qDeltaAB.getAngle();
		if (!FuzzyZero(m_swingCorrection))
		{
			m_solveSwingLimit = true;
		}
		return;
	}

	{
		// compute rotation of A wrt B (in constraint space)
		Quat qA = transA.getRotation() * m_rbAFrame.getRotation();
		Quat qB = transB.getRotation() * m_rbBFrame.getRotation();
		Quat qAB = qB.inverse() * qA;
		// split rotation into cone and twist
		// (all this is done from B's perspective. Maybe I should be averaging axes...)
		Vec3 vConeNoTwist = quatRotate(qAB, vTwist);
		vConeNoTwist.normalize();
		Quat qABCone = shortestArcQuat(vTwist, vConeNoTwist);
		qABCone.normalize();
		Quat qABTwist = qABCone.inverse() * qAB;
		qABTwist.normalize();

		if (m_swingSpan1 >= m_fixThresh && m_swingSpan2 >= m_fixThresh)
		{
			Scalar swingAngle, swingLimit = 0;
			Vec3 swingAxis;
			computeConeLimitInfo(qABCone, swingAngle, swingAxis, swingLimit);

			if (swingAngle > swingLimit * m_limitSoftness)
			{
				m_solveSwingLimit = true;

				// compute limit ratio: 0->1, where
				// 0 == beginning of soft limit
				// 1 == hard/real limit
				m_swingLimitRatio = 1.f;
				if (swingAngle < swingLimit && m_limitSoftness < 1.f - SIMD_EPSILON)
				{
					m_swingLimitRatio = (swingAngle - swingLimit * m_limitSoftness) /
										(swingLimit - swingLimit * m_limitSoftness);
				}

				// swing correction tries to get back to soft limit
				m_swingCorrection = swingAngle - (swingLimit * m_limitSoftness);

				// adjustment of swing axis (based on ellipse normal)
				adjustSwingAxisToUseEllipseNormal(swingAxis);

				// Calculate necessary axis & factors
				m_swingAxis = quatRotate(qB, -swingAxis);

				m_twistAxisA.setVal(0, 0, 0);

				m_kSwing = Scalar(1.) /
						   (computeAngularImpulseDenominator(m_swingAxis, invInertiaWorldA) +
							computeAngularImpulseDenominator(m_swingAxis, invInertiaWorldB));
			}
		}
		else
		{
			// you haven't set any limits;
			// or you're trying to set at least one of the swing limits too small. (if so, do you really want a conetwist constraint?)
			// anyway, we have either hinge or fixed joint
			Vec3 ivA = transA.getBasis() * m_rbAFrame.getBasis().getColumn(0);
			Vec3 jvA = transA.getBasis() * m_rbAFrame.getBasis().getColumn(1);
			Vec3 kvA = transA.getBasis() * m_rbAFrame.getBasis().getColumn(2);
			Vec3 ivB = transB.getBasis() * m_rbBFrame.getBasis().getColumn(0);
			Vec3 target;
			Scalar x = ivB.dot(ivA);
			Scalar y = ivB.dot(jvA);
			Scalar z = ivB.dot(kvA);
			if ((m_swingSpan1 < m_fixThresh) && (m_swingSpan2 < m_fixThresh))
			{  // fixed. We'll need to add one more row to constraint
				if ((!FuzzyZero(y)) || (!(FuzzyZero(z))))
				{
					m_solveSwingLimit = true;
					m_swingAxis = -ivB.cross(ivA);
				}
			}
			else
			{
				if (m_swingSpan1 < m_fixThresh)
				{  // hinge around Y axis
					//					if(!(FuzzyZero(y)))
					if ((!(FuzzyZero(x))) || (!(FuzzyZero(z))))
					{
						m_solveSwingLimit = true;
						if (m_swingSpan2 >= m_fixThresh)
						{
							y = Scalar(0.f);
							Scalar span2 = Atan2(z, x);
							if (span2 > m_swingSpan2)
							{
								x = Cos(m_swingSpan2);
								z = Sin(m_swingSpan2);
							}
							else if (span2 < -m_swingSpan2)
							{
								x = Cos(m_swingSpan2);
								z = -Sin(m_swingSpan2);
							}
						}
					}
				}
				else
				{  // hinge around Z axis
					//					if(!FuzzyZero(z))
					if ((!(FuzzyZero(x))) || (!(FuzzyZero(y))))
					{
						m_solveSwingLimit = true;
						if (m_swingSpan1 >= m_fixThresh)
						{
							z = Scalar(0.f);
							Scalar span1 = Atan2(y, x);
							if (span1 > m_swingSpan1)
							{
								x = Cos(m_swingSpan1);
								y = Sin(m_swingSpan1);
							}
							else if (span1 < -m_swingSpan1)
							{
								x = Cos(m_swingSpan1);
								y = -Sin(m_swingSpan1);
							}
						}
					}
				}
				target[0] = x * ivA[0] + y * jvA[0] + z * kvA[0];
				target[1] = x * ivA[1] + y * jvA[1] + z * kvA[1];
				target[2] = x * ivA[2] + y * jvA[2] + z * kvA[2];
				target.normalize();
				m_swingAxis = -ivB.cross(target);
				m_swingCorrection = m_swingAxis.length();

				if (!FuzzyZero(m_swingCorrection))
					m_swingAxis.normalize();
			}
		}

		if (m_twistSpan >= Scalar(0.f))
		{
			Vec3 twistAxis;
			computeTwistLimitInfo(qABTwist, m_twistAngle, twistAxis);

			if (m_twistAngle > m_twistSpan * m_limitSoftness)
			{
				m_solveTwistLimit = true;

				m_twistLimitRatio = 1.f;
				if (m_twistAngle < m_twistSpan && m_limitSoftness < 1.f - SIMD_EPSILON)
				{
					m_twistLimitRatio = (m_twistAngle - m_twistSpan * m_limitSoftness) /
										(m_twistSpan - m_twistSpan * m_limitSoftness);
				}

				// twist correction tries to get back to soft limit
				m_twistCorrection = m_twistAngle - (m_twistSpan * m_limitSoftness);

				m_twistAxis = quatRotate(qB, -twistAxis);

				m_kTwist = Scalar(1.) /
						   (computeAngularImpulseDenominator(m_twistAxis, invInertiaWorldA) +
							computeAngularImpulseDenominator(m_twistAxis, invInertiaWorldB));
			}

			if (m_solveSwingLimit)
				m_twistAxisA = quatRotate(qA, -twistAxis);
		}
		else
		{
			m_twistAngle = Scalar(0.f);
		}
	}
}

// given a cone rotation in constraint space, (pre: twist must already be removed)
// this method computes its corresponding swing angle and axis.
// more interestingly, it computes the cone/swing limit (angle) for this cone "pose".
void ConeTwistConstraint::computeConeLimitInfo(const Quat& qCone,
												 Scalar& swingAngle,   // out
												 Vec3& vSwingAxis,  // out
												 Scalar& swingLimit)   // out
{
	swingAngle = qCone.getAngle();
	if (swingAngle > SIMD_EPSILON)
	{
		vSwingAxis = Vec3(qCone.x(), qCone.y(), qCone.z());
		vSwingAxis.normalize();
#if 0
        // non-zero twist?! this should never happen.
       Assert(fabs(vSwingAxis.x()) <= SIMD_EPSILON));
#endif

		// Compute limit for given swing. tricky:
		// Given a swing axis, we're looking for the intersection with the bounding cone ellipse.
		// (Since we're dealing with angles, this ellipse is embedded on the surface of a sphere.)

		// For starters, compute the direction from center to surface of ellipse.
		// This is just the perpendicular (ie. rotate 2D vector by PI/2) of the swing axis.
		// (vSwingAxis is the cone rotation (in z,y); change vars and rotate to (x,y) coords.)
		Scalar xEllipse = vSwingAxis.y();
		Scalar yEllipse = -vSwingAxis.z();

		// Now, we use the slope of the vector (using x/yEllipse) and find the length
		// of the line that intersects the ellipse:
		//  x^2   y^2
		//  --- + --- = 1, where a and b are semi-major axes 2 and 1 respectively (ie. the limits)
		//  a^2   b^2
		// Do the math and it should be clear.

		swingLimit = m_swingSpan1;  // if xEllipse == 0, we have a pure vSwingAxis.z rotation: just use swingspan1
		if (fabs(xEllipse) > SIMD_EPSILON)
		{
			Scalar surfaceSlope2 = (yEllipse * yEllipse) / (xEllipse * xEllipse);
			Scalar norm = 1 / (m_swingSpan2 * m_swingSpan2);
			norm += surfaceSlope2 / (m_swingSpan1 * m_swingSpan1);
			Scalar swingLimit2 = (1 + surfaceSlope2) / norm;
			swingLimit = std::sqrt(swingLimit2);
		}

		// test!
		/*swingLimit = m_swingSpan2;
		if (fabs(vSwingAxis.z()) > SIMD_EPSILON)
		{
		Scalar mag_2 = m_swingSpan1*m_swingSpan1 + m_swingSpan2*m_swingSpan2;
		Scalar sinphi = m_swingSpan2 / sqrt(mag_2);
		Scalar phi = asin(sinphi);
		Scalar theta = atan2(fabs(vSwingAxis.y()),fabs(vSwingAxis.z()));
		Scalar alpha = 3.14159f - theta - phi;
		Scalar sinalpha = sin(alpha);
		swingLimit = m_swingSpan1 * sinphi/sinalpha;
		}*/
	}
	else if (swingAngle < 0)
	{
		// this should never happen!
#if 0
        Assert(0);
#endif
	}
}

Vec3 ConeTwistConstraint::GetPointForAngle(Scalar fAngleInRadians, Scalar fLength) const
{
	// compute x/y in ellipse using cone angle (0 -> 2*PI along surface of cone)
	Scalar xEllipse = Cos(fAngleInRadians);
	Scalar yEllipse = Sin(fAngleInRadians);

	// Use the slope of the vector (using x/yEllipse) and find the length
	// of the line that intersects the ellipse:
	//  x^2   y^2
	//  --- + --- = 1, where a and b are semi-major axes 2 and 1 respectively (ie. the limits)
	//  a^2   b^2
	// Do the math and it should be clear.

	Scalar swingLimit = m_swingSpan1;  // if xEllipse == 0, just use axis b (1)
	if (fabs(xEllipse) > SIMD_EPSILON)
	{
		Scalar surfaceSlope2 = (yEllipse * yEllipse) / (xEllipse * xEllipse);
		Scalar norm = 1 / (m_swingSpan2 * m_swingSpan2);
		norm += surfaceSlope2 / (m_swingSpan1 * m_swingSpan1);
		Scalar swingLimit2 = (1 + surfaceSlope2) / norm;
		swingLimit = std::sqrt(swingLimit2);
	}

	// convert into point in constraint space:
	// note: twist is x-axis, swing 1 and 2 are along the z and y axes respectively
	Vec3 vSwingAxis(0, xEllipse, -yEllipse);
	Quat qSwing(vSwingAxis, swingLimit);
	Vec3 vPointInConstraintSpace(fLength, 0, 0);
	return quatRotate(qSwing, vPointInConstraintSpace);
}

// given a twist rotation in constraint space, (pre: cone must already be removed)
// this method computes its corresponding angle and axis.
void ConeTwistConstraint::computeTwistLimitInfo(const Quat& qTwist,
												  Scalar& twistAngle,   // out
												  Vec3& vTwistAxis)  // out
{
	Quat qMinTwist = qTwist;
	twistAngle = qTwist.getAngle();

	if (twistAngle > SIMD_PI)  // long way around. flip quat and recalculate.
	{
		qMinTwist = -(qTwist);
		twistAngle = qMinTwist.getAngle();
	}
	if (twistAngle < 0)
	{
		// this should never happen
#if 0
        Assert(0);
#endif
	}

	vTwistAxis = Vec3(qMinTwist.x(), qMinTwist.y(), qMinTwist.z());
	if (twistAngle > SIMD_EPSILON)
		vTwistAxis.normalize();
}

void ConeTwistConstraint::adjustSwingAxisToUseEllipseNormal(Vec3& vSwingAxis) const
{
	// the swing axis is computed as the "twist-free" cone rotation,
	// but the cone limit is not circular, but elliptical (if swingspan1 != swingspan2).
	// so, if we're outside the limits, the closest way back inside the cone isn't
	// along the vector back to the center. better (and more stable) to use the ellipse normal.

	// convert swing axis to direction from center to surface of ellipse
	// (ie. rotate 2D vector by PI/2)
	Scalar y = -vSwingAxis.z();
	Scalar z = vSwingAxis.y();

	// do the math...
	if (fabs(z) > SIMD_EPSILON)  // avoid division by 0. and we don't need an update if z == 0.
	{
		// compute gradient/normal of ellipse surface at current "point"
		Scalar grad = y / z;
		grad *= m_swingSpan2 / m_swingSpan1;

		// adjust y/z to represent normal at point (instead of vector to point)
		if (y > 0)
			y = fabs(grad * z);
		else
			y = -fabs(grad * z);

		// convert ellipse direction back to swing axis
		vSwingAxis.setZ(-y);
		vSwingAxis.setY(z);
		vSwingAxis.normalize();
	}
}

void ConeTwistConstraint::setMotorTarget(const Quat& q)
{
	//Transform2 trACur = m_rbA.getCenterOfMassTransform();
	//Transform2 trBCur = m_rbB.getCenterOfMassTransform();
	//	Transform2 trABCur = trBCur.inverse() * trACur;
	//	Quat qABCur = trABCur.getRotation();
	//	Transform2 trConstraintCur = (trBCur * m_rbBFrame).inverse() * (trACur * m_rbAFrame);
	//Quat qConstraintCur = trConstraintCur.getRotation();

	Quat qConstraint = m_rbBFrame.getRotation().inverse() * q * m_rbAFrame.getRotation();
	setMotorTargetInConstraintSpace(qConstraint);
}

void ConeTwistConstraint::setMotorTargetInConstraintSpace(const Quat& q)
{
	m_qTarget = q;

	// clamp motor target to within limits
	{
		Scalar softness = 1.f;  //m_limitSoftness;

		// split into twist and cone
		Vec3 vTwisted = quatRotate(m_qTarget, vTwist);
		Quat qTargetCone = shortestArcQuat(vTwist, vTwisted);
		qTargetCone.normalize();
		Quat qTargetTwist = qTargetCone.inverse() * m_qTarget;
		qTargetTwist.normalize();

		// clamp cone
		if (m_swingSpan1 >= Scalar(0.05f) && m_swingSpan2 >= Scalar(0.05f))
		{
			Scalar swingAngle, swingLimit;
			Vec3 swingAxis;
			computeConeLimitInfo(qTargetCone, swingAngle, swingAxis, swingLimit);

			if (fabs(swingAngle) > SIMD_EPSILON)
			{
				if (swingAngle > swingLimit * softness)
					swingAngle = swingLimit * softness;
				else if (swingAngle < -swingLimit * softness)
					swingAngle = -swingLimit * softness;
				qTargetCone = Quat(swingAxis, swingAngle);
			}
		}

		// clamp twist
		if (m_twistSpan >= Scalar(0.05f))
		{
			Scalar twistAngle;
			Vec3 twistAxis;
			computeTwistLimitInfo(qTargetTwist, twistAngle, twistAxis);

			if (fabs(twistAngle) > SIMD_EPSILON)
			{
				// eddy todo: limitSoftness used here???
				if (twistAngle > m_twistSpan * softness)
					twistAngle = m_twistSpan * softness;
				else if (twistAngle < -m_twistSpan * softness)
					twistAngle = -m_twistSpan * softness;
				qTargetTwist = Quat(twistAxis, twistAngle);
			}
		}

		m_qTarget = qTargetCone * qTargetTwist;
	}
}

///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5).
///If no axis is provided, it uses the default axis for this constraint.
void ConeTwistConstraint::setParam(i32 num, Scalar value, i32 axis)
{
	switch (num)
	{
		case DRX3D_CONSTRAINT_ERP:
		case DRX3D_CONSTRAINT_STOP_ERP:
			if ((axis >= 0) && (axis < 3))
			{
				m_linERP = value;
				m_flags |= DRX3D_CONETWIST_FLAGS_LIN_ERP;
			}
			else
			{
				m_biasFactor = value;
			}
			break;
		case DRX3D_CONSTRAINT_CFM:
		case DRX3D_CONSTRAINT_STOP_CFM:
			if ((axis >= 0) && (axis < 3))
			{
				m_linCFM = value;
				m_flags |= DRX3D_CONETWIST_FLAGS_LIN_CFM;
			}
			else
			{
				m_angCFM = value;
				m_flags |= DRX3D_CONETWIST_FLAGS_ANG_CFM;
			}
			break;
		default:
			AssertConstrParams(0);
			break;
	}
}

///return the local value of parameter
Scalar ConeTwistConstraint::getParam(i32 num, i32 axis) const
{
	Scalar retVal = 0;
	switch (num)
	{
		case DRX3D_CONSTRAINT_ERP:
		case DRX3D_CONSTRAINT_STOP_ERP:
			if ((axis >= 0) && (axis < 3))
			{
				AssertConstrParams(m_flags & DRX3D_CONETWIST_FLAGS_LIN_ERP);
				retVal = m_linERP;
			}
			else if ((axis >= 3) && (axis < 6))
			{
				retVal = m_biasFactor;
			}
			else
			{
				AssertConstrParams(0);
			}
			break;
		case DRX3D_CONSTRAINT_CFM:
		case DRX3D_CONSTRAINT_STOP_CFM:
			if ((axis >= 0) && (axis < 3))
			{
				AssertConstrParams(m_flags & DRX3D_CONETWIST_FLAGS_LIN_CFM);
				retVal = m_linCFM;
			}
			else if ((axis >= 3) && (axis < 6))
			{
				AssertConstrParams(m_flags & DRX3D_CONETWIST_FLAGS_ANG_CFM);
				retVal = m_angCFM;
			}
			else
			{
				AssertConstrParams(0);
			}
			break;
		default:
			AssertConstrParams(0);
	}
	return retVal;
}

void ConeTwistConstraint::setFrames(const Transform2& frameA, const Transform2& frameB)
{
	m_rbAFrame = frameA;
	m_rbBFrame = frameB;
	buildJacobian();
	//calculateTransforms();
}
