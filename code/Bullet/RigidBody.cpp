
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <drx3D/Maths/Linear/MotionState.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Maths/Linear/Serializer.h>

//'temporarily' global variables
Scalar gDeactivationTime = Scalar(2.);
bool gDisableDeactivation = false;
static i32 uniqueId = 0;

RigidBody::RigidBody(const RigidBody::RigidBodyConstructionInfo& constructionInfo)
{
	setupRigidBody(constructionInfo);
}

RigidBody::RigidBody(Scalar mass, MotionState* motionState, CollisionShape* collisionShape, const Vec3& localInertia)
{
	RigidBodyConstructionInfo cinfo(mass, motionState, collisionShape, localInertia);
	setupRigidBody(cinfo);
}

void RigidBody::setupRigidBody(const RigidBody::RigidBodyConstructionInfo& constructionInfo)
{
	m_internalType = CO_RIGID_BODY;

	m_linearVelocity.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	m_angularVelocity.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
	m_angularFactor.setVal(1, 1, 1);
	m_linearFactor.setVal(1, 1, 1);
	m_gravity.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	m_gravity_acceleration.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	m_totalForce.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0));
	m_totalTorque.setVal(Scalar(0.0), Scalar(0.0), Scalar(0.0)),
		setDamping(constructionInfo.m_linearDamping, constructionInfo.m_angularDamping);

	m_linearSleepingThreshold = constructionInfo.m_linearSleepingThreshold;
	m_angularSleepingThreshold = constructionInfo.m_angularSleepingThreshold;
	m_optionalMotionState = constructionInfo.m_motionState;
	m_contactSolverType = 0;
	m_frictionSolverType = 0;
	m_additionalDamping = constructionInfo.m_additionalDamping;
	m_additionalDampingFactor = constructionInfo.m_additionalDampingFactor;
	m_additionalLinearDampingThresholdSqr = constructionInfo.m_additionalLinearDampingThresholdSqr;
	m_additionalAngularDampingThresholdSqr = constructionInfo.m_additionalAngularDampingThresholdSqr;
	m_additionalAngularDampingFactor = constructionInfo.m_additionalAngularDampingFactor;

	if (m_optionalMotionState)
	{
		m_optionalMotionState->getWorldTransform(m_worldTransform);
	}
	else
	{
		m_worldTransform = constructionInfo.m_startWorldTransform;
	}

	m_interpolationWorldTransform = m_worldTransform;
	m_interpolationLinearVelocity.setVal(0, 0, 0);
	m_interpolationAngularVelocity.setVal(0, 0, 0);

	//moved to CollisionObject2
	m_friction = constructionInfo.m_friction;
	m_rollingFriction = constructionInfo.m_rollingFriction;
	m_spinningFriction = constructionInfo.m_spinningFriction;

	m_restitution = constructionInfo.m_restitution;

	setCollisionShape(constructionInfo.m_collisionShape);
	m_debugBodyId = uniqueId++;

	setMassProps(constructionInfo.m_mass, constructionInfo.m_localInertia);
	updateInertiaTensor();

	m_rigidbodyFlags = DRX3D_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY;

	m_deltaLinearVelocity.setZero();
	m_deltaAngularVelocity.setZero();
	m_invMass = m_inverseMass * m_linearFactor;
	m_pushVelocity.setZero();
	m_turnVelocity.setZero();
}

void RigidBody::predictIntegratedTransform2(Scalar timeStep, Transform2& predictedTransform2)
{
	Transform2Util::integrateTransform(m_worldTransform, m_linearVelocity, m_angularVelocity, timeStep, predictedTransform2);
}

