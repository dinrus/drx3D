#ifndef DRX3D_HINGE2_CONSTRAINT_H
#define DRX3D_HINGE2_CONSTRAINT_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/TypedConstraint.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>

// Constraint similar to ODE Hinge2 Joint
// has 3 degrees of frredom:
// 2 rotational degrees of freedom, similar to Euler rotations around Z (axis 1) and X (axis 2)
// 1 translational (along axis Z) with suspension spring

ATTRIBUTE_ALIGNED16(class)
Hinge2Constraint : public Generic6DofSpring2Constraint
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
	Hinge2Constraint(RigidBody & rbA, RigidBody & rbB, Vec3 & anchor, Vec3 & axis1, Vec3 & axis2);
	// access
	const Vec3& getAnchor() { return m_calculatedTransformA.getOrigin(); }
	const Vec3& getAnchor2() { return m_calculatedTransformB.getOrigin(); }
	const Vec3& getAxis1() { return m_axis1; }
	const Vec3& getAxis2() { return m_axis2; }
	Scalar getAngle1() { return getAngle(2); }
	Scalar getAngle2() { return getAngle(0); }
	// limits
	void setUpperLimit(Scalar ang1max) { setAngularUpperLimit(Vec3(-1.f, 0.f, ang1max)); }
	void setLowerLimit(Scalar ang1min) { setAngularLowerLimit(Vec3(1.f, 0.f, ang1min)); }
};

#endif  // DRX3D_HINGE2_CONSTRAINT_H
