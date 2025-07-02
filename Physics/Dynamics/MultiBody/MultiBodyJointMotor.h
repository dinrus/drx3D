#ifndef DRX3D_MULTIBODY_JOINT_MOTOR_H
#define DRX3D_MULTIBODY_JOINT_MOTOR_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
struct SolverInfo;

class MultiBodyJointMotor : public MultiBodyConstraint
{
protected:
	Scalar m_desiredVelocity;
	Scalar m_desiredPosition;
	Scalar m_kd;
	Scalar m_kp;
	Scalar m_erp;
	Scalar m_rhsClamp;  //maximum error

public:
	MultiBodyJointMotor(MultiBody* body, i32 link, Scalar desiredVelocity, Scalar maxMotorImpulse);
	MultiBodyJointMotor(MultiBody* body, i32 link, i32 linkDoF, Scalar desiredVelocity, Scalar maxMotorImpulse);
	virtual ~MultiBodyJointMotor();
	virtual void finalizeMultiDof();

	virtual i32 getIslandIdA() const;
	virtual i32 getIslandIdB() const;

	virtual void createConstraintRows(MultiBodyConstraintArray& constraintRows,
									  MultiBodyJacobianData& data,
									  const ContactSolverInfo& infoGlobal);

	virtual void setVelocityTarget(Scalar velTarget, Scalar kd = 1.f)
	{
		m_desiredVelocity = velTarget;
		m_kd = kd;
	}

	virtual void setPositionTarget(Scalar posTarget, Scalar kp = 1.f)
	{
		m_desiredPosition = posTarget;
		m_kp = kp;
	}

	virtual void setErp(Scalar erp)
	{
		m_erp = erp;
	}
	virtual Scalar getErp() const
	{
		return m_erp;
	}
	virtual void setRhsClamp(Scalar rhsClamp)
	{
		m_rhsClamp = rhsClamp;
	}
	virtual void debugDraw(class IDebugDraw* drawer)
	{
		//todo(erwincoumans)
	}
};

#endif  //DRX3D_MULTIBODY_JOINT_MOTOR_H