void RigidBody::saveKinematicState(Scalar timeStep)
{
	//todo: clamp to some (user definable) safe minimum timestep, to limit maximum angular/linear velocities
	if (timeStep != Scalar(0.))
	{
		//if we use motionstate to synchronize world transforms, get the new kinematic/animated world transform
		if (getMotionState())
			getMotionState()->getWorldTransform(m_worldTransform);
		Vec3 linVel, angVel;

		Transform2Util::calculateVelocity(m_interpolationWorldTransform, m_worldTransform, timeStep, m_linearVelocity, m_angularVelocity);
		m_interpolationLinearVelocity = m_linearVelocity;
		m_interpolationAngularVelocity = m_angularVelocity;
		m_interpolationWorldTransform = m_worldTransform;
		//printf("angular = %f %f %f\n",m_angularVelocity.getX(),m_angularVelocity.getY(),m_angularVelocity.getZ());
	}
}

void RigidBody::getAabb(Vec3& aabbMin, Vec3& aabbMax) const
{
	getCollisionShape()->getAabb(m_worldTransform, aabbMin, aabbMax);
}

void RigidBody::setGravity(const Vec3& acceleration)
{
	if (m_inverseMass != Scalar(0.0))
	{
		m_gravity = acceleration * (Scalar(1.0) / m_inverseMass);
	}
	m_gravity_acceleration = acceleration;
}

void RigidBody::setDamping(Scalar lin_damping, Scalar ang_damping)
{
#ifdef DRX3D_USE_OLD_DAMPING_METHOD
	m_linearDamping = d3Max(lin_damping, Scalar(0.0));
	m_angularDamping = d3Max(ang_damping, Scalar(0.0));
#else
	m_linearDamping = Clamped(lin_damping, Scalar(0.0), Scalar(1.0));
	m_angularDamping = Clamped(ang_damping, Scalar(0.0), Scalar(1.0));
#endif
}

///applyDamping damps the velocity, using the given m_linearDamping and m_angularDamping
void RigidBody::applyDamping(Scalar timeStep)
{
	//On new damping: see discussion/issue report here: http://code.google.com/p/bullet/issues/detail?id=74
	//todo: do some performance comparisons (but other parts of the engine are probably bottleneck anyway

#ifdef DRX3D_USE_OLD_DAMPING_METHOD
	m_linearVelocity *= d3Max((Scalar(1.0) - timeStep * m_linearDamping), Scalar(0.0));
	m_angularVelocity *= d3Max((Scalar(1.0) - timeStep * m_angularDamping), Scalar(0.0));
#else
	m_linearVelocity *= Pow(Scalar(1) - m_linearDamping, timeStep);
	m_angularVelocity *= Pow(Scalar(1) - m_angularDamping, timeStep);
#endif

	if (m_additionalDamping)
	{
		//Additional damping can help avoiding lowpass jitter motion, help stability for ragdolls etc.
		//Such damping is undesirable, so once the overall simulation quality of the rigid body dynamics system has improved, this should become obsolete
		if ((m_angularVelocity.length2() < m_additionalAngularDampingThresholdSqr) &&
			(m_linearVelocity.length2() < m_additionalLinearDampingThresholdSqr))
		{
			m_angularVelocity *= m_additionalDampingFactor;
			m_linearVelocity *= m_additionalDampingFactor;
		}

		Scalar speed = m_linearVelocity.length();
		if (speed < m_linearDamping)
		{
			Scalar dampVel = Scalar(0.005);
			if (speed > dampVel)
			{
				Vec3 dir = m_linearVelocity.normalized();
				m_linearVelocity -= dir * dampVel;
			}
			else
			{
				m_linearVelocity.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
			}
		}

		Scalar angSpeed = m_angularVelocity.length();
		if (angSpeed < m_angularDamping)
		{
			Scalar angDampVel = Scalar(0.005);
			if (angSpeed > angDampVel)
			{
				Vec3 dir = m_angularVelocity.normalized();
				m_angularVelocity -= dir * angDampVel;
			}
			else
			{
				m_angularVelocity.setVal(Scalar(0.), Scalar(0.), Scalar(0.));
			}
		}
	}
}

