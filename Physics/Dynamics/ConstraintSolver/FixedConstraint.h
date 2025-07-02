#ifndef DRX3D_FIXED_CONSTRAINT_H
#define DRX3D_FIXED_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>

ATTRIBUTE_ALIGNED16(class)
FixedConstraint : public Generic6DofSpring2Constraint
{
public:
	FixedConstraint(RigidBody & rbA, RigidBody & rbB, const Transform2& frameInA, const Transform2& frameInB);

	virtual ~FixedConstraint();
};

#endif  //DRX3D_FIXED_CONSTRAINT_H
