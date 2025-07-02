#ifndef DRX3D_MULTIBODY_SPHERICAL_JOINT_LIMIT_H
#define DRX3D_MULTIBODY_SPHERICAL_JOINT_LIMIT_H

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
struct SolverInfo;

class MultiBodySphericalJointLimit : public MultiBodyConstraint
{
protected:
	Vec3 m_desiredVelocity;
	Quat m_desiredPosition;
	bool m_use_multi_dof_params;
	Vec3 m_kd;
	Vec3 m_kp;
	Scalar m_erp;
	Scalar m_rhsClamp;  //maximum error
	Vec3 m_maxAppliedImpulseMultiDof;
	Vec3 m_pivotA;
	Vec3 m_pivotB;
	Scalar m_swingxRange;
	Scalar m_swingyRange;
	Scalar m_twistRange;

public:
	MultiBodySphericalJointLimit(MultiBody* body, i32 link, 
		Scalar swingxRange,
		Scalar swingyRange,
		Scalar twistRange,
		Scalar maxAppliedImpulse);
	
	virtual ~MultiBodySphericalJointLimit();
	virtual void finalizeMultiDof();

	virtual i32 getIslandIdA() const;
	virtual i32 getIslandIdB() const;

	virtual void createConstraintRows(MultiBodyConstraintArray& constraintRows,
									  MultiBodyJacobianData& data,
									  const ContactSolverInfo& infoGlobal);

	virtual void setVelocityTarget(const Vec3& velTarget, Scalar kd = 1.0)
	{
		m_desiredVelocity = velTarget;
		m_kd = Vec3(kd, kd, kd);
		m_use_multi_dof_params = false;
	}

	virtual void setVelocityTargetMultiDof(const Vec3& velTarget, const Vec3& kd = Vec3(1.0, 1.0, 1.0))
	{
		m_desiredVelocity = velTarget;
		m_kd = kd;
		m_use_multi_dof_params = true;
	}

	virtual void setPositionTarget(const Quat& posTarget, Scalar kp =1.f)
	{
		m_desiredPosition = posTarget;
		m_kp = Vec3(kp, kp, kp);
		m_use_multi_dof_params = false;
	}

	virtual void setPositionTargetMultiDof(const Quat& posTarget, const Vec3& kp = Vec3(1.f, 1.f, 1.f))
	{
		m_desiredPosition = posTarget;
		m_kp = kp;
		m_use_multi_dof_params = true;
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

	Scalar getMaxAppliedImpulseMultiDof(i32 i) const
	{
		return m_maxAppliedImpulseMultiDof[i];
	}

	void setMaxAppliedImpulseMultiDof(const Vec3& maxImp)
	{
		m_maxAppliedImpulseMultiDof = maxImp;
		m_use_multi_dof_params = true;
	}


	virtual void debugDraw(class IDebugDraw* drawer);

};

#endif  //DRX3D_MULTIBODY_SPHERICAL_JOINT_LIMIT_H