void RigidBody::applyGravity()
{
	if (isStaticOrKinematicObject())
		return;

	applyCentralForce(m_gravity);
}

void RigidBody::clearGravity()
{
    if (isStaticOrKinematicObject())
        return;
    
    applyCentralForce(-m_gravity);
}

void RigidBody::proceedToTransform2(const Transform2& newTrans)
{
	setCenterOfMassTransform(newTrans);
}

void RigidBody::setMassProps(Scalar mass, const Vec3& inertia)
{
	if (mass == Scalar(0.))
	{
		m_collisionFlags |= CollisionObject2::CF_STATIC_OBJECT;
		m_inverseMass = Scalar(0.);
	}
	else
	{
		m_collisionFlags &= (~CollisionObject2::CF_STATIC_OBJECT);
		m_inverseMass = Scalar(1.0) / mass;
	}

	//Fg = m * a
	m_gravity = mass * m_gravity_acceleration;

	m_invInertiaLocal.setVal(inertia.x() != Scalar(0.0) ? Scalar(1.0) / inertia.x() : Scalar(0.0),
							   inertia.y() != Scalar(0.0) ? Scalar(1.0) / inertia.y() : Scalar(0.0),
							   inertia.z() != Scalar(0.0) ? Scalar(1.0) / inertia.z() : Scalar(0.0));

	m_invMass = m_linearFactor * m_inverseMass;
}

void RigidBody::updateInertiaTensor()
{
	m_invInertiaTensorWorld = m_worldTransform.getBasis().scaled(m_invInertiaLocal) * m_worldTransform.getBasis().transpose();
}

Vec3 RigidBody::getLocalInertia() const
{
	Vec3 inertiaLocal;
	const Vec3 inertia = m_invInertiaLocal;
	inertiaLocal.setVal(inertia.x() != Scalar(0.0) ? Scalar(1.0) / inertia.x() : Scalar(0.0),
						  inertia.y() != Scalar(0.0) ? Scalar(1.0) / inertia.y() : Scalar(0.0),
						  inertia.z() != Scalar(0.0) ? Scalar(1.0) / inertia.z() : Scalar(0.0));
	return inertiaLocal;
}

inline Vec3 evalEulerEqn(const Vec3& w1, const Vec3& w0, const Vec3& T, const Scalar dt,
							  const Matrix3x3& I)
{
	const Vec3 w2 = I * w1 + w1.cross(I * w1) * dt - (T * dt + I * w0);
	return w2;
}

inline Matrix3x3 evalEulerEqnDeriv(const Vec3& w1, const Vec3& w0, const Scalar dt,
									 const Matrix3x3& I)
{
	Matrix3x3 w1x, Iw1x;
	const Vec3 Iwi = (I * w1);
	w1.getSkewSymmetricMatrix(&w1x[0], &w1x[1], &w1x[2]);
	Iwi.getSkewSymmetricMatrix(&Iw1x[0], &Iw1x[1], &Iw1x[2]);

	const Matrix3x3 dfw1 = I + (w1x * I - Iw1x) * dt;
	return dfw1;
}

Vec3 RigidBody::computeGyroscopicForceExplicit(Scalar maxGyroscopicForce) const
{
	Vec3 inertiaLocal = getLocalInertia();
	Matrix3x3 inertiaTensorWorld = getWorldTransform().getBasis().scaled(inertiaLocal) * getWorldTransform().getBasis().transpose();
	Vec3 tmp = inertiaTensorWorld * getAngularVelocity();
	Vec3 gf = getAngularVelocity().cross(tmp);
	Scalar l2 = gf.length2();
	if (l2 > maxGyroscopicForce * maxGyroscopicForce)
	{
		gf *= Scalar(1.) / Sqrt(l2) * maxGyroscopicForce;
	}
	return gf;
}

