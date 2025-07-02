#ifndef DRX3D_MULTIBODY_POINT2POINT_H
#define DRX3D_MULTIBODY_POINT2POINT_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>

//#define BTMBP2PCONSTRAINT_BLOCK_ANGULAR_MOTION_TEST

ATTRIBUTE_ALIGNED16(class)
MultiBodyPoint2Point : public MultiBodyConstraint
{
protected:
	RigidBody* m_rigidBodyA;
	RigidBody* m_rigidBodyB;
	Vec3 m_pivotInA;
	Vec3 m_pivotInB;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	MultiBodyPoint2Point(MultiBody * body, i32 link, RigidBody* bodyB, const Vec3& pivotInA, const Vec3& pivotInB);
	MultiBodyPoint2Point(MultiBody * bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB);

	virtual ~MultiBodyPoint2Point();

	virtual void finalizeMultiDof();

	virtual i32 getIslandIdA() const;
	virtual i32 getIslandIdB() const;

	virtual void createConstraintRows(MultiBodyConstraintArray & constraintRows,
									  MultiBodyJacobianData & data,
									  const ContactSolverInfo& infoGlobal);

	const Vec3& getPivotInB() const
	{
		return m_pivotInB;
	}

	virtual void setPivotInB(const Vec3& pivotInB)
	{
		m_pivotInB = pivotInB;
	}

	virtual void debugDraw(class IDebugDraw * drawer);
};

#endif  //DRX3D_MULTIBODY_POINT2POINT_H
