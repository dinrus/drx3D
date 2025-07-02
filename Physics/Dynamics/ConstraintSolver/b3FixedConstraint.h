
#ifndef D3_FIXED_CONSTRAINT_H
#define D3_FIXED_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/ConstraintSolver/b3TypedConstraint.h>

D3_ATTRIBUTE_ALIGNED16(class)
b3FixedConstraint : public b3TypedConstraint
{
	b3Vec3 m_pivotInA;
	b3Vec3 m_pivotInB;
	b3Quat m_relTargetAB;

public:
	b3FixedConstraint(i32 rbA, i32 rbB, const b3Transform& frameInA, const b3Transform& frameInB);

	virtual ~b3FixedConstraint();

	virtual void getInfo1(b3ConstraintInfo1 * info, const b3RigidBodyData* bodies);

	virtual void getInfo2(b3ConstraintInfo2 * info, const b3RigidBodyData* bodies);

	virtual void setParam(i32 num, b3Scalar value, i32 axis = -1)
	{
		drx3DAssert(0);
	}
	virtual b3Scalar getParam(i32 num, i32 axis = -1) const
	{
		drx3DAssert(0);
		return 0.f;
	}
};

#endif  //D3_FIXED_CONSTRAINT_H