Vec3 RigidBody::computeGyroscopicImpulseImplicit_Body(Scalar step) const
{
	Vec3 idl = getLocalInertia();
	Vec3 omega1 = getAngularVelocity();
	Quat q = getWorldTransform().getRotation();

	// Convert to body coordinates
	Vec3 omegab = quatRotate(q.inverse(), omega1);
	Matrix3x3 Ib;
	Ib.setVal(idl.x(), 0, 0,
				0, idl.y(), 0,
				0, 0, idl.z());

	Vec3 ibo = Ib * omegab;

	// Residual vector
	Vec3 f = step * omegab.cross(ibo);

	Matrix3x3 skew0;
	omegab.getSkewSymmetricMatrix(&skew0[0], &skew0[1], &skew0[2]);
	Vec3 om = Ib * omegab;
	Matrix3x3 skew1;
	om.getSkewSymmetricMatrix(&skew1[0], &skew1[1], &skew1[2]);

	// Jacobian
	Matrix3x3 J = Ib + (skew0 * Ib - skew1) * step;

	//	Matrix3x3 Jinv = J.inverse();
	//	Vec3 omega_div = Jinv*f;
	Vec3 omega_div = J.solve33(f);

	// Single Newton-Raphson update
	omegab = omegab - omega_div;  //Solve33(J, f);
	// Back to world coordinates
	Vec3 omega2 = quatRotate(q, omegab);
	Vec3 gf = omega2 - omega1;
	return gf;
}

Vec3 RigidBody::computeGyroscopicImpulseImplicit_World(Scalar step) const
{
	// use full newton-euler equations.  common practice to drop the wxIw term. want it for better tumbling behavior.
	// calculate using implicit euler step so it's stable.

	const Vec3 inertiaLocal = getLocalInertia();
	const Vec3 w0 = getAngularVelocity();

	Matrix3x3 I;

	I = m_worldTransform.getBasis().scaled(inertiaLocal) *
		m_worldTransform.getBasis().transpose();

	// use newtons method to find implicit solution for new angular velocity (w')
	// f(w') = -(T*step + Iw) + Iw' + w' + w'xIw'*step = 0
	// df/dw' = I + 1xIw'*step + w'xI*step

	Vec3 w1 = w0;

	// one step of newton's method
	{
		const Vec3 fw = evalEulerEqn(w1, w0, Vec3(0, 0, 0), step, I);
		const Matrix3x3 dfw = evalEulerEqnDeriv(w1, w0, step, I);

		Vec3 dw;
		dw = dfw.solve33(fw);
		//const Matrix3x3 dfw_inv = dfw.inverse();
		//dw = dfw_inv*fw;

		w1 -= dw;
	}

	Vec3 gf = (w1 - w0);
	return gf;
}

void RigidBody::integrateVelocities(Scalar step)
{
	if (isStaticOrKinematicObject())
		return;

	m_linearVelocity += m_totalForce * (m_inverseMass * step);
	m_angularVelocity += m_invInertiaTensorWorld * m_totalTorque * step;

#define MAX_ANGVEL SIMD_HALF_PI
	/// clamp angular velocity. collision calculations will fail on higher angular velocities
	Scalar angvel = m_angularVelocity.length();
	if (angvel * step > MAX_ANGVEL)
	{
		m_angularVelocity *= (MAX_ANGVEL / step) / angvel;
	}
	#if defined(DRX3D_CLAMP_VELOCITY_TO) && DRX3D_CLAMP_VELOCITY_TO > 0
	clampVelocity(m_angularVelocity);
	#endif
}

Quat RigidBody::getOrientation() const
{
	Quat orn;
	m_worldTransform.getBasis().getRotation(orn);
	return orn;
}

void RigidBody::setCenterOfMassTransform(const Transform2& xform)
{
	if (isKinematicObject())
	{
		m_interpolationWorldTransform = m_worldTransform;
	}
	else
	{
		m_interpolationWorldTransform = xform;
	}
	m_interpolationLinearVelocity = getLinearVelocity();
	m_interpolationAngularVelocity = getAngularVelocity();
	m_worldTransform = xform;
	updateInertiaTensor();
}

