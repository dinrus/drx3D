#ifndef DRX3D_UNIVERSAL_CONSTRAINT_H
#define DRX3D_UNIVERSAL_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofConstraint.h>

/// Constraint similar to ODE Universal Joint
/// has 2 rotatioonal degrees of freedom, similar to Euler rotations around Z (axis 1)
/// and Y (axis 2)
/// Description from ODE manual :
/// "Given axis 1 on body 1, and axis 2 on body 2 that is perpendicular to axis 1, it keeps them perpendicular.
/// In other words, rotation of the two bodies about the direction perpendicular to the two axes will be equal."

ATTRIBUTE_ALIGNED16(class)
UniversalConstraint : public Generic6DofConstraint
{
protected:
	Vec3 m_anchor;
	Vec3 m_axis1;
	Vec3 m_axis2;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	// constructor
	// anchor, axis1 and axis2 are in world coordinate system
	// axis1 must be orthogonal to axis2
	UniversalConstraint(RigidBody & rbA, RigidBody & rbB, const Vec3& anchor, const Vec3& axis1, const Vec3& axis2);
	// access
	const Vec3& getAnchor() { return m_calculatedTransformA.getOrigin(); }
	const Vec3& getAnchor2() { return m_calculatedTransformB.getOrigin(); }
	const Vec3& getAxis1() { return m_axis1; }
	const Vec3& getAxis2() { return m_axis2; }
	Scalar getAngle1() { return getAngle(2); }
	Scalar getAngle2() { return getAngle(1); }
	// limits
	void setUpperLimit(Scalar ang1max, Scalar ang2max) { setAngularUpperLimit(Vec3(0.f, ang1max, ang2max)); }
	void setLowerLimit(Scalar ang1min, Scalar ang2min) { setAngularLowerLimit(Vec3(0.f, ang1min, ang2min)); }

	void setAxis(const Vec3& axis1, const Vec3& axis2);
};

#endif  // DRX3D_UNIVERSAL_CONSTRAINT_H
