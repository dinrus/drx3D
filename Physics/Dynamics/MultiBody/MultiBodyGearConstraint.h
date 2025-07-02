#ifndef DRX3D_MULTIBODY_GEAR_CONSTRAINT_H
#define DRX3D_MULTIBODY_GEAR_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>

class MultiBodyGearConstraint : public MultiBodyConstraint
{
protected:
	RigidBody* m_rigidBodyA;
	RigidBody* m_rigidBodyB;
	Vec3 m_pivotInA;
	Vec3 m_pivotInB;
	Matrix3x3 m_frameInA;
	Matrix3x3 m_frameInB;
	Scalar m_gearRatio;
	i32 m_gearAuxLink;
	Scalar m_erp;
	Scalar m_relativePositionTarget;

public:
	//MultiBodyGearConstraint(MultiBody* body, i32 link, RigidBody* bodyB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB);
	MultiBodyGearConstraint(MultiBody* bodyA, i32 linkA, MultiBody* bodyB, i32 linkB, const Vec3& pivotInA, const Vec3& pivotInB, const Matrix3x3& frameInA, const Matrix3x3& frameInB);

	virtual ~MultiBodyGearConstraint();

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

	virtual void debugDraw(class IDebugDraw* drawer)
	{
		//todo(erwincoumans)
	}

	virtual void setGearRatio(Scalar gearRatio)
	{
		m_gearRatio = gearRatio;
	}
	virtual void setGearAuxLink(i32 gearAuxLink)
	{
		m_gearAuxLink = gearAuxLink;
	}
	virtual void setRelativePositionTarget(Scalar relPosTarget)
	{
		m_relativePositionTarget = relPosTarget;
	}
	virtual void setErp(Scalar erp)
	{
		m_erp = erp;
	}
};

#endif  //DRX3D_MULTIBODY_GEAR_CONSTRAINT_H