void RigidBody::addConstraintRef(TypedConstraint* c)
{
	///disable collision with the 'other' body

	i32 index = m_constraintRefs.findLinearSearch(c);
	//don't add constraints that are already referenced
	//Assert(index == m_constraintRefs.size());
	if (index == m_constraintRefs.size())
	{
		m_constraintRefs.push_back(c);
		CollisionObject2* colObjA = &c->getRigidBodyA();
		CollisionObject2* colObjB = &c->getRigidBodyB();
		if (colObjA == this)
		{
			colObjA->setIgnoreCollisionCheck(colObjB, true);
		}
		else
		{
			colObjB->setIgnoreCollisionCheck(colObjA, true);
		}
	}
}

void RigidBody::removeConstraintRef(TypedConstraint* c)
{
	i32 index = m_constraintRefs.findLinearSearch(c);
	//don't remove constraints that are not referenced
	if (index < m_constraintRefs.size())
	{
		m_constraintRefs.remove(c);
		CollisionObject2* colObjA = &c->getRigidBodyA();
		CollisionObject2* colObjB = &c->getRigidBodyB();
		if (colObjA == this)
		{
			colObjA->setIgnoreCollisionCheck(colObjB, false);
		}
		else
		{
			colObjB->setIgnoreCollisionCheck(colObjA, false);
		}
	}
}

i32 RigidBody::calculateSerializeBufferSize() const
{
	i32 sz = sizeof(RigidBodyData);
	return sz;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk RigidBody::serialize(uk dataBuffer, class Serializer* serializer) const
{
	RigidBodyData* rbd = (RigidBodyData*)dataBuffer;

	CollisionObject2::serialize(&rbd->m_collisionObjectData, serializer);

	m_invInertiaTensorWorld.serialize(rbd->m_invInertiaTensorWorld);
	m_linearVelocity.serialize(rbd->m_linearVelocity);
	m_angularVelocity.serialize(rbd->m_angularVelocity);
	rbd->m_inverseMass = m_inverseMass;
	m_angularFactor.serialize(rbd->m_angularFactor);
	m_linearFactor.serialize(rbd->m_linearFactor);
	m_gravity.serialize(rbd->m_gravity);
	m_gravity_acceleration.serialize(rbd->m_gravity_acceleration);
	m_invInertiaLocal.serialize(rbd->m_invInertiaLocal);
	m_totalForce.serialize(rbd->m_totalForce);
	m_totalTorque.serialize(rbd->m_totalTorque);
	rbd->m_linearDamping = m_linearDamping;
	rbd->m_angularDamping = m_angularDamping;
	rbd->m_additionalDamping = m_additionalDamping;
	rbd->m_additionalDampingFactor = m_additionalDampingFactor;
	rbd->m_additionalLinearDampingThresholdSqr = m_additionalLinearDampingThresholdSqr;
	rbd->m_additionalAngularDampingThresholdSqr = m_additionalAngularDampingThresholdSqr;
	rbd->m_additionalAngularDampingFactor = m_additionalAngularDampingFactor;
	rbd->m_linearSleepingThreshold = m_linearSleepingThreshold;
	rbd->m_angularSleepingThreshold = m_angularSleepingThreshold;

	// Fill padding with zeros to appease msan.
#ifdef DRX3D_USE_DOUBLE_PRECISION
	memset(rbd->m_padding, 0, sizeof(rbd->m_padding));
#endif

	return RigidBodyDataName;
}

void RigidBody::serializeSingleObject(class Serializer* serializer) const
{
	Chunk* chunk = serializer->allocate(calculateSerializeBufferSize(), 1);
	tukk structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk, structType, DRX3D_RIGIDBODY_CODE, (uk )this);
}
