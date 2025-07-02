#include <drx3D/Physics/Dynamics/ConstraintSolver/FixedConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Transform2Util.h>
#include <new>

FixedConstraint::FixedConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB)
	: Generic6DofSpring2Constraint(rbA, rbB, frameInA, frameInB)
{
	setAngularLowerLimit(Vec3(0, 0, 0));
	setAngularUpperLimit(Vec3(0, 0, 0));
	setLinearLowerLimit(Vec3(0, 0, 0));
	setLinearUpperLimit(Vec3(0, 0, 0));
}

FixedConstraint::~FixedConstraint()
{
}
