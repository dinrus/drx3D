#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Physics/Collision/NarrowPhase/ManifoldPoint.h>

ContactConstraint::ContactConstraint(PersistentManifold* contactManifold, RigidBody& rbA, RigidBody& rbB)
	: TypedConstraint(CONTACT_CONSTRAINT_TYPE, rbA, rbB),
	  m_contactManifold(*contactManifold)
{
}

ContactConstraint::~ContactConstraint()
{
}

void ContactConstraint::setContactManifold(PersistentManifold* contactManifold)
{
	m_contactManifold = *contactManifold;
}

void ContactConstraint::getInfo1(ConstraintInfo1* info)
{
}

void ContactConstraint::getInfo2(ConstraintInfo2* info)
{
}

void ContactConstraint::buildJacobian()
{
}

#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/ContactSolverInfo.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Physics/Collision/NarrowPhase/ManifoldPoint.h>

//response  between two dynamic objects without friction and no restitution, assuming 0 penetration depth
Scalar resolveSingleCollision(
	RigidBody* body1,
	CollisionObject2* colObj2,
	const Vec3& contactPositionWorld,
	const Vec3& contactNormalOnB,
	const ContactSolverInfo& solverInfo,
	Scalar distance)
{
	RigidBody* body2 = RigidBody::upcast(colObj2);

	const Vec3& normal = contactNormalOnB;

	Vec3 rel_pos1 = contactPositionWorld - body1->getWorldTransform().getOrigin();
	Vec3 rel_pos2 = contactPositionWorld - colObj2->getWorldTransform().getOrigin();

	Vec3 vel1 = body1->getVelocityInLocalPoint(rel_pos1);
	Vec3 vel2 = body2 ? body2->getVelocityInLocalPoint(rel_pos2) : Vec3(0, 0, 0);
	Vec3 vel = vel1 - vel2;
	Scalar rel_vel;
	rel_vel = normal.dot(vel);

	Scalar combinedRestitution = 0.f;
	Scalar restitution = combinedRestitution * -rel_vel;

	Scalar positionalError = solverInfo.m_erp * -distance / solverInfo.m_timeStep;
	Scalar velocityError = -(1.0f + restitution) * rel_vel;  // * damping;
	Scalar denom0 = body1->computeImpulseDenominator(contactPositionWorld, normal);
	Scalar denom1 = body2 ? body2->computeImpulseDenominator(contactPositionWorld, normal) : 0.f;
	Scalar relaxation = 1.f;
	Scalar jacDiagABInv = relaxation / (denom0 + denom1);

	Scalar penetrationImpulse = positionalError * jacDiagABInv;
	Scalar velocityImpulse = velocityError * jacDiagABInv;

	Scalar normalImpulse = penetrationImpulse + velocityImpulse;
	normalImpulse = 0.f > normalImpulse ? 0.f : normalImpulse;

	body1->applyImpulse(normal * (normalImpulse), rel_pos1);
	if (body2)
		body2->applyImpulse(-normal * (normalImpulse), rel_pos2);

	return normalImpulse;
}

//bilateral constraint between two dynamic objects
void resolveSingleBilateral(RigidBody& body1, const Vec3& pos1,
							RigidBody& body2, const Vec3& pos2,
							Scalar distance, const Vec3& normal, Scalar& impulse, Scalar timeStep)
{
	(void)timeStep;
	(void)distance;

	Scalar normalLenSqr = normal.length2();
	Assert(Fabs(normalLenSqr) < Scalar(1.1));
	if (normalLenSqr > Scalar(1.1))
	{
		impulse = Scalar(0.);
		return;
	}
	Vec3 rel_pos1 = pos1 - body1.getCenterOfMassPosition();
	Vec3 rel_pos2 = pos2 - body2.getCenterOfMassPosition();
	//this jacobian entry could be re-used for all iterations

	Vec3 vel1 = body1.getVelocityInLocalPoint(rel_pos1);
	Vec3 vel2 = body2.getVelocityInLocalPoint(rel_pos2);
	Vec3 vel = vel1 - vel2;

	JacobianEntry jac(body1.getCenterOfMassTransform().getBasis().transpose(),
						body2.getCenterOfMassTransform().getBasis().transpose(),
						rel_pos1, rel_pos2, normal, body1.getInvInertiaDiagLocal(), body1.getInvMass(),
						body2.getInvInertiaDiagLocal(), body2.getInvMass());

	Scalar jacDiagAB = jac.getDiagonal();
	Scalar jacDiagABInv = Scalar(1.) / jacDiagAB;

	Scalar rel_vel = jac.getRelativeVelocity(
		body1.getLinearVelocity(),
		body1.getCenterOfMassTransform().getBasis().transpose() * body1.getAngularVelocity(),
		body2.getLinearVelocity(),
		body2.getCenterOfMassTransform().getBasis().transpose() * body2.getAngularVelocity());

	rel_vel = normal.dot(vel);

	//todo: move this into proper structure
	Scalar contactDamping = Scalar(0.2);

#ifdef ONLY_USE_LINEAR_MASS
	Scalar massTerm = Scalar(1.) / (body1.getInvMass() + body2.getInvMass());
	impulse = -contactDamping * rel_vel * massTerm;
#else
	Scalar velocityImpulse = -contactDamping * rel_vel * jacDiagABInv;
	impulse = velocityImpulse;
#endif
}
