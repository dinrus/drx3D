#ifndef DRX3D_MULTIBODY_SLIDER_CONSTRAINT_H
#define DRX3D_MULTIBODY_SLIDER_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>

class MultiBodySliderConstraint : public MultiBodyConstraint
{
protected:
	RigidBody* m_rigidBodyA;
	RigidBody* m_rigidBodyB;
	Vec3 m_pivotInA;
	Vec3 m_pivotInB;
	Matrix3x3 m_frameInA;
	Matrix3x3 m_frameInB;
	Vec3 m_jointAxis;

public:
	MultiBodySliderConstraint(MultiBody* body, i32 link, RigidBody* bodyB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB, const Vec3& jointAxis);
	MultiBodySliderConstraint(MultiBody* bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB, const Vec3& jointAxis);

	virtual ~MultiBodySliderConstraint();

	virtual void finalizeMultiDof();

	virtual i32 getIslandIdA() const;
	virtual i32 getIslandIdB() const;

	virtual void createConstraintRows(MultiBodyConstraintArray& constraintRows,
									  MultiBodyJacobianData& data,
									  const ContactSolverInfo& infoGlobal);

	const Vec3& getPivotInA() const
	{
		return m_pivotInA;
	}

	void setPivotInA(const Vec3& pivotInA)
	{
		m_pivotInA = pivotInA;
	}

	const Vec3& getPivotInB() const
	{
		return m_pivotInB;
	}

	virtual void setPivotInB(const Vec3& pivotInB)
	{
		m_pivotInB = pivotInB;
	}

	const Matrix3x3& getFrameInA() const
	{
		return m_frameInA;
	}

	void setFrameInA(const Matrix3x3& frameInA)
	{
		m_frameInA = frameInA;
	}

	const Matrix3x3& getFrameInB() const
	{
		return m_frameInB;
	}

	virtual void setFrameInB(const Matrix3x3& frameInB)
	{
		m_frameInB = frameInB;
	}

	const Vec3& getJointAxis() const
	{
		return m_jointAxis;
	}

	void setJointAxis(const Vec3& jointAxis)
	{
		m_jointAxis = jointAxis;
	}

	virtual void debugDraw(class IDebugDraw* drawer);
};

#endif  //DRX3D_MULTIBODY_SLIDER_CONSTRAINT_H
