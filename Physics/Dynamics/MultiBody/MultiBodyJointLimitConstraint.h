#ifndef DRX3D_MULTIBODY_JOINT_LIMIT_CONSTRAINT_H
#define DRX3D_MULTIBODY_JOINT_LIMIT_CONSTRAINT_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
struct SolverInfo;

class MultiBodyJointLimitConstraint : public MultiBodyConstraint
{
protected:
	Scalar m_lowerBound;
	Scalar m_upperBound;

public:
	MultiBodyJointLimitConstraint(MultiBody* body, i32 link, Scalar lower, Scalar upper);
	virtual ~MultiBodyJointLimitConstraint();

	virtual void finalizeMultiDof();

	virtual i32 getIslandIdA() const;
	virtual i32 getIslandIdB() const;

	virtual void createConstraintRows(MultiBodyConstraintArray& constraintRows,
									  MultiBodyJacobianData& data,
									  const ContactSolverInfo& infoGlobal);

	virtual void debugDraw(class IDebugDraw* drawer)
	{
		//todo(erwincoumans)
	}
	Scalar getLowerBound() const
	{
		return m_lowerBound;
	}
	Scalar getUpperBound() const
	{
		return m_upperBound;
	}
	void setLowerBound(Scalar lower)
	{
		m_lowerBound = lower;
	}
	void setUpperBound(Scalar upper)
	{
		m_upperBound = upper;
	}
};

#endif  //DRX3D_MULTIBODY_JOINT_LIMIT_CONSTRAINT_H
