

#include <drx3D/Physics/Dynamics/ConstraintSolver/UniversalConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

#define UNIV_EPS Scalar(0.01f)

// constructor
// anchor, axis1 and axis2 are in world coordinate system
// axis1 must be orthogonal to axis2
UniversalConstraint::UniversalConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& anchor, const Vec3& axis1, const Vec3& axis2)
	: Generic6DofConstraint(rbA, rbB, Transform2::getIdentity(), Transform2::getIdentity(), true),
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
	Vec3 zAxis = m_axis1.normalize();
	Vec3 yAxis = m_axis2.normalize();
	Vec3 xAxis = yAxis.cross(zAxis);  // we want right coordinate system
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
	setLinearLowerLimit(Vec3(0., 0., 0.));
	setLinearUpperLimit(Vec3(0., 0., 0.));
	setAngularLowerLimit(Vec3(0.f, -SIMD_HALF_PI + UNIV_EPS, -SIMD_PI + UNIV_EPS));
	setAngularUpperLimit(Vec3(0.f, SIMD_HALF_PI - UNIV_EPS, SIMD_PI - UNIV_EPS));
}

void UniversalConstraint::setAxis(const Vec3& axis1, const Vec3& axis2)
{
	m_axis1 = axis1;
	m_axis2 = axis2;

	Vec3 zAxis = axis1.normalized();
	Vec3 yAxis = axis2.normalized();
	Vec3 xAxis = yAxis.cross(zAxis);  // we want right coordinate system

	Transform2 frameInW;
	frameInW.setIdentity();
	frameInW.getBasis().setVal(xAxis[0], yAxis[0], zAxis[0],
								 xAxis[1], yAxis[1], zAxis[1],
								 xAxis[2], yAxis[2], zAxis[2]);
	frameInW.setOrigin(m_anchor);

	// now get constraint frame in local coordinate systems
	m_frameInA = m_rbA.getCenterOfMassTransform().inverse() * frameInW;
	m_frameInB = m_rbB.getCenterOfMassTransform().inverse() * frameInW;

	calculateTransforms();
}
