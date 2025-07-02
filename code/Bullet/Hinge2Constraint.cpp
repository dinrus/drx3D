#include <drx3D/Physics/Dynamics/ConstraintSolver/Hinge2Constraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

// constructor
// anchor, axis1 and axis2 are in world coordinate system
// axis1 must be orthogonal to axis2
Hinge2Constraint::Hinge2Constraint(RigidBody& rbA, RigidBody& rbB, Vec3& anchor, Vec3& axis1, Vec3& axis2)
	: Generic6DofSpring2Constraint(rbA, rbB, Transform2::getIdentity(), Transform2::getIdentity(), RO_XYZ),
	  m_anchor(anchor),
	  m_axis1(axis1),
	  m_axis2(axis2)
{
	// build frame basis
	// 6DOF constraint uses Euler angles and to define limits
	// it is assumed that rotational order is :
	// Z - first, allowed limits are (-PI,PI);
	// new position of Y - second (allowed limits are (-PI/2 + epsilon, PI/2 - epsilon), where epsilon is a small positive number
	// used to prevent constraint from instability on poles;
	// new position of X, allowed limits are (-PI,PI);
	// So to simulate ODE Universal joint we should use parent axis as Z, child axis as Y and limit all other DOFs
	// Build the frame in world coordinate system first
	Vec3 zAxis = axis1.normalize();
	Vec3 xAxis = axis2.normalize();
	Vec3 yAxis = zAxis.cross(xAxis);  // we want right coordinate system
	Transform2 frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setVal(xAxis[0], yAxis[0], zAxis[0],
								 xAxis[1], yAxis[1], zAxis[1],
								 xAxis[2], yAxis[2], zAxis[2]);
	frameInW.setOrigin(anchor);
	// now get constraint frame in local coordinate systems
	m_frameInA = rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = rbB.getCenterOfMassTransform().inverse() * frameInW;
	// sei limits
	setLinearLowerLimit(Vec3(0.f, 0.f, -1.f));
	setLinearUpperLimit(Vec3(0.f, 0.f, 1.f));
	// like front wheels of a car
	setAngularLowerLimit(Vec3(1.f, 0.f, -SIMD_HALF_PI * 0.5f));
	setAngularUpperLimit(Vec3(-1.f, 0.f, SIMD_HALF_PI * 0.5f));
	// enable suspension
	enableSpring(2, true);
	setStiffness(2, SIMD_PI * SIMD_PI * 4.f);
	setDamping(2, 0.01f);
	setEquilibriumPoint();
}
